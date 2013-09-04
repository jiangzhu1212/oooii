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
#include <oPlatform/Windows/oWindows.h>
#include <oStd/assert.h>
#include <oStd/byte.h>
#include <oStd/color.h>
#include <oBasis/oError.h>
#include <oStd/fixed_string.h>
#include <oConcurrency/mutex.h>
#include <oBasis/oPath.h>
#include <oBasis/oRef.h>
#include <oBasis/oString.h>
#include <oPlatform/oDisplay.h>
#include <oPlatform/oFile.h>
#include <oPlatform/oModule.h>
#include <oPlatform/oProcess.h>
#include <oPlatform/oProcessHeap.h>
#include <oPlatform/oSingleton.h>
#include <oPlatform/oSystem.h>
#include <oPlatform/Windows/oWinRect.h>
#include <oPlatform/Windows/oWinAsString.h>
#include <oPlatform/oMsgBox.h>
#include "SoftLink/oWinDbgHelp.h"
#include "SoftLink/oWinKernel32.h"
#include "SoftLink/oWinNetApi32.h"
#include "SoftLink/oWinDWMAPI.h"
#include "SoftLink/oWinPSAPI.h"
#include "SoftLink/oWinSetupAPI.h"
#include "oStaticMutex.h"
#include <io.h>
#include <time.h>
#include <tlhelp32.h>
#include <Windowsx.h>
#include <comdef.h>
#include <Wbemidl.h>
#include <devguid.h>
#include <devpkey.h>


// Use the Windows Vista UI look. If this causes issues or the dialog not to appear, try other values from processorAchitecture { x86 ia64 amd64 * }
#pragma comment(linker, "\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

int oWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow, int (*oMain)(int argc, const char* argv[]))
{
	int argc = 0;
	const char** argv = oWinCommandLineToArgvA(true, lpCmdLine, &argc);
	int result = oMain(argc, argv);
	oWinCommandLineToArgvAFree(argv);
	return result;
}

// _____________________________________________________________________________

std::errc::errc oWinGetErrc(HRESULT _hResult)
{
	switch (_hResult)
	{
		case ERROR_FILE_EXISTS: return std::errc::operation_in_progress;
		case ERROR_FILE_NOT_FOUND: case ERROR_PATH_NOT_FOUND: return std::errc::no_such_file_or_directory;
		case ERROR_INVALID_NAME: return std::errc::invalid_argument;
		case ERROR_ACCESS_DENIED: case ERROR_SHARING_VIOLATION: return std::errc::permission_denied;
		case E_OUTOFMEMORY: return std::errc::no_buffer_space;
		default: break;
	}
	return std::errc::protocol_error;
}

bool oWinSetLastError(HRESULT _hResult, const char* _ErrorDescPrefix)
{
	if (_hResult == oDEFAULT)
		_hResult = ::GetLastError();

	char err[2048];
	char* p = err;
	size_t count = oCOUNTOF(err);
	if (_ErrorDescPrefix)
	{
		size_t len = oPrintf(err, "%s", _ErrorDescPrefix);
		p += len;
		count -= len;
	}

	size_t len = 0;
	const char* HRAsString = oWinAsStringHR(_hResult);
	if ('u' == *HRAsString)
		len = oPrintf(p, count, " (HRESULT 0x%08x: ", _hResult);
	else
		len = oPrintf(p, count, " (%s: ", HRAsString);

	p += len;
	count -= len;

	if (!oWinParseHRESULT(p, count, _hResult))
		return false;

	oStrcat(p, count, ")");

	std::errc::errc std_err = oWinGetErrc(_hResult);
	return oErrorSetLast(std_err, err);
}

// Link to MessageBoxTimeout based on code from:
// http://www.codeproject.com/KB/cpp/MessageBoxTimeout.aspx

//Functions & other definitions required-->
typedef int (__stdcall *MSGBOXAAPI)(IN HWND hWnd, 
        IN LPCSTR lpText, IN LPCSTR lpCaption, 
        IN UINT uType, IN WORD wLanguageId, IN DWORD dwMilliseconds);
typedef int (__stdcall *MSGBOXWAPI)(IN HWND hWnd, 
        IN LPCWSTR lpText, IN LPCWSTR lpCaption, 
        IN UINT uType, IN WORD wLanguageId, IN DWORD dwMilliseconds);

int MessageBoxTimeoutA(HWND hWnd, LPCSTR lpText, 
    LPCSTR lpCaption, UINT uType, WORD wLanguageId, 
    DWORD dwMilliseconds)
{
    static MSGBOXAAPI MsgBoxTOA = nullptr;

    if (!MsgBoxTOA)
    {
        HMODULE hUser32 = GetModuleHandle("user32.dll");
        if (hUser32)
        {
            MsgBoxTOA = (MSGBOXAAPI)GetProcAddress(hUser32, 
                                      "MessageBoxTimeoutA");
            //fall through to 'if (MsgBoxTOA)...'
        }
        else
        {
            //stuff happened, add code to handle it here 
            //(possibly just call MessageBox())
            return 0;
        }
    }

    if (MsgBoxTOA)
    {
        return MsgBoxTOA(hWnd, lpText, lpCaption, 
              uType, wLanguageId, dwMilliseconds);
    }

    return 0;
}

int MessageBoxTimeoutW(HWND hWnd, LPCWSTR lpText, 
    LPCWSTR lpCaption, UINT uType, WORD wLanguageId, DWORD dwMilliseconds)
{
    static MSGBOXWAPI MsgBoxTOW = nullptr;

    if (!MsgBoxTOW)
    {
        HMODULE hUser32 = GetModuleHandle("user32.dll");
        if (hUser32)
        {
            MsgBoxTOW = (MSGBOXWAPI)GetProcAddress(hUser32, 
                                      "MessageBoxTimeoutW");
            //fall through to 'if (MsgBoxTOW)...'
        }
        else
        {
            //stuff happened, add code to handle it here 
            //(possibly just call MessageBox())
            return 0;
        }
    }

    if (MsgBoxTOW)
    {
        return MsgBoxTOW(hWnd, lpText, lpCaption, 
               uType, wLanguageId, dwMilliseconds);
    }

    return 0;
}

struct SCHEDULED_FUNCTION_CONTEXT
{
	HANDLE hTimer;
	oTASK OnTimer;
	time_t ScheduledTime;
	oStd::sstring DebugName;
};

static void CALLBACK ExecuteScheduledFunctionAndCleanup(LPVOID lpArgToCompletionRoutine, DWORD dwTimerLowValue, DWORD dwTimerHighValue)
{
	SCHEDULED_FUNCTION_CONTEXT& Context = *(SCHEDULED_FUNCTION_CONTEXT*)lpArgToCompletionRoutine;
	if (Context.OnTimer)
	{
		#ifdef _DEBUG
			oStd::sstring diff;
			oStd::format_duration(diff, (double)(time(nullptr) - Context.ScheduledTime));
			oTRACE("Running scheduled function \"%s\" %s after it was scheduled", oSAFESTRN(Context.DebugName), diff.c_str());
		#endif

		Context.OnTimer();
		oTRACE("Finished scheduled function \"%s\"", oSAFESTRN(Context.DebugName));
	}
	oVB(CloseHandle((HANDLE)Context.hTimer));
	delete &Context;
}

// copy from DEVPKEY.H
static const DEVPROPKEY oDEVPKEY_Device_DeviceDesc = { { 0xa45c254e, 0xdf1c, 0x4efd, { 0x80, 0x20, 0x67, 0xd1, 0x46, 0xa8, 0x50, 0xe0 } }, 2 };
static const DEVPROPKEY oDEVPKEY_Device_HardwareIds = { { 0xa45c254e, 0xdf1c, 0x4efd, { 0x80, 0x20, 0x67, 0xd1, 0x46, 0xa8, 0x50, 0xe0 } }, 3 };
static const DEVPROPKEY oDEVPKEY_Device_Class = { { 0xa45c254e, 0xdf1c, 0x4efd, { 0x80, 0x20, 0x67, 0xd1, 0x46, 0xa8, 0x50, 0xe0 } }, 9 };
static const DEVPROPKEY oDEVPKEY_Device_PDOName = { { 0xa45c254e, 0xdf1c, 0x4efd, { 0x80, 0x20, 0x67, 0xd1, 0x46, 0xa8, 0x50, 0xe0 } }, 16 };
static const DEVPROPKEY oDEVPKEY_Device_Parent = { { 0x4340a6c5, 0x93fa, 0x4706, { 0x97, 0x2c, 0x7b, 0x64, 0x80, 0x08, 0xa5, 0xa7 } }, 8 }; 
static const DEVPROPKEY oDEVPKEY_Device_InstanceId = { { 0x78c34fc8, 0x104a, 0x4aca, { 0x9e, 0xa4, 0x52, 0x4d, 0x52, 0x99, 0x6e, 0x57 } }, 256 };
static const DEVPROPKEY oDEVPKEY_Device_Siblings = { { 0x4340a6c5, 0x93fa, 0x4706, { 0x97, 0x2c, 0x7b, 0x64, 0x80, 0x08, 0xa5, 0xa7 } }, 10 };

// if _EnumerateAll is false, then only existing/plugged in devices are searched.
// A single PnP device can support multiple device types, so this sets the 
// array of types the PnP device represents.

static char* oWinPnPEnumeratorToSetupClass(char* _StrDestination, size_t _SizeofStrDestination, const char* _PnPEnumerator)
{
	if (strlen(_PnPEnumerator) <= 4) return nullptr;
	size_t len = strlcpy(_StrDestination, _PnPEnumerator + 4, _SizeofStrDestination);
	if (len >= _SizeofStrDestination) return nullptr;
	char* hash = strrchr(_StrDestination, '#');
	if (!hash) return nullptr;
	*hash = '\0';
	char* end = _StrDestination + len;
	oStd::transform(_StrDestination, end, _StrDestination, [=](char c) { return (c == '#') ? '\\' : c; });
	oStd::transform(_StrDestination, end, _StrDestination, oStd::toupper<const char&>);
	char* first_slash = strchr(_StrDestination, '\\');
	if (!first_slash) return nullptr;
	*first_slash = '\0';
	return _StrDestination;
}

