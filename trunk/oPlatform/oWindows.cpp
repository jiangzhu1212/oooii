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
#include <oPlatform/oWindows.h>
#include <oBasis/oAssert.h>
#include <oBasis/oByte.h>
#include <oBasis/oColor.h>
#include <oBasis/oError.h>
#include <oBasis/oFixedString.h>
#include <oBasis/oMutex.h>
#include <oBasis/oPath.h>
#include <oBasis/oRef.h>
#include <oBasis/oSize.h>
#include <oBasis/oString.h>
#include <oPlatform/oDateTime.h>
#include <oPlatform/oDisplay.h>
#include <oPlatform/oFile.h>
#include <oPlatform/oProcessHeap.h>
#include <oPlatform/oSystem.h>
#include <oPlatform/oWinRect.h>
#include <oPlatform/oWinAsString.h>
#include "oDbgHelp.h"
#include "oWinDWMAPI.h"
#include "oWinPSAPI.h"
#include "oWinsock.h"
#include "oStaticMutex.h"
#include <io.h>
#include <time.h>
#include <tlhelp32.h>
#include <shellapi.h>
#include <Windowsx.h>
#include <comdef.h>
#include <Wbemidl.h>

// Use the Windows Vista UI look. If this causes issues or the dialog not to appear, try other values from processorAchitecture { x86 ia64 amd64 * }
#pragma comment(linker, "\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

// _____________________________________________________________________________
// Soft links that are still too small to deserve their own source. Prefer soft
// linking to hard linking so the DLL does not have to be loaded in smaller apps
// that use oooii lib.

#if oDXVER >= oDXVER_10

	static const char* kernel32_dll_functions[] = 
	{
		"GetThreadId",
	};

	struct oDLLKernel32 : oModuleSingleton<oDLLKernel32>
	{	oHMODULE hModule;
		oDLLKernel32() : hModule(oModuleLinkSafe("kernel32.dll", kernel32_dll_functions, (void**)&GetThreadId)) { oASSERT(hModule, ""); }
		~oDLLKernel32() { oModuleUnlink(hModule); }

		DWORD (__stdcall *GetThreadId)(HANDLE Thread);
	};

#endif 

// _____________________________________________________________________________

void oOutputDebugStringV(const char* _Format, va_list _Args)
{
	char buf[2048];
	vsprintf_s(buf, _Format, _Args);
	OutputDebugStringA(oAddTruncationElipse(buf));
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
		size_t len = sprintf_s(err, "%s", _ErrorDescPrefix);
		p += len;
		count -= len;
	}

	size_t len = 0;
	const char* HRAsString = oWinAsStringHR(_hResult);
	if ('u' == *HRAsString)
		len = sprintf_s(p, count, " (HRESULT 0x%08x: ", _hResult);
	else
		len = sprintf_s(p, count, " (%s: ", HRAsString);

	p += len;
	count -= len;

	if (!oWinParseHRESULT(p, count, _hResult))
		return false;

	strcat_s(p, count, ")");

	switch (_hResult)
	{
		case ERROR_FILE_NOT_FOUND:
		case ERROR_PATH_NOT_FOUND:
			return oErrorSetLast(oERROR_NOT_FOUND, err);
		case ERROR_ACCESS_DENIED:
		case ERROR_SHARING_VIOLATION:
			return oErrorSetLast(oERROR_REFUSED, err);
		case E_OUTOFMEMORY:
			return oErrorSetLast(oERROR_AT_CAPACITY, err);
	}
	return oErrorSetLast(oERROR_PLATFORM, err);
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

void oUnixTimeToFileTime(time_t _Time, FILETIME* _pFileTime)
{
	// http://msdn.microsoft.com/en-us/library/ms724228(v=vs.85).aspx
	// Note that LONGLONG is a 64-bit value
	LONGLONG ll = Int32x32To64(_Time, 10000000) + 116444736000000000;
	_pFileTime->dwLowDateTime = (DWORD) ll;
	_pFileTime->dwHighDateTime = ll >> 32;
}

time_t oFileTimeToUnixTime(const FILETIME* _pFileTime)
{
	if (!_pFileTime) return 0;
	// this ought to be the reverse of to_filetime
	LONGLONG ll = ((LONGLONG)_pFileTime->dwHighDateTime << 32) | _pFileTime->dwLowDateTime;
	return static_cast<time_t>((ll - 116444736000000000) / 10000000);
}

