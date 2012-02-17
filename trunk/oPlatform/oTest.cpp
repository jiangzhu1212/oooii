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
#include <oPlatform/oTest.h>
#include <oBasis/oBuffer.h>
#include <oBasis/oCppParsing.h>
#include <oBasis/oError.h>
#include <oBasis/oFixedString.h>
#include <oBasis/oHash.h>
#include <oBasis/oLockedPointer.h>
#include <oBasis/oRef.h>
#include <oPlatform/oReporting.h>
#include <oPlatform/oConsole.h>
#include <oPlatform/oDebugger.h> // needed to ensure leak tracker is up before oTestManager
#include <oPlatform/oEvent.h> // inter-process event required to sync "special tests" as they launch a new process
#include <oPlatform/oFile.h> // needed for oFileExists... this could be passed through as an oFUNCTION
#include <oPlatform/oGPU.h> // needed for oGPUEnum so we can generate different images for AMD cards when appropriate
#include <oPlatform/oImage.h> // the crux of most tests... going to be hard to get rid of this dependency
#include <oPlatform/oMsgBox.h> // only used to notify about zombies
#include <oPlatform/oProgressBar.h> // only really so it itself can be tested, but perhaps this can be moved to a unit test?
#include <oPlatform/oProcess.h> // used to launch special tests
#include <oPlatform/oSystem.h> // used for getting various paths
#include <oPlatform/oStandards.h> // standard colors for a console app, maybe this can be callouts? log file path... can be an option?
#include <algorithm>
#include <unordered_map>

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
	static_assert(oTest::NUM_TEST_RESULTS == oCOUNTOF(sStrings), "");
	return sStrings[_Result];
}

struct oTestManager_Impl : public oTestManager
{
	oTestManager_Impl();
	~oTestManager_Impl();

	void GetDesc(DESC* _pDesc) override;
	void SetDesc(DESC* _pDesc) override;

	oTest::RESULT RunTests(oFilterChain::FILTER* _pTestFilters, size_t _SizeofTestFilters) override;
	oTest::RESULT RunSpecialMode(const char* _Name) override;
	oTest::RESULT RunTest(RegisterTestBase* _pRegisterTestBase, char* _StatusMessage, size_t _SizeofStatusMessage);

	void PrintDesc();
	void RegisterSpecialModeTests();
	void RegisterZombies();
	bool KillZombies(const char* _Name);
	bool KillZombies();

	bool BuildPath(char* _StrFullPath, size_t _SizeofStrFullPath, const char* _StrRelativePath, oTest::PATH_TYPE _PathType, bool _FileMustExist) const;

	inline void Report(oConsoleReporting::REPORT_TYPE _Type, const char* _Format, ...) { va_list args; va_start(args, _Format); oConsoleReporting::VReport(_Type, _Format, args); }
	inline void ReportSep() { Report(oConsoleReporting::DEFAULT, "%c ", 179); }

	// Returns the number of tests that will run based on num iterations and 
	// discounting filtered tests.
	size_t CalculateNumTests(const oTestManager::DESC& _Desc, threadsafe oFilterChain* _pFilterChain);

	typedef std::vector<RegisterTestBase*> tests_t;
	tests_t Tests;
	DESC Desc;
	oGPU_DESC GPUs[8];
	unsigned int NumGPUs;
	bool ShowProgressBar;
	std::string TestSuiteName;
	std::string DataPath;
	std::string ExecutablesPath;
	std::string GoldenBinariesPath;
	std::string GoldenImagesPath;
	std::string TempPath;
	std::string InputPath;
	std::string OutputPath;
	typedef std::vector<oFilterChain::FILTER> filters_t;
	filters_t Filters;

	typedef std::tr1::unordered_map<std::string, RegisterTestBase*> specialmodes_t;
	specialmodes_t SpecialModes;

	typedef std::vector<std::string> zombies_t;
	zombies_t PotentialZombies;
};

#include "oCRTLeakTracker.h"

struct oTestManagerImplSingleton : oProcessSingleton<oTestManagerImplSingleton>
{
	oTestManagerImplSingleton();
	~oTestManagerImplSingleton();

	oTestManager_Impl* pImpl;

	static const oGUID GUID;
	oRef<oCRTLeakTracker> CRTLeakTracker;
};

// {97E7D7DD-B3B6-4691-A383-6D9F88C034C6}
const oGUID oTestManagerImplSingleton::GUID = { 0x97e7d7dd, 0xb3b6, 0x4691, { 0xa3, 0x83, 0x6d, 0x9f, 0x88, 0xc0, 0x34, 0xc6 } };

