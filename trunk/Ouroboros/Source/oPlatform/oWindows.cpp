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
#include <oBase/assert.h>
#include <oBase/byte.h>
#include <oBase/color.h>
#include <oBasis/oError.h>
#include <oBase/fixed_string.h>
#include <oConcurrency/mutex.h>
#include <oPlatform/oDisplay.h>
#include <oPlatform/oProcessHeap.h>
#include <oPlatform/oSingleton.h>
#include <oPlatform/Windows/oWinRect.h>
#include <oPlatform/Windows/oWinAsString.h>
#include <oPlatform/oMsgBox.h>
#include <oPlatform/oStream.h>
#include "oStaticMutex.h"
#include <io.h>
#include <time.h>
#include <tlhelp32.h>
#include <Windowsx.h>
#include <comdef.h>
#include <Wbemidl.h>
#include <devguid.h>
#include <devpkey.h>
#include <psapi.h>
#include <SetupAPI.h>

// Use the Windows Vista UI look. If this causes issues or the dialog not to appear, try other values from processorAchitecture { x86 ia64 amd64 * }
#pragma comment(linker, "\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

int oWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow, int (*oMain)(int argc, const char* argv[]))
{
	ouro::path AppPath = ouro::filesystem::app_path(true);
	int argc = 0;
	const char** argv = ouro::argtok(malloc, AppPath.c_str(), lpCmdLine, &argc);
	int result = oMain(argc, argv);
	free(argv);
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
		size_t len = snprintf(err, "%s", _ErrorDescPrefix);
		p += len;
		count -= len;
	}

	size_t len = 0;
	const char* HRAsString = oWinAsStringHR(_hResult);
	if ('u' == *HRAsString)
		len = snprintf(p, count, " (HRESULT 0x%08x: ", _hResult);
	else
		len = snprintf(p, count, " (%s: ", HRAsString);

	p += len;
	count -= len;

	if (!oWinParseHRESULT(p, count, _hResult))
		return false;

	strlcat(p, ")", count);

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
	ouro::sstring DebugName;
};

