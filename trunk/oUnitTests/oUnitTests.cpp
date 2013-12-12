// $(header)
#include <oGUI/console.h>
#include <oGUI/oMsgBox.h>
#include <oPlatform/oTest.h>
#include <oPlatform/oStandards.h>
#include <oPlatform/oStream.h>
#include <oCore/reporting.h>
#include <oCore/system.h>
#include <oBase/opttok.h>
#include <oBase/scc.h>
#include <oGUI/Windows/oGDI.h>
#include "resource.h"

#include <oCore/windows/win_exception_handler.h>

using namespace ouro;

static const char* sTITLE = "Ouroboros Unit Test Suite";

static const option sCmdLineOptions[] = 
{
	{ 'i', "include", "regex", "regex matching a test name to include" },
	{ 'e', "exclude", "regex", "regex matching a test name to exclude" },
	{ 'p', "path", "path", "Path where all test data is loaded from. The current working directory is used by default." },
	{ 's', "special-mode", "mode", "Run the test harness in a special mode (used mostly by multi-process/client-server unit tests)" },
	{ 'r', "random-seed", "seed", "Set the random seed to be used for this run. This is reset at the start of each test." },
	{ 'b', "golden-binaries", "path", "Path where all known-good \"golden\" binaries are stored. The current working directory is used by default." },
	{ 'g', "golden-images", "path", "Path where all known-good \"golden\" images are stored. The current working directory is used by default." },
	{ 'z', "output-golden-images", 0, "Copy golden images of error images to the output as well, renamed to <image>_golden.png." },
	{ 'o', "output", "path", "Path where all logging and error images are created." },
	{ 'n', "repeat-number", "nRuntimes", "Repeat the test a certain number of times." },
	{ 'd', "disable-timeouts", 0, "Disable timeouts, mainly while debugging." },
	{ 'c', "capture-callstack", 0, "Capture full callstack to allocations for per-test leaks (slow!)" },
	{ '_', "disable-leaktracking", 0, "Disable the leak tracking when it is suspected of causing performance issues" },
	{ 'a', "automated", 0, "Run unit tests in automated mode, disable dialog boxes and exit on critical failure" },
	{ 'l', "logfile", "path", "Uses specified path for the log file" },
	{ 'x', "Exhaustive", 0, "Run tests in exhaustive mode. Probably should only be run in Release. May take a very long time." },
};

void InitEnv()
{
	// Situation:
	// exe statically linked to oooii lib
	// dll statically linked to oooii lib
	// exe hard-linked to dll
	//
	// In this case we're seeing that it is possible for DllMain 
	// to be called on a thread that is NOT the main thread. Strange, no?
	// This would cause TBB, the underlying implementation of oConcurrency::parallel_for
	// and friends, to be initialized in a non-main thread. This upsets TBB,
	// so disable static init of TBB and force initialization here in a 
	// function known to execute on the main thread.
	//
	// @tony: TODO: FIND OUT - why can DllMain execute in a not-main thread?

	oConcurrency::init_task_scheduler();

	oTRACEA("Aero is %sactive", system::uses_gpu_compositing() ? "" : "in");
	oTRACE("Remote desktop is %sactive", system::is_remote_session() ? "" : "in");

	// IOCP needs to be initialized or it will show up as a leak in the first test
	// to use it.
	void InitializeIOCP();
	InitializeIOCP();

	module::info mi = this_module::get_info();
	sstring Ver;
	mstring title2(sTITLE);
	sncatf(title2, " v%s%s", to_string(Ver, mi.version), mi.is_special ? "*" : "");
	console::set_title(title2);

	// Resize console
	try
	{
		console::info i;
		i.buffer_size = int2(255, 1024);
		i.window_position = int2(10, 10);
		i.window_size = int2(120, 50);
		i.foreground = LimeGreen;
		i.background = Black;
		console::set_info(i);
	}
	catch (std::exception& e)
	{
		oTRACEA("console::set_info failed: %s", e.what());
	}

#if defined(WIN64) || defined(WIN32)
	oGDIScopedObject<HICON> hIcon = oGDILoadIcon(IDI_APPICON);
	oWinSetIconAsync(GetConsoleWindow(), hIcon, false);
	oWinSetIconAsync(GetConsoleWindow(), hIcon, true);
#endif
}