oTestManagerImplSingleton::oTestManagerImplSingleton()
	: CRTLeakTracker(oCRTLeakTracker::Singleton())
{
	oDebuggerReportCRTLeaksOnExit(true); // oTestManager can be instantiated very early in static init, so make sure we're tracking memory for it
	pImpl = new oTestManager_Impl();
}

oTestManagerImplSingleton::~oTestManagerImplSingleton()
{
	delete pImpl;
}

oTestManager* oTestManager::Singleton()
{
	// Because this object has the potential to be initialized very early on as
	// tests register themselves, ensure the memory tracker is up to track it...
	return oTestManagerImplSingleton::Singleton()->pImpl;
}

oDEFINE_FLAG(oTest, FileMustExist);

oTest::oTest()
{
}

oTest::~oTest()
{
}

bool oTest::BuildPath(char* _StrFullPath, size_t _SizeofStrFullPath, const char* _StrRelativePath, PATH_TYPE _PathType, bool _FileMustExist) const
{
	return static_cast<oTestManager_Impl*>(oTestManager::Singleton())->BuildPath(_StrFullPath, _SizeofStrFullPath, _StrRelativePath, _PathType, _FileMustExist);
}

const char* oTest::GetName() const
{
	return oGetTypename(typeid(*this).name());
}

static void BuildDataPath(char* _StrDestination, size_t _SizeofStrDestination, const char* _TestName, const char* _DataPath, const char* _DataSubpath, const char* _Path, unsigned int _NthItem, const char* _Ext)
{
	oStringPath base;
	if (_Path && *_Path)
		base = _Path;
	else if (_DataSubpath && *_DataSubpath)
		sprintf_s(base, "%s/%s", _DataPath, _DataSubpath);
	else
		sprintf_s(base, "%s", _DataPath);

	if (_NthItem)
		sprintf_s(_StrDestination, _SizeofStrDestination, "%s/%s%u%s", base.c_str(), _TestName, _NthItem, _Ext);
	else
		sprintf_s(_StrDestination, _SizeofStrDestination, "%s/%s%s", base.c_str(), _TestName, _Ext);

	oCleanPath(_StrDestination, _SizeofStrDestination, _StrDestination);
}

template<size_t size> void BuildDataPath(char (&_StrDestination)[size], const char* _TestName, const char* _DataPath, const char* _DataSubpath, const char* _Path, unsigned int _NthItem, const char* _Ext) { BuildDataPath(_StrDestination, size, _TestName, _DataPath, _DataSubpath, _Path, _NthItem, _Ext); }

bool oTest::TestBinary(const void* _pBuffer, size_t _SizeofBuffer, const char* _FileExtension, unsigned int _NthBinary)
{
	oTestManager::DESC desc;
	oTestManager::Singleton()->GetDesc(&desc);
	oGPU_VENDOR GPUVendor = static_cast<oTestManager_Impl*>(oTestManager::Singleton())->GPUs[0].Vendor; // @oooii-tony: Make this more elegant

	oStringPath golden;
	BuildDataPath(golden.c_str(), GetName(), desc.DataPath, "GoldenBinaries", desc.GoldenBinariesPath, _NthBinary, _FileExtension);
	oStringPath goldenAMD;
	char ext[32];
	sprintf_s(ext, "_AMD%s", _FileExtension);
	BuildDataPath(goldenAMD.c_str(), GetName(), desc.DataPath, "GoldenBinaries", desc.GoldenBinariesPath, _NthBinary, ext);
	oStringPath output;
	BuildDataPath(output.c_str(), GetName(), desc.DataPath, "Output", desc.OutputPath, _NthBinary, _FileExtension);

	bool bSaveTestBuffer = false;
	oOnScopeExit SaveTestBuffer([&]
	{
		if (bSaveTestBuffer)
		{
			if (!oFileSave(output, _pBuffer, _SizeofBuffer, false))
				oErrorSetLast(oERROR_INVALID_PARAMETER, "Output binary save failed: %s", output.c_str());
		}
	});

	oRef<oBuffer> GoldenBinary;
	{
		if (GPUVendor == oGPU_VENDOR_AMD)
		{
			// Try to load a more-specific golden binary, but if it's not there it's
			// ok to try to use the default one.
			oBufferCreate(goldenAMD, &GoldenBinary);
		}

		if (!GoldenBinary)
		{
			if (!oBufferCreate(golden, &GoldenBinary))
			{
				if (GPUVendor != oGPU_VENDOR_NVIDIA)
				{
					oStringPath outputAMD;
					char ext[32];
					sprintf_s(ext, "_AMD%s", _FileExtension);
					BuildDataPath(outputAMD.c_str(), GetName(), desc.DataPath, "Output", desc.OutputPath, _NthBinary, ext);
					oWARN("Shared Golden Images are only valid if generated from an NVIDIA card. Note: it may be appropriate to check this in as an AMD-specific card to %s if there's a difference in golden images between NVIDIA and AMD.", outputAMD.c_str());
				}

				bSaveTestBuffer = true;
				return oErrorSetLast(oERROR_NOT_FOUND, "Golden binary load failed: %s", golden.c_str());
			}
		}
	}

	if (_SizeofBuffer != GoldenBinary->GetSize())
	{
		oStringS testSize, goldenSize;
		oFormatMemorySize(testSize, _SizeofBuffer, 2);
		oFormatMemorySize(goldenSize, GoldenBinary->GetSize(), 2);
		bSaveTestBuffer = true;
		return oErrorSetLast(oERROR_GENERIC, "Golden binary compare failed because the binaries are different sizes (test is %s, golden is %s)", testSize.c_str(), goldenSize.c_str());
	}

	if (memcmp(_pBuffer, GoldenBinary->GetData(), GoldenBinary->GetSize()))
	{
		bSaveTestBuffer = true;
		return oErrorSetLast(oERROR_GENERIC, "Golden binary compare failed because the bytes differ");
	}

	return true;
}