static void CALLBACK ExecuteScheduledFunctionAndCleanup(LPVOID lpArgToCompletionRoutine, DWORD dwTimerLowValue, DWORD dwTimerHighValue)
{
	SCHEDULED_FUNCTION_CONTEXT& Context = *(SCHEDULED_FUNCTION_CONTEXT*)lpArgToCompletionRoutine;
	if (Context.OnTimer)
	{
		#ifdef _DEBUG
			ouro::sstring diff;
			ouro::format_duration(diff, (double)(time(nullptr) - Context.ScheduledTime));
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
	ouro::transform(_StrDestination, end, _StrDestination, [=](char c) { return (c == '#') ? '\\' : c; });
	ouro::transform(_StrDestination, end, _StrDestination, ouro::toupper<const char&>);
	char* first_slash = strchr(_StrDestination, '\\');
	if (!first_slash) return nullptr;
	*first_slash = '\0';
	return _StrDestination;
}

template<size_t size> char* oWinPnPEnumeratorToSetupClass(char (&_StrDestination)[size], const char* _PnPEnumerator) { return oWinPnPEnumeratorToSetupClass(_StrDestination, size, _PnPEnumerator); }
template<typename charT, size_t capacity> char* oWinPnPEnumeratorToSetupClass(ouro::fixed_string<charT, capacity>& _StrDestination, const char* _PnPEnumerator) { return oWinPnPEnumeratorToSetupClass(_StrDestination, _StrDestination.capacity(), _PnPEnumerator); }

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

static bool oWinSetupGetString(ouro::mstring& _StrDestination, HDEVINFO _hDevInfo, SP_DEVINFO_DATA* _pSPDID, const DEVPROPKEY* _pPropKey)
{
	DEVPROPTYPE dpType;
	DWORD size = 0;
	SetupDiGetDevicePropertyW(_hDevInfo, _pSPDID, _pPropKey, &dpType, nullptr, 0, &size, 0);
	if (size)
	{
		ouro::mwstring buffer;
		oASSERT(buffer.capacity() > (size / sizeof(ouro::mwstring::char_type)), "");
		if (!SetupDiGetDevicePropertyW(_hDevInfo, _pSPDID, _pPropKey, &dpType, (BYTE*)buffer.c_str(), size, &size, 0) || dpType != DEVPROP_TYPE_STRING)
			return oWinSetLastError();
		_StrDestination = buffer;
	}
	else
		_StrDestination.clear();

	return true;
}

// _NumStrings must be initialized to maximum capacity
static bool oWinSetupGetStringList(ouro::mstring* _StrDestination, size_t& _NumStrings, HDEVINFO _hDevInfo, SP_DEVINFO_DATA* _pSPDID, const DEVPROPKEY* _pPropKey)
{
	DEVPROPTYPE dpType;
	DWORD size = 0;
	SetupDiGetDevicePropertyW(_hDevInfo, _pSPDID, _pPropKey, &dpType, nullptr, 0, &size, 0);
	if (size)
	{
		ouro::xlwstring buffer;
		oASSERT(buffer.capacity() > size, "");
		if (!SetupDiGetDevicePropertyW(_hDevInfo, _pSPDID, _pPropKey, &dpType, (BYTE*)buffer.c_str(), size, &size, 0) || dpType != DEVPROP_TYPE_STRING_LIST)
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
	ouro::finally([=]
	{ 
		if (hDevInfo != INVALID_HANDLE_VALUE) 
			SetupDiDestroyDeviceInfoList(hDevInfo);
	});

	DWORD dwFlag = DIGCF_ALLCLASSES;
	if (!_EnumerateAll)
		dwFlag |= DIGCF_PRESENT;

	hDevInfo = SetupDiGetClassDevsA(nullptr, _Enumerator, nullptr, dwFlag);
	if (INVALID_HANDLE_VALUE == hDevInfo)
		return oWinSetLastError();

	SP_DEVINFO_DATA SPDID;
	memset(&SPDID, 0, sizeof(SPDID));
	SPDID.cbSize = sizeof(SPDID);
	int DeviceIndex = 0;
	bool KnownTypeFound = false;

	oWINDOWS_HID_DESC HIDDesc;

	ouro::mstring KinectCameraSibling;
	ouro::mstring KinectCameraParent;

	while (SetupDiEnumDeviceInfo(hDevInfo, DeviceIndex, &SPDID))
	{
		DeviceIndex++;

		ouro::mstring ClassValue;
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

				ouro::mstring Strings[2];
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

bool IsProcessThread(oStd::thread::id _ThreadID, oStd::thread::id _FindThreadID, bool* _pFound)
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
	oStd::thread::id ThreadId = *((oStd::thread::id*)&_dwThreadId);
	ouro::this_process::enumerate_threads(oBIND(IsProcessThread, oBIND1, ThreadId, &isProcessThread));

	ouro::module::id hookModuleId = ouro::module::get_id(_pHookProc);
	ouro::module::id currentModuleId = ouro::this_module::get_id();
	if (hookModuleId == currentModuleId && isProcessThread)
		hookModuleId = ouro::module::id();

	hc->pUserData = _pUserData;
	hc->UserHookProc = _pHookProc;
	hc->hHook = ::SetWindowsHookExA(_idHook, hc->UniqueHookProc, *(HMODULE*)&hookModuleId, _dwThreadId);
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
		oMsgBox(oMSGBOX_DESC(oMSGBOX_ERR, "Invalid Windows Version"), "%s or greater required.  Application will now terminate.", ouro::as_string(_Version));
		std::terminate();
	}
}

namespace ouro {

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

} // namespace ouro

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
			if (!strcmp(mi.szDevice, _pDevice->DeviceName))
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

void oWinGetVersion(const ouro::version& _Version, DWORD* _pVersionMS, DWORD* _pVersionLS)
{
	*_pVersionMS = (_Version.major << 16) | _Version.minor;
	*_pVersionLS = (_Version.build << 16) | _Version.revision;
}

ouro::version oWinGetVersion(DWORD _VersionMS, DWORD _VersionLS)
{
	return ouro::version(HIWORD(_VersionMS), LOWORD(_VersionMS), HIWORD(_VersionLS), LOWORD(_VersionLS));
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
	return sizeof(WCHAR) * (strlen(_Text ? _Text : "") + 1);
}

static size_t oDlgCalcTemplateSize(const char* _MenuName, const char* _ClassName, const char* _Caption, const char* _FontName)
{
	size_t size = sizeof(DLGTEMPLATE) + sizeof(WORD); // extra word for font size if we specify it
	size += oDlgCalcTextSize(_MenuName);
	size += oDlgCalcTextSize(_ClassName);
	size += oDlgCalcTextSize(_Caption);
	size += oDlgCalcTextSize(_FontName);
	return ouro::byte_align(size, sizeof(DWORD)); // align to DWORD because dialog items need DWORD alignment, so make up for any padding here
}

static size_t oDlgCalcTemplateItemSize(const char* _Text)
{
	return ouro::byte_align(sizeof(DLGITEMTEMPLATE) +
		2 * sizeof(WORD) + // will keep it simple 0xFFFF and the ControlClass
		oDlgCalcTextSize(_Text) + // text
		sizeof(WORD), // 0x0000. This is used for data to be passed to WM_CREATE, bypass this for now
		sizeof(DWORD)); // in size calculations, ensure everything is DWORD-aligned
}

static WORD* oDlgCopyString(WORD* _pDestination, const char* _String)
{
	if (!_String) _String = "";
	size_t len = strlen(_String);
	mbsltowsc((WCHAR*)_pDestination, _String, len+1);
	return ouro::byte_add(_pDestination, (len+1) * sizeof(WCHAR));
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

	return (LPDLGITEMTEMPLATE)ouro::byte_align(p, sizeof(DWORD));
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
	WORD* p = ouro::byte_add((WORD*)_lpDlgItemTemplate, sizeof(DLGITEMTEMPLATE));
	*p++ = 0xFFFF; // flag that a simple ControlClass is to follow
	*p++ = oDlgGetClass(_Desc.Type);
	p = oDlgCopyString(p, _Desc.Text);
	*p++ = 0x0000; // no WM_CREATE data
	return (LPDLGITEMTEMPLATE)ouro::byte_align(p, sizeof(DWORD));
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