struct PARAMETERS
{
	std::vector<oFilterChain::FILTER> Filters;
	const char* SpecialMode;
	const char* DataPath;
	const char* GoldenBinariesPath;
	const char* GoldenImagesPath;
	const char* OutputPath;
	unsigned int RandomSeed;
	unsigned int RepeatNumber;
	bool EnableTimeouts;
	bool CaptureCallstackForTestLeaks;
	bool EnableLeakTracking;
	bool EnableAutomatedMode;
	const char* LogFilePath;
	bool Exhaustive;
	bool EnableOutputGoldenImages;
};

// _pHasChanges should be large enough to receive a result for each of the 
// specified path parts.
static bool oSCCCheckPathHasChanges(const char** _pPathParts, size_t _NumPathParts, bool* _pHasChanges, size_t _NumOpenedFilesToTest = 128)
{
	auto scc = make_scc(scc_protocol::svn, std::bind(system::spawn, std::placeholders::_1, std::placeholders::_2, false, std::placeholders::_3));

	path BranchPath = filesystem::dev_path();

	std::vector<scc_file> temp;
	temp.resize(_NumOpenedFilesToTest);

	// If we can't connect to SCC, then always run all tests.
	std::vector<scc_file> ModifiedFiles;
	try
	{ 
		scc->status(BranchPath, 0, scc_visit_option::modified_only, [&](const scc_file& _File)
		{
			ModifiedFiles.push_back(_File);
		});
	}
	catch (std::exception&)
	{
		return oErrorSetLast(std::errc::io_error, "oSCCPathHasChanges could not find modified files. This may indicate %s is not accessible.", as_string(scc->protocol()));
	}

	memset(_pHasChanges, 0, _NumPathParts);

	path_string LibWithSeps;
	for (size_t i = 0; i < _NumPathParts; i++)
	{
		snprintf(LibWithSeps, "/%s/", _pPathParts[i]);
		oFOR(const auto& f, ModifiedFiles)
		{
			if (strstr(f.path, LibWithSeps))
			{
				_pHasChanges[i] = true;
				break;
			}
		}
	}

	return true;
}
template<size_t size> bool oSCCCheckPathHasChanges(const char* (&_pPathParts)[size], bool (&_pHasChanges)[size], size_t _NumOpenedFilesToTest = 128) { return oSCCCheckPathHasChanges(_pPathParts, size, _pHasChanges, _NumOpenedFilesToTest); }

