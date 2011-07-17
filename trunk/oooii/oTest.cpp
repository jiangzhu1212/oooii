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
#include <oooii/oTest.h>
#include <oooii/oAssert.h>
#include <oooii/oBuffer.h>
#include <oooii/oConsole.h>
#include <oooii/oErrno.h>
#include <oooii/oFile.h>
#include <oooii/oFilterChain.h>
#include <oooii/oHash.h>
#include <oooii/oImage.h>
#include <oooii/oLockedPointer.h>
#include <oooii/oPath.h>
#include <oooii/oStdio.h>
#include <oooii/oString.h>
#include <oooii/oRef.h>
#include <oooii/oProcess.h>
#include <oooii/oThreading.h>
#include <algorithm>
#include <string>
#include <vector>
#include <hash_map>

struct oTestManager_Impl : public oTestManager
{
	void GetDesc(DESC* _pDesc) override;
	void SetDesc(DESC* _pDesc) override;

	int RunTests(oFilterChain::FILTER* _pTestFilters, size_t _SizeofTestFilters) override;
	int RunSpecialMode(const char* _Name) override;
	bool FindFullPath(char* _StrFullPath, size_t _SizeofStrFullPath, const char* _StrRelativePath) const override;

	oTest::RESULT RunTest(RegisterTestBase* _pRegisterTestBase, char* _StatusMessage, size_t _SizeofStatusMessage);

	void PrintDesc();
	void RegisterSpecialModeTests();

	typedef std::vector<RegisterTestBase*> tests_t;
	tests_t Tests;
	DESC Desc;
	std::string TestSuiteName;
	std::string DataPath;
	std::string GoldenPath;
	std::string OutputPath;
	typedef std::vector<oFilterChain::FILTER> filters_t;
	filters_t Filters;

	typedef stdext::hash_map<std::string, RegisterTestBase*> specialmodes_t;
	specialmodes_t SpecialModes;
};

oTestManager* oTestManager::Singleton()
{
	static oTestManager_Impl sTestManager;
	return &sTestManager;
}

