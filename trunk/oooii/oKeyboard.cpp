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
#include <oooii/oKeyboard.h>
#include <oooii/oRefCount.h>
#include <oooii/oThreading.h>
#include <oooii/oWindows.h>
#include "oKeyState.h"

template<> const char* oAsString(const oKeyboard::KEY& _Key)
{
	static const char* sKeyStrings[] =
	{
		"UNKNOWN",
		"A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K", "L", "M", "N", "O", "P", "Q", "R", "S", "T", "U", "V", "W", "X", "Y", "Z",
		"F1", "F2", "F3", "F4", "F5", "F6", "F7", "F8", "F9", "F10", "F11", "F12",
		"0", "1", "2", "3", "4", "5", "6", "7", "8", "9",
		"PAD0", "PAD1", "PAD2", "PAD3", "PAD4", "PAD5", "PAD6", "PAD7", "PAD8", "PAD9",
		"PADENTER", "PADADD", "PADSUBTRACT", "PADMULTIPLY", "PADDIVIDE", "PADDECIMAL",
		"UP", "DOWN", "LEFT", "RIGHT", "PGUP", "PGDOWN", "HOME", "END", "INS", "DEL", "ESC", "PRINTSCREEN", "PAUSEBREAK", "SCROLLLOCK", "TILDE", "MINUS", "EQUALS", "BACKSPACE", "TAB",
		"LBRACKET", "RBRACKET", "BACKSLASH", "CAPSLOCK", "SEMICOLON", "QUOTE", "ENTER", "COMMA", "PERIOD", "SLASH",
		"LSHIFT", "RSHIFT", "START", "LCTRL", "RCTRL", "LALT", "RALT", "MENU", "SPACE",
		"MUTE", "MEDIAPLAYPAUSE", "MEDIASTOP", "VOLUMEUP", "VOLUMEDOWN", "MEDIAPREV", "MEDIANEXT",
	};
	oSTATICASSERT(oCOUNTOF(sKeyStrings) == oKeyboard::NUM_KEYS);

	if (_Key < oKeyboard::NUM_KEYS)
		return sKeyStrings[_Key];
	return "unknown";
}