template<size_t size> char* oWinPnPEnumeratorToSetupClass(char (&_StrDestination)[size], const char* _PnPEnumerator) { return oWinPnPEnumeratorToSetupClass(_StrDestination, size, _PnPEnumerator); }
template<typename charT, size_t capacity> char* oWinPnPEnumeratorToSetupClass(oStd::fixed_string<charT, capacity>& _StrDestination, const char* _PnPEnumerator) { return oWinPnPEnumeratorToSetupClass(_StrDestination, _StrDestination.capacity(), _PnPEnumerator); }

oGUI_INPUT_DEVICE_TYPE oWinGetDeviceTypeFromClass(const char* _Class)
{
	static const char* DeviceClass[oGUI_INPUT_DEVICE_TYPE_COUNT] = 
	{
		"Unknown",
		"Keyboard",
		"Mouse",
		"Controller",
		"Control", // won't ever get this from HW config
		"Skeleton",
		"Voice",
		"Touch", // not sure what this is called generically
	};
	static_assert(oCOUNTOF(DeviceClass) == oGUI_INPUT_DEVICE_TYPE_COUNT, "array mismatch");
	for (int i = 0; i < oCOUNTOF(DeviceClass); i++)
		if (!strcmp(DeviceClass[i], _Class))
			return (oGUI_INPUT_DEVICE_TYPE)i;
	return oGUI_INPUT_DEVICE_UNKNOWN;
}

static bool oWinSetupGetString(oStd::mstring& _StrDestination, HDEVINFO _hDevInfo, SP_DEVINFO_DATA* _pSPDID, const DEVPROPKEY* _pPropKey)
{
	DEVPROPTYPE dpType;
	DWORD size = 0;
	oWinSetupAPI::Singleton()->SetupDiGetDevicePropertyW(_hDevInfo, _pSPDID, _pPropKey, &dpType, nullptr, 0, &size, 0);
	if (size)
	{
		oStd::mwstring buffer;
		oASSERT(buffer.capacity() > (size / sizeof(oStd::mwstring::char_type)), "");
		if (!oWinSetupAPI::Singleton()->SetupDiGetDevicePropertyW(_hDevInfo, _pSPDID, _pPropKey, &dpType, (BYTE*)buffer.c_str(), size, &size, 0) || dpType != DEVPROP_TYPE_STRING)
			return oWinSetLastError();
		_StrDestination = buffer;
	}
	else
		_StrDestination.clear();

	return true;
}

// _NumStrings must be initialized to maximum capacity
static bool oWinSetupGetStringList(oStd::mstring* _StrDestination, size_t& _NumStrings, HDEVINFO _hDevInfo, SP_DEVINFO_DATA* _pSPDID, const DEVPROPKEY* _pPropKey)
{
	DEVPROPTYPE dpType;
	DWORD size = 0;
	oWinSetupAPI::Singleton()->SetupDiGetDevicePropertyW(_hDevInfo, _pSPDID, _pPropKey, &dpType, nullptr, 0, &size, 0);
	if (size)
	{
		oStd::xlwstring buffer;
		oASSERT(buffer.capacity() > size, "");
		if (!oWinSetupAPI::Singleton()->SetupDiGetDevicePropertyW(_hDevInfo, _pSPDID, _pPropKey, &dpType, (BYTE*)buffer.c_str(), size, &size, 0) || dpType != DEVPROP_TYPE_STRING_LIST)
			return oWinSetLastError();

		const wchar_t* c = buffer.c_str();
		size_t i = 0;
		for (; i < _NumStrings; i++)
		{
			_StrDestination[i] = c;
			c += wcslen(c) + 1;

			if (!*c)
			{
				_NumStrings = i + 1;
				break;
			}
		}

		if (i > _NumStrings)
		{
			while (*c)
			{
				c += wcslen(c) + 1;
				i++;
			}

			_NumStrings = i + 1;
			return oErrorSetLast(std::errc::no_buffer_space, "not enough strings");
		}
	}
	else
		_NumStrings = 0;

	return true;
}

static bool oWinEnumInputDevices(bool _EnumerateAll, const char* _Enumerator, const oFUNCTION<void(const oWINDOWS_HID_DESC& _HIDDesc)>& _Visitor)
{
	if (!_Visitor)
		return oErrorSetLast(std::errc::invalid_argument, "Must specify a visitor.");

	HDEVINFO hDevInfo = INVALID_HANDLE_VALUE;
	oStd::finally([=]
	{ 
		if (hDevInfo != INVALID_HANDLE_VALUE) 
			oWinSetupAPI::Singleton()->SetupDiDestroyDeviceInfoList(hDevInfo);
	});

	DWORD dwFlag = DIGCF_ALLCLASSES;
	if (!_EnumerateAll)
		dwFlag |= DIGCF_PRESENT;

	hDevInfo = oWinSetupAPI::Singleton()->SetupDiGetClassDevsA(nullptr, _Enumerator, nullptr, dwFlag);
	if (INVALID_HANDLE_VALUE == hDevInfo)
		return oWinSetLastError();

	SP_DEVINFO_DATA SPDID;
	memset(&SPDID, 0, sizeof(SPDID));
	SPDID.cbSize = sizeof(SPDID);
	int DeviceIndex = 0;
	bool KnownTypeFound = false;

	oWINDOWS_HID_DESC HIDDesc;

	oStd::mstring KinectCameraSibling;
	oStd::mstring KinectCameraParent;

	while (oWinSetupAPI::Singleton()->SetupDiEnumDeviceInfo(hDevInfo, DeviceIndex, &SPDID))
	{
		DeviceIndex++;

		oStd::mstring ClassValue;
		if (!oWinSetupGetString(ClassValue, hDevInfo, &SPDID, &oDEVPKEY_Device_Class))
			return false; // pass through error

		if (!oWinSetupGetString(HIDDesc.ParentDeviceInstancePath, hDevInfo, &SPDID, &oDEVPKEY_Device_Parent))
			return false; // pass through error

		// Fall back on generic desc string for Kinect, which apparently is in a 
		// class by itself at the moment.
		if (!strcmp("Microsoft Kinect", ClassValue))
		{
			if (!oWinSetupGetString(ClassValue, hDevInfo, &SPDID, &oDEVPKEY_Device_DeviceDesc))
				return false; // pass through error

			if (strstr(ClassValue, "Camera"))
			{
				ClassValue = "Skeleton";
				HIDDesc.DeviceInstancePath = HIDDesc.ParentDeviceInstancePath;

				oStd::mstring Strings[2];
				size_t Capacity = oCOUNTOF(Strings);
				if (!oWinSetupGetStringList(Strings, Capacity, hDevInfo, &SPDID, &oDEVPKEY_Device_Siblings) || Capacity != 1)
					return false; // pass through error

				KinectCameraSibling = Strings[0];
				KinectCameraParent = HIDDesc.ParentDeviceInstancePath;
			}

			else if (strstr(ClassValue, "Audio"))
			{
				ClassValue = "Voice";
				if (HIDDesc.ParentDeviceInstancePath == KinectCameraSibling)
					HIDDesc.DeviceInstancePath = KinectCameraParent;

				else
					HIDDesc.DeviceInstancePath = HIDDesc.ParentDeviceInstancePath;
			}
			else
				ClassValue = "Unknown";
		}

		else
		{
			if (!oWinSetupGetString(HIDDesc.DeviceInstancePath, hDevInfo, &SPDID, &oDEVPKEY_Device_InstanceId))
				return false; // pass through error
		}

		// There are often meta-classes to be ignored, so don't flag unknown unless
		// every other type is not present.
		HIDDesc.Type = oWinGetDeviceTypeFromClass(ClassValue);
		HIDDesc.DevInst = SPDID.DevInst;

		_Visitor(HIDDesc);
	}

	return true;
}

bool oWinEnumInputDevices(bool _EnumerateAll, const oFUNCTION<void(const oWINDOWS_HID_DESC& _HIDDesc)>& _Visitor)
{
	return oWinEnumInputDevices(_EnumerateAll, "HID", _Visitor) && oWinEnumInputDevices(_EnumerateAll, "USB", _Visitor);
}

void oWinKillExplorer()
{
	if (oProcessExists("explorer.exe"))
	{
		oTRACE("Terminating explorer.exe because the taskbar can interfere with cooperative fullscreen");
		system("TASKKILL /F /IM explorer.exe");
	}
}

bool oScheduleTask(const char* _DebugName, time_t _AbsoluteTime, bool _Alertable, oTASK _Task)
{
	SCHEDULED_FUNCTION_CONTEXT& Context = *new SCHEDULED_FUNCTION_CONTEXT();
	Context.hTimer = CreateWaitableTimer(nullptr, TRUE, nullptr);
	oASSERT(Context.hTimer, "CreateWaitableTimer failed LastError=0x%08x", GetLastError());
	Context.OnTimer = _Task;
	Context.ScheduledTime = _AbsoluteTime;

	if (_DebugName && *_DebugName)
		oStrcpy(Context.DebugName, _DebugName);
	
	#ifdef _DEBUG
		oStd::date then;
		oStd::sstring StrTime, StrDiff;
		try { then = oStd::date_cast<oStd::date>(_AbsoluteTime); }
		catch (std::exception&) { StrTime = "(out-of-time_t-range)"; }
		oStd::strftime(StrTime, oStd::sortable_date_format, then);
		oStd::format_duration(StrDiff, (double)(time(nullptr) - Context.ScheduledTime));
		oTRACE("Setting timer to run function \"%s\" at %s (%s from now)", oSAFESTRN(Context.DebugName), StrTime.c_str(), StrDiff.c_str());
	#endif

	FILETIME ft = oStd::date_cast<FILETIME>(_AbsoluteTime);
	LARGE_INTEGER liDueTime;
	liDueTime.LowPart = ft.dwLowDateTime;
	liDueTime.HighPart = ft.dwHighDateTime;
	if (!SetWaitableTimer(Context.hTimer, &liDueTime, 0, ExecuteScheduledFunctionAndCleanup, (LPVOID)&Context, _Alertable ? TRUE : FALSE))
	{
		oWinSetLastError();
		return false;
	}

	return true;
}