bool oTest::TestImage(oImage* _pImage, unsigned int _NthImage)
{
	const char* goldenImageExt = ".png";
	const char* goldenImageExtAMD = "_AMD.png";
	const char* diffImageExt = "_diff.png";

	oTestManager::DESC desc;
	oTestManager::Singleton()->GetDesc(&desc);
	oGPU_VENDOR GPUVendor = static_cast<oTestManager_Impl*>(oTestManager::Singleton())->GPUs[0].Vendor; // @oooii-tony: Make this more elegant

	oStringPath golden;
	BuildDataPath(golden.c_str(), GetName(), desc.DataPath, "GoldenImages", desc.GoldenImagesPath, _NthImage, goldenImageExt);
	oStringPath goldenAMD;
	BuildDataPath(goldenAMD.c_str(), GetName(), desc.DataPath, "GoldenImages", desc.GoldenImagesPath, _NthImage, goldenImageExtAMD);
	oStringPath output;
	BuildDataPath(output.c_str(), GetName(), desc.DataPath, "Output", desc.OutputPath, _NthImage, goldenImageExt);
	oStringPath diff;
	BuildDataPath(diff.c_str(), GetName(), desc.DataPath, "Output", desc.OutputPath, _NthImage, diffImageExt);

	oImage::DESC iDesc, gDesc;
	_pImage->GetDesc(&iDesc);
	oRef<oImage> GoldenImage;
	{
		oRef<oBuffer> b;
		if (GPUVendor == oGPU_VENDOR_AMD)
		{
			// Try to load a more-specific golden image, but if it's not there it's ok
			// to try to use the default one.
			oBufferCreate(goldenAMD, &b);
		}

		if (!b)
		{
			if (!oBufferCreate(golden, &b))
			{
				if (GPUVendor != oGPU_VENDOR_NVIDIA)
				{
					oStringPath outputAMD;
					BuildDataPath(outputAMD.c_str(), GetName(), desc.DataPath, "Output", desc.OutputPath, _NthImage, goldenImageExtAMD);
					oWARN("Shared Golden Images are only valid if generated from an NVIDIA card. Note: it may be appropriate to check this in as an AMD-specific card to %s if there's a difference in golden images between NVIDIA and AMD.", outputAMD.c_str());
				}

				if (!oImageSave(_pImage, output))
					return oErrorSetLast(oERROR_INVALID_PARAMETER, "Output image save failed: %s", output.c_str());
				return oErrorSetLast(oERROR_NOT_FOUND, "Golden image load failed: %s", golden.c_str());
			}
		}

		if(oImageIsAlphaFormat(iDesc.Format)) //if source has alpha, force loading an opaque alpha for golden image as well.
		{
			if (!oImageCreate(golden, b->GetData(), b->GetSize(), oImage::ForceAlphaFlag(), &GoldenImage))
				return oErrorSetLast(oERROR_INVALID_PARAMETER, "Corrupt/unloadable golden image file: %s", golden.c_str());
		}
		else
		{
			if (!oImageCreate(golden, b->GetData(), b->GetSize(), &GoldenImage))
				return oErrorSetLast(oERROR_INVALID_PARAMETER, "Corrupt/unloadable golden image file: %s", golden.c_str());
		}
	}

	oRef<oImage> diffs;
	unsigned int nDifferences = 0;

	// Compare dimensions/format before going into pixels
	{
		GoldenImage->GetDesc(&gDesc);

		if (iDesc.Dimensions != gDesc.Dimensions)
			return oErrorSetLast(oERROR_GENERIC, "Golden image compare failed because the images are different dimensions (test is %ix%i, golden is %ix%i)", iDesc.Dimensions.x, iDesc.Dimensions.y, gDesc.Dimensions.x, gDesc.Dimensions.y);

		if (iDesc.Format != gDesc.Format)
		{
			if (!oImageSave(_pImage, output))
				return oErrorSetLast(oERROR_IO, "Output image save failed: %s", output);
			return oErrorSetLast(oERROR_GENERIC, "Golden image compare failed because the images are different formats (test is %s, golden is %s)", oAsString(iDesc.Format), oAsString(gDesc.Format));
		}
	}

	oTest::DESC testDescOverrides;
	testDescOverrides.maxRMSError = desc.maxRMSError;
	testDescOverrides.colorChannelTolerance = desc.colorChannelTolerance;
  
	testDescOverrides.DiffImageMultiplier = desc.DefaultDiffImageMultiplier;
	OverrideTestDesc(testDescOverrides);

  float RMSError = 0.0f;
	bool compareSucceeded = oImageCompare(_pImage, GoldenImage, testDescOverrides.colorChannelTolerance, &RMSError, &diffs, testDescOverrides.DiffImageMultiplier);

	if (!compareSucceeded || (RMSError > testDescOverrides.maxRMSError))
	{
		if (!oImageSave(_pImage, output))
			return oErrorSetLast(oERROR_IO, "Output image save failed: %s", output.c_str());

		if (diffs && !oImageSave(diffs, diff))
			return oErrorSetLast(oERROR_IO, "Diff image save failed: %s", diff.c_str());

		return oErrorSetLast(oERROR_GENERIC, "Golden image compare failed (%.03f RMS Error, Max Allowed %f): (Golden)%s != (Output)%s", 
      RMSError, testDescOverrides.maxRMSError, golden.c_str(), output.c_str());
	}

	return true;
}