void ParseCommandLine(int _Argc, const char* _Argv[], PARAMETERS* _pParameters)
{
	_pParameters->Filters.clear();
	_pParameters->SpecialMode = nullptr;
	_pParameters->DataPath = nullptr;
	_pParameters->GoldenBinariesPath = nullptr;
	_pParameters->GoldenImagesPath = nullptr;
	_pParameters->OutputPath = nullptr;
	_pParameters->RandomSeed = 0;
	_pParameters->RepeatNumber = 1;
	_pParameters->EnableTimeouts = true;
	_pParameters->CaptureCallstackForTestLeaks = false;
	_pParameters->EnableLeakTracking = true;
	_pParameters->EnableAutomatedMode = false;
	_pParameters->LogFilePath = nullptr;
	_pParameters->Exhaustive = false;
	_pParameters->EnableOutputGoldenImages = false;

	const char* value = 0;
	char ch = opttok(&value, _Argc, _Argv, sCmdLineOptions);
	while (ch)
	{
		switch (ch)
		{
			case 'i':
			{
				_pParameters->Filters.resize(_pParameters->Filters.size() + 1);
				_pParameters->Filters.back().Type = oFilterChain::INCLUDE1;
				_pParameters->Filters.back().RegularExpression = value;
				break;
			}

			case 'e':
			{
				_pParameters->Filters.resize(_pParameters->Filters.size() + 1);
				_pParameters->Filters.back().Type = oFilterChain::EXCLUDE1;
				_pParameters->Filters.back().RegularExpression = value;
				break;
			}

			case 'p': _pParameters->DataPath = value; break;
			case 's': _pParameters->SpecialMode = value; break;
			case 'r': _pParameters->RandomSeed = atoi(value) ? atoi(value) : 1;  break; // ensure it's not 0, meaning choose randomly
			case 'b': _pParameters->GoldenBinariesPath = value; break;
			case 'g': _pParameters->GoldenImagesPath = value; break;
			case 'o': _pParameters->OutputPath = value; break;
			case 'n': _pParameters->RepeatNumber = atoi(value); break;
			case 'd': _pParameters->EnableTimeouts = false; break;
			case 'c': _pParameters->CaptureCallstackForTestLeaks = true; break;
			case '_': _pParameters->EnableLeakTracking = false; break;
			case 'a': _pParameters->EnableAutomatedMode = true; break;
			case 'l': _pParameters->LogFilePath = value; break;
			case 'x': _pParameters->Exhaustive = true; break;
			case 'z': _pParameters->EnableOutputGoldenImages = true; break;
			default: break;
		}

		ch = opttok(&value);
	}

	// @ooii-tony: Disabled in the OpenSource distro because running the unit 
	// tests is the only proof of life in the Ouroboros branch
	static bool IsOpenSourceDistribution = /*true*/false;

	// oBasis is pretty stable these days, only test if there are changes.
	// Some libs have less and less changes these days, so in the common case if 
	// there are no modifications to them, don't build them
	if (_pParameters->Filters.empty() && !IsOpenSourceDistribution)
	{
		const char* sLibNames[] =
		{
			"oStd",
			"oCore",
			"oHLSL",
			"oCompute",
			"oConcurrency",
			"oBasis",
			"oPlatform",
			"oGPU",
		};
		const char* sFilter[] =
		{
			"oStd_.*",
			"oCore_.*",
			"oHLSL.*",
			"oCompute_.*",
			"oConcurrency_.*",
			"oBasis_.*",
			"PLATFORM_.*",
			"GPU_.*",
		};
		bool HasChanges[oCOUNTOF(sLibNames)];
		if (oSCCCheckPathHasChanges(sLibNames, HasChanges))
		{
			for (int i = 0; i < oCOUNTOF(HasChanges); i++)
			{
				bool ThisHasChanges = HasChanges[i];
				if (!ThisHasChanges)
				{
					for (int j = 0; j < i; j++)
						if (HasChanges[j])
							ThisHasChanges = true;
				}

				if (!ThisHasChanges && oSTRVALID(sFilter))
				{
					auto pFilter = append(_pParameters->Filters);
					pFilter->Type = oFilterChain::EXCLUDE1;
					pFilter->RegularExpression = sFilter[i];
				}
			}
		}
	}
}

struct oNamedFileDesc
{
	path Path;
	time_t LastWritten;
	static bool NewerToOlder(const oNamedFileDesc& _File1, const oNamedFileDesc& _File2)
	{
		return _File1.LastWritten > _File2.LastWritten;
	}
};

void DeleteOldLogFiles(const char* _SpecialModeName)
{
	const size_t kLogHistory = 10;

	path logFileWildcard = filesystem::log_path(true, _SpecialModeName);
	logFileWildcard.replace_extension_with_suffix("_*.stdout");

	std::vector<oNamedFileDesc> logs;
	logs.reserve(20);

	filesystem::enumerate(logFileWildcard, [&](const path& _FullPath, const filesystem::file_status& _Status, unsigned long long _Size)->bool
	{
		oNamedFileDesc nfd;
		nfd.LastWritten = filesystem::last_write_time(_FullPath);
		nfd.Path = _FullPath;
		logs.push_back(nfd);
		return true;
	});

	if (logs.size() > kLogHistory)
	{
		std::sort(logs.begin(), logs.end(), oNamedFileDesc::NewerToOlder);
		for (size_t i = kLogHistory; i < logs.size(); i++)
			filesystem::remove_filename(logs[i].Path.replace_extension(".stderr"));
	}
}