void oUnixTimeToSystemTime(time_t _Time, SYSTEMTIME* _pSystemTime)
{
	FILETIME ft;
	oUnixTimeToFileTime(_Time, &ft);
	FileTimeToSystemTime(&ft, _pSystemTime);
}

time_t oSystemTimeToUnixTime(const SYSTEMTIME* _pSystemTime)
{
	FILETIME ft;
	SystemTimeToFileTime(_pSystemTime, &ft);
	return oFileTimeToUnixTime(&ft);
}

struct SCHEDULED_FUNCTION_CONTEXT
{
	HANDLE hTimer;
	oTASK OnTimer;
	time_t ScheduledTime;
	char DebugName[64];
};

static void CALLBACK ExecuteScheduledFunctionAndCleanup(LPVOID lpArgToCompletionRoutine, DWORD dwTimerLowValue, DWORD dwTimerHighValue)
{
	SCHEDULED_FUNCTION_CONTEXT& Context = *(SCHEDULED_FUNCTION_CONTEXT*)lpArgToCompletionRoutine;
	if (Context.OnTimer)
	{
		#ifdef _DEBUG
			char strDiff[64];
			oFormatTimeSize(strDiff, (double)(time(nullptr) - Context.ScheduledTime));
			oTRACE("Running scheduled function \"%s\" %s after it was scheduled", oSAFESTRN(Context.DebugName), strDiff);
		#endif

		Context.OnTimer();
		oTRACE("Finished scheduled function \"%s\"", oSAFESTRN(Context.DebugName));
	}
	oVB(CloseHandle((HANDLE)Context.hTimer));
	delete &Context;
}

bool oScheduleTask(const char* _DebugName, time_t _AbsoluteTime, bool _Alertable, oTASK _Task)
{
	SCHEDULED_FUNCTION_CONTEXT& Context = *new SCHEDULED_FUNCTION_CONTEXT();
	Context.hTimer = CreateWaitableTimer(nullptr, TRUE, nullptr);
	oASSERT(Context.hTimer, "CreateWaitableTimer failed LastError=0x%08x", GetLastError());
	Context.OnTimer = _Task;
	Context.ScheduledTime = _AbsoluteTime;

	if (_DebugName && *_DebugName)
		strcpy_s(Context.DebugName, oSAFESTR(_DebugName));
	else
		Context.DebugName[0] = 0;

	Context.ScheduledTime = time(nullptr);

	FILETIME ft;
	oUnixTimeToFileTime(_AbsoluteTime, &ft);

	#ifdef _DEBUG
		oDateTime then;
		oConvertDateTime(&then, _AbsoluteTime);
		char strTime[64];
		char strDiff[64];
		oToString(strTime, then);
		oFormatTimeSize(strDiff, (double)(_AbsoluteTime - Context.ScheduledTime));
		oTRACE("Setting timer to run function \"%s\" at %s (%s from now)", oSAFESTRN(Context.DebugName), strTime, strDiff);
	#endif

		LARGE_INTEGER liDueTime;

		liDueTime.QuadPart = -100000000LL;
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
	rect.SetMin( int2( _Rect.left, _Rect.top ) );
	rect.SetMax( int2( _Rect.right, _Rect.bottom ) );
	return rect;
}

HICON oIconFromBitmap(HBITMAP _hBmp)
{
	HICON hIcon = 0;
	BITMAPINFO bi = {0};
	bi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	if (GetDIBits(GetDC(0), _hBmp, 0, 0, 0, &bi, DIB_RGB_COLORS))
	{
		HBITMAP hMask = CreateCompatibleBitmap(GetDC(0), bi.bmiHeader.biWidth, bi.bmiHeader.biHeight);
		ICONINFO ii = {0};
		ii.fIcon = TRUE;
		ii.hbmColor = _hBmp;
		ii.hbmMask = hMask;
		hIcon = CreateIconIndirect(&ii);
		DeleteObject(hMask);
	}

	return hIcon;
}