const char* oAsString(oTest::RESULT _Result)
{
	switch (_Result)
	{
	case oTest::SUCCESS: return "SUCCESS";
	case oTest::FAILURE: return "FAILURE";
	case oTest::FILTERED: return "FILTERED";
	case oTest::SKIPPED: return "SKIPPED";
	case oTest::LEAKS: return "LEAKS";
	default: oASSUME(0);
	}
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

oTestManager::RegisterTestBase::RegisterTestBase()
{
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

	oConsole::printf(0, 0, "CWD Path: %s\n", cwd);
	oConsole::printf(0, 0, "Data Path: %s%s\n", (Desc.DataPath && *Desc.DataPath) ? Desc.DataPath : cwd, dataPathIsCWD ? " (CWD)" : "");
	oConsole::printf(0, 0, "Golden Path: %s\n", *Desc.GoldenPath ? Desc.GoldenPath : "(null)");
	oConsole::printf(0, 0, "Output Path: %s\n", *Desc.OutputPath ? Desc.OutputPath : "(null)");
	oConsole::printf(0, 0, "Random Seed: %u\n", Desc.RandomSeed);
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

oTest::RESULT oTestManager_Impl::RunTest(RegisterTestBase* _pRegisterTestBase, char* _StatusMessage, size_t _SizeofStatusMessage)
{
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

int oTestManager_Impl::RunTests(oFilterChain::FILTER* _pTestFilters, size_t _SizeofTestFilters)
{
	RegisterSpecialModeTests();

	size_t TotalNumSucceeded = 0;
	size_t TotalNumFailed = 0;
	size_t TotalNumSkipped = 0;

	char timeMessage[512];
	double allIterationsStartTime = oTimer();
	for (size_t r = 0; r < Desc.NumRunIterations; r++)
	{
		size_t NumSucceeded = 0;
		size_t NumFailed = 0;
		size_t NumSkipped = 0;

		oRef<threadsafe oFilterChain> filterChain;
		if (_pTestFilters && _SizeofTestFilters && !oFilterChain::Create(_pTestFilters, _SizeofTestFilters, &filterChain))
			return -1;

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

		oConsole::printf(0, 0, "========== %s Run %u ==========\n", Desc.TestSuiteName, r+1);
		PrintDesc();

		// Print table headers
		{
			oConsole::printf(0, 0, nameSpec, "TEST NAME");
			oConsole::printf(0, 0, ": ");
			oConsole::printf(0, 0, statusSpec, "STATUS");
			oConsole::printf(0, 0, ": ");
			oConsole::printf(0, 0, timeSpec, "TIME");
			oConsole::printf(0, 0, ": ");
			oConsole::printf(0, 0, messageSpec, "STATUS MESSAGE");
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

				oColor fg = 0, bg = 0;
				oConsole::printf(fg, bg, nameSpec, TestName);
				oConsole::printf(0, 0, ": ");

				oTest::RESULT result = oTest::FILTERED;

				if (!filterChain || filterChain->Passes(TestName, 0)) // put in skip filter here
				{
					oTRACE("========== Begin %s Run %u ==========", TestName, r+1);
					testTime = oTimer();
					result = RunTest(pRTB, statusMessage, oCOUNTOF(statusMessage));
					testTime = oTimer() - testTime;
					oTRACE("========== End %s Run %u ==========", TestName, r+1);

					switch (result)
					{
					case oTest::SUCCESS:
						if (!*statusMessage)
							sprintf_s(statusMessage, "---");
						fg = std::Lime;
						NumSucceeded++;
						break;

					case oTest::FAILURE:
						if (!*statusMessage)
							sprintf_s(statusMessage, "Failed");
						bg = std::Red;
						fg = std::Yellow;
						NumFailed++;
						break;

					case oTest::SKIPPED:
						if (!*statusMessage)
							sprintf_s(statusMessage, "Skipped");
						fg = std::Yellow;
						NumSkipped++;
						break;

					case oTest::LEAKS:
						if (!*statusMessage)
							sprintf_s(statusMessage, "Leaks memory");
						fg = std::Yellow;
						NumFailed++;
						break;
					}
				}

				else
				{
					fg = std::Yellow;
					NumSkipped++;
					sprintf_s(statusMessage, "---");
				}

				oConsole::printf(fg, bg, statusSpec, oAsString(result));
				oConsole::printf(0, 0, ": ");

				fg = 0;
				if (testTime > Desc.TestReallyTooSlowTimeInSeconds)
					fg = std::Red;
				else if (testTime > Desc.TestTooSlowTimeInSeconds)
					fg = std::Yellow;

				oFormatTimeSize(timeMessage, testTime);
				oConsole::printf(fg, 0, timeSpec, timeMessage);
				oConsole::printf(0, 0, ": ");
				oConsole::printf(0, 0, messageSpec, statusMessage);
			}
		}

		oFormatTimeSize(timeMessage, oTimer() - totalTestStartTime);
    if ((NumSucceeded + NumFailed == 0))
  		oConsole::printf(0, 0, "========== Unit Tests: ERROR NO TESTS RUN ==========\n");
    else
		  oConsole::printf(0, 0, "========== Unit Tests: %u succeeded, %u failed, %u skipped in %s ==========\n", NumSucceeded, NumFailed, NumSkipped, timeMessage);

		TotalNumSucceeded += NumSucceeded;
		TotalNumFailed += NumFailed;
		TotalNumSkipped += NumSkipped;
	}

	if (Desc.NumRunIterations != 1) // != so we report if somehow a 0 got through to here
	{
		oFormatTimeSize(timeMessage, oTimer() - allIterationsStartTime);
		oConsole::printf(0, 0, "========== %u Iterations: %u succeeded, %u failed, %u skipped in %s ==========\n", Desc.NumRunIterations, TotalNumSucceeded, TotalNumFailed, TotalNumSkipped, timeMessage);
	}

  if ((TotalNumFailed > 0) || (TotalNumSucceeded + TotalNumFailed == 0))
    return -1;

  return 0;
}

int oTestManager_Impl::RunSpecialMode(const char* _Name)
{
	RegisterSpecialModeTests();

	int err = ENOENT;

	RegisterTestBase* pRTB = SpecialModes[_Name];
	if (pRTB)
	{
		char statusMessage[512];
		oTest::RESULT result = RunTest(pRTB, statusMessage, oCOUNTOF(statusMessage));

		switch (result)
		{
		case oTest::SUCCESS:
			printf("SpecialMode %s: Success", _Name);
			err = 0;
			break;

		case oTest::SKIPPED:
			printf("SpecialMode %s: Skipped", _Name);
			err = -4;
			break;

		case oTest::FILTERED:
			printf("SpecialMode %s: Filtered", _Name);
			err = -3;
			break;

		case oTest::LEAKS:
			printf("SpecialMode %s: Leaks", _Name);
			err = -2;
			break;

		default:
			printf("SpecialMode %s: %s", _Name, *statusMessage ? statusMessage : "(no status message)");
			err = -1;
			break;
		}
	}

	else 
		printf("Special Mode %s not found\n", oSAFESTRN(_Name));

	return err;
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

bool oTestRunSpecialTest(const char* _SpecialTestName, char* _StrStatus, size_t _SizeofStrStatus, int* _pExitCode, threadsafe interface oProcess** _ppProcess)
{
	if (_ppProcess)
		*_ppProcess = 0;

	char cmdline[_MAX_PATH + 128];
	oGetExePath(cmdline);
	sprintf_s(cmdline, "%s -s %s", cmdline, _SpecialTestName);
	oProcess::DESC desc;
	desc.CommandLine = cmdline;
	desc.EnvironmentString = 0;
	desc.StdHandleBufferSize = 64 * 1024;
	oRef<threadsafe oProcess> Process;
	if (!oProcess::Create(&desc, &Process))
	{
		sprintf_s(_StrStatus, _SizeofStrStatus, "Failed to create process for \"%s\"", oSAFESTRN(_SpecialTestName));
		return false;
	}

	Process->Start();

	// If we timeout, that's good, it means the app is still running
	if (Process->Wait(200) &&Process->GetExitCode(_pExitCode))
	{
		char msg[512];
		size_t bytes = Process->ReadFromStdout(msg, oCOUNTOF(msg));
		msg[bytes] = 0;
		sprintf_s(_StrStatus, _SizeofStrStatus, "%s: %s", oSAFESTRN(_SpecialTestName), msg);
		return false;
	}

	if (_ppProcess)
	{
		Process->Reference();
		*_ppProcess = Process;
	}

	return true;
}

