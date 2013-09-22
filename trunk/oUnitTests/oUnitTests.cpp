// $(header)
#include <oPlatform/oReporting.h>
#include <oPlatform/oConsole.h>
#include <oPlatform/oImage.h>
#include <oPlatform/oMsgBox.h>
#include <oPlatform/oTest.h>
#include <oPlatform/oStandards.h>
#include <oPlatform/oStream.h>
#include <oCore/system.h>
#include <oStd/opttok.h>
#include <oStd/scc.h>

static const char* sTITLE = "OOOii Unit Test Suite";

static const oStd::option sCmdLineOptions[] = 
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
	// @oooii-tony: TODO: FIND OUT - why can DllMain execute in a not-main thread?

	oConcurrency::init_task_scheduler();

	oTRACEA("Aero is %sactive", oCore::system::uses_gpu_compositing() ? "" : "in");
	oTRACE("Remote desktop is %sactive", oCore::system::is_remote_session() ? "" : "in");

	// IOCP needs to be initialized or it will show up as a leak in the first test
	// to use it.
	void InitializeIOCP();
	InitializeIOCP();

	oCore::module::info mi = oCore::this_module::get_info();
	oStd::sstring Ver;
	oStd::mstring title2(sTITLE);
	oStd::sncatf(title2, " v%s%s", oStd::to_string(Ver, mi.version), mi.is_special ? "*" : "");
	oConsole::SetTitle(title2);

	// Resize console
	{
		oConsole::DESC desc;
		desc.BufferWidth = 255;
		desc.BufferHeight = 1024;
		desc.Top = 10;
		desc.Left = 10;
		desc.Width = 120;
		desc.Height = 50;
		desc.Foreground = oStd::LimeGreen;
		desc.Background = oStd::Black;
		oConsole::SetDesc(&desc);
	}
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
	auto scc = oStd::make_scc(oStd::scc_protocol::svn, std::bind(oCore::system::spawn, std::placeholders::_1, std::placeholders::_2, false, std::placeholders::_3));

	oStd::path BranchPath = oCore::filesystem::dev_path();

	std::vector<oStd::scc_file> temp;
	temp.resize(_NumOpenedFilesToTest);

	// If we can't connect to SCC, then always run all tests.
	std::vector<oStd::scc_file> ModifiedFiles;
	try
	{ 
		scc->status(BranchPath, 0, oStd::scc_visit_option::modified_only, [&](const oStd::scc_file& _File)
		{
			ModifiedFiles.push_back(_File);
		});
	}
	catch (std::exception&)
	{
		return oErrorSetLast(std::errc::io_error, "oSCCPathHasChanges could not find modified files. This may indicate %s is not accessible.", oStd::as_string(scc->protocol()));
	}

	memset(_pHasChanges, 0, _NumPathParts);

	oStd::path_string LibWithSeps;
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
	char ch = oStd::opttok(&value, _Argc, _Argv, sCmdLineOptions);
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

		ch = oStd::opttok(&value);
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
					auto pFilter = oStd::append(_pParameters->Filters);
					pFilter->Type = oFilterChain::EXCLUDE1;
					pFilter->RegularExpression = sFilter[i];
				}
			}
		}
	}
}

struct oNamedFileDesc
{
	oStd::path Path;
	time_t LastWritten;
	static bool NewerToOlder(const oNamedFileDesc& _File1, const oNamedFileDesc& _File2)
	{
		return _File1.LastWritten > _File2.LastWritten;
	}
};

