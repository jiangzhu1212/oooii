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
#include <oBase/scc.h>
#include <oCore/process.h>
#include <oCore/windows/win_iocp.h>
#undef CreateProcess

#include <oConcurrency/event.h>

// Wouldn't it be nice if oTesting logic were oBasis so it can be used with
// that lib too (without shims)...
#include <oPlatform/oReporting.h>
#include <oGUI/console.h>
#include <oGUI/oMsgBox.h> // only used to notify about zombies
#include <oGUI/progress_bar.h> // only really so it itself can be tested, but perhaps this can be moved to a unit test?
#include <oPlatform/oStandards.h> // standard colors for a console app, maybe this can be callouts? log file path... can be an option?
#include <oPlatform/oStream.h> // oStreamExists
#include <oPlatform/oStreamUtil.h> // used for loading buffers
#include <oPlatform/Windows/oCRTHeap.h>
#include <algorithm>
#include <unordered_map>

#include <oSurface/codec.h>

using namespace ouro;

// Some well-known apps cause grief in the unit tests (including older versions
// of the unit test exe), so prompt, shout, and kill where possible to ensure
// there are no false-positive failures due to an unclean system. If _PromptUser
// is false, this will assume the user wants to terminate all processes.
static bool oTestTerminateInterferingProcesses(bool _PromptUser = true)
{
	// @tony: This should turn into a custom dialog box that contains a 
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

	fixed_vector<process::id, oCOUNTOF(sExternalProcessNames)> ActiveProcesses;

	xlstring Message;
	snprintf(Message, "The following active processes will interfere with tests either by using resources during benchmark tests or by altering image compares.\n");

	oFORI(i, sExternalProcessNames)
	{
		process::id PID = process::get_id(sExternalProcessNames[i]);
		if (PID)
		{
			sncatf(Message, "%s\n", sExternalProcessNames[i]);
			ActiveProcesses.push_back(PID);
		}
	}

	sncatf(Message, "\nWould you like to terminate these processes now?");

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
			oFOR(auto PID, ActiveProcesses)
			{
				try { process::terminate(PID, 0x0D1EC0DE); }
				catch (std::system_error& e)
				{
					if (e.code().value() != std::errc::no_such_process)
					{
						path Name;
						try { Name = process::get_name(PID); }
						catch (std::exception&)
						{
							continue; // might've been a child of another process on the list, so it appears it's no longer around, thus continue.
						}

						if (_PromptUser)
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
		// TODO: Add more AntiVirus processes
	};

	process::id AVPID;
	oFORI(i, sAntiVirusProcesses)
	{
		AVPID = process::get_id(sAntiVirusProcesses[i]);
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

namespace ouro {

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

} // namespace ouro

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

	bool BuildPath(path& _FullPath, const path& _RelativePath, oTest::PATH_TYPE _PathType, bool _FileMustExist) const;

	inline void Report(oConsoleReporting::REPORT_TYPE _Type, const char* _Format, ...) { va_list args; va_start(args, _Format); oConsoleReporting::VReport(_Type, _Format, args); va_end(args); }
	inline void ReportSep() { Report(oConsoleReporting::DEFAULT, "%c ", 179); }

	// Returns the number of tests that will run based on num iterations and 
	// discounting filtered tests.
	size_t CalculateNumTests(const oTestManager::DESC& _Desc, threadsafe oFilterChain* _pFilterChain);

	typedef std::vector<RegisterTestBase*> tests_t;
	tests_t Tests;
	DESC Desc;
	fixed_vector<adapter::info, 8> DriverDescs;
	bool ShowProgressBar;
	std::string TestSuiteName;
	std::string DataPath;
	std::string ExecutablesPath;
	std::string GoldenBinariesPath;
	std::string GoldenImagesPath;
	path_string TempPath;
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
};

// {97E7D7DD-B3B6-4691-A383-6D9F88C034C6}
const oGUID oTestManagerImplSingleton::GUID = { 0x97e7d7dd, 0xb3b6, 0x4691, { 0xa3, 0x83, 0x6d, 0x9f, 0x88, 0xc0, 0x34, 0xc6 } };
oSINGLETON_REGISTER(oTestManagerImplSingleton);

oTestManagerImplSingleton::oTestManagerImplSingleton()
{
	// oTestManager can be instantiated very early in static init, so make sure 
	// we're tracking memory for it
	//ouro::windows::crt_heap::enable_at_exit_leak_report(true);
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

bool oTest::BuildPath(path& _FullPath, const path& _RelativePath, PATH_TYPE _PathType, bool _FileMustExist) const
{
	return static_cast<oTestManager_Impl*>(oTestManager::Singleton())->BuildPath(_FullPath, _RelativePath, _PathType, _FileMustExist);
}

const char* oTest::GetName() const
{
	return type_name(typeid(*this).name());
}

static void BuildDataPath(char* _StrDestination, size_t _SizeofStrDestination, const char* _TestName, const char* _DataPath, const char* _DataSubpath, const char* _Path, unsigned int _NthItem, const char* _Ext)
{
	path_string base;
	if (_Path && *_Path)
		base = _Path;
	else if (_DataSubpath && *_DataSubpath)
		snprintf(base, "%s/%s", _DataPath, _DataSubpath);
	else
		snprintf(base, "%s", _DataPath);

	if (_NthItem)
		snprintf(_StrDestination, _SizeofStrDestination, "%s/%s%u%s", base.c_str(), _TestName, _NthItem, _Ext);
	else
		snprintf(_StrDestination, _SizeofStrDestination, "%s/%s%s", base.c_str(), _TestName, _Ext);

	clean_path(_StrDestination, _SizeofStrDestination, _StrDestination);
}

template<size_t size> void BuildDataPath(char (&_StrDestination)[size], const char* _TestName, const char* _DataPath, const char* _DataSubpath, const char* _Path, unsigned int _NthItem, const char* _Ext) { BuildDataPath(_StrDestination, size, _TestName, _DataPath, _DataSubpath, _Path, _NthItem, _Ext); }

bool oTest::TestBinary(const void* _pBuffer, size_t _SizeofBuffer, const char* _FileExtension, unsigned int _NthBinary)
{
	oTestManager::DESC desc;
	oTestManager::Singleton()->GetDesc(&desc);
	vendor::value Vendor = static_cast<oTestManager_Impl*>(oTestManager::Singleton())->DriverDescs[0].vendor; // todo: Make this more elegant

	path_string golden;
	BuildDataPath(golden.c_str(), GetName(), desc.DataPath, "GoldenBinaries", desc.GoldenBinariesPath, _NthBinary, _FileExtension);
	path_string goldenAMD;
	char ext[32];
	snprintf(ext, "_AMD%s", _FileExtension);
	BuildDataPath(goldenAMD.c_str(), GetName(), desc.DataPath, "GoldenBinaries", desc.GoldenBinariesPath, _NthBinary, ext);
	path_string output;
	BuildDataPath(output.c_str(), GetName(), desc.DataPath, "Output", desc.OutputPath, _NthBinary, _FileExtension);

	bool bSaveTestBuffer = false;
	finally SaveTestBuffer([&]
	{
		if (bSaveTestBuffer)
			filesystem::save(path(output), _pBuffer, _SizeofBuffer, filesystem::save_option::binary_write);
	});

	intrusive_ptr<oBuffer> GoldenBinary;
	{
		if (Vendor == vendor::amd)
		{
			// Try to load a more-specific golden binary, but if it's not there it's
			// ok to try to use the default one.
			oBufferLoad(goldenAMD, &GoldenBinary);
		}

		if (!GoldenBinary)
		{
			if (!oBufferLoad(golden, &GoldenBinary))
			{
				if (Vendor != vendor::nvidia)
				{
					path_string outputAMD;
					char ext[32];
					snprintf(ext, "_AMD%s", _FileExtension);
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
		sstring testSize, goldenSize;
		format_bytes(testSize, _SizeofBuffer, 2);
		format_bytes(goldenSize, GoldenBinary->GetSize(), 2);
		bSaveTestBuffer = true;
		return oErrorSetLast(std::errc::protocol_error, "Golden binary compare failed because the binaries are different sizes (test is %s, golden is %s)", testSize.c_str(), goldenSize.c_str());
	}

	if (memcmp(_pBuffer, GoldenBinary->GetData(), GoldenBinary->GetSize()))
	{
		bSaveTestBuffer = true;
		return oErrorSetLast(std::errc::protocol_error, "Golden binary compare failed because the bytes differ");
	}

	return true;
}

static void surface_save(const surface::buffer* _pSurface, surface::alpha_option::value _Option, const path& _Path)
{
	size_t size = 0;
	std::shared_ptr<char> encoded = encode(_pSurface, &size, surface::get_file_format(_Path), _Option);
	filesystem::save(_Path, encoded.get(), size, filesystem::save_option::binary_write);
}
inline void surface_save(std::shared_ptr<const surface::buffer>& _pSurface, surface::alpha_option::value _Option, const path& _Path) { surface_save(_pSurface.get(), _Option, _Path); }
inline void surface_save(std::shared_ptr<surface::buffer>& _pSurface, surface::alpha_option::value _Option, const path& _Path) { surface_save(_pSurface.get(), _Option, _Path); }

bool oTest::TestImage(const surface::buffer* _pTestImage
	, const char* _GoldenImagePath
	, const char* _FailedImagePath
	, unsigned int _NthImage
	, int _ColorChannelTolerance
	, float _MaxRMSError
	, unsigned int _DiffImageMultiplier
	, bool _OutputGoldenImage)
{
	size_t commonPathLength = cmnroot(_GoldenImagePath, _FailedImagePath);
	const char* gPath = _GoldenImagePath + commonPathLength;
	const char* fPath = _FailedImagePath + commonPathLength;

	surface::info si = _pTestImage->get_info();

	const surface::alpha_option::value ao = surface::has_alpha(si.format) ? surface::alpha_option::force_alpha : surface::alpha_option::force_no_alpha;

	std::shared_ptr<surface::buffer> GoldenImage;
	{
		size_t bSize = 0;
		std::shared_ptr<char> b;
		try { b = filesystem::load(_GoldenImagePath, filesystem::load_option::binary_read, &bSize); }
		catch (std::exception&) { return oErrorSetLast(std::errc::io_error, "Load failed: (Golden)...%s", gPath); }

		try { GoldenImage = surface::decode(b.get(), bSize, ao); }
		catch (std::exception&) { return oErrorSetLast(std::errc::protocol_error, "Corrupt Image: (Golden)...%s", gPath); }
	}

	surface::info gsi = GoldenImage->get_info();

	// Compare dimensions/format before going into pixels
	{
		if (any(si.dimensions != gsi.dimensions))
		{
			try { surface_save(_pTestImage, ao, _FailedImagePath); }
			catch (std::exception&) { return oErrorSetLast(std::errc::io_error, "Save failed: (Output)...%s", fPath); }
			return oErrorSetLast(std::errc::protocol_error, "Differing dimensions: (Output %dx%d)...%s != (Golden %dx%d)...%s", si.dimensions.x, si.dimensions.y, fPath, gsi.dimensions.x, gsi.dimensions.y, gPath);
		}

		if (si.format != gsi.format)
		{
			try { surface_save(_pTestImage, ao, _FailedImagePath); }
			catch (std::exception&) { return oErrorSetLast(std::errc::io_error, "Save failed: (Output)...%s", fPath); }
			return oErrorSetLast(std::errc::protocol_error, "Differing formats: (Golden %s)...%s != (Output %s)...%s", as_string(gsi.format), gPath, as_string(si.format), fPath);
		}
	}

	// Resolve parameter settings against global settings
	{
		oTestManager::DESC TestDesc;
		oTestManager::Singleton()->GetDesc(&TestDesc);

		if (_MaxRMSError < 0.0f)
			_MaxRMSError = TestDesc.MaxRMSError;
		if (_DiffImageMultiplier == oDEFAULT)
			_DiffImageMultiplier = TestDesc.DiffImageMultiplier;
	}

	// Do the real test
	surface::info dsi;
	dsi.format = surface::r8_unorm;
	dsi.dimensions = si.dimensions;
	std::shared_ptr<surface::buffer> diffs = surface::buffer::make(dsi);
	float RMSError = surface::calc_rms(_pTestImage, GoldenImage.get(), diffs.get(), _DiffImageMultiplier);

	// Save out test image and diffs if there is a non-similar result.
	if (RMSError > _MaxRMSError)
	{
		try { surface_save(_pTestImage, ao, _FailedImagePath); }
		catch (std::exception&) { return oErrorSetLast(std::errc::io_error, "Save failed: (Output)...%s", fPath); }

		path diffPath(_FailedImagePath);
		diffPath.replace_extension_with_suffix("_diff.png");
		const char* dPath = diffPath.c_str() + commonPathLength;

		try { surface_save(diffs, ao, diffPath); }
		catch (std::exception&) { return oErrorSetLast(std::errc::io_error, "Save failed: (Diff)...%s", dPath); }

		if (_OutputGoldenImage)
		{
			path goldenPath(_FailedImagePath);
			goldenPath.replace_extension_with_suffix("_golden.png");
			const char* gPath = goldenPath.c_str() + commonPathLength;

			try { surface_save(GoldenImage, ao, goldenPath); }
			catch (std::exception&) { return oErrorSetLast(std::errc::io_error, "Save failed: (Golden)...%s", gPath); }
		}

		return oErrorSetLast(std::errc::protocol_error, "Compare failed: %.03f RMS error (threshold %.03f): (Output)...%s != (Golden)...%s", RMSError, _MaxRMSError, fPath, gPath);
	}

	return true;
}

struct DriverPaths
{
	path Generic;
	path VendorSpecific;
	path CardSpecific;
	path DriverSpecific;
};

static bool oInitialize(const char* _RootPath, const char* _Filename, const adapter::info& _DriverDesc, DriverPaths* _pDriverPaths)
{
	_pDriverPaths->Generic = _RootPath;
	_pDriverPaths->VendorSpecific = _pDriverPaths->Generic / as_string(_DriverDesc.vendor);

	path_string tmp, tmp2;
	tmp = _pDriverPaths->VendorSpecific / _DriverDesc.description.c_str();
	replace(tmp2, tmp, " ", "_");
	_pDriverPaths->CardSpecific = tmp2; 

	sstring driverVer;
	_pDriverPaths->DriverSpecific = _pDriverPaths->CardSpecific / to_string(driverVer, _DriverDesc.version);

	_pDriverPaths->Generic /= _Filename;
	_pDriverPaths->VendorSpecific /= _Filename;
	_pDriverPaths->CardSpecific /= _Filename;
	_pDriverPaths->DriverSpecific /= _Filename;

	return true;
}

bool oTest::TestImage(const surface::buffer* _pTestImage, unsigned int _NthImage, int _ColorChannelTolerance, float _MaxRMSError, unsigned int _DiffImageMultiplier)
{
	// Check: GoldenDir/CardName/DriverVersion/GoldenImage
	// Then Check: GoldenDir/CardName/GoldenImage
	// Then Check: GoldenDir/GoldenImage

	oTestManager::DESC TestDesc;
	oTestManager::Singleton()->GetDesc(&TestDesc);

	const adapter::info& DriverDesc = static_cast<oTestManager_Impl*>(oTestManager::Singleton())->DriverDescs[0]; // todo: Make this more elegant

	sstring nthImageBuf;
	path_string Filename;
	snprintf(Filename, "%s%s.png", GetName(), _NthImage == 0 ? "" : to_string(nthImageBuf, _NthImage));

	DriverPaths GoldenPaths, FailurePaths;
	oVERIFY(oInitialize(TestDesc.GoldenImagesPath, Filename, DriverDesc, &GoldenPaths));
	oVERIFY(oInitialize(TestDesc.OutputPath, Filename, DriverDesc, &FailurePaths));

	if (filesystem::exists(GoldenPaths.DriverSpecific))
		return TestImage(_pTestImage, GoldenPaths.DriverSpecific, FailurePaths.DriverSpecific, _NthImage, _ColorChannelTolerance, _MaxRMSError, _DiffImageMultiplier, TestDesc.EnableOutputGoldenImages);

	else if (filesystem::exists(GoldenPaths.VendorSpecific))
		return TestImage(_pTestImage, GoldenPaths.VendorSpecific, FailurePaths.VendorSpecific, _NthImage, _ColorChannelTolerance, _MaxRMSError, _DiffImageMultiplier, TestDesc.EnableOutputGoldenImages);

	else if (filesystem::exists(GoldenPaths.CardSpecific))
		return TestImage(_pTestImage, GoldenPaths.CardSpecific, FailurePaths.CardSpecific, _NthImage, _ColorChannelTolerance, _MaxRMSError, _DiffImageMultiplier, TestDesc.EnableOutputGoldenImages);

	else if (filesystem::exists(GoldenPaths.Generic))
		return TestImage(_pTestImage, GoldenPaths.Generic, FailurePaths.Generic, _NthImage, _ColorChannelTolerance, _MaxRMSError, _DiffImageMultiplier, TestDesc.EnableOutputGoldenImages);

	surface::info si = _pTestImage->get_info();

	surface::alpha_option::value ao = surface::has_alpha(si.format) ? surface::alpha_option::force_alpha : surface::alpha_option::force_no_alpha;
	try { surface_save(_pTestImage, ao, FailurePaths.DriverSpecific); }
	catch (std::exception&) { return oErrorSetLast(std::errc::io_error, "Save failed: (Output)%s", FailurePaths.DriverSpecific.c_str()); }

	return oErrorSetLast(std::errc::no_such_file_or_directory, "Not found: (Golden).../%s Test Image saved to %s", Filename, FailurePaths.DriverSpecific.c_str());
}

bool oSpecialTest::CreateProcess(const char* _SpecialTestName, std::shared_ptr<process>* _pProcess)
{
	if (!_SpecialTestName || !_pProcess)
		return oErrorSetLast(std::errc::invalid_argument);

	xlstring cmdline = filesystem::app_path(true);

	snprintf(cmdline, "%s -s %s", cmdline.c_str(), _SpecialTestName);

	process::info pi;
	pi.command_line = cmdline;
	pi.stdout_buffer_size = oKB(64);
	*_pProcess = process::make(pi);
	return true;
}

//#define DEBUG_SPECIAL_TEST

bool oSpecialTest::Start(process* _pProcess, char* _StrStatus, size_t _SizeofStrStatus, int* _pExitCode, unsigned int _TimeoutMS)
{
	if (!_pProcess || !_StrStatus || !_pExitCode)
		return oErrorSetLast(std::errc::invalid_argument);

	#ifdef DEBUG_SPECIAL_TEST
		pi.suspended = true;
	#endif
	process::info pi = _pProcess->get_info();
	const char* SpecialTestName = rstrstr(pi.command_line, "-s ") + 3;
	if (!SpecialTestName || !*SpecialTestName)
		return oErrorSetLast(std::errc::invalid_argument, "Process with an invalid command line for oSpecialTest specified.");

	mstring interprocessName;
	snprintf(interprocessName, "oTest.%s.Started", SpecialTestName);
	ouro::process::event Started(interprocessName);
	oASSERTA(!Started.wait_for(oStd::chrono::milliseconds(0)), "Started event set when it shouldn't be (before start).");
	#ifdef DEBUG_SPECIAL_TEST
		_pProcess->Start();
	#endif

	oTestManager::DESC testingDesc;
	oTestManager::Singleton()->GetDesc(&testingDesc);

	if (testingDesc.EnableSpecialTestTimeouts)
	{
		if (!Started.wait_for(oStd::chrono::milliseconds(_TimeoutMS)))
		{
			snprintf(_StrStatus, _SizeofStrStatus, "Timed out waiting for %s to start.", SpecialTestName);
			oTRACE("*** SPECIAL MODE UNIT TEST %s timed out waiting for Started event. (Ensure the special mode test sets the started event when appropriate.) ***", SpecialTestName);
			if (!_pProcess->exit_code(_pExitCode))
				_pProcess->kill(std::errc::timed_out);

			snprintf(_StrStatus, _SizeofStrStatus, "Special Mode %s timed out on start.", SpecialTestName);
			return false;
		}
	}

	else
		Started.wait();

	// If we timeout on ending, that's good, it means the app is still running
	if ((_pProcess->wait_for(oStd::chrono::milliseconds(200)) && _pProcess->exit_code(_pExitCode)))
	{
		xlstring msg;
		size_t bytes = _pProcess->from_stdout(msg.c_str(), msg.capacity());
		msg[bytes] = 0;
		if (bytes)
			snprintf(_StrStatus, _SizeofStrStatus, "%s: %s", SpecialTestName, msg.c_str());
		return false;
	}

	return true;
}

void oSpecialTest::NotifyReady()
{
	mstring interprocessName;
	const char* testName = type_name(typeid(*this).name());
	snprintf(interprocessName, "oTest.%s.Started", testName);
	ouro::process::event Ready(interprocessName);
	oASSERTA(!Ready.wait_for(oStd::chrono::milliseconds(0)), "Ready event set when it shouldn't be (in NotifyReady).");
	Ready.set();
}

oTestManager::RegisterTestBase::RegisterTestBase(unsigned int _BugNumber, oTest::RESULT _BugResult, const char* _PotentialZombieProcesses)
	: BugNumber(_BugNumber)
	, BugResult(_BugResult)
{
	strlcpy(PotentialZombieProcesses, _PotentialZombieProcesses);
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
		module::info mi = this_module::get_info();
		sstring Ver;
		TestSuiteName = mi.product_name;
		TestSuiteName += " ";
		TestSuiteName += to_string(Ver, mi.version);
	}

	path_string defaultDataPath = filesystem::data_path();
	DataPath = _pDesc->DataPath ? _pDesc->DataPath : defaultDataPath;
	if (_pDesc->ExecutablesPath)
		ExecutablesPath = _pDesc->ExecutablesPath;
	else
	{
		path_string exes(defaultDataPath);
		#ifdef o64BIT
			strlcat(exes, "../bin/x64/");
		#elif o32BIT
			strlcat(exes, "../bin/win32/");
		#else
			#error Unknown bitness
		#endif
		ExecutablesPath = clean_path(exes.c_str(), exes);
	}

	GoldenBinariesPath = _pDesc->GoldenBinariesPath ? _pDesc->GoldenBinariesPath : (DataPath + "GoldenBinaries/");
	GoldenImagesPath = _pDesc->GoldenImagesPath ? _pDesc->GoldenImagesPath : (DataPath + "GoldenImages/");
	TempPath = filesystem::temp_path() / "oUnitTestTemp/";
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
	path cwd = filesystem::current_path();
	path datapath(Desc.DataPath);
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

	sstring StrVer;
	for (unsigned int i = 0; i < DriverDescs.size(); i++)
		Report(oConsoleReporting::INFO, "Video Driver %u: %s v%s\n", i, DriverDescs[i].description.c_str(), to_string2(StrVer, DriverDescs[i].version));

	auto scc = make_scc(scc_protocol::svn, std::bind(system::spawn, std::placeholders::_1, std::placeholders::_2, false, std::placeholders::_3));

	bool oSystemExecute(const char* _CommandLine
		, const oFUNCTION<void(char* _Line)>& _GetLine = nullptr
		, int* _pExitCode = nullptr
		, bool _ShowWindow = false
		, unsigned int _ExecutionTimeout = oInfiniteWait);

	path DevPath = filesystem::dev_path();
	lstring CLStr;
	uint CL = 0;//scc->revision(DevPath); //@tony: this takes too long
	if (CL)
	{
		try
		{
			if (scc->is_up_to_date(DevPath, CL))
				snprintf(CLStr, "%d", CL);
			else
				snprintf(CLStr, "%d + modifications", CL);
		}

		catch (std::exception& e)
		{
			snprintf(CLStr, "%d + ???", CL);
			oTRACE("scc failure: %s", e.what());
		}
	}
	else
		snprintf(CLStr, "???");

	Report(oConsoleReporting::INFO, "SCC Revision: %s\n", CLStr.c_str());
}

void oTestManager_Impl::RegisterSpecialModeTests()
{
	oFOR(auto pRTB, Tests)
	{
		if (!pRTB->IsSpecialTest())
			continue;

		const char* Name = type_name(pRTB->GetTypename());
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
				push_back_unique(PotentialZombies, std::string(z));
				std::string DebugName = z;
				DebugName += oMODULE_DEBUG_SUFFIX_A;
				push_back_unique(PotentialZombies, DebugName);
				z = oStrTok(nullptr, ";", &ctx);
			}
		}
	}
}

static bool FindDuplicateProcessInstanceByName(process::id _ProcessID, process::id _ParentProcessID, const char* _ProcessExePath, process::id _IgnorePID, const char* _FindName, process::id* _pOutPIDs, size_t _NumOutPIDs, size_t* _pCurrentCount)
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
	process::id ThisID = this_process::get_id();
	process::id pids[1024];
	size_t npids = 0;

	process::enumerate(std::bind(FindDuplicateProcessInstanceByName, oBIND1, oBIND2, oBIND3, ThisID, _Name, pids, oCOUNTOF(pids), &npids));

	unsigned int retries = 3;
	for (size_t i = 0; i < npids; i++)
	{
		if (process::has_debugger_attached(pids[i]))
			continue;

		process::terminate(pids[i], std::errc::operation_canceled);
		if (!process::wait_for(pids[i], oSeconds(5)))
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

// @tony: hack for some macro glue as the codebase gets converted to exceptions.
oTest* g_Test = nullptr;

oTest::RESULT oTestManager_Impl::RunTest(RegisterTestBase* _pRegisterTestBase, char* _StatusMessage, size_t _SizeofStatusMessage)
{
	if (!KillZombies())
	{
		snprintf(_StatusMessage, _SizeofStatusMessage, "oTest infrastructure could not kill zombie process");
		return oTest::FAILURE;
	}

	*_StatusMessage = 0;

	srand(Desc.RandomSeed);

	filesystem::remove(path(TempPath));

	// Initialize iocp before leak tracking flags it as a leak
	windows::iocp::ensure_initialized();

	// @tony: Moving other stuff that are false-positives here so I can see
	// them all...
	windows::crt_leak_tracker::new_context();

	oTest* pTest = _pRegisterTestBase->New();
	g_Test = pTest;
	oTest::RESULT result = pTest->Run(_StatusMessage, _SizeofStatusMessage);
	g_Test = nullptr;
	delete pTest;

	// obug_1763: We need to forcefully flushIOCP to ensure it doesn't report 
	// memory leaks.
	// @tony: This isn't the only culprit. Maybe we should move all these
	// into oCRTLeakTracker so the dependency on reporting seems explicit.
	extern void FlushIOCP();
	FlushIOCP();

	if (!windows::iocp::wait_for(20000))
		oTRACE("iocp wait timed out");

	bool Leaks = windows::crt_leak_tracker::report();
	if (result != oTest::FAILURE && Leaks)
	{
		result = oTest::LEAKS;
		snprintf(_StatusMessage, _SizeofStatusMessage, "Leaks (see debug log for full report)");
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
			mstring TestName;
			TestName = type_name(pRTB->GetTypename());
			
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

static void oTraceCPUFeatures()
{
	// @tony: This and the header that is currently printed out is becoming 
	// large... we should move it somewhere?

	cpu::info cpu_info = cpu::get_info();
	oTRACE("CPU: %s. %d Processors, %d HWThreads", cpu_info.brand_string.c_str()
		, cpu_info.processor_count, cpu_info.hardware_thread_count);

	cpu::enumerate_features([&](const char* _FeatureName, const cpu::support::value& _Support)->bool
	{
		oTRACE(" CPU Feature %s has %s", _FeatureName, as_string(_Support));
		return true;
	});
}

oTest::RESULT oTestManager_Impl::RunTests(oFilterChain::FILTER* _pTestFilters, size_t _SizeofTestFilters)
{
	if (!adapter::all_up_to_date())
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
	adapter::enumerate([&](const adapter::info& _Info)->bool
	{
		if (*(int*)&_Info.id > oCOUNTOF(DriverDescs))
		{
			oTRACE("WARNING: There are more GPUs attached to the system than we have storage for! Only holding information for the first %d", oCOUNTOF(DriverDescs));
			return false;
		}
		else
			DriverDescs.push_back(_Info);
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

	// @tony: For testing progress bar functionality. If we like this as
	// a feature of the unit test, we should expose it through the DESC.
	//ShowProgressBar = true;

	oStd::thread ProgressBarThread;
	bool ShouldStop = false;
	oConcurrency::event Ready;
	progress_bar* pProgressBar = nullptr;
	finally StopProgressBar([&]
	{
		if (ShowProgressBar)
			Ready.wait();
		
		if (pProgressBar)
			pProgressBar->quit();
		
		ProgressBarThread.join();
	});
	
	if (ShowProgressBar)
	{
		ProgressBarThread = std::move(oStd::thread([&]
		{
			oConcurrency::begin_thread("Progress Bar Thread");
			xlstring title;
			console::get_title(title);
			std::shared_ptr<progress_bar> ProgressBar = progress_bar::make(title, console::icon(), [&] { ShouldStop = true; });
			pProgressBar = ProgressBar.get();
			Ready.set();
			ProgressBar->flush_messages(true);
			pProgressBar = nullptr;
			oConcurrency::end_thread();
		}));

		Ready.wait();
	}

	size_t ProgressTotalNumTests = 0;
	size_t ProgressNumTestsSoFar = 0;
	if (ShowProgressBar)
	{
		pProgressBar->set_text("Calculating the number of tests...");
		pProgressBar->restore();

		ProgressTotalNumTests = CalculateNumTests(Desc, &filterChain);
		if (ShouldStop)
		{
			oTRACE("ProgressBar Stop Pressed, aborting.");
			return oTest::FAILURE;
		}
	}

	windows::crt_leak_tracker::enable(Desc.EnableLeakTracking);
	windows::crt_leak_tracker::capture_callstack(Desc.CaptureCallstackForTestLeaks);

	xlstring timeMessage;
	double allIterationsStartTime = ouro::timer::now();
	for (size_t r = 0; r < Desc.NumRunIterations; r++)
	{
		size_t Count[oTest::NUM_TEST_RESULTS];
		memset(Count, 0, sizeof(size_t) * oTest::NUM_TEST_RESULTS);

		// Prepare formatting used to print results
		sstring nameSpec;
		snprintf(nameSpec, "%%-%us", Desc.NameColumnWidth);

		sstring statusSpec;
		snprintf(statusSpec, "%%-%us", Desc.StatusColumnWidth);

		sstring timeSpec;
		snprintf(timeSpec, "%%-%us", Desc.TimeColumnWidth);

		sstring messageSpec;
		snprintf(messageSpec, "%%s\n");

		xlstring statusMessage;

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

		double totalTestStartTime = ouro::timer::now();
		oFOR(auto pRTB, Tests)
		{
			if (pRTB && !pRTB->IsSpecialTest())
			{
				mstring TestName;
				strlcpy(TestName, type_name(pRTB->GetTypename()));
				
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

							pProgressBar->set_text(TestName);
						}

						oTRACE("========== Begin %s Run %u ==========", TestName.c_str(), r+1);
						double testStart = ouro::timer::now();
						result = RunTest(pRTB, statusMessage.c_str(), statusMessage.capacity());
						testDuration = ouro::timer::now() - testStart;
						oTRACE("========== End %s Run %u (%s) ==========", TestName.c_str(), r+1, as_string(result));
						Count[result]++;

						if (ShowProgressBar)
						{
							ProgressNumTestsSoFar++;
							pProgressBar->set_percentage(static_cast<int>((100 * (ProgressNumTestsSoFar+1)) / ProgressTotalNumTests));
						}
					}

					else
						result = pRTB->GetBugResult();

					switch (result)
					{
						case oTest::SUCCESS:
							if (!*statusMessage || !strcmp("operation was successful", statusMessage))
								snprintf(statusMessage, "---");
							ReportType = oConsoleReporting::SUCCESS;
							break;

						case oTest::FAILURE:
							if (!*statusMessage)
								snprintf(statusMessage, "Failed with no test-specific status message");
							ReportType = oConsoleReporting::CRIT;
							break;

						case oTest::SKIPPED:
							if (!*statusMessage)
								snprintf(statusMessage, "Skipped");
							ReportType = oConsoleReporting::WARN;
							break;

						case oTest::BUGGED:
							snprintf(statusMessage, "Test disabled. See oBug_%u", pRTB->GetBugNumber());
							ReportType = oConsoleReporting::ERR;
							break;

						case oTest::NOTREADY:
							snprintf(statusMessage, "Test not yet ready. See oBug_%u", pRTB->GetBugNumber());
							ReportType = oConsoleReporting::INFO;
							break;

						case oTest::LEAKS:
							if (!*statusMessage)
								snprintf(statusMessage, "Leaks memory");
							ReportType = oConsoleReporting::WARN;
							break;

						case oTest::PERFTEST:
							snprintf(statusMessage, "Potentially lengthy perf-test not run by default");
							ReportType = oConsoleReporting::WARN;
							break;
					}
				}

				else
				{
					ReportType = oConsoleReporting::WARN;
					Count[oTest::SKIPPED]++;
					snprintf(statusMessage, "---");
				}

				Report(ReportType, statusSpec, as_string(result));
				ReportSep();

				ReportType = oConsoleReporting::DEFAULT;
				if (testDuration > Desc.TestReallyTooSlowTimeInSeconds)
					ReportType = oConsoleReporting::ERR;
				else if (testDuration > Desc.TestTooSlowTimeInSeconds)
					ReportType = oConsoleReporting::WARN;

				format_duration(timeMessage, round(testDuration), true);
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

		format_duration(timeMessage, round(ouro::timer::now() - totalTestStartTime));
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
		format_duration(timeMessage, round(ouro::timer::now() - allIterationsStartTime));
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
		xlstring statusMessage;
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

bool oTestManager_Impl::BuildPath(path& _FullPath, const path& _RelativePath, oTest::PATH_TYPE _PathType, bool _FileMustExist) const
{
	switch (_PathType)
	{
		case oTest::EXECUTABLES: _FullPath = Desc.ExecutablesPath; break;
		case oTest::DATA: _FullPath = Desc.DataPath; break;
		case oTest::GOLDEN_BINARIES: _FullPath = Desc.GoldenBinariesPath; break;
		case oTest::GOLDEN_IMAGES: _FullPath = Desc.GoldenImagesPath;	break;
		case oTest::TEMP: _FullPath = Desc.TempPath; break;
		case oTest::INPUT: _FullPath = Desc.InputPath; break;
		case oTest::OUTPUT: _FullPath = Desc.OutputPath; break;
		oNODEFAULT;
	}

	_FullPath /= _RelativePath;
	if (_FileMustExist && !oStreamExists(_FullPath))
		return oErrorSetLast(std::errc::no_such_file_or_directory, "not found: %s", _FullPath.c_str());
	return true;
}
