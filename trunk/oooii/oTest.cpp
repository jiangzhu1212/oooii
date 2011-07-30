// $(header)
#include <oooii/oTest.h>
#include <oooii/oAssert.h>
#include <oooii/oBuffer.h>
#include <oooii/oConsole.h>
#include <oooii/oErrno.h>
#include <oooii/oEvent.h>
#include <oooii/oFile.h>
#include <oooii/oFilterChain.h>
#include <oooii/oHash.h>
#include <oooii/oImage.h>
#include <oooii/oLockedPointer.h>
#include <oooii/oMsgBox.h>
#include <oooii/oPath.h>
#include <oooii/oStdio.h>
#include <oooii/oString.h>
#include <oooii/oRef.h>
#include <oooii/oProcess.h>
#include <oooii/oSTL.h>
#include <oooii/oThreading.h>
#include <oooii/oStandards.h>
#include <algorithm>

const char* oAsString(const oTest::RESULT& _Result)
{
	static const char* sStrings[] = 
	{
		"SUCCESS",
		"FAILURE",
		"NOTFOUND",
		"FILTERED",
		"SKIPPED",
		"BUGGED",
		"NOTREADY",
		"LEAKS",
	};
	oSTATICASSERT(oTest::NUM_TEST_RESULTS == oCOUNTOF(sStrings));
	return sStrings[_Result];
}

struct oTestManager_Impl : public oTestManager
{
	void GetDesc(DESC* _pDesc) override;
	void SetDesc(DESC* _pDesc) override;

	oTest::RESULT RunTests(oFilterChain::FILTER* _pTestFilters, size_t _SizeofTestFilters) override;
	oTest::RESULT RunSpecialMode(const char* _Name) override;
	bool FindFullPath(char* _StrFullPath, size_t _SizeofStrFullPath, const char* _StrRelativePath) const override;

	oTest::RESULT RunTest(RegisterTestBase* _pRegisterTestBase, char* _StatusMessage, size_t _SizeofStatusMessage);

	void PrintDesc();
	void RegisterSpecialModeTests();
	void RegisterZombies();
	bool KillZombies(const char* _Name);
	bool KillZombies();

	inline void Report(oConsoleReporting::REPORT_TYPE _Type, const char* _Format, ...) { va_list args; va_start(args, _Format); oConsoleReporting::VReport(_Type, _Format, args); }

	inline void ReportSep() { Report(oConsoleReporting::DEFAULT, "%c ", 179); }

	typedef std::vector<RegisterTestBase*> tests_t;
	tests_t Tests;
	DESC Desc;
	std::string TestSuiteName;
	std::string DataPath;
	std::string GoldenPath;
	std::string OutputPath;
	typedef std::vector<oFilterChain::FILTER> filters_t;
	filters_t Filters;

	typedef std::tr1::unordered_map<std::string, RegisterTestBase*> specialmodes_t;
	specialmodes_t SpecialModes;

	typedef std::vector<std::string> zombies_t;
	zombies_t PotentialZombies;
};

oTestManager* oTestManager::Singleton()
{
	static oTestManager_Impl sTestManager;
	return &sTestManager;
}

oTest::oTest()
{
}

oTest::~oTest()
{
}

const char* oTest::GetName() const
{
	return oGetSimpleTypename(typeid(*this).name());
}

void oTest::BuildPath(char* _StrDestination, size_t _SizeofStrDestination, const char* _TestName, const char* _DataPath, const char* _DataSubpath, const char* _Path, unsigned int _NthImage, const char* _Ext)
{
	char base[_MAX_PATH];
	if (_Path && *_Path)
		strcpy_s(base, _Path);
	else if (_DataSubpath && *_DataSubpath)
		sprintf_s(base, "%s/%s", _DataPath, _DataSubpath);
	else
		sprintf_s(base, "%s", _DataPath);

	if (_NthImage)
		sprintf_s(_StrDestination, _SizeofStrDestination, "%s/%s%u%s", base, _TestName, _NthImage, _Ext);
	else
		sprintf_s(_StrDestination, _SizeofStrDestination, "%s/%s%s", base, _TestName, _Ext);

	oCleanPath(_StrDestination, _SizeofStrDestination, _StrDestination);
}

