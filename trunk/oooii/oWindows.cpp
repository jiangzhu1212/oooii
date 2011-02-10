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
#include "pch.h"
#include <oooii/oWindows.h>
#include <oooii/oAssert.h>
#include <oooii/oAtomic.h>
#include <oooii/oColor.h>
#include <oooii/oDisplay.h>
#include <oooii/oErrno.h>
#include <oooii/oStdio.h>
#include <oooii/oString.h>
#include <oooii/oRef.h>
#include "oWinsock.h"
#include <tlhelp32.h>

// Use the Windows Vista UI look. If this causes issues or the dialog not to appear, try other values from processorAchitecture { x86 ia64 amd64 * }
#pragma comment(linker, "\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

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
	_pFileTime->dwHighDateTime = ll >>32;
}

time_t oFileTimeToUnixTime(const FILETIME* _pFileTime)
{
	if (!_pFileTime) return 0;
	// this ought to be the reverse of to_filetime
	LONGLONG ll = ((LONGLONG)_pFileTime->dwHighDateTime << 32) | _pFileTime->dwLowDateTime;
	return static_cast<time_t>((ll - 116444736000000000) / 10000000);
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

	#ifdef UNICODE
		WCHAR T[1024];
		oStrConvert(T, title);
	#else
		const char* T = _Title;
	#endif

	return TRUE == SendMessage(_hWnd, WM_SETTEXT, 0, (LPARAM)T);
}

