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
#include <oooii/oWindows.h>
#include <oooii/oAssert.h>
#include <oooii/oAtomic.h>
#include <oooii/oByte.h>
#include <oooii/oColor.h>
#include <oooii/oDisplay.h>
#include <oooii/oErrno.h>
#include <oooii/oProcessHeap.h>
#include <oooii/oRef.h>
#include <oooii/oSize.h>
#include <oooii/oStdio.h>
#include <oooii/oSTL.h>
#include <oooii/oString.h>
#include <oooii/oTimerHelpers.h>
#include <oooii/oMath.h>
#include <oooii/oMutex.h>
#include "oWinDWMAPI.h"
#include "oWinPSAPI.h"
#include "oWinsock.h"
#include <io.h>
#include <time.h>
#include <tlhelp32.h>
#include <shellapi.h>
#include <Windowsx.h>

// Use the Windows Vista UI look. If this causes issues or the dialog not to appear, try other values from processorAchitecture { x86 ia64 amd64 * }
#pragma comment(linker, "\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

bool oWinSetLastError(HRESULT _hResult, const char* _ErrorDescPrefix)
{
	if (_hResult == oWINDOWS_DEFAULT)
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

	size_t len = sprintf_s(p, count, "HRESULT 0x%08x: ", _hResult);
	p += len;
	count -= len;

	if (oGetWindowsErrorDescription(p, count, _hResult))
		return false;

	// @oooii-tony: it would be nice to convert the errno a bit better, but that's
	// a lot of typing! Maybe one day...
	return oSetLastError(EINVAL, err);
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
    static MSGBOXAAPI MsgBoxTOA = NULL;

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
    static MSGBOXWAPI MsgBoxTOW = NULL;

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
	oFUNCTION<void()> OnTimer;
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

bool oScheduleFunction(const char* _DebugName, time_t _AbsoluteTime, bool _Alertable, oFUNCTION<void()> _Function)
{
	SCHEDULED_FUNCTION_CONTEXT& Context = *new SCHEDULED_FUNCTION_CONTEXT();
	Context.hTimer = CreateWaitableTimer(nullptr, TRUE, nullptr);
	oASSERT(Context.hTimer, "CreateWaitableTimer failed LastError=0x%08x", GetLastError());
	Context.OnTimer = _Function;
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

HRESULT oGetClientScreenRect(HWND _hWnd, RECT* _pRect)
{
	HRESULT hr = E_FAIL;
	if (_hWnd && _pRect)
	{
		if (GetClientRect(_hWnd, _pRect))
		{
			POINT p;
			p.x = _pRect->left;
			p.y = _pRect->top;
			ClientToScreen(_hWnd, &p);
			_pRect->left = p.x;
			_pRect->top = p.y;
			_pRect->right += _pRect->left;
			_pRect->bottom += _pRect->top;
		}

		else
			hr = GetLastError();
	}

	return hr;
}

oRECT oToRect(const RECT& _Rect)
{
	oRECT rect;
	rect.SetMin( int2( _Rect.left, _Rect.top ) );
	rect.SetMax( int2( _Rect.right, _Rect.bottom ) );
	return rect;
}


bool oIsCursorVisible()
{
	CURSORINFO ci;
	ci.cbSize = sizeof(CURSORINFO);
	GetCursorInfo(&ci);
	return ci.flags == CURSOR_SHOWING;
}

bool oIsCursorClipped(HWND _hWnd)
{
	RECT rClip, rClient;
	GetClipCursor(&rClip);
	GetClientRect(_hWnd, &rClient);
	return !memcmp(&rClip, &rClient, sizeof(RECT));
}

HRESULT oClipCursorToClient(HWND _hWnd, bool _Clip)
{
	if (!_hWnd)
		return E_FAIL;

	if (_Clip)
	{
		RECT r;
		GetClientRect(_hWnd, &r);
		POINT p = { 0, 0 };
		ClientToScreen(_hWnd, &p);
		r.left += p.x;
		r.top += p.y;
		r.right += p.x;
		r.bottom += p.y;
		if (!ClipCursor(&r))
			return GetLastError();
	}

	else
		ClipCursor(0);

	return S_OK;
}

bool oGetCursorClientPos(HWND _hWnd, int* _pX, int* _pY)
{
	POINT p;
	bool result = GetCursorPos(&p) && ScreenToClient(_hWnd, &p);
	*_pX = p.x;
	*_pY = p.y;
	return result;
}

bool oSetCursorClientPos(HWND _hWnd, int X, int Y)
{
	POINT p = { X, Y };
	return ClientToScreen(_hWnd, &p) && SetCursorPos(p.x, p.y);
}

bool oSetTitle(HWND _hWnd, const char* _Title)
{
	if (!_hWnd) return false;
	if (!_Title) _Title = "";
	return TRUE == SendMessageA(_hWnd, WM_SETTEXT, 0, (LPARAM)_Title);
}

bool oGetTitle(HWND _hWnd, char* _Title, size_t _SizeofTitle)
{
	if (!_hWnd || !_Title) return false;
	return TRUE == SendMessage(_hWnd, WM_SETTEXT, _SizeofTitle, (LPARAM)_Title);
}

void oSetAlwaysOnTop(HWND _hWnd, bool _AlwaysOnTop)
{
	RECT r;
	GetWindowRect(_hWnd, &r);
	::SetWindowPos(_hWnd, _AlwaysOnTop ? HWND_TOPMOST : HWND_TOP, r.left, r.top, r.right - r.left, r.bottom - r.top, IsWindowVisible(_hWnd) ? SWP_SHOWWINDOW : SWP_HIDEWINDOW);
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

void oAllocateBMI(BITMAPINFO** _ppBITMAPINFO, const oSurface::DESC* _pDesc, oFUNCTION<void*(size_t _Size)> _Allocate, bool _FlipVertically, unsigned int _ARGBMonochrome8Zero, unsigned int _ARGBMonochrome8One)
{
	if (_pDesc && _ppBITMAPINFO)
	{
		const WORD bmiBitCount = (WORD)GetBitSize(_pDesc->Format);

		oASSERT(*_ppBITMAPINFO || _Allocate, "If no pre-existing BITMAPINFO is specified, then an _Allocate function is required.");

		oASSERT(!IsBlockCompressedFormat(_pDesc->Format), "block compressed formats not supported by BITMAPINFO");
		const unsigned int pitch = _pDesc->RowPitch ? _pDesc->RowPitch : CalcRowPitch(_pDesc->Format, _pDesc->Width);

		size_t bmiSize = oGetBMISize(_pDesc->Format);

		if (!*_ppBITMAPINFO)
			*_ppBITMAPINFO = (BITMAPINFO*)_Allocate(bmiSize);
		BITMAPINFO* pBMI = *_ppBITMAPINFO;

		pBMI->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
		pBMI->bmiHeader.biBitCount = bmiBitCount;
		pBMI->bmiHeader.biClrImportant = 0;
		pBMI->bmiHeader.biClrUsed = 0;
		pBMI->bmiHeader.biCompression = BI_RGB;
		pBMI->bmiHeader.biHeight = (_FlipVertically ? -1 : 1) * (LONG)_pDesc->Height;
		pBMI->bmiHeader.biWidth = _pDesc->Width;
		pBMI->bmiHeader.biPlanes = 1;
		pBMI->bmiHeader.biSizeImage = pitch * _pDesc->Height;
		pBMI->bmiHeader.biXPelsPerMeter = 0;
		pBMI->bmiHeader.biYPelsPerMeter = 0;

		if (bmiBitCount == 8)
		{
			// BMI doesn't understand 8-bit monochrome, so create a monochrome palette
			unsigned int r,g,b,a;
			oDecomposeColor(_ARGBMonochrome8Zero, &r, &g, &b, &a);
			float4 c0(oUBYTEAsUNORM(r), oUBYTEAsUNORM(g), oUBYTEAsUNORM(b), oUBYTEAsUNORM(a));

			oDecomposeColor(_ARGBMonochrome8One, &r, &g, &b, &a);
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

size_t oGetBMISize(oSurface::FORMAT _Format)
{
	return oSurface::GetBitSize(_Format) == 8 ? (sizeof(BITMAPINFO) + sizeof(RGBQUAD) * 255) : sizeof(BITMAPINFO);
}

void oResolveRect(const RECT* _pParentRect, const RECT* _pInRect, bool _ClipToParent, RECT* _pOutRect)
{
	if (_pInRect)
	{
		if (_ClipToParent && _pParentRect)
		{
			_pOutRect->left = __max(_pInRect->left, _pParentRect->left);
			_pOutRect->top = __max(_pInRect->top, _pParentRect->top);
			_pOutRect->right = _pInRect->right == oWINDOWS_DEFAULT ? _pParentRect->right : __min(_pInRect->right, _pParentRect->right);
			_pOutRect->bottom = _pInRect->bottom == oWINDOWS_DEFAULT ? _pParentRect->bottom : __min(_pInRect->bottom, _pParentRect->bottom);
		}
		
		else
		{
			_pOutRect->left = _pInRect->left == oWINDOWS_DEFAULT ? 0 : _pInRect->left;
			_pOutRect->top = _pInRect->top == oWINDOWS_DEFAULT ? 0 : _pInRect->top;
			_pOutRect->right = _pInRect->right == oWINDOWS_DEFAULT ? _pParentRect->right : _pInRect->right;
			_pOutRect->bottom = _pInRect->bottom == oWINDOWS_DEFAULT ? _pParentRect->bottom : _pInRect->bottom;
		}
	}
	
	else if (_pParentRect)
		*_pOutRect = *_pParentRect;
		
	else
		memset(_pOutRect, 0, sizeof(RECT));
}

void oAlignRect(const RECT* _pParentRect, const RECT* _pInRect, RECT* _pOutAlignedRect, const char* _Options)
{
	const int PARENT_W = _pParentRect->right - _pParentRect->left;
	const int PARENT_H = _pParentRect->bottom - _pParentRect->top;
	
	const int W = _pInRect->right - _pInRect->left;
	const int H = _pInRect->bottom - _pInRect->top;
	
	int x = 0, y = 0;

	if (_Options)
	{
		if (strchr(_Options, 'c')) x = (PARENT_W - W) / 2;
		else if (strchr(_Options, 'r')) x = _pParentRect->right - W;
			
		if (strchr(_Options, 'm')) y = (PARENT_H - H) / 2;
		else if (strchr(_Options, 'b')) y = _pParentRect->bottom - H;
	}

	_pOutAlignedRect->left += x;
	_pOutAlignedRect->top += y;
	_pOutAlignedRect->right = _pOutAlignedRect->left + W;
	_pOutAlignedRect->bottom = _pOutAlignedRect->top + H;
}

void oAdjustRect(const RECT* _pParentRect, const RECT* _pInRect, RECT* _pOutAdjustedRect, int _Scale, const char* _Options)
{
	oResolveRect(_pParentRect, _pInRect, _Options && !!strchr(_Options, 'f'), _pOutAdjustedRect);
	oAlignRect(_pParentRect, _pOutAdjustedRect, _pOutAdjustedRect, _Options);
	_pOutAdjustedRect->left *= _Scale;
	_pOutAdjustedRect->top *= _Scale;
	_pOutAdjustedRect->right *= _Scale;
	_pOutAdjustedRect->bottom *= _Scale;
}

bool oGDIScreenCaptureWindow(HWND _hWnd, const RECT* _pRect, void* _pImageBuffer, size_t _SizeofImageBuffer, BITMAPINFO* _pBitmapInfo)
{
	RECT r;
	if (_pRect)
	{
		// Find offset into client area
		POINT p = {0,0};
		ClientToScreen(_hWnd, &p);

		RECT wr;
		GetWindowRect(_hWnd, &wr);
		p.x -= wr.left;
		p.y -= wr.top;

		r.left = _pRect->left + p.x;
		r.right = _pRect->right + p.x;
		r.top = _pRect->top + p.y;
		r.bottom = _pRect->bottom + p.y;
	}

	else
	{
		RECT wr;
		GetWindowRect(_hWnd, &wr);
		int w = wr.right - wr.left;
		int h = wr.bottom - wr.top;

		r.left = 0;
		r.top = 0;
		r.right = w;
		r.bottom = h;
	}

	LONG w = r.right - r.left;
	LONG h = r.bottom - r.top;

	WORD bitdepth = 0;
	{
		oDisplay::DESC displayDesc;
		oDisplay::GetDesc(oGetWindowDisplayIndex(_hWnd), &displayDesc);
		bitdepth = static_cast<WORD>(displayDesc.ModeDesc.Bitdepth);
		if (bitdepth == 32) bitdepth = 24;
	}

	if (!_pBitmapInfo)
	{
		oSetLastError(EINVAL);
		return false;
	}

	memset(_pBitmapInfo, 0, sizeof(BITMAPINFO));
	_pBitmapInfo->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	_pBitmapInfo->bmiHeader.biWidth = w;
	_pBitmapInfo->bmiHeader.biHeight = h;
	_pBitmapInfo->bmiHeader.biPlanes = 1;
	_pBitmapInfo->bmiHeader.biBitCount = bitdepth;
	_pBitmapInfo->bmiHeader.biCompression = BI_RGB;
	_pBitmapInfo->bmiHeader.biSizeImage = w * h * bitdepth / 8;

	if (_pImageBuffer)
	{
		if (_SizeofImageBuffer < _pBitmapInfo->bmiHeader.biSizeImage)
		{
			oSetLastError(EINVAL, "Destination buffer too small");
			return false;
		}

		HDC hDC = GetWindowDC(_hWnd);
		HDC hMemDC = CreateCompatibleDC(hDC);

		HBITMAP hBMP = CreateCompatibleBitmap(hDC, w, h);
		HBITMAP hOld = (HBITMAP)SelectObject(hMemDC, hBMP);

		oVB(RedrawWindow(_hWnd, 0, 0, RDW_INVALIDATE|RDW_UPDATENOW));
		BitBlt(hMemDC, 0, 0, w, h, hDC, r.left, r.top, SRCCOPY);
		GetDIBits(hMemDC, hBMP, 0, h, _pImageBuffer, _pBitmapInfo, DIB_RGB_COLORS);

		SelectObject(hMemDC, hOld);
		DeleteDC(hMemDC);
		ReleaseDC(0, hDC);
	}

	return true;
} 

BOOL oGDIDrawBitmap(HDC _hDC, INT _X, INT _Y, HBITMAP _hBitmap, DWORD _dwROP)
{
	HDC hDCBitmap = 0;
	BITMAP Bitmap;
	BOOL bResult = false;

	if (_hDC && _hBitmap)
	{
		hDCBitmap = CreateCompatibleDC(_hDC);
		GetObject(_hBitmap, sizeof(BITMAP), (LPSTR)&Bitmap);
		HGDIOBJ hOld = SelectObject(hDCBitmap, _hBitmap);
		bResult = BitBlt(_hDC, _X, _Y, Bitmap.bmWidth, Bitmap.bmHeight, hDCBitmap, 0, 0, _dwROP);
		SelectObject(hDCBitmap, hOld);
		DeleteDC(hDCBitmap);
	}

	return bResult;
}

BOOL oGDIStretchBitmap(HDC _hDC, INT _X, INT _Y, INT _Width, INT _Height, HBITMAP _hBitmap, DWORD _dwROP)
{
	HDC hDCBitmap = 0;
	BITMAP Bitmap;
	BOOL bResult = false;

	if (_hDC && _hBitmap)
	{
		hDCBitmap = CreateCompatibleDC(_hDC);
		GetObject(_hBitmap, sizeof(BITMAP), (LPSTR)&Bitmap);
		HGDIOBJ hOld = SelectObject(hDCBitmap, _hBitmap);
		bResult = StretchBlt(_hDC, _X, _Y, _Width, _Height, hDCBitmap, 0, 0, Bitmap.bmWidth, Bitmap.bmHeight, _dwROP);
		SelectObject(hDCBitmap, hOld);
		DeleteDC(hDCBitmap);
	}

	return bResult;
}

BOOL oGDIDrawText(HDC _hDC, const RECT* _pRect, unsigned int _ARGBForegroundColor, unsigned int _ARGBBackgroundColor, const char* _Options, const char* _Text)
{
	unsigned int r,g,b,a;
	oDecomposeColor(_ARGBForegroundColor, &r, &g, &b, &a);

	unsigned int br,bg,bb,ba;
	oDecomposeColor(_ARGBBackgroundColor, &br, &bg, &bb, &ba);

	if (!a)
	{
		if (!ba) return false;
		r = br;
		g = bg;
		b = bb;
	}

	UINT uFormat = DT_WORDBREAK;
	bool requestedSingleLine = false;
	if (_Options)
	{
		if (strchr(_Options, 'c')) uFormat |= DT_CENTER;
		else if (strchr(_Options, 'r')) uFormat |= DT_RIGHT;
		else uFormat |= DT_LEFT;
			
		if (strchr(_Options, 'm')) uFormat |= DT_VCENTER;
		else if (strchr(_Options, 'b')) uFormat |= DT_BOTTOM;
		else uFormat |= DT_TOP;

		requestedSingleLine = !!strchr(_Options, 's');
	}

	bool forcedSingleLine = !requestedSingleLine && ((uFormat & DT_BOTTOM) || (uFormat & DT_VCENTER));
	if (forcedSingleLine || requestedSingleLine)
	{
		if (forcedSingleLine)
			oTRACE_ONCE("GDI doesn't support multi-line, vertically aligned text. See DrawText docs for more details. http://msdn.microsoft.com/en-us/library/ms901121.aspx");
		uFormat &=~ DT_WORDBREAK;
		uFormat |= DT_SINGLELINE;
	}

	int oldBkMode = 0;
	COLORREF oldBkColor = 0;

	if (ba)
		oldBkColor = SetBkColor(_hDC, RGB(br,bg,bb));
	else
		oldBkMode = SetBkMode(_hDC, TRANSPARENT);
	
	COLORREF oldColor = SetTextColor(_hDC, RGB(r,g,b));
	RECT rect = *_pRect;
	DrawText(_hDC, _Text, -1, &rect, uFormat);
	SetTextColor(_hDC, oldColor);

	if (ba)
		SetBkColor(_hDC, oldBkColor);
	else
		SetBkMode(_hDC, oldBkMode);

	return true;
}

HPEN oGDICreatePen(unsigned int _ARGBColor, int _Width)
{
	unsigned int r,g,b,a;
	oDecomposeColor(_ARGBColor, &r, &g, &b, &a);
	return CreatePen(a ? PS_SOLID : PS_NULL, _Width, RGB(r,g,b));
}

HBRUSH oGDICreateBrush(unsigned int _ARGBColor)
{
	unsigned int r,g,b,a;
	oDecomposeColor(_ARGBColor, &r, &g, &b, &a);
	return a ? CreateSolidBrush(RGB(r,g,b)) : (HBRUSH)GetStockObject(HOLLOW_BRUSH);
}

HFONT oGDICreateFont(const char* _FontName, int _PointSize, bool _Bold, bool _Italics, bool _Underline)
{
		HDC hDC = GetDC(GetDesktopWindow());
		HFONT hFont = CreateFont(
			oPointToLogicalHeight(hDC, _PointSize)
			, 0
			, 0
			, 0
			, _Bold ? FW_BOLD : FW_NORMAL
			, _Italics
			, _Underline
			, FALSE
			, DEFAULT_CHARSET
			, OUT_DEFAULT_PRECIS
			, CLIP_DEFAULT_PRECIS
			, CLEARTYPE_QUALITY
			, DEFAULT_PITCH
			, _FontName);

	ReleaseDC(GetDesktopWindow(), hDC);
	return hFont;
}

const char* oGDIGetFontFamily(BYTE _tmPitchAndFamily)
{
	switch (_tmPitchAndFamily & 0xf0)
	{
		case FF_DECORATIVE: return "Decorative";
		case FF_DONTCARE: return "Don't care";
		case FF_MODERN: return "Modern";
		case FF_ROMAN: return "Roman";
		case FF_SCRIPT: return "Script";
		case FF_SWISS: return "Swiss";
		default: break;
	}

	return "Unknown";
}

const char* oGDIGetCharSet(BYTE _tmCharSet)
{
	switch (_tmCharSet)
	{
		case ANSI_CHARSET: return "ANSI";
		case BALTIC_CHARSET: return "Baltic";
		case CHINESEBIG5_CHARSET: return "Chinese Big5";
		case DEFAULT_CHARSET: return "Default";
		case EASTEUROPE_CHARSET: return "East Europe";
		case GB2312_CHARSET: return "GB2312";
		case GREEK_CHARSET: return "Greek";
		case HANGUL_CHARSET: return "Hangul";
		case MAC_CHARSET: return "Mac";
		case OEM_CHARSET: return "OEM";
		case RUSSIAN_CHARSET: return "Russian";
		case SHIFTJIS_CHARSET: return "Shift JIS";
		case SYMBOL_CHARSET: return "Symbol";
		case TURKISH_CHARSET: return "Turkish";
		case VIETNAMESE_CHARSET: return "Vietnamese";
		case JOHAB_CHARSET: return "Johab";
		case ARABIC_CHARSET: return "Arabic";
		case HEBREW_CHARSET: return "Hebrew";
		case THAI_CHARSET: return "Thai";
		default: break;
	}

	return "Unknown";
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
	UNIQUE_PROC(ProcName, 7)

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
	}

	oHOOKPROC GetHookProc(size_t _Index) { return HookContexts[_Index].UserHookProc; }
	void* GetHookUserData(size_t _Index) { return HookContexts[_Index].pUserData; }

	HOOK_CONTEXT* Allocate()
	{
		for (size_t i = 0; i < oCOUNTOF(HookContexts); i++)
		{
			HHOOK hh = HookContexts[i].hHook;
			// Do a quick mark of the slot so no other thread grabs it
			if (!hh && oCAS<HHOOK>(&HookContexts[i].hHook, (HHOOK)0x1337c0de, hh) == hh)
				return &HookContexts[i];
		}

		return 0;
	}

	void Deallocate(HHOOK _hHook)
	{
		for (size_t i = 0; i < oCOUNTOF(HookContexts); i++)
		{
			HHOOK hh = HookContexts[i].hHook;
			if (_hHook == hh && oCAS<HHOOK>(&HookContexts[i].hHook, 0, hh) == hh)
			{
					HookContexts[i].pUserData = 0;
					HookContexts[i].UserHookProc = 0;
			}
		}
	}

	HOOK_CONTEXT HookContexts[8];
};

HHOOK oSetWindowsHook(int _idHook, oHOOKPROC _pHookProc, void* _pUserData, HINSTANCE _hModule, DWORD _dwThreadId)
{
	HOOK_CONTEXT* hc = oWindowsHookContext::Singleton()->Allocate();
	if (!hc)
		return 0;

	hc->pUserData = _pUserData;
	hc->UserHookProc = _pHookProc;
	hc->hHook = ::SetWindowsHookExA(_idHook, hc->UniqueHookProc, _hModule, _dwThreadId);
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

void oPumpMessages(HWND _hWnd, unsigned int _TimeoutMS)
{
	MSG msg;
	unsigned int start = oTimerMS();
	while (PeekMessage(&msg, _hWnd, 0, 0, PM_REMOVE) && (_TimeoutMS == oINFINITE_WAIT || (oTimerMS() - start) < _TimeoutMS))
		TranslateMessage(&msg), DispatchMessage(&msg);
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
		oSetLastError(EINVAL, "EnvBlock buffer not large enough to contain converted EnvString");
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

HRESULT oCreateSimpleWindow(HWND* _pHwnd, WNDPROC _Wndproc, void* _pInstance, const char* _Title, int _X, int _Y, int _Width, int _Height, bool _SupportDoubleClicks)
{
	if (!_pHwnd) return E_FAIL;

	char className[32];
	sprintf_s(className, "SimpleWindow", _SupportDoubleClicks ? "DblClks" : "");

	WNDCLASSEXA wc;
	ZeroMemory(&wc, sizeof(WNDCLASSEX));
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.hInstance = GetModuleHandle(0);
	wc.lpfnWndProc = _Wndproc;                    
	wc.lpszClassName = className;                        
	wc.style = CS_BYTEALIGNCLIENT|CS_HREDRAW|CS_VREDRAW|CS_OWNDC|(_SupportDoubleClicks ? CS_DBLCLKS : 0);
	wc.hCursor = LoadCursor(0, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)(COLOR_BACKGROUND+1);
	RegisterClassEx(&wc);
	*_pHwnd = CreateWindowExA(WS_EX_ACCEPTFILES|WS_EX_APPWINDOW, className, _Title ? _Title : "", WS_OVERLAPPEDWINDOW, _X, _Y, _Width, _Height, 0, 0, 0, _pInstance);
	return *_pHwnd ? S_OK : GetLastError();
}

void* oGetWindowContext(HWND _hWnd, UINT _uMsg, WPARAM _wParam, LPARAM _lParam, char* _StrDescription, size_t _SizeOfStrDescription)
{
	if (!_hWnd)
		return 0;

	void* context = 0;

	if (_uMsg == WM_CREATE)
	{
		// a context (probably some 'this' pointer) was passed during the call 
		// to CreateWindow, so put that in userdata.
		CREATESTRUCTA* cs = (CREATESTRUCTA*)_lParam;
		SetWindowLongPtr(_hWnd, GWLP_USERDATA, (LONG_PTR)cs->lpCreateParams);
		#ifdef _DEBUG
			if (_StrDescription)
				sprintf_s(_StrDescription, _SizeOfStrDescription, "%s \"%s\" %d,%d %dx%d", cs->lpszClass, cs->lpszName, cs->x, cs->y, cs->cx, cs->cy);
		#endif
	}

	else if (_uMsg == WM_INITDIALOG)
	{
		// dialogs don't use CREATESTRUCT, so assume it's directly the context
		SetWindowLongPtr(_hWnd, GWLP_USERDATA, (LONG_PTR)_lParam);
		context = (void*)_lParam;
	}

	else
	{
		// For any other message, grab the context and return it
		context = _hWnd ? (void*)GetWindowLongPtr(_hWnd, GWLP_USERDATA) : 0;
	}

	return context;
}

void oRestoreWindow(HWND _hWnd)
{
	// There's a known issue that a simple ShowWindow doesn't
	// always work on some minimized apps. The WAR seems to be
	// to set focus to anything else, then try to restore the 
	// app.

	HWND hProgMan = FindWindow(0, "Program Manager");
	oASSERT(hProgMan, "Program Manager not found");
	oSetFocus(hProgMan);
	oSetFocus(_hWnd);
	ShowWindow(_hWnd, SW_SHOWDEFAULT);
}

void oRespectfulAnimateWindow(HWND _hWnd, const RECT* _pFrom, const RECT* _pTo)
{
	ANIMATIONINFO ai;
	ai.cbSize = sizeof(ai);
	SystemParametersInfo(SPI_GETANIMATION, sizeof(ai), &ai, 0);
	if (ai.iMinAnimate)
		oV(DrawAnimatedRects(_hWnd, IDANI_CAPTION, _pFrom, _pTo));
}

void oTaskbarGetRect(RECT* _pRect)
{
	APPBARDATA abd;
	abd.cbSize = sizeof(abd);
	SHAppBarMessage(ABM_GETTASKBARPOS, &abd);
	*_pRect = abd.rc;
}

HWND oTrayGetHwnd()
{
	static const char* sHierarchy[] = 
	{
		"Shell_TrayWnd",
		"TrayNotifyWnd",
		"SysPager",
		"ToolbarWindow32",
	};

	size_t i = 0;
	HWND hWnd = FindWindow(sHierarchy[i++], nullptr);
	while (hWnd && i < oCOUNTOF(sHierarchy))
		hWnd = FindWindowEx(hWnd, nullptr, sHierarchy[i++], nullptr);

	return hWnd;
}

// This API can be used to determine if the specified
// icon exists at all.
bool oTrayGetIconRect(HWND _hWnd, UINT _ID, RECT* _pRect)
{
	#ifdef oWINDOWS_HAS_TRAY_NOTIFYICONIDENTIFIER
		NOTIFYICONIDENTIFIER nii;
		memset(&nii, 0, sizeof(nii));
		nii.cbSize = sizeof(nii);
		nii.hWnd = _hWnd;
		nii.uID = _ID;
		HRESULT hr = Shell_NotifyIconGetRect(&nii, _pRect);
		return SUCCEEDED(hr);
	#else
		// http://social.msdn.microsoft.com/forums/en-US/winforms/thread/4ac8d81e-f281-4b32-9407-e663e6c234ae/
	
		HWND hTray = oTrayGetHwnd();
		DWORD TrayProcID;
		GetWindowThreadProcessId(hTray, &TrayProcID);
		HANDLE hTrayProc = OpenProcess(PROCESS_ALL_ACCESS, 0, TrayProcID);
		bool success = false;
		if (!hTrayProc)
		{
			TBBUTTON* lpBI = (TBBUTTON*)VirtualAllocEx(hTrayProc, nullptr, sizeof(TBBUTTON), MEM_COMMIT, PAGE_READWRITE);
			int nButtons = (int)SendMessage(hTray, TB_BUTTONCOUNT, 0, 0);
			for (int i = 0; i < nButtons; i++)
			{
				if (SendMessage(hTray, TB_GETBUTTON, i, (LPARAM)lpBI))
				{
					TBBUTTON bi;
					DWORD extraData[2];
					ReadProcessMemory(hTrayProc, lpBI, &bi, sizeof(TBBUTTON), NULL);
					ReadProcessMemory(hTrayProc, (LPCVOID)bi.dwData, extraData, sizeof(DWORD) * 2, nullptr);
					HWND IconNotifiesThisHwnd = (HWND)extraData[0];
					UINT IconID = extraData[1];

					if (_hWnd == IconNotifiesThisHwnd && _ID == IconID)
					{
						RECT r;
						RECT* lpRect = (RECT*)VirtualAllocEx(hTrayProc, nullptr, sizeof(RECT), MEM_COMMIT, PAGE_READWRITE);
						SendMessage(hTray, TB_GETITEMRECT, i, (LPARAM)lpRect);
						ReadProcessMemory(hTrayProc, lpRect, &r, sizeof(RECT), nullptr);
						VirtualFreeEx(hTrayProc, lpRect, 0, MEM_RELEASE);
						MapWindowPoints(hTray, nullptr, (LPPOINT)&r, 2);
						success = true;
						break;
					}
				}
			}

			VirtualFreeEx(hTrayProc, lpBI, 0, MEM_RELEASE);
			CloseHandle(hTrayProc);
		}

		return success;
	#endif
}

struct REMOVE_TRAY_ICON
{
	HWND hWnd;
	UINT ID;
	UINT TimeoutMS;
};

bool operator==(const REMOVE_TRAY_ICON& _RTI1, const REMOVE_TRAY_ICON& _RTI2) { return _RTI1.hWnd == _RTI2.hWnd && _RTI1.ID == _RTI2.ID; }

struct oTrayCleanup : public oProcessSingleton<oTrayCleanup>
{
	oTrayCleanup()
		: AllowInteraction(true)
	{}

	~oTrayCleanup()
	{
		oRWMutex::ScopedLock lock(Mutex);

		if (!Removes.empty())
		{
			char buf[oKB(1)];
			sprintf_s(buf, "oWindows Trace %s Cleaning up tray icons\n", oGetExecutionPath());
			oThreadsafeOutputDebugStringA(buf);
		}

		AllowInteraction = false;
		for (size_t i = 0; i < Removes.size(); i++)
			oTrayShowIcon(Removes[i].hWnd, Removes[i].ID, 0, 0, false);

		Removes.clear();
	}

	void Register(HWND _hWnd, UINT _ID)
	{
		if (!AllowInteraction)
			return;

		if (Removes.size() < Removes.capacity())
		{
			REMOVE_TRAY_ICON rti;
			rti.hWnd = _hWnd;
			rti.ID = _ID;
			rti.TimeoutMS = 0;
			oRWMutex::ScopedLock lock(Mutex);
			Removes.push_back(rti);
		}
		else
			oThreadsafeOutputDebugStringA("--- Too many tray icons registered for cleanup: ignoring. ---");
	}

	void Unregister(HWND _hWnd, UINT _ID)
	{
		if (!AllowInteraction)
			return;

		REMOVE_TRAY_ICON rti;
		rti.hWnd = _hWnd;
		rti.ID = _ID;
		rti.TimeoutMS = 0;

		oRWMutex::ScopedLock lock(Mutex);
		oFindAndErase(Removes, rti);
	}

	oArray<REMOVE_TRAY_ICON, 20> Removes;
	oRWMutex Mutex;
	volatile bool AllowInteraction;
};

void oTrayShowIcon(HWND _hWnd, UINT _ID, UINT _CallbackMessage, HICON _hIcon, bool _Show)
{
	NOTIFYICONDATA nid;
	memset(&nid, 0, sizeof(nid));
	nid.cbSize = sizeof(nid);
	nid.hWnd = _hWnd;
	nid.hIcon = _hIcon ? _hIcon : oGetIcon(_hWnd, false);
	if (!nid.hIcon)
		nid.hIcon = oGetIcon(_hWnd, true);
	nid.uID = _ID;
	nid.uCallbackMessage = _CallbackMessage;
	nid.uFlags = NIF_ICON | (_CallbackMessage ? NIF_MESSAGE : 0);
	nid.uVersion = NOTIFYICON_VERSION_4;
	oV(Shell_NotifyIcon(_Show ? NIM_ADD : NIM_DELETE, &nid));

	oTrayCleanup* pTrayCleanup = oTrayCleanup::Singleton();
	if (_Show)
	{
		// Ensure we know exactly what version behavior we're dealing with
		oV(Shell_NotifyIcon(NIM_SETVERSION, &nid));

		if (oIsValidSingletonPointer(pTrayCleanup))
			pTrayCleanup->Register(_hWnd, _ID);
	}

	else
	{
		if (oIsValidSingletonPointer(pTrayCleanup))
			pTrayCleanup->Unregister(_hWnd, _ID);
	}
}

void oTraySetFocus()
{
	oV(Shell_NotifyIcon(NIM_SETFOCUS, nullptr));
}

static DWORD WINAPI oTrayScheduleIconHide_Proc(LPVOID lpParameter)
{
	REMOVE_TRAY_ICON* pRTI = (REMOVE_TRAY_ICON*)lpParameter;
	Sleep(pRTI->TimeoutMS);
	oTRACE("Auto-closing tray icon HWND=0x%p ID=%u", pRTI->hWnd, pRTI->ID);
	oTrayShowIcon(pRTI->hWnd, pRTI->ID, 0, 0, false);
	delete pRTI;
	ExitThread(0);
}

static void oTrayScheduleIconHide(HWND _hWnd, UINT _ID, unsigned int _TimeoutMS)
{
	REMOVE_TRAY_ICON* pRTI = new REMOVE_TRAY_ICON();
	pRTI->hWnd = _hWnd;
	pRTI->ID = _ID;
	pRTI->TimeoutMS = _TimeoutMS;

	#ifdef _DEBUG
		HANDLE hThread = 
	#endif
	CreateThread(nullptr, oKB(64), oTrayScheduleIconHide_Proc, pRTI, 0, nullptr);
	oASSERT(hThread, "");
}

bool oTrayShowMessage(HWND _hWnd, UINT _ID, HICON _hIcon, UINT _TimeoutMS, const char* _Title, const char* _Message)
{
	NOTIFYICONDATA nid;
	memset(&nid, 0, sizeof(nid));
	nid.cbSize = sizeof(nid);
	nid.hWnd = _hWnd;
	nid.hIcon = _hIcon ? _hIcon : oGetIcon(_hWnd, false);
	if (!nid.hIcon)
		nid.hIcon = oGetIcon(_hWnd, true);
	nid.uID = _ID;
	nid.uFlags = NIF_INFO;
	nid.uTimeout = __max(__min(_TimeoutMS, 30000), 10000);

	// MS recommends truncating at 200 for English: http://msdn.microsoft.com/en-us/library/bb773352(v=vs.85).aspx
	strcpy_s(nid.szInfo, 201, oSAFESTR(_Message));
	oAddTruncationElipse(nid.szInfo, 201);

	// MS recommends truncating at 48 for English: http://msdn.microsoft.com/en-us/library/bb773352(v=vs.85).aspx
	strcpy_s(nid.szInfoTitle, 49, oSAFESTR(_Title));

	nid.dwInfoFlags = NIIF_NOSOUND;

	#ifdef oWINDOWS_HAS_TRAY_QUIETTIME
		nid.dwInfoFlags |= NIIF_RESPECT_QUIET_TIME;
	#endif

	RECT r;
	if (!oTrayGetIconRect(_hWnd, _ID, &r))
	{
		UINT timeout = 0;
		switch (oGetWindowsVersion())
		{
			case oWINDOWS_2000:
			case oWINDOWS_XP:
			case oWINDOWS_SERVER_2003:
				timeout = nid.uTimeout;
				break;
			default:
			{
				ULONG duration = 0;
				oV(SystemParametersInfo(SPI_GETMESSAGEDURATION, 0, &duration, 0));
				timeout = (UINT)duration * 1000;
				break;
			};
		}

		oTrayShowIcon(_hWnd, _ID, 0, _hIcon, true);
		if (timeout != oINFINITE_WAIT)
			oTrayScheduleIconHide(_hWnd, _ID, timeout);
	}

	if (!Shell_NotifyIcon(NIM_MODIFY, &nid))
	{
		oWinSetLastError();
		return false;
	}

	return true;
}

void oTrayDecodeCallbackMessageParams(WPARAM _wParam, LPARAM _lParam, UINT* _pNotificationEvent, UINT* _pID, int* _pX, int* _pY)
{
	// http://msdn.microsoft.com/en-us/library/bb773352(v=vs.85).aspx
	// Search for uCallbackMessage

	*_pNotificationEvent = LOWORD(_lParam);
	*_pID = HIWORD(_lParam);
	if (_pX)
		*_pX = GET_X_LPARAM(_wParam);
	if (_pY)
		*_pY = GET_Y_LPARAM(_wParam);
}

// false means animate from sys tray out to window position
static void oTrayRespectfulAnimateWindow(HWND _hWnd, bool _ToSysTray)
{
	RECT rDesktop, rWindow;
	GetWindowRect(GetDesktopWindow(), &rDesktop);
	GetWindowRect(_hWnd, &rWindow);
	rDesktop.left = rDesktop.right;
	rDesktop.top = rDesktop.bottom;
	RECT* from = _ToSysTray ? &rWindow : &rDesktop;
	RECT* to = _ToSysTray ? &rDesktop : &rWindow;
	oRespectfulAnimateWindow(_hWnd, from, to);
}

void oTrayMinimize(HWND _hWnd, UINT _CallbackMessage, HICON _hIcon)
{
	oTrayRespectfulAnimateWindow(_hWnd, true);
	ShowWindow(_hWnd, SW_HIDE);
	oTrayShowIcon(_hWnd, 0, _CallbackMessage, _hIcon, true);
}

void oTrayRestore(HWND _hWnd)
{
	oTrayRespectfulAnimateWindow(_hWnd, false);
	ShowWindow(_hWnd, SW_SHOW);
	SetActiveWindow(_hWnd);
	SetForegroundWindow(_hWnd);
	oTrayShowIcon(_hWnd, 0, 0, 0, false);
}

#if oDXVER >= oDXVER_10

typedef HRESULT (__stdcall *CreateDXGIFactoryFn1)(REFIID riid, void **ppFactory);
typedef HRESULT (__stdcall *DWriteCreateFactoryFn)(DWRITE_FACTORY_TYPE factoryType, REFIID iid, IUnknown **factory);
typedef HRESULT (__stdcall *D2D1CreateFactoryFn)(D2D1_FACTORY_TYPE factoryType, REFIID riid, const D2D1_FACTORY_OPTIONS *pFactoryOptions, void **ppIFactory);
typedef DWORD (__stdcall *GetThreadIdFn)(HANDLE Thread);

struct oVistaAPIsContext : oProcessSingleton<oVistaAPIsContext>
{
	oVistaAPIsContext()
		: hDXGIModule(0)
		, hDWriteModule(0)
		, hD2DModule(0)
		, oGetThreadId(0)
		, oCreateDXGIFactory1(0)
		, oD2D1CreateFactory(0)
		, oDWriteCreateFactory(0)
	{
		if (LoadModules())
		{
			if (!GetProcs() || !LoadSingletons())
			{
				oASSERT(false, "Could not soft-link to all APIs from DXGI DirectWrite and/or Direct2D.");
				FreeModules();
			}
		}
	}

	~oVistaAPIsContext()
	{
		FreeSingletons();
		FreeModules();
	}

	oRef<IDWriteFactory> DWriteFactory;
	GetThreadIdFn oGetThreadId;
	CreateDXGIFactoryFn1 oCreateDXGIFactory1;
	D2D1CreateFactoryFn oD2D1CreateFactory;

protected:

	bool LoadModules()
	{
		hDXGIModule = oModule::oSafeLoadLibrary("dxgi");
		hDWriteModule = oModule::oSafeLoadLibrary("dwrite");
		hD2DModule = oModule::oSafeLoadLibrary("d2d1");
		return hDXGIModule && hDWriteModule && hD2DModule;
	}

	void FreeModules()
	{
		oCreateDXGIFactory1 = 0;
		oDWriteCreateFactory = 0;
		oD2D1CreateFactory = 0;

		if (hDXGIModule)
		{
			oModule::oSafeFreeLibrary(hDXGIModule);
			hDXGIModule = 0;
		}

		if (hDWriteModule)
		{
			oModule::oSafeFreeLibrary(hD2DModule);
			hDWriteModule = 0;
		}

		if (hD2DModule)
		{
			oModule::oSafeFreeLibrary(hD2DModule);
			hD2DModule = 0;
		}
	}

	bool GetProcs()
	{
		oCreateDXGIFactory1 = reinterpret_cast<CreateDXGIFactoryFn1>(GetProcAddress((HMODULE)hDXGIModule, "CreateDXGIFactory1"));
		oDWriteCreateFactory = reinterpret_cast<DWriteCreateFactoryFn>(GetProcAddress((HMODULE)hDWriteModule, "DWriteCreateFactory"));
		oD2D1CreateFactory = reinterpret_cast<D2D1CreateFactoryFn>(GetProcAddress((HMODULE)hD2DModule, "D2D1CreateFactory"));
		oGetThreadId = reinterpret_cast<GetThreadIdFn>(GetProcAddress(GetModuleHandleA("kernel32"), "GetThreadId"));
		return oCreateDXGIFactory1 && oDWriteCreateFactory && oD2D1CreateFactory;
	}

	bool LoadSingletons()
	{
		if (S_OK != oDWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), (IUnknown**)&DWriteFactory))
			return false;
		return true;
	}

	void FreeSingletons()
	{
		// @oooii-tony: Because we're creating a shared DWrite interface, this 
		// isn't always the last ref on the factory
		//oASSERT(oGetRefCount(DWriteFactory) == 1, "Outstanding refcount (%u) on DWrite interfaces", oGetRefCount(DWriteFactory));
		DWriteFactory = 0;
	}

	oHMODULE hDXGIModule;
	oHMODULE hDWriteModule;
	oHMODULE hD2DModule;

	DWriteCreateFactoryFn oDWriteCreateFactory;
};


bool oCreateDXGIFactory( IDXGIFactory1** _ppFactory )
{
	if (S_OK != oVistaAPIsContext::Singleton()->oCreateDXGIFactory1(__uuidof(IDXGIFactory1), (void**)_ppFactory))
		return false;

	return true;
}

HRESULT oDXGICreateSwapchain(IUnknown* _pDevice, unsigned int Width, unsigned int Height, unsigned int RefreshRateN, unsigned int RefreshRateD, DXGI_FORMAT _Fmt, HWND _hWnd, IDXGISwapChain** _ppSwapChain)
{
	DXGI_SWAP_CHAIN_DESC scDesc;
	ZeroMemory( &scDesc, sizeof( scDesc ) );

	DXGI_MODE_DESC& ModeDesc = scDesc.BufferDesc;
	ModeDesc.Format = _Fmt;
	ModeDesc.Height = Height;
	ModeDesc.Width = Width;
	ModeDesc.RefreshRate.Numerator = RefreshRateN;
	ModeDesc.RefreshRate.Denominator = RefreshRateD;

	DXGI_SAMPLE_DESC& SampleDesc = scDesc.SampleDesc;
	SampleDesc.Count = 1;
	SampleDesc.Quality = 0;

	scDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT | DXGI_USAGE_SHADER_INPUT;
	scDesc.BufferCount = 3;
	scDesc.OutputWindow = _hWnd;
	scDesc.Windowed = true;
	scDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	
	oRef<IDXGIDevice> D3DDevice;
	oV_RETURN( _pDevice->QueryInterface(&D3DDevice) );

	oRef<IDXGIAdapter> Adapter;
	oV_RETURN( D3DDevice->GetParent( __uuidof(IDXGIAdapter), (void**)&Adapter ) );

	oRef<IDXGIFactory> Factory;
	oV_RETURN( Adapter->GetParent( __uuidof(IDXGIFactory), (void**)&Factory ) );

	return Factory->CreateSwapChain( _pDevice, &scDesc, _ppSwapChain );
}

bool oDXGIGetAdapterWithMonitor(const RECT& _Rect, IDXGIAdapter1** _ppAdapter, IDXGIOutput** _ppOutput)
{
	oRef<IDXGIFactory1> Factory;
	if( !oCreateDXGIFactory(&Factory) )
		return false;

	oRECT WindowRegion = oToRect(_Rect);

	int BestAdapterCoverage = -1;
	oRef<IDXGIAdapter1> BestAdapter;
	oRef<IDXGIOutput> BestOutput;
	oRef<IDXGIAdapter1> Adapter;
	
	UINT a = 0;
	while( S_OK == Factory->EnumAdapters1( a++, &Adapter) )
	{
		oRef<IDXGIOutput> Output;
		UINT o = 0;
		while (DXGI_ERROR_NOT_FOUND != Adapter->EnumOutputs(o++, &Output) )
		{
			DXGI_OUTPUT_DESC desc;
			Output->GetDesc(&desc);

			oRECT MonitorRegion = oToRect( desc.DesktopCoordinates );

			oRECT ClippedRect = oClip(WindowRegion, MonitorRegion);
			if (ClippedRect.IsEmpty())
				continue;

			int2 dim = ClippedRect.GetDimensions();
			
			int coverage = dim.x * dim.y;
			if( coverage > BestAdapterCoverage )
			{
				BestAdapterCoverage = coverage;
				BestAdapter = Adapter;
				BestOutput = Output;
			}
		}
	}
	if( BestAdapter )
	{
		BestAdapter->AddRef();
		BestOutput->AddRef();
		*_ppAdapter = BestAdapter;
		*_ppOutput = BestOutput;
		return true;
	}

	return false;
}


bool oD2D1CreateFactory(ID2D1Factory** _ppFactory)
{
	D2D1_FACTORY_OPTIONS opt;
	opt.debugLevel = 
#ifdef _DEBUG
		D2D1_DEBUG_LEVEL_WARNING
#else
		D2D1_DEBUG_LEVEL_NONE
#endif
	;	
	return(S_OK == oVistaAPIsContext::Singleton()->oD2D1CreateFactory(D2D1_FACTORY_TYPE_MULTI_THREADED, __uuidof(ID2D1Factory), &opt, (void**)_ppFactory));
}

IDWriteFactory* oGetDWriteFactorySingleton()
{
	return oVistaAPIsContext::Singleton()->DWriteFactory;
}

float oDXGIGetD3DVersion(IDXGIAdapter* _pAdapter)
{
	#if D3D11_MAJOR_VERSION
		if (_pAdapter->CheckInterfaceSupport(__uuidof(ID3D11Device), 0)) return 11.0f;
	#endif
	#ifdef _D3D10_1_CONSTANTS
		if (_pAdapter->CheckInterfaceSupport(__uuidof(ID3D10Device1), 0)) return 10.1f;
	#endif
	#ifdef _D3D10_CONSTANTS
		if (_pAdapter->CheckInterfaceSupport(__uuidof(ID3D10Device), 0)) return 10.0f;
	#endif
	return 0.0f;
}

bool oDXGIFindOutput(IDXGIFactory* _pFactory, HMONITOR _hMonitor, IDXGIOutput** _ppOutput)
{
	if (!_pFactory || !_hMonitor || !_ppOutput) return false;

	*_ppOutput = 0;
	IDXGIAdapter* pAdapter = 0;
	unsigned int i = 0;
	while (!*_ppOutput && DXGI_ERROR_NOT_FOUND != _pFactory->EnumAdapters(i++, &pAdapter))
	{
		IDXGIOutput* pOutput = 0;

		unsigned int o = 0;
		while (DXGI_ERROR_NOT_FOUND != pAdapter->EnumOutputs(o++, &pOutput))
		{
			DXGI_OUTPUT_DESC desc;
			pOutput->GetDesc(&desc);
			if (desc.Monitor == _hMonitor)
			{
				*_ppOutput = pOutput;
				break;
			}

			pOutput->Release();
		}

		pAdapter->Release();
	}

	return !!*_ppOutput;
}

bool oDXGIIsDepthFormat(DXGI_FORMAT _Format)
{
	switch (_Format)
	{
		case DXGI_FORMAT_D32_FLOAT:
		case DXGI_FORMAT_R32_TYPELESS:
		case DXGI_FORMAT_D24_UNORM_S8_UINT:
		case DXGI_FORMAT_R24_UNORM_X8_TYPELESS:
		case DXGI_FORMAT_D32_FLOAT_S8X24_UINT:
		case DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS:
		case DXGI_FORMAT_D16_UNORM:
		case DXGI_FORMAT_R16_TYPELESS:
			return true;
		default:
			break;
	}

	return false;
}

DXGI_FORMAT oDXGIGetDepthCompatibleFormat(DXGI_FORMAT _TypelessDepthFormat)
{
	switch (_TypelessDepthFormat)
	{
		case DXGI_FORMAT_R32_TYPELESS: return DXGI_FORMAT_D32_FLOAT;
		case DXGI_FORMAT_R24_UNORM_X8_TYPELESS: return DXGI_FORMAT_D24_UNORM_S8_UINT;
		case DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS: return DXGI_FORMAT_D32_FLOAT_S8X24_UINT;
		case DXGI_FORMAT_R16_TYPELESS: return DXGI_FORMAT_D16_UNORM;
		default: return _TypelessDepthFormat;
	}
}

DXGI_FORMAT oDXGIGetColorCompatibleFormat(DXGI_FORMAT _DepthFormat)
{
	switch (_DepthFormat)
	{
		case DXGI_FORMAT_R32_TYPELESS:
		case DXGI_FORMAT_D32_FLOAT: return DXGI_FORMAT_R32_FLOAT;
		case DXGI_FORMAT_R24_UNORM_X8_TYPELESS:
		case DXGI_FORMAT_D24_UNORM_S8_UINT: return DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
		case DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS:
		case DXGI_FORMAT_D32_FLOAT_S8X24_UINT: return DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS;
		case DXGI_FORMAT_R16_TYPELESS:
		case DXGI_FORMAT_D16_UNORM: return DXGI_FORMAT_R16_UNORM;
		default: return _DepthFormat;
	}
}

#endif // oDXVER >= oDXVER_10

#if oDXVER >= oDXVER_11
	float oGetD3DVersion(D3D_FEATURE_LEVEL _Level)
	{
		switch (_Level)
		{
			case D3D_FEATURE_LEVEL_11_0: return 11.0f;
			case D3D_FEATURE_LEVEL_10_1: return 10.1f;
			case D3D_FEATURE_LEVEL_10_0: return 10.0f;
			case D3D_FEATURE_LEVEL_9_3: return 9.3f;
			case D3D_FEATURE_LEVEL_9_2: return 9.2f;
			case D3D_FEATURE_LEVEL_9_1: return 9.1f;
			default: oASSUME(0);
		}
	}

#endif // oDXVER >= oDXVER_11

unsigned int oGetDisplayDevice(HMONITOR _hMonitor, DISPLAY_DEVICE* _pDevice)
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

	return oINVALID;
}

unsigned int oGetWindowDisplayIndex(HWND _hWnd)
{
	DISPLAY_DEVICE dev;
	return oGetDisplayDevice(MonitorFromWindow(_hWnd, MONITOR_DEFAULTTONEAREST), &dev);
}

void oGetVirtualDisplayRect(RECT* _pRect)
{
	_pRect->left = GetSystemMetrics(SM_XVIRTUALSCREEN);
	_pRect->top = GetSystemMetrics(SM_YVIRTUALSCREEN);
	_pRect->right = _pRect->left + GetSystemMetrics(SM_CXVIRTUALSCREEN);
	_pRect->bottom = _pRect->top + GetSystemMetrics(SM_CYVIRTUALSCREEN);
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

char* oGetWMDesc(char* _StrDestination, size_t _SizeofStrDestination, HWND _hWnd, UINT _uMsg, WPARAM _wParam, LPARAM _lParam)
{
	switch (_uMsg)
	{ 
		case WM_ACTIVATE: sprintf_s(_StrDestination, _SizeofStrDestination, "WM_ACTIVATE"); break;
		case WM_ACTIVATEAPP: sprintf_s(_StrDestination, _SizeofStrDestination, "WM_ACTIVATEAPP"); break;
		case WM_APPCOMMAND: sprintf_s(_StrDestination, _SizeofStrDestination, "WM_APPCOMMAND"); break;
		case WM_CANCELMODE: sprintf_s(_StrDestination, _SizeofStrDestination, "WM_CANCELMODE"); break;
		case WM_CHAR: sprintf_s(_StrDestination, _SizeofStrDestination, "WM_CHAR"); break;
		case WM_CHILDACTIVATE: sprintf_s(_StrDestination, _SizeofStrDestination, "WM_CHILDACTIVATE"); break;
		case WM_CLOSE: sprintf_s(_StrDestination, _SizeofStrDestination, "WM_CLOSE"); break;
		case WM_COMPACTING: sprintf_s(_StrDestination, _SizeofStrDestination, "WM_COMPACTING"); break;
		case WM_COMPAREITEM: sprintf_s(_StrDestination, _SizeofStrDestination, "WM_COMPAREITEM"); break;
		case WM_CREATE: sprintf_s(_StrDestination, _SizeofStrDestination, "WM_CREATE"); break;
		case WM_DEADCHAR: sprintf_s(_StrDestination, _SizeofStrDestination, "WM_DEADCHAR"); break;
		case WM_DESTROY: sprintf_s(_StrDestination, _SizeofStrDestination, "WM_DESTROY"); break;
		case WM_DISPLAYCHANGE: sprintf_s(_StrDestination, _SizeofStrDestination, "WM_DISPLAYCHANGE %dx%dx%d", static_cast<int>(LOWORD(_lParam)), static_cast<int>(HIWORD(_lParam)), _wParam); break;
		case WM_DRAWITEM: sprintf_s(_StrDestination, _SizeofStrDestination, "WM_DRAWITEM"); break;
		case WM_DWMCOLORIZATIONCOLORCHANGED: sprintf_s(_StrDestination, _SizeofStrDestination, "WM_DWMCOLORIZATIONCOLORCHANGED"); break;
		case WM_DWMCOMPOSITIONCHANGED: sprintf_s(_StrDestination, _SizeofStrDestination, "WM_DWMCOMPOSITIONCHANGED"); break;
		case WM_DWMNCRENDERINGCHANGED: sprintf_s(_StrDestination, _SizeofStrDestination, "WM_DWMNCRENDERINGCHANGED Desktop Window Manager (DWM) %s", (BOOL)_wParam ? "enabled" : "disabled"); break;
		case WM_DWMWINDOWMAXIMIZEDCHANGE: sprintf_s(_StrDestination, _SizeofStrDestination, "WM_DWMWINDOWMAXIMIZEDCHANGE"); break;
		case WM_ENABLE: sprintf_s(_StrDestination, _SizeofStrDestination, "WM_ENABLE"); break;
		case WM_ENTERSIZEMOVE: sprintf_s(_StrDestination, _SizeofStrDestination, "WM_ENTERSIZEMOVE"); break;
		case WM_ERASEBKGND: sprintf_s(_StrDestination, _SizeofStrDestination, "WM_ERASEBKGND"); break;
		case WM_EXITSIZEMOVE: sprintf_s(_StrDestination, _SizeofStrDestination, "WM_EXITSIZEMOVE"); break;
		case WM_GETICON: sprintf_s(_StrDestination, _SizeofStrDestination, "WM_GETICON"); break;
		case WM_GETMINMAXINFO: sprintf_s(_StrDestination, _SizeofStrDestination, "WM_GETMINMAXINFO"); break;
		case WM_GETTEXT: sprintf_s(_StrDestination, _SizeofStrDestination, "WM_GETTEXT"); break;
		case WM_GETTEXTLENGTH: sprintf_s(_StrDestination, _SizeofStrDestination, "WM_GETTEXTLENGTH"); break;
		case WM_HOTKEY: sprintf_s(_StrDestination, _SizeofStrDestination, "WM_HOTKEY"); break;
		case WM_ICONERASEBKGND: sprintf_s(_StrDestination, _SizeofStrDestination, "WM_ICONERASEBKGND"); break;
		case WM_INPUTLANGCHANGE: sprintf_s(_StrDestination, _SizeofStrDestination, "WM_INPUTLANGCHANGE"); break;
		case WM_INPUTLANGCHANGEREQUEST: sprintf_s(_StrDestination, _SizeofStrDestination, "WM_INPUTLANGCHANGEREQUEST"); break;
		case WM_KEYDOWN: sprintf_s(_StrDestination, _SizeofStrDestination, "WM_KEYDOWN"); break;
		case WM_KEYUP: sprintf_s(_StrDestination, _SizeofStrDestination, "WM_KEYUP"); break;
		case WM_KILLFOCUS: sprintf_s(_StrDestination, _SizeofStrDestination, "WM_KILLFOCUS"); break;
		case WM_MEASUREITEM: sprintf_s(_StrDestination, _SizeofStrDestination, "WM_MEASUREITEM"); break;
		case WM_MOVE: sprintf_s(_StrDestination, _SizeofStrDestination, "WM_MOVE %ux%u", static_cast<int>(LOWORD(_lParam)), static_cast<int>(HIWORD(_lParam))); break;
		case WM_MOVING: sprintf_s(_StrDestination, _SizeofStrDestination, "WM_MOVING"); break;
		case WM_NCMOUSEMOVE: sprintf_s(_StrDestination, _SizeofStrDestination, "WM_NCMOUSEMOVE"); break;
		case WM_NCMOUSELEAVE: sprintf_s(_StrDestination, _SizeofStrDestination, "WM_NCMOUSELEAVE"); break;
		case WM_NCACTIVATE: sprintf_s(_StrDestination, _SizeofStrDestination, "WM_NCACTIVATE"); break;
		case WM_NCCALCSIZE: sprintf_s(_StrDestination, _SizeofStrDestination, "WM_NCCALCSIZE"); break;
		case WM_NCCREATE: sprintf_s(_StrDestination, _SizeofStrDestination, "WM_NCCREATE"); break;
		case WM_NCDESTROY: sprintf_s(_StrDestination, _SizeofStrDestination, "WM_NCDESTROY"); break;
		case WM_NCPAINT: sprintf_s(_StrDestination, _SizeofStrDestination, "WM_NCPAINT"); break;
		case WM_NULL: sprintf_s(_StrDestination, _SizeofStrDestination, "WM_NULL"); break;
		case WM_PAINT: sprintf_s(_StrDestination, _SizeofStrDestination, "WM_PAINT"); break;
		case WM_PRINT: sprintf_s(_StrDestination, _SizeofStrDestination, "WM_PRINT"); break;
		case WM_PRINTCLIENT: sprintf_s(_StrDestination, _SizeofStrDestination, "WM_PRINTCLIENT"); break;
		case WM_PAINTICON: sprintf_s(_StrDestination, _SizeofStrDestination, "WM_PAINTICON"); break;
		case WM_PARENTNOTIFY: sprintf_s(_StrDestination, _SizeofStrDestination, "WM_PARENTNOTIFY"); break;
		case WM_QUERYDRAGICON: sprintf_s(_StrDestination, _SizeofStrDestination, "WM_QUERYDRAGICON"); break;
		case WM_QUERYOPEN: sprintf_s(_StrDestination, _SizeofStrDestination, "WM_QUERYOPEN"); break;
		case WM_NCHITTEST: sprintf_s(_StrDestination, _SizeofStrDestination, "WM_NCHITTEST"); break;
		case WM_SETCURSOR: sprintf_s(_StrDestination, _SizeofStrDestination, "WM_SETCURSOR %s hit=%d, id=%d", (HWND)_wParam == _hWnd ? "Is in this window" : "outside this window", LOWORD(_lParam), HIWORD(_lParam)); break;
		case WM_SETFOCUS: sprintf_s(_StrDestination, _SizeofStrDestination, "WM_SETFOCUS"); break;
		case WM_SETICON: sprintf_s(_StrDestination, _SizeofStrDestination, "WM_SETICON"); break;
		case WM_SETREDRAW: sprintf_s(_StrDestination, _SizeofStrDestination, "WM_SETREDRAW"); break;
		case WM_SETTEXT: sprintf_s(_StrDestination, _SizeofStrDestination, "WM_SETTEXT"); break;
		case WM_SHOWWINDOW: sprintf_s(_StrDestination, _SizeofStrDestination, "WM_SHOWWINDOW"); break;
		case WM_SIZE: sprintf_s(_StrDestination, _SizeofStrDestination, "WM_SIZE %ux%u", static_cast<unsigned int>(LOWORD(_lParam)), static_cast<unsigned int>(HIWORD(_lParam))); break;
		case WM_SIZING: sprintf_s(_StrDestination, _SizeofStrDestination, "WM_SIZING"); break;
		case WM_STYLECHANGED: sprintf_s(_StrDestination, _SizeofStrDestination, "WM_STYLECHANGED"); break;
		case WM_STYLECHANGING: sprintf_s(_StrDestination, _SizeofStrDestination, "WM_STYLECHANGING"); break;
		case WM_SYSDEADCHAR: sprintf_s(_StrDestination, _SizeofStrDestination, "WM_SYSDEADCHAR"); break;
		case WM_SYSKEYDOWN: sprintf_s(_StrDestination, _SizeofStrDestination, "WM_SYSKEYDOWN"); break;
		case WM_SYSKEYUP: sprintf_s(_StrDestination, _SizeofStrDestination, "WM_SYSKEYUP"); break;
		case WM_THEMECHANGED: sprintf_s(_StrDestination, _SizeofStrDestination, "WM_THEMECHANGED"); break;
		case WM_UNICHAR: sprintf_s(_StrDestination, _SizeofStrDestination, "WM_UNICHAR"); break;
		case WM_USERCHANGED: sprintf_s(_StrDestination, _SizeofStrDestination, "WM_USERCHANGED"); break;
		case WM_WINDOWPOSCHANGED: sprintf_s(_StrDestination, _SizeofStrDestination, "WM_WINDOWPOSCHANGED"); break;
		case WM_WINDOWPOSCHANGING: sprintf_s(_StrDestination, _SizeofStrDestination, "WM_WINDOWPOSCHANGING"); break;
		case WM_MOUSEMOVE: sprintf_s(_StrDestination, _SizeofStrDestination, "WM_MOUSEMOVE %dx%d", LOWORD(_lParam), HIWORD(_lParam)); break;
		case WM_LBUTTONDOWN: sprintf_s(_StrDestination, _SizeofStrDestination, "WM_LBUTTONDOWN %dx%d", LOWORD(_lParam), HIWORD(_lParam)); break;
		case WM_LBUTTONUP: sprintf_s(_StrDestination, _SizeofStrDestination, "WM_LBUTTONUP %dx%d", LOWORD(_lParam), HIWORD(_lParam)); break;
		case WM_MBUTTONDOWN: sprintf_s(_StrDestination, _SizeofStrDestination, "WM_MBUTTONDOWN %dx%d", LOWORD(_lParam), HIWORD(_lParam)); break;
		case WM_MBUTTONUP: sprintf_s(_StrDestination, _SizeofStrDestination, "WM_MBUTTONUP %dx%d", LOWORD(_lParam), HIWORD(_lParam)); break;
		case WM_RBUTTONDOWN: sprintf_s(_StrDestination, _SizeofStrDestination, "WM_RBUTTONDOWN %dx%d", LOWORD(_lParam), HIWORD(_lParam)); break;
		case WM_RBUTTONUP: sprintf_s(_StrDestination, _SizeofStrDestination, "WM_RBUTTONUP %dx%d", LOWORD(_lParam), HIWORD(_lParam)); break;
		case WM_XBUTTONDOWN: sprintf_s(_StrDestination, _SizeofStrDestination, "WM_XBUTTONDOWN(%d) %dx%d", GET_XBUTTON_WPARAM(_wParam), LOWORD(_lParam), HIWORD(_lParam)); break;
		case WM_XBUTTONUP: sprintf_s(_StrDestination, _SizeofStrDestination, "WM_XBUTTONUP(%d) %dx%d", GET_XBUTTON_WPARAM(_wParam), LOWORD(_lParam), HIWORD(_lParam)); break;
		case WM_MOUSEWHEEL: sprintf_s(_StrDestination, _SizeofStrDestination, "WM_MOUSEWHEEL %d", GET_WHEEL_DELTA_WPARAM(_wParam)); break;
		case WM_MOUSEHWHEEL: sprintf_s(_StrDestination, _SizeofStrDestination, "WM_MOUSEHWHEEL %d", GET_WHEEL_DELTA_WPARAM(_wParam)); break;
		case WM_CONTEXTMENU: sprintf_s(_StrDestination, _SizeofStrDestination, "WM_CONTEXTMENU"); break;
		default: sprintf_s(_StrDestination, _SizeofStrDestination, "Unrecognized uMsg=%u", _uMsg); break;
	}

	return _StrDestination;
}

void oGetScreenDPIScale(float* _pScaleX, float* _pScaleY)
{
	HDC screen = GetDC(0);
	*_pScaleX = GetDeviceCaps(screen, LOGPIXELSX) / 96.0f;
	*_pScaleY = GetDeviceCaps(screen, LOGPIXELSY) / 96.0f;
	ReleaseDC(0, screen);
}

static void InitializeCS(void* _Memory)
{
	InitializeCriticalSection((CRITICAL_SECTION*)_Memory);
}

static CRITICAL_SECTION* sOutputDebugStringCS = 0;
void oThreadsafeOutputDebugStringA_AtExit()
{
	DeleteCriticalSection(sOutputDebugStringCS);
	oProcessHeap::Deallocate(sOutputDebugStringCS);
	sOutputDebugStringCS = (CRITICAL_SECTION*)0x1;
}

void oThreadsafeOutputDebugStringA(const char* _OutputString)
{
	if (!sOutputDebugStringCS)
	{
		if (oProcessHeap::FindOrAllocate("OOOii.OutputDebugStringA.CriticalSection", sizeof(CRITICAL_SECTION), InitializeCS, (void**)&sOutputDebugStringCS))
			atexit(oThreadsafeOutputDebugStringA_AtExit);
	}

	// If we're very late into atexit() time, then threadsafety isn't a worry anymore
	if (sOutputDebugStringCS == (CRITICAL_SECTION*)0x1)
	{
		OutputDebugStringA(_OutputString);
	}

	else
	{
		EnterCriticalSection(sOutputDebugStringCS);
		OutputDebugStringA(_OutputString);
		LeaveCriticalSection(sOutputDebugStringCS);
	}
}

bool oWaitSingle(HANDLE _Handle, unsigned int _TimeoutMS)
{
	return WAIT_OBJECT_0 == ::WaitForSingleObject(_Handle, _TimeoutMS == oINVALID ? INFINITE : _TimeoutMS);
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
		oSetLastError(ETIMEDOUT);
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
		oSetLastError(EINVAL);
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
		else if (oVistaAPIsContext::Singleton()->oGetThreadId)
			ID = oVistaAPIsContext::Singleton()->oGetThreadId((HANDLE)_hThread);
		else if (oGetWindowsVersion() < oWINDOWS_VISTA)
			oTRACE("WARNING: oGetThreadID doesn't work with non-zero thread handles on versions of Windows prior to Vista.");

		return ID;
	#else
		oTRACE("oGetThreadID doesn't behave properly on versions of Windows prior to Vista because GetThreadId(HANDLE _hThread) didn't exist.");
		return 0;
	#endif
}

struct oGetWindowFromThreadID_Context 
{
	DWORD ThreadID;
	HWND hWnd;
};

BOOL CALLBACK oGetWindowFromThreadID_ENUM(HWND _hWnd, LPARAM _lParam)
{
	oGetWindowFromThreadID_Context& ctx = *(oGetWindowFromThreadID_Context*)_lParam;
	DWORD tid = ::GetWindowThreadProcessId(_hWnd, nullptr);
	if (ctx.ThreadID == tid)
		ctx.hWnd = _hWnd;
	return !ctx.hWnd;
}

HWND oGetWindowFromThreadID(DWORD _ThreadID)
{
	oGetWindowFromThreadID_Context ctx;
	ctx.ThreadID = _ThreadID;
	ctx.hWnd = 0;
	::EnumWindows(oGetWindowFromThreadID_ENUM, (LPARAM)&ctx);
	return ctx.hWnd;
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
		oSetLastError(STRUNCATE, "There are more than %u processes on the system currently, oWinGetProcessID needs to be more general-purpose.", oCOUNTOF(ProcessIDs));
		truncationResult = false;
	}

	else
		oSetLastError(ESRCH);

	return false;
}

bool oEnumProcessThreads(DWORD _ProcessID, oFUNCTION<bool(DWORD _ThreadID, DWORD _ParentProcessID)> _Function)
{
	HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, GetCurrentProcessId());
	if (hSnapshot == INVALID_HANDLE_VALUE)
	{
		oSetLastError(EINVAL, "Failed to get a snapshot of current process");
		return false;
	}

	THREADENTRY32 entry;
	entry.dwSize = sizeof(THREADENTRY32);

	BOOL keepLooking = Thread32First(hSnapshot, &entry);

	if (!keepLooking)
	{
		oSetLastError(ENOENT, "No process threads found");
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
		oSetLastError(EINVAL);
		return false;
	}

	HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (hSnapshot == INVALID_HANDLE_VALUE)
	{
		oSetLastError(EINVAL, "Failed to get a snapshot of current process");
		return false;
	}

	DWORD ppid = 0;

	PROCESSENTRY32 entry;
	entry.dwSize = sizeof(entry);
	BOOL keepLooking = Process32First(hSnapshot, &entry);

	if (!keepLooking)
	{
		oSetLastError(ECHILD);
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

int oGetWindowsErrorDescription(char* _StrDestination, size_t _SizeofStrDestination, HRESULT _hResult)
{
	int len = 0;
	*_StrDestination = 0;
	FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM, 0, static_cast<DWORD>(_hResult), 0, _StrDestination, static_cast<DWORD>(_SizeofStrDestination), 0);
	if (!*_StrDestination || !memcmp(_StrDestination, "???", 3))
	{
		#if defined(_DX9) || defined(_DX10) || defined(_DX11)
			strcpy_s(_StrDestination, _SizeofStrDestination, DXGetErrorStringA(_hResult));
		#else
			len = sprintf_s(_StrDestination, _SizeofStrDestination, "unrecognized error code 0x%08x", _hResult);
		#endif
	}

	return len != -1 ? 0 : STRUNCATE;
}

bool oIsWindows64Bit()
{
	if (sizeof(void*) != 4) // If ptr size is larger than 32-bit we must be on 64-bit windows
		return true;

	// If ptr size is 4 bytes then we're a 32-bit process so check if we're running under
	// wow64 which would indicate that we're on a 64-bit system
	BOOL bWow64 = FALSE;
	IsWow64Process(GetCurrentProcess(), &bWow64);
	return !!bWow64;
}

bool oIsAeroEnabled()
{
	BOOL enabled = FALSE;
	oV(oWinDWMAPI::Singleton()->DwmIsCompositionEnabled(&enabled));
	return !!enabled;
}

void oWinsockCreateAddr(sockaddr_in* _pOutSockAddr, const char* _Hostname)
{
	oWinsock* ws = oWinsock::Singleton();

	char host[1024];
	strcpy_s(host, _Hostname);
	unsigned short port = 0;
	char* strPort = strstr(host, ":");
	if (strPort)
	{
		port = static_cast<unsigned short>(atoi(strPort+1));
		*strPort = 0;
	}

	memset(_pOutSockAddr, 0, sizeof(sockaddr_in));
	hostent* pHost = ws->gethostbyname(host);
	_pOutSockAddr->sin_family = AF_INET;
	_pOutSockAddr->sin_port = ws->htons(port);
	_pOutSockAddr->sin_addr.s_addr = ws->inet_addr(ws->inet_ntoa(*(in_addr*)*pHost->h_addr_list));
}

void oWinsockAddrToHostname(sockaddr_in* _pSockAddr, char* _OutHostname, size_t _SizeOfHostname)
{
	oWinsock* ws = oWinsock::Singleton();

	unsigned long addr = ws->ntohl(_pSockAddr->sin_addr.s_addr);
	unsigned short port = ws->ntohs(_pSockAddr->sin_port);

	sprintf_s(_OutHostname, _SizeOfHostname, "%u.%u.%u.%u:%u", (addr&0xFF000000)>>24, (addr&0xFF0000)>>16, (addr&0xFF00)>>8, addr&0xFF, port);
}

//#define FD_TRACE(TracePrefix, TraceName, FDEvent) oTRACE("%s%s%s: %s (%s)", oSAFESTR(TracePrefix), _TracePrefix ? " " : "", oSAFESTR(TraceName), #FDEvent, oWinsock::GetErrorString(_pNetworkEvents->iErrorCode[FDEvent##_BIT]))
#define FD_TRACE(TracePrefix, TraceName, FDEvent)

// Assumes WSANETWORKEVENTS ne; int err;
#define FD_CHECK(FDEvent) \
	if (_pNetworkEvents->lNetworkEvents & FDEvent) \
	{	FD_TRACE(_TracePrefix, _TraceName, ##FDEvent); \
		err = _pNetworkEvents->iErrorCode[FDEvent##_BIT]; \
	}

// Checks all values from a network event and return an error based on what happened.
int oWinsockTraceEvents(const char* _TracePrefix, const char* _TraceName, const WSANETWORKEVENTS* _pNetworkEvents)
{
	int err = 0;

	if (!_pNetworkEvents->lNetworkEvents)
	{
		// http://www.mombu.com/microsoft/alt-winsock-programming/t-wsaenumnetworkevents-returns-no-event-how-is-this-possible-1965867.html
		// Also Google "spurious wakeup" 
		oASSERT(false, "%s%s%s: WSAEVENT, but no lNetworkEvent: \"spurious wakeup\". You should ignore the event as if it never happened by testing for _pNetworkEvents->lNetworkEvents == 0 in calling code.", oSAFESTR(_TracePrefix), _TracePrefix ? " " : "", oSAFESTR(_TraceName));
		err = EINVAL;
	}
	
	FD_CHECK(FD_READ); FD_CHECK(FD_WRITE); FD_CHECK(FD_OOB); FD_CHECK(FD_CONNECT); 
	FD_CHECK(FD_ACCEPT); FD_CHECK(FD_CLOSE); FD_CHECK(FD_QOS); FD_CHECK(FD_GROUP_QOS); 
	FD_CHECK(FD_ROUTING_INTERFACE_CHANGE); FD_CHECK(FD_ADDRESS_LIST_CHANGE);

	return err;
}

SOCKET oWinsockCreate(const char* _Hostname, int _ORedWinsockOptions, unsigned int _MaxNumConnections, size_t _SendBufferSize, size_t _ReceiveBufferSize)
{
	oWinsock* ws = oWinsock::Singleton();
	const bool kReliable = !!(_ORedWinsockOptions & oWINSOCK_RELIABLE);

	SOCKET hSocket = ws->WSASocket(AF_INET, kReliable ? SOCK_STREAM : SOCK_DGRAM, kReliable ? IPPROTO_TCP : IPPROTO_UDP, 0, 0, WSA_FLAG_OVERLAPPED);
	if (hSocket == INVALID_SOCKET) goto error;

	u_long enabled = !( _ORedWinsockOptions & oWINSOCK_BLOCKING );
	if (SOCKET_ERROR == ws->ioctlsocket(hSocket, FIONBIO, &enabled)) goto error;
	if ((_ORedWinsockOptions & oWINSOCK_REUSE_ADDRESS) && SOCKET_ERROR == ws->setsockopt(hSocket, SOL_SOCKET, SO_REUSEADDR, (const char*)&enabled, sizeof(enabled))) goto error;
	if (_SendBufferSize && SOCKET_ERROR == ws->setsockopt(hSocket, SOL_SOCKET, SO_SNDBUF, (const char*)&_SendBufferSize, sizeof(_SendBufferSize))) goto error;
	if (_ReceiveBufferSize && SOCKET_ERROR == ws->setsockopt(hSocket, SOL_SOCKET, SO_RCVBUF, (const char*)&_ReceiveBufferSize, sizeof(_ReceiveBufferSize))) goto error;
	if (!kReliable && (_ORedWinsockOptions & oWINSOCK_ALLOW_BROADCAST) && SOCKET_ERROR == ws->setsockopt(hSocket, SOL_SOCKET, SO_BROADCAST, (const char*)&enabled, sizeof(enabled))) goto error;
	
	char host[1024];
	strcpy_s(host, _Hostname);

	unsigned short port = 0;
	char* strPort = strstr(host, ":");
	if (strPort)
	{
		port = static_cast<unsigned short>(atoi(strPort+1));
		*strPort = 0;
	}

	sockaddr_in saddr;
	oWinsockCreateAddr(&saddr, _Hostname);

	if (_MaxNumConnections)
	{
		saddr.sin_addr.s_addr = INADDR_ANY;
		if (SOCKET_ERROR == ws->bind(hSocket, (const sockaddr*)&saddr, sizeof(saddr))) goto error;
		if (kReliable && SOCKET_ERROR == ws->listen(hSocket, _MaxNumConnections)) goto error;
	}

	else
	{
		if (kReliable)
		{
			if (SOCKET_ERROR == ws->connect(hSocket, (const sockaddr*)&saddr, sizeof(saddr)) && ws->WSAGetLastError() != WSAEWOULDBLOCK) goto error;
		}

		else
		{
			if (SOCKET_ERROR == ws->bind(hSocket, (const sockaddr*)&saddr, sizeof(saddr))) goto error;
		}
	}

	return hSocket;
error:
	oWINSOCK_SETLASTERROR("oWinsockCreate");
	if (hSocket != INVALID_SOCKET)
		ws->closesocket(hSocket);
	return INVALID_SOCKET;
}

bool oWinsockClose(SOCKET _hSocket)
{
	// http://msdn.microsoft.com/en-us/library/ms738547(v=vs.85).aspx
	// Specifically read the community comment that says the main article doesn't 
	// work...

	// @oooii-tony: Ignore WSAENOTCONN since we're closing this socket anyway. It 
	// means the other side is detached already.

	if (_hSocket)
	{
		oWinsock* ws = oWinsock::Singleton();

		if (SOCKET_ERROR == ws->shutdown(_hSocket, SD_BOTH) && ws->WSAGetLastError() != WSAENOTCONN)
		{
			oWINSOCK_SETLASTERROR("shutdown");
			return false;
		}

		LPFN_DISCONNECTEX FNDisconnectEx = 0;
		if (!ws->GetFunctionPointer_DisconnectEx(_hSocket, &FNDisconnectEx) && ws->WSAGetLastError() != WSAENOTCONN)
		{
			oWINSOCK_SETLASTERROR("GetFunctionPointer_DisconnectEx");
			return false;
		}

		if (!FNDisconnectEx(_hSocket, 0, 0, 0) && ws->WSAGetLastError() != WSAENOTCONN)
		{
			oWINSOCK_SETLASTERROR("DisconnectEx");
			return false;
		}

		if (SOCKET_ERROR == ws->closesocket(_hSocket) && ws->WSAGetLastError() != WSAENOTCONN)
		{
			oWINSOCK_SETLASTERROR("closesocket");
			return false;
		}
	}

	return true;
}

bool oWinsockWaitMultiple(WSAEVENT* _pHandles, size_t _NumberOfHandles, bool _WaitAll, bool _Alertable, unsigned int _TimeoutMS)
{
	// @oooii-tony: there is something called "spurious wakeups" (Google it for 
	// more info) that can signal an event though no user-space event has been 
	// triggered. My reading was that it only applied to the mask in 
	// WSANETWORKEVENTS, so if you're not using that then this doesn't matter, but
	// it's the event that gets triggered incorrectly, not the mask that gets 
	// filled out incorrectly from my readings, so it *could* happen any time the
	// event is waited on. If something funky occurs in this wait, start debugging 
	// with more "spurious wakeup" investigation.

	return WSA_WAIT_TIMEOUT != oWinsock::Singleton()->WSAWaitForMultipleEvents(static_cast<DWORD>(_NumberOfHandles), _pHandles, _WaitAll, _TimeoutMS == oINFINITE_WAIT ? WSA_INFINITE : _TimeoutMS, _Alertable);
}

// If the socket was created using oWinSockCreate (WSAEventSelect()), this function can 
// be used to wait on that event and receive any events breaking the wait.
bool oWinsockWait(SOCKET _hSocket, WSAEVENT _hEvent, WSANETWORKEVENTS* _pNetEvents, unsigned int _TimeoutMS)
{
	oWinsock* ws = oWinsock::Singleton();
	bool eventFired = true;
	unsigned int timeout = _TimeoutMS;
	_pNetEvents->lNetworkEvents = 0;
	oScopedPartialTimeout spt(&timeout);
	while (!_pNetEvents->lNetworkEvents && eventFired)
	{
		{
			spt.UpdateTimeout();
			eventFired = oWinsockWaitMultiple(&_hEvent, 1, true, false, timeout);
			if (eventFired)
			{
				if (SOCKET_ERROR == ws->WSAEnumNetworkEvents(_hSocket, _hEvent, _pNetEvents))
				{
					oSetLastError(ws->GetErrno(ws->WSAGetLastError()), 0);
					eventFired = false;
					break;
				}

				if (_pNetEvents->lNetworkEvents)
					break;
			}
		}
	}

	return eventFired;
}

bool oWinsockSend(SOCKET _hSocket, const void* _pSource, size_t _SizeofSource, const sockaddr_in* _pDestination)
{
	oASSERT(_SizeofSource < INT_MAX, "Underlying implementation uses 32-bit signed int for buffer size.");
	oWinsock* ws = oWinsock::Singleton();
	int bytesSent = 0;
	
	if (_pDestination)
		bytesSent = ws->sendto(_hSocket, (const char*)_pSource, static_cast<int>(_SizeofSource), 0, (const sockaddr*)_pDestination, sizeof(sockaddr_in));
	else
		bytesSent = ws->send(_hSocket, (const char*)_pSource, static_cast<int>(_SizeofSource), 0);
	if (bytesSent == SOCKET_ERROR)
		oSetLastError(oWinsock::GetErrno(ws->WSAGetLastError()));
	else if ((size_t)bytesSent == _SizeofSource)
		oSetLastError(0);
	return (size_t)bytesSent == _SizeofSource;
}

size_t oWinsockReceive(SOCKET _hSocket, WSAEVENT _hEvent, void* _pDestination, size_t _SizeofDestination, unsigned int _TimeoutMS, int* _pInOutCanReceive, sockaddr_in* _pSource)
{
	oASSERT(_pInOutCanReceive, "_pInOutCanReceive must be specified.");
	oASSERT(_SizeofDestination < INT_MAX, "Underlying implementation uses 32-bit signed int for buffer size.");

	if (!_pDestination)
	{
		oSetLastError(EINVAL, "Must specify a destination buffer");
		return 0;
	}

	oWinsock* ws = oWinsock::Singleton();

	int err = 0;
	WSANETWORKEVENTS ne;
	memset(&ne, 0, sizeof(ne));
	if (*_pInOutCanReceive)
	{
		err = WSAETIMEDOUT;
		bool eventFired = oWinsockWait(_hSocket, _hEvent, &ne, _TimeoutMS);
		if (eventFired)
				err = oWinsockTraceEvents("oWinSockReceive", 0, &ne);
	}

	int bytesReceived = 0;
	if (!err && (ne.lNetworkEvents & FD_READ))
	{
		if (_pSource)
		{
			int size = sizeof(sockaddr_in);
			bytesReceived = ws->recvfrom(_hSocket, (char*)_pDestination, static_cast<int>(_SizeofDestination), 0, (sockaddr*)_pSource, &size);
		}

		else
			bytesReceived = ws->recv(_hSocket, (char*)_pDestination, static_cast<int>(_SizeofDestination), 0);

		if (bytesReceived == SOCKET_ERROR)
		{
			err = oWinsock::GetErrno(ws->WSAGetLastError());
			bytesReceived = 0;
		}

		else if (!bytesReceived)
		{
			oSWAP(_pInOutCanReceive, false);
			err = ESHUTDOWN;
		}

		else
			err = 0;
	}

	else if ((ne.lNetworkEvents & FD_CLOSE) || ((ne.lNetworkEvents & FD_CONNECT) && err))
	{
		oSetLastError(oWinsock::GetErrno(ne.iErrorCode[FD_CLOSE_BIT]));
		oSWAP(_pInOutCanReceive, false);
	}

	oSetLastError(err);
	return bytesReceived;
}

bool oWinsockReceiveNonBlocking(SOCKET _hSocket, WSAEVENT _hEvent, void* _pDestination, size_t _SizeofDestination, sockaddr_in* _pSource, size_t* _pBytesReceived)
{
	oASSERT(_SizeofDestination < INT_MAX, "Underlying implementation uses 32-bit signed int for buffer size.");

	if (!_pDestination)
	{
		oSetLastError(EINVAL, "Must specify a destination buffer");
		return false;
	}

	oWinsock* ws = oWinsock::Singleton();

	timeval waitTime;
	waitTime.tv_sec = 0;
	waitTime.tv_usec = 0;

	fd_set set;
	FD_ZERO(&set);
	FD_SET(_hSocket, &set);

	if(!ws->select(1, &set, NULL, NULL, &waitTime))
	{
		_pBytesReceived = 0;
		return true;
	}

	int err = 0;
	int bytesReceived = 0;
	if (_pSource)
	{
		int size = sizeof(sockaddr_in);
		bytesReceived = ws->recvfrom(_hSocket, (char*)_pDestination, static_cast<int>(_SizeofDestination), 0, (sockaddr*)_pSource, &size);
	}

	else
		bytesReceived = ws->recv(_hSocket, (char*)_pDestination, static_cast<int>(_SizeofDestination), 0);

	if (bytesReceived == SOCKET_ERROR)
	{
		err = oWinsock::GetErrno(ws->WSAGetLastError());
		bytesReceived = 0;
	}

	else if (!bytesReceived)
	{
		err = ESHUTDOWN;
	}

	//else if ((ne.lNetworkEvents & FD_CLOSE) || ((ne.lNetworkEvents & FD_CONNECT) && err))
	//{
	//	oSetLastError(oWinsock::GetErrno(ne.iErrorCode[FD_CLOSE_BIT]));
	//	oSWAP(_pInOutCanReceive, false);
	//}

	*_pBytesReceived = bytesReceived;

	oSetLastError(err);
	return(0 == err);
}

bool oWinsockGetNameBase(char* _OutHostname, size_t _SizeofOutHostname, char* _OutIPAddress, size_t _SizeofOutIPAddress, char* _OutPort, size_t _SizeofOutPort, SOCKET _hSocket, oFUNCTION<int(SOCKET _hSocket, sockaddr* _Name, int* _pNameLen)> _GetSockAddr)
{
	oWinsock* ws = oWinsock::Singleton();

	sockaddr_in saddr;
	int size = sizeof(saddr);
	if (SOCKET_ERROR == _GetSockAddr(_hSocket, (sockaddr*)&saddr, &size))
	{
		errno_t err = oWinsock::GetErrno(ws->WSAGetLastError());
		oSetLastError(err, "%s: %s", oWinsock::AsString(ws->WSAGetLastError()), oWinsock::GetErrorDesc(ws->WSAGetLastError()));
		return false;
	}

	// Allow for the user to specify null for the parts they don't want.
	char localHostname[_MAX_PATH];
	char localService[16];

	char* pHostname = _OutHostname ? _OutHostname : localHostname;
	size_t sizeofHostname = _OutHostname ? _SizeofOutHostname : oCOUNTOF(localHostname);
	
	char* pService = _OutPort ? _OutPort : localService;
	size_t sizeofService = _OutPort ? _SizeofOutPort : oCOUNTOF(localService);

	ws->getnameinfo((sockaddr*)&saddr, size, pHostname, static_cast<DWORD>(sizeofHostname), pService, static_cast<DWORD>(sizeofService), 0);

	if (_OutIPAddress)
	{
		const char* ip = ws->inet_ntoa(saddr.sin_addr);
		errno_t err = strcpy_s(_OutIPAddress, _SizeofOutIPAddress, ip);
		if (err)
		{
			oSetLastError(err);
			return false;
		}
	}

	return true;
}

bool oWinsockGetHostname(char* _OutHostname, size_t _SizeofOutHostname, char* _OutIPAddress, size_t _SizeofOutIPAddress, char* _OutPort, size_t _SizeofOutPort, SOCKET _hSocket)
{
	return oWinsockGetNameBase(_OutHostname, _SizeofOutHostname, _OutIPAddress, _SizeofOutIPAddress, _OutPort, _SizeofOutPort, _hSocket, oWinsock::Singleton()->getsockname);
}

bool oWinsockGetPeername(char* _OutHostname, size_t _SizeofOutHostname, char* _OutIPAddress, size_t _SizeofOutIPAddress, char* _OutPort, size_t _SizeofOutPort, SOCKET _hSocket)
{
	return oWinsockGetNameBase(_OutHostname, _SizeofOutHostname, _OutIPAddress, _SizeofOutIPAddress, _OutPort, _SizeofOutPort, _hSocket, oWinsock::Singleton()->getpeername);
}

bool oWinsockIsConnected(SOCKET _hSocket)
{
	// According to Google's think-tank, getting a peer name will touch the 
	// connection, so let's do that. (Another option would be to get the # secs
	// of connection time through SO_CONNECT_TIME, but I worry about checking 
	// immediately after connect before there get's a up for 1 sec count).

	char tmp[_MAX_PATH];
	return oWinsockGetPeername(tmp, oCOUNTOF(tmp), 0, 0, 0, 0, _hSocket);
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
		default: oASSUME(0);
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
	_lpDlgTemplate->cx = short(_Desc.Rect.right - _Desc.Rect.left);
	_lpDlgTemplate->cy = short(_Desc.Rect.bottom - _Desc.Rect.top);

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
		case oDLG_LARGELABEL: style |= ES_LEFT|ES_MULTILINE|ES_READONLY|WS_VSCROLL; break;
		default: break;
	}
	
	_lpDlgItemTemplate->style = style;
	_lpDlgItemTemplate->dwExtendedStyle = WS_EX_NOPARENTNOTIFY;
	_lpDlgItemTemplate->x = short(_Desc.Rect.left);
	_lpDlgItemTemplate->y = short(_Desc.Rect.top);
	_lpDlgItemTemplate->cx = short(_Desc.Rect.right - _Desc.Rect.left);
	_lpDlgItemTemplate->cy = short(_Desc.Rect.bottom - _Desc.Rect.top);
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