bool oSpecialTest::CreateProcess(const char* _SpecialTestName, threadsafe interface oProcess** _ppProcess)
{
	if (!_SpecialTestName || !_ppProcess)
		return oErrorSetLast(oERROR_INVALID_PARAMETER);

	oStringML cmdline;
	if (!oSystemGetPath(cmdline.c_str(), oSYSPATH_APP_FULL))
		return oErrorSetLast(oERROR_NOT_FOUND);

	sprintf_s(cmdline, "%s -s %s", cmdline.c_str(), _SpecialTestName);
	oProcess::DESC desc;
	desc.CommandLine = cmdline;
	desc.EnvironmentString = 0;
	desc.StdHandleBufferSize = 64 * 1024;
	return oProcessCreate(desc, _ppProcess);
}

bool oSpecialTest::Start(threadsafe interface oProcess* _pProcess, char* _StrStatus, size_t _SizeofStrStatus, int* _pExitCode, unsigned int _TimeoutMS)
{
	if (!_pProcess || !_StrStatus || !_pExitCode)
		return oErrorSetLast(oERROR_INVALID_PARAMETER);

	oProcess::DESC desc;
	_pProcess->GetDesc(&desc);
	const char* SpecialTestName = oStrStrReverse(desc.CommandLine, "-s ") + 3;
	if (!SpecialTestName || !*SpecialTestName)
		return oErrorSetLast(oERROR_INVALID_PARAMETER, "Process with an invalid command line for oSpecialTest specified.");

	oStringM interprocessName;
	sprintf_s(interprocessName, "oTest.%s.Started", SpecialTestName);
	oEvent Started(interprocessName);
	_pProcess->Start();

	oTestManager::DESC testingDesc;
	oTestManager::Singleton()->GetDesc(&testingDesc);

	if (testingDesc.EnableSpecialTestTimeouts && !Started.Wait(_TimeoutMS))
	{
		sprintf_s(_StrStatus, _SizeofStrStatus, "Timed out waiting for %s to start.", SpecialTestName);
		oTRACE("*** SPECIAL MODE UNIT TEST %s timed out waiting for Started event. (Ensure the special mode test sets the started event when appropriate.) ***", SpecialTestName);
		if (!_pProcess->GetExitCode(_pExitCode))
			_pProcess->Kill(oERROR_TIMEOUT);

		sprintf_s(_StrStatus, _SizeofStrStatus, "Special Mode %s timed out on start.", SpecialTestName);
		return false;
	}

	// If we timeout on ending, that's good, it means the app is still running
	if ((_pProcess->Wait(200) && _pProcess->GetExitCode(_pExitCode)))
	{
		oStringXL msg;
		size_t bytes = _pProcess->ReadFromStdout(msg.c_str(), msg.capacity());
		msg[bytes] = 0;
		if (bytes)
			sprintf_s(_StrStatus, _SizeofStrStatus, "%s: %s", SpecialTestName, msg.c_str());
		return false;
	}

	return true;
}