oRECT oToRect(const RECT& _Rect)
{
	oRECT rect;
	rect.Min = int2(_Rect.left, _Rect.top);
	rect.Max = int2(_Rect.right, _Rect.bottom);
	return rect;
}

// @oooii-tony: I can't believe there are callbacks with no way to specify
// context and a whole web of people who just declare a global and forget
// about it. Microsoft, inter-web you are weak. I can't even map something,
// so create a bunch of wrappers hooks to get their unique function pointers
// hand map that back to some user context.

struct HOOK_CONTEXT
{
	HHOOK hHook;
	void* pUserData;
	HOOKPROC UniqueHookProc;
	oHOOKPROC UserHookProc;
};

#define UNIQUE_PROC(ProcName, Index) static LRESULT CALLBACK ProcName##Index(int _nCode, WPARAM _wParam, LPARAM _lParam) { oHOOKPROC hp = Singleton()->GetHookProc(Index); return hp(_nCode, _wParam, _lParam, Singleton()->GetHookUserData(Index)); }
#define UNIQUE_PROCS(ProcName) \
	UNIQUE_PROC(ProcName, 0) \
	UNIQUE_PROC(ProcName, 1) \
	UNIQUE_PROC(ProcName, 2) \
	UNIQUE_PROC(ProcName, 3) \
	UNIQUE_PROC(ProcName, 4) \
	UNIQUE_PROC(ProcName, 5) \
	UNIQUE_PROC(ProcName, 6) \
	UNIQUE_PROC(ProcName, 7) \
	UNIQUE_PROC(ProcName, 8) \
	UNIQUE_PROC(ProcName, 9) \
	UNIQUE_PROC(ProcName, 10) \
	UNIQUE_PROC(ProcName, 11) \
	UNIQUE_PROC(ProcName, 12) \
	UNIQUE_PROC(ProcName, 13) \
	UNIQUE_PROC(ProcName, 14) \
	UNIQUE_PROC(ProcName, 15) \

struct oWindowsHookContext : public oProcessSingleton<oWindowsHookContext>
{
	UNIQUE_PROCS(HookProc)
	
	oWindowsHookContext()
	{
		memset(HookContexts, 0, sizeof(HOOK_CONTEXT) * oCOUNTOF(HookContexts));
		HookContexts[0].UniqueHookProc = HookProc0;
		HookContexts[1].UniqueHookProc = HookProc1;
		HookContexts[2].UniqueHookProc = HookProc2;
		HookContexts[3].UniqueHookProc = HookProc3;
		HookContexts[4].UniqueHookProc = HookProc4;
		HookContexts[5].UniqueHookProc = HookProc5;
		HookContexts[6].UniqueHookProc = HookProc6;
		HookContexts[7].UniqueHookProc = HookProc7;
		HookContexts[8].UniqueHookProc = HookProc8;
		HookContexts[9].UniqueHookProc = HookProc9;
		HookContexts[10].UniqueHookProc = HookProc10;
		HookContexts[11].UniqueHookProc = HookProc11;
		HookContexts[12].UniqueHookProc = HookProc12;
		HookContexts[13].UniqueHookProc = HookProc13;
		HookContexts[14].UniqueHookProc = HookProc14;
		HookContexts[15].UniqueHookProc = HookProc15;
	}

	oHOOKPROC GetHookProc(size_t _Index) { return HookContexts[_Index].UserHookProc; }
	void* GetHookUserData(size_t _Index) { return HookContexts[_Index].pUserData; }

	HOOK_CONTEXT* Allocate()
	{
		oFORI(i, HookContexts)
		{
			HHOOK hh = HookContexts[i].hHook;
			// Do a quick mark of the slot so no other thread grabs it
			if (!hh && oStd::atomic_compare_exchange<HHOOK>(&HookContexts[i].hHook, (HHOOK)0x1337c0de, hh))
				return &HookContexts[i];
		}

		return 0;
	}

	void Deallocate(HHOOK _hHook)
	{
		oFORI(i, HookContexts)
		{
			HHOOK hh = HookContexts[i].hHook;
			if (_hHook == hh && oStd::atomic_compare_exchange<HHOOK>(&HookContexts[i].hHook, 0, hh))
			{
					HookContexts[i].pUserData = 0;
					HookContexts[i].UserHookProc = 0;
			}
		}
	}

	static const oGUID GUID;
	HOOK_CONTEXT HookContexts[16];
};


// {64550D95-07B5-441D-A239-4DFA1200F198}
const oGUID oWindowsHookContext::GUID = { 0x64550d95, 0x7b5, 0x441d, { 0xa2, 0x39, 0x4d, 0xfa, 0x12, 0x0, 0xf1, 0x98 } };
oSINGLETON_REGISTER(oWindowsHookContext);

bool IsProcessThread(DWORD _ThreadID, DWORD _ParentProcessID, DWORD _FindThreadID, bool* _pFound)
{
	if (_ThreadID == _FindThreadID)
	{
		*_pFound = true;
		return false;
	}

	return true;
}

HHOOK oSetWindowsHook(int _idHook, oHOOKPROC _pHookProc, void* _pUserData, DWORD _dwThreadId)
{
	HOOK_CONTEXT* hc = oWindowsHookContext::Singleton()->Allocate();
	if (!hc)
		return 0;

	bool isProcessThread = false;
	oVERIFY(oEnumProcessThreads(GetCurrentProcessId(), oBIND(IsProcessThread, oBIND1, oBIND2, _dwThreadId, &isProcessThread)));

	HMODULE hMod = oGetModule(_pHookProc);
	if (hMod == (HMODULE)oModuleGetCurrent() && isProcessThread)
		hMod = nullptr;

	hc->pUserData = _pUserData;
	hc->UserHookProc = _pHookProc;
	hc->hHook = ::SetWindowsHookExA(_idHook, hc->UniqueHookProc, hMod, _dwThreadId);
	oVB(hc->hHook && "SetWindowsHookEx");

	// recover from any error
	if (!hc->hHook)
	{
		hc->pUserData = 0;
		hc->UserHookProc = 0;
		hc->hHook = 0;
	}

	return hc->hHook;
}

bool oUnhookWindowsHook(HHOOK _hHook)
{
	oWindowsHookContext::Singleton()->Deallocate(_hHook);
	return !!::UnhookWindowsHookEx(_hHook);
}

oWINDOWS_VERSION oGetWindowsVersion()
{
	OSVERSIONINFOEX osvi;
	osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
	if (GetVersionEx((OSVERSIONINFO*)&osvi))
	{
		if (osvi.dwMajorVersion == 6)
		{
			if (osvi.dwMinorVersion == 2)
			{
				if (osvi.wServicePackMajor == 1)
					return osvi.wProductType == VER_NT_WORKSTATION ? oWINDOWS_8_SP1 : oWINDOWS_SERVER_2012_SP1;
				else if (osvi.wServicePackMajor == 0)
					return osvi.wProductType == VER_NT_WORKSTATION ? oWINDOWS_8 : oWINDOWS_SERVER_2012;
			}

			else if (osvi.dwMinorVersion == 1)
			{
				if (osvi.wServicePackMajor == 0)
					return osvi.wProductType == VER_NT_WORKSTATION ? oWINDOWS_7 : oWINDOWS_SERVER_2008R2;
				else if (osvi.wServicePackMajor == 1)
					return osvi.wProductType == VER_NT_WORKSTATION ? oWINDOWS_7_SP1 : oWINDOWS_SERVER_2008R2_SP1;
			}
			else if (osvi.dwMinorVersion == 0)
			{
				if (osvi.wServicePackMajor == 2)
					return osvi.wProductType == VER_NT_WORKSTATION ? oWINDOWS_VISTA_SP2 : oWINDOWS_SERVER_2008_SP2;
				else if (osvi.wServicePackMajor == 1)
					return osvi.wProductType == VER_NT_WORKSTATION ? oWINDOWS_VISTA_SP1 : oWINDOWS_SERVER_2008_SP1;
				else if (osvi.wServicePackMajor == 0)
					return osvi.wProductType == VER_NT_WORKSTATION ? oWINDOWS_VISTA : oWINDOWS_SERVER_2008;
			}
		}

		else if (osvi.dwMajorVersion == 5)
		{
			if (osvi.dwMinorVersion == 2)
			{
				SYSTEM_INFO si;
				GetSystemInfo(&si);
				if ((osvi.wProductType == VER_NT_WORKSTATION) && (si.wProcessorArchitecture==PROCESSOR_ARCHITECTURE_AMD64))
					return oWINDOWS_XP_PRO_64BIT;
				else if (osvi.wSuiteMask & 0x00008000 /*VER_SUITE_WH_SERVER*/)
					return oWINDOWS_HOME_SERVER;
				else
					return GetSystemMetrics(SM_SERVERR2) ? oWINDOWS_SERVER_2003R2 : oWINDOWS_SERVER_2003;
			}

			else if (osvi.dwMinorVersion == 1)
				return oWINDOWS_XP;
			else if (osvi.dwMinorVersion == 0)
				return oWINDOWS_2000;
		}
	}

	return oWINDOWS_UNKNOWN;
}

void oVerifyMinimumWindowsVersion( oWINDOWS_VERSION _Version )
{
	if( oGetWindowsVersion() < _Version )
	{
		oMsgBox(oMSGBOX_DESC(oMSGBOX_ERR, "Invalid Windows Version"), "%s or greater required.  Application will now terminate.", oStd::as_string(_Version));
		std::terminate();
	}
}