bool oGetTitle(HWND _hWnd, char* _Title, size_t _SizeofTitle)
{
	if (!_hWnd || !_Title) return false;
	
	#ifdef UNICODE
		TCHAR* T = _alloca(_SizeofTitle * sizeof(TCHAR));
	#else
		char* T = _Title;
	#endif
	
	LRESULT success = SendMessage(_hWnd, WM_SETTEXT, _SizeofTitle, (LPARAM)T);

	#ifdef UNICODE
		oStrConvert(_Title, _SizeofTitle, T);
	#endif

	return success == TRUE;
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

		// @oooii-tony: BitBlt only works for the visible portion of the screen,
		// whereas PrintWindow works with even hidden windows, but doesn't allow
		// any partial capture.
		#if 0
			PrintWindow(_hWnd, hMemDC, _pRect ? 0 : PW_CLIENTONLY);
		#else
			SetForegroundWindow(_hWnd);
			BitBlt(hMemDC, 0, 0, w, h, hDC, r.left, r.top, SRCCOPY);
		#endif

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

struct oWindowsHookContext : public oSingleton<oWindowsHookContext>
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
	while (PeekMessage(&msg, _hWnd, 0, 0, PM_REMOVE) && (_TimeoutMS == ~0u || (oTimerMS() - start) < _TimeoutMS))
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

	#ifdef UNICODE
		WCHAR CName[128];
		oStrConvert(classNameW, className);
	#else
		const char* CName = className;
	#endif

	WNDCLASSEX wc;
	ZeroMemory(&wc, sizeof(WNDCLASSEX));
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.hInstance = GetModuleHandle(0);
	wc.lpfnWndProc = _Wndproc;                    
	wc.lpszClassName = CName;                        
	wc.style = CS_BYTEALIGNCLIENT|CS_HREDRAW|CS_VREDRAW|CS_OWNDC|(_SupportDoubleClicks ? CS_DBLCLKS : 0);
	wc.hCursor = LoadCursor(0, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)(COLOR_BACKGROUND+1);
	RegisterClassEx(&wc);
	*_pHwnd = CreateWindowEx(WS_EX_ACCEPTFILES|WS_EX_APPWINDOW, CName, _Title ? _Title : "", WS_OVERLAPPEDWINDOW, 0, 0, 1, 1, 0, 0, 0, _pInstance);
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
		CREATESTRUCT* cs = (CREATESTRUCT*)_lParam;
		SetWindowLongPtr(_hWnd, GWLP_USERDATA, (LONG_PTR)cs->lpCreateParams);
		#ifdef _DEBUG
			#ifdef UNICODE
				char className[32];
				oStrConvert(className, cs->lpszClass);
				char title[128];
				oStrConvert(title, cs->lpszName);
			#else
				const char* className = cs->lpszClass;
				const char* title = cs->lpszName;
			#endif
			if (_StrDescription)
				sprintf_s(_StrDescription, _SizeOfStrDescription, "%s \"%s\" %d,%d %dx%d", className, title, cs->x, cs->y, cs->cx, cs->cy);
		#endif
	}

	else
	{
		// For any other message, grab the context and return it
		context = _hWnd ? (void*)GetWindowLongPtr(_hWnd, GWLP_USERDATA) : 0;
	}

	return context;
}

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

	UINT oDX11GetNumElements(D3D11_PRIMITIVE_TOPOLOGY _PrimitiveTopology, UINT _NumPrimitives)
	{
		switch (_PrimitiveTopology)
		{
			case D3D11_PRIMITIVE_TOPOLOGY_POINTLIST: return _NumPrimitives;
			case D3D11_PRIMITIVE_TOPOLOGY_LINELIST: return _NumPrimitives * 2;
			case D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP: return _NumPrimitives + 1;
			case D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST: return _NumPrimitives * 3;
			case D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP: return _NumPrimitives + 2;
			default: oASSUME(0);
		}
	}

	UINT oDX11Draw(ID3D11DeviceContext* _pDeviceContext
		, D3D11_PRIMITIVE_TOPOLOGY _PrimitiveTopology
		, UINT _NumPrimitives
		, UINT _NumVertexBuffers
		, const ID3D11Buffer* const* _ppVertexBuffers
		, const UINT* _VertexStrides
		, UINT _IndexOfFirstVertexToDraw
		, UINT _OffsetToAddToEachVertexIndex
		, const ID3D11Buffer* _IndexBuffer
		, bool _32BitIndexBuffer
		, UINT _IndexOfFirstIndexToDraw
		, UINT _NumInstances
		, UINT _IndexOfFirstInstanceIndexToDraw)
	{
		const UINT nElements = oDX11GetNumElements(_PrimitiveTopology, _NumPrimitives);
		static UINT sOffsets[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

		// DirectX as an API has a funny definition for const, probably because of 
		// ref-counting, so consider it a platform-quirk and keep const correctness
		// above this API, but cast it away as DirectX requires here...

		_pDeviceContext->IASetVertexBuffers(0, _NumVertexBuffers, const_cast<ID3D11Buffer* const*>(_ppVertexBuffers), _VertexStrides, sOffsets);
		_pDeviceContext->IASetPrimitiveTopology(_PrimitiveTopology);

		if (_IndexBuffer)
		{
			_pDeviceContext->IASetIndexBuffer(const_cast<ID3D11Buffer*>(_IndexBuffer), _32BitIndexBuffer ? DXGI_FORMAT_R32_UINT : DXGI_FORMAT_R16_UINT, _IndexOfFirstIndexToDraw);

			if (_NumInstances)
				_pDeviceContext->DrawIndexedInstanced(nElements, _NumInstances, _IndexOfFirstIndexToDraw, _OffsetToAddToEachVertexIndex, _IndexOfFirstInstanceIndexToDraw);
			else
				_pDeviceContext->DrawIndexed(nElements, _IndexOfFirstIndexToDraw, _OffsetToAddToEachVertexIndex);
		}

		else
		{
			if (_NumInstances)
				_pDeviceContext->DrawInstanced(nElements, _NumInstances, _IndexOfFirstVertexToDraw, _IndexOfFirstInstanceIndexToDraw);
			else
				_pDeviceContext->Draw(nElements, _IndexOfFirstVertexToDraw);
		}

		return _NumPrimitives * __min(1, _NumInstances);
	}

	template<> const char* oAsString(const oD3D11_PIPELINE_STAGE& _Stage)
	{
		switch (_Stage)
		{
			case oD3D11_PIPELINE_STAGE_VERTEX: return "oD3D11_PIPELINE_STAGE_VERTEX";
			case oD3D11_PIPELINE_STAGE_HULL: return "oD3D11_PIPELINE_STAGE_HULL";
			case oD3D11_PIPELINE_STAGE_DOMAIN: return "oD3D11_PIPELINE_STAGE_DOMAIN";
			case oD3D11_PIPELINE_STAGE_GEOMETRY: return "oD3D11_PIPELINE_STAGE_GEOMETRY";
			case oD3D11_PIPELINE_STAGE_PIXEL: return "oD3D11_PIPELINE_STAGE_PIXEL";
			default: oASSUME(0);
		}
	}

	const char* oDX11GetShaderProfile(ID3D11Device* _pDevice, oD3D11_PIPELINE_STAGE _Stage)
	{
		static const char* sDX9Profiles[] = 
		{
			"vs_3_0",
			0,
			0,
			0,
			"ps_3_0",
			0,
		};

		static const char* sDX10Profiles[] = 
		{
			"vs_4_0",
			0,
			0,
			"gs_4_0",
			"ps_4_0",
			0,
		};

		static const char* sDX10_1Profiles[] = 
		{
			"vs_4_1",
			0,
			0,
			"gs_4_1",
			"ps_4_1",
			0,
		};

		static const char* sDX11Profiles[] = 
		{
			"vs_5_0",
			"hs_5_0",
			"ds_5_0",
			"gs_5_0",
			"ps_5_0",
			"cs_5_0",
		};

		const char** profiles = 0;
		switch (_pDevice->GetFeatureLevel())
		{
		case D3D_FEATURE_LEVEL_9_1:
		case D3D_FEATURE_LEVEL_9_2:
		case D3D_FEATURE_LEVEL_9_3:
			profiles = sDX9Profiles;
			break;
		case D3D_FEATURE_LEVEL_10_0:
			profiles = sDX10Profiles;
			break;
		case D3D_FEATURE_LEVEL_10_1:
			profiles = sDX10_1Profiles;
			break;
		case D3D_FEATURE_LEVEL_11_0:
			profiles = sDX11Profiles;
			break;
		default:
			oASSUME(0);
		}

		const char* profile = profiles[_Stage];
		if (!profile)
			oSetLastError(ENOENT, "Shader profile does not exist for D3D%.2f's stage %s", oGetD3DVersion(_pDevice->GetFeatureLevel()), oAsString(_Stage));

		return profile;
	}

	bool oDX11ConvertCompileErrorBuffer(char* _OutErrorMessageString, size_t _SizeofOutErrorMessageString, ID3DBlob* _pErrorMessages)
	{
		if (!_OutErrorMessageString)
		{
			oSetLastError(EINVAL);
			return false;
		}

		if (_pErrorMessages)
		{
			errno_t err = oReplace(_OutErrorMessageString, _SizeofOutErrorMessageString, (const char*)_pErrorMessages->GetBufferPointer(), "%", "%%");
			if (err)
			{
				oSetLastError(err);
				return false;
			}
		}

		else
			*_OutErrorMessageString = 0;

		return true;
	}

#endif

#if oDXVER >= oDXVER_10

	typedef HRESULT (__stdcall *CreateDXGIFactoryFn)(REFIID riid, void **ppFactory);
	typedef HRESULT (__stdcall *DWriteCreateFactoryFn)(DWRITE_FACTORY_TYPE factoryType, REFIID iid, IUnknown **factory);
	typedef HRESULT (__stdcall *D2D1CreateFactoryFn)(D2D1_FACTORY_TYPE factoryType, REFIID riid, const D2D1_FACTORY_OPTIONS *pFactoryOptions, void **ppIFactory);
	typedef DWORD (__stdcall *GetThreadIdFn)(HANDLE Thread);

	struct oVistaAPIsContext : oSingleton<oVistaAPIsContext>
	{
		oVistaAPIsContext()
			: hDXGIModule(0)
			, hDWriteModule(0)
			, hD2DModule(0)
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

		oRef<IDXGIFactory> DXGIFactory;
		oRef<IDWriteFactory> DWriteFactory;
		oRef<ID2D1Factory> D2DFactory;
		GetThreadIdFn oGetThreadId;

	protected:

		bool LoadModules()
		{
			hDXGIModule = LoadLibraryA("dxgi");
			hDWriteModule = LoadLibraryA("dwrite");
			hD2DModule = LoadLibraryA("d2d1");
			return hDXGIModule && hDWriteModule && hD2DModule;
		}

		void FreeModules()
		{
			oCreateDXGIFactory = 0;
			oDWriteCreateFactory = 0;
			oD2D1CreateFactory = 0;

			if (hDXGIModule)
			{
				FreeLibrary(hDXGIModule);
				hDXGIModule = 0;
			}

			if (hDWriteModule)
			{
				FreeLibrary(hD2DModule);
				hDWriteModule = 0;
			}

			if (hD2DModule)
			{
				FreeLibrary(hD2DModule);
				hD2DModule = 0;
			}
		}

		bool GetProcs()
		{
			oCreateDXGIFactory = reinterpret_cast<CreateDXGIFactoryFn>(GetProcAddress(hDXGIModule, "CreateDXGIFactory"));
			oDWriteCreateFactory = reinterpret_cast<DWriteCreateFactoryFn>(GetProcAddress(hDWriteModule, "DWriteCreateFactory"));
			oD2D1CreateFactory = reinterpret_cast<D2D1CreateFactoryFn>(GetProcAddress(hD2DModule, "D2D1CreateFactory"));
			oGetThreadId = reinterpret_cast<GetThreadIdFn>(GetProcAddress(GetModuleHandleA("kernel32"), "GetThreadId"));
			return oCreateDXGIFactory && oDWriteCreateFactory && oD2D1CreateFactory;
		}

		bool LoadSingletons()
		{
			if (S_OK != oCreateDXGIFactory(__uuidof(IDXGIFactory), (void**)&DXGIFactory))
				return false;
			if (S_OK != oDWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), (IUnknown**)&DWriteFactory))
				return false;
			if (S_OK != oD2D1CreateFactory(D2D1_FACTORY_TYPE_MULTI_THREADED, __uuidof(ID2D1Factory), 0, (void**)&D2DFactory))
				return false;
			return true;
		}

		void FreeSingletons()
		{
			oASSERT(oGetRefCount(DXGIFactory) == 1, "Outstanding refcount (%u) on DXGI interfaces", oGetRefCount(DXGIFactory));
			DXGIFactory = 0;
			// @oooii-tony: Because we're creating a shared DWrite interface, this 
			// isn't always the last ref on the factory
			//oASSERT(oGetRefCount(DWriteFactory) == 1, "Outstanding refcount (%u) on DWrite interfaces", oGetRefCount(DWriteFactory));
			DWriteFactory = 0;
			oASSERT(oGetRefCount(D2DFactory) == 1, "Outstanding refcount (%u) on D2D interfaces", oGetRefCount(D2DFactory));
			D2DFactory = 0;
		}

		HMODULE hDXGIModule;
		HMODULE hDWriteModule;
		HMODULE hD2DModule;

		CreateDXGIFactoryFn oCreateDXGIFactory;
		DWriteCreateFactoryFn oDWriteCreateFactory;
		D2D1CreateFactoryFn oD2D1CreateFactory;
	};

	IDXGIFactory* oGetDXGIFactorySingleton()
	{
		return oVistaAPIsContext::Singleton()->DXGIFactory;
	}

	IDWriteFactory* oGetDWriteFactorySingleton()
	{
		return oVistaAPIsContext::Singleton()->DWriteFactory;
	}

	ID2D1Factory* oGetD2DFactorySingleton()
	{
		return oVistaAPIsContext::Singleton()->D2DFactory;
	}

	float oGetD3DVersion(IDXGIAdapter* _pAdapter)
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

	bool oFindDXGIOutput(IDXGIFactory* _pFactory, HMONITOR _hMonitor, IDXGIOutput** _ppOutput)
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