void oAllocateBMI(BITMAPINFO** _ppBITMAPINFO, const oImage::DESC& _Desc, oFUNCTION<void*(size_t _Size)> _Allocate, bool _FlipVertically, unsigned int _ARGBMonochrome8Zero, unsigned int _ARGBMonochrome8One)
{
	if (_ppBITMAPINFO)
	{
		const WORD bmiBitCount = static_cast<WORD>(oImageGetBitSize(_Desc.Format));

		oASSERT(*_ppBITMAPINFO || _Allocate, "If no pre-existing BITMAPINFO is specified, then an _Allocate function is required.");
		const unsigned int pitch = _Desc.RowPitch ? _Desc.RowPitch : oImageCalcRowPitch(_Desc.Format, _Desc.Dimensions.x);

		size_t bmiSize = oGetBMISize(_Desc.Format);

		if (!*_ppBITMAPINFO)
			*_ppBITMAPINFO = (BITMAPINFO*)_Allocate(bmiSize);
		BITMAPINFO* pBMI = *_ppBITMAPINFO;

		pBMI->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
		pBMI->bmiHeader.biBitCount = bmiBitCount;
		pBMI->bmiHeader.biClrImportant = 0;
		pBMI->bmiHeader.biClrUsed = 0;
		pBMI->bmiHeader.biCompression = BI_RGB;
		pBMI->bmiHeader.biHeight = (_FlipVertically ? -1 : 1) * (LONG)_Desc.Dimensions.y;
		pBMI->bmiHeader.biWidth = _Desc.Dimensions.x;
		pBMI->bmiHeader.biPlanes = 1;
		pBMI->bmiHeader.biSizeImage = pitch * _Desc.Dimensions.y;
		pBMI->bmiHeader.biXPelsPerMeter = 0;
		pBMI->bmiHeader.biYPelsPerMeter = 0;

		if (bmiBitCount == 8)
		{
			// BMI doesn't understand 8-bit monochrome, so create a monochrome palette
			int r,g,b,a;
			oColorDecompose(_ARGBMonochrome8Zero, &r, &g, &b, &a);
			float4 c0(oUBYTEAsUNORM(r), oUBYTEAsUNORM(g), oUBYTEAsUNORM(b), oUBYTEAsUNORM(a));

			oColorDecompose(_ARGBMonochrome8One, &r, &g, &b, &a);
			float4 c1(oUBYTEAsUNORM(r), oUBYTEAsUNORM(g), oUBYTEAsUNORM(b), oUBYTEAsUNORM(a));

			for (size_t i = 0; i < 256; i++)
			{
				float4 c = lerp(c0, c1, oUBYTEAsUNORM(i));
				RGBQUAD& q = pBMI->bmiColors[i];
				q.rgbRed = oUNORMAsUBYTE(c.x);
				q.rgbGreen = oUNORMAsUBYTE(c.y);
				q.rgbBlue = oUNORMAsUBYTE(c.z);
				q.rgbReserved = oUNORMAsUBYTE(c.w);
			}
		}
	}
}

size_t oGetBMISize(oSURFACE_FORMAT _Format)
{
	return oSurfaceFormatGetBitSize(_Format) == 8 ? (sizeof(BITMAPINFO) + sizeof(RGBQUAD) * 255) : sizeof(BITMAPINFO);
}

oSURFACE_FORMAT oGetFormat(const BITMAPINFOHEADER& _BitmapInfoHeader)
{
	switch (_BitmapInfoHeader.biBitCount)
	{
		case 1: return oSURFACE_R1_UNORM;
		case 16: return oSURFACE_B5G5R5A1_UNORM; // not sure if alpha is respected/but there is no B5G5R5X1_UNORM currently
		case 0:
		case 24: return oSURFACE_B8G8R8_UNORM;
		case 32: return oSURFACE_B8G8R8X8_UNORM;
		default: return oSURFACE_UNKNOWN; // no FORMAT for paletted types currently
	}
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
		for (size_t i = 0; i < oCOUNTOF(HookContexts); i++)
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
		for (size_t i = 0; i < oCOUNTOF(HookContexts); i++)
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
			if (osvi.dwMinorVersion == 1)
				return osvi.wProductType == VER_NT_WORKSTATION ? oWINDOWS_7 : oWINDOWS_SERVER_2008R2;
			else if (osvi.dwMinorVersion == 0)
				return osvi.wProductType == VER_NT_WORKSTATION ? oWINDOWS_VISTA : oWINDOWS_SERVER_2008;
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

const char* oAsString(const oWINDOWS_VERSION& _Version)
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
		case oWINDOWS_UNKNOWN:
		default:
			break;
	}

	return "unknown Windows version";
}