namespace oStd {

const char* as_string(const oWINDOWS_VERSION& _Version)
{
	switch (_Version)
	{
		case oWINDOWS_2000: return "Windows 2000";
		case oWINDOWS_XP: return "Windows XP";
		case oWINDOWS_XP_PRO_64BIT: return "Windows XP Pro 64-bit";
		case oWINDOWS_SERVER_2003: return "Windows Server 2003";
		case oWINDOWS_HOME_SERVER: return "Windows Home Server";
		case oWINDOWS_SERVER_2003R2: return "Windows Server 2003R2";
		case oWINDOWS_VISTA: return "Windows Vista";
		case oWINDOWS_SERVER_2008: return "Windows Server 2008";
		case oWINDOWS_SERVER_2008R2: return "Windows Server 2008R2";
		case oWINDOWS_7: return "Windows 7";
		case oWINDOWS_7_SP1: return "Windows 7 SP1";
		case oWINDOWS_UNKNOWN:
		default: break;
	}

	return "unknown Windows version";
}

} // namespace oStd

bool oConvertEnvStringToEnvBlock(char* _EnvBlock, size_t _SizeofEnvBlock, const char* _EnvString, char _Delimiter)
{
	if (!_EnvString || ((oStrlen(_EnvString)+1) > _SizeofEnvBlock))
	{
		oErrorSetLast(std::errc::invalid_argument, "EnvBlock buffer not large enough to contain converted EnvString");
		return false;
	}

	const char* r = _EnvString;
	char* w = _EnvBlock;
	while (1)
	{
		*w++ = *r == _Delimiter ? '\0' : *r;
		if (!*r)
			break;
		r++;
	}

	return true;
}

void oTaskbarGetRect(RECT* _pRect)
{
	APPBARDATA abd;
	abd.cbSize = sizeof(abd);
	SHAppBarMessage(ABM_GETTASKBARPOS, &abd);
	*_pRect = abd.rc;
}

int oWinGetDisplayDevice(HMONITOR _hMonitor, DISPLAY_DEVICE* _pDevice)
{
	MONITORINFOEX mi;
	mi.cbSize = sizeof(mi);
	if (GetMonitorInfo(_hMonitor, &mi))
	{
		_pDevice->cb = sizeof(DISPLAY_DEVICE);
		unsigned int index = 0;
		while (EnumDisplayDevices(0, index, _pDevice, 0))
		{
			if (!oStrcmp(mi.szDevice, _pDevice->DeviceName))
				return index;
			index++;
		}
	}

	return oInvalid;
}

void oGetVirtualDisplayRect(RECT* _pRect)
{
	_pRect->left = GetSystemMetrics(SM_XVIRTUALSCREEN);
	_pRect->top = GetSystemMetrics(SM_YVIRTUALSCREEN);
	_pRect->right = _pRect->left + GetSystemMetrics(SM_CXVIRTUALSCREEN);
	_pRect->bottom = _pRect->top + GetSystemMetrics(SM_CYVIRTUALSCREEN);
}

static bool oSetWindowsPrivileges(const char* _privilege)
{
	bool result = false;
	HANDLE hToken;
	TOKEN_PRIVILEGES tkp;
	if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken))
	{
		oWinSetLastError();
		return false;
	}

	if (LookupPrivilegeValue(nullptr, _privilege, &tkp.Privileges[0].Luid))
	{
		tkp.PrivilegeCount = 1;      
		tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED; 
		result = !!AdjustTokenPrivileges(hToken, FALSE, &tkp, 0, nullptr, 0);
	}

	if (!result)
		oWinSetLastError();

	// AdjustTokenPrivileges only fails for invalid parameters. If it fails because the app did not have permissions, then it succeeds (result will be true), but sets
	//	last error to ERROR_NOT_ALL_ASSIGNED.
	if(::GetLastError() != ERROR_SUCCESS) 
	{
		oErrorSetLast(std::errc::permission_denied, "Probably did not have permissions to set the requested privilege on this machine: %s", _privilege);
		result = false;
	}
	
	oVB(CloseHandle(hToken));
	return result;
}

static bool oWinAdjustPrivilegesForExitWindows()
{
	return oSetWindowsPrivileges(SE_SHUTDOWN_NAME);
}

bool oWinAddLargePagePrivileges()
{
	return oSetWindowsPrivileges(SE_LOCK_MEMORY_NAME);
}

// Adjusts privileges and calls ExitWindowsEx with the specified parameters
bool oWinExitWindows(UINT _uFlags, DWORD _dwReason)
{
	if (!oWinAdjustPrivilegesForExitWindows())
		return false;

	if (!ExitWindowsEx(_uFlags, _dwReason))
	{
		oWinSetLastError();
		return false;
	}

	return true;
}

const char** oWinCommandLineToArgvA(bool _ExePathAsArg0, const char* CmdLine, int* _argc)
{
	/** <citation
		usage="Implementation" 
		reason="Need an ASCII version of CommandLineToArgvW" 
		author="Alexander A. Telyatnikov"
		description="http://alter.org.ua/docs/win/args/"
		license="*** Assumed Public Domain ***"
		licenseurl="http://alter.org.ua/docs/win/args/"
		modification="Changed allocator, changes to get it to compile, add _ExePathAsArg0 functionality"
	/>*/
	// $(CitedCodeBegin)

	PCHAR* argv;
	PCHAR  _argv;
	ULONG   len;
	ULONG   argc;
	CHAR   a;
	ULONG   i, j;

	BOOLEAN  in_QM;
	BOOLEAN  in_TEXT;
	BOOLEAN  in_SPACE;

	len = (ULONG)oStrlen(CmdLine);
	if (_ExePathAsArg0) // @oooii
		len += MAX_PATH * sizeof(CHAR);

	i = ((len+2)/2)*sizeof(PVOID) + sizeof(PVOID);
	if (_ExePathAsArg0) // @oooii
		i += sizeof(PVOID);
	
	// @oooii-tony: change allocator from original code
	argv = (PCHAR*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, i + (len+2)*sizeof(CHAR));

	_argv = (PCHAR)(((PUCHAR)argv)+i);

	argc = 0;
	argv[argc] = _argv;
	in_QM = FALSE;
	in_TEXT = FALSE;
	in_SPACE = TRUE;
	i = 0;
	j = 0;

	// @oooii-tony: Optionally insert exe path so this is exactly like argc/argv
	if (_ExePathAsArg0)
	{
		oSystemGetPath(argv[argc], MAX_PATH, oSYSPATH_APP_FULL);
		oStd::clean_path(argv[argc], MAX_PATH, argv[argc], '\\');
		j = (ULONG)oStrlen(argv[argc])+1;
		argc++;
		argv[argc] = _argv+oStrlen(argv[0])+1;
	}

	while( (a = CmdLine[i]) != 0 ) {
		if(in_QM) {
			if(a == '\"') {
				in_QM = FALSE;
			} else {
				_argv[j] = a;
				j++;
			}
		} else {
			switch(a) {
			case '\"':
				in_QM = TRUE;
				in_TEXT = TRUE;
				if(in_SPACE) {
					argv[argc] = _argv+j;
					argc++;
				}
				in_SPACE = FALSE;
				break;
			case ' ':
			case '\t':
			case '\n':
			case '\r':
				if(in_TEXT) {
					_argv[j] = '\0';
					j++;
				}
				in_TEXT = FALSE;
				in_SPACE = TRUE;
				break;
			default:
				in_TEXT = TRUE;
				if(in_SPACE) {
					argv[argc] = _argv+j;
					argc++;
				}
				_argv[j] = a;
				j++;
				in_SPACE = FALSE;
				break;
			}
		}
		i++;
	}
	_argv[j] = '\0';
	argv[argc] = nullptr;

	(*_argc) = argc;
	return (const char**)argv;
}

void oWinCommandLineToArgvAFree(const char** _pArgv)
{
	HeapFree(GetProcessHeap(), 0, _pArgv);
}

void oWinGetVersion(const oVersion& _Version, DWORD* _pVersionMS, DWORD* _pVersionLS)
{
	*_pVersionMS = (_Version.Major << 16) | _Version.Minor;
	*_pVersionLS = (_Version.Build << 16) | _Version.Revision;
}

oVersion oWinGetVersion(DWORD _VersionMS, DWORD _VersionLS)
{
	oVersion v;
	v.Major = HIWORD(_VersionMS);
	v.Minor = LOWORD(_VersionMS);
	v.Build = HIWORD(_VersionLS);
	v.Revision = LOWORD(_VersionLS);
	return v;
}

void oGetScreenDPIScale(float* _pScaleX, float* _pScaleY)
{
	HDC screen = GetDC(0);
	*_pScaleX = GetDeviceCaps(screen, LOGPIXELSX) / 96.0f;
	*_pScaleY = GetDeviceCaps(screen, LOGPIXELSY) / 96.0f;
	ReleaseDC(0, screen);
}

// This singleton enusures only one thread is calling into LoadLibrary/FreeLibrary at a time
struct oLoadLibrarySingleton : oProcessSingleton<oLoadLibrarySingleton>
{
public:
	HMODULE ThreadsafeLoadLibrary(LPCTSTR _lpFileName)
	{
		oConcurrency::lock_guard<oConcurrency::recursive_mutex> Lock(Mutex);
		return LoadLibrary(_lpFileName);
	}
	BOOL ThreadsafeFreeLibrary(HMODULE _hModule)
	{
		oConcurrency::lock_guard<oConcurrency::recursive_mutex> Lock(Mutex);
		return FreeLibrary(_hModule);
	}
	static const oGUID GUID;

private:
	oConcurrency::recursive_mutex Mutex;
};

void oLoadLibrarySingletonCreate()
{
	oLoadLibrarySingleton::Singleton()->Reference();
}

void oLoadLibrarySingletonDestroy()
{
	oLoadLibrarySingleton::Singleton()->Release();
}

// {5E05E413-8A70-41D4-B058-3FE203D65841}
const oGUID oLoadLibrarySingleton::GUID = { 0x5e05e413, 0x8a70, 0x41d4, { 0xb0, 0x58, 0x3f, 0xe2, 0x3, 0xd6, 0x58, 0x41 } };
oSINGLETON_REGISTER(oLoadLibrarySingleton);

