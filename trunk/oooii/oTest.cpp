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
#include "pch.h"
#include <oooii/oTest.h>
#include <oooii/oAssert.h>
#include <oooii/oBuffer.h>
#include <oooii/oConsole.h>
#include <oooii/oErrno.h>
#include <oooii/oFilterChain.h>
#include <oooii/oHash.h>
#include <oooii/oImage.h>
#include <oooii/oLockedPointer.h>
#include <oooii/oPath.h>
#include <oooii/oStdio.h>
#include <oooii/oString.h>
#include <oooii/oRef.h>
#include <oooii/oProcess.h>
#include <string>
#include <vector>
#include <hash_map>

struct oTestManager_Impl : public oTestManager
{
	void GetDesc(DESC* _pDesc) override;
	void SetDesc(DESC* _pDesc) override;

	int RunTests(oFilterChain::FILTER* _pTestFilters, size_t _SizeofTestFilters) override;
	int RunSpecialMode(const char* _Name) override;
	bool RegisterSpecialMode(oSpecialTest* _pSpecialModeTest) override;
	bool FindFullPath(char* _StrFullPath, size_t _SizeofStrFullPath, const char* _StrRelativePath) const override;

	oTest::RESULT RunTest(oTest* _pTest, char* _StatusMessage, size_t _SizeofStatusMessage);

	void PrintDesc();
	void RegisterSpecialModeTests();

	typedef std::vector<oTest*> tests_t;
	tests_t Tests;
	DESC Desc;
	std::string TestSuiteName;
	std::string DataPath;
	std::string GoldenPath;
	std::string OutputPath;
	typedef std::vector<oFilterChain::FILTER> filters_t;
	filters_t Filters;

	typedef stdext::hash_map<std::string, oTest*> specialmodes_t;
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
	static_cast<oTestManager_Impl*>(oTestManager::Singleton())->Tests.push_back(this);
}

oTest::~oTest()
{
	oTestManager_Impl::tests_t& tests = static_cast<oTestManager_Impl*>(oTestManager::Singleton())->Tests;
	oTestManager_Impl::tests_t::iterator it = std::find(tests.begin(), tests.end(), this);
	if (it != tests.end())
		tests.erase(it);
}

const char* oTest::GetName() const
{
	return oGetSimpleTypename(typeid(*this).name());
}

static bool CompareImages(const oImage* _pImage1, const oImage* _pImage2, unsigned int _BitFuzziness)
{
	oImage::DESC desc1, desc2;
	_pImage1->GetDesc(&desc1);
	_pImage2->GetDesc(&desc2);
	
	if (desc1.Size != desc2.Size)
	{
		oSetLastError(EINVAL, "Image sizes don't match.");
		return false;
	}


	// @oooii-tony: This is ripe for parallelism. oParallelFor is still being
	// brought up at this time, but should be deployed here.

	const oColor* c1 = (oColor*)_pImage1->GetData();
	const oColor* c2 = (oColor*)_pImage2->GetData();

	const size_t nPixels = desc1.Size / sizeof(oColor);
	for (size_t i = 0; i < nPixels; i++)
	{
		if (!oEqual(c1[i], c2[i], _BitFuzziness))
			return false;
	}

	return true;
}

