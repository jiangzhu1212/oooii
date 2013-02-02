// $(header)
#include <oBasis/oString.h>
#include <oPlatform/oReporting.h>
#include <oPlatform/oConsole.h>
#include <oPlatform/oDebugger.h>
#include <oPlatform/oFile.h>
#include <oPlatform/oImage.h>
#include <oPlatform/oModule.h>
#include <oPlatform/oMsgBox.h>
#include <oPlatform/oTest.h>
#include <oPlatform/oP4.h>
#include <oPlatform/oProcess.h>
#include <oPlatform/oStandards.h>
#include <oPlatform/oSystem.h>
#include <oBasis/oTask.h>
#include <oBasis/oRef.h>

static const char* sTITLE = "OOOii Unit Test Suite";

static const oOption sCmdLineOptions[] = 
{
	{ "include", 'i', "regex", "regex matching a test name to include" },
	{ "exclude", 'e', "regex", "regex matching a test name to exclude" },
	{ "path", 'p', "path", "Path where all test data is loaded from. The current working directory is used by default." },
	{ "special-mode", 's', "mode", "Run the test harness in a special mode (used mostly by multi-process/client-server unit tests)" },
	{ "random-seed", 'r', "seed", "Set the random seed to be used for this run. This is reset at the start of each test." },
	{ "golden-binaries", 'b', "path", "Path where all known-good \"golden\" images are stored. The current working directory is used by default." },
	{ "golden-images", 'g', "path", "Path where all known-good \"golden\" images are stored. The current working directory is used by default." },
	{ "output", 'o', "path", "Path where all logging and error images are created." },
	{ "repeat-number", 'n', "nRuntimes", "Repeat the test a certain number of times." },
	{ "disable-timeouts", 'd', nullptr, "Disable timeouts, mainly while debugging." },
	{ "capture-callstack", 'c', nullptr, "Capture full callstack to allocations for per-test leaks (slow!)" },
	{ "disable-leaktracking", '_', nullptr, "Disable the leak tracking when it is suspected of causing performance issues" },
	{ "automated", 'a', nullptr, "Run unit tests in automated mode, disable dialog boxes and exit on critical failure" },
	{ "logfile", 'l', "path", "Uses specified path for the log file" },
	{ "Exhaustive", 'x', nullptr, "Run tests in exhaustive mode. Probably should only be run in Release. May take a very long time." },
	{ nullptr,0,nullptr,nullptr }
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
	// This would cause TBB, the underlying implementation of oParallelFor
	// and friends, to be initialized in a non-main thread. This upsets TBB,
	// so disable static init of TBB and force initialization here in a 
	// function known to execute on the main thread.
	//
	// @oooii-tony: TODO: FIND OUT - why can DllMain execute in a not-main thread?

	oTaskInitScheduler();

	oTRACEA("Aero is %sactive", oSystemGUIUsesGPUCompositing() ? "" : "in");
	oTRACE("Remote desktop is %sactive", oIsRemoteDesktopConnected() ? "" : "in");

	// IOCP needs to be initialized or it will show up as a leak in the first test
	// to use it.
	void InitializeIOCP();
	InitializeIOCP();

	oMODULE_DESC md;
	oModuleGetDesc(&md);
	oStringS Ver;
	oStringM title2(sTITLE);
	oStrAppendf(title2, " v%s%s", oToString(Ver, md.ProductVersion), md.IsSpecialBuild ? "*" : "");
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
		desc.Foreground = std::LimeGreen;
		desc.Background = std::Black;
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
};

static bool oBasisHasChanges()
{
	oP4_FILE_DESC test[128];
	size_t nOpenFiles = oP4ListOpened(test);
	
	// If we can't connect to P4, then always run all tests.
	if (nOpenFiles == oInvalid)
	{
		oTRACE("NOTE: If P4 is installed and accessible through env vars, oUnitTests will detect if any oBasis files have been modified and if not, it will automatically skip all oBasis tests when no other filter is specified.");
		return true;
	}

	for (size_t i = 0; i < nOpenFiles; i++)
		if (strstr(test[i].Path, "oBasis"))
			return true;
	return false;
}

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

	const char* value = 0;
	char ch = oOptTok(&value, _Argc, _Argv, sCmdLineOptions);
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
			default: break;
		}

		ch = oOptTok(&value, 0, 0, 0);
	}

	// @ooii-tony: Disabled in the OpenSource distro because running the unit tests is the only proof of life in the Ouroboros branch
	static bool IsOpenSourceDistribution = true;

	// oBasis is pretty stable these days, only test if there are changes.
	if (_pParameters->Filters.empty() && !oBasisHasChanges() && !IsOpenSourceDistribution)
	{
		_pParameters->Filters.resize(_pParameters->Filters.size() + 1);
		_pParameters->Filters.back().Type = oFilterChain::EXCLUDE1;
		_pParameters->Filters.back().RegularExpression = "BASIS_.*";
	}
}

struct oNamedFileDesc : oSTREAM_DESC
{
	char FileName[_MAX_PATH];
	static bool NewerToOlder(const oNamedFileDesc& _File1, const oNamedFileDesc& _File2)
	{
		return _File1.Written > _File2.Written;
	}
};

static bool PushBackNFDs(const char* _Path, const oSTREAM_DESC& _Desc, std::vector<oNamedFileDesc>& _NFDs)
{
	oNamedFileDesc nfd;
	reinterpret_cast<oSTREAM_DESC&>(nfd) = _Desc;
	oStrcpy(nfd.FileName, _Path);
	_NFDs.push_back(nfd);
	return true;
}