HMODULE oThreadsafeLoadLibrary(LPCTSTR _lpFileName)
{
	return oLoadLibrarySingleton::Singleton()->ThreadsafeLoadLibrary(_lpFileName);
}

BOOL oThreadsafeFreeLibrary(HMODULE _hModule)
{
	return oLoadLibrarySingleton::Singleton()->ThreadsafeFreeLibrary(_hModule);
}

// {EE17C49C-A27A-45F0-B2C8-D049E0ECD3C6}
const oGUID guid = { 0xee17c49c, 0xa27a, 0x45f0, { 0xb2, 0xc8, 0xd0, 0x49, 0xe0, 0xec, 0xd3, 0xc6 } };
oDEFINE_STATIC_MUTEX(oConcurrency::recursive_mutex, sOutputDebugStringMutex, guid);

void oThreadsafeOutputDebugStringA(const char* _OutputString)
{
	oConcurrency::lock_guard<oStaticMutex<oConcurrency::recursive_mutex, sOutputDebugStringMutex_t> > Lock(sOutputDebugStringMutex);
	OutputDebugStringA(_OutputString);
}

bool oWaitSingle(HANDLE _Handle, unsigned int _TimeoutMS)
{
	return WAIT_OBJECT_0 == ::WaitForSingleObject(_Handle, _TimeoutMS == oInfiniteWait ? INFINITE : _TimeoutMS);
}

bool oWaitMultiple(HANDLE* _pHandles, size_t _NumberOfHandles, size_t* _pWaitBreakingIndex, unsigned int _TimeoutMS)
{
	oASSERT(_NumberOfHandles <= 64, "Windows has a limit of 64 handles that can be waited on by WaitForMultipleObjects");
	DWORD result = ::WaitForMultipleObjects(static_cast<DWORD>(_NumberOfHandles), _pHandles, !_pWaitBreakingIndex, _TimeoutMS == oInfiniteWait ? INFINITE : _TimeoutMS);

	if (result == WAIT_FAILED)
	{
		oWinSetLastError();
		return false;
	}

	else if (result == WAIT_TIMEOUT)
	{
		oErrorSetLast(std::errc::timed_out);
		return false;
	}

	else if (_pWaitBreakingIndex)
	{
		if (result >= WAIT_ABANDONED_0) 
			*_pWaitBreakingIndex = result - WAIT_ABANDONED_0;
		else
			*_pWaitBreakingIndex = result - WAIT_OBJECT_0;
	}

	return true;
}

bool oWaitSingle(DWORD _ThreadID, unsigned int _Timeout)
{
	HANDLE hThread = OpenThread(SYNCHRONIZE, FALSE, _ThreadID);
	bool result = oWaitSingle(hThread, _Timeout);
	oVB(CloseHandle(hThread));
	return result;
}

bool oWaitMultiple(DWORD* _pThreadIDs, size_t _NumberOfThreadIDs, size_t* _pWaitBreakingIndex, unsigned int _Timeout)
{
	if (!_pThreadIDs || !_NumberOfThreadIDs || _NumberOfThreadIDs >= 64)
	{
		oErrorSetLast(std::errc::invalid_argument);
		return false;
	}

	HANDLE hThreads[64]; // max of windows anyway
	for (size_t i = 0; i < _NumberOfThreadIDs; i++)
		hThreads[i] = OpenThread(SYNCHRONIZE, FALSE, _pThreadIDs[i]);

	bool result = oWaitMultiple(hThreads, _NumberOfThreadIDs, _pWaitBreakingIndex, _Timeout);
	
	for (size_t i = 0; i < _NumberOfThreadIDs; i++)
		oVB(CloseHandle(hThreads[i]));

	return result;
}

HANDLE oGetFileHandle(FILE* _File)
{
	return (HANDLE)_get_osfhandle(_fileno(_File));
}

DWORD oGetThreadID(HANDLE _hThread)
{
	#if oDXVER >= oDXVER_10
		DWORD ID = 0;
		if (!_hThread)
			ID = GetCurrentThreadId();
		else if (oGetWindowsVersion() >= oWINDOWS_VISTA)
			ID = oWinKernel32::Singleton()->GetThreadId((HANDLE)_hThread);
		else
			oTRACE("WARNING: oGetThreadID doesn't work with non-zero thread handles on versions of Windows prior to Vista.");
			// todo: Traverse all threads in the system to find the one with the specified handle, then from that struct return the ID.

		return ID;
	#else
		oTRACE("oGetThreadID doesn't behave properly on versions of Windows prior to Vista because GetThreadId(HANDLE _hThread) didn't exist.");
		return 0;
	#endif
}

bool oWinGetWorkgroupName(char* _StrDestination, size_t _SizeofStrDestination)
{
	LPWKSTA_INFO_102 pInfo = nullptr;
	NET_API_STATUS nStatus;
	LPTSTR pszServerName = nullptr;

	nStatus = oWinNetApi32::Singleton()->NetWkstaGetInfo(nullptr, 102, (LPBYTE *)&pInfo);
	oStd::finally OSCFreeBuffer([&] { if (pInfo) oWinNetApi32::Singleton()->NetApiBufferFree(pInfo); });
	if (nStatus == NERR_Success)
	{
		WideCharToMultiByte(CP_ACP, 0, pInfo->wki102_langroup, -1, _StrDestination, oInt(_SizeofStrDestination), 0, 0);
		return true;
	}

	return false;
}

HMODULE oGetModule(void* _ModuleFunctionPointer)
{
	HMODULE hModule = 0;

	if (_ModuleFunctionPointer)
		oVB(GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT, (LPCTSTR)_ModuleFunctionPointer, &hModule));	
	else
	{
		DWORD cbNeeded;
		oWinPSAPI::Singleton()->EnumProcessModules(GetCurrentProcess(), &hModule, sizeof(hModule), &cbNeeded);
	}
	
	return hModule;
}

bool oWinGetProcessID(const char* _Name, DWORD* _pOutProcessID)
{
	// From http://msdn.microsoft.com/en-us/library/ms682623(v=VS.85).aspx
	// Get the list of process identifiers.

	*_pOutProcessID = 0;
	oWinPSAPI* pPSAPI = oWinPSAPI::Singleton();

	DWORD ProcessIDs[1024], cbNeeded;
	if (!pPSAPI->EnumProcesses(ProcessIDs, sizeof(ProcessIDs), &cbNeeded))
	{
		oWinSetLastError();
		return false;
	}

	bool truncationResult = true;
	const size_t nProcessIDs = cbNeeded / sizeof(DWORD);

	for (size_t i = 0; i < nProcessIDs; i++)
	{
		if (ProcessIDs[i] != 0)
		{
			HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION|PROCESS_VM_READ|PROCESS_DUP_HANDLE, FALSE, ProcessIDs[i]);
			if (hProcess)
			{
				HMODULE hMod;
				DWORD cbNeeded;

				char szProcessName[MAX_PATH];
				if (pPSAPI->EnumProcessModules(hProcess, &hMod, sizeof(hMod), &cbNeeded))
				{
					pPSAPI->GetModuleBaseNameA(hProcess, hMod, szProcessName, sizeof(szProcessName)/sizeof(char));
					CloseHandle(hProcess);
					if (oStricmp(szProcessName, _Name) == 0)
					{
						*_pOutProcessID = ProcessIDs[i];
						return true;
					}
				}
			}
		}
	}

	if (sizeof(ProcessIDs) == cbNeeded)
	{
		oErrorSetLast(std::errc::no_buffer_space, "There are more than %u processes on the system currently, oWinGetProcessID needs to be more general-purpose.", oCOUNTOF(ProcessIDs));
		truncationResult = false;
	}

	else
		oErrorSetLast(std::errc::no_such_process);

	return false;
}

bool oEnumProcessThreads(DWORD _ProcessID, oFUNCTION<bool(DWORD _ThreadID, DWORD _ParentProcessID)> _Function)
{
	HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, GetCurrentProcessId());
	if (hSnapshot == INVALID_HANDLE_VALUE)
	{
		oErrorSetLast(std::errc::invalid_argument, "Failed to get a snapshot of current process");
		return false;
	}

	THREADENTRY32 entry;
	entry.dwSize = sizeof(THREADENTRY32);

	BOOL keepLooking = Thread32First(hSnapshot, &entry);

	if (!keepLooking)
	{
		oErrorSetLast(std::errc::no_such_process, "no process threads found");
		return false;
	}

	while (keepLooking)
	{
		if (_ProcessID == entry.th32OwnerProcessID && _Function)
			if (!_Function(entry.th32ThreadID, entry.th32OwnerProcessID))
				break;
		keepLooking = Thread32Next(hSnapshot, &entry);
	}

	oVB(CloseHandle(hSnapshot));
	return true;
}

bool oWinEnumProcesses(oFUNCTION<bool(DWORD _ProcessID, DWORD _ParentProcessID, const char* _ProcessExePath)> _Function)
{
	if (!_Function)
	{
		oErrorSetLast(std::errc::invalid_argument);
		return false;
	}

	HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (hSnapshot == INVALID_HANDLE_VALUE)
		return oErrorSetLast(std::errc::invalid_argument, "Failed to get a snapshot of current process");

	DWORD ppid = 0;

	PROCESSENTRY32 entry;
	entry.dwSize = sizeof(entry);
	BOOL keepLooking = Process32First(hSnapshot, &entry);

	if (!keepLooking)
		return oErrorSetLast(std::errc::no_child_process);

	while (keepLooking)
	{
		if (!_Function(entry.th32ProcessID, entry.th32ParentProcessID, entry.szExeFile))
			break;
		entry.dwSize = sizeof(entry);
		keepLooking = !ppid && Process32Next(hSnapshot, &entry);
	}

	oVB(CloseHandle(hSnapshot));
	return true;
}

BOOL CALLBACK oWinEnumWindowsProc(HWND _hWnd, LPARAM _lParam)
{
	return (*(oFUNCTION<bool(HWND _Hwnd)>*)_lParam)(_hWnd) ? TRUE : FALSE;
}