bool oTest::TestImage(oImage* _pImage, unsigned int _NthImage)
{
	const char* goldenImageExt = ".png";
	const char* diffImageExt = "_diff.png";

	oTestManager::DESC desc;
	oTestManager::Singleton()->GetDesc(&desc);

	char golden[_MAX_PATH];
	BuildPath(golden, GetName(), desc.DataPath, "GoldenImages", desc.GoldenPath, _NthImage, goldenImageExt);
	char output[_MAX_PATH];
	BuildPath(output, GetName(), desc.DataPath, "Output", desc.OutputPath, _NthImage, goldenImageExt);
	char diff[_MAX_PATH];
	BuildPath(diff, GetName(), desc.DataPath, "Output", desc.OutputPath, _NthImage, diffImageExt);

	oRef<oImage> GoldenImage;
	{
		oRef<threadsafe oBuffer> b;
		if (!oBuffer::Create(golden, false, &b))
		{
			if (!_pImage->Save(output))
			{
				oSetLastError(EINVAL, "Output image save failed: %s", output);
				return false;
			}

			oSetLastError(ENOENT, "Golden image load failed: %s", golden);
			return false;
		}

		oLockedPointer<oBuffer> pLockedBuffer(b);
		if (!oImage::Create(pLockedBuffer->GetData(), pLockedBuffer->GetSize(), oSurface::UNKNOWN, &GoldenImage))
		{
			oSetLastError(EINVAL, "Corrupt/unloadable golden image file: %s", golden);
			return false;
		}
	}

	oRef<oImage> diffs;
	unsigned int nDifferences = 0;

	bool compareSucceeded = _pImage->Compare(GoldenImage, desc.ImageFuzziness, &nDifferences, &diffs, desc.DiffImageMultiplier);

	float percentageDifferent = 100.0f;
	if (compareSucceeded)
	{
		oImage::DESC idesc;
		GoldenImage->GetDesc(&idesc);
		percentageDifferent = 100.0f * (nDifferences / static_cast<float>(idesc.Width * idesc.Height));
	}

	if (!compareSucceeded || (100.0f - percentageDifferent) < desc.PixelPercentageMinSuccess)
	{
		if (!_pImage->Save(output))
		{
			oSetLastError(EIO, "Output image save failed: %s", output);
			return false;
		}

		if (diffs && !diffs->Save(diff))
		{
			oSetLastError(EIO, "Diff image save failed: %s", diff);
			return false;
		}

		oSetLastError(EINVAL, "Golden image compare failed (%.1f%% differences): (Golden)%s != (Output)%s", percentageDifferent, golden, output);
		return false;
	}

	return true;
}

bool oSpecialTest::CreateProcess(const char* _SpecialTestName, threadsafe interface oProcess** _ppProcess)
{
	if (!_SpecialTestName || !_ppProcess)
	{
		oSetLastError(EINVAL);
		return false;
	}

	char cmdline[_MAX_PATH + 128];
	oGetExePath(cmdline);
	sprintf_s(cmdline, "%s -s %s", cmdline, _SpecialTestName);
	oProcess::DESC desc;
	desc.CommandLine = cmdline;
	desc.EnvironmentString = 0;
	desc.StdHandleBufferSize = 64 * 1024;
	return oProcess::Create(&desc, _ppProcess);
}

bool oSpecialTest::Start(threadsafe interface oProcess* _pProcess, char* _StrStatus, size_t _SizeofStrStatus, int* _pExitCode, unsigned int _TimeoutMS)
{
	if (!_pProcess || !_StrStatus || !*_StrStatus || !_pExitCode)
	{
		oSetLastError(EINVAL);
		return false;
	}

	oProcess::DESC desc;
	_pProcess->GetDesc(&desc);
	const char* SpecialTestName = oStrStrReverse(desc.CommandLine, "-s ") + 3;
	if (!SpecialTestName || !*SpecialTestName)
	{
		oSetLastError(EINVAL, "Process with an invalid command line for oSpecialTest specified.");
		return false;
	}

	char interprocessName[256];
	sprintf_s(interprocessName, "oTest.%s.Started", SpecialTestName);
	oEvent Started(interprocessName, interprocessName);
	_pProcess->Start();

	oTestManager::DESC testingDesc;
	oTestManager::Singleton()->GetDesc(&testingDesc);

	if (testingDesc.EnableSpecialTestTimeouts && !Started.Wait(_TimeoutMS))
	{
		sprintf_s(_StrStatus, _SizeofStrStatus, "Timed out waiting for %s to start.", SpecialTestName);
		oTRACE("*** SPECIAL MODE UNIT TEST %s timed out waiting for Started event. (Ensure the special mode test sets the started event when appropriate.) ***", SpecialTestName);
		if (!_pProcess->GetExitCode(_pExitCode))
			_pProcess->Kill(ETIMEDOUT);

		sprintf_s(_StrStatus, _SizeofStrStatus, "Special Mode %s timed out on start.", SpecialTestName);
		return false;
	}

	// If we timeout on ending, that's good, it means the app is still running
	if ((_pProcess->Wait(200) && _pProcess->GetExitCode(_pExitCode)))
	{
		char msg[512];
		size_t bytes = _pProcess->ReadFromStdout(msg, oCOUNTOF(msg));
		msg[bytes] = 0;
		if (bytes)
			sprintf_s(_StrStatus, _SizeofStrStatus, "%s: %s", SpecialTestName, msg);
		return false;
	}

	return true;
}