void oSpecialTest::NotifyReady()
{
	oStringM interprocessName;
	const char* testName = oGetTypename(typeid(*this).name());
	sprintf_s(interprocessName, "oTest.%s.Started", testName);
	oEvent Ready(interprocessName);
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

oTestManager_Impl::oTestManager_Impl()
	: ShowProgressBar(false)
{
}

oTestManager_Impl::~oTestManager_Impl()
{
}

void oTestManager_Impl::GetDesc(DESC* _pDesc)
{
	*_pDesc = Desc;
}

void oTestManager_Impl::SetDesc(DESC* _pDesc)
{
	TestSuiteName = _pDesc->TestSuiteName && *_pDesc->TestSuiteName ? _pDesc->TestSuiteName : "OOOii Unit Test Suite";
	
	oStringPath defaultDataPath;
	oSystemGetPath(defaultDataPath.c_str(), oSYSPATH_DATA);

	DataPath = _pDesc->DataPath ? _pDesc->DataPath : defaultDataPath;
	if (_pDesc->ExecutablesPath)
		ExecutablesPath = _pDesc->ExecutablesPath;
	else
	{
		oStringPath exes(defaultDataPath);
		#ifdef o64BIT
			strcat_s(exes.c_str(), "../bin/x64/");
		#elif o32BIT
			strcat_s(exes.c_str(), "../bin/win32/");
		#else
			#error Unknown bitness
		#endif
		ExecutablesPath = oCleanPath(exes.c_str(), exes);
	}

	GoldenBinariesPath = _pDesc->GoldenBinariesPath ? _pDesc->GoldenBinariesPath : (DataPath + "GoldenBinaries/");
	GoldenImagesPath = _pDesc->GoldenImagesPath ? _pDesc->GoldenImagesPath : (DataPath + "GoldenImages/");
	TempPath = _pDesc->TempPath ? _pDesc->TempPath : (DataPath + "Temp/");
	InputPath = _pDesc->InputPath ? _pDesc->InputPath : DataPath;
	OutputPath = _pDesc->OutputPath ? _pDesc->OutputPath : (DataPath + "FailedImageCompares/");
	Desc = *_pDesc;
	Desc.TestSuiteName = TestSuiteName.c_str();
	Desc.DataPath = DataPath.c_str();
	Desc.ExecutablesPath = ExecutablesPath.c_str();
	Desc.GoldenBinariesPath = GoldenBinariesPath.c_str();
	Desc.GoldenImagesPath = GoldenImagesPath.c_str();
	Desc.TempPath = TempPath.c_str();
	Desc.InputPath = InputPath.c_str();
	Desc.OutputPath = OutputPath.c_str();
}

void oTestManager_Impl::PrintDesc()
{
	oStringPath cwd;
	oSystemGetPath(cwd.c_str(), oSYSPATH_CWD);
	oStringPath datapath;
	oCleanPath(datapath.c_str(), oSAFESTR(Desc.DataPath));
	oEnsureSeparator(datapath.c_str());
	bool dataPathIsCWD = !_stricmp(cwd, datapath);

	Report(oConsoleReporting::INFO, "CWD Path: %s\n", cwd.c_str());
	Report(oConsoleReporting::INFO, "Data Path: %s%s\n", (Desc.DataPath && *Desc.DataPath) ? Desc.DataPath : cwd.c_str(), dataPathIsCWD ? " (CWD)" : "");
	Report(oConsoleReporting::INFO, "Executables Path: %s\n", *Desc.ExecutablesPath ? Desc.ExecutablesPath : "(null)");
	Report(oConsoleReporting::INFO, "Golden Binaries Path: %s\n", *Desc.GoldenImagesPath ? Desc.GoldenImagesPath : "(null)");
	Report(oConsoleReporting::INFO, "Golden Images Path: %s\n", *Desc.GoldenImagesPath ? Desc.GoldenImagesPath : "(null)");
	Report(oConsoleReporting::INFO, "Temp Path: %s\n", *Desc.TempPath ? Desc.TempPath : "(null)");
	Report(oConsoleReporting::INFO, "Input Path: %s\n", *Desc.InputPath ? Desc.InputPath : "(null)");
	Report(oConsoleReporting::INFO, "Output Path: %s\n", *Desc.OutputPath ? Desc.OutputPath : "(null)");
	Report(oConsoleReporting::INFO, "Random Seed: %u\n", Desc.RandomSeed);
	Report(oConsoleReporting::INFO, "Special Test Timeouts: %sabled\n", Desc.EnableSpecialTestTimeouts ? "en" : "dis");

	for (unsigned int i = 0; i < NumGPUs; i++)
	{
		Report(oConsoleReporting::INFO, "Video Card %u: %s\n", i, GPUs[i].GPUDescription.c_str());
		Report(oConsoleReporting::INFO, "Video Driver %u: %s v%d.%d %s %d.%d interface running %d.%d features\n", i, GPUs[i].DriverDescription.c_str(), GPUs[i].DriverVersion.Major, GPUs[i].DriverVersion.Minor, oAsString(GPUs[i].API), GPUs[i].InterfaceVersion.Major, GPUs[i].InterfaceVersion.Minor, GPUs[i].FeatureVersion.Major, GPUs[i].FeatureVersion.Minor);
	}
}

void oTestManager_Impl::RegisterSpecialModeTests()
{
	for (tests_t::iterator it = Tests.begin(); it != Tests.end(); ++it)
	{
		RegisterTestBase* pRTB = *it;

		if (!pRTB->IsSpecialTest())
			continue;

		const char* Name = oGetTypename(pRTB->GetTypename());
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

		oProcessTerminate(pids[i], oERROR_CANCELED);
		if (!oProcessWaitExit(pids[i], 5000))
		{
			oMSGBOX_DESC mb;
			mb.Type = oMSGBOX_WARN;
			mb.TimeoutMS = 20000;
			mb.Title = "OOOii Test Manager";
			oMsgBox(mb, "Cannot terminate stale process %u, please end this process before continuing.", pids[i]);
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

	if (!oFileDelete(TempPath.c_str()) && oErrorGetLast() != oERROR_NOT_FOUND)
		oVERIFY(false && "oFileDelete(TempPath.c_str())");

	// @oooii-tony: Moving other stuff that are false-positives here so I can see
	// them all...
	oCRTLeakTracker::Singleton()->NewContext();

	oTest* pTest = _pRegisterTestBase->New();
	oTest::RESULT result = pTest->Run(_StatusMessage, _SizeofStatusMessage);
	delete pTest;

	// obug_1763: We need to forcefully flushIOCP to ensure it doesn't report 
	// memory leaks.
	// @oooii-tony: This isn't the only culprit. Maybe we should move all these
	// into oCRTLeakTracker so the dependency on reporting seems explicit.
	extern void FlushIOCP();
	FlushIOCP();
	bool Leaks = oCRTLeakTracker::Singleton()->ReportLeaks();
	if (result != oTest::FAILURE && Leaks)
	{
		result = oTest::LEAKS;
		sprintf_s(_StatusMessage, _SizeofStatusMessage, "Leaks (see debug log for full report)");
	}

	return result;
}

size_t oTestManager_Impl::CalculateNumTests(const oTestManager::DESC& _Desc, threadsafe oFilterChain* _pFilterChain)
{
	size_t nTests = 0;
	for (tests_t::iterator it = Tests.begin(); it != Tests.end(); ++it)
	{
		RegisterTestBase* pRTB = *it;
		if (pRTB && !pRTB->IsSpecialTest())
		{
			oStringM TestName;
			TestName = oGetTypename(pRTB->GetTypename());
			
			if (!_pFilterChain || _pFilterChain->Passes(TestName, 0))
				nTests++;
		}
	}
	
	return _Desc.NumRunIterations * nTests;
}

static bool SortAlphabetically(oTestManager::RegisterTestBase* _Test1, oTestManager::RegisterTestBase* _Test2)
{
	return _stricmp(_Test1->GetTypename(), _Test2->GetTypename()) < 0;
}

oTest::RESULT oTestManager_Impl::RunTests(oFilterChain::FILTER* _pTestFilters, size_t _SizeofTestFilters)
{
	RegisterZombies();

	RegisterSpecialModeTests();

	// Ensure - regardless of linkage order - that tests appear in alphabetical order
	std::sort(Tests.begin(), Tests.end(), SortAlphabetically);

	// This can't be set in the ctor because the ctor could happen at static init
	// time and the underlying code relies on a statically compiled regex. If that
	// regex weren't statically compiled, it would either be slow or could report
	// as a leak if it were a function-local static.
	{
		unsigned int GPUI = 0;
		while (oGPUEnum(GPUI, &GPUs[GPUI]))
		{
			if (GPUI > oCOUNTOF(GPUs))
				oWARN("There are more GPUs attached to the system than we have storage for! Only holding information for the first %d", oCOUNTOF(GPUs));

			GPUI++;
		}

		NumGPUs = GPUI;
	}

	size_t TotalNumSucceeded = 0;
	size_t TotalNumFailed = 0;
	size_t TotalNumLeaks = 0;
	size_t TotalNumSkipped = 0;

	char fcErr[1024];
	bool fcSuccess = false;
	oFilterChain filterChain(_pTestFilters, _SizeofTestFilters, fcErr, oCOUNTOF(fcErr), &fcSuccess);

	// @oooii-tony: For testing progress bar functionality. If we like this as
	// a feature of the unit test, we should expose it through the DESC.
	//ShowProgressBar = true;

	oRef<threadsafe oProgressBar> ProgressBar;
	if (ShowProgressBar)
	{
		oProgressBar::DESC PBDesc;
		PBDesc.Show = false;
		PBDesc.ShowStopButton = true;
		PBDesc.AlwaysOnTop = false;
		PBDesc.UnknownProgress = true;
		PBDesc.Stopped = false;

		oVERIFY(oProgressBarCreate(PBDesc, oConsole::GetNativeHandle(), &ProgressBar));

		oStringXL title;
		oConsole::GetTitle(title.c_str());
		ProgressBar->SetTitle(title);
	}

	size_t ProgressTotalNumTests = 0;
	size_t ProgressNumTestsSoFar = 0;
	if (ShowProgressBar)
	{
		ProgressBar->SetText("Calculating the number of tests...", "");
		oProgressBar::DESC* pDesc = ProgressBar->Map();
		pDesc->Show = true;
		pDesc->UnknownProgress = true;
		ProgressBar->Unmap();
		ProgressTotalNumTests = CalculateNumTests(Desc, &filterChain);
		if (!ProgressBar->Wait(0) && oErrorGetLast() == oERROR_CANCELED)
		{
			oTRACE("ProgressBar Stop Pressed, aborting.");
			return oTest::FAILURE;
		}
		
		ProgressBar->SetPercentage(1);
		pDesc = ProgressBar->Map();
		pDesc->UnknownProgress = false;
		ProgressBar->Unmap();
	}

	oCRTLeakTracker::Singleton()->Enable(Desc.EnableLeakTracking);
	oCRTLeakTracker::Singleton()->CaptureCallstack(Desc.CaptureCallstackForTestLeaks);

	oStringXL timeMessage;
	double allIterationsStartTime = oTimer();
	for (size_t r = 0; r < Desc.NumRunIterations; r++)
	{
		size_t Count[oTest::NUM_TEST_RESULTS];
		memset(Count, 0, sizeof(size_t) * oTest::NUM_TEST_RESULTS);

		// Prepare formatting used to print results
		oStringS nameSpec;
		sprintf_s(nameSpec, "%%-%us", Desc.NameColumnWidth);

		oStringS statusSpec;
		sprintf_s(statusSpec, "%%-%us", Desc.StatusColumnWidth);

		oStringS timeSpec;
		sprintf_s(timeSpec, "%%-%us", Desc.TimeColumnWidth);

		oStringS messageSpec;
		sprintf_s(messageSpec, "%%s\n");

		oStringXL statusMessage;

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
				oStringM TestName;
				strcpy_s(TestName.c_str(), oGetTypename(pRTB->GetTypename()));
				
				double testDuration = 0.0;

				Report(oConsoleReporting::DEFAULT, nameSpec, TestName.c_str());
				ReportSep();

				oTest::RESULT result = oTest::FILTERED;
				oConsoleReporting::REPORT_TYPE ReportType = oConsoleReporting::DEFAULT;

				if (filterChain.Passes(TestName, 0)) // put in skip filter here
				{
					if (pRTB->GetBugNumber() == 0)
					{
						if (ShowProgressBar)
						{
							if (!ProgressBar->Wait(0) && oErrorGetLast() == oERROR_CANCELED)
								break;

							ProgressBar->SetText(TestName);
						}

						oTRACE("========== Begin %s Run %u ==========", TestName.c_str(), r+1);
						double testStart = oTimer();
						result = RunTest(pRTB, statusMessage.c_str(), statusMessage.capacity());
						testDuration = oTimer() - testStart;
						oTRACE("========== End %s Run %u ==========", TestName.c_str(), r+1);
						Count[result]++;

						if (ShowProgressBar)
						{
							ProgressNumTestsSoFar++;
							ProgressBar->SetPercentage(static_cast<int>((100 * (ProgressNumTestsSoFar+1)) / ProgressTotalNumTests));
						}
					}

					else
						result = pRTB->GetBugResult();

					switch (result)
					{
						case oTest::SUCCESS:
							if (!*statusMessage || !strcmp("operation was successful", statusMessage))
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
				if (testDuration > Desc.TestReallyTooSlowTimeInSeconds)
					ReportType = oConsoleReporting::ERR;
				else if (testDuration > Desc.TestTooSlowTimeInSeconds)
					ReportType = oConsoleReporting::WARN;

				oFormatTimeSize(timeMessage.c_str(), round(testDuration), true);
				Report(ReportType, timeSpec, timeMessage.c_str());
				ReportSep();
				Report(oConsoleReporting::DEFAULT, messageSpec, statusMessage.c_str());
			}
		}

		size_t NumSucceeded = Count[oTest::SUCCESS];
		size_t NumFailed = Count[oTest::FAILURE];
		size_t NumLeaks = Count[oTest::LEAKS]; 
		size_t NumSkipped = Count[oTest::SKIPPED] + Count[oTest::FILTERED] + Count[oTest::BUGGED] + Count[oTest::NOTREADY];

		oFormatTimeSize(timeMessage.c_str(), round(oTimer() - totalTestStartTime));
    if ((NumSucceeded + NumFailed + NumLeaks == 0))
  		Report(oConsoleReporting::ERR, "========== Unit Tests: ERROR NO TESTS RUN ==========\n");
    else
		  Report(oConsoleReporting::INFO, "========== Unit Tests: %u succeeded, %u failed, %u skipped in %s ==========\n", NumSucceeded, NumFailed + NumLeaks, NumSkipped, timeMessage.c_str());

		TotalNumSucceeded += NumSucceeded;
		TotalNumFailed += NumFailed;
		TotalNumLeaks += NumLeaks;
		TotalNumSkipped += NumSkipped;
	}

	if (ShowProgressBar && ProgressBar->Wait(0) && oErrorGetLast() == oERROR_CANCELED)
	{
		Report(oConsoleReporting::ERR, "\n\n========== Stopped by user ==========");
		return oTest::FAILURE;
	}
	
	if (Desc.NumRunIterations != 1) // != so we report if somehow a 0 got through to here
	{
		oFormatTimeSize(timeMessage.c_str(), round(oTimer() - allIterationsStartTime));
		Report(oConsoleReporting::INFO, "========== %u Iterations: %u succeeded, %u failed, %u skipped in %s ==========\n", Desc.NumRunIterations, TotalNumSucceeded, TotalNumFailed + TotalNumLeaks, TotalNumSkipped, timeMessage.c_str());
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
		oStringXL statusMessage;
		result = RunTest(pRTB, statusMessage.c_str(), statusMessage.capacity());
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

bool oTestManager_Impl::BuildPath(char* _StrFullPath, size_t _SizeofStrFullPath, const char* _StrRelativePath, oTest::PATH_TYPE _PathType, bool _FileMustExist) const
{
	const char* root = nullptr;
	switch (_PathType)
	{
		case oTest::EXECUTABLES: root = Desc.ExecutablesPath; break;
		case oTest::DATA: root = Desc.DataPath; break;
		case oTest::GOLDEN_BINARIES: root = Desc.GoldenBinariesPath; break;
		case oTest::GOLDEN_IMAGES: root = Desc.GoldenImagesPath;	break;
		case oTest::TEMP: root = Desc.TempPath; break;
		case oTest::INPUT: root = Desc.InputPath; break;
		case oTest::OUTPUT: root = Desc.OutputPath; break;
		oNODEFAULT;
	}

	oStringPath RawPath(root);
	oEnsureSeparator(RawPath.c_str());
	strcat_s(RawPath, _StrRelativePath);
	if (!oCleanPath(_StrFullPath, _SizeofStrFullPath, RawPath))
		return oErrorSetLast(oERROR_INVALID_PARAMETER);

	if (_FileMustExist && !oFileExists(_StrFullPath))
		return oErrorSetLast(oERROR_NOT_FOUND, "not found: %s", _StrFullPath);
	return true;
}