#endif

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

	return ~0u;
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

bool oWaitSingle(DWORD _ThreadID, unsigned int _Timeout)
{
	HANDLE hThread = OpenThread(SYNCHRONIZE, FALSE, _ThreadID);
	bool result = oWaitSingle(hThread, _Timeout);
	oVB(CloseHandle(hThread));
	return result;
}

bool oWaitMultiple(DWORD* _pThreadIDs, size_t _NumberOfThreadIDs, bool _WaitAll, unsigned int _Timeout)
{
	if (!_pThreadIDs || !_NumberOfThreadIDs || _NumberOfThreadIDs >= 64)
	{
		oSetLastError(EINVAL);
		return false;
	}

	HANDLE hThreads[64]; // max of windows anyway
	for (size_t i = 0; i < _NumberOfThreadIDs; i++)
		hThreads[i] = OpenThread(SYNCHRONIZE, FALSE, _pThreadIDs[i]);

	bool result = oWaitMultiple(hThreads, _NumberOfThreadIDs, _WaitAll, _Timeout);
	
	for (size_t i = 0; i < _NumberOfThreadIDs; i++)
		oVB(CloseHandle(hThreads[i]));

	return result;
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

DWORD oGetParentProcessID()
{
	HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (hSnapshot == INVALID_HANDLE_VALUE)
		return 0;

	DWORD ppid = 0;

	PROCESSENTRY32 entry;
	entry.dwSize = sizeof(entry);
	DWORD pid = GetCurrentProcessId();
	BOOL keepLooking = Process32First(hSnapshot, &entry);
	while (keepLooking)
	{
		if (pid == entry.th32ProcessID)
			ppid = entry.th32ParentProcessID;
		entry.dwSize = sizeof(entry);
		keepLooking = !ppid && Process32Next(hSnapshot, &entry);
	}

	oVB(CloseHandle(hSnapshot));
	return ppid;
}

unsigned int oGetProcessThreads(DWORD* _pThreadIDs, size_t _SizeofThreadIDs)
{
	HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, GetCurrentProcessId());
	if (hSnapshot == INVALID_HANDLE_VALUE)
		return 0;

	unsigned int nThreads = 0;

	THREADENTRY32 entry;
	entry.dwSize = sizeof(THREADENTRY32);

	DWORD pid = GetCurrentProcessId();
	BOOL keepLooking = Thread32First(hSnapshot, &entry);
	bool overrun = false;
	while (keepLooking)
	{
		if (pid == entry.th32OwnerProcessID)
		{
			if (_pThreadIDs && nThreads < _SizeofThreadIDs)
			{
				if (pid == entry.th32OwnerProcessID)
					_pThreadIDs[nThreads] = entry.th32ThreadID;
			}
			else
				overrun = true;

			nThreads++;
		}

		keepLooking = Thread32Next(hSnapshot, &entry);
	}

	if (overrun)
		oSetLastError(EINVAL, "Buffer not large enough");

	oVB(CloseHandle(hSnapshot));
	return nThreads;
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
	if( sizeof( void* ) != 4 ) // If ptr size is larger than 32-bit we must be on 64-bit windows
		return true;

	// If ptr size is 4 bytes then we're a 32-bit process so check if we're running under
	// wow64 which would indicate that we're on a 64-bit system
	BOOL bWow64 = 0;
	IsWow64Process( GetCurrentProcess(), &bWow64 );
	return bWow64 != 0;
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

bool oWinsockWaitMultiple(WSAEVENT* _pHandles, size_t _NumberOfHandles, bool _WaitAll, bool _Alertable, unsigned int _Timeout)
{
	// @oooii-tony: there is something called "spurious wakeups" (Google it for 
	// more info) that can signal an event though no user-space event has been 
	// triggered. My reading was that it only applied to the mask in 
	// WSANETWORKEVENTS, so if you're not using that then this doesn't matter, but
	// it's the event that gets triggered incorrectly, not the mask that gets 
	// filled out incorrectly from my readings, so it *could* happen any time the
	// event is waited on. If something funky occurs in this wait, start debugging 
	// with more "spurious wakeup" investigation.

	return WSA_WAIT_TIMEOUT != oWinsock::Singleton()->WSAWaitForMultipleEvents(static_cast<DWORD>(_NumberOfHandles), _pHandles, _WaitAll, _Timeout == ~0u ? WSA_INFINITE : _Timeout, _Alertable);
}

// If the socket was created using oWinSockCreate (WSAEventSelect()), this function can 
// be used to wait on that event and receive any events breaking the wait.
bool oWinsockWait(SOCKET _hSocket, WSAEVENT _hEvent, WSANETWORKEVENTS* _pNetEvents, unsigned int _TimeoutMS)
{
	oWinsock* ws = oWinsock::Singleton();
	bool eventFired = true;
	unsigned int timeout = _TimeoutMS;
	_pNetEvents->lNetworkEvents = 0;
	while (!_pNetEvents->lNetworkEvents && eventFired)
	{
		{
			oScopedPartialTimeout spt(&timeout);
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