bool oTest::TestImage(oImage* _pImage, unsigned int _NthImage)
{
	const char* goldenImageExt = ".png";

	oTestManager::DESC desc;
	oTestManager::Singleton()->GetDesc(&desc);

	char golden[_MAX_PATH];
	{
		char goldenBase[_MAX_PATH];
		if (*desc.GoldenPath)
			strcpy_s(goldenBase, desc.GoldenPath);
		else
			sprintf_s(goldenBase, "%s/%s", desc.DataPath, "GoldenImages");

		if (_NthImage)
			sprintf_s(golden, "%s/%s%u%s", goldenBase, GetName(), _NthImage, goldenImageExt);
		else
			sprintf_s(golden, "%s/%s%s", goldenBase, GetName(), goldenImageExt);

		oCleanPath(golden, golden);
	}

	char output[_MAX_PATH];
	{
		char outputBase[_MAX_PATH];
		if (*desc.OutputPath)
			strcpy_s(outputBase, desc.OutputPath);
		else
			sprintf_s(outputBase, "%s/%s", desc.DataPath, "Output");

		if (_NthImage)
			sprintf_s(output, "%s/%s%u%s", outputBase, GetName(), _NthImage, goldenImageExt);
		else
			sprintf_s(output, "%s/%s%s", outputBase, GetName(), goldenImageExt);

		oCleanPath(output, output);
	}

	oRef<oImage> GoldenImage;
	{
		threadsafe oRef<oBuffer> b;
		if (!oBuffer::Create(golden, false, &b))
		{
			oSetLastError(ENOENT, "Golden image load failed: %s", golden);
			_pImage->Save(output, oImage::MEDIUM);
			return false;
		}

		oLockedPointer<oBuffer> pLockedBuffer(b);
		if (!oImage::Create(pLockedBuffer->GetData(), pLockedBuffer->GetSize(), &GoldenImage))
		{
			oSetLastError(EINVAL, "Corrupt/unloadable golden image file: %s", golden);
			return false;
		}
	}

	if (!CompareImages(_pImage, GoldenImage, 0))
	{
		oSetLastError(EINVAL, "Images did not match");
		_pImage->Save(output);
		return false;
	}

	return true;
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
	oConsole::printf(0, 0, "Data Path: %s%s\n", *Desc.DataPath ? Desc.DataPath : cwd, *Desc.DataPath ? Desc.DataPath : " (CWD)");
	oConsole::printf(0, 0, "Golden Path: %s\n", *Desc.GoldenPath ? Desc.GoldenPath : "(null)");
	oConsole::printf(0, 0, "Output Path: %s\n", *Desc.OutputPath ? Desc.OutputPath : "(null)");
	oConsole::printf(0, 0, "Random Seed: %u\n", Desc.RandomSeed);
}

void oTestManager_Impl::RegisterSpecialModeTests()
{
	for (tests_t::iterator it = Tests.begin(); it != Tests.end(); ++it)
	{
		oSpecialTest* pSpecialTest = dynamic_cast<oSpecialTest*>(*it);
		if (pSpecialTest)
			RegisterSpecialMode(pSpecialTest);
	}
}

oTest::RESULT oTestManager_Impl::RunTest(oTest* _pTest, char* _StatusMessage, size_t _SizeofStatusMessage)
{
	*_StatusMessage = 0;

	srand(Desc.RandomSeed);

	#ifdef _DEBUG
		_CrtMemState before, after, difference;
	#endif
	_CrtMemCheckpoint(&before);
	oTest::RESULT result = _pTest->Run(_StatusMessage, _SizeofStatusMessage);

	_CrtMemCheckpoint(&after);
	if (_CrtMemDifference(&difference, &before, &after))
	{
		// @oooii-tony: This mimics prior behavior, but is this appropriate? How
		// should I handle things that resize internal buffers on first use, so it 
		// changes the memory footprint, but is not a leak?
		if (result != oTest::FAILURE)
		{
			result = oTest::LEAKS;
			#ifdef _DEBUG
				sprintf_s(_StatusMessage, _SizeofStatusMessage, "%u bytes in %u normal blocks, %u bytes in %u CRT blocks", difference.lSizes[_NORMAL_BLOCK], difference.lCounts[_NORMAL_BLOCK], difference.lSizes[_CRT_BLOCK], difference.lCounts[_CRT_BLOCK]);
			#endif
		}

		_CrtMemDumpStatistics(&difference);
	}

	return result;
}