void oSpecialTest::NotifyReady()
{
	char interprocessName[128];
	const char* testName = oGetSimpleTypename(typeid(*this).name());
	sprintf_s(interprocessName, "oTest.%s.Started", testName);
	oEvent Ready(interprocessName, interprocessName);
	Ready.Set();
}

oTestManager::RegisterTestBase::RegisterTestBase(unsigned int _BugNumber, oTest::RESULT _BugResult, const char* _PotentialZombieProcesses)
	: BugNumber(_BugNumber)
	, BugResult(_BugResult)
{
	strcpy_s(PotentialZombieProcesses, oSAFESTR(_PotentialZombieProcesses));
	static_cast<oTestManager_Impl*>(oTestManager::Singleton())->Tests.push_back(this);
}

oTestManager::RegisterTestBase::~RegisterTestBase()
{
	oTestManager_Impl::tests_t& tests = static_cast<oTestManager_Impl*>(oTestManager::Singleton())->Tests;
	oTestManager_Impl::tests_t::iterator it = std::find(tests.begin(), tests.end(), this);
	if (it != tests.end())
		tests.erase(it);
}

void oTestManager_Impl::GetDesc(DESC* _pDesc)
{
	*_pDesc = Desc;
}

void oTestManager_Impl::SetDesc(DESC* _pDesc)
{
	TestSuiteName = _pDesc->TestSuiteName && *_pDesc->TestSuiteName ? _pDesc->TestSuiteName : "OOOii Unit Test Suite";
	DataPath = oSAFESTR(_pDesc->DataPath);
	GoldenPath = oSAFESTR(_pDesc->GoldenPath);
	OutputPath = oSAFESTR(_pDesc->OutputPath);
	Desc = *_pDesc;
	Desc.TestSuiteName = TestSuiteName.c_str();
	Desc.DataPath = DataPath.c_str();
	Desc.GoldenPath = GoldenPath.c_str();
	Desc.OutputPath = OutputPath.c_str();
}

void oTestManager_Impl::PrintDesc()
{
	char cwd[_MAX_PATH];
	oGetSysPath(cwd, oSYSPATH_CWD);
	char datapath[_MAX_PATH];
	oCleanPath(datapath, oSAFESTR(Desc.DataPath));
	oEnsureFileSeparator(datapath);
	bool dataPathIsCWD = !_stricmp(cwd, datapath);

	Report(oConsoleReporting::INFO, "CWD Path: %s\n", cwd);
	Report(oConsoleReporting::INFO, "Data Path: %s%s\n", (Desc.DataPath && *Desc.DataPath) ? Desc.DataPath : cwd, dataPathIsCWD ? " (CWD)" : "");
	Report(oConsoleReporting::INFO, "Golden Path: %s\n", *Desc.GoldenPath ? Desc.GoldenPath : "(null)");
	Report(oConsoleReporting::INFO, "Output Path: %s\n", *Desc.OutputPath ? Desc.OutputPath : "(null)");
	Report(oConsoleReporting::INFO, "Random Seed: %u\n", Desc.RandomSeed);
	Report(oConsoleReporting::INFO, "Special Test Timeouts: %sabled\n", Desc.EnableSpecialTestTimeouts ? "en" : "dis");
}

void oTestManager_Impl::RegisterSpecialModeTests()
{
	for (tests_t::iterator it = Tests.begin(); it != Tests.end(); ++it)
	{
		RegisterTestBase* pRTB = *it;

		if (!pRTB->IsSpecialTest())
			continue;

		const char* Name = oGetSimpleTypename(pRTB->GetTypename());
		oASSERT(SpecialModes[Name] == 0, "%s already registered", Name);
		SpecialModes[Name] = *it;
	}
}