bool oWinEnumWindows( oFUNCTION<bool(HWND _Hwnd)> _Function )
{
	void* pFunc = &_Function;
	return TRUE == EnumWindows((WNDENUMPROC)oWinEnumWindowsProc, (LPARAM)pFunc);
}

#pragma pack(push,8)
typedef struct tagTHREADNAME_INFO
{
   DWORD dwType; // Must be 0x1000.
   LPCSTR szName; // Pointer to name (in user addr space).
   DWORD dwThreadID; // Thread ID (-1=caller thread).
   DWORD dwFlags; // Reserved for future use, must be zero.
} THREADNAME_INFO;
#pragma pack(pop)

bool oIsWindows64Bit()
{
	if (sizeof(void*) != 4) // If ptr size is larger than 32-bit we must be on 64-bit windows
		return true;

	// If ptr size is 4 bytes then we're a 32-bit process so check if we're running under
	// wow64 which would indicate that we're on a 64-bit system
	BOOL bWow64 = FALSE;
	IsWow64Process(GetCurrentProcess(), &bWow64);
	return !bWow64;
}

bool oIsAeroEnabled()
{
	BOOL enabled = FALSE;
	oV(oWinDWMAPI::Singleton()->DwmIsCompositionEnabled(&enabled));
	return !!enabled;
}

bool oEnableAero(bool _Enabled, bool _Force)
{
	if (_Enabled && _Force && !oIsAeroEnabled())
	{
		system("%SystemRoot%\\system32\\rundll32.exe %SystemRoot%\\system32\\shell32.dll,Control_RunDLL %SystemRoot%\\system32\\desk.cpl desk,@Themes /Action:OpenTheme /file:\"C:\\Windows\\Resources\\Themes\\aero.theme\"");
		oSleep(31000); // Windows takes about 30 sec to settle after doing this.
	}
	else	
		oVB_RETURN2(oWinDWMAPI::Singleton()->DwmEnableComposition(_Enabled ? DWM_EC_ENABLECOMPOSITION : DWM_EC_DISABLECOMPOSITION));
	
	return true;
}

bool oIsRemoteDesktopConnected()
{
	return GetSystemMetrics(SM_REMOTESESSION) != 0;
}

static WORD oDlgGetClass(oWINDOWS_DIALOG_ITEM_TYPE _Type)
{
	switch (_Type)
	{
		case oDLG_BUTTON: return 0x0080;
		case oDLG_EDITBOX: return 0x0081;
		case oDLG_LABEL_LEFT_ALIGNED: return 0x0082;
		case oDLG_LABEL_CENTERED: return 0x0082;
		case oDLG_LABEL_RIGHT_ALIGNED: return 0x0082;
		case oDLG_LARGELABEL: return 0x0081; // an editbox with a particular style set
		case oDLG_ICON: return 0x0082; // (it's really a label with no text and a picture with SS_ICON style)
		case oDLG_LISTBOX: return 0x0083;
		case oDLG_SCROLLBAR: return 0x0084;
		case oDLG_COMBOBOX: return 0x0085;
		oNODEFAULT;
	}
}

static size_t oDlgCalcTextSize(const char* _Text)
{
	return sizeof(WCHAR) * (oStrlen(_Text) + 1);
}

static size_t oDlgCalcTemplateSize(const char* _MenuName, const char* _ClassName, const char* _Caption, const char* _FontName)
{
	size_t size = sizeof(DLGTEMPLATE) + sizeof(WORD); // extra word for font size if we specify it
	size += oDlgCalcTextSize(_MenuName);
	size += oDlgCalcTextSize(_ClassName);
	size += oDlgCalcTextSize(_Caption);
	size += oDlgCalcTextSize(_FontName);
	return oStd::byte_align(size, sizeof(DWORD)); // align to DWORD because dialog items need DWORD alignment, so make up for any padding here
}

static size_t oDlgCalcTemplateItemSize(const char* _Text)
{
	return oStd::byte_align(sizeof(DLGITEMTEMPLATE) +
		2 * sizeof(WORD) + // will keep it simple 0xFFFF and the ControlClass
		oDlgCalcTextSize(_Text) + // text
		sizeof(WORD), // 0x0000. This is used for data to be passed to WM_CREATE, bypass this for now
		sizeof(DWORD)); // in size calculations, ensure everything is DWORD-aligned
}

static WORD* oDlgCopyString(WORD* _pDestination, const char* _String)
{
	if (!_String) _String = "";
	size_t len = oStrlen(_String);
	oStrcpy((WCHAR*)_pDestination, len+1, _String);
	return oStd::byte_add(_pDestination, (len+1) * sizeof(WCHAR));
}

// Returns a DWORD-aligned pointer to where to write the first item
static LPDLGITEMTEMPLATE oDlgInitialize(const oWINDOWS_DIALOG_DESC& _Desc, LPDLGTEMPLATE _lpDlgTemplate)
{
	// Set up the dialog box itself
	DWORD style = WS_POPUP|DS_MODALFRAME;
	if (_Desc.SetForeground) style |= DS_SETFOREGROUND;
	if (_Desc.Center) style |= DS_CENTER;
	if (_Desc.Caption) style |= WS_CAPTION;
	if (_Desc.Font && *_Desc.Font) style |= DS_SETFONT;
	if (_Desc.Visible) style |= WS_VISIBLE;
	if (!_Desc.Enabled) style |= WS_DISABLED;

	_lpDlgTemplate->style = style;
	_lpDlgTemplate->dwExtendedStyle = _Desc.AlwaysOnTop ? WS_EX_TOPMOST : 0;
	_lpDlgTemplate->cdit = WORD(_Desc.NumItems);
	_lpDlgTemplate->x = 0;
	_lpDlgTemplate->y = 0;
	_lpDlgTemplate->cx = short(oWinRectW(_Desc.Rect));
	_lpDlgTemplate->cy = short(oWinRectH(_Desc.Rect));

	// Fill in dialog data menu, class, caption and font
	WORD* p = (WORD*)(_lpDlgTemplate+1);
	p = oDlgCopyString(p, nullptr); // no menu
	p = oDlgCopyString(p, nullptr); // use default class
	p = oDlgCopyString(p, _Desc.Caption);
	
	if (style & DS_SETFONT)
	{
		*p++ = WORD(_Desc.FontPointSize);
		p = oDlgCopyString(p, _Desc.Font);
	}

	return (LPDLGITEMTEMPLATE)oStd::byte_align(p, sizeof(DWORD));
}
#pragma warning(disable:4505)
// Returns a DWORD-aligned pointer to the next place to write a DLGITEMTEMPLATE
static LPDLGITEMTEMPLATE oDlgItemInitialize(const oWINDOWS_DIALOG_ITEM& _Desc, LPDLGITEMTEMPLATE _lpDlgItemTemplate)
{
	DWORD style = WS_CHILD;
	if (!_Desc.Enabled) style |= WS_DISABLED;
	if (_Desc.Visible) style |= WS_VISIBLE;
	if (_Desc.TabStop) style |= WS_TABSTOP;
	
	switch (_Desc.Type)
	{
		case oDLG_ICON: style |= SS_ICON; break;
		case oDLG_LABEL_LEFT_ALIGNED: style |= SS_LEFT|SS_ENDELLIPSIS; break;
		case oDLG_LABEL_CENTERED: style |= SS_CENTER|SS_ENDELLIPSIS; break;
		case oDLG_LABEL_RIGHT_ALIGNED: style |= SS_RIGHT|SS_ENDELLIPSIS; break;
		case oDLG_LARGELABEL: style |= ES_LEFT|ES_READONLY|ES_MULTILINE|WS_VSCROLL; break;
		default: break;
	}
	
	_lpDlgItemTemplate->style = style;
	_lpDlgItemTemplate->dwExtendedStyle = WS_EX_NOPARENTNOTIFY;
	_lpDlgItemTemplate->x = short(_Desc.Rect.left);
	_lpDlgItemTemplate->y = short(_Desc.Rect.top);
	_lpDlgItemTemplate->cx = short(oWinRectW(_Desc.Rect));
	_lpDlgItemTemplate->cy = short(oWinRectH(_Desc.Rect));
	_lpDlgItemTemplate->id = _Desc.ItemID;
	WORD* p = oStd::byte_add((WORD*)_lpDlgItemTemplate, sizeof(DLGITEMTEMPLATE));
	*p++ = 0xFFFF; // flag that a simple ControlClass is to follow
	*p++ = oDlgGetClass(_Desc.Type);
	p = oDlgCopyString(p, _Desc.Text);
	*p++ = 0x0000; // no WM_CREATE data
	return (LPDLGITEMTEMPLATE)oStd::byte_align(p, sizeof(DWORD));
}

void oDlgDeleteTemplate(LPDLGTEMPLATE _lpDlgTemplate)
{
	delete [] _lpDlgTemplate;
}

LPDLGTEMPLATE oDlgNewTemplate(const oWINDOWS_DIALOG_DESC& _Desc)
{
	size_t templateSize = oDlgCalcTemplateSize(nullptr, nullptr, _Desc.Caption, _Desc.Font);
	for (UINT i = 0; i < _Desc.NumItems; i++)
		templateSize += oDlgCalcTemplateItemSize(_Desc.pItems[i].Text);

	LPDLGTEMPLATE lpDlgTemplate = (LPDLGTEMPLATE)new char[templateSize];
	LPDLGITEMTEMPLATE lpItem = oDlgInitialize(_Desc, lpDlgTemplate);
	
	for (UINT i = 0; i < _Desc.NumItems; i++)
		lpItem = oDlgItemInitialize(_Desc.pItems[i], lpItem);

	return lpDlgTemplate;
}

// NVIDIA's version string is of the form "x.xx.xM.MMmm" where
// MMM is the major version and mm is the minor version.
// (outside function below so this doesn't get tracked as a leak)
static std::regex reNVVersionString("[0-9]+\\.[0-9]+\\.[0-9]+([0-9])\\.([0-9][0-9])([0-9]+)");

