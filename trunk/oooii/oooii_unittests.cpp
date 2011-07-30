// $(header)
#include <oooii/oAssert.h>
#include <oooii/oConsole.h>
#include <oooii/oDebugger.h>
#include <oooii/oFile.h>
#include <oooii/oImage.h>
#include <oooii/oStdio.h>
#include <oooii/oString.h>
#include <oooii/oMsgBox.h>
#include <oooii/oPath.h>
#include <oooii/oTest.h>
#include <oooii/oThreading.h>
#include <oooii/oRef.h>

static const char* sTITLE = "OOOii Unit Test Suite";

static const oOption sCmdLineOptions[] = 
{
	{ "include", 'i', "regex", "regex matching a test name to include" },
	{ "exclude", 'e', "regex", "regex matching a test name to exclude" },
	{ "path", 'p', "path", "Path where all test data is loaded from. The current working directory is used by default." },
	{ "special-mode", 's', "mode", "Run the test harness in a special mode (used mostly by multi-process/client-server unit tests)" },
	{ "random-seed", 'r', "seed", "Set the random seed to be used for this run. This is reset at the start of each test." },
	{ "golden-images", 'g', "path", "Path where all known-good \"golden\" images are stored. The current working directory is used by default." },
	{ "output", 'o', "path", "Path where all logging and error images are created." },
	{ "repeat-number", 'n', "Repeat the test a certain number of times." },
	{ "disable-timeouts", 'd', "Disable timeouts, mainly while debugging." },
	{ 0,0,0,0 }
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

	oRecycleScheduler();

	oTRACEA("Aero is %sactive", oSysGUIUsesGPUCompositing() ? "" : "in");

	// IOCP needs to be initialized or it will show up as a leak in the first test
	// to use it.
	extern void InitializeIOCP();
	InitializeIOCP();

	oConsole::SetTitle(sTITLE);
	oDebugger::ReportLeaksOnExit(true);

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
	const char* GoldenPath;
	const char* OutputPath;
	unsigned int RandomSeed;
	unsigned int RepeatNumber;
	bool EnableTimeouts;
};

void ParseCommandLine(int _Argc, const char* _Argv[], PARAMETERS* _pParameters)
{
	_pParameters->Filters.clear();
	_pParameters->SpecialMode = NULL;
	_pParameters->DataPath = NULL;
	_pParameters->GoldenPath = NULL;
	_pParameters->OutputPath = NULL;
	_pParameters->RandomSeed = 0;
	_pParameters->RepeatNumber = 1;
	_pParameters->EnableTimeouts = true;

	const char* value = 0;
	char ch = oOptTok(&value, _Argc, _Argv, sCmdLineOptions);
	while (ch)
	{
		switch (ch)
		{
			case 'i':
			{
				_pParameters->Filters.resize(_pParameters->Filters.size() + 1);
				_pParameters->Filters.back().FilterType = oFilterChain::INCLUDE1;
				_pParameters->Filters.back().RegularExpression = value;
				break;
			}

			case 'e':
			{
				_pParameters->Filters.resize(_pParameters->Filters.size() + 1);
				_pParameters->Filters.back().FilterType = oFilterChain::EXCLUDE1;
				_pParameters->Filters.back().RegularExpression = value;
				break;
			}

			case 'p':
				_pParameters->DataPath = value;
				break;

			case 's':
				_pParameters->SpecialMode = value;
				break;

			case 'r':
				_pParameters->RandomSeed = atoi(value) ? atoi(value) : 1; // ensure it's not 0, meaning choose randomly
				break;

			case 'g':
				_pParameters->GoldenPath = value;
				break;

			case 'o':
				_pParameters->OutputPath = value;
				break;

			case 'n':
				_pParameters->RepeatNumber = atoi(value);
				break;

			case 'd':
				_pParameters->EnableTimeouts = false;

			default:
				break;
		}

		ch = oOptTok(&value, 0, 0, 0);
	}
}

bool GetDataPath(char* _DataPath, size_t _SizeofDataPath, const char* _UserDataPath)
{
	if( _UserDataPath && oFindPath(_DataPath, _SizeofDataPath, _UserDataPath, NULL, NULL, oFile::Exists ) )
		return true;

	// If here, give up to CWD.
	return oGetSysPath(_DataPath, _SizeofDataPath, oSYSPATH_CWD);
}

template<size_t size> inline bool GetDataPath(char (&_DataPath)[size], const char* _UserDataPath) { return GetDataPath(_DataPath, size, _UserDataPath); }

struct oNamedFileDesc : oFile::DESC
{
	char FileName[_MAX_PATH];
	static bool NewerToOlder(const oNamedFileDesc& _File1, const oNamedFileDesc& _File2)
	{
		return _File1.Written > _File2.Written;
	}
};

static bool PushBackNFDs(const char* _Path, const oFile::DESC& _Desc, std::vector<oNamedFileDesc>& _NFDs)
{
	oNamedFileDesc nfd;
	reinterpret_cast<oFile::DESC&>(nfd) = _Desc;
	strcpy_s(nfd.FileName, _Path);
	_NFDs.push_back(nfd);
	return true;
}