void oTestManager_Impl::RegisterZombies()
{
	for (tests_t::iterator it = Tests.begin(); it != Tests.end(); ++it)
	{
		RegisterTestBase* pRTB = *it;

		const char* TestPotentialZombies = pRTB->GetPotentialZombieProcesses();
		if (TestPotentialZombies && *TestPotentialZombies)
		{
			char* ctx = 0;
			char* z = oStrTok(TestPotentialZombies, ";", &ctx);
			while (z)
			{
				oPushBackUnique(PotentialZombies, std::string(z));
				z = oStrTok(nullptr, ";", &ctx);
			}
		}
	}
}

static bool FindDuplicateProcessInstanceByName(unsigned int _ProcessID, unsigned int _ParentProcessID, const char* _ProcessExePath, unsigned int _IgnorePID, const char* _FindName, unsigned int* _pOutPIDs, size_t _NumOutPIDs, size_t* _pCurrentCount)
{
	if (_IgnorePID != _ProcessID && !_stricmp(_FindName, _ProcessExePath))
	{
		if (*_pCurrentCount >= _NumOutPIDs)
			return false;

		_pOutPIDs[*_pCurrentCount] = _ProcessID;
		(*_pCurrentCount)++;
	}

	return true;
}

bool oTestManager_Impl::KillZombies(const char* _Name)
{
	unsigned int ThisID = oProcessGetCurrentID();

	unsigned int pids[1024];
	size_t npids = 0;

	oFUNCTION<bool(unsigned int _ProcessID, unsigned int _ParentProcessID, const char* _ProcessExePath)> FindDups = oBIND(FindDuplicateProcessInstanceByName, oBIND1, oBIND2, oBIND3, ThisID, _Name, pids, oCOUNTOF(pids), &npids);
	oProcessEnum(FindDups);

	unsigned int retries = 3;
	for (size_t i = 0; i < npids; i++)
	{
		if (oProcessHasDebuggerAttached(pids[i]))
			continue;

		oProcessTerminate(pids[i], ECANCELED);
		if (!oProcessWaitExit(pids[i], 5000))
		{
			oMsgBox::tprintf(oMsgBox::WARN, 20000, "OOOii Test Manager", "Cannot terminate stale process %u, please end this process before continuing.", pids[i]);
			if (--retries == 0)
				return false;

			i--;
			continue;
		}

		retries = 3;
	}

	return true;
}

bool oTestManager_Impl::KillZombies()
{
	for (zombies_t::const_iterator it = PotentialZombies.begin(); it != PotentialZombies.end(); ++it)
		if (!KillZombies(it->c_str()))
			return false;
	return true;
}

oTest::RESULT oTestManager_Impl::RunTest(RegisterTestBase* _pRegisterTestBase, char* _StatusMessage, size_t _SizeofStatusMessage)
{
	if (!KillZombies())
	{
		sprintf_s(_StatusMessage, _SizeofStatusMessage, "oTest infrastructure could not kill zombie process");
		return oTest::FAILURE;
	}

	*_StatusMessage = 0;

	srand(Desc.RandomSeed);

	#ifdef _DEBUG
		_CrtMemState before, after, difference;
		_CrtMemCheckpoint(&before);
	#endif
	oTest* pTest = _pRegisterTestBase->New();
	oTest::RESULT result = pTest->Run(_StatusMessage, _SizeofStatusMessage);
	delete pTest;

	#ifdef _DEBUG
		// @oooii-kevin: Due to TBB's static memory initialization and our endemic use of TBB
		// we need to recycle the scheduler after every test to avoid any spurious memory leaks
		oRecycleScheduler();
		_CrtMemCheckpoint(&after);
		if (result != oTest::FAILURE && _CrtMemDifference(&difference, &before, &after))
		{
			// @oooii-tony: How should I handle things that resize internal 
			// buffers on first use, so it changes the memory footprint, but 
			// is not a leak?
			result = oTest::LEAKS;
			sprintf_s(_StatusMessage, _SizeofStatusMessage, "%u bytes in %u normal blocks, %u bytes in %u CRT blocks", difference.lSizes[_NORMAL_BLOCK], difference.lCounts[_NORMAL_BLOCK], difference.lSizes[_CRT_BLOCK], difference.lCounts[_CRT_BLOCK]);
			_CrtMemDumpStatistics(&difference);
			_CrtMemDumpAllObjectsSince(&before);
		}
	#endif

	return result;
}