void EnableLogFile(const char* _SpecialModeName, const char* _LogFileName)
{
	path logFilePath = _LogFileName ? _LogFileName : filesystem::log_path(true, _SpecialModeName);
	auto logFileExt = logFilePath.extension();
	
	// Log stdout (that which is printed to the console TTY)
	{
		path logStdout(logFilePath);
		logStdout.replace_extension(".stdout");
		logStdout.append(logFileExt, false);
		console::set_log(logStdout);
	}

	// Log stderr (that which is printed to the debug TTY)
	{
		path logStderr(logFilePath);
		logStderr.replace_extension(".stderr");
		logStderr.append(logFileExt, false);
		reporting::set_log(logStderr);
	}

	// Create a place for the dump file
	{
		path DumpBase = filesystem::app_path(true);
		DumpBase.replace_extension();
		if (_SpecialModeName)
		{
			sstring suffix;
			snprintf(suffix, "-%s", _SpecialModeName);
			DumpBase /= suffix;
		}

		windows::exception::mini_dump_path(DumpBase);
		windows::exception::prompt_after_dump(false);
	}
}

void SetTestManagerDesc(const PARAMETERS* _pParameters)
{
	path dataPath;

	if (_pParameters->DataPath)
		dataPath = _pParameters->DataPath;
	else
		dataPath = filesystem::data_path();

	// @tony: This is important to be here for now because it touches the 
	// underlying singleton so it doesn't appear in unit tests as a leak. Also
	// this should become where the paths to all source data should be registered
	// and thus each test can be simpler and remove BuildPath and other weird
	// one-offy APIs.
	oVERIFY(oStreamSetURIBaseSearchPath(""));

	oTestManager::DESC desc;
	desc.TestSuiteName = sTITLE;
	desc.DataPath = dataPath;
	desc.GoldenBinariesPath = _pParameters->GoldenBinariesPath;
	desc.GoldenImagesPath = _pParameters->GoldenImagesPath;
	desc.OutputPath = _pParameters->OutputPath;
	desc.NameColumnWidth = 40;
	desc.TimeColumnWidth = 5;
	desc.StatusColumnWidth = 9;
	desc.RandomSeed = _pParameters->RandomSeed ? _pParameters->RandomSeed : (unsigned int)oStd::chrono::high_resolution_clock::now().time_since_epoch().count();
	desc.NumRunIterations = _pParameters->RepeatNumber ? _pParameters->RepeatNumber : 1;
	desc.EnableSpecialTestTimeouts = _pParameters->EnableTimeouts;
	desc.CaptureCallstackForTestLeaks = _pParameters->CaptureCallstackForTestLeaks;
	desc.EnableLeakTracking = _pParameters->EnableLeakTracking;
	desc.Exhaustive = _pParameters->Exhaustive;
	desc.AutomatedMode = _pParameters->EnableAutomatedMode;
	desc.EnableOutputGoldenImages = _pParameters->EnableOutputGoldenImages;

	oTestManager::Singleton()->SetDesc(&desc);
}

static bool FindDuplicateProcessInstanceByName(process::id _ProcessID, process::id _ParentProcessID, const char* _ProcessExePath, process::id _IgnorePID, const char* _FindName, process::id* _pOutPID)
{
	if (_IgnorePID != _ProcessID && !_stricmp(_FindName, _ProcessExePath))
	{
		*_pOutPID = _ProcessID;
		return false;
	}

	return true;
}

