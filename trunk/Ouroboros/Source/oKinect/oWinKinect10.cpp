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
#include "oWinKinect10.h"
#include <oStd/assert.h>

#ifdef oHAS_KINECT_SDK

#pragma comment(lib, "Kinect10.lib")

static const char* sExportedAPIs[] = 
{
	"NuiCreateSensorByIndex",
	"NuiCreateSensorById",
	"NuiSetDeviceStatusCallback",
	"NuiGetSensorCount",
};

// {88A26003-9F51-4828-9C8F-70B50C48B2CF}
const oGUID oWinKinect10::GUID = { 0x88a26003, 0x9f51, 0x4828, { 0x9c, 0x8f, 0x70, 0xb5, 0xc, 0x48, 0xb2, 0xcf } };

void oWinKinect10::RecordPreKinectThreads()
{
	oVERIFY(oEnumProcessThreads(GetCurrentProcessId(), [&](DWORD _ThreadID, DWORD _ParentProcessID)->bool
	{
		TIDs.push_back(_ThreadID);
		return true;
	}));
}

void oWinKinect10::RecordKinectWorkerThreads()
{
	std::vector<DWORD, oProcessHeapAllocator<DWORD>> BeforeTIDs = TIDs;
	TIDs.clear();
	oEnumProcessThreads(GetCurrentProcessId(), [&](DWORD _ThreadID, DWORD _ParentProcessID)->bool
	{
		TIDs.push_back(_ThreadID);
		return true;
	});

	oFOR(DWORD TID, BeforeTIDs)
	{
		auto it = oStd::find(TIDs, TID);
		if (it != TIDs.end())
			TIDs.erase(it);
	}
}

HRESULT oWinKinect10::SafeNuiSetDeviceStatusCallback(NuiStatusProc callback, void* pUserData)
{
	NuiSetDeviceStatusCallbackWasCalled = true;
	return NuiSetDeviceStatusCallback__(callback, pUserData);
}

void oWinKinect10::SafeNuiShutdown(INuiSensor* _pSensor)
{
	// NuiShutdown can hang forever. This problem seems to go away if the Kinect
	// HW is unplugged from its USB and replugged in but in case we get stuck,
	// don't hang the whole process.

	if (_pSensor)
	{
		#if 0
			_pSensor->NuiShutdown();
		#else
			oStd::thread Shutdown([&]{ _pSensor->NuiShutdown(); });
			HANDLE hThread = (HANDLE)Shutdown.native_handle();
			if (WAIT_TIMEOUT == ::WaitForSingleObject(hThread, 1000))
			{
				KinectThreadTerminated = true;
				oVB(::TerminateThread(hThread, 0));
			}
			Shutdown.join();
		#endif
	}	
}

oWinKinect10::oWinKinect10()
	: TIDsRecorded(false)
	, KinectThreadTerminated(false)
	, NuiSetDeviceStatusCallbackWasCalled(false)
{	
	hModule = oCore::module::link("Kinect10.dll", sExportedAPIs, (void**)&NuiCreateSensorByIndex__, oCOUNTOF(sExportedAPIs));
}
oSINGLETON_REGISTER(oWinKinect10);

oWinKinect10::~oWinKinect10()
{
	if (KinectThreadTerminated)
		oTRACE("A Kinect thread needed to be terminated. Kinect SDK may report errors. Ignore the errors or unplug/replug the Kinect to address this reporting.");

	oFOR(DWORD TID, TIDs)
	{
		HANDLE hThread = OpenThread(THREAD_TERMINATE, FALSE, TID);
		if (hThread)
		{
			try { TerminateThread(hThread, 0xdeadc0de); }
			catch (...) {}
		}
	}

	// @oooii-tony: this is pretty frustrating: If NuiSetDeviceStatusCallback is
	// called in any capacity, even with nullptrs, then there's an exception 
	// thrown when the module is unlinked. So in the case that this API was called
	// don't unlink the DLL to avoid the hang/exception/crash.

	if (NuiSetDeviceStatusCallbackWasCalled)
		oTRACE("NuiSetDeviceStatusCallback was called, which would cause the DLL unload to crash, so skipping unload.");
	else
		oCore::module::close(hModule);
}

oStd::version oWinKinect10::GetVersion() const
{
	oCore::module::info mi = oCore::module::get_info(*(oCore::module::id*)&hModule);
	return mi.version;
}

#endif