oTest::RESULT oTestManager_Impl::RunTests(oFilterChain::FILTER* _pTestFilters, size_t _SizeofTestFilters)
{
	RegisterZombies();

	RegisterSpecialModeTests();

	size_t TotalNumSucceeded = 0;
	size_t TotalNumFailed = 0;
	size_t TotalNumLeaks = 0;
	size_t TotalNumSkipped = 0;

	char timeMessage[512];
	double allIterationsStartTime = oTimer();
	for (size_t r = 0; r < Desc.NumRunIterations; r++)
	{
		size_t Count[oTest::NUM_TEST_RESULTS];
		memset(Count, 0, sizeof(size_t) * oTest::NUM_TEST_RESULTS);

		oRef<threadsafe oFilterChain> filterChain;
		if (_pTestFilters && _SizeofTestFilters && !oFilterChain::Create(_pTestFilters, _SizeofTestFilters, &filterChain))
			return oTest::FAILURE;

		// Prepare formatting used to print results
		char nameSpec[32];
		sprintf_s(nameSpec, "%%-%us", Desc.NameColumnWidth);

		char statusSpec[32];
		sprintf_s(statusSpec, "%%-%us", Desc.StatusColumnWidth);

		char timeSpec[32];
		sprintf_s(timeSpec, "%%-%us", Desc.TimeColumnWidth);

		char messageSpec[32];
		sprintf_s(messageSpec, "%%s\n");

		char statusMessage[512];

		Report(oConsoleReporting::DEFAULT, "========== %s Run %u ==========\n", Desc.TestSuiteName, r+1);
		PrintDesc();

		// Print table headers
		{
			Report(oConsoleReporting::HEADING, nameSpec, "Test Name");
			ReportSep();
			Report(oConsoleReporting::HEADING, statusSpec, "Status");
			ReportSep();
			Report(oConsoleReporting::HEADING, timeSpec, "Time");
			ReportSep();
			Report(oConsoleReporting::HEADING, messageSpec, "Status Message");
		}

		double totalTestStartTime = oTimer();
		for (tests_t::iterator it = Tests.begin(); it != Tests.end(); ++it)
		{
			RegisterTestBase* pRTB = *it;

			if (pRTB && !pRTB->IsSpecialTest())
			{
				char TestName[256];
				strcpy_s(TestName, oGetSimpleTypename(pRTB->GetTypename()));

				double testTime = 0.0;

				Report(oConsoleReporting::DEFAULT, nameSpec, TestName);
				ReportSep();

				oTest::RESULT result = oTest::FILTERED;
				oConsoleReporting::REPORT_TYPE ReportType = oConsoleReporting::DEFAULT;

				if (!filterChain || filterChain->Passes(TestName, 0)) // put in skip filter here
				{
					if (pRTB->GetBugNumber() == 0)
					{
						oTRACE("========== Begin %s Run %u ==========", TestName, r+1);
						testTime = oTimer();
						result = RunTest(pRTB, statusMessage, oCOUNTOF(statusMessage));
						testTime = oTimer() - testTime;
						oTRACE("========== End %s Run %u ==========", TestName, r+1);
						Count[result]++;
					}

					else
						result = pRTB->GetBugResult();

					switch (result)
					{
						case oTest::SUCCESS:
							if (!*statusMessage)
								sprintf_s(statusMessage, "---");
							ReportType = oConsoleReporting::SUCCESS;
							break;

						case oTest::FAILURE:
							if (!*statusMessage)
								sprintf_s(statusMessage, "Failed with no test-specific status message");
							ReportType = oConsoleReporting::CRIT;
							break;

						case oTest::SKIPPED:
							if (!*statusMessage)
								sprintf_s(statusMessage, "Skipped");
							ReportType = oConsoleReporting::WARN;
							break;

						case oTest::BUGGED:
							sprintf_s(statusMessage, "Test disabled. See oBug_%u", pRTB->GetBugNumber());
							ReportType = oConsoleReporting::ERR;
							break;

						case oTest::NOTREADY:
							sprintf_s(statusMessage, "Test not yet ready. See oBug_%u", pRTB->GetBugNumber());
							ReportType = oConsoleReporting::INFO;
							break;

						case oTest::LEAKS:
							if (!*statusMessage)
								sprintf_s(statusMessage, "Leaks memory");
							ReportType = oConsoleReporting::WARN;
							break;
					}
				}

				else
				{
					ReportType = oConsoleReporting::WARN;
					Count[oTest::SKIPPED]++;
					sprintf_s(statusMessage, "---");
				}

				Report(ReportType, statusSpec, oAsString(result));
				ReportSep();

				ReportType = oConsoleReporting::DEFAULT;
				if (testTime > Desc.TestReallyTooSlowTimeInSeconds)
					ReportType = oConsoleReporting::ERR;
				else if (testTime > Desc.TestTooSlowTimeInSeconds)
					ReportType = oConsoleReporting::WARN;

				oFormatTimeSize(timeMessage, testTime);
				Report(ReportType, timeSpec, timeMessage);
				ReportSep();
				Report(oConsoleReporting::DEFAULT, messageSpec, statusMessage);
			}
		}

		size_t NumSucceeded = Count[oTest::SUCCESS];
		size_t NumFailed = Count[oTest::FAILURE];
		size_t NumLeaks = Count[oTest::LEAKS]; 
		size_t NumSkipped = Count[oTest::SKIPPED] + Count[oTest::FILTERED] + Count[oTest::BUGGED] + Count[oTest::NOTREADY];

		oFormatTimeSize(timeMessage, oTimer() - totalTestStartTime);
    if ((NumSucceeded + NumFailed == 0))
  		Report(oConsoleReporting::ERR, "========== Unit Tests: ERROR NO TESTS RUN ==========\n");
    else
		  Report(oConsoleReporting::INFO, "========== Unit Tests: %u succeeded, %u failed, %u skipped in %s ==========\n", NumSucceeded, NumFailed + NumLeaks, NumSkipped, timeMessage);

		TotalNumSucceeded += NumSucceeded;
		TotalNumFailed += NumFailed;
		TotalNumLeaks += NumLeaks;
		TotalNumSkipped += NumSkipped;
	}

	if (Desc.NumRunIterations != 1) // != so we report if somehow a 0 got through to here
	{
		oFormatTimeSize(timeMessage, oTimer() - allIterationsStartTime);
		Report(oConsoleReporting::INFO, "========== %u Iterations: %u succeeded, %u failed, %u skipped in %s ==========\n", Desc.NumRunIterations, TotalNumSucceeded, TotalNumFailed + TotalNumLeaks, TotalNumSkipped, timeMessage);
	}

	if ((TotalNumSucceeded + TotalNumFailed + TotalNumLeaks) == 0)
		return oTest::NOTFOUND;

  if (TotalNumFailed > 0)
    return oTest::FAILURE;

	if (TotalNumLeaks > 0)
		return oTest::LEAKS;

  return oTest::SUCCESS;
}