static oKeyboard::KEY GetKey(WPARAM _wParam)
{
	static const oKeyboard::KEY VKs[] = 
	{
		oKeyboard::KEY_UNKNOWN, // unassigned
		oKeyboard::KEY_UNKNOWN, // #define VK_LBUTTON        0x01
		oKeyboard::KEY_UNKNOWN, // #define VK_RBUTTON        0x02
		oKeyboard::KEY_UNKNOWN, // #define VK_CANCEL         0x03
		oKeyboard::KEY_UNKNOWN, // #define VK_MBUTTON        0x04    /* NOT contiguous with L & RBUTTON */
		oKeyboard::KEY_UNKNOWN, // #define VK_XBUTTON1       0x05    /* NOT contiguous with L & RBUTTON */
		oKeyboard::KEY_UNKNOWN, // #define VK_XBUTTON2       0x06    /* NOT contiguous with L & RBUTTON */
		oKeyboard::KEY_UNKNOWN, // unassigned 0x07
		oKeyboard::KEY_BACKSPACE, //#define VK_BACK           0x08
		oKeyboard::KEY_TAB, // #define VK_TAB            0x09
		oKeyboard::KEY_UNKNOWN, // reserved 0x0A
		oKeyboard::KEY_UNKNOWN, // reserved 0x0B
		oKeyboard::KEY_UNKNOWN, // #define VK_CLEAR          0x0C
		oKeyboard::KEY_ENTER, // #define VK_RETURN         0x0D
		oKeyboard::KEY_UNKNOWN, // unassigned 0x0E
		oKeyboard::KEY_UNKNOWN, // unassigned 0x0F
		oKeyboard::KEY_LSHIFT, // #define VK_SHIFT          0x10
		oKeyboard::KEY_LCTRL, // #define VK_CONTROL        0x11
		oKeyboard::KEY_LALT, // #define VK_MENU           0x12
		oKeyboard::KEY_PAUSEBREAK, // #define VK_PAUSE          0x13
		oKeyboard::KEY_UNKNOWN, // #define VK_CAPITAL        0x14
		oKeyboard::KEY_UNKNOWN, // #define VK_KANA           0x15
		oKeyboard::KEY_UNKNOWN, // ??? 0x16
		oKeyboard::KEY_UNKNOWN, // #define VK_JUNJA          0x17
		oKeyboard::KEY_UNKNOWN, // #define VK_FINAL          0x18
		oKeyboard::KEY_UNKNOWN, // #define VK_HANJA          0x19
		oKeyboard::KEY_UNKNOWN, // ??? 0x1A
		oKeyboard::KEY_ESC, // #define VK_ESCAPE         0x1B
		oKeyboard::KEY_UNKNOWN, // #define VK_CONVERT        0x1C
		oKeyboard::KEY_UNKNOWN, // #define VK_NONCONVERT     0x1D
		oKeyboard::KEY_UNKNOWN, // #define VK_ACCEPT         0x1E
		oKeyboard::KEY_UNKNOWN, // #define VK_MODECHANGE     0x1F
		oKeyboard::KEY_SPACE, // #define VK_SPACE          0x20
		oKeyboard::KEY_PGUP, // #define VK_PRIOR          0x21
		oKeyboard::KEY_PGDOWN, // #define VK_NEXT           0x22
		oKeyboard::KEY_END, // #define VK_END            0x23
		oKeyboard::KEY_HOME, // #define VK_HOME           0x24
		oKeyboard::KEY_LEFT, // #define VK_LEFT           0x25
		oKeyboard::KEY_UP, // #define VK_UP             0x26
		oKeyboard::KEY_RIGHT, // #define VK_RIGHT          0x27
		oKeyboard::KEY_DOWN, // #define VK_DOWN           0x28
		oKeyboard::KEY_UNKNOWN, // #define VK_SELECT         0x29
		oKeyboard::KEY_UNKNOWN, // #define VK_PRINT          0x2A
		oKeyboard::KEY_UNKNOWN, // #define VK_EXECUTE        0x2B
		oKeyboard::KEY_UNKNOWN, // #define VK_SNAPSHOT       0x2C
		oKeyboard::KEY_INS, // #define VK_INSERT         0x2D
		oKeyboard::KEY_DEL, //#define VaK_DELETE         0x2E
		oKeyboard::KEY_UNKNOWN, // #define VK_HELP           0x2F
		oKeyboard::KEY_0, oKeyboard::KEY_1, oKeyboard::KEY_2, oKeyboard::KEY_3, oKeyboard::KEY_4, oKeyboard::KEY_5, oKeyboard::KEY_6, oKeyboard::KEY_7, oKeyboard::KEY_8, oKeyboard::KEY_9,
		oKeyboard::KEY_UNKNOWN, // 0x3A unassigned
		oKeyboard::KEY_UNKNOWN, // 0x3B unassigned
		oKeyboard::KEY_UNKNOWN, // 0x3C unassigned
		oKeyboard::KEY_UNKNOWN, // 0x3D unassigned
		oKeyboard::KEY_UNKNOWN, // 0x3E unassigned
		oKeyboard::KEY_UNKNOWN, // 0x3F unassigned
		oKeyboard::KEY_UNKNOWN, // 0x40 unassigned
		oKeyboard::KEY_A, oKeyboard::KEY_B, oKeyboard::KEY_C, oKeyboard::KEY_D, oKeyboard::KEY_E, oKeyboard::KEY_F, oKeyboard::KEY_G, oKeyboard::KEY_H, oKeyboard::KEY_I, oKeyboard::KEY_J, oKeyboard::KEY_K, oKeyboard::KEY_L, oKeyboard::KEY_M, oKeyboard::KEY_N, oKeyboard::KEY_O, oKeyboard::KEY_P, oKeyboard::KEY_Q, oKeyboard::KEY_R, oKeyboard::KEY_S, oKeyboard::KEY_T, oKeyboard::KEY_U, oKeyboard::KEY_V, oKeyboard::KEY_W, oKeyboard::KEY_X, oKeyboard::KEY_Y, oKeyboard::KEY_Z,
		oKeyboard::KEY_START, // #define VK_LWIN           0x5B
		oKeyboard::KEY_UNKNOWN, // #define VK_RWIN           0x5C
		oKeyboard::KEY_MENU, // #define VK_APPS           0x5D
		oKeyboard::KEY_UNKNOWN, // 0x5E reserved
		oKeyboard::KEY_UNKNOWN, //#define VK_SLEEP          0x5F
		oKeyboard::KEY_PAD0, //#define VK_NUMPAD0        0x60
		oKeyboard::KEY_PAD1, //#define VK_NUMPAD1        0x61
		oKeyboard::KEY_PAD2, //#define VK_NUMPAD2        0x62
		oKeyboard::KEY_PAD3, //#define VK_NUMPAD3        0x63
		oKeyboard::KEY_PAD4, //#define VK_NUMPAD4        0x64
		oKeyboard::KEY_PAD5, //#define VK_NUMPAD5        0x65
		oKeyboard::KEY_PAD6, //#define VK_NUMPAD6        0x66
		oKeyboard::KEY_PAD7, //#define VK_NUMPAD7        0x67
		oKeyboard::KEY_PAD8, //#define VK_NUMPAD8        0x68
		oKeyboard::KEY_PAD9, //#define VK_NUMPAD9        0x69
		oKeyboard::KEY_PADMULTIPLY, // #define VK_MULTIPLY       0x6A
		oKeyboard::KEY_PADADD, // #define VK_ADD            0x6B
		oKeyboard::KEY_UNKNOWN, // #define VK_SEPARATOR      0x6C
		oKeyboard::KEY_PADSUBTRACT, // #define VK_SUBTRACT       0x6D
		oKeyboard::KEY_PADDECIMAL, // #define VK_DECIMAL        0x6E
		oKeyboard::KEY_PADDIVIDE, // #define VK_DIVIDE         0x6F
		oKeyboard::KEY_F1, // #define VK_F1             0x70
		oKeyboard::KEY_F2, // #define VK_F2             0x71
		oKeyboard::KEY_F3, // #define VK_F3             0x72
		oKeyboard::KEY_F4, // #define VK_F4             0x73
		oKeyboard::KEY_F5, // #define VK_F5             0x74
		oKeyboard::KEY_F6, // #define VK_F6             0x75
		oKeyboard::KEY_F7, // #define VK_F7             0x76
		oKeyboard::KEY_F8, // #define VK_F8             0x77
		oKeyboard::KEY_F9, // #define VK_F9             0x78
		oKeyboard::KEY_F10, // #define VK_F10            0x79
		oKeyboard::KEY_F11, // #define VK_F11            0x7A
		oKeyboard::KEY_F12, // #define VK_F12            0x7B
		oKeyboard::KEY_UNKNOWN, // #define VK_F13            0x7C
		oKeyboard::KEY_UNKNOWN, // #define VK_F14            0x7D
		oKeyboard::KEY_UNKNOWN, // #define VK_F15            0x7E
		oKeyboard::KEY_UNKNOWN, // #define VK_F16            0x7F
		oKeyboard::KEY_UNKNOWN, // #define VK_F17            0x80
		oKeyboard::KEY_UNKNOWN, // #define VK_F18            0x81
		oKeyboard::KEY_UNKNOWN, // #define VK_F19            0x82
		oKeyboard::KEY_UNKNOWN, // #define VK_F20            0x83
		oKeyboard::KEY_UNKNOWN, // #define VK_F21            0x84
		oKeyboard::KEY_UNKNOWN, // #define VK_F22            0x85
		oKeyboard::KEY_UNKNOWN, // #define VK_F23            0x86
		oKeyboard::KEY_UNKNOWN, // #define VK_F24            0x87
		oKeyboard::KEY_UNKNOWN, // 0x88 unassigned
		oKeyboard::KEY_UNKNOWN, // 0x89 unassigned
		oKeyboard::KEY_UNKNOWN, // 0x8A unassigned
		oKeyboard::KEY_UNKNOWN, // 0x8B unassigned
		oKeyboard::KEY_UNKNOWN, // 0x8C unassigned
		oKeyboard::KEY_UNKNOWN, // 0x8D unassigned
		oKeyboard::KEY_UNKNOWN, // 0x8E unassigned
		oKeyboard::KEY_UNKNOWN, // 0x8F unassigned
		oKeyboard::KEY_UNKNOWN, // #define VK_NUMLOCK        0x90
		oKeyboard::KEY_SCROLLLOCK, // #define VK_SCROLL         0x91
		oKeyboard::KEY_UNKNOWN, // #define VK_OEM_NEC_EQUAL  0x92   // '=' key on numpad
		oKeyboard::KEY_UNKNOWN, // #define VK_OEM_FJ_MASSHOU 0x93   // 'Unregister word' key
		oKeyboard::KEY_UNKNOWN, // #define VK_OEM_FJ_TOUROKU 0x94   // 'Register word' key
		oKeyboard::KEY_UNKNOWN, // #define VK_OEM_FJ_LOYA    0x95   // 'Left OYAYUBI' key
		oKeyboard::KEY_UNKNOWN, // #define VK_OEM_FJ_ROYA    0x96   // 'Right OYAYUBI' key
		oKeyboard::KEY_UNKNOWN, // 0x97 unassigned
		oKeyboard::KEY_UNKNOWN, // 0x98 unassigned
		oKeyboard::KEY_UNKNOWN, // 0x99 unassigned
		oKeyboard::KEY_UNKNOWN, // 0x9A unassigned
		oKeyboard::KEY_UNKNOWN, // 0x9B unassigned
		oKeyboard::KEY_UNKNOWN, // 0x9C unassigned
		oKeyboard::KEY_UNKNOWN, // 0x9D unassigned
		oKeyboard::KEY_UNKNOWN, // 0x9E unassigned
		oKeyboard::KEY_UNKNOWN, // 0x9F unassigned

		/*
		 * VK_L* & VK_R* - left and right Alt, Ctrl and Shift virtual keys.
		 * Used only as parameters to GetAsyncKeyState() and GetKeyState().
		 * No other API or message will distinguish left and right keys in this way.
		 */
		oKeyboard::KEY_LSHIFT, // #define VK_LSHIFT         0xA0
		oKeyboard::KEY_RSHIFT, // #define VK_RSHIFT         0xA1
		oKeyboard::KEY_LCTRL, // #define VK_LCONTROL       0xA2
		oKeyboard::KEY_RCTRL, // #define VK_RCONTROL       0xA3
		oKeyboard::KEY_LALT, // #define VK_LMENU          0xA4
		oKeyboard::KEY_RALT, // #define VK_RMENU          0xA5
		oKeyboard::KEY_UNKNOWN, // #define VK_BROWSER_BACK        0xA6
		oKeyboard::KEY_UNKNOWN, // #define VK_BROWSER_FORWARD     0xA7
		oKeyboard::KEY_UNKNOWN, // #define VK_BROWSER_REFRESH     0xA8
		oKeyboard::KEY_UNKNOWN, // #define VK_BROWSER_STOP        0xA9
		oKeyboard::KEY_UNKNOWN, // #define VK_BROWSER_SEARCH      0xAA
		oKeyboard::KEY_UNKNOWN, // #define VK_BROWSER_FAVORITES   0xAB
		oKeyboard::KEY_UNKNOWN, // #define VK_BROWSER_HOME        0xAC
		oKeyboard::KEY_MUTE, // #define VK_VOLUME_MUTE         0xAD
		oKeyboard::KEY_VOLUMEDOWN, // #define VK_VOLUME_DOWN         0xAE
		oKeyboard::KEY_VOLUMEUP, // #define VK_VOLUME_UP           0xAF
		oKeyboard::KEY_MEDIANEXT, // #define VK_MEDIA_NEXT_TRACK    0xB0
		oKeyboard::KEY_MEDIAPREV, // #define VK_MEDIA_PREV_TRACK    0xB1
		oKeyboard::KEY_MEDIASTOP, // #define VK_MEDIA_STOP          0xB2
		oKeyboard::KEY_MEDIAPLAYPAUSE, // #define VK_MEDIA_PLAY_PAUSE    0xB3
		oKeyboard::KEY_UNKNOWN, // #define VK_LAUNCH_MAIL         0xB4
		oKeyboard::KEY_UNKNOWN, // #define VK_LAUNCH_MEDIA_SELECT 0xB5
		oKeyboard::KEY_UNKNOWN, // #define VK_LAUNCH_APP1         0xB6
		oKeyboard::KEY_UNKNOWN, // #define VK_LAUNCH_APP2         0xB7
		oKeyboard::KEY_UNKNOWN, // 0xB8 reserved
		oKeyboard::KEY_UNKNOWN, // 0xB9 reserved
		oKeyboard::KEY_SEMICOLON, // #define VK_OEM_1          0xBA   // ';:' for US
		oKeyboard::KEY_EQUALS, // #define VK_OEM_PLUS       0xBB   // '+' any country
		oKeyboard::KEY_COMMA, // #define VK_OEM_COMMA      0xBC   // ',' any country
		oKeyboard::KEY_MINUS, // #define VK_OEM_MINUS      0xBD   // '-' any country
		oKeyboard::KEY_PERIOD, // #define VK_OEM_PERIOD     0xBE   // '.' any country
		oKeyboard::KEY_SLASH, // #define VK_OEM_2          0xBF   // '/?' for US
		oKeyboard::KEY_TILDE, // #define VK_OEM_3          0xC0   // '`~' for US
		oKeyboard::KEY_UNKNOWN, // 0xC1 unassigned
		oKeyboard::KEY_UNKNOWN, // 0xC2 unassigned
		oKeyboard::KEY_UNKNOWN, // 0xC3 unassigned
		oKeyboard::KEY_UNKNOWN, // 0xC4 unassigned
		oKeyboard::KEY_UNKNOWN, // 0xC5 unassigned
		oKeyboard::KEY_UNKNOWN, // 0xC6 unassigned
		oKeyboard::KEY_UNKNOWN, // 0xC7 unassigned
		oKeyboard::KEY_UNKNOWN, // 0xC8 unassigned
		oKeyboard::KEY_UNKNOWN, // 0xC9 unassigned
		oKeyboard::KEY_UNKNOWN, // 0xCA unassigned
		oKeyboard::KEY_UNKNOWN, // 0xCB unassigned
		oKeyboard::KEY_UNKNOWN, // 0xCC unassigned
		oKeyboard::KEY_UNKNOWN, // 0xCD unassigned
		oKeyboard::KEY_UNKNOWN, // 0xCE unassigned
		oKeyboard::KEY_UNKNOWN, // 0xCF unassigned
		oKeyboard::KEY_UNKNOWN, // 0xD0 unassigned
		oKeyboard::KEY_UNKNOWN, // 0xD1 unassigned
		oKeyboard::KEY_UNKNOWN, // 0xD2 unassigned
		oKeyboard::KEY_UNKNOWN, // 0xD3 unassigned
		oKeyboard::KEY_UNKNOWN, // 0xD4 unassigned
		oKeyboard::KEY_UNKNOWN, // 0xD5 unassigned
		oKeyboard::KEY_UNKNOWN, // 0xD6 unassigned
		oKeyboard::KEY_UNKNOWN, // 0xD7 unassigned
		oKeyboard::KEY_UNKNOWN, // 0xD8 unassigned
		oKeyboard::KEY_UNKNOWN, // 0xD9 unassigned
		oKeyboard::KEY_UNKNOWN, // 0xDA unassigned
		oKeyboard::KEY_LBRACKET, // #define VK_OEM_4          0xDB  //  '[{' for US
		oKeyboard::KEY_BACKSLASH, // #define VK_OEM_5          0xDC  //  '\\|' for US
		oKeyboard::KEY_RBRACKET, // #define VK_OEM_6          0xDD  //  ']}' for US
		oKeyboard::KEY_QUOTE, // #define VK_OEM_7          0xDE  //  ''"' for US
		oKeyboard::KEY_UNKNOWN, // #define VK_OEM_8          0xDF
		oKeyboard::KEY_UNKNOWN, // 0xE0 reserved
		oKeyboard::KEY_UNKNOWN, // #define VK_OEM_AX         0xE1  //  'AX' key on Japanese AX kbd
		oKeyboard::KEY_UNKNOWN, // #define VK_OEM_102        0xE2  //  "<>" or "\\|" on RT 102-key kbd.
		oKeyboard::KEY_UNKNOWN, // #define VK_ICO_HELP       0xE3  //  Help key on ICO
		oKeyboard::KEY_UNKNOWN, // #define VK_ICO_00         0xE4  //  00 key on ICO
		oKeyboard::KEY_UNKNOWN, // #define VK_PROCESSKEY     0xE5
		oKeyboard::KEY_UNKNOWN, // #define VK_ICO_CLEAR      0xE6
		oKeyboard::KEY_UNKNOWN, // #define VK_PACKET         0xE7
		oKeyboard::KEY_UNKNOWN, // 0xE8 unassigned
		oKeyboard::KEY_UNKNOWN, // #define VK_OEM_RESET      0xE9
		oKeyboard::KEY_UNKNOWN, // #define VK_OEM_JUMP       0xEA
		oKeyboard::KEY_UNKNOWN, // #define VK_OEM_PA1        0xEB
		oKeyboard::KEY_UNKNOWN, // #define VK_OEM_PA2        0xEC
		oKeyboard::KEY_UNKNOWN, // #define VK_OEM_PA3        0xED
		oKeyboard::KEY_UNKNOWN, // #define VK_OEM_WSCTRL     0xEE
		oKeyboard::KEY_UNKNOWN, // #define VK_OEM_CUSEL      0xEF
		oKeyboard::KEY_UNKNOWN, // #define VK_OEM_ATTN       0xF0
		oKeyboard::KEY_UNKNOWN, // #define VK_OEM_FINISH     0xF1
		oKeyboard::KEY_UNKNOWN, // #define VK_OEM_COPY       0xF2
		oKeyboard::KEY_UNKNOWN, // #define VK_OEM_AUTO       0xF3
		oKeyboard::KEY_UNKNOWN, // #define VK_OEM_ENLW       0xF4
		oKeyboard::KEY_UNKNOWN, // #define VK_OEM_BACKTAB    0xF5
		oKeyboard::KEY_UNKNOWN, // #define VK_ATTN           0xF6
		oKeyboard::KEY_UNKNOWN, // #define VK_CRSEL          0xF7
		oKeyboard::KEY_UNKNOWN, // #define VK_EXSEL          0xF8
		oKeyboard::KEY_UNKNOWN, // #define VK_EREOF          0xF9
		oKeyboard::KEY_UNKNOWN, // #define VK_PLAY           0xFA
		oKeyboard::KEY_UNKNOWN, // #define VK_ZOOM           0xFB
		oKeyboard::KEY_UNKNOWN, // #define VK_NONAME         0xFC
		oKeyboard::KEY_UNKNOWN, // #define VK_PA1            0xFD
		oKeyboard::KEY_UNKNOWN, // #define VK_OEM_CLEAR      0xFE
	};

	oKeyboard::KEY k = oKeyboard::KEY_UNKNOWN;
	if ((int)_wParam < oCOUNTOF(VKs))
		k = VKs[_wParam];

	if (k == oKeyboard::KEY_LSHIFT && (GetKeyState(VK_RSHIFT) & 0x8000)) return oKeyboard::KEY_RSHIFT;
	else if (k == oKeyboard::KEY_LCTRL && (GetKeyState(VK_RCONTROL) & 0x8000)) return oKeyboard::KEY_RCTRL;
	else if (k == oKeyboard::KEY_LALT && (GetKeyState(VK_RMENU) & 0x8000)) return oKeyboard::KEY_RALT;
	return k;
}

