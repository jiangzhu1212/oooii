/**************************************************************************
 * The MIT License                                                        *
 * Copyright (c) 2011 Antony Arciuolo & Kevin Myers                       *
 *                                                                        *
 * Permission is hereby granted, free of charge, to any person obtaining  *
 * a copy of this software and associated documentation files (the        *
 * "Software"), to deal in the Software without restriction, including    *
 * without limitation the rights to use, copy, modify, merge, publish,    *
 * distribute, sublicense, and/or sell copies of the Software, and to     *
 * permit persons to whom the Software is furnished to do so, subject to  *
 * the following conditions:                                              *
 *                                                                        *
 * The above copyright notice and this permission notice shall be         *
 * included in all copies or substantial portions of the Software.        *
 *                                                                        *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,        *
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF     *
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND                  *
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE *
 * LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION *
 * OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION  *
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.        *
 **************************************************************************/
#include <oBasis/oSize.h>
#include <oBasis/oString.h>
#include <oPlatform/oReporting.h>
#include <oPlatform/oConsole.h>
#include <oPlatform/oDebugger.h>
#include <oPlatform/oFile.h>
#include <oPlatform/oImage.h>
#include <oPlatform/oMsgBox.h>
#include <oPlatform/oTest.h>
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
	{ "golden-images", 'g', "path", "Path where all known-good \"golden\" images are stored. The current working directory is used by default." },
	{ "output", 'o', "path", "Path where all logging and error images are created." },
	{ "repeat-number", 'n', "nRuntimes", "Repeat the test a certain number of times." },
	{ "disable-timeouts", 'd', nullptr, "Disable timeouts, mainly while debugging." },
	{ "capture-callstack", 'c', nullptr, "Capture full callstack to allocations for per-test leaks (slow!)" },
	{ "disable-leaktracking", '_', nullptr, "Disable the leak tracking when it is suspected of causing performance issues" },
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

	// IOCP needs to be initialized or it will show up as a leak in the first test
	// to use it.
	extern void InitializeIOCP();
	InitializeIOCP();

	oConsole::SetTitle(sTITLE);

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
	bool CaptureCallstackForTestLeaks;
	bool EnableLeakTracking;
};

void ParseCommandLine(int _Argc, const char* _Argv[], PARAMETERS* _pParameters)
{
	_pParameters->Filters.clear();
	_pParameters->SpecialMode = nullptr;
	_pParameters->DataPath = nullptr;
	_pParameters->GoldenPath = nullptr;
	_pParameters->OutputPath = nullptr;
	_pParameters->RandomSeed = 0;
	_pParameters->RepeatNumber = 1;
	_pParameters->EnableTimeouts = true;
	_pParameters->CaptureCallstackForTestLeaks = false;
	_pParameters->EnableLeakTracking = true;

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
				break;

			case 'c':
				_pParameters->CaptureCallstackForTestLeaks = true;
				break;

			case '_':
				_pParameters->EnableLeakTracking = false;
				break;

			default:
				break;
		}

		ch = oOptTok(&value, 0, 0, 0);
	}
}

struct oNamedFileDesc : oFILE_DESC
{
	char FileName[_MAX_PATH];
	static bool NewerToOlder(const oNamedFileDesc& _File1, const oNamedFileDesc& _File2)
	{
		return _File1.Written > _File2.Written;
	}
};

static bool PushBackNFDs(const char* _Path, const oFILE_DESC& _Desc, std::vector<oNamedFileDesc>& _NFDs)
{
	oNamedFileDesc nfd;
	reinterpret_cast<oFILE_DESC&>(nfd) = _Desc;
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
	oFileEnum(logFileWildcard, oBIND(PushBackNFDs, oBIND1, oBIND2, oBINDREF(logs)));

	if (logs.size() > kLogHistory)
	{
		std::sort(logs.begin(), logs.end(), oNamedFileDesc::NewerToOlder);
		for (size_t i = kLogHistory; i < logs.size(); i++)
			oFileDelete(logs[i].FileName);
	}
}

void EnableLogFile(const char* _SpecialModeName)
{
	#ifdef _DEBUG
		char logFilePath[_MAX_PATH];
		oGetLogFilePath(logFilePath, _SpecialModeName);
		oREPORTING_DESC desc;
		oReportingGetDesc(&desc);
		desc.LogFilePath = logFilePath;
		oReportingSetDesc(desc);
	#endif
}

void SetTestManagerDesc(const PARAMETERS* _pParameters)
{
	char dataPath[_MAX_PATH];

	if (_pParameters->DataPath)
		strcpy_s(dataPath, _pParameters->DataPath);
	else if (!oSystemGetPath(dataPath, oSYSPATH_DATA))
	{
		oASSERT(false, "Failed to find data dir");
	}

	oTestManager::DESC desc;
	desc.TestSuiteName = sTITLE;
	desc.DataPath = dataPath;
	desc.GoldenPath = _pParameters->GoldenPath;
	desc.OutputPath = _pParameters->OutputPath;
	desc.NameColumnWidth = 32;
	desc.TimeColumnWidth = 13;
	desc.StatusColumnWidth = 9;
	desc.RandomSeed = _pParameters->RandomSeed ? _pParameters->RandomSeed : (unsigned int)oStd::chrono::high_resolution_clock::now().time_since_epoch().count();
	desc.NumRunIterations = _pParameters->RepeatNumber ? _pParameters->RepeatNumber : 1;
	desc.EnableSpecialTestTimeouts = _pParameters->EnableTimeouts;
	desc.CaptureCallstackForTestLeaks = _pParameters->CaptureCallstackForTestLeaks;
	desc.EnableLeakTracking = _pParameters->EnableLeakTracking;

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
	if (_memicmp("DEBUG-", name, 6))
		sprintf_s(name, "DEBUG-%s", oGetFilebase(path));

	strcat_s(name, oGetFileExtension(path));

	if (!TerminateDuplicateInstances(name))
		return false;
	if (!TerminateDuplicateInstances(name+6))
		return false;

	return true;
}

//#define DEBUG_oCAMERA

#include "../oPlatform/oCRTLeakTracker.h"

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

		oMSGBOX_DESC mb;
		mb.Type = result ? oMSGBOX_NOTIFY_ERR : oMSGBOX_NOTIFY;
		mb.TimeoutMS = 10000;
		mb.Title = sTITLE;
		oMsgBox(mb, "Completed%s", result ? " with errors" : " successfully");
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

	return result;
}
