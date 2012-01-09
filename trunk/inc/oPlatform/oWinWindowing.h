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
// Utilities for working with HWND objects. It's amazing how
// much of the Windows API is still a bit buggy, so prefer using
// API here over native calls, a seemingly simply wrappers might
// call several carefully-ordered win32 API.
#pragma once
#ifndef oWinWindowing_h
#define oWinWindowing_h

#include <oPlatform/oWindows.h>

// @oooii-tony: TODOs:
// Take out xywh from oWinCreate - use a separate API.
// Take out title from oWinCreate - use a separate API.

// Creates a simple window class and window. This should be the start of most
// windows to be used with other API from this system. Styles are controlled 
// separately with other API. _pThis should be the class that will be managing/
// wrapping the window. In this way a static WNDPROC can quickly become a method
// of a class. See oWinGetWrapper for more details.
bool oWinCreate(HWND* _pHwnd, WNDPROC _Wndproc, void* _pThis = nullptr, bool _SupportDoubleClicks = false, unsigned int _ClassUniqueID = 0);

// This function simplifies the association of a 'this' pointer with the HWND. 
// In the WndProc passed to oWinCreate, call this early to get the pointer 
// passed as _pThis. Then user code can convert quickly from the static 
// function requirement of Windows to a more encapsulated C++ class style
// handling of the WNDPROC. This transfers the CREATESTRUCT values to USERDATA 
// during WM_CREATE or for WM_INITDIALOG directly sets the pointer to USERDATA.
// If _pThis is nullptr, then DefWindowProc is called. See oDEFINE_STATIC_WNDPROC
// for a boilerplate implementation of the static wrapper.
void* oWinGetThis(HWND _hWnd, UINT _uMsg, WPARAM _wParam, LPARAM _lParam);
template<typename T> inline T* oWinGetThis(HWND _hWnd, UINT _uMsg, WPARAM _wParam, LPARAM _lParam) { return (T*)oWinGetThis(_hWnd, _uMsg, _wParam, _lParam); }

#define oDECLARE_WNDPROC(_Static, _Name) \
	_Static LRESULT CALLBACK _Name(HWND _hWnd, UINT _uMsg, WPARAM _wParam, LPARAM _lParam)

#define oDECLARE_DLGPROC(_Static, _Name) \
	_Static INT_PTR CALLBACK _Name(HWND _hWnd, UINT _uMsg, WPARAM _wParam, LPARAM _lParam)

#define oDEFINE_WNDPROC(_ClassName, _Name) \
	LRESULT CALLBACK _ClassName::_Name(HWND _hWnd, UINT _uMsg, WPARAM _wParam, LPARAM _lParam) \
	{	_ClassName* pThis = oWinGetThis<_ClassName>(_hWnd, _uMsg, _wParam, _lParam); \
		return pThis ? pThis->WndProc(_hWnd, _uMsg, _wParam, _lParam) : DefWindowProc(_hWnd, _uMsg, _wParam, _lParam); \
	}

#define oDEFINE_WNDPROC_DEBUG(_ClassName, _Name) \
	LRESULT CALLBACK _ClassName::_Name(HWND _hWnd, UINT _uMsg, WPARAM _wParam, LPARAM _lParam) \
	{	char WM[1024]; \
		oTRACE("%s", oWinParseWMMessage(WM, _hWnd, _uMsg, _wParam, _lParam)); \
		_ClassName* pThis = oWinGetThis<_ClassName>(_hWnd, _uMsg, _wParam, _lParam); \
		return pThis ? pThis->WndProc(_hWnd, _uMsg, _wParam, _lParam) : DefWindowProc(_hWnd, _uMsg, _wParam, _lParam); \
	}

#define oDEFINE_DLGPROC(_ClassName, _Name) \
	INT_PTR CALLBACK _ClassName::_Name(HWND _hWnd, UINT _uMsg, WPARAM _wParam, LPARAM _lParam) \
{	_ClassName* pThis = oWinGetThis<_ClassName>(_hWnd, _uMsg, _wParam, _lParam); \
	return pThis ? pThis->WndProc(_hWnd, _uMsg, _wParam, _lParam) : DefWindowProc(_hWnd, _uMsg, _wParam, _lParam); \
}

#define oDEFINE_DLGPROC_DEBUG(_ClassName, _Name) \
	INT_PTR CALLBACK _ClassName::_Name(HWND _hWnd, UINT _uMsg, WPARAM _wParam, LPARAM _lParam) \
{	char WM[1024]; \
	oTRACE("%s", oWinParseWMMessage(WM, _hWnd, _uMsg, _wParam, _lParam)); \
	_ClassName* pThis = oWinGetThis<_ClassName>(_hWnd, _uMsg, _wParam, _lParam); \
	return pThis ? pThis->WndProc(_hWnd, _uMsg, _wParam, _lParam) : DefWindowProc(_hWnd, _uMsg, _wParam, _lParam); \
}