void DeleteOldLogFiles(const char* _SpecialModeName)
{
	const size_t kLogHistory = 10;

	char logFileWildcard[_MAX_PATH];
	oGetLogFilePath(logFileWildcard, _SpecialModeName);

	char* p = oStrStrReverse(logFileWildcard, "_");
	strcpy_s(p, oCOUNTOF(logFileWildcard) - std::distance(logFileWildcard, p), "*.txt");

	std::vector<oNamedFileDesc> logs;
	logs.reserve(20);
	oFile::EnumFiles(logFileWildcard, oBIND(PushBackNFDs, oBIND1, oBIND2, oBINDREF(logs)));

	if (logs.size() > kLogHistory)
	{
		std::sort(logs.begin(), logs.end(), oNamedFileDesc::NewerToOlder);
		for (size_t i = kLogHistory; i < logs.size(); i++)
			oFile::Delete(logs[i].FileName);
	}
}

void EnableLogFile(const char* _SpecialModeName)
{
	#ifdef _DEBUG
		char logFilePath[_MAX_PATH];
		oGetLogFilePath(logFilePath, _SpecialModeName);
		oAssert::DESC desc;
		oAssert::GetDesc(&desc);
		desc.LogFilePath = logFilePath;
		oAssert::SetDesc(&desc);
	#endif
}

void SetTestManagerDesc(const PARAMETERS* _pParameters)
{
	char dataPath[_MAX_PATH];
	GetDataPath(dataPath, _pParameters->DataPath);

	oTestManager::DESC desc;
	desc.TestSuiteName = sTITLE;
	desc.DataPath = dataPath;
	desc.GoldenPath = _pParameters->GoldenPath;
	desc.OutputPath = _pParameters->OutputPath;
	desc.NameColumnWidth = 30;
	desc.TimeColumnWidth = 13;
	desc.StatusColumnWidth = 9;
	desc.RandomSeed = _pParameters->RandomSeed ? _pParameters->RandomSeed : oTimerMS();
	desc.NumRunIterations = _pParameters->RepeatNumber ? _pParameters->RepeatNumber : 1;
	desc.ImageFuzziness = 10; // @oooii-tony: FIXME: compression seems to be non-repeatable, so leave a wide margin for error
	desc.PixelPercentageMinSuccess = 98;
	desc.EnableSpecialTestTimeouts = _pParameters->EnableTimeouts;

	oTestManager::Singleton()->SetDesc(&desc);
}

static bool FindDuplicateProcessInstanceByName(unsigned int _ProcessID, unsigned int _ParentProcessID, const char* _ProcessExePath, unsigned int _IgnorePID, const char* _FindName, unsigned int* _pOutPID)
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
	unsigned int ThisID = oProcessGetCurrentID();
	unsigned int duplicatePID = 0;
	oProcessEnum(oBIND(FindDuplicateProcessInstanceByName, oBIND1, oBIND2, oBIND3, ThisID, _Name, &duplicatePID));
	while (duplicatePID)
	{
		oMsgBox::RESULT result = oMsgBox::tprintf(oMsgBox::YESNO, 20000, sTITLE, "An instance of the unittest executable was found at process %u. Do you want to kill it now? (no means this application will exit)", duplicatePID);
		if (result == oMsgBox::NO)
			return false;

		oProcessTerminate(duplicatePID, ECANCELED);
		if (!oProcessWaitExit(duplicatePID, 5000))
			oMsgBox::tprintf(oMsgBox::WARN, 20000, sTITLE, "Cannot terminate stale process %u, please end this process before continuing.", duplicatePID);

		duplicatePID = 0;
		oProcessEnum(oBIND(FindDuplicateProcessInstanceByName, oBIND1, oBIND2, oBIND3, ThisID, _Name, &duplicatePID));
	}

	return true;
}

bool EnsureOneInstanceIsRunning()
{
	// Scan for both release and debug builds already running
	char path[_MAX_PATH];
	oVERIFY(oGetExePath(path));

	char name[_MAX_PATH];
	oGetFilebase(name, path);
	if (_memicmp("DEBUG-", name, 6))
		sprintf_s(name, "DEBUG-%s", oGetFilebase(path));

	strcat_s(name, oGetFileExtension(path));

	if (!TerminateDuplicateInstances(name))
		return false;
	if (!TerminateDuplicateInstances(name+6))
		return false;

	return true;
}

int main(int argc, const char* argv[])
{
	InitEnv();

	PARAMETERS parameters;
	ParseCommandLine(argc, argv, &parameters);
	SetTestManagerDesc(&parameters);
	DeleteOldLogFiles(parameters.SpecialMode);
	EnableLogFile(parameters.SpecialMode);

	int result = 0;

	if (parameters.SpecialMode)
		result = oTestManager::Singleton()->RunSpecialMode(parameters.SpecialMode);
	else
	{
		if (EnsureOneInstanceIsRunning())
			result = oTestManager::Singleton()->RunTests(parameters.Filters.empty() ? 0 : &parameters.Filters[0], parameters.Filters.size());
		else
			result = oTest::SKIPPED;

		oMsgBox::tprintf(result ? oMsgBox::NOTIFY_ERR : oMsgBox::NOTIFY, 10000, sTITLE, "Completed%s", result ? " with errors" : " successfully");
		if (oProcessHasDebuggerAttached())
		{
			system("echo.");
			system("pause");
		}
	}

	if (parameters.SpecialMode)
		oTRACE("Unit test (special mode %s) exiting with result: %s", parameters.SpecialMode, oAsString((oTest::RESULT)result));
	else
		oTRACE("Unit test exiting with result: %s", oAsString((oTest::RESULT)result));

	return result;
}
