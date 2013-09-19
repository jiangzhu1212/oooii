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
#include <oPlatform/Windows/oWinKey.h>
#include <windowsx.h>

DWORD oWinKeyTranslate(DWORD _vkCode, oWINKEY_CONTROL_STATE* _pState)
{
	#define TEST_DOWN_SIDE(Side, Key, KEY) do \
		{	if (!_pState->oCONCAT(Side, Key) && GetAsyncKeyState(oCONCAT(VK_,Side##KEY))) \
			{	_pState->oCONCAT(Side, Key) = true; \
				_pState->oCONCAT(Last, Key) = oCONCAT(VK_,Side##KEY); \
			} \
		} while (false)

	#define TEST_UP_SIDE(Side, Key, KEY) do \
		{	if (_pState->oCONCAT(Side, Key) && !GetAsyncKeyState(oCONCAT(VK_,Side##KEY))) \
			{	_pState->oCONCAT(Side, Key) = false; \
				_pState->oCONCAT(Last, Key) = oCONCAT(VK_,Side##KEY); \
			} \
		} while (false)


	if (_pState)
	{
		#define TEST(Key, KEY) TEST_DOWN_SIDE(L, Key, KEY); TEST_DOWN_SIDE(R, Key, KEY); TEST_UP_SIDE(L, Key, KEY); TEST_UP_SIDE(R, Key, KEY)
		TEST(Control, CONTROL); TEST(Shift, SHIFT); TEST(Menu, MENU);

		switch (_vkCode)
		{
			case VK_CONTROL: return _pState->LastControl;
			case VK_MENU: return _pState->LastMenu;
			case VK_SHIFT: return _pState->LastShift;
			default: break;
		}
	}

	return _vkCode;
}

oGUI_KEY oWinKeyToKey(DWORD _vkCode)
{
	static unsigned char sKeys[] =
	{
		0,
		oGUI_KEY_MOUSE_LEFT,
		oGUI_KEY_MOUSE_RIGHT,
		VK_CANCEL,
		oGUI_KEY_MOUSE_MIDDLE,
		oGUI_KEY_MOUSE_SIDE1,
		oGUI_KEY_MOUSE_SIDE2,
		oGUI_KEY_NONE, // 0x07
		oGUI_KEY_BACKSPACE,
		oGUI_KEY_TAB,
		oGUI_KEY_NONE, // 0x0A
		oGUI_KEY_NONE, // 0x0B
		oGUI_KEY_NONE, // VK_CLEAR
		oGUI_KEY_ENTER,
		oGUI_KEY_NONE, // 0x0E
		oGUI_KEY_NONE, // 0x0F
		oGUI_KEY_LSHIFT, // VK_SHIFT
		oGUI_KEY_LCTRL, // VK_CONTROL
		oGUI_KEY_LALT, // VK_MENU
		oGUI_KEY_PAUSE,
		oGUI_KEY_CAPSLOCK,
		oGUI_KEY_NONE, // VK_KANA
		oGUI_KEY_NONE, //VK_HANGUEL/VK_HANGUL
		oGUI_KEY_NONE, // 0x16
		oGUI_KEY_NONE, // VK_KANA
		oGUI_KEY_NONE, // VK_HANJA/VK_KANJI
		oGUI_KEY_NONE, // 0x1A
		oGUI_KEY_ESC,
		oGUI_KEY_NONE, // VK_CONVERT
		oGUI_KEY_NONE, // VK_NONCONVERT
		oGUI_KEY_NONE, // VK_ACCEPT
		oGUI_KEY_NONE, // VK_MODECHANGE
		oGUI_KEY_SPACE,
		oGUI_KEY_PGUP,
		oGUI_KEY_PGDN,
		oGUI_KEY_END,
		oGUI_KEY_HOME,
		oGUI_KEY_LEFT,
		oGUI_KEY_UP,
		oGUI_KEY_RIGHT,
		oGUI_KEY_DOWN,
		oGUI_KEY_NONE, // VK_SELECT
		oGUI_KEY_NONE, // VK_PRINT
		oGUI_KEY_NONE, // VK_EXECUTE
		oGUI_KEY_PRINTSCREEN, // VK_SNAPSHOT
		oGUI_KEY_INS,
		oGUI_KEY_DEL,
		oGUI_KEY_NONE, // VK_HELP
		oGUI_KEY_0,
		oGUI_KEY_1,
		oGUI_KEY_2,
		oGUI_KEY_3,
		oGUI_KEY_4,
		oGUI_KEY_5,
		oGUI_KEY_6,
		oGUI_KEY_7,
		oGUI_KEY_8,
		oGUI_KEY_9,
		oGUI_KEY_NONE, // 0x3A
		oGUI_KEY_NONE, // 0x3B
		oGUI_KEY_NONE, // 0x3C
		oGUI_KEY_NONE, // 0x3D
		oGUI_KEY_NONE, // 0x3E
		oGUI_KEY_NONE, // 0x3F
		oGUI_KEY_NONE, // 0x40
		oGUI_KEY_A,
		oGUI_KEY_B,
		oGUI_KEY_C,
		oGUI_KEY_D,
		oGUI_KEY_E,
		oGUI_KEY_F,
		oGUI_KEY_G,
		oGUI_KEY_H,
		oGUI_KEY_I,
		oGUI_KEY_J,
		oGUI_KEY_K,
		oGUI_KEY_L,
		oGUI_KEY_M,
		oGUI_KEY_N,
		oGUI_KEY_O,
		oGUI_KEY_P,
		oGUI_KEY_Q,
		oGUI_KEY_R,
		oGUI_KEY_S,
		oGUI_KEY_T,
		oGUI_KEY_U,
		oGUI_KEY_V,
		oGUI_KEY_W,
		oGUI_KEY_X,
		oGUI_KEY_Y,
		oGUI_KEY_Z,
		oGUI_KEY_LWIN,
		oGUI_KEY_RWIN,
		oGUI_KEY_APP_CONTEXT,
		oGUI_KEY_NONE, // 0x5E
		oGUI_KEY_SLEEP,
		oGUI_KEY_NUM0,
		oGUI_KEY_NUM1,
		oGUI_KEY_NUM2,
		oGUI_KEY_NUM3,
		oGUI_KEY_NUM4,
		oGUI_KEY_NUM5,
		oGUI_KEY_NUM6,
		oGUI_KEY_NUM7,
		oGUI_KEY_NUM8,
		oGUI_KEY_NUM9,
		oGUI_KEY_NUMMUL,
		oGUI_KEY_NUMADD,
		oGUI_KEY_NONE, // VK_SEPARATOR
		oGUI_KEY_NUMSUB,
		oGUI_KEY_NUMDECIMAL,
		oGUI_KEY_NUMDIV,
		oGUI_KEY_F1,
		oGUI_KEY_F2,
		oGUI_KEY_F3,
		oGUI_KEY_F4,
		oGUI_KEY_F5,
		oGUI_KEY_F6,
		oGUI_KEY_F7,
		oGUI_KEY_F8,
		oGUI_KEY_F9,
		oGUI_KEY_F10,
		oGUI_KEY_F11,
		oGUI_KEY_F12,
		oGUI_KEY_F13,
		oGUI_KEY_F14,
		oGUI_KEY_F15,
		oGUI_KEY_F16,
		oGUI_KEY_F17,
		oGUI_KEY_F18,
		oGUI_KEY_F19,
		oGUI_KEY_F20,
		oGUI_KEY_F21,
		oGUI_KEY_F22,
		oGUI_KEY_F23,
		oGUI_KEY_F24,
		oGUI_KEY_NONE, // 0x88
		oGUI_KEY_NONE, // 0x89
		oGUI_KEY_NONE, // 0x8A
		oGUI_KEY_NONE, // 0x8B
		oGUI_KEY_NONE, // 0x8C
		oGUI_KEY_NONE, // 0x8D
		oGUI_KEY_NONE, // 0x8E
		oGUI_KEY_NONE, // 0x8F
		oGUI_KEY_NUMLOCK,
		oGUI_KEY_SCROLLLOCK,
		oGUI_KEY_NONE, // 0x92
		oGUI_KEY_NONE, // 0x93
		oGUI_KEY_NONE, // 0x94
		oGUI_KEY_NONE, // 0x95
		oGUI_KEY_NONE, // 0x96
		oGUI_KEY_NONE, // 0x97
		oGUI_KEY_NONE, // 0x98
		oGUI_KEY_NONE, // 0x99
		oGUI_KEY_NONE, // 0x9A
		oGUI_KEY_NONE, // 0x9B
		oGUI_KEY_NONE, // 0x9C
		oGUI_KEY_NONE, // 0x9D
		oGUI_KEY_NONE, // 0x9E
		oGUI_KEY_NONE, // 0x9F
		oGUI_KEY_LSHIFT,
		oGUI_KEY_RSHIFT,
		oGUI_KEY_LCTRL,
		oGUI_KEY_RCTRL,
		oGUI_KEY_LALT,
		oGUI_KEY_RALT,
		oGUI_KEY_BACK,
		oGUI_KEY_FORWARD,
		oGUI_KEY_REFRESH,
		oGUI_KEY_STOP,
		oGUI_KEY_SEARCH,
		oGUI_KEY_FAVS,
		oGUI_KEY_HOME,
		oGUI_KEY_MUTE,
		oGUI_KEY_VOLDN,
		oGUI_KEY_VOLUP,
		oGUI_KEY_NEXT_TRACK,
		oGUI_KEY_PREV_TRACK,
		oGUI_KEY_STOP_TRACK,
		oGUI_KEY_PLAY_PAUSE_TRACK,
		oGUI_KEY_MAIL,
		oGUI_KEY_MEDIA,
		oGUI_KEY_APP1,
		oGUI_KEY_APP2,
		oGUI_KEY_NONE, // 0xB8
		oGUI_KEY_NONE, // 0xB9
		oGUI_KEY_SEMICOLON,
		oGUI_KEY_EQUAL,
		oGUI_KEY_COMMA,
		oGUI_KEY_DASH,
		oGUI_KEY_PERIOD,
		oGUI_KEY_SLASH,
		oGUI_KEY_BACKTICK,

		oGUI_KEY_NONE, // 0xC1
		oGUI_KEY_NONE, // 0xC2
		oGUI_KEY_NONE, // 0xC3
		oGUI_KEY_NONE, // 0xC4
		oGUI_KEY_NONE, // 0xC5
		oGUI_KEY_NONE, // 0xC6
		oGUI_KEY_NONE, // 0xC7
		oGUI_KEY_NONE, // 0xC8
		oGUI_KEY_NONE, // 0xC9
		oGUI_KEY_NONE, // 0xCA
		oGUI_KEY_NONE, // 0xCB
		oGUI_KEY_NONE, // 0xCC
		oGUI_KEY_NONE, // 0xCD
		oGUI_KEY_NONE, // 0xCE
		oGUI_KEY_NONE, // 0xCF

		oGUI_KEY_NONE, // 0xD0
		oGUI_KEY_NONE, // 0xD1
		oGUI_KEY_NONE, // 0xD2
		oGUI_KEY_NONE, // 0xD3
		oGUI_KEY_NONE, // 0xD4
		oGUI_KEY_NONE, // 0xD5
		oGUI_KEY_NONE, // 0xD6
		oGUI_KEY_NONE, // 0xD7
		oGUI_KEY_NONE, // 0xD8
		oGUI_KEY_NONE, // 0xD9
		oGUI_KEY_NONE, // 0xDA
		oGUI_KEY_LBRACKET,
		oGUI_KEY_BACKSLASH,
		oGUI_KEY_RBRACKET,
		oGUI_KEY_APOSTROPHE,
		oGUI_KEY_NONE, // VK_OEM_8
		oGUI_KEY_NONE, // 0xE0
		oGUI_KEY_NONE, // 0xE1
		oGUI_KEY_NONE, // VK_OEM_102
		oGUI_KEY_NONE, // 0xE3
		oGUI_KEY_NONE, // 0xE4
		oGUI_KEY_NONE, // VK_PROCESSKEY
		oGUI_KEY_NONE, // 0xE6
		oGUI_KEY_NONE, // VK_PACKET
		oGUI_KEY_NONE, // 0xE8
		oGUI_KEY_NONE, // 0xE9
		oGUI_KEY_NONE, // 0xEA
		oGUI_KEY_NONE, // 0xEB
		oGUI_KEY_NONE, // 0xEC
		oGUI_KEY_NONE, // 0xED
		oGUI_KEY_NONE, // 0xEE
		oGUI_KEY_NONE, // 0xEF
		oGUI_KEY_NONE, // 0xF0
		oGUI_KEY_NONE, // 0xF1
		oGUI_KEY_NONE, // 0xF2
		oGUI_KEY_NONE, // 0xF3
		oGUI_KEY_NONE, // 0xF4
		oGUI_KEY_NONE, // 0xF5
		oGUI_KEY_NONE, // VK_ATTN
		oGUI_KEY_NONE, // VK_CRSEL
		oGUI_KEY_NONE, // VK_EXSEL
		oGUI_KEY_NONE, // VK_EREOF
		oGUI_KEY_NONE, // VK_PLAY
		oGUI_KEY_NONE, // VK_ZOOM
		oGUI_KEY_NONE, // VK_NONAME
		oGUI_KEY_NONE, // VK_PA1
		oGUI_KEY_NONE, // VK_OEM_CLEAR
		oGUI_KEY_NONE, // 0xFF
	};
	static const unsigned short MAX_NUM_VK_CODES = 256;
	static_assert(oCOUNTOF(sKeys) == MAX_NUM_VK_CODES, "array mismatch");
	
	if (sKeys[_vkCode] == oGUI_KEY_NONE)
		oTRACE("No mapping for vkcode = %x", _vkCode);
	
	return (oGUI_KEY)sKeys[_vkCode];
}

DWORD oWinKeyFromKey(oGUI_KEY _Key)
{
	static const unsigned char sVKCodes[] = 
	{
		0,

		// Mouse keys
		VK_LBUTTON,
		VK_RBUTTON,
		VK_MBUTTON,
		VK_XBUTTON1,
		VK_XBUTTON2,

		VK_LBUTTON,
		VK_RBUTTON,
		VK_MBUTTON,
		VK_XBUTTON1,
		VK_XBUTTON2,

		// Joystick keys
		0, 0, 0, 0,
		0, 0, 0, 0,
		0, 0, 0, 0,
		0, 0, 0, 0, 0,

		// Control keys
		VK_LCONTROL, VK_RCONTROL,
		VK_LMENU, VK_RMENU,
		VK_LSHIFT, VK_RSHIFT,
		VK_LWIN, VK_RWIN,
		0, VK_APPS,

		// Toggle keys
		VK_CAPITAL,
		VK_SCROLL,
		VK_NUMLOCK,

		// Typing keys
		VK_SPACE, VK_OEM_3, VK_OEM_MINUS, VK_OEM_PLUS, VK_OEM_4, VK_OEM_5, VK_OEM_6, VK_OEM_1, VK_OEM_7, VK_OEM_COMMA, VK_OEM_PERIOD, VK_OEM_2,
		'0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
		'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',

		// Numpad keys
		VK_NUMPAD0, VK_NUMPAD1, VK_NUMPAD2, VK_NUMPAD3, VK_NUMPAD4, VK_NUMPAD5, VK_NUMPAD6, VK_NUMPAD7, VK_NUMPAD8, VK_NUMPAD9,
		VK_MULTIPLY, VK_ADD, VK_SUBTRACT, VK_DECIMAL, VK_DIVIDE,

		// Typing control keys
		VK_ESCAPE,
		VK_BACK,
		VK_TAB,
		VK_RETURN,
		VK_INSERT, VK_DELETE,
		VK_HOME, VK_END,
		VK_PRIOR, VK_NEXT,

		// System control keys
		VK_F1, VK_F2, VK_F3, VK_F4, VK_F5, VK_F6, VK_F7, VK_F8, VK_F9, VK_F10, VK_F11, VK_F12, VK_F13, VK_F14, VK_F15, VK_F16, VK_F17, VK_F18, VK_F19, VK_F20, VK_F21, VK_F22, VK_F23, VK_F24,
		VK_PAUSE,
		VK_SLEEP,
		VK_SNAPSHOT,

		// Directional keys
		VK_LEFT, VK_UP, VK_RIGHT, VK_DOWN,

		// Browser keys
		VK_LAUNCH_MAIL,
		VK_BROWSER_BACK, 
		VK_BROWSER_FORWARD, 
		VK_BROWSER_REFRESH, 
		VK_BROWSER_STOP, 
		VK_BROWSER_SEARCH, 
		VK_BROWSER_FAVORITES,

		// Media keys
		VK_LAUNCH_MEDIA_SELECT, 
		VK_VOLUME_MUTE, 
		VK_VOLUME_UP, 
		VK_VOLUME_DOWN, 
		VK_MEDIA_PREV_TRACK, 
		VK_MEDIA_NEXT_TRACK, 
		VK_MEDIA_STOP, 
		VK_MEDIA_PLAY_PAUSE,

		// Misc keys
		VK_LAUNCH_APP1, VK_LAUNCH_APP2,

		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	};
	static_assert(oCOUNTOF(sVKCodes) == oGUI_KEY_COUNT, "array mismatch");
	return sVKCodes[_Key];
}

static float3 oWinGetMousePosition(LPARAM _lParam, WPARAM _wParam)
{
	return float3((float)GET_X_LPARAM(_lParam), (float)GET_Y_LPARAM(_lParam), (float)GET_WHEEL_DELTA_WPARAM(_wParam));
}

// X11 does not support multiple mouse buttons down at time of drag, so create a 
// priority system (left, right, middle, back, front) for the wParam mask 
// specified in WM_MOUSEMOVE and WM_MOUSEWHEEL events. oKB_VoidSymbol is 
// returned if the specified wParam is not a valid Keys mask.
static oGUI_KEY oWinKeyMouseMoveGetTopPriorityKey(WPARAM _wParam)
{
	WPARAM Keys = GET_KEYSTATE_WPARAM(_wParam);
	if (Keys & MK_LBUTTON) return oGUI_KEY_MOUSE_LEFT;
	if (Keys & MK_RBUTTON) return oGUI_KEY_MOUSE_RIGHT;
	if (Keys & MK_MBUTTON) return oGUI_KEY_MOUSE_MIDDLE;
	if (Keys & MK_XBUTTON1) return oGUI_KEY_MOUSE_SIDE1;
	if (Keys & MK_XBUTTON2) return oGUI_KEY_MOUSE_SIDE2;
	return oGUI_KEY_NONE;
}

bool oWinKeyDispatchMessage(HWND _hWnd, UINT _uMsg, WPARAM _wParam, LPARAM _lParam, unsigned int _TimestampMS, oWINKEY_CONTROL_STATE* _pState, oGUI_ACTION_DESC* _pAction)
{
	// NOTE: Only keep simple down/up functionality here. Anything more advanced
	// should not be handled here and handled explicitly in a place where there's
	// more context for the handling.

	bool handled = true;

	#define GETPOS _pAction->Position = float4(oWinGetMousePosition(_lParam, 0), 0.0f);
	#define ISKEYUP _pAction->Action = oGUI_ACTION_KEY_UP

	_pAction->Action = oGUI_ACTION_KEY_DOWN;
	_pAction->DeviceID = 0;
	_pAction->Key = oGUI_KEY_NONE;
	_pAction->Position = 0.0f;
	_pAction->hWindow = (oGUI_WINDOW)_hWnd;
	_pAction->TimestampMS = _TimestampMS;
	_pAction->ActionCode = oInvalid;

	switch (_uMsg)
	{
		case WM_LBUTTONUP: ISKEYUP; case WM_LBUTTONDOWN: GETPOS; _pAction->Key = oGUI_KEY_MOUSE_LEFT; break;
		case WM_RBUTTONUP: ISKEYUP; case WM_RBUTTONDOWN: GETPOS; _pAction->Key = oGUI_KEY_MOUSE_RIGHT; break;
		case WM_MBUTTONUP: ISKEYUP; case WM_MBUTTONDOWN: GETPOS; _pAction->Key = oGUI_KEY_MOUSE_MIDDLE; break;
		case WM_XBUTTONUP: ISKEYUP; case WM_XBUTTONDOWN: GETPOS; _pAction->Key = GET_XBUTTON_WPARAM(_wParam) == XBUTTON1 ? oGUI_KEY_MOUSE_SIDE1 : oGUI_KEY_MOUSE_SIDE2; break;

		case WM_LBUTTONDBLCLK: GETPOS; _pAction->Key = oGUI_KEY_MOUSE_LEFT_DOUBLE; break;
		case WM_RBUTTONDBLCLK: GETPOS; _pAction->Key = oGUI_KEY_MOUSE_RIGHT_DOUBLE; break;
		case WM_MBUTTONDBLCLK: GETPOS; _pAction->Key = oGUI_KEY_MOUSE_MIDDLE_DOUBLE; break;
		case WM_XBUTTONDBLCLK: GETPOS; _pAction->Key = GET_XBUTTON_WPARAM(_wParam) == XBUTTON1 ? oGUI_KEY_MOUSE_SIDE1_DOUBLE : oGUI_KEY_MOUSE_SIDE2_DOUBLE; break;

		case WM_KEYUP: ISKEYUP; case WM_KEYDOWN: _pAction->Key = oWinKeyToKey((DWORD)oWinKeyTranslate(_wParam, _pState)); break;

		// Pass ALT buttons through to regular key handling
		case WM_SYSKEYUP: ISKEYUP; case WM_SYSKEYDOWN: { switch (_wParam) { case VK_LMENU: case VK_MENU: case VK_RMENU: _pAction->Key = oWinKeyToKey((DWORD)oWinKeyTranslate(_wParam, _pState)); break; default: handled = false; break; } break; }

		case WM_MOUSEMOVE: _pAction->Action = oGUI_ACTION_POINTER_MOVE; _pAction->Key = oWinKeyMouseMoveGetTopPriorityKey(_wParam); GETPOS; break;
		case WM_MOUSEWHEEL: _pAction->Action = oGUI_ACTION_POINTER_MOVE; _pAction->Key = oWinKeyMouseMoveGetTopPriorityKey(_wParam); _pAction->Position = float4(oWinGetMousePosition(_lParam, _wParam), 0.0f); break;

		default: handled = false; break;
	}

	if (handled)
		_pAction->DeviceType = oGUIDeviceFromKey(_pAction->Key);

	return handled;
}

bool oWinKeyIsExtended(DWORD _vkCode)
{
	switch (_vkCode)
	{
		case VK_RMENU:
		case VK_RCONTROL:
		case VK_INSERT:
		case VK_DELETE:
		case VK_HOME:
		case VK_END:
		case VK_NEXT:
		case VK_PRIOR:
		case VK_LEFT:
		case VK_UP:
		case VK_RIGHT:
		case VK_DOWN:
		case VK_DIVIDE:
		//case VK_NUMPAD_ENTER:
			return true;
		default:
			break;
	}
	return false;
}

bool oWinKeyIsExtended(oGUI_KEY _Key)
{
	return oWinKeyIsExtended(oWinKeyFromKey(_Key));
}

bool oWinKeyIsShortCircuited(DWORD _vkCode)
{
	switch (_vkCode)
	{
		case VK_VOLUME_MUTE:
		case VK_VOLUME_DOWN:
		case VK_VOLUME_UP:
		case VK_MEDIA_NEXT_TRACK:
		case VK_MEDIA_PREV_TRACK:
		case VK_MEDIA_STOP:
		case VK_MEDIA_PLAY_PAUSE:
		case VK_LAUNCH_MAIL:
		case VK_LAUNCH_MEDIA_SELECT:
		case VK_LAUNCH_APP1:
		case VK_LAUNCH_APP2:
			return true;
		default:
			break;
	}

	return false;
}

LPARAM oWinKeyToLParam(const KBDLLHOOKSTRUCT& _KB, unsigned short _RepeatCount, bool _PrevStateDown)
{
	// Ignore repeat count, the info isn't availabe at this level and at higher 
	// levels the message gets eaten.
	const int Repeat = 0;
	const bool IsExtended = !!(_KB.flags & 1);
	const bool IsDown = !(_KB.flags & (1<<7));
	return (LPARAM)(_RepeatCount | ((_KB.scanCode & 0xff) << 16) | (IsExtended ? (1<<24) : 0) | ((_PrevStateDown ? 1 : 0) << 30) | ((IsDown ? 0 : 1) << 31));
}

#if 0

static LPARAM oWinKeytoKeyLParam(DWORD _vkCode, bool _IsDown)
{
	LPARAM lParam = 0;
	if (_IsDown)
		lParam |= 1; // 1 repeat count

	if (oWinKeyIsExtended(_vkCode))
		lParam |= (1<<24);

	if (!_IsDown)
		lParam |= (3<<30);

	return lParam;
}

static UINT oWinKeyToMouseMsg(DWORD _vkCodeMouse, bool _IsDown)
{
	switch (_vkCodeMouse)
	{
	case VK_LBUTTON: return _IsDown ? WM_LBUTTONDOWN : WM_LBUTTONUP;
	case VK_RBUTTON: return _IsDown ? WM_RBUTTONDOWN : WM_RBUTTONUP;
	case VK_MBUTTON: return _IsDown ? WM_MBUTTONDOWN : WM_MBUTTONUP;
	case VK_XBUTTON1: case VK_XBUTTON2: return _IsDown ? WM_XBUTTONDOWN : WM_XBUTTONUP;
	default: break;
	}
	return WM_NULL;
}

static WPARAM oWinKeyToMouseWParam(oGUI_KEY _MouseKey, bool _IsDown)
{
	if (_IsDown)
	{
		switch (_MouseKey)
		{
		case oGUI_KEY_MOUSE_LEFT: return MK_LBUTTON;
		case oGUI_KEY_MOUSE_RIGHT: return MK_RBUTTON;
		case oGUI_KEY_MOUSE_MIDDLE: return MK_MBUTTON;
		case oGUI_KEY_MOUSE_SIDE1: return MAKEWPARAM(MK_XBUTTON1, XBUTTON1);
		case oGUI_KEY_MOUSE_SIDE2: return MAKEWPARAM(MK_XBUTTON2, XBUTTON2);
		default: break;
		}
	}
	return 0;
}
static short2 oWinCalcMousePosition(HWND _hWnd, const int2& _MousePosition)
{
	WINDOWPLACEMENT wp;	
	GetWindowPlacement(_hWnd, &wp);	
	unsigned short X = (unsigned short)(65535 * (_MousePosition.x + wp.rcNormalPosition.left) / GetSystemMetrics(SM_CXSCREEN));
	unsigned short Y = (unsigned short)(65535 * (_MousePosition.y + wp.rcNormalPosition.top) / GetSystemMetrics(SM_CYSCREEN));
	return short2(*(short*)&X, *(short*)&Y);
}

static LPARAM oWinCalcMousePosition(HWND _hWnd, const int2& _MousePosition)
{
	WINDOWPLACEMENT wp;	
	GetWindowPlacement(_hWnd, &wp);	
	unsigned short X = (unsigned short)(65535 * (_MousePosition.x + wp.rcNormalPosition.left) / GetSystemMetrics(SM_CXSCREEN));
	unsigned short Y = (unsigned short)(65535 * (_MousePosition.y + wp.rcNormalPosition.top) / GetSystemMetrics(SM_CYSCREEN));
	return MAKELPARAM(X,Y);
}

void oWinKeySend(HWND _hWnd, oGUI_KEY _Key, bool _IsDown, const int2& _MousePosition)
{
	oGUI_INPUT_DEVICE_TYPE Dev = oGUIDeviceFromKey(_Key);

	UINT msg = WM_NULL;
	WPARAM wParam = 0;
	LPARAM lParam = 0;

	switch (Dev)
	{
		case oGUI_INPUT_DEVICE_KEYBOARD:
		{
			msg = _IsDown ? WM_KEYDOWN : WM_KEYUP;
			wParam = oWinKeyFromKey(_Key);
			lParam = oWinKeytoKeyLParam((DWORD)wParam, _IsDown);
			break;
		}

		case oGUI_INPUT_DEVICE_MOUSE:
		{
			msg = oWinKeyToMouseMsg(_Key, _IsDown);
			wParam = oWinKeyToMouseWParam(_Key, _IsDown);
			lParam = oWinCalcMousePosition(_hWnd, _MousePosition);
		}

		default:
			break;
	}

	if (msg != WM_NULL)
		PostMessage(_hWnd, msg, wParam, lParam);
}

void oWinKeySend(HWND _hWnd, const oGUI_KEY* _pKeys, size_t _NumKeys)
{
	for (size_t i = 0; i < _NumKeys; i++)
	{
		oWinKeySend(_hWnd, _pKeys[i], true);
		oWinKeySend(_hWnd, _pKeys[i], false);
	}
}

void oWinKeySendMouse(HWND _hWnd, const int2& _Position)
{
	PostMessage(_hWnd, WM_MOUSEMOVE, 0, oWinCalcMousePosition(_hWnd, _Position));
}

void oWinKeySend(HWND _hWnd, const char* _String)
{
	size_t len = strlen(_String);
	if (len)
	{
		for (size_t i = 0; i < len; i++)
		{
			DWORD vkCode = VkKeyScanEx(_String[i], GetKeyboardLayout(0));
			LPARAM lParamDN = oWinKeytoKeyLParam(vkCode, true);
			LPARAM lParamUP = oWinKeytoKeyLParam(vkCode, false);
			PostMessage(_hWnd, WM_KEYDOWN, vkCode, lParamDN);
			PostMessage(_hWnd, WM_KEYUP, vkCode, lParamUP);
		}
	}
}


#else
#include <oPlatform/Windows/oWinWindowing.h>
void AppendKey(short int _Key, bool _KeyUp, INPUT** _ppInput)
{
	auto& pInput = *_ppInput;

	pInput->type = INPUT_KEYBOARD;

	pInput->ki.wVk = _Key;
	pInput->ki.time = 0;
	pInput->ki.dwFlags = _KeyUp ? KEYEVENTF_KEYUP : 0;
	pInput->ki.dwExtraInfo = GetMessageExtraInfo();
	++pInput;
}

void oWinKeySend(HWND _hWnd, oGUI_KEY _Key, bool _IsDown, const int2& _MousePosition)
{
	INPUT Input;

	const DWORD TID = oConcurrency::asuint(oWinGetWindowThread(_hWnd));

	AttachThreadInput(GetCurrentThreadId(), TID, true);
	
	switch (oGUIDeviceFromKey(_Key))
	{
		case oGUI_INPUT_DEVICE_KEYBOARD:
		{
			Input.type = INPUT_KEYBOARD;
			Input.ki.wVk = (WORD)oWinKeyFromKey(_Key);
			Input.ki.time = 0;
			Input.ki.dwFlags = _IsDown ? 0 : KEYEVENTF_KEYUP;
			Input.ki.dwExtraInfo = GetMessageExtraInfo();
			SendInput(1, &Input, sizeof(INPUT));
			break;
		}

		case oGUI_INPUT_DEVICE_MOUSE:
		{
			WINDOWPLACEMENT wp;	
			GetWindowPlacement(_hWnd, &wp);	
			Input.type = INPUT_MOUSE;
			memset(&Input.mi, 0, sizeof(Input.mi));
			Input.mi.mouseData = 0;
			Input.mi.dx = ( 65535 * ((long)_MousePosition.x + wp.rcNormalPosition.left)) / GetSystemMetrics(SM_CXSCREEN);
			Input.mi.dy = ( 65535 * ((long)_MousePosition.y + wp.rcNormalPosition.top)) / GetSystemMetrics(SM_CYSCREEN);
			Input.ki.dwExtraInfo = GetMessageExtraInfo();
			unsigned int mouseDown, mouseUp;

			switch (_Key)
			{
				case oGUI_KEY_MOUSE_RIGHT:
					mouseDown = MOUSEEVENTF_RIGHTDOWN;
					mouseUp = MOUSEEVENTF_RIGHTUP;
					break;
				case oGUI_KEY_MOUSE_MIDDLE:
					mouseDown = MOUSEEVENTF_MIDDLEDOWN;
					mouseUp = MOUSEEVENTF_MIDDLEUP;
					break;
				case oGUI_KEY_MOUSE_LEFT:
					mouseDown = MOUSEEVENTF_LEFTDOWN;
					mouseUp = MOUSEEVENTF_LEFTUP;
					break;
				oNODEFAULT;
			}

			Input.mi.dwFlags = MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_MOVE | (_IsDown ? mouseDown : mouseUp);
			SendInput(1, &Input, sizeof(INPUT));
			break;
		}

		oNODEFAULT;
	}
	
	AttachThreadInput(GetCurrentThreadId(), TID, false);
}

bool oWinSendKeys(HWND _hWnd, unsigned int _ThreadID, short int* _pVKeys, int _NumberKeys)
{
	static const int MAX_KEYS = 64;
	INPUT Input[(MAX_KEYS + 2 ) * 4]; // We need to ensure we have twice as many keys so we can send both an up and down command as well as adjust caps

	if(_NumberKeys > MAX_KEYS)
		return oErrorSetLast(std::errc::no_buffer_space, "Only support %d keys", MAX_KEYS);

	AttachThreadInput(GetCurrentThreadId(), _ThreadID, true);

	INPUT* pKeyHead = Input;
	bool CapsLockWasOn = GetKeyState(VK_CAPITAL) == 0 ? false : true;
	if(CapsLockWasOn)
	{
		AppendKey(VK_CAPITAL, false, &pKeyHead);
		AppendKey(VK_CAPITAL, true, &pKeyHead);
	}

	for(int i = 0; i < _NumberKeys; ++i)
	{
		short int Key = _pVKeys[i];

		bool ShouldShift = (0x0100 & Key) > 0;

		if(ShouldShift)
			AppendKey(VK_SHIFT, false, &pKeyHead);

		AppendKey(Key, false, &pKeyHead);
		AppendKey(Key, true, &pKeyHead);

		if(ShouldShift)
			AppendKey(VK_SHIFT, true, &pKeyHead);
	}
	if(CapsLockWasOn)
	{
		AppendKey(VK_CAPITAL, false, &pKeyHead);
		AppendKey(VK_CAPITAL, true, &pKeyHead);
	}

	oWinSetFocus(_hWnd);

	int NumberOfKeys = oInt(pKeyHead - Input);
	SendInput(NumberOfKeys, Input, sizeof(INPUT));

	AttachThreadInput(GetCurrentThreadId(), _ThreadID, false);
	return true;
}

bool oWinSendASCIIMessage(HWND _hWnd, unsigned int _ThreadID, const char* _pMessage)
{
	short int VirtualKeys[64];
	int MessageLength = oInt(strlen(_pMessage));
	if(MessageLength > oCOUNTOF(VirtualKeys))
		return oErrorSetLast(std::errc::no_buffer_space, "Only support %d length messages", oCOUNTOF(VirtualKeys));

	oFORI(i, VirtualKeys)
	{
		VirtualKeys[i] = VkKeyScanEx(_pMessage[i], GetKeyboardLayout(0));
	}

	return oWinSendKeys(_hWnd, _ThreadID, VirtualKeys, MessageLength);
}



#endif