oTest::RESULT oTestManager_Impl::RunSpecialMode(const char* _Name)
{
	RegisterSpecialModeTests();

	oTest::RESULT result = oTest::NOTFOUND;

	RegisterTestBase* pRTB = SpecialModes[_Name];
	if (pRTB)
	{
		char statusMessage[512];
		result = RunTest(pRTB, statusMessage, oCOUNTOF(statusMessage));
		switch (result)
		{
		case oTest::SUCCESS:
			printf("SpecialMode %s: Success", _Name);
			break;

		case oTest::SKIPPED:
			printf("SpecialMode %s: Skipped", _Name);
			break;

		case oTest::FILTERED:
			printf("SpecialMode %s: Filtered", _Name);
			break;

		case oTest::LEAKS:
			printf("SpecialMode %s: Leaks", _Name);
			break;

		default:
			printf("SpecialMode %s: %s", _Name, *statusMessage ? statusMessage : "(no status message)");
			break;
		}
	}

	else 
		printf("Special Mode %s not found\n", oSAFESTRN(_Name));

	return result;
}

bool oTestManager_Impl::FindFullPath(char* _StrFullPath, size_t _SizeofStrFullPath, const char* _StrRelativePath) const
{
	char RawPath[_MAX_PATH];
	if (oFindPath(RawPath, _StrRelativePath, NULL, DataPath.c_str(), oFile::Exists))
	{
		return 0 == oCleanPath(_StrFullPath, _SizeofStrFullPath, RawPath);
	}

	return false;
}
