// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.
#pragma once
#ifndef oVKToX11Keyboard_h
#define oVKToX11Keyboard_h

#include <oGUI/oGUI.h>

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

// Convert between ouro::input::key and VK codes.
ouro::input::key oWinKeyToKey(DWORD _vkCode);
DWORD oWinKeyFromKey(ouro::input::key _Key);

// Call this from a WndProc to handle key-based events and translate them into
// an ouro::input::value. This requires an oWINKEY_CONTROL_STATE to stick around. It
// should be default-constructed before first call, then this function will 
// update it. If this returns true, then _pAction is well-formatted and should
// be dispatched. If this is false, then it means that further processing is 
// necessary and _pAction is not valid.
bool oWinKeyDispatchMessage(HWND _hWnd, UINT _uMsg, WPARAM _wParam, LPARAM _lParam, unsigned int _TimestampMS, oWINKEY_CONTROL_STATE* _pState, ouro::input::action* _pAction);

// Returns true if the specified key fits Microsoft's documented definition.
// This may not be wholly accurate because the actual definition is Keyboard 
// vendor-specific.
// http://msdn.microsoft.com/en-us/library/windows/desktop/ms646281(v=vs.85).aspx
bool oWinKeyIsExtended(ouro::input::key _Key);

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
void oWinKeySend(HWND _hWnd, ouro::input::key _Key, bool _IsDown, const int2& _MousePosition = int2(oDEFAULT, oDEFAULT));

void oWinSendKeys(HWND _Hwnd, unsigned int _ThreadID, short int* _pVKeys, int _NumberOfKeys);
void oWinSendASCIIMessage(HWND _Hwnd, unsigned int _ThreadID, const char* _pMessage);

#endif
