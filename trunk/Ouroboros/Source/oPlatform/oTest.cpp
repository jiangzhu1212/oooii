/**************************************************************************
 * The MIT License                                                        *
 * Copyright (c) 2013 Antony Arciuolo.                                    *
 * arciuolo@gmail.com                                                     *
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
#include <oBasis/oLockedPointer.h>
#include <oBasis/oRef.h>
#include <oStd/scc.h>

#include <oConcurrency/event.h>

// Wouldn't it be nice if oTesting logic were oBasis so it can be used with
// that lib too (without shims)...
#include <oPlatform/oReporting.h>
#include <oPlatform/oConsole.h>
#include <oPlatform/oCPU.h> // only used to print (TRACE) out CPU stats
#include <oPlatform/oDebugger.h> // needed to ensure leak tracker is up before oTestManager
#include <oPlatform/oDisplay.h> // to print out driver versions
#include <oPlatform/oInterprocessEvent.h> // inter-process event required to sync "special tests" as they launch a new process
#include <oPlatform/oFile.h> // needed for oFileExists... this could be passed through as an oFUNCTION
#include <oPlatform/oImage.h> // the crux of most tests... going to be hard to get rid of this dependency
#include <oPlatform/oModule.h> // For oModuleGetDesc
#include <oPlatform/oMsgBox.h> // only used to notify about zombies
#include <oPlatform/oProgressBar.h> // only really so it itself can be tested, but perhaps this can be moved to a unit test?
#include <oPlatform/oProcess.h> // used to launch special tests
#include <oPlatform/oStandards.h> // standard colors for a console app, maybe this can be callouts? log file path... can be an option?
#include <oPlatform/oStreamUtil.h> // used for loading buffers
#include <oPlatform/oSystem.h> // used for getting various paths
#include <algorithm>
#include <unordered_map>

// Some well-known apps cause grief in the unit tests (including older versions
// of the unit test exe), so prompt, shout, and kill where possible to ensure
// there are no false-positive failures due to an unclean system. If _PromptUser
// is false, this will assume the user wants to terminate all processes.
static bool oTestTerminateInterferingProcesses(bool _PromptUser = true)
{
	// @oooii-tony: This should turn into a custom dialog box that contains a 
	// listbox with a list of all matched processes that could cause problems and
	// have buttons for terminating selected, or terminating all. Also, it should
	// refresh in case the user decided to terminate the processes external to the 
	// dialog. Perhaps this should also take the list of processes to kill if this
	// API gets exposed further.

	static const char* sExternalProcessNames[] = 
	{
		"fraps.exe",
		"fraps64.dat",
		"FahCore_a4.exe",
	};

	oStd::fixed_vector<unsigned int, oCOUNTOF(sExternalProcessNames)> ActiveProcesses;

	oStd::xlstring Message;
	oPrintf(Message, "The following active processes will interfere with tests either by using resources during benchmark tests or by altering image compares.\n");

	oFORI(i, sExternalProcessNames)
	{
		unsigned int PID = oProcessGetID(sExternalProcessNames[i]);
		if (PID)
		{
			oStrAppendf(Message, "%s\n", sExternalProcessNames[i]);
			ActiveProcesses.push_back(PID);
		}
	}

	oStrAppendf(Message, "\nWould you like to terminate these processes now?");

	if (!ActiveProcesses.empty())
	{
		oMSGBOX_DESC mb;
		mb.Title = "Ouroboros Test Environment Discovery";
		mb.Type = oMSGBOX_YESNO;
		oMSGBOX_RESULT r = oMSGBOX_YES;
		if (_PromptUser)
			r = oMsgBox(mb, Message.c_str());

		int retries = 10;
TryAgain:
		if (r == oMSGBOX_YES && retries)
		{
			oFOR(unsigned int PID, ActiveProcesses)
			{
				if (!oProcessTerminate(PID, 0x0D1EC0DE) && oErrorGetLast() != std::errc::no_such_process)
				{
					oStd::path_string Name("(null)");
					if (!oProcessGetName(Name, PID))
						continue; // might've been a child of another process on the list, so it appears it's no longer around, thus continue.

					else if (_PromptUser)
						r = oMsgBox(mb, "Terminating Process '%s' (%u) failed. Please close the process manually.\n\nTry Again?", Name.c_str(), PID);
					else
					{
						if (0 == --retries)
							return oErrorSetLast(std::errc::permission_denied, "Process '%s' (%u) could not be terminated", Name.c_str(), PID);
					}

					if (r == oMSGBOX_NO)
						return oErrorSetLast(std::errc::operation_canceled, "Process '%s' (%u) could not be terminated and the user elected to continue", Name.c_str(), PID);

					goto TryAgain;
				}
			}
		}
	}

	return true;
}

static bool oTestNotifyOfAntiVirus(bool _PromptUser)
{
	static const char* sAntiVirusProcesses[] = 
	{
		"avgcsrva.exe",
		"avgemca.exe",
		"avgidsagent.exe",
		"avgnsa.exe",
		"avgrsa.exe",
		"avggui.exe",
		"avgwdsvc.exe",

		// @oooii-tony: TODO: Add more AntiVirus processes
	};

	unsigned int AVPID = 0;
	oFORI(i, sAntiVirusProcesses)
	{
		AVPID = oProcessGetID(sAntiVirusProcesses[i]);
		if (AVPID)
			break;
	}

	if (AVPID)
	{
		oMSGBOX_DESC mb;
		mb.Title = "Ouroboros Test Environment Discovery";
		mb.Type = oMSGBOX_YESNO;
		mb.TimeoutMS = 10000;
		oMSGBOX_RESULT r = oMSGBOX_YES;
		if (_PromptUser)
			r = oMsgBox(mb, "An Anti-Virus program was detected. This can severely impact the time taken on some tests of large buffers. Do you want to continue?");

		if (r != oMSGBOX_YES)
			return oErrorSetLast(std::errc::permission_denied);
	}

	return true;
}

namespace oStd {

const char* as_string(const oTest::RESULT& _Result)
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
		"PERFTEST",
	};
	static_assert(oTest::NUM_TEST_RESULTS == oCOUNTOF(sStrings), "");
	return sStrings[_Result];
}

} // namespace oStd

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

	inline void Report(oConsoleReporting::REPORT_TYPE _Type, const char* _Format, ...) { va_list args; va_start(args, _Format); oConsoleReporting::VReport(_Type, _Format, args); va_end(args); }
	inline void ReportSep() { Report(oConsoleReporting::DEFAULT, "%c ", 179); }

	// Returns the number of tests that will run based on num iterations and 
	// discounting filtered tests.
	size_t CalculateNumTests(const oTestManager::DESC& _Desc, threadsafe oFilterChain* _pFilterChain);

	typedef std::vector<RegisterTestBase*> tests_t;
	tests_t Tests;
	DESC Desc;
	oStd::fixed_vector<oDISPLAY_ADAPTER_DRIVER_DESC, 8> DriverDescs;
	bool ShowProgressBar;
	std::string TestSuiteName;
	std::string DataPath;
	std::string ExecutablesPath;
	std::string GoldenBinariesPath;
	std::string GoldenImagesPath;
	oStd::path_string TempPath;
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
oSINGLETON_REGISTER(oTestManagerImplSingleton);

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

const oTest::FileMustExistFlag oTest::FileMustExist;

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
	oStd::path_string base;
	if (_Path && *_Path)
		base = _Path;
	else if (_DataSubpath && *_DataSubpath)
		oPrintf(base, "%s/%s", _DataPath, _DataSubpath);
	else
		oPrintf(base, "%s", _DataPath);

	if (_NthItem)
		oPrintf(_StrDestination, _SizeofStrDestination, "%s/%s%u%s", base.c_str(), _TestName, _NthItem, _Ext);
	else
		oPrintf(_StrDestination, _SizeofStrDestination, "%s/%s%s", base.c_str(), _TestName, _Ext);

	oCleanPath(_StrDestination, _SizeofStrDestination, _StrDestination);
}

template<size_t size> void BuildDataPath(char (&_StrDestination)[size], const char* _TestName, const char* _DataPath, const char* _DataSubpath, const char* _Path, unsigned int _NthItem, const char* _Ext) { BuildDataPath(_StrDestination, size, _TestName, _DataPath, _DataSubpath, _Path, _NthItem, _Ext); }

bool oTest::TestBinary(const void* _pBuffer, size_t _SizeofBuffer, const char* _FileExtension, unsigned int _NthBinary)
{
	oTestManager::DESC desc;
	oTestManager::Singleton()->GetDesc(&desc);
	oGPU_VENDOR Vendor = static_cast<oTestManager_Impl*>(oTestManager::Singleton())->DriverDescs[0].Vendor; // @oooii-tony: Make this more elegant

	oStd::path_string golden;
	BuildDataPath(golden.c_str(), GetName(), desc.DataPath, "GoldenBinaries", desc.GoldenBinariesPath, _NthBinary, _FileExtension);
	oStd::path_string goldenAMD;
	char ext[32];
	oPrintf(ext, "_AMD%s", _FileExtension);
	BuildDataPath(goldenAMD.c_str(), GetName(), desc.DataPath, "GoldenBinaries", desc.GoldenBinariesPath, _NthBinary, ext);
	oStd::path_string output;
	BuildDataPath(output.c_str(), GetName(), desc.DataPath, "Output", desc.OutputPath, _NthBinary, _FileExtension);

	bool bSaveTestBuffer = false;
	oStd::finally SaveTestBuffer([&]
	{
		if (bSaveTestBuffer)
		{
			if (!oFileSave(output, _pBuffer, _SizeofBuffer, false))
				oErrorSetLast(std::errc::invalid_argument, "Output binary save failed: %s", output.c_str());
		}
	});

	oRef<oBuffer> GoldenBinary;
	{
		if (Vendor == oGPU_VENDOR_AMD)
		{
			// Try to load a more-specific golden binary, but if it's not there it's
			// ok to try to use the default one.
			oBufferLoad(goldenAMD, &GoldenBinary);
		}

		if (!GoldenBinary)
		{
			if (!oBufferLoad(golden, &GoldenBinary))
			{
				if (Vendor != oGPU_VENDOR_NVIDIA)
				{
					oStd::path_string outputAMD;
					char ext[32];
					oPrintf(ext, "_AMD%s", _FileExtension);
					BuildDataPath(outputAMD.c_str(), GetName(), desc.DataPath, "Output", desc.OutputPath, _NthBinary, ext);
					oTRACE("WARNING: Shared Golden Images are only valid if generated from an NVIDIA card. Note: it may be appropriate to check this in as an AMD-specific card to %s if there's a difference in golden images between NVIDIA and AMD.", outputAMD.c_str());
				}

				bSaveTestBuffer = true;
				return oErrorSetLast(std::errc::io_error, "Golden binary load failed: %s", golden.c_str());
			}
		}
	}

	if (_SizeofBuffer != GoldenBinary->GetSize())
	{
		oStd::sstring testSize, goldenSize;
		bSaveTestBuffer = true;
		return oErrorSetLast(std::errc::protocol_error, "Golden binary compare failed because the binaries are different sizes (test is %s, golden is %s)", oFormatMemorySize(testSize, _SizeofBuffer, 2), oFormatMemorySize(goldenSize, GoldenBinary->GetSize(), 2));
	}

	if (memcmp(_pBuffer, GoldenBinary->GetData(), GoldenBinary->GetSize()))
	{
		bSaveTestBuffer = true;
		return oErrorSetLast(std::errc::protocol_error, "Golden binary compare failed because the bytes differ");
	}

	return true;
}

bool oTest::TestImage(oImage* _pTestImage, const char* _GoldenImagePath, const char* _FailedImagePath, unsigned int _NthImage, int _ColorChannelTolerance, float _MaxRMSError, unsigned int _DiffImageMultiplier, bool _OutputGoldenImage)
{
	size_t commonPathLength = oGetCommonBaseLength(_GoldenImagePath, _FailedImagePath);
	const char* gPath = _GoldenImagePath + commonPathLength;
	const char* fPath = _FailedImagePath + commonPathLength;

	oImage::DESC iDesc;
	_pTestImage->GetDesc(&iDesc);

	oRef<oImage> GoldenImage;
	{
		oRef<oBuffer> b;
		if (!oBufferLoad(_GoldenImagePath, &b))
			return oErrorSetLast(std::errc::io_error, "Load failed: (Golden)...%s", gPath);

		if (!oImageCreate(_GoldenImagePath, b->GetData(), b->GetSize(), &GoldenImage))
			return oErrorSetLast(std::errc::protocol_error, "Corrupt Image: (Golden)...%s", gPath);

		bool success = false;
		if (oImageIsAlphaFormat(iDesc.Format))
			success = oImageCreate(_GoldenImagePath, b->GetData(), b->GetSize(), oImage::FORCE_ALPHA, &GoldenImage);
		else
			success = oImageCreate(_GoldenImagePath, b->GetData(), b->GetSize(), &GoldenImage);

		if (!success)
			return oErrorSetLast(std::errc::protocol_error, "Corrupt Image: (Golden)...%s", gPath);
	}

	oImage::DESC gDesc;
	GoldenImage->GetDesc(&gDesc);

	// Compare dimensions/format before going into pixels
	{
		if (iDesc.Dimensions != gDesc.Dimensions)
		{
			if (!oImageSave(_pTestImage, oImageIsAlphaFormat(iDesc.Format) ? oImage::FORCE_ALPHA : oImage::FORCE_NO_ALPHA, _FailedImagePath))
				return oErrorSetLast(std::errc::io_error, "Save failed: (Output)...%s", fPath);
			return oErrorSetLast(std::errc::protocol_error, "Differing dimensions: (Output %dx%d)...%s != (Golden %dx%d)...%s", iDesc.Dimensions.x, iDesc.Dimensions.y, fPath, gDesc.Dimensions.x, gDesc.Dimensions.y, gPath);
		}

		if (iDesc.Format != gDesc.Format)
		{
			if (!oImageSave(_pTestImage, oImageIsAlphaFormat(iDesc.Format) ? oImage::FORCE_ALPHA : oImage::FORCE_NO_ALPHA, _FailedImagePath))
				return oErrorSetLast(std::errc::io_error, "Save failed: (Output)...%s", fPath);
			return oErrorSetLast(std::errc::protocol_error, "Differing formats: (Golden %s)...%s != (Output %s)...%s", oStd::as_string(gDesc.Format), gPath, oStd::as_string(iDesc.Format), fPath);
		}
	}

	// Resolve parameter settings against global settings
	{
		oTestManager::DESC TestDesc;
		oTestManager::Singleton()->GetDesc(&TestDesc);

		if (_ColorChannelTolerance == oDEFAULT)
			_ColorChannelTolerance = TestDesc.ColorChannelTolerance;
		if (_MaxRMSError < 0.0f)
			_MaxRMSError = TestDesc.MaxRMSError;
		if (_DiffImageMultiplier == oDEFAULT)
			_DiffImageMultiplier = TestDesc.DiffImageMultiplier;
	}

	// Do the real test
	oRef<oImage> diffs;
	float RMSError = 0.0f;
	bool compareSucceeded = oImageCompare(_pTestImage, GoldenImage, _ColorChannelTolerance, &RMSError, &diffs, _DiffImageMultiplier);

	// Save out test image and diffs if there is a non-similar result.
	if (!compareSucceeded || (RMSError > _MaxRMSError))
	{
		if (!oImageSave(_pTestImage, oImageIsAlphaFormat(iDesc.Format) ? oImage::FORCE_ALPHA : oImage::FORCE_NO_ALPHA , _FailedImagePath))
			return oErrorSetLast(std::errc::io_error, "Save failed: (Output)...%s", fPath);

		oStd::path_string diffPath(_FailedImagePath);
		oReplaceFileExtension(diffPath, diffPath.capacity(), "_diff.png");
		const char* dPath = diffPath.c_str() + commonPathLength;

		if (diffs && !oImageSave(diffs, oImageIsAlphaFormat(iDesc.Format) ? oImage::FORCE_ALPHA : oImage::FORCE_NO_ALPHA, diffPath))
			return oErrorSetLast(std::errc::io_error, "Save failed: (Diff)...%s", dPath);

		if (_OutputGoldenImage)
		{
			oStd::path_string goldenPath(_FailedImagePath);
			oReplaceFileExtension(goldenPath, goldenPath.capacity(), "_golden.png");
			const char* gPath = goldenPath.c_str() + commonPathLength;

			if (GoldenImage && !oImageSave(GoldenImage, oImageIsAlphaFormat(iDesc.Format) ? oImage::FORCE_ALPHA : oImage::FORCE_NO_ALPHA, goldenPath))
				return oErrorSetLast(std::errc::io_error, "Save failed: (Golden)...%s", gPath);
		}

		return oErrorSetLast(std::errc::protocol_error, "Compare failed: %.03f RMS error (threshold %.03f): (Output)...%s != (Golden)...%s", RMSError, _MaxRMSError, fPath, gPath);
	}

	return true;
}

struct DriverPaths
{
	oStd::path_string Generic;
	oStd::path_string VendorSpecific;
	oStd::path_string CardSpecific;
	oStd::path_string DriverSpecific;
};

static bool oInitialize(const char* _RootPath, const char* _Filename, const oDISPLAY_ADAPTER_DRIVER_DESC& _DriverDesc, DriverPaths* _pDriverPaths)
{
	_pDriverPaths->Generic = _RootPath;
	oEnsureSeparator(_pDriverPaths->Generic);
	oCleanPath(_pDriverPaths->Generic, _pDriverPaths->Generic);

	oPrintf(_pDriverPaths->VendorSpecific, "%s%s/", _pDriverPaths->Generic.c_str(), oStd::as_string(_DriverDesc.Vendor));

	oStd::path_string tmp;
	oPrintf(tmp, "%s%s/", _pDriverPaths->VendorSpecific.c_str(), _DriverDesc.Description.c_str());
	oReplace(_pDriverPaths->CardSpecific, tmp, " ", "_");

	oStd::sstring driverVer;
	oPrintf(_pDriverPaths->DriverSpecific, "%s%s/", _pDriverPaths->CardSpecific.c_str(), oStd::to_string(driverVer, _DriverDesc.Version));

	oStrAppendf(_pDriverPaths->Generic, _Filename);
	oStrAppendf(_pDriverPaths->VendorSpecific, _Filename);
	oStrAppendf(_pDriverPaths->CardSpecific, _Filename);
	oStrAppendf(_pDriverPaths->DriverSpecific, _Filename);

	return true;
}

bool oTest::TestImage(oImage* _pTestImage, unsigned int _NthImage, int _ColorChannelTolerance, float _MaxRMSError, unsigned int _DiffImageMultiplier)
{
	// Check: GoldenDir/CardName/DriverVersion/GoldenImage
	// Then Check: GoldenDir/CardName/GoldenImage
	// Then Check: GoldenDir/GoldenImage

	oTestManager::DESC TestDesc;
	oTestManager::Singleton()->GetDesc(&TestDesc);

	const oDISPLAY_ADAPTER_DRIVER_DESC& DriverDesc = static_cast<oTestManager_Impl*>(oTestManager::Singleton())->DriverDescs[0]; // @oooii-tony: Make this more elegant

	oStd::sstring nthImageBuf;
	oStd::path_string Filename;
	oPrintf(Filename, "%s%s.png", GetName(), _NthImage == 0 ? "" : oStd::to_string(nthImageBuf, _NthImage));

	DriverPaths GoldenPaths, FailurePaths;
	oVERIFY(oInitialize(TestDesc.GoldenImagesPath, Filename, DriverDesc, &GoldenPaths));
	oVERIFY(oInitialize(TestDesc.OutputPath, Filename, DriverDesc, &FailurePaths));

	// @oooii-tony: Use oStream's search path facility to simplify this and all oTest
	// search path evaluation.

	if (oStreamExists(GoldenPaths.DriverSpecific))
		return TestImage(_pTestImage, GoldenPaths.DriverSpecific, FailurePaths.DriverSpecific, _NthImage, _ColorChannelTolerance, _MaxRMSError, _DiffImageMultiplier, TestDesc.EnableOutputGoldenImages);

	else if (oStreamExists(GoldenPaths.VendorSpecific))
		return TestImage(_pTestImage, GoldenPaths.VendorSpecific, FailurePaths.VendorSpecific, _NthImage, _ColorChannelTolerance, _MaxRMSError, _DiffImageMultiplier, TestDesc.EnableOutputGoldenImages);

	else if (oStreamExists(GoldenPaths.CardSpecific))
		return TestImage(_pTestImage, GoldenPaths.CardSpecific, FailurePaths.CardSpecific, _NthImage, _ColorChannelTolerance, _MaxRMSError, _DiffImageMultiplier, TestDesc.EnableOutputGoldenImages);

	else if (oStreamExists(GoldenPaths.Generic))
		return TestImage(_pTestImage, GoldenPaths.Generic, FailurePaths.Generic, _NthImage, _ColorChannelTolerance, _MaxRMSError, _DiffImageMultiplier, TestDesc.EnableOutputGoldenImages);

	oImage::DESC iDesc;
	_pTestImage->GetDesc(&iDesc);

	if (!oImageSave(_pTestImage, oImageIsAlphaFormat(iDesc.Format) ? oImage::FORCE_ALPHA : oImage::FORCE_NO_ALPHA, FailurePaths.DriverSpecific))
		return oErrorSetLast(std::errc::io_error, "Save failed: (Output)%s", FailurePaths.DriverSpecific.c_str());

	return oErrorSetLast(std::errc::no_such_file_or_directory, "Not found: (Golden).../%s Test Image saved to %s", Filename, FailurePaths.DriverSpecific.c_str());
}

bool oSpecialTest::CreateProcess(const char* _SpecialTestName, threadsafe oProcess** _ppProcess)
{
	if (!_SpecialTestName || !_ppProcess)
		return oErrorSetLast(std::errc::invalid_argument);

	oStd::xlstring cmdline;
	if (!oSystemGetPath(cmdline.c_str(), oSYSPATH_APP_FULL))
		return oErrorSetLast(std::errc::no_such_file_or_directory);

	oPrintf(cmdline, "%s -s %s", cmdline.c_str(), _SpecialTestName);
	oProcess::DESC desc;
	desc.CommandLine = cmdline;
	desc.EnvironmentString = 0;
	desc.StdHandleBufferSize = 64 * 1024;
	return oProcessCreate(desc, _ppProcess);
}

//#define DEBUG_SPECIAL_TEST

bool oSpecialTest::Start(threadsafe interface oProcess* _pProcess, char* _StrStatus, size_t _SizeofStrStatus, int* _pExitCode, unsigned int _TimeoutMS)
{
	if (!_pProcess || !_StrStatus || !_pExitCode)
		return oErrorSetLast(std::errc::invalid_argument);

	oProcess::DESC desc;
	#ifdef DEBUG_SPECIAL_TEST
		desc.StartSuspended = true;
	#endif
	_pProcess->GetDesc(&desc);
	const char* SpecialTestName = oStrStrReverse(desc.CommandLine, "-s ") + 3;
	if (!SpecialTestName || !*SpecialTestName)
		return oErrorSetLast(std::errc::invalid_argument, "Process with an invalid command line for oSpecialTest specified.");

	oStd::mstring interprocessName;
	oPrintf(interprocessName, "oTest.%s.Started", SpecialTestName);
	oInterprocessEvent Started(interprocessName);
	oASSERTA(!Started.Wait(0), "Started event set when it shouldn't be (before start).");
	#ifdef DEBUG_SPECIAL_TEST
		_pProcess->Start();
	#endif

	oTestManager::DESC testingDesc;
	oTestManager::Singleton()->GetDesc(&testingDesc);

	if (testingDesc.EnableSpecialTestTimeouts)
	{
		if (!Started.Wait(_TimeoutMS))
		{
			oPrintf(_StrStatus, _SizeofStrStatus, "Timed out waiting for %s to start.", SpecialTestName);
			oTRACE("*** SPECIAL MODE UNIT TEST %s timed out waiting for Started event. (Ensure the special mode test sets the started event when appropriate.) ***", SpecialTestName);
			if (!_pProcess->GetExitCode(_pExitCode))
				_pProcess->Kill(std::errc::timed_out);

			oPrintf(_StrStatus, _SizeofStrStatus, "Special Mode %s timed out on start.", SpecialTestName);
			return false;
		}
	}

	else
		Started.Wait();

	// If we timeout on ending, that's good, it means the app is still running
	if ((_pProcess->Wait(200) && _pProcess->GetExitCode(_pExitCode)))
	{
		oStd::xlstring msg;
		size_t bytes = _pProcess->ReadFromStdout(msg.c_str(), msg.capacity());
		msg[bytes] = 0;
		if (bytes)
			oPrintf(_StrStatus, _SizeofStrStatus, "%s: %s", SpecialTestName, msg.c_str());
		return false;
	}

	return true;
}

void oSpecialTest::NotifyReady()
{
	oStd::mstring interprocessName;
	const char* testName = oGetTypename(typeid(*this).name());
	oPrintf(interprocessName, "oTest.%s.Started", testName);
	oInterprocessEvent Ready(interprocessName);
	oASSERTA(!Ready.Wait(0), "Ready event set when it shouldn't be (in NotifyReady).");
	Ready.Set();
}

oTestManager::RegisterTestBase::RegisterTestBase(unsigned int _BugNumber, oTest::RESULT _BugResult, const char* _PotentialZombieProcesses)
	: BugNumber(_BugNumber)
	, BugResult(_BugResult)
{
	oStrcpy(PotentialZombieProcesses, _PotentialZombieProcesses);
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
	if (oSTRVALID(_pDesc->TestSuiteName))
		TestSuiteName = _pDesc->TestSuiteName;
	else
	{
		oMODULE_DESC md;
		oModuleGetDesc(&md);
		oStd::sstring Ver;
		TestSuiteName = md.ProductName;
		TestSuiteName += " ";
		TestSuiteName += oStd::to_string(Ver, md.ProductVersion);
	}

	oStd::path_string defaultDataPath;
	oSystemGetPath(defaultDataPath.c_str(), oSYSPATH_DATA);

	DataPath = _pDesc->DataPath ? _pDesc->DataPath : defaultDataPath;
	if (_pDesc->ExecutablesPath)
		ExecutablesPath = _pDesc->ExecutablesPath;
	else
	{
		oStd::path_string exes(defaultDataPath);
		#ifdef o64BIT
			oStrcat(exes.c_str(), "../bin/x64/");
		#elif o32BIT
			oStrcat(exes.c_str(), "../bin/win32/");
		#else
			#error Unknown bitness
		#endif
		ExecutablesPath = oCleanPath(exes.c_str(), exes);
	}

	GoldenBinariesPath = _pDesc->GoldenBinariesPath ? _pDesc->GoldenBinariesPath : (DataPath + "GoldenBinaries/");
	GoldenImagesPath = _pDesc->GoldenImagesPath ? _pDesc->GoldenImagesPath : (DataPath + "GoldenImages/");
	oSystemGetPath(TempPath, oSYSPATH_TESTTMP);
	InputPath = _pDesc->InputPath ? _pDesc->InputPath : DataPath;
	OutputPath = _pDesc->OutputPath ? _pDesc->OutputPath : (DataPath + "FailedImageCompares/");

	#ifdef o64BIT
		OutputPath += "x64/";
	#else
		OutputPath += "win32/";
	#endif

	#ifdef _DEBUG
		OutputPath += "Debug/";
	#else
		OutputPath += "Release/";
	#endif

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
	oStd::path_string cwd;
	oSystemGetPath(cwd.c_str(), oSYSPATH_CWD);
	oStd::path_string datapath;
	oCleanPath(datapath.c_str(), Desc.DataPath);
	oEnsureSeparator(datapath.c_str());
	bool dataPathIsCWD = !oStricmp(cwd, datapath);

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

	for (unsigned int i = 0; i < DriverDescs.size(); i++)
		Report(oConsoleReporting::INFO, "Video Driver %u: %s v%d.%d\n", i, DriverDescs[i].Description.c_str(), DriverDescs[i].Version.Major, DriverDescs[i].Version.Minor);

	auto scc = oStd::make_scc(oStd::scc_protocol::svn, oBIND(oSystemExecute, oBIND1, oBIND2, oBIND3, oBIND4, oBIND5, false));

	oStd::path_string DevPath;
	oVERIFY(oSystemGetPath(DevPath, oSYSPATH_DEV));
	oStd::lstring CLStr;
	uint CL = scc->revision(DevPath);
	if (CL)
	{
		oStd::scc_file f;
		if (0 == scc->modifications(DevPath, CL, &f, 1))
			oPrintf(CLStr, "%d", CL);
		else
			oPrintf(CLStr, "%d + modifications", CL);
	}
	else
		oPrintf(CLStr, "???");

	Report(oConsoleReporting::INFO, "Perforce Changelist: %s\n", CLStr.c_str());
}

void oTestManager_Impl::RegisterSpecialModeTests()
{
	oFOR(auto pRTB, Tests)
	{
		if (!pRTB->IsSpecialTest())
			continue;

		const char* Name = oGetTypename(pRTB->GetTypename());
		oASSERT(SpecialModes[Name] == 0, "%s already registered", Name);
		SpecialModes[Name] = pRTB;
	}
}

void oTestManager_Impl::RegisterZombies()
{
	oFOR(auto pRTB, Tests)
	{
		const char* TestPotentialZombies = pRTB->GetPotentialZombieProcesses();
		if (TestPotentialZombies && *TestPotentialZombies)
		{
			char* ctx = 0;
			char* z = oStrTok(TestPotentialZombies, ";", &ctx);
			while (z)
			{
				oStd::push_back_unique(PotentialZombies, std::string(z));
				std::string DebugName = z;
				DebugName += oMODULE_DEBUG_SUFFIX_A;
				oStd::push_back_unique(PotentialZombies, DebugName);
				z = oStrTok(nullptr, ";", &ctx);
			}
		}
	}
}

static bool FindDuplicateProcessInstanceByName(unsigned int _ProcessID, unsigned int _ParentProcessID, const char* _ProcessExePath, unsigned int _IgnorePID, const char* _FindName, unsigned int* _pOutPIDs, size_t _NumOutPIDs, size_t* _pCurrentCount)
{
	if (_IgnorePID != _ProcessID && !oStricmp(_FindName, _ProcessExePath))
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

		oProcessTerminate(pids[i], std::errc::operation_canceled);
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
	oFOR(const auto& z, PotentialZombies)
		if (!KillZombies(z.c_str()))
			return false;
	return true;
}

oTest::RESULT oTestManager_Impl::RunTest(RegisterTestBase* _pRegisterTestBase, char* _StatusMessage, size_t _SizeofStatusMessage)
{
	if (!KillZombies())
	{
		oPrintf(_StatusMessage, _SizeofStatusMessage, "oTest infrastructure could not kill zombie process");
		return oTest::FAILURE;
	}

	*_StatusMessage = 0;

	srand(Desc.RandomSeed);

	if (!oStreamDelete(TempPath) && oErrorGetLast() != std::errc::no_such_file_or_directory)
		oVERIFY(false && "oStreamDelete(TempPath))");

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
		oPrintf(_StatusMessage, _SizeofStatusMessage, "Leaks (see debug log for full report)");
	}

	return result;
}

size_t oTestManager_Impl::CalculateNumTests(const oTestManager::DESC& _Desc, threadsafe oFilterChain* _pFilterChain)
{
	size_t nTests = 0;
	oFOR(auto pRTB, Tests)
	{
		if (pRTB && !pRTB->IsSpecialTest())
		{
			oStd::mstring TestName;
			TestName = oGetTypename(pRTB->GetTypename());
			
			if (!_pFilterChain || _pFilterChain->Passes(TestName, 0))
				nTests++;
		}
	}
	
	return _Desc.NumRunIterations * nTests;
}

static bool SortAlphabetically(oTestManager::RegisterTestBase* _Test1, oTestManager::RegisterTestBase* _Test2)
{
	return oStricmp(_Test1->GetTypename(), _Test2->GetTypename()) < 0;
}

static void oTraceCPUFeatures()
{
	// @oooii-tony: This and the header that is currently printed out is becoming 
	// large... we should move it somewhere?

	oCPU_DESC cpud;
	if (oCPUGetDesc(&cpud))
		oTRACE("CPU: %s. %d Processors, %d HWThreads", cpud.BrandString, cpud.NumProcessors, cpud.NumHardwareThreads);

	int i = 0;
	const char* s = oCPUEnumFeatures(i++);
	while (s)
	{
		oStd::mstring buf;
		oPrintf(buf, " CPU Feature %s has %s", s, oStd::as_string(oCPUCheckFeatureSupport(s)));
		oTRACE("%s", buf.c_str());
		s = oCPUEnumFeatures(i++);
	}
}

oTest::RESULT oTestManager_Impl::RunTests(oFilterChain::FILTER* _pTestFilters, size_t _SizeofTestFilters)
{
	if (!oDisplayAdapterIsUpToDate())
	{
		oMSGBOX_DESC mb;
		mb.Type = oMSGBOX_YESNO;
		if (oMSGBOX_YES != oMsgBox(mb, "Display adapter drivers are out-of-date. Do you want to continue?"))
			return oTest::FAILURE;
	}

	oTraceCPUFeatures();

	if (!oTestTerminateInterferingProcesses(!Desc.AutomatedMode) && oErrorGetLast() == std::errc::permission_denied)
	{
		Report(oConsoleReporting::ERR, "%s\n", oErrorGetLastString());
		Report(oConsoleReporting::ERR, "========== Unit Tests: ERROR NO TESTS RUN (Interfering Processes) ==========\n");
		return oTest::FAILURE;
	}

	if (Desc.Exhaustive)
	{
		if (!oTestNotifyOfAntiVirus(!Desc.AutomatedMode) && oErrorGetLast() == std::errc::permission_denied)
		{
			Report(oConsoleReporting::ERR, "%s\n", oErrorGetLastString());
			Report(oConsoleReporting::ERR, "========== Unit Tests: ERROR NO TESTS RUN (AntiVirus) ==========\n");
			return oTest::FAILURE;
		}
	}

	RegisterZombies();

	RegisterSpecialModeTests();

	// Ensure - regardless of linkage order - that tests appear in alphabetical order
	std::sort(Tests.begin(), Tests.end(), SortAlphabetically);

	DriverDescs.clear();
	oDisplayAdapterEnum([&](unsigned int _AdapterIndex, const oDISPLAY_ADAPTER_DRIVER_DESC& _DriverDesc)->bool
	{
		if (_AdapterIndex > oCOUNTOF(DriverDescs))
		{
			oTRACE("WARNING: There are more GPUs attached to the system than we have storage for! Only holding information for the first %d", oCOUNTOF(DriverDescs));
			return false;
		}
		else
			DriverDescs.push_back(_DriverDesc);
		return true;
	});

	size_t TotalNumSucceeded = 0;
	size_t TotalNumFailed = 0;
	size_t TotalNumLeaks = 0;
	size_t TotalNumSkipped = 0;

	// This can't be set in the ctor because the ctor could happen at static init
	// time and the underlying code relies on a statically compiled regex. If that
	// regex weren't statically compiled, it would either be slow or could report
	// as a leak if it were a function-local static.
	char fcErr[1024];
	bool fcSuccess = false;
	oFilterChain filterChain(_pTestFilters, _SizeofTestFilters, fcErr, oCOUNTOF(fcErr), &fcSuccess);

	// @oooii-tony: For testing progress bar functionality. If we like this as
	// a feature of the unit test, we should expose it through the DESC.
	//ShowProgressBar = true;

	oStd::thread ProgressBarThread;
	bool ShouldStop = false;
	oConcurrency::event Ready;
	threadsafe oProgressBar* pProgressBar = nullptr;
	oStd::finally StopProgressBar([&]
	{
		if (ShowProgressBar)
			Ready.wait();
		
		if (pProgressBar)
			pProgressBar->Quit();
		
		ProgressBarThread.join();
	});
	
	if (ShowProgressBar)
	{
		ProgressBarThread = std::move(oStd::thread([&]
		{
			oConcurrency::begin_thread("Progress Bar Thread");
			oStd::xlstring title;
			oConsole::GetTitle(title);
			oRef<oProgressBar> ProgressBar;
			oVERIFY(oProgressBarCreate([&] { ShouldStop = true; }, title, &ProgressBar));
			pProgressBar = ProgressBar;
			Ready.set();
			ProgressBar->FlushMessages(true);
			pProgressBar = nullptr;
			oConcurrency::end_thread();
		}));

		Ready.wait();
	}

	size_t ProgressTotalNumTests = 0;
	size_t ProgressNumTestsSoFar = 0;
	if (ShowProgressBar)
	{
		pProgressBar->SetText("Calculating the number of tests...");
		pProgressBar->Restore();

		ProgressTotalNumTests = CalculateNumTests(Desc, &filterChain);
		if (ShouldStop)
		{
			oTRACE("ProgressBar Stop Pressed, aborting.");
			return oTest::FAILURE;
		}
	}

	oCRTLeakTracker::Singleton()->Enable(Desc.EnableLeakTracking);
	oCRTLeakTracker::Singleton()->CaptureCallstack(Desc.CaptureCallstackForTestLeaks);

	oStd::xlstring timeMessage;
	double allIterationsStartTime = oTimer();
	for (size_t r = 0; r < Desc.NumRunIterations; r++)
	{
		size_t Count[oTest::NUM_TEST_RESULTS];
		memset(Count, 0, sizeof(size_t) * oTest::NUM_TEST_RESULTS);

		// Prepare formatting used to print results
		oStd::sstring nameSpec;
		oPrintf(nameSpec, "%%-%us", Desc.NameColumnWidth);

		oStd::sstring statusSpec;
		oPrintf(statusSpec, "%%-%us", Desc.StatusColumnWidth);

		oStd::sstring timeSpec;
		oPrintf(timeSpec, "%%-%us", Desc.TimeColumnWidth);

		oStd::sstring messageSpec;
		oPrintf(messageSpec, "%%s\n");

		oStd::xlstring statusMessage;

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
		oFOR(auto pRTB, Tests)
		{
			if (pRTB && !pRTB->IsSpecialTest())
			{
				oStd::mstring TestName;
				oStrcpy(TestName, oGetTypename(pRTB->GetTypename()));
				
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
							if (ShouldStop)
								break;

							pProgressBar->SetText(TestName);
						}

						oTRACE("========== Begin %s Run %u ==========", TestName.c_str(), r+1);
						double testStart = oTimer();
						result = RunTest(pRTB, statusMessage.c_str(), statusMessage.capacity());
						testDuration = oTimer() - testStart;
						oTRACE("========== End %s Run %u (%s) ==========", TestName.c_str(), r+1, oStd::as_string(result));
						Count[result]++;

						if (ShowProgressBar)
						{
							ProgressNumTestsSoFar++;
							pProgressBar->SetPercentage(static_cast<int>((100 * (ProgressNumTestsSoFar+1)) / ProgressTotalNumTests));
						}
					}

					else
						result = pRTB->GetBugResult();

					switch (result)
					{
						case oTest::SUCCESS:
							if (!*statusMessage || !oStrcmp("operation was successful", statusMessage))
								oPrintf(statusMessage, "---");
							ReportType = oConsoleReporting::SUCCESS;
							break;

						case oTest::FAILURE:
							if (!*statusMessage)
								oPrintf(statusMessage, "Failed with no test-specific status message");
							ReportType = oConsoleReporting::CRIT;
							break;

						case oTest::SKIPPED:
							if (!*statusMessage)
								oPrintf(statusMessage, "Skipped");
							ReportType = oConsoleReporting::WARN;
							break;

						case oTest::BUGGED:
							oPrintf(statusMessage, "Test disabled. See oBug_%u", pRTB->GetBugNumber());
							ReportType = oConsoleReporting::ERR;
							break;

						case oTest::NOTREADY:
							oPrintf(statusMessage, "Test not yet ready. See oBug_%u", pRTB->GetBugNumber());
							ReportType = oConsoleReporting::INFO;
							break;

						case oTest::LEAKS:
							if (!*statusMessage)
								oPrintf(statusMessage, "Leaks memory");
							ReportType = oConsoleReporting::WARN;
							break;

						case oTest::PERFTEST:
							oPrintf(statusMessage, "Potentially lengthy perf-test not run by default");
							ReportType = oConsoleReporting::WARN;
							break;
					}
				}

				else
				{
					ReportType = oConsoleReporting::WARN;
					Count[oTest::SKIPPED]++;
					oPrintf(statusMessage, "---");
				}

				Report(ReportType, statusSpec, oStd::as_string(result));
				ReportSep();

				ReportType = oConsoleReporting::DEFAULT;
				if (testDuration > Desc.TestReallyTooSlowTimeInSeconds)
					ReportType = oConsoleReporting::ERR;
				else if (testDuration > Desc.TestTooSlowTimeInSeconds)
					ReportType = oConsoleReporting::WARN;

				oStd::format_duration(timeMessage, round(testDuration), true);
				Report(ReportType, timeSpec, timeMessage.c_str());
				ReportSep();
				Report(oConsoleReporting::DEFAULT, messageSpec, statusMessage.c_str());
				::_flushall();
			}
		}

		::_flushall();
		size_t NumSucceeded = Count[oTest::SUCCESS];
		size_t NumFailed = Count[oTest::FAILURE];
		size_t NumLeaks = Count[oTest::LEAKS]; 
		size_t NumSkipped = Count[oTest::SKIPPED] + Count[oTest::FILTERED] + Count[oTest::BUGGED] + Count[oTest::NOTREADY];

		oStd::format_duration(timeMessage, round(oTimer() - totalTestStartTime));
    if ((NumSucceeded + NumFailed + NumLeaks) == 0)
  		Report(oConsoleReporting::ERR, "========== Unit Tests: ERROR NO TESTS RUN ==========\n");
    else
		  Report(oConsoleReporting::INFO, "========== Unit Tests: %u succeeded, %u failed, %u leaked, %u skipped in %s ==========\n", NumSucceeded, NumFailed, NumLeaks, NumSkipped, timeMessage.c_str());

		TotalNumSucceeded += NumSucceeded;
		TotalNumFailed += NumFailed;
		TotalNumLeaks += NumLeaks;
		TotalNumSkipped += NumSkipped;
	}

	if (ShowProgressBar && ShouldStop)
	{
		Report(oConsoleReporting::ERR, "\n\n========== Stopped by user ==========");
		return oTest::FAILURE;
	}
	
	if (Desc.NumRunIterations != 1) // != so we report if somehow a 0 got through to here
	{
		oStd::format_duration(timeMessage, round(oTimer() - allIterationsStartTime));
		Report(oConsoleReporting::INFO, "========== %u Iterations: %u succeeded, %u failed, %u leaked, %u skipped in %s ==========\n", Desc.NumRunIterations, TotalNumSucceeded, TotalNumFailed, TotalNumLeaks, TotalNumSkipped, timeMessage.c_str());
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
		oStd::xlstring statusMessage;
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
			printf("SpecialMode %s: %s", _Name, statusMessage.empty() ? "(no status message)" : statusMessage.c_str());
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

	oStd::path_string RawPath(root);
	oEnsureSeparator(RawPath);
	oStrcat(RawPath, _StrRelativePath);
	if (!oCleanPath(_StrFullPath, _SizeofStrFullPath, RawPath))
		return oErrorSetLast(std::errc::invalid_argument);

	if (_FileMustExist && !oStreamExists(_StrFullPath))
		return oErrorSetLast(std::errc::no_such_file_or_directory, "not found: %s", _StrFullPath);
	return true;
}