void DeleteOldLogFiles(const char* _SpecialModeName)
{
	const size_t kLogHistory = 10;

	char logFileWildcard[_MAX_PATH];
	oGetLogFilePath(logFileWildcard, _SpecialModeName);

	char* p = oStd::rstrstr(logFileWildcard, "_");
	strlcpy(p, "*.stdout", oCOUNTOF(logFileWildcard) - std::distance(logFileWildcard, p));

	std::vector<oNamedFileDesc> logs;
	logs.reserve(20);

	oCore::filesystem::enumerate(logFileWildcard, [&](const oStd::path& _FullPath, const oCore::filesystem::file_status& _Status, unsigned long long _Size)->bool
	{
		oNamedFileDesc nfd;
		nfd.LastWritten = oCore::filesystem::last_write_time(_FullPath);
		nfd.Path = _FullPath;
		logs.push_back(nfd);
		return true;
	});

	if (logs.size() > kLogHistory)
	{
		std::sort(logs.begin(), logs.end(), oNamedFileDesc::NewerToOlder);
		for (size_t i = kLogHistory; i < logs.size(); i++)
			oCore::filesystem::remove_filename(logs[i].Path.replace_extension(".stderr"));
	}
}

void EnableLogFile(const char* _SpecialModeName, const char* _LogFileName)
{
	char logFilePath[_MAX_PATH];
	if (_LogFileName == nullptr)
		oGetLogFilePath(logFilePath, _SpecialModeName);
	else
		strlcpy(logFilePath, _LogFileName);
	oREPORTING_DESC desc;
	oReportingGetDesc(&desc);
	desc.LogFilePath = logFilePath;

	// insert .stderr at end of filebasename
	auto ext = desc.LogFilePath.extension();
	desc.LogFilePath.replace_extension(".stderr");
	desc.LogFilePath.append(ext, false);

	oConsole::DESC cdesc;
	oConsole::GetDesc(&cdesc);
	cdesc.LogFilePath = logFilePath;

	// insert .stdout at end of filebasename
	ext = cdesc.LogFilePath.extension();
	cdesc.LogFilePath.replace_extension(".stdout");
	cdesc.LogFilePath.append(ext, false);

	oConsole::SetDesc(&cdesc);

	oStd::path DumpBase = oCore::filesystem::app_path(true);
	DumpBase.replace_extension();
	if (_SpecialModeName)
	{
		oStd::sstring suffix;
		snprintf(suffix, "-%s", _SpecialModeName);
		DumpBase /= suffix;
	}

	desc.MiniDumpBase = DumpBase;
	desc.PromptAfterDump = false;
	oReportingSetDesc(desc);
}

void SetTestManagerDesc(const PARAMETERS* _pParameters)
{
	oStd::path dataPath;

	if (_pParameters->DataPath)
		dataPath = _pParameters->DataPath;
	else
		dataPath = oCore::filesystem::data_path();

	// @oooii-tony: This is important to be here for now because it touches the 
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

static bool FindDuplicateProcessInstanceByName(oCore::process::id _ProcessID, oCore::process::id _ParentProcessID, const char* _ProcessExePath, oCore::process::id _IgnorePID, const char* _FindName, oCore::process::id* _pOutPID)
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
	oCore::process::id ThisID = oCore::this_process::get_id();
	oCore::process::id duplicatePID;
	oCore::process::enumerate(std::bind(FindDuplicateProcessInstanceByName
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

		oCore::process::terminate(duplicatePID, ECANCELED);
		if (!oCore::process::wait_for(duplicatePID, oSeconds(5)))
			oMsgBox(mb, "Cannot terminate stale process %u, please end this process before continuing.", duplicatePID);

		duplicatePID = oCore::process::id();
		oCore::process::enumerate(std::bind(FindDuplicateProcessInstanceByName
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
	oStd::path tmp = oCore::filesystem::app_path(true);
	oStd::path path(tmp);
	oStd::path relname = path.basename();
	relname.remove_basename_suffix(oMODULE_DEBUG_SUFFIX_A);

	oStd::path dbgname(relname);
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
		oReportingEnableErrorDialogBoxes(false);

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

		if (oCore::this_process::has_debugger_attached())
		{
			system("echo.");
			system("pause");
		}
	}

	if (parameters.SpecialMode)
		oTRACE("Unit test (special mode %s) exiting with result: %s", parameters.SpecialMode, oStd::as_string((oTest::RESULT)result));
	else
		oTRACE("Unit test exiting with result: %s", oStd::as_string((oTest::RESULT)result));

	// Final flush to ensure oBuildTool gets all our stdout
	::_flushall();

	return result;
}
