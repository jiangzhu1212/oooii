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
// Soft-link SetupAPI
#pragma once
#ifndef oSetupAPI_h
#define oSetupAPI_h

#include <oPlatform/oModuleUtil.h>
#include <SetupAPI.h>

oDECLARE_DLL_SINGLETON_BEGIN(oWinSetupAPI)
	HDEVINFO (__stdcall *SetupDiGetClassDevsA)(const _GUID *ClassGuid, PCTSTR Enumerator, HWND hwndParent, DWORD Flags);
	HDEVINFO (__stdcall *SetupDiGetClassDevsW)(const _GUID *ClassGuid, PCWSTR Enumerator, HWND hwndParent, DWORD Flags);
	BOOL (__stdcall *SetupDiDestroyDeviceInfoList)(HDEVINFO DeviceInfoSet);
	BOOL (__stdcall *SetupDiEnumDeviceInfo)(HDEVINFO DeviceInfoSet, DWORD MemberIndex, PSP_DEVINFO_DATA DeviceInfoData);
	BOOL (__stdcall *SetupDiGetDevicePropertyW)(HDEVINFO DeviceInfoSet, PSP_DEVINFO_DATA DeviceInfoData, const DEVPROPKEY *PropertyKey, DEVPROPTYPE *PropertyType, PBYTE PropertyBuffer, DWORD PropertyBufferSize, PDWORD RequiredSize, DWORD Flags);
oDECLARE_DLL_SINGLETON_END()

#endif