char oKeyboard::ToAscii(KEY _Key, bool _IsShiftDown)
{
	if (_Key >= KEY_A && _Key <= KEY_Z)
		return (char)(_Key - KEY_A + (_IsShiftDown ? 'A' : 'a'));

	if (_Key >= KEY_0 && _Key <= KEY_9 && !_IsShiftDown)
		return (char)(_Key - KEY_0 + '0');

	if (_Key >= KEY_PAD0 && _Key <= KEY_PAD9)
		return (char)(_Key - KEY_PAD0 + '0');

	switch (_Key)
	{
		// for KEY_? if we got here, _IsShiftDown is true
		case KEY_0: return ')';
		case KEY_1: return '!';
		case KEY_2: return '@';
		case KEY_3: return '#';
		case KEY_4: return '$';
		case KEY_5: return '%';
		case KEY_6: return '^';
		case KEY_7: return '&';
		case KEY_8: return '*';
		case KEY_9: return '(';
		case KEY_PADADD: return '+';
		case KEY_PADSUBTRACT: return '-';
		case KEY_PADMULTIPLY: return '*';
		case KEY_PADDIVIDE: return '/';
		case KEY_PADDECIMAL: return '.';
		case KEY_TILDE: return _IsShiftDown ? '~' : '`';
		case KEY_MINUS: return _IsShiftDown ? '_' : '-';
		case KEY_EQUALS: return _IsShiftDown ? '+' : '=';
		case KEY_TAB: return '\t';
		case KEY_LBRACKET: return _IsShiftDown ? '{' : '[';
		case KEY_RBRACKET: return _IsShiftDown ? '}' : ']';
		case KEY_BACKSLASH: return _IsShiftDown ? '|' : '\\';
		case KEY_SEMICOLON: return _IsShiftDown ? ':' : ';';
		case KEY_QUOTE: return _IsShiftDown ? '\'' : '\"';
		case KEY_COMMA: return _IsShiftDown ? '<' : ',';
		case KEY_PERIOD: return _IsShiftDown ? '>' : '.';
		case KEY_SLASH: return _IsShiftDown ? '?' : '/';
		case KEY_SPACE: return ' ';
		default: break;
	}

	return 0;
}

