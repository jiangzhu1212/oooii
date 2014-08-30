/**************************************************************************
 * The MIT License                                                        *
 * Copyright (c) 2014 Antony Arciuolo.                                    *
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
#include <oGUI/Windows/oWinKey.h>

#undef interface
#include <oCore/windows/win_util.h>
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

ouro::input::key oWinKeyToKey(DWORD _vkCode)
{
	static unsigned char sKeys[] =
	{
		0,
		ouro::input::mouse_left,
		ouro::input::mouse_right,
		VK_CANCEL,
		ouro::input::mouse_middle,
		ouro::input::mouse_side1,
		ouro::input::mouse_side2,
		ouro::input::none, // 0x07
		ouro::input::backspace,
		ouro::input::tab,
		ouro::input::none, // 0x0A
		ouro::input::none, // 0x0B
		ouro::input::none, // VK_CLEAR
		ouro::input::enter,
		ouro::input::none, // 0x0E
		ouro::input::none, // 0x0F
		ouro::input::lshift, // VK_SHIFT
		ouro::input::lctrl, // VK_CONTROL
		ouro::input::lalt, // VK_MENU
		ouro::input::pause,
		ouro::input::capslock,
		ouro::input::none, // VK_KANA
		ouro::input::none, //VK_HANGUEL/VK_HANGUL
		ouro::input::none, // 0x16
		ouro::input::none, // VK_KANA
		ouro::input::none, // VK_HANJA/VK_KANJI
		ouro::input::none, // 0x1A
		ouro::input::esc,
		ouro::input::none, // VK_CONVERT
		ouro::input::none, // VK_NONCONVERT
		ouro::input::none, // VK_ACCEPT
		ouro::input::none, // VK_MODECHANGE
		ouro::input::space,
		ouro::input::pgup,
		ouro::input::pgdn,
		ouro::input::end,
		ouro::input::home,
		ouro::input::left,
		ouro::input::up,
		ouro::input::right,
		ouro::input::down,
		ouro::input::none, // VK_SELECT
		ouro::input::none, // VK_PRINT
		ouro::input::none, // VK_EXECUTE
		ouro::input::printscreen, // VK_SNAPSHOT
		ouro::input::ins,
		ouro::input::del,
		ouro::input::none, // VK_HELP
		ouro::input::_0,
		ouro::input::_1,
		ouro::input::_2,
		ouro::input::_3,
		ouro::input::_4,
		ouro::input::_5,
		ouro::input::_6,
		ouro::input::_7,
		ouro::input::_8,
		ouro::input::_9,
		ouro::input::none, // 0x3A
		ouro::input::none, // 0x3B
		ouro::input::none, // 0x3C
		ouro::input::none, // 0x3D
		ouro::input::none, // 0x3E
		ouro::input::none, // 0x3F
		ouro::input::none, // 0x40
		ouro::input::a,
		ouro::input::b,
		ouro::input::c,
		ouro::input::d,
		ouro::input::e,
		ouro::input::f,
		ouro::input::g,
		ouro::input::h,
		ouro::input::i,
		ouro::input::j,
		ouro::input::k,
		ouro::input::l,
		ouro::input::m,
		ouro::input::n,
		ouro::input::o,
		ouro::input::p,
		ouro::input::q,
		ouro::input::r,
		ouro::input::s,
		ouro::input::t,
		ouro::input::u,
		ouro::input::v,
		ouro::input::w,
		ouro::input::x,
		ouro::input::y,
		ouro::input::z,
		ouro::input::lwin,
		ouro::input::rwin,
		ouro::input::app_context,
		ouro::input::none, // 0x5E
		ouro::input::sleep,
		ouro::input::num0,
		ouro::input::num1,
		ouro::input::num2,
		ouro::input::num3,
		ouro::input::num4,
		ouro::input::num5,
		ouro::input::num6,
		ouro::input::num7,
		ouro::input::num8,
		ouro::input::num9,
		ouro::input::nummul,
		ouro::input::numadd,
		ouro::input::none, // VK_SEPARATOR
		ouro::input::numsub,
		ouro::input::numdecimal,
		ouro::input::numdiv,
		ouro::input::f1,
		ouro::input::f2,
		ouro::input::f3,
		ouro::input::f4,
		ouro::input::f5,
		ouro::input::f6,
		ouro::input::f7,
		ouro::input::f8,
		ouro::input::f9,
		ouro::input::f10,
		ouro::input::f11,
		ouro::input::f12,
		ouro::input::f13,
		ouro::input::f14,
		ouro::input::f15,
		ouro::input::f16,
		ouro::input::f17,
		ouro::input::f18,
		ouro::input::f19,
		ouro::input::f20,
		ouro::input::f21,
		ouro::input::f22,
		ouro::input::f23,
		ouro::input::f24,
		ouro::input::none, // 0x88
		ouro::input::none, // 0x89
		ouro::input::none, // 0x8A
		ouro::input::none, // 0x8B
		ouro::input::none, // 0x8C
		ouro::input::none, // 0x8D
		ouro::input::none, // 0x8E
		ouro::input::none, // 0x8F
		ouro::input::numlock,
		ouro::input::scrolllock,
		ouro::input::none, // 0x92
		ouro::input::none, // 0x93
		ouro::input::none, // 0x94
		ouro::input::none, // 0x95
		ouro::input::none, // 0x96
		ouro::input::none, // 0x97
		ouro::input::none, // 0x98
		ouro::input::none, // 0x99
		ouro::input::none, // 0x9A
		ouro::input::none, // 0x9B
		ouro::input::none, // 0x9C
		ouro::input::none, // 0x9D
		ouro::input::none, // 0x9E
		ouro::input::none, // 0x9F
		ouro::input::lshift,
		ouro::input::rshift,
		ouro::input::lctrl,
		ouro::input::rctrl,
		ouro::input::lalt,
		ouro::input::ralt,
		ouro::input::back,
		ouro::input::forward,
		ouro::input::refresh,
		ouro::input::stop,
		ouro::input::search,
		ouro::input::favs,
		ouro::input::home,
		ouro::input::mute,
		ouro::input::voldn,
		ouro::input::volup,
		ouro::input::next_track,
		ouro::input::prev_track,
		ouro::input::stop_track,
		ouro::input::play_pause_track,
		ouro::input::mail,
		ouro::input::media,
		ouro::input::app1,
		ouro::input::app2,
		ouro::input::none, // 0xB8
		ouro::input::none, // 0xB9
		ouro::input::semicolon,
		ouro::input::equal_,
		ouro::input::comma,
		ouro::input::dash,
		ouro::input::period,
		ouro::input::slash,
		ouro::input::backtick,

		ouro::input::none, // 0xC1
		ouro::input::none, // 0xC2
		ouro::input::none, // 0xC3
		ouro::input::none, // 0xC4
		ouro::input::none, // 0xC5
		ouro::input::none, // 0xC6
		ouro::input::none, // 0xC7
		ouro::input::none, // 0xC8
		ouro::input::none, // 0xC9
		ouro::input::none, // 0xCA
		ouro::input::none, // 0xCB
		ouro::input::none, // 0xCC
		ouro::input::none, // 0xCD
		ouro::input::none, // 0xCE
		ouro::input::none, // 0xCF

		ouro::input::none, // 0xD0
		ouro::input::none, // 0xD1
		ouro::input::none, // 0xD2
		ouro::input::none, // 0xD3
		ouro::input::none, // 0xD4
		ouro::input::none, // 0xD5
		ouro::input::none, // 0xD6
		ouro::input::none, // 0xD7
		ouro::input::none, // 0xD8
		ouro::input::none, // 0xD9
		ouro::input::none, // 0xDA
		ouro::input::lbracket,
		ouro::input::backslash,
		ouro::input::rbracket,
		ouro::input::apostrophe,
		ouro::input::none, // VK_OEM_8
		ouro::input::none, // 0xE0
		ouro::input::none, // 0xE1
		ouro::input::none, // VK_OEM_102
		ouro::input::none, // 0xE3
		ouro::input::none, // 0xE4
		ouro::input::none, // VK_PROCESSKEY
		ouro::input::none, // 0xE6
		ouro::input::none, // VK_PACKET
		ouro::input::none, // 0xE8
		ouro::input::none, // 0xE9
		ouro::input::none, // 0xEA
		ouro::input::none, // 0xEB
		ouro::input::none, // 0xEC
		ouro::input::none, // 0xED
		ouro::input::none, // 0xEE
		ouro::input::none, // 0xEF
		ouro::input::none, // 0xF0
		ouro::input::none, // 0xF1
		ouro::input::none, // 0xF2
		ouro::input::none, // 0xF3
		ouro::input::none, // 0xF4
		ouro::input::none, // 0xF5
		ouro::input::none, // VK_ATTN
		ouro::input::none, // VK_CRSEL
		ouro::input::none, // VK_EXSEL
		ouro::input::none, // VK_EREOF
		ouro::input::none, // VK_PLAY
		ouro::input::none, // VK_ZOOM
		ouro::input::none, // VK_NONAME
		ouro::input::none, // VK_PA1
		ouro::input::none, // VK_OEM_CLEAR
		ouro::input::none, // 0xFF
	};
	static const unsigned short MAX_NUM_VK_CODES = 256;
	static_assert(oCOUNTOF(sKeys) == MAX_NUM_VK_CODES, "array mismatch");
	
	if (sKeys[_vkCode] == ouro::input::none)
		oTRACE("No mapping for vkcode = %x", _vkCode);
	
	return (ouro::input::key)sKeys[_vkCode];
}

DWORD oWinKeyFromKey(ouro::input::key _Key)
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
	static_assert(oCOUNTOF(sVKCodes) == ouro::input::key::count, "array mismatch");
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
static ouro::input::key oWinKeyMouseMoveGetTopPriorityKey(WPARAM _wParam)
{
	WPARAM Keys = GET_KEYSTATE_WPARAM(_wParam);
	if (Keys & MK_LBUTTON) return ouro::input::mouse_left;
	if (Keys & MK_RBUTTON) return ouro::input::mouse_right;
	if (Keys & MK_MBUTTON) return ouro::input::mouse_middle;
	if (Keys & MK_XBUTTON1) return ouro::input::mouse_side1;
	if (Keys & MK_XBUTTON2) return ouro::input::mouse_side2;
	return ouro::input::none;
}

bool oWinKeyDispatchMessage(HWND _hWnd, UINT _uMsg, WPARAM _wParam, LPARAM _lParam, unsigned int _TimestampMS, oWINKEY_CONTROL_STATE* _pState, ouro::input::action* _pAction)
{
	// NOTE: Only keep simple down/up functionality here. Anything more advanced
	// should not be handled here and handled explicitly in a place where there's
	// more context for the handling.

	bool handled = true;

	#define GETPOS _pAction->position(float4(oWinGetMousePosition(_lParam, 0), 0.0f));
	#define ISKEYUP _pAction->action_type = ouro::input::action_type::key_up

	_pAction->action_type = ouro::input::action_type::key_down;
	_pAction->device_id = 0;
	_pAction->key = ouro::input::key::none;
	_pAction->position(0.0f);
	_pAction->window = _hWnd;
	_pAction->timestamp_ms = _TimestampMS;
	_pAction->action_code = ouro::invalid;

	switch (_uMsg)
	{
		case WM_LBUTTONUP: ISKEYUP; case WM_LBUTTONDOWN: GETPOS; _pAction->key = ouro::input::key::mouse_left; break;
		case WM_RBUTTONUP: ISKEYUP; case WM_RBUTTONDOWN: GETPOS; _pAction->key = ouro::input::key::mouse_right; break;
		case WM_MBUTTONUP: ISKEYUP; case WM_MBUTTONDOWN: GETPOS; _pAction->key = ouro::input::key::mouse_middle; break;
		case WM_XBUTTONUP: ISKEYUP; case WM_XBUTTONDOWN: GETPOS; _pAction->key = GET_XBUTTON_WPARAM(_wParam) == XBUTTON1 ? ouro::input::mouse_side1 : ouro::input::mouse_side2; break;

		case WM_LBUTTONDBLCLK: GETPOS; _pAction->key = ouro::input::key::mouse_left_double; break;
		case WM_RBUTTONDBLCLK: GETPOS; _pAction->key = ouro::input::key::mouse_right_double; break;
		case WM_MBUTTONDBLCLK: GETPOS; _pAction->key = ouro::input::key::mouse_middle_double; break;
		case WM_XBUTTONDBLCLK: GETPOS; _pAction->key = GET_XBUTTON_WPARAM(_wParam) == XBUTTON1 ? ouro::input::key::mouse_side1_double : ouro::input::key::mouse_side2_double; break;

		case WM_KEYUP: ISKEYUP; case WM_KEYDOWN: _pAction->key = oWinKeyToKey((DWORD)oWinKeyTranslate(_wParam, _pState)); break;

		// Pass ALT buttons through to regular key handling
		case WM_SYSKEYUP: ISKEYUP; case WM_SYSKEYDOWN: { switch (_wParam) { case VK_LMENU: case VK_MENU: case VK_RMENU: _pAction->key = oWinKeyToKey((DWORD)oWinKeyTranslate(_wParam, _pState)); break; default: handled = false; break; } break; }

		case WM_MOUSEMOVE: _pAction->action_type = ouro::input::action_type::pointer_move; _pAction->key = oWinKeyMouseMoveGetTopPriorityKey(_wParam); GETPOS; break;
		case WM_MOUSEWHEEL: _pAction->action_type = ouro::input::action_type::pointer_move; _pAction->key = oWinKeyMouseMoveGetTopPriorityKey(_wParam); _pAction->position(float4(oWinGetMousePosition(_lParam, _wParam), 0.0f)); break;

		default: handled = false; break;
	}

	if (handled)
		_pAction->device_type = ouro::input::get_type(_pAction->key);

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

bool oWinKeyIsExtended(ouro::input::key _Key)
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

#include <oGUI/Windows/oWinWindowing.h>
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

void oWinKeySend(HWND _hWnd, ouro::input::key _Key, bool _IsDown, const int2& _MousePosition)
{
	INPUT Input;
	std::thread::id tid = oWinGetWindowThread(_hWnd);
	AttachThreadInput(GetCurrentThreadId(), asdword(tid), true);
	
	switch (ouro::input::get_type(_Key))
	{
		case ouro::input::type::keyboard:
		{
			Input.type = INPUT_KEYBOARD;
			Input.ki.wVk = (WORD)oWinKeyFromKey(_Key);
			Input.ki.time = 0;
			Input.ki.dwFlags = _IsDown ? 0 : KEYEVENTF_KEYUP;
			Input.ki.dwExtraInfo = GetMessageExtraInfo();
			SendInput(1, &Input, sizeof(INPUT));
			break;
		}

		case ouro::input::type::mouse:
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
				case ouro::input::mouse_right:
					mouseDown = MOUSEEVENTF_RIGHTDOWN;
					mouseUp = MOUSEEVENTF_RIGHTUP;
					break;
				case ouro::input::mouse_middle:
					mouseDown = MOUSEEVENTF_MIDDLEDOWN;
					mouseUp = MOUSEEVENTF_MIDDLEUP;
					break;
				case ouro::input::mouse_left:
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
	
	AttachThreadInput(GetCurrentThreadId(), asdword(tid), false);
}

void oWinSendKeys(HWND _hWnd, unsigned int _ThreadID, short int* _pVKeys, int _NumberOfKeys)
{
	static const int MAX_KEYS = 64;
	
	// Ensure twice as many keys so both an up and down command as well as 
	// adjust caps can be sent.
	INPUT Input[(MAX_KEYS + 2) * 4];

	if (_NumberOfKeys > MAX_KEYS)
		oTHROW(no_buffer_space, "Only support %d keys", MAX_KEYS);

	AttachThreadInput(GetCurrentThreadId(), _ThreadID, true);

	INPUT* pKeyHead = Input;
	bool CapsLockWasOn = GetKeyState(VK_CAPITAL) == 0 ? false : true;
	if(CapsLockWasOn)
	{
		AppendKey(VK_CAPITAL, false, &pKeyHead);
		AppendKey(VK_CAPITAL, true, &pKeyHead);
	}

	for (int i = 0; i < _NumberOfKeys; i++)
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

	if (CapsLockWasOn)
	{
		AppendKey(VK_CAPITAL, false, &pKeyHead);
		AppendKey(VK_CAPITAL, true, &pKeyHead);
	}

	oWinSetFocus(_hWnd);

	int NumberOfKeys = as_int(pKeyHead - Input);
	SendInput(NumberOfKeys, Input, sizeof(INPUT));

	AttachThreadInput(GetCurrentThreadId(), _ThreadID, false);
}

void oWinSendASCIIMessage(HWND _hWnd, unsigned int _ThreadID, const char* _pMessage)
{
	short int VirtualKeys[64];
	int MessageLength = as_int(strlen(_pMessage));
	if(MessageLength > oCOUNTOF(VirtualKeys))
		oTHROW(no_buffer_space, "Only support %d length messages", oCOUNTOF(VirtualKeys));

	oFORI(i, VirtualKeys)
		VirtualKeys[i] = VkKeyScanEx(_pMessage[i], GetKeyboardLayout(0));

	oWinSendKeys(_hWnd, _ThreadID, VirtualKeys, MessageLength);
}