bool TerminateDuplicateInstances(const char* _Name)
{
	process::id ThisID = this_process::get_id();
	process::id duplicatePID;
	process::enumerate(std::bind(FindDuplicateProcessInstanceByName
		, std::placeholders::_1
		, std::placeholders::_2
		, std::placeholders::_3
		, ThisID
		, _Name
		, &duplicatePID));

	while (duplicatePID)
	{
		oMSGBOX_DESC mb;
		mb.Type = oMSGBOX_YESNO;
		mb.TimeoutMS = 20000;
		mb.Title = sTITLE;

		oMSGBOX_RESULT result = oMsgBox(mb, "An instance of the unittest executable was found at process %u. Do you want to kill it now? (no means this application will exit)", duplicatePID);
		if (result == oMSGBOX_NO)
			return false;

		process::terminate(duplicatePID, ECANCELED);
		if (!process::wait_for(duplicatePID, oSeconds(5)))
			oMsgBox(mb, "Cannot terminate stale process %u, please end this process before continuing.", duplicatePID);

		duplicatePID = process::id();
		process::enumerate(std::bind(FindDuplicateProcessInstanceByName
			, std::placeholders::_1
			, std::placeholders::_2
			, std::placeholders::_3
			, ThisID
			, _Name
			, &duplicatePID));
	}

	return true;
}

bool EnsureOneInstanceIsRunning()
{
	// Scan for both release and debug builds already running
	path tmp = filesystem::app_path(true);
	path Path(tmp);
	path relname = Path.basename();
	relname.remove_basename_suffix(oMODULE_DEBUG_SUFFIX_A);

	path dbgname(relname);
	dbgname.insert_basename_suffix(oMODULE_DEBUG_SUFFIX_A);

	if (!TerminateDuplicateInstances(dbgname))
		return false;
	if (!TerminateDuplicateInstances(relname))
		return false;

	return true;
}

int main(int argc, const char* argv[])
{
	#ifdef DEBUG_oCAMERA
		extern int ShowAllCameras();
		ShowAllCameras();
		if (1) return 0;
	#endif

	InitEnv();

	PARAMETERS parameters;
	ParseCommandLine(argc, argv, &parameters);
	SetTestManagerDesc(&parameters);
	DeleteOldLogFiles(parameters.SpecialMode);
	EnableLogFile(parameters.SpecialMode, parameters.LogFilePath);
	if (parameters.EnableAutomatedMode)
		windows::exception::enable_dialogs(false);

	{
		reporting::info ri;
		ri.prefix_thread_id = false;
		reporting::set_info(ri);
	}

	int result = 0;

	if (parameters.SpecialMode)
		result = oTestManager::Singleton()->RunSpecialMode(parameters.SpecialMode);
	else
	{
		if (EnsureOneInstanceIsRunning())
			result = oTestManager::Singleton()->RunTests(parameters.Filters.empty() ? 0 : &parameters.Filters[0], parameters.Filters.size());
		else
			result = oTest::SKIPPED;

		bool ShowTrayStatus = false;

		// This is really just a unit test for wintray. Disabled because of the async
		// nature of the WinTray teardown causes either a delay or a false positive
		// mem leak report.
		// ShowTrayStatus = true;

		if (ShowTrayStatus)
		{
			oMSGBOX_DESC mb;
			mb.Type = result ? oMSGBOX_NOTIFY_ERR : oMSGBOX_NOTIFY;
			mb.TimeoutMS = 10000;
			mb.Title = sTITLE;
			oMsgBox(mb, "Completed%s", result ? " with errors" : " successfully");
		}

		if (this_process::has_debugger_attached())
		{
			::system("echo.");
			::system("pause");
		}
	}

	if (parameters.SpecialMode)
		oTRACE("Unit test (special mode %s) exiting with result: %s", parameters.SpecialMode, as_string((oTest::RESULT)result));
	else
		oTRACE("Unit test exiting with result: %s", as_string((oTest::RESULT)result));

	// Final flush to ensure oBuildTool gets all our stdout
	::_flushall();

	return result;
}