const oGUID& oGetGUID( threadsafe const oKeyboard* threadsafe const * )
{
	// {13E8CC08-E3DF-4d09-A0C4-E6AB84EFB714}
	static const oGUID oIIDKeyboard = { 0x13e8cc08, 0xe3df, 0x4d09, { 0xa0, 0xc4, 0xe6, 0xab, 0x84, 0xef, 0xb7, 0x14 } };
	return oIIDKeyboard;
}

struct Keyboard_Impl : public oKeyboard
{
	oDEFINE_REFCOUNT_INTERFACE(RefCount);
	oDEFINE_TRIVIAL_QUERYINTERFACE(oGetGUID<oKeyboard>());

	Keyboard_Impl(HWND _hWnd, bool _ShortCircuitChain, bool* pSuccess = 0)
		: ShortCircuitChain(_ShortCircuitChain)
	{
		hHook = oSetWindowsHook(WH_KEYBOARD, KBHook, this, _hWnd);
		if (pSuccess)
			*pSuccess = !!hHook;
	}

	~Keyboard_Impl()
	{
		if (hHook)
			oUnhookWindowsHook(hHook);
	}

	void Update() threadsafe override
	{
		State.PromoteReleasedPressedToUpDown();
	}

	bool IsUp(KEY _Key) const threadsafe override { return State.IsUp(_Key); }
	bool IsReleased(KEY _Key) const threadsafe override { return State.IsReleased(_Key); }
	bool IsDown(KEY _Key) const threadsafe override { return State.IsDown(_Key); }
	bool IsPressed(KEY _Key) const threadsafe override { return State.IsPressed(_Key); }

