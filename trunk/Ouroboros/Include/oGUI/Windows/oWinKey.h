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
#ifndef oVKToX11Keyboard_h
#define oVKToX11Keyboard_h

#include <oBasis/oGUI.h>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

struct oWINKEY_CONTROL_STATE
{
	oWINKEY_CONTROL_STATE()
		: LControl(false)
		, RControl(false)
		, LShift(false)
		, RShift(false)
		, LMenu(false)
		, RMenu(false)
		, LastControl(0)
		, LastShift(0)
		, LastMenu(0)
	{}

	bool LControl;
	bool RControl;
	bool LShift;
	bool RShift;
	bool LMenu;
	bool RMenu;
	unsigned char LastControl;
	unsigned char LastShift;
	unsigned char LastMenu;
};

// For VK codes VK_CONTROL, VK_ALT, VK_SHIFT and VK_MENU, use some persistent
// state to translate the VK-code into a left or right value. Keep an 
// oCONTROL_KEY_STATE object somewhere where it can be passed to this function
// for each WM_SYSKEYDOWN/WM_SYSKEYUP or WM_KEYDOWN/WM_KEYUP message. Any other
// key will pass through unmodified.
DWORD oWinKeyTranslate(DWORD _vkCode, oWINKEY_CONTROL_STATE* _pState);
inline WPARAM oWinKeyTranslate(WPARAM _wParam, oWINKEY_CONTROL_STATE* _pState) { return (WPARAM)oWinKeyTranslate((DWORD)_wParam, _pState); }

// Convert between oGUI_KEY and VK codes.
oGUI_KEY oWinKeyToKey(DWORD _vkCode);
DWORD oWinKeyFromKey(oGUI_KEY _Key);

// Call this from a WndProc to handle key-based events and translate them into
// an oGUI_ACTION. This requires an oWINKEY_CONTROL_STATE to stick around. It
// should be default-constructed before first call, then this function will 
// update it. If this returns true, then _pAction is well-formatted and should
// be dispatched. If this is false, then it means that further processing is 
// necessary and _pAction is not valid.
bool oWinKeyDispatchMessage(HWND _hWnd, UINT _uMsg, WPARAM _wParam, LPARAM _lParam, unsigned int _TimestampMS, oWINKEY_CONTROL_STATE* _pState, oGUI_ACTION_DESC* _pAction);

// Returns true if the specified key fits Microsoft's documented definition.
// This may not be wholly accurate because the actual definition is Keyboard 
// vendor-specific.
// http://msdn.microsoft.com/en-us/library/windows/desktop/ms646281(v=vs.85).aspx
bool oWinKeyIsExtended(oGUI_KEY _Key);

// Returns true if the operating system reinterprets the specified VK key into
// something else without ever passing through WM_KEYDOWN/WM_KEYUP or 
// WM_SYSKEYDOWN/WM_SYSKEYUP events.
bool oWinKeyIsShortCircuited(DWORD _vkCode);

// Converts the information derived from a low-level keyboard hook into the 
// LPARAM format used in WM_KEYDOWN and WM_KEYUP messages. Because the struct
// cannot determine repeat or prior state, those must be derived from some other
// source and be passed in.
LPARAM oWinKeyToLParam(const KBDLLHOOKSTRUCT& _KB, unsigned short _RepeatCount, bool _PrevStateDown);

// Sends the specified key as up or down to the specified Window. This uses 
// PostMessage directly, so this can be sent to windows not in focus. 
// _MousePosition is respected only for mouse keys.
void oWinKeySend(HWND _hWnd, oGUI_KEY _Key, bool _IsDown, const int2& _MousePosition = int2(oDEFAULT, oDEFAULT));

#if 0

// Sends the position of the mouse as a WM_MOUSEMOVE
void oWinKeySendMouse(HWND _hWnd, const int2& _Position);

// Sends a down then up for each key in the specified array
void oWinKeySend(HWND _hWnd, const oGUI_KEY* _pKeys, size_t _NumKeys);
template<size_t size> void oWinKeySend(HWND _hWnd, const oGUI_KEY (&_pKeys)[size]) { oWinKeySend(_hWnd, _pKeys, Size); }

// Convenience wrapper for sending a readable string. There is no special format 
// analysis does for this, it basically sends what is specified.
void oWinKeySend(HWND _hWnd, const char* _String);

#else

bool oWinSendKeys(HWND _Hwnd, unsigned int _ThreadID, short int* _pVKeys, int _NumberKeys);
bool oWinSendASCIIMessage(HWND _Hwnd, unsigned int _ThreadID, const char* _pMessage);

#endif

#endif