bool oConvertEnvStringToEnvBlock(char* _EnvBlock, size_t _SizeofEnvBlock, const char* _EnvString, char _Delimiter)
{
	if (!_EnvString || ((strlen(_EnvString)+1) > _SizeofEnvBlock))
	{
		oErrorSetLast(oERROR_INVALID_PARAMETER, "EnvBlock buffer not large enough to contain converted EnvString");
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

#if oDXVER >= oDXVER_11
	oVersion oGetD3DVersion(D3D_FEATURE_LEVEL _Level)
	{
		switch (_Level)
		{
			case D3D_FEATURE_LEVEL_11_0: return oVersion(11,0);
			case D3D_FEATURE_LEVEL_10_1: return oVersion(10,1);
			case D3D_FEATURE_LEVEL_10_0: return oVersion(10,0);
			case D3D_FEATURE_LEVEL_9_3: return oVersion(9,3);
			case D3D_FEATURE_LEVEL_9_2: return oVersion(9,2);
			case D3D_FEATURE_LEVEL_9_1: return oVersion(9,1);
			oNODEFAULT;
		}
	}

#endif // oDXVER >= oDXVER_11

unsigned int oWinGetDisplayDevice(HMONITOR _hMonitor, DISPLAY_DEVICE* _pDevice)
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

static bool oWinAdjustPrivilegesForExitWindows()
{
	bool result = false;
	HANDLE hToken;
	TOKEN_PRIVILEGES tkp;
	if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken))
	{
		oWinSetLastError();
		return false;
	}

	if (LookupPrivilegeValue(nullptr, SE_SHUTDOWN_NAME, &tkp.Privileges[0].Luid))
	{
		tkp.PrivilegeCount = 1;      
		tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED; 
		result = !!AdjustTokenPrivileges(hToken, FALSE, &tkp, 0, nullptr, 0);
	}

	if (!result)
		oWinSetLastError();

	oVB(CloseHandle(hToken));
	return result;
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

size_t oStrConvert(char* _MultiByteString, size_t _SizeofMultiByteString, const wchar_t* _StrUnicodeSource)
{
	#if defined(_WIN32) || defined(_WIN64)
		size_t bufferSize = (size_t)WideCharToMultiByte(CP_ACP, 0, _StrUnicodeSource, -1, _MultiByteString, static_cast<int>(_SizeofMultiByteString), "?", 0);
		if (_SizeofMultiByteString && bufferSize)
			bufferSize--;
		return bufferSize;
	#else
		#error Unsupported platform
	#endif
}

size_t oStrConvert(wchar_t* _UnicodeString, size_t _NumberOfCharactersInUnicodeString, const char* _StrMultibyteSource)
{
	#if defined(_WIN32) || defined(_WIN64)
		size_t bufferCount = (size_t)MultiByteToWideChar(CP_ACP, 0, _StrMultibyteSource, -1, _UnicodeString, static_cast<int>(_NumberOfCharactersInUnicodeString));
		if (_NumberOfCharactersInUnicodeString && bufferCount)
			bufferCount--;
		return bufferCount;
	#else
		#error Unsupported platform
	#endif
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

	len = (ULONG)strlen(CmdLine);
	if (_ExePathAsArg0) // @oooii
		len += MAX_PATH * sizeof(CHAR);

	i = ((len+2)/2)*sizeof(PVOID) + sizeof(PVOID);
	if (_ExePathAsArg0) // @oooii
		i += sizeof(PVOID);
	
	// @oooii-tony: change allocator from original code
	argv = (PCHAR*)LocalAlloc(LMEM_FIXED,
		i + (len+2)*sizeof(CHAR));

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
		oCleanPath(argv[argc], MAX_PATH, argv[argc], '\\');
		j = (ULONG)strlen(argv[argc])+1;
		argc++;
		argv[argc] = _argv+strlen(argv[0])+1;
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
	LocalFree(_pArgv);
}

void oGetScreenDPIScale(float* _pScaleX, float* _pScaleY)
{
	HDC screen = GetDC(0);
	*_pScaleX = GetDeviceCaps(screen, LOGPIXELSX) / 96.0f;
	*_pScaleY = GetDeviceCaps(screen, LOGPIXELSY) / 96.0f;
	ReleaseDC(0, screen);
}

HMODULE oThreadsafeLoadLibrary(LPCTSTR _lpFileName)
{
	oProcessHeapLockGuard lock;
	return LoadLibrary(_lpFileName);
}

BOOL oThreadsafeFreeLibrary(HMODULE _hModule)
{
	oProcessHeapLockGuard lock;
	return FreeLibrary(_hModule);
}

// {EE17C49C-A27A-45F0-B2C8-D049E0ECD3C6}
const oGUID guid = { 0xee17c49c, 0xa27a, 0x45f0, { 0xb2, 0xc8, 0xd0, 0x49, 0xe0, 0xec, 0xd3, 0xc6 } };
oDEFINE_STATIC_MUTEX(sOutputDebugStringMutex, guid);

void oThreadsafeOutputDebugStringA(const char* _OutputString)
{
	oLockGuard<oStaticMutex<sOutputDebugStringMutex_t> > Lock(sOutputDebugStringMutex);
	OutputDebugStringA(_OutputString);
}

bool oWaitSingle(HANDLE _Handle, unsigned int _TimeoutMS)
{
	return WAIT_OBJECT_0 == ::WaitForSingleObject(_Handle, _TimeoutMS == oInvalid ? INFINITE : _TimeoutMS);
}

bool oWaitMultiple(HANDLE* _pHandles, size_t _NumberOfHandles, size_t* _pWaitBreakingIndex, unsigned int _TimeoutMS)
{
	oASSERT(_NumberOfHandles <= 64, "Windows has a limit of 64 handles that can be waited on by WaitForMultipleObjects");
	DWORD result = ::WaitForMultipleObjects(static_cast<DWORD>(_NumberOfHandles), _pHandles, !_pWaitBreakingIndex, _TimeoutMS == oINFINITE_WAIT ? INFINITE : _TimeoutMS);

	if (result == WAIT_FAILED)
	{
		oWinSetLastError();
		return false;
	}

	else if (result == WAIT_TIMEOUT)
	{
		oErrorSetLast(oERROR_TIMEOUT);
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
		oErrorSetLast(oERROR_INVALID_PARAMETER);
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
			ID = oDLLKernel32::Singleton()->GetThreadId((HANDLE)_hThread);
		else
			oTRACE("WARNING: oGetThreadID doesn't work with non-zero thread handles on versions of Windows prior to Vista.");
			// todo: Traverse all threads in the system to find the one with the specified handle, then from that struct return the ID.

		return ID;
	#else
		oTRACE("oGetThreadID doesn't behave properly on versions of Windows prior to Vista because GetThreadId(HANDLE _hThread) didn't exist.");
		return 0;
	#endif
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
					if (_stricmp(szProcessName, _Name) == 0)
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
		oErrorSetLast(oERROR_AT_CAPACITY, "There are more than %u processes on the system currently, oWinGetProcessID needs to be more general-purpose.", oCOUNTOF(ProcessIDs));
		truncationResult = false;
	}

	else
		oErrorSetLast(oERROR_NOT_FOUND, "no such process");

	return false;
}

bool oEnumProcessThreads(DWORD _ProcessID, oFUNCTION<bool(DWORD _ThreadID, DWORD _ParentProcessID)> _Function)
{
	HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, GetCurrentProcessId());
	if (hSnapshot == INVALID_HANDLE_VALUE)
	{
		oErrorSetLast(oERROR_INVALID_PARAMETER, "Failed to get a snapshot of current process");
		return false;
	}

	THREADENTRY32 entry;
	entry.dwSize = sizeof(THREADENTRY32);

	BOOL keepLooking = Thread32First(hSnapshot, &entry);

	if (!keepLooking)
	{
		oErrorSetLast(oERROR_NOT_FOUND, "No process threads found");
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
		oErrorSetLast(oERROR_INVALID_PARAMETER);
		return false;
	}

	HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (hSnapshot == INVALID_HANDLE_VALUE)
	{
		oErrorSetLast(oERROR_INVALID_PARAMETER, "Failed to get a snapshot of current process");
		return false;
	}

	DWORD ppid = 0;

	PROCESSENTRY32 entry;
	entry.dwSize = sizeof(entry);
	BOOL keepLooking = Process32First(hSnapshot, &entry);

	if (!keepLooking)
	{
		oErrorSetLast(oERROR_NOT_FOUND, "no child processes");
		return false;
	}

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

#pragma pack(push,8)
typedef struct tagTHREADNAME_INFO
{
   DWORD dwType; // Must be 0x1000.
   LPCSTR szName; // Pointer to name (in user addr space).
   DWORD dwThreadID; // Thread ID (-1=caller thread).
   DWORD dwFlags; // Reserved for future use, must be zero.
} THREADNAME_INFO;
#pragma pack(pop)

void oSetThreadNameInDebugger(DWORD _ThreadID, const char* _Name)
{
	if (_Name && *_Name)
	{
		// http://msdn.microsoft.com/en-us/library/xcb2z8hs.aspx
		Sleep(10);
		THREADNAME_INFO i;
		i.dwType = 0x1000;
		i.szName = _Name;
		i.dwThreadID = _ThreadID ? _ThreadID : -1;
		i.dwFlags = 0;

		const static DWORD MS_VC_EXCEPTION = 0x406D1388;
		__try
		{
			RaiseException(MS_VC_EXCEPTION, 0, sizeof(i)/sizeof(ULONG_PTR), (ULONG_PTR*)&i);
		}

		__except(EXCEPTION_EXECUTE_HANDLER)
		{
		}
	}
}

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
	return sizeof(WCHAR) * (strlen(oSAFESTR(_Text)) + 1);
}