// Pump the specified window's message pump for the specified time, by default
// infinitely/until it runs out of messages.
bool oWinPumpMessages(HWND _hWnd, unsigned int _TimeoutMS = oINFINITE_WAIT);

// Send this from a thread to wake the specified window's message
// pump thread up from a blocking GetMessage() call. This is useful
// if the thread does other work as well.
bool oWinWake(HWND _hWnd);

// Here are some wrapper accessors/mutators for HWNDs that bring some simplicity
// and consistency back to the API.

bool oWinExists(HWND _hWnd);
bool oWinHasFocus(HWND _hWnd);
bool oWinSetFocus(HWND _hWnd, bool _Focus = true);

bool oWinGetEnabled(HWND _hWnd);
bool oWinSetEnabled(HWND _hWnd, bool _Enabled = true);

// There's a known issue that a simple ShowWindow doesn't always work on some 
// minimized apps. The WAR seems to be to set focus to anything else, then try 
// to restore the  app.
bool oWinRestore(HWND _hWnd);

enum oWINDOW_STATE
{
	oWINDOW_NONEXISTANT,
	oWINDOW_HIDDEN,
	oWINDOW_MINIMIZED,
	oWINDOW_RESTORED,
	oWINDOW_MAXIMIZED,
};

oWINDOW_STATE oWinGetState(HWND _hWnd);
bool oWinSetState(HWND _hWnd, oWINDOW_STATE _State, bool _TakeFocus = true);

enum oWINDOW_STYLE
{
	oWINDOW_BORDERLESS, // Only the client area
	oWINDOW_FIXED, // Border and system controls: min/max disabled
	oWINDOW_DIALOG, // Border, but no system controls
	oWINDOW_SIZEABLE, // Border with min/max/close system controls
};

oWINDOW_STYLE oWinGetStyle(HWND _hWnd);
// SetStyle can also move/resize the window if _prClient is specified. _prClient
// is the position and size of the client area of the window. The actual window
// position and size are adjusted to accommodate the specified client size.
bool oWinSetStyle(HWND _hWnd, oWINDOW_STYLE _Style, const RECT* _prClient = nullptr);

// returns screen coordinates of client 0,0
int2 oWinGetPosition(HWND _hWnd);
// Sets window position such that the client coordinate 0,0 is at the specified 
// screen position
bool oWinSetPosition(HWND _hWnd, const int2& _ScreenPosition);

// Play a windows animation from and to the specified RECTs. This respects user 
// settings as to whether window animated transitions are on.
bool oWinAnimate(HWND _hWnd, const RECT& _From, const RECT& _To);

bool oWinSetTitle(HWND _hWnd, const char* _Title);
bool oWinGetTitle(HWND _hWnd, char* _Title, size_t _SizeofTitle);
template<size_t size> inline bool oGetTitle(HWND _hWnd, char (&_Title)[size]) { return oWinGetTitle(_hWnd, _Title, size); }

bool oWinGetAlwaysOnTop(HWND _hWnd);
bool oWinSetAlwaysOnTop(HWND _hWnd, bool _AlwaysOnTop = true);

HICON oWinGetIcon(HWND _hWnd, bool _BigIcon = false);
bool oWinSetIcon(HWND _hWnd, HICON _hIcon, bool _BigIcon = false);

// Gets the index of the display that is most closely associated with the 
// specified window. NOTE: This is an oDisplay-style index, not a DirectX
// index.
unsigned int oWinGetDisplayIndex(HWND _hWnd);

// Fills specified rect with the specified window's client rect, but
// in screen coordinates
bool oWinGetClientScreenRect(HWND _hWnd, RECT* _pRect);

// Modifies the default cursor associated with the client area of a Window
HCURSOR oWinGetCursor(HWND _hWnd);
bool oWinSetCursor(HWND _hWnd, HCURSOR _hCursor);

bool oWinCursorGetClipped(HWND _hWnd);
bool oWinCursorSetClipped(HWND _hWnd, bool _Clipped = true);

bool oWinCursorGetVisible();
void oWinCursorSetVisible(bool _Visible = true);

bool oWinCursorGetPosition(HWND _hWnd, int2* _ClientPosition);
bool oWinCursorSetPosition(HWND _hWnd, const int2& _ClientPosition);

#endif