void DeleteOldLogFiles(const char* _SpecialModeName)
{
	const size_t kLogHistory = 10;

	char logFileWildcard[_MAX_PATH];
	oGetLogFilePath(logFileWildcard, _SpecialModeName);

	char* p = oStrStrReverse(logFileWildcard, "_");
	oStrcpy(p, oCOUNTOF(logFileWildcard) - std::distance(logFileWildcard, p), "*.txt");

	std::vector<oNamedFileDesc> logs;
	logs.reserve(20);
	oFileEnum(logFileWildcard, oBIND(PushBackNFDs, oBIND1, oBIND2, oBINDREF(logs)));

	if (logs.size() > kLogHistory)
	{
		std::sort(logs.begin(), logs.end(), oNamedFileDesc::NewerToOlder);
		for (size_t i = kLogHistory; i < logs.size(); i++)
			oStreamDelete(logs[i].FileName);
	}
}

void EnableLogFile(const char* _SpecialModeName, const char* _LogFileName)
{
	char logFilePath[_MAX_PATH];
	if (_LogFileName == nullptr)
		oGetLogFilePath(logFilePath, _SpecialModeName);
	else
		oStrcpy(logFilePath, _LogFileName);
	oREPORTING_DESC desc;
	oReportingGetDesc(&desc);
	desc.LogFilePath = logFilePath;

	oStringPath DumpBase;
	oSystemGetPath(DumpBase, oSYSPATH_APP_FULL);
	oTrimFileExtension(DumpBase);
	if (_SpecialModeName)
		oStrAppendf(DumpBase, "-%s", _SpecialModeName);

	desc.MiniDumpBase = DumpBase;
	desc.PromptAfterDump = false;
	oReportingSetDesc(desc);
}

void SetTestManagerDesc(const PARAMETERS* _pParameters)
{
	char dataPath[_MAX_PATH];

	if (_pParameters->DataPath)
		oStrcpy(dataPath, _pParameters->DataPath);
	else if (!oSystemGetPath(dataPath, oSYSPATH_DATA))
	{
		oASSERT(false, "Failed to find data dir");
	}

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
	desc.NameColumnWidth = 32;
	desc.TimeColumnWidth = 5;
	desc.StatusColumnWidth = 9;
	desc.RandomSeed = _pParameters->RandomSeed ? _pParameters->RandomSeed : (unsigned int)oStd::chrono::high_resolution_clock::now().time_since_epoch().count();
	desc.NumRunIterations = _pParameters->RepeatNumber ? _pParameters->RepeatNumber : 1;
	desc.EnableSpecialTestTimeouts = _pParameters->EnableTimeouts;
	desc.CaptureCallstackForTestLeaks = _pParameters->CaptureCallstackForTestLeaks;
	desc.EnableLeakTracking = _pParameters->EnableLeakTracking;
	desc.Exhaustive = _pParameters->Exhaustive;
	desc.AutomatedMode = _pParameters->EnableAutomatedMode;

	oTestManager::Singleton()->SetDesc(&desc);
}

static bool FindDuplicateProcessInstanceByName(unsigned int _ProcessID, unsigned int _ParentProcessID, const char* _ProcessExePath, unsigned int _IgnorePID, const char* _FindName, unsigned int* _pOutPID)
{
	if (_IgnorePID != _ProcessID && !oStricmp(_FindName, _ProcessExePath))
	{
		*_pOutPID = _ProcessID;
		return false;
	}

	return true;
}

bool TerminateDuplicateInstances(const char* _Name)
{
	unsigned int ThisID = oProcessGetCurrentID();
	unsigned int duplicatePID = 0;
	oProcessEnum(oBIND(FindDuplicateProcessInstanceByName, oBIND1, oBIND2, oBIND3, ThisID, _Name, &duplicatePID));
	while (duplicatePID)
	{
		oMSGBOX_DESC mb;
		mb.Type = oMSGBOX_YESNO;
		mb.TimeoutMS = 20000;
		mb.Title = sTITLE;

		oMSGBOX_RESULT result = oMsgBox(mb, "An instance of the unittest executable was found at process %u. Do you want to kill it now? (no means this application will exit)", duplicatePID);
		if (result == oMSGBOX_NO)
			return false;

		oProcessTerminate(duplicatePID, ECANCELED);
		if (!oProcessWaitExit(duplicatePID, 5000))
			oMsgBox(mb, "Cannot terminate stale process %u, please end this process before continuing.", duplicatePID);

		duplicatePID = 0;
		oProcessEnum(oBIND(FindDuplicateProcessInstanceByName, oBIND1, oBIND2, oBIND3, ThisID, _Name, &duplicatePID));
	}

	return true;
}

bool EnsureOneInstanceIsRunning()
{
	// Scan for both release and debug builds already running
	char path[_MAX_PATH];
	oVERIFY(oSystemGetPath(path, oSYSPATH_APP_FULL));

	char name[_MAX_PATH];
	oGetFilebase(name, path);
	if (_memicmp(oMODULE_DEBUG_PREFIX, name, strlen(oMODULE_DEBUG_PREFIX)))
		oPrintf(name, oMODULE_DEBUG_PREFIX "%s", oGetFilebase(path));

	oStrcat(name, oGetFileExtension(path));

	if (!TerminateDuplicateInstances(name))
		return false;
	if (!TerminateDuplicateInstances(name+6))
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

		if (oProcessHasDebuggerAttached(oProcessGetCurrentID()))
		{
			system("echo.");
			system("pause");
		}
	}

	if (parameters.SpecialMode)
		oTRACE("Unit test (special mode %s) exiting with result: %s", parameters.SpecialMode, oAsString((oTest::RESULT)result));
	else
		oTRACE("Unit test exiting with result: %s", oAsString((oTest::RESULT)result));

	// Final flush to ensure oBuildTool gets all our stdout
	::_flushall();

	return result;
}