int oTestManager_Impl::RunTests(oFilterChain::FILTER* _pTestFilters, size_t _SizeofTestFilters)
{
	RegisterSpecialModeTests();

	size_t TotalNumSucceeded = 0;
	size_t TotalNumFailed = 0;
	size_t TotalNumSkipped = 0;

	for (size_t r = 0; r < Desc.NumRunIterations; r++)
	{
		size_t NumSucceeded = 0;
		size_t NumFailed = 0;
		size_t NumSkipped = 0;

		threadsafe oRef<oFilterChain> filterChain;
		if (_pTestFilters && _SizeofTestFilters && !oFilterChain::Create(_pTestFilters, _SizeofTestFilters, &filterChain))
			return -1;

		// Prepare formatting used to print results
		char nameSpec[32];
		sprintf_s(nameSpec, "%%-%us: ", Desc.NameColumnWidth);

		char statusSpec[32];
		sprintf_s(statusSpec, "%%-%us", Desc.StatusColumnWidth);

		char messageSpec[32];
		sprintf_s(messageSpec, ": %%s\n");

		char statusMessage[512];

		oConsole::printf(0, 0, "========== %s Run %u ==========\n", Desc.TestSuiteName, r+1);
		PrintDesc();
		for (tests_t::iterator it = Tests.begin(); it != Tests.end(); ++it)
		{
			oTest* pTest = *it;
			oSpecialTest* pSpecialTest = dynamic_cast<oSpecialTest*>(pTest);

			if (pTest && !pSpecialTest)
			{
				char TestName[256];
				strcpy_s(TestName, pTest->GetName());

				oTRACE("========== Begin %s Run %u ==========", TestName, r+1);

				oColor fg = 0, bg = 0;
				oConsole::printf(fg, bg, nameSpec, TestName);

				oTest::RESULT result = oTest::FILTERED;

				if (!filterChain || filterChain->Passes(TestName, 0)) // put in skip filter here
				{
					result = RunTest(pTest, statusMessage, oCOUNTOF(statusMessage));

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
				oConsole::printf(0, 0, messageSpec, statusMessage);

				oTRACE("========== End %s Run %u ==========", TestName, r+1);
			}
		}

		oConsole::printf(0, 0, "========== Unit Tests: %u succeeded, %u failed, %u skipped ==========\n", NumSucceeded, NumFailed, NumSkipped);

		TotalNumSucceeded += NumSucceeded;
		TotalNumFailed += NumFailed;
		TotalNumSkipped += NumSkipped;
	}

	if (Desc.NumRunIterations != 1) // != so we report if somehow a 0 got through to here
		oConsole::printf(0, 0, "========== %u Iterations: %u succeeded, %u failed, %u skipped ==========\n", Desc.NumRunIterations, TotalNumSucceeded, TotalNumFailed, TotalNumSkipped);

	return (TotalNumFailed > 0) ? -1 : 0;
}

int oTestManager_Impl::RunSpecialMode(const char* _Name)
{
	RegisterSpecialModeTests();

	int err = ENOENT;

	oTest* pTest = SpecialModes[_Name];
	if (pTest)
	{
		char statusMessage[512];
		oTest::RESULT result = RunTest(pTest, statusMessage, oCOUNTOF(statusMessage));
		
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

bool oTestManager_Impl::RegisterSpecialMode(oSpecialTest* _pSpecialModeTest)
{
	oTest* pTest = SpecialModes[_pSpecialModeTest->GetName()];
	if (pTest) return false;
	SpecialModes[_pSpecialModeTest->GetName()] = _pSpecialModeTest;
	return true;
}

bool oTestManager_Impl::FindFullPath(char* _StrFullPath, size_t _SizeofStrFullPath, const char* _StrRelativePath) const
{
	char appPath[_MAX_PATH];
	oGetSysPath(appPath, oSYSPATH_APP);
	return oFindPath(_StrFullPath, _SizeofStrFullPath, _StrRelativePath, appPath, DataPath.c_str(), oFile::Exists);
}


bool oTestRunSpecialTest( const char* _SpecialTestName, char* _StrStatus, size_t _SizeofStrStatus, int* _pExitCode, threadsafe interface oProcess** _ppProcess )
{
	char cmdline[_MAX_PATH + 128];
	oGetExePath(cmdline);
	sprintf_s(cmdline, "%s -s %s", cmdline, _SpecialTestName);
	oProcess::DESC desc;
	desc.CommandLine = cmdline;
	desc.EnvironmentString = 0;
	desc.StdHandleBufferSize = 64 * 1024;
	if (!oProcess::Create(&desc, _ppProcess))
	{
		sprintf_s(_StrStatus, _SizeofStrStatus, "Failed to create process for \"%s\"", oSAFESTRN(_SpecialTestName));
		return false;
	}

	(*_ppProcess)->Start();

	// If we timeout, that's good, it means the app is still running
	if ((*_ppProcess)->Wait(200) && (*_ppProcess)->GetExitCode(_pExitCode))
	{
		char msg[512];
		size_t bytes = (*_ppProcess)->ReadFromStdout(msg, oCOUNTOF(msg));
		msg[bytes] = 0;
		sprintf_s(_StrStatus, _SizeofStrStatus, "%s: %s", oSAFESTRN(_SpecialTestName), msg);
		(*_ppProcess)->Release();
		*_ppProcess = 0;
		return false;
	}

	return true;
}