static bool ParseVersionStringNV(const char* _VersionString, oVersion* _pVersion)
{
	if (!_VersionString || !_pVersion)
		return oErrorSetLast(std::errc::invalid_argument);

	std::cmatch matches;
	if (!regex_match(_VersionString, matches, reNVVersionString))
		return oErrorSetLast(std::errc::invalid_argument, "The specified string \"%s\" is not a well-formed NVIDIA version string", oSAFESTRN(_VersionString));

	char major[4];
	major[0] = *matches[1].first;
	major[1] = *matches[2].first;
	major[2] = *(matches[2].first+1);
	major[3] = 0;
	*_pVersion = oVersion(oUShort(atoi(major)), oUShort(atoi(matches[3].first)));
	return true;
}

// AMD's version string is of the form M.mm.x.x where
// M is the major version and mm is the minor version.
// (outside function below so this doesn't get tracked as a leak)
static std::regex reAMDVersionString("([0-9]+)\\.([0-9]+)\\.[0-9]+\\.[0-9]+");

static bool ParseVersionStringAMD(const char* _VersionString, oVersion* _pVersion)
{
	if (!_VersionString || !_pVersion)
		return oErrorSetLast(std::errc::invalid_argument);

	std::cmatch matches;
	if (!regex_match(_VersionString, matches, reAMDVersionString))
		return oErrorSetLast(std::errc::invalid_argument, "The specified string \"%s\" is not a well-formed AMD version string", oSAFESTRN(_VersionString));

	*_pVersion = oVersion(oUShort(atoi(matches[1].first)), oUShort(atoi(matches[2].first)));
	return true;
}

static bool ParseVersionStringIntel(const char* _VersionString, oVersion* _pVersion)
{
	// @oooii-tony: This initial version was done based on Dave's laptop which 
	// appears to have AMD-like drivers... so use the same parsing for now...
	return ParseVersionStringAMD(_VersionString, _pVersion);
}

bool oWinEnumVideoDriverDesc(oFUNCTION<void(const oDISPLAY_ADAPTER_DRIVER_DESC& _Desc)> _Enumerator)
{
	// redefine some MS GUIDs so we don't have to link to them

	// 4590F811-1D3A-11D0-891F-00AA004B2E24
	static const oGUID oGUID_CLSID_WbemLocator = { 0x4590F811, 0x1D3A, 0x11D0, { 0x89, 0x1F, 0x00, 0xAA, 0x00, 0x4B, 0x2E, 0x24 } };

	// DC12A687-737F-11CF-884D-00AA004B2E24
	static const oGUID oGUID_IID_WbemLocator = { 0xdc12a687, 0x737f, 0x11cf, { 0x88, 0x4D, 0x00, 0xAA, 0x00, 0x4B, 0x2E, 0x24 } };

	CoInitializeEx(nullptr, COINIT_MULTITHREADED);
	oStd::finally OnScopeExit([&] { CoUninitialize(); });

	oRef<IWbemLocator> WbemLocator;
	oV(CoCreateInstance((const GUID&)oGUID_CLSID_WbemLocator, 0, CLSCTX_INPROC_SERVER, (const IID&)oGUID_IID_WbemLocator, (LPVOID *) &WbemLocator));

	oRef<IWbemServices> WbemServices;
	oV(WbemLocator->ConnectServer(_bstr_t(L"ROOT\\CIMV2"), NULL, NULL, 0, NULL, 0, 0, &WbemServices));
	oV(CoSetProxyBlanket(WbemServices, RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE, nullptr, RPC_C_AUTHN_LEVEL_CALL, RPC_C_IMP_LEVEL_IMPERSONATE, nullptr, EOAC_NONE));
	
	oRef<IEnumWbemClassObject> Enumerator;
	oV(WbemServices->ExecQuery(bstr_t("WQL"), bstr_t("SELECT * FROM Win32_VideoController"), WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY, NULL, &Enumerator));

	oRef<IWbemClassObject> WbemClassObject;
	while (Enumerator)
	{
		oDISPLAY_ADAPTER_DRIVER_DESC desc;
		memset(&desc, 0, sizeof(oDISPLAY_ADAPTER_DRIVER_DESC));

		ULONG uReturn = 0;
		WbemClassObject = nullptr;
		oV(Enumerator->Next(WBEM_INFINITE, 1, &WbemClassObject, &uReturn));
		if (0 == uReturn)
			break;

		VARIANT vtProp;
		oV(WbemClassObject->Get(L"Description", 0, &vtProp, 0, 0));
		desc.Description = vtProp.bstrVal;
		VariantClear(&vtProp);
		oV(WbemClassObject->Get(L"PNPDeviceID", 0, &vtProp, 0, 0));
		desc.PlugNPlayID = vtProp.bstrVal;
		VariantClear(&vtProp);
		oV(WbemClassObject->Get(L"DriverVersion", 0, &vtProp, 0, 0));

		oStd::sstring version = vtProp.bstrVal;
		if (strstr(desc.Description, "NVIDIA"))
		{
			desc.Vendor = oGPU_VENDOR_NVIDIA;
			oVERIFY(ParseVersionStringNV(version, &desc.Version));
		}
		
		else if (strstr(desc.Description, "ATI") || strstr(desc.Description, "AMD"))
		{
			desc.Vendor = oGPU_VENDOR_AMD;
			oVERIFY(ParseVersionStringAMD(version, &desc.Version));
		}

		else if (strstr(desc.Description, "Intel"))
		{
			desc.Vendor = oGPU_VENDOR_INTEL;
			oVERIFY(ParseVersionStringIntel(version, &desc.Version));
		}

		else
		{
			desc.Vendor = oGPU_VENDOR_UNKNOWN;
			desc.Version = oVersion();
		}

		_Enumerator(desc);
	}
	
	return true;
}

bool oWinServicesEnum(oFUNCTION<bool(SC_HANDLE _hSCManager, const ENUM_SERVICE_STATUS_PROCESS& _Status)> _Enumerator)
{
	SC_HANDLE hSCManager = OpenSCManager(nullptr, nullptr, SC_MANAGER_ENUMERATE_SERVICE);
	if (!hSCManager)
	{
		oWinSetLastError();
		return false;
	}

	DWORD requiredBytes = 0;
	DWORD nServices = 0;
	EnumServicesStatusEx(hSCManager, SC_ENUM_PROCESS_INFO, SERVICE_TYPE_ALL, SERVICE_STATE_ALL, nullptr, 0, &requiredBytes, &nServices, nullptr, nullptr);
	oASSERT(GetLastError() == ERROR_MORE_DATA, "");

	ENUM_SERVICE_STATUS_PROCESS* lpServices = (ENUM_SERVICE_STATUS_PROCESS*)malloc(requiredBytes);

	bool result = false;
	if (EnumServicesStatusEx(hSCManager, SC_ENUM_PROCESS_INFO, SERVICE_TYPE_ALL, SERVICE_STATE_ALL, (LPBYTE)lpServices, requiredBytes, &requiredBytes, &nServices, nullptr, nullptr))
	{
		for (DWORD i = 0; i < nServices; i++)
			if (!_Enumerator(hSCManager, lpServices[i]))
				break;
				
		result = true;
	}
	
	else
		oWinSetLastError();

	free(lpServices);
	if (!CloseServiceHandle(hSCManager))
	{
		oWinSetLastError();
		return false;
	}

	return result;
}

bool oWinGetServiceBinaryPath(char* _StrDestination, size_t _SizeofStrDestination, SC_HANDLE _hSCManager, const char* _ServiceName)
{
	if (!_StrDestination || !_SizeofStrDestination || !_hSCManager || !_ServiceName || !*_ServiceName)
	{
		oErrorSetLast(std::errc::invalid_argument);
		return false;
	}

	*_StrDestination = 0;
	SC_HANDLE hService = OpenService(_hSCManager, _ServiceName, SERVICE_ALL_ACCESS);
	if (!hService)
	{
		oWinSetLastError();
		return false;
	}

	DWORD dwSize = 0;
	QueryServiceConfig(hService, nullptr, 0, &dwSize);
	oASSERT(GetLastError() == ERROR_INSUFFICIENT_BUFFER, "");

	QUERY_SERVICE_CONFIG* pQSC = (QUERY_SERVICE_CONFIG*)malloc(dwSize);

	bool result = false;
	if (QueryServiceConfig(hService, pQSC, dwSize, &dwSize))
	{
		// Based on:
		// http://www.codeproject.com/KB/system/SigmaDriverList.aspx

		// There's some env vars to resolve, so prepare for that
		oStd::path_string SystemRootPath;
		GetEnvironmentVariable("SYSTEMROOT", SystemRootPath.c_str(), oUInt(SystemRootPath.capacity()));

		oStrcpy(_StrDestination, _SizeofStrDestination, pQSC->lpBinaryPathName);
		char* lpFirstOccurance = nullptr;
		lpFirstOccurance = strstr(_StrDestination, "\\SystemRoot");
		if (oStrlen(_StrDestination) > 0 && lpFirstOccurance)
		{
			// replace \SysteRoot with system root value obtained by
			// calling GetEnvironmentVariable() to get SYSTEMROOT
			lpFirstOccurance += oStrlen("\\SystemRoot");
			oPrintf(_StrDestination, _SizeofStrDestination, "%s%s", SystemRootPath.c_str(), lpFirstOccurance);
		}

		else
		{
			lpFirstOccurance = nullptr;
			lpFirstOccurance = strstr(_StrDestination, "\\??\\");
			if (oStrlen(_StrDestination) > 0 && lpFirstOccurance != nullptr)
			{
				// Remove \??\ at the starting of binary path
				lpFirstOccurance += oStrlen("\\??\\");
				oPrintf(_StrDestination, _SizeofStrDestination, "%s", lpFirstOccurance);
			}

			else
			{
				lpFirstOccurance = nullptr;
				lpFirstOccurance = strstr(_StrDestination, "system32");
				if (oStrlen(_StrDestination) > 0 && lpFirstOccurance && _StrDestination == lpFirstOccurance)
				{
					// Add SYSTEMROOT value if there is only system32 mentioned in binary path
					oStd::path_string temp;
					oPrintf(temp.c_str(), temp.capacity(), "%s\\%s", SystemRootPath.c_str(), _StrDestination); 
					oStrcpy(_StrDestination, _SizeofStrDestination, temp);
				}

				else
				{
					lpFirstOccurance = nullptr;
					lpFirstOccurance = strstr(_StrDestination, "System32");
					if (oStrlen(_StrDestination) > 0 && lpFirstOccurance && _StrDestination == lpFirstOccurance)
					{
						// Add SYSTEMROOT value if there is only System32 mentioned in binary path
						oStd::path_string temp;
						oPrintf(temp.c_str(), temp.capacity(), "%s\\%s", SystemRootPath.c_str(), _StrDestination); 
						oStrcpy(_StrDestination, _SizeofStrDestination, temp); 
					}
				}
			}
		}

		result = true;
	}

	else
		oWinSetLastError();

	free(pQSC);
	CloseServiceHandle(hService);
	return result;
}