	KEY FindPressedKey(KEY _KeyStart) const threadsafe
	{
		int k = State.FindFirstPressed(_KeyStart);
		return k >= 0 ? (KEY)k : KEY_UNKNOWN;
	}

	bool KBHook(int _nCode, WPARAM _wParam, LPARAM _lParam) threadsafe
	{
		oKeyboard::KEY k = GetKey(_wParam);
		if (k == KEY_UNKNOWN)
			return false;

		bool isDown = (_lParam & (1<<31)) == 0;
		State.RecordUpDown(k, isDown);

		// always allow WM_SYSKEY messages to go through
		if (IsDown(KEY_LALT) || IsDown(KEY_RALT))
			return false;
		
		return ShortCircuitChain;
	}

	static LRESULT CALLBACK KBHook(int _nCode, WPARAM _wParam, LPARAM _lParam, void* _pUserData)
	{
		if (_nCode == HC_ACTION)
		{
			threadsafe Keyboard_Impl* pThis = static_cast<threadsafe Keyboard_Impl*>(_pUserData);
			if (pThis->KBHook(_nCode, _wParam, _lParam))
				return 1;
		}

		return CallNextHookEx(0, _nCode, _wParam, _lParam);
	}

	HHOOK hHook;
	oRefCount RefCount;
	bool ShortCircuitChain;
	oKeyState<NUM_KEYS> State;
};

bool oKeyboard::Create(void* _AssociatedNativeHandle, bool _ShortCircuitKeyEvents, threadsafe oKeyboard** _ppKeyboard)
{
	if (!_AssociatedNativeHandle || !_ppKeyboard) return false;
	bool success = false;
	*_ppKeyboard = new Keyboard_Impl((HWND)_AssociatedNativeHandle, _ShortCircuitKeyEvents, &success);
	if (*_ppKeyboard && !success)
	{
		delete *_ppKeyboard;
		*_ppKeyboard = 0;
	}

	return success;
}