static size_t oDlgCalcTemplateSize(const char* _MenuName, const char* _ClassName, const char* _Caption, const char* _FontName)
{
	size_t size = sizeof(DLGTEMPLATE) + sizeof(WORD); // extra word for font size if we specify it
	size += oDlgCalcTextSize(_MenuName);
	size += oDlgCalcTextSize(_ClassName);
	size += oDlgCalcTextSize(_Caption);
	size += oDlgCalcTextSize(_FontName);
	return oByteAlign(size, sizeof(DWORD)); // align to DWORD because dialog items need DWORD alignment, so make up for any padding here
}

static size_t oDlgCalcTemplateItemSize(const char* _Text)
{
	return oByteAlign(sizeof(DLGITEMTEMPLATE) +
		2 * sizeof(WORD) + // will keep it simple 0xFFFF and the ControlClass
		oDlgCalcTextSize(_Text) + // text
		sizeof(WORD), // 0x0000. This is used for data to be passed to WM_CREATE, bypass this for now
		sizeof(DWORD)); // in size calculations, ensure everything is DWORD-aligned
}

static WORD* oDlgCopyString(WORD* _pDestination, const char* _String)
{
	if (!_String) _String = "";
	size_t len = strlen(_String);
	oStrConvert((WCHAR*)_pDestination, len+1, _String);
	return oByteAdd(_pDestination, (len+1) * sizeof(WCHAR));
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

	return (LPDLGITEMTEMPLATE)oByteAlign(p, sizeof(DWORD));
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
	WORD* p = oByteAdd((WORD*)_lpDlgItemTemplate, sizeof(DLGITEMTEMPLATE));
	*p++ = 0xFFFF; // flag that a simple ControlClass is to follow
	*p++ = oDlgGetClass(_Desc.Type);
	p = oDlgCopyString(p, _Desc.Text);
	*p++ = 0x0000; // no WM_CREATE data
	return (LPDLGITEMTEMPLATE)oByteAlign(p, sizeof(DWORD));
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

bool oWinVideoDriverIsUpToDate()
{
	oWINDOWS_VIDEO_DRIVER_DESC desc;
	memset(&desc, 0, sizeof(oWINDOWS_VIDEO_DRIVER_DESC));

	oWinGetVideoDriverDesc(&desc);

	const char* VendorName = "Unknown";
	oVersion RequiredVersion;

	bool bCompatibleVersion = false;
	switch (desc.Vendor)
	{
		case oWINDOWS_VIDEO_DRIVER_DESC::NVIDIA:
			VendorName = "NVIDIA";
			RequiredVersion = oVersion(oNVVER_MAJOR, oNVVER_MINOR);
			break;

		case oWINDOWS_VIDEO_DRIVER_DESC::AMD:
			VendorName = "AMD";
			RequiredVersion = oVersion(oAMDVER_MAJOR, oAMDVER_MINOR);
			break;
		
		default:
			return oErrorSetLast(oERROR_NOT_FOUND, "unrecognized video driver type");
	}

	if (desc.Version != RequiredVersion)
		return oErrorSetLast(oERROR_GENERIC, "incompatible %s video driver version. current: %d.%d / required: %d.%d", VendorName, desc.Version.Major, desc.Version.Minor, RequiredVersion.Major, RequiredVersion.Minor);

	return true;
}

// NVIDIA's version string is of the form "x.xx.xM.MMmm" where
// MMM is the major version and mm is the minor version.
// (outside function below so this doesn't get tracked as a leak)
static std::regex reNVVersionString("[0-9]+\\.[0-9]+\\.[0-9]+([0-9])\\.([0-9][0-9])([0-9]+)");

static bool ParseVersionStringNV(const char* _VersionString, oVersion* _pVersion)
{
	if (!_VersionString || !_pVersion)
		return oErrorSetLast(oERROR_INVALID_PARAMETER);

	std::cmatch matches;
	if (!regex_match(_VersionString, matches, reNVVersionString))
		return oErrorSetLast(oERROR_INVALID_PARAMETER, "The specified string \"%s\" is not a well-formed NVIDIA version string", oSAFESTRN(_VersionString));

	char major[4];
	major[0] = *matches[1].first;
	major[1] = *matches[2].first;
	major[2] = *(matches[2].first+1);
	major[3] = 0;
	_pVersion->Major = atoi(major);
	_pVersion->Minor = atoi(matches[3].first);

	return true;
}

// AMD's version string is of the form M.mm.x.x where
// M is the major version and mm is the minor version.
// (outside function below so this doesn't get tracked as a leak)
static std::regex reAMDVersionString("([0-9]+)\\.([0-9]+)\\.[0-9]+\\.[0-9]+");

static bool ParseVersionStringAMD(const char* _VersionString, oVersion* _pVersion)
{
	if (!_VersionString || !_pVersion)
		return oErrorSetLast(oERROR_INVALID_PARAMETER);

	std::cmatch matches;
	if (!regex_match(_VersionString, matches, reAMDVersionString))
		return oErrorSetLast(oERROR_INVALID_PARAMETER, "The specified string \"%s\" is not a well-formed AMD version string", oSAFESTRN(_VersionString));

	_pVersion->Major = atoi(matches[1].first);
	_pVersion->Minor = atoi(matches[2].first);

	return true;
}

bool oWinGetVideoDriverDesc(oWINDOWS_VIDEO_DRIVER_DESC* _pDesc)
{
	// redefine some MS GUIDs so we don't have to link to them

	// 4590F811-1D3A-11D0-891F-00AA004B2E24
	static const oGUID oGUID_CLSID_WbemLocator = { 0x4590F811, 0x1D3A, 0x11D0, { 0x89, 0x1F, 0x00, 0xAA, 0x00, 0x4B, 0x2E, 0x24 } };

	// DC12A687-737F-11CF-884D-00AA004B2E24
	static const oGUID oGUID_IID_WbemLocator = { 0xdc12a687, 0x737f, 0x11cf, { 0x88, 0x4D, 0x00, 0xAA, 0x00, 0x4B, 0x2E, 0x24 } };

	oV(CoInitializeEx(nullptr, COINIT_MULTITHREADED));
	oOnScopeExit OnScopeExit([&] { CoUninitialize(); });

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
		ULONG uReturn = 0;
		WbemClassObject = nullptr;
		oV(Enumerator->Next(WBEM_INFINITE, 1, &WbemClassObject, &uReturn));
		if (0 == uReturn)
			break;

		VARIANT vtProp;
		oV(WbemClassObject->Get(L"Description", 0, &vtProp, 0, 0));
		_pDesc->Desc = vtProp.bstrVal;
		VariantClear(&vtProp);
		oV(WbemClassObject->Get(L"DriverVersion", 0, &vtProp, 0, 0));

		oStringS version = vtProp.bstrVal;
		if (strstr(_pDesc->Desc, "NVIDIA"))
		{
			_pDesc->Vendor = oWINDOWS_VIDEO_DRIVER_DESC::NVIDIA;
			oVERIFY(ParseVersionStringNV(version, &_pDesc->Version));
		}
		
		else if (strstr(_pDesc->Desc, "ATI") || strstr(_pDesc->Desc, "AMD"))
		{
			_pDesc->Vendor = oWINDOWS_VIDEO_DRIVER_DESC::AMD;
			oVERIFY(ParseVersionStringAMD(version, &_pDesc->Version));
		}

		else
		{
			_pDesc->Vendor = oWINDOWS_VIDEO_DRIVER_DESC::UNKNOWN;
			_pDesc->Version = oVersion();
		}
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
		oErrorSetLast(oERROR_INVALID_PARAMETER);
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
		oStringPath SystemRootPath;
		GetEnvironmentVariable("SYSTEMROOT", SystemRootPath.c_str(), oSize32(SystemRootPath.capacity()));

		strcpy_s(_StrDestination, _SizeofStrDestination, pQSC->lpBinaryPathName);
		char* lpFirstOccurance = nullptr;
		lpFirstOccurance = strstr(_StrDestination, "\\SystemRoot");
		if (strlen(_StrDestination) > 0 && lpFirstOccurance)
		{
			// replace \SysteRoot with system root value obtained by
			// calling GetEnvironmentVariable() to get SYSTEMROOT
			lpFirstOccurance += strlen("\\SystemRoot");
			sprintf_s(_StrDestination, _SizeofStrDestination, "%s%s", SystemRootPath.c_str(), lpFirstOccurance);
		}

		else
		{
			lpFirstOccurance = nullptr;
			lpFirstOccurance = strstr(_StrDestination, "\\??\\");
			if (strlen(_StrDestination) > 0 && lpFirstOccurance != nullptr)
			{
				// Remove \??\ at the starting of binary path
				lpFirstOccurance += strlen("\\??\\");
				sprintf_s(_StrDestination, _SizeofStrDestination, "%s", lpFirstOccurance);
			}

			else
			{
				lpFirstOccurance = nullptr;
				lpFirstOccurance = strstr(_StrDestination, "system32");
				if (strlen(_StrDestination) > 0 && lpFirstOccurance && _StrDestination == lpFirstOccurance)
				{
					// Add SYSTEMROOT value if there is only system32 mentioned in binary path
					oStringPath temp;
					sprintf_s(temp.c_str(), temp.capacity(), "%s\\%s", SystemRootPath.c_str(), _StrDestination); 
					strcpy_s(_StrDestination, _SizeofStrDestination, temp);
				}

				else
				{
					lpFirstOccurance = nullptr;
					lpFirstOccurance = strstr(_StrDestination, "System32");
					if (strlen(_StrDestination) > 0 && lpFirstOccurance && _StrDestination == lpFirstOccurance)
					{
						// Add SYSTEMROOT value if there is only System32 mentioned in binary path
						oStringPath temp;
						sprintf_s(temp.c_str(), temp.capacity(), "%s\\%s", SystemRootPath.c_str(), _StrDestination); 
						strcpy_s(_StrDestination, _SizeofStrDestination, temp); 
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

bool oWinSystemIs64BitBinary(const char* _StrPath)
{
	bool result = false;
	void* mapped = nullptr;

	oFILE_DESC FDesc;
	if (!oFileGetDesc(_StrPath, &FDesc))
		return false; // pass through error

	oFileRange r;
	r.Offset = 0;
	r.Size = FDesc.Size;

	if (oFileMap(_StrPath, false, r, &mapped))
	{
		IMAGE_NT_HEADERS* pHeader = oDbgHelp::Singleton()->ImageNtHeader(mapped);
		result = pHeader->FileHeader.Machine == IMAGE_FILE_MACHINE_AMD64;
		oVERIFY(oFileUnmap(mapped));
	}

	return result;
}
