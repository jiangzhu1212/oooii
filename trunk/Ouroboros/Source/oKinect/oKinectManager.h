/**************************************************************************
 * The MIT License                                                        *
 * Copyright (c) 2013 OOOii.                                              *
 * antony.arciuolo@oooii.com                                              *
 * kevin.myers@oooii.com                                                  *
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
// Encapsulate the status changes when Kinects get plugged/unplugged.
#pragma once
#ifndef oKinectManager_h
#define oKinectManager_h

#ifdef oHAS_KINECT_SDK

#include <oKinect/oKinect.h>
#include <oKinect/oKinectUtil.h>
#include <oPlatform/oSingleton.h>
#include <oConcurrency/mutex.h>
#include <oStd/fixed_string.h>
#include <oStd/fixed_vector.h>

struct oKinectManager : oProcessSingleton<oKinectManager>
{
	static const oGUID GUID;

	oKinectManager();
	~oKinectManager();

	void Register(threadsafe oKinect* _pKinect);
	void Unregister(threadsafe oKinect* _pKinect);

private:

	oConcurrency::mutex KinectsMutex;
	oStd::fixed_vector<threadsafe oKinect*, 8> Kinects;

	void OnStatus(oGUI_INPUT_DEVICE_STATUS _Status, const char* _InstanceName, const char* _UniqueDeviceName);
	static void CALLBACK StatusProc(HRESULT _hrStatus, const OLECHAR* _InstanceName, const OLECHAR* _UniqueDeviceName, void* _pThis);
};

#endif // oHAS_KINECT_SDK
#endif