static bool CheckStatusForNonSteady(SC_HANDLE _hSCManager, const ENUM_SERVICE_STATUS_PROCESS& _Status, bool* _pNonSteadyFound)
{
	if (SERVICE_PAUSED != _Status.ServiceStatusProcess.dwCurrentState && SERVICE_RUNNING != _Status.ServiceStatusProcess.dwCurrentState && SERVICE_STOPPED != _Status.ServiceStatusProcess.dwCurrentState)
	{
		*_pNonSteadyFound = true;
		return false;
	}

	return true;
}

bool oWinSystemAllServicesInSteadyState()
{
	bool NonSteadyStateFound = false;
	if (!oWinServicesEnum(oBIND(CheckStatusForNonSteady, oBIND1, oBIND2, &NonSteadyStateFound)))
		return false;
	return !NonSteadyStateFound;
}

double oWinSystemCalculateCPUUsage(unsigned long long* _pPreviousIdleTime, unsigned long long* _pPreviousSystemTime)
{
	double CPUUsage = 0.0;

	unsigned long long idleTime, kernelTime, userTime;
	oVB(GetSystemTimes((LPFILETIME)&idleTime, (LPFILETIME)&kernelTime, (LPFILETIME)&userTime));
	unsigned long long systemTime = kernelTime + userTime;
	
	if (*_pPreviousIdleTime && *_pPreviousSystemTime)
	{
		unsigned long long idleDelta = idleTime - *_pPreviousIdleTime;
		unsigned long long systemDelta = systemTime - *_pPreviousSystemTime;
		CPUUsage = (systemDelta - idleDelta) * 100.0 / (double)systemDelta;
	}		

	*_pPreviousIdleTime = idleTime;
	*_pPreviousSystemTime = systemTime;
	return CPUUsage; 
}

bool oWinSystemOpenDocument(const char* _DocumentName, bool _ForEdit)
{
	int hr = (int)ShellExecuteA(nullptr, _ForEdit ? "edit" : "open", _DocumentName, nullptr, nullptr, SW_SHOW);

	if (32 > hr)
	{
		if (hr == 0)
			return oErrorSetLast(std::errc::no_buffer_space, "The operating system is out of memory or resources.");
		return oWinSetLastError();
	}

	return true;
}

bool oWinGetProcessTopWindowAndThread(unsigned int _ProcessID, HWND* _pHWND, unsigned int* _pThreadID, const char* _pOptionalWindowName /*= nullptr*/)
{
	HWND Hwnd = 0;
	unsigned int ThreadID = 0;

	oWinEnumWindows(
		[&](HWND _Hwnd)->bool
	{
		unsigned int HWNDProcessID;
		auto NewThreadID = GetWindowThreadProcessId(_Hwnd, (DWORD*)&HWNDProcessID);

		if(HWNDProcessID != _ProcessID)
			return true;

		// Look for windows that might get injected into our process that we would never want to target
		const char* WindowsToSkip[] = {"Default IME", "MSCTFIME UI"};
		oStd::mstring WindowText;
		// For some reason in release builds Default IME can take a while to respond to this. so wait longer unless the app appears to be hung.
		if (SendMessageTimeout(_Hwnd, WM_GETTEXT, oStd::mstring::Capacity - 1, (LPARAM)WindowText.c_str(), SMTO_ABORTIFHUNG, 2000, nullptr) == 0)
		{
			WindowText = "";
		}

		if (!WindowText.empty())
		{
			oFORI(i, WindowsToSkip)
			{
				if( 0 == oStrcmp(WindowText.c_str(), WindowsToSkip[i]) )
				{
					return true;
				}
			}
		}

		// If the caller specified a window name make certain this is the right window or else keep searching
		if(_pOptionalWindowName && 0 != oStrcmp(_pOptionalWindowName, WindowText))
			return true;

		Hwnd = _Hwnd;
		ThreadID = NewThreadID;
		return false;
	});

	if(!Hwnd)
		return oErrorSetLast(std::errc::no_such_process);

	*_pHWND = Hwnd;
	*_pThreadID = ThreadID;
	return true;
}

bool oWinSystemIs64BitBinary(const char* _StrPath)
{
	bool result = false;
	void* mapped = nullptr;

	oSTREAM_DESC FDesc;
	if (!oStreamGetDesc(_StrPath, &FDesc))
		return false; // pass through error

	if (oFileMap(_StrPath, false, oSTREAM_RANGE(FDesc), &mapped))
	{
		IMAGE_NT_HEADERS* pHeader = oWinDbgHelp::Singleton()->ImageNtHeader(mapped);
		result = pHeader->FileHeader.Machine == IMAGE_FILE_MACHINE_AMD64;
		oVERIFY(oFileUnmap(mapped));
	}

	return result;
}

bool oWinSetPrivilege(HANDLE _hProcessToken, LPCTSTR _PrivilegeName, bool _Enabled)
{
	TOKEN_PRIVILEGES TP;
	LUID luid;
	if (!LookupPrivilegeValue(nullptr, _PrivilegeName, &luid))
		return oWinSetLastError();
	TP.PrivilegeCount = 1;
	TP.Privileges[0].Luid = luid;
	TP.Privileges[0].Attributes = _Enabled ? SE_PRIVILEGE_ENABLED : 0;

	if (!AdjustTokenPrivileges(_hProcessToken, FALSE, &TP, sizeof(TP), nullptr, nullptr))
		return oWinSetLastError();

	if (GetLastError() == ERROR_NOT_ALL_ASSIGNED)
		return oWinSetLastError();

	return true;
}

bool oWinEnableDebugPrivilege(bool _Enabled)
{
	HANDLE hToken = nullptr;
	if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES, &hToken))
		return oWinSetLastError();
	
	if (!oWinSetPrivilege(hToken, SE_DEBUG_NAME, true))
		return false; // pass through error

	if (!CloseHandle(hToken))
		return oWinSetLastError();
	
	return true;
}

long oWinRCGetFileFlags(const oMODULE_DESC& _Desc)
{
	long flags = 0;
	if (_Desc.IsDebugBuild)
		flags |= VS_FF_DEBUG;
	if (_Desc.IsPrereleaseBuild)
		flags |= VS_FF_PRERELEASE;
	if (_Desc.IsPatchedBuild)
		flags |= VS_FF_PATCHED;
	if (_Desc.IsPrivateBuild)
		flags |= VS_FF_PRIVATEBUILD;
	if (_Desc.IsSpecialBuild)
		flags |= VS_FF_SPECIALBUILD;

	return flags;
}

void oWinRCGetFileType(const oMODULE_TYPE _Type, DWORD* _pType, DWORD* _pSubtype)
{
	*_pType = VFT_UNKNOWN;
	*_pSubtype = VFT2_UNKNOWN;

	switch (_Type)
	{
		case oMODULE_APP: *_pType = VFT_APP; break;
		case oMODULE_DLL: *_pType = VFT_DLL; break;
		case oMODULE_LIB: *_pType = VFT_STATIC_LIB; break;
		case oMODULE_FONT_UNKNOWN: *_pType = VFT_FONT; break;
		case oMODULE_FONT_RASTER: *_pType = VFT_FONT; *_pSubtype = VFT2_FONT_RASTER; break;
		case oMODULE_FONT_TRUETYPE: *_pType = VFT_FONT; *_pSubtype = VFT2_FONT_TRUETYPE; break;
		case oMODULE_FONT_VECTOR: *_pType = VFT_FONT; *_pSubtype = VFT2_FONT_VECTOR; break;
		case oMODULE_VIRTUAL_DEVICE: *_pType = VFT_VXD; break;
		case oMODULE_DRV_UNKNOWN: *_pType = VFT_DRV; break;
		case oMODULE_DRV_COMM: *_pType = VFT_DRV; *_pSubtype = VFT2_DRV_COMM; break;
		case oMODULE_DRV_DISPLAY: *_pType = VFT_DRV; *_pSubtype = VFT2_DRV_DISPLAY; break;
		case oMODULE_DRV_INSTALLABLE: *_pType = VFT_DRV; *_pSubtype = VFT2_DRV_INSTALLABLE; break;
		case oMODULE_DRV_KEYBOARD: *_pType = VFT_DRV; *_pSubtype = VFT2_DRV_KEYBOARD; break;
		case oMODULE_DRV_LANGUAGE: *_pType = VFT_DRV; *_pSubtype = VFT2_DRV_LANGUAGE; break;
		case oMODULE_DRV_MOUSE: *_pType = VFT_DRV; *_pSubtype = VFT2_DRV_MOUSE; break;
		case oMODULE_DRV_NETWORK: *_pType = VFT_DRV; *_pSubtype = VFT2_DRV_NETWORK; break;
		case oMODULE_DRV_PRINTER: *_pType = VFT_DRV; *_pSubtype = VFT2_DRV_PRINTER; break;
		case oMODULE_DRV_SOUND: *_pType = VFT_DRV; *_pSubtype = VFT2_DRV_SOUND; break;
		case oMODULE_DRV_SYSTEM: *_pType = VFT_DRV; *_pSubtype = VFT2_DRV_SYSTEM; break;
	default: break;
	}
}
