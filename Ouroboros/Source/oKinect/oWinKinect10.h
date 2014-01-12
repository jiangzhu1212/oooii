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
#pragma once
#ifndef oWinKinect10_h
#define oWinKinect10_h

#ifdef oHAS_KINECT_SDK

#include <windows.h>
#include <NuiApi.h>
#include <oCore/process_heap.h>
#include <thread>

class oWinKinect10
{	
public:

	static oWinKinect10& Singleton();

	ouro::version GetVersion() const;

	HRESULT (__stdcall* NuiCreateSensorByIndex__)(int index, INuiSensor** ppNuiSensor);
	HRESULT (__stdcall* NuiCreateSensorById__)(const OLECHAR* strInstanceId, INuiSensor** ppNuiSensor);
	HRESULT (__stdcall* NuiSetDeviceStatusCallback__)(NuiStatusProc callback, void* pUserData);
	HRESULT (__stdcall* NuiGetSensorCount)(int* pCount);

	// Kinect threads don't close out before DLL gets unloaded, causing a corrupt
	// crash, so figure out which threads are Kinect's, and ensure they're dead 
	// before unloading the module.

	inline HRESULT SafeNuiCreateSensorByIndex(int index, INuiSensor** ppNuiSensor)
	{
		if (!TIDsRecorded) RecordPreKinectThreads();
		HRESULT hr = NuiCreateSensorByIndex__(index, ppNuiSensor);
		if (!TIDsRecorded) RecordKinectWorkerThreads();
		TIDsRecorded = true;
		return hr;
	}

	inline HRESULT SafeNuiCreateSensorById(const OLECHAR* strInstanceId, INuiSensor** ppNuiSensor)
	{
		if (!TIDsRecorded) RecordPreKinectThreads();
		HRESULT hr = NuiCreateSensorById__(strInstanceId, ppNuiSensor);
		if (!TIDsRecorded) RecordKinectWorkerThreads();
		TIDsRecorded = true;
		return hr;
	}

	// If NuiSetDeviceStatusCallback is called in any way then the system will
	// throw a write violation when unloading the DLL, even if the handler is set 
	// to nullptr. So here, register that we called it and then don't unload the
	// DLL.
	HRESULT SafeNuiSetDeviceStatusCallback(NuiStatusProc callback, void* pUserData);

	// Call this instead of _pSensor->NuiShutdown() because the simple call can
	// hang due to a confused hardware state.
	void SafeNuiShutdown(INuiSensor* _pSensor);

protected: 
	oWinKinect10();
	~oWinKinect10();

	ouro::module::id hModule;
	std::vector<std::thread::id, ouro::process_heap::std_allocator<std::thread::id>> TIDs;
	bool TIDsRecorded;
	bool KinectThreadTerminated;
	bool NuiSetDeviceStatusCallbackWasCalled;

	void RecordPreKinectThreads();
	void RecordKinectWorkerThreads();
};

#endif

#endif
