/**************************************************************************
 * The MIT License                                                        *
 * Copyright (c) 2013 OOOii.                                              *
 * antony.arciuolo@oooii.com                                              *
 * kevin.myers@oooii.com                                                  *
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
#include "oVKToX11Keyboard.h"

bool TranslateMouseButtonToX11(UINT _uMsg, WPARAM _wParam, oKEYBOARD_KEY* _pKey, oGUI_ACTION* _pAction)
{
	*_pKey = oKB_VoidSymbol;
	*_pAction = oGUI_ACTION_KEY_DOWN;
	switch (_uMsg)
	{
		case WM_LBUTTONUP: *_pAction = oGUI_ACTION_KEY_UP; case WM_LBUTTONDOWN: *_pKey = oKB_Pointer_Button_Left; break;
		case WM_RBUTTONUP: *_pAction = oGUI_ACTION_KEY_UP; case WM_RBUTTONDOWN: *_pKey = oKB_Pointer_Button_Right; break;
		case WM_MBUTTONUP: *_pAction = oGUI_ACTION_KEY_UP; case WM_MBUTTONDOWN: *_pKey = oKB_Pointer_Button_Middle; break;
		case WM_XBUTTONUP: *_pAction = oGUI_ACTION_KEY_UP; case WM_XBUTTONDOWN: *_pKey = GET_XBUTTON_WPARAM(_wParam) == XBUTTON1 ? oKB_Pointer_Button3 : oKB_Pointer_Button4; break;

		case WM_LBUTTONDBLCLK: *_pKey = oKB_Pointer_DblClick_Left; break;
		case WM_RBUTTONDBLCLK: *_pKey = oKB_Pointer_DblClick_Right; break;
		case WM_MBUTTONDBLCLK: *_pKey = oKB_Pointer_DblClick_Middle; break;
		case WM_XBUTTONDBLCLK: *_pKey = GET_XBUTTON_WPARAM(_wParam) == XBUTTON1 ? oKB_Pointer_DblClick3 : oKB_Pointer_DblClick4; break;

		default: break;
	}

	return *_pKey != oKB_VoidSymbol;
}

oKEYBOARD_KEY TranslateTouchToX11(int _TouchIdx)
{
	oKEYBOARD_KEY xKey = oKB_VoidSymbol;
	if (_TouchIdx < MAX_TOUCHES)
	{
		switch (_TouchIdx)
		{
			case 0: xKey = oKB_Touch0; break;
			case 1: xKey = oKB_Touch1; break;
			case 2: xKey = oKB_Touch2; break;
			case 3: xKey = oKB_Touch3; break;
			case 4: xKey = oKB_Touch4; break;
			case 5: xKey = oKB_Touch5; break;
			case 6: xKey = oKB_Touch6; break;
			case 7: xKey = oKB_Touch7; break;
			case 8: xKey = oKB_Touch8; break;
			case 9: xKey = oKB_Touch9; break;
		}
	}

	return xKey;
}

// We cannot process or send a key up event for media keys
oKEYBOARD_KEY TranslateAppCommandToX11Key(LPARAM _lParam)
{
	oKEYBOARD_KEY xKey = oKB_VoidSymbol;
	short cmd = GET_APPCOMMAND_LPARAM(_lParam);
	for (int i = 0; i < oCOUNTOF(appKeyMap); ++i)
	{
		if (appKeyMap[i].AppCmd == (UINT)cmd)
		{
			xKey = (oKEYBOARD_KEY)appKeyMap[i].XCode;
			break;
		}
	}

	return xKey;
}
oKEYBOARD_KEY TranslateKeyToX11(WPARAM _wParam)
{
	WPARAM param = _wParam;
	// Support for left/right control, shift and alt
	switch (_wParam)
	{
		case VK_CONTROL:
			if (::GetKeyState(VK_RCONTROL) & 0x800)
				param = VK_RCONTROL;
			else
				param = VK_LCONTROL;
			break;
		case VK_MENU:
			if (::GetKeyState(VK_RMENU) & 0x800)
				param = VK_RMENU;
			else
				param = VK_LMENU;
			break;
		case VK_SHIFT:
			if (::GetKeyState(VK_RSHIFT) & 0x800)
				param = VK_RSHIFT;
			else
				param = VK_LSHIFT;
			break;
	}

	oKEYBOARD_KEY xKey = oKB_VoidSymbol;
	// First see if the key is in the map
	bool bGotKey = false;
	for (int i = 0; i < oCOUNTOF(keymap); ++i)
	{
		if (keymap[i].VKCode == param)
		{
			xKey = (oKEYBOARD_KEY)keymap[i].XCode;
			bGotKey = true;
			break;
		}
	}

	// Not in map, must be a alpha or numeric key
	if (!bGotKey)
	{
		// try converting to ascii
		BYTE keyState[256];
		char buf[4];

		// GetKeyState and GetKeyboardState return the state at the time of the message.  Use Async version of these methods
		// to get the current keyboard state.
		::GetKeyboardState(keyState);

		// if ret < 0 the returned value is a dead key
		// It is possible to return up to 2 keys, specificially if one of the keys is a dead key that cannot be composed with the specified
		// virtual key.
		int ret = ::ToAscii((UINT)param, 0, keyState, (WORD *) buf, 0);

		// TODO: German keyboard support?  AltGr = ctrl + alt: e.g. ctrl + alt + Q = @
		// TODO: multiple key support, it is possible that with dead key there is more than one key to send in this case.  In the event that
		// the dead key cannot be composed with the virtual key.
		// Do we need to send the dead key?  Do we care?  Right now I am just sending the virtual key and dropping the dead key

		// Check for only a dead key: dead keys are keys on foreign keyboards that must proceed a key press.  For example an accent mark,
		// the accent mark is pressed prior to the character it modifies.
		if (ret < 0)
		{
			switch(*buf)
			{
				case '`':
					xKey = oKB_dead_grave;
					break;
				case '\'':
					xKey = oKB_dead_acute;
					break;
				case '~':
					xKey = oKB_dead_tilde;
					break;
				case '^':
					xKey = oKB_dead_circumflex;
					break;
			}
		}
		else if (ret >= 1)
		{
			xKey = (oKEYBOARD_KEY)(*(buf + (ret - 1))); // Assign the key, ignore any dead keys
		}
	}
	return xKey;
}

UINT TranslateX11KeyboardToVK(oKEYBOARD_KEY _Key)
{
	UINT vkKey = 0;
	// First see if the key is in the map
	bool bGotKey = false;
	for (int i = 0; i < oCOUNTOF(keymap); ++i)
	{
		if (keymap[i].XCode == (UINT)_Key)
		{
			vkKey = keymap[i].VKCode;
			bGotKey = true;
			break;
		}
	}

	if (!bGotKey)
	{
		switch (_Key)
		{
			case oKB_A: case oKB_a: vkKey = 'A'; break;
			case oKB_B: case oKB_b: vkKey = 'B'; break;
			case oKB_C: case oKB_c: vkKey = 'C'; break;
			case oKB_D: case oKB_d: vkKey = 'D'; break;
			case oKB_E: case oKB_e: vkKey = 'E'; break;
			case oKB_F: case oKB_f: vkKey = 'F'; break;
			case oKB_G: case oKB_g: vkKey = 'G'; break;
			case oKB_H: case oKB_h: vkKey = 'H'; break;
			case oKB_I: case oKB_i: vkKey = 'I'; break;
			case oKB_J: case oKB_j: vkKey = 'J'; break;
			case oKB_K: case oKB_k: vkKey = 'K'; break;
			case oKB_L: case oKB_l: vkKey = 'L'; break;
			case oKB_M: case oKB_m: vkKey = 'M'; break;
			case oKB_N: case oKB_n: vkKey = 'N'; break;
			case oKB_O: case oKB_o: vkKey = 'O'; break;
			case oKB_P: case oKB_p: vkKey = 'P'; break;
			case oKB_Q: case oKB_q: vkKey = 'Q'; break;
			case oKB_R: case oKB_r: vkKey = 'R'; break;
			case oKB_S: case oKB_s: vkKey = 'S'; break;
			case oKB_T: case oKB_t: vkKey = 'T'; break;
			case oKB_U: case oKB_u: vkKey = 'U'; break;
			case oKB_V: case oKB_v: vkKey = 'V'; break;
			case oKB_W: case oKB_w: vkKey = 'W'; break;
			case oKB_X: case oKB_x: vkKey = 'X'; break;
			case oKB_Y: case oKB_y: vkKey = 'Y'; break;
			case oKB_Z: case oKB_z: vkKey = 'Z'; break;
			case oKB_bracketleft: case oKB_braceleft: vkKey = '['; break;;
			case oKB_backslash: case oKB_bar: vkKey = '\\'; break;
			case oKB_bracketright: case oKB_braceright: vkKey = ']'; break;
			case oKB_underscore: vkKey = '_'; break;;
			case oKB_grave: vkKey = '`'; break;	
			default: vkKey = 0; break;
		}
	}
	return vkKey;
}

const char* oAsString(oKEYBOARD_KEY _Key)
{
	switch (_Key)
	{
		case oKB_Touch0: return "oKB_Touch0";
		case oKB_Touch1: return "oKB_Touch1";
		case oKB_Touch2: return "oKB_Touch2";
		case oKB_Touch3: return "oKB_Touch3";
		case oKB_Touch4: return "oKB_Touch4";
		case oKB_Touch5: return "oKB_Touch5";
		case oKB_Touch6: return "oKB_Touch6";
		case oKB_Touch7: return "oKB_Touch7";
		case oKB_Touch8: return "oKB_Touch8";
		case oKB_Touch9: return "oKB_Touch9";
		// Microsoft / Windows keyboard media keys
		case oKB_Volume_Down: return "oKB_Volume_Down";
		case oKB_Volume_Mute: return "oKB_Volume_Mute";
		case oKB_Volume_Up: return "oKB_Volume_Up";
		case oKB_Media_Play_Pause: return "oKB_Media_Play_Pause";
		case oKB_Media_Stop: return "oKB_Media_Stop";
		case oKB_Media_Prev_Track: return "oKB_Media_Prev_Track";
		case oKB_Media_Next_Track: return "oKB_Media_Next_Track";
		case oKB_Launch_Mail: return "oKB_Launch_Mail";
		case oKB_Browser_Search: return "oKB_Browser_Search";
		case oKB_Browser_Home: return "oKB_Browser_Home";
		case oKB_Browser_Back: return "oKB_Browser_Back";
		case oKB_Browser_Forward: return "oKB_Browser_Forward";
		case oKB_Browser_Stop: return "oKB_Browser_Stop";
		case oKB_Browser_Refresh: return "oKB_Browser_Refresh";
		case oKB_Browser_Favorites: return "oKB_Browser_Favorites";
		case oKB_Launch_Media_Select: return "oKB_Launch_Media_Select";
		case oKB_VoidSymbol: return "oKB_VoidSymbol";
#ifdef XK_MISCELLANY
		/*
		 * TTY Functions, cleverly chosen to map to ascii, for convenience of
		 * programming, but could have been arbitrary (at the cost of lookup
		 * tables in client code.
		 */ 
		case oKB_BackSpace: return "oKB_BackSpace";
		case oKB_Tab: return "oKB_Tab";
		case oKB_Linefeed: return "oKB_Linefeed";
		case oKB_Clear: return "oKB_Clear";
		case oKB_Return: return "oKB_Return";
		case oKB_Pause: return "oKB_Pause";
		case oKB_Scroll_Lock: return "oKB_Scroll_Lock";
		case oKB_Sys_Req: return "oKB_Sys_Req";
		case oKB_Escape: return "oKB_Escape";
		case oKB_Delete: return "oKB_Delete";
		/* International & multi-key character composition*/
		case oKB_Multi_key: return "oKB_Multi_key";
		/* Japanese keyboard support*/
		case oKB_Kanji: return "oKB_Kanji";
		case oKB_Muhenkan: return "oKB_Muhenkan";
		case oKB_Henkan_Mode: return "oKB_Henkan_Mode";
		//case oKB_Henkan: return "oKB_Henkan";
		case oKB_Romaji: return "oKB_Romaji";
		case oKB_Hiragana: return "oKB_Hiragana";
		case oKB_Katakana: return "oKB_Katakana";
		case oKB_Hiragana_Katakana: return "oKB_Hiragana_Katakana";
		case oKB_Zenkaku: return "oKB_Zenkaku";
		case oKB_Hankaku: return "oKB_Hankaku";
		case oKB_Zenkaku_Hankaku: return "oKB_Zenkaku_Hankaku";
		case oKB_Touroku: return "oKB_Touroku";
		case oKB_Massyo: return "oKB_Massyo";
		case oKB_Kana_Lock: return "oKB_Kana_Lock";
		case oKB_Kana_Shift: return "oKB_Kana_Shift";
		case oKB_Eisu_Shift: return "oKB_Eisu_Shift";
		case oKB_Eisu_toggle: return "oKB_Eisu_toggle";
		/* 0xFF31 thru 0xFF3F are under XK_KOREAN*/
		/* Cursor control & motion*/ 
		case oKB_Home: return "oKB_Home";
		case oKB_Left: return "oKB_Left";
		case oKB_Up: return "oKB_Up";
		case oKB_Right: return "oKB_Right";
		case oKB_Down: return "oKB_Down";
		//case oKB_Prior: return "oKB_Prior";
		case oKB_Page_Up: return "oKB_Page_Up";
		//case oKB_Next: return "oKB_Next";
		case oKB_Page_Down: return "oKB_Page_Down";
		case oKB_End: return "oKB_End";
		case oKB_Begin: return "oKB_Begin";
		/* Misc Functions*/
		case oKB_Select: return "oKB_Select";
		case oKB_Print: return "oKB_Print";
		case oKB_Execute: return "oKB_Execute";
		case oKB_Insert: return "oKB_Insert";
		case oKB_Undo: return "oKB_Undo";
		case oKB_Redo: return "oKB_Redo";
		case oKB_Menu: return "oKB_Menu";
		case oKB_Find: return "oKB_Find";
		case oKB_Cancel: return "oKB_Cancel";
		case oKB_Help: return "oKB_Help";
		case oKB_Break: return "oKB_Break";
		case oKB_Mode_switch: return "oKB_Mode_switch";
		//case oKB_script_switch: return "oKB_script_switch";
		case oKB_Num_Lock: return "oKB_Num_Lock";
		/* Keypad Functions, keypad numbers cleverly chosen to map to ascii*/
		case oKB_KP_Space: return "oKB_KP_Space";
		case oKB_KP_Tab: return "oKB_KP_Tab";
		case oKB_KP_Enter: return "oKB_KP_Enter";
		case oKB_KP_F1: return "oKB_KP_F1";
		case oKB_KP_F2: return "oKB_KP_F2";
		case oKB_KP_F3: return "oKB_KP_F3";
		case oKB_KP_F4: return "oKB_KP_F4";
		case oKB_KP_Home: return "oKB_KP_Home";
		case oKB_KP_Left: return "oKB_KP_Left";
		case oKB_KP_Up: return "oKB_KP_Up";
		case oKB_KP_Right: return "oKB_KP_Right";
		case oKB_KP_Down: return "oKB_KP_Down";
		//case oKB_KP_Prior: return "oKB_KP_Prior";
		case oKB_KP_Page_Up: return "oKB_KP_Page_Up";
		//case oKB_KP_Next: return "oKB_KP_Next";
		case oKB_KP_Page_Down: return "oKB_KP_Page_Down";
		case oKB_KP_End: return "oKB_KP_End";
		case oKB_KP_Begin: return "oKB_KP_Begin";
		case oKB_KP_Insert: return "oKB_KP_Insert";
		case oKB_KP_Delete: return "oKB_KP_Delete";
		case oKB_KP_Equal: return "oKB_KP_Equal";
		case oKB_KP_Multiply: return "oKB_KP_Multiply";
		case oKB_KP_Add: return "oKB_KP_Add";
		case oKB_KP_Separator: return "oKB_KP_Separator";
		case oKB_KP_Subtract: return "oKB_KP_Subtract";
		case oKB_KP_Decimal: return "oKB_KP_Decimal";
		case oKB_KP_Divide: return "oKB_KP_Divide";
		case oKB_KP_0: return "oKB_KP_0";
		case oKB_KP_1: return "oKB_KP_1";
		case oKB_KP_2: return "oKB_KP_2";
		case oKB_KP_3: return "oKB_KP_3";
		case oKB_KP_4: return "oKB_KP_4";
		case oKB_KP_5: return "oKB_KP_5";
		case oKB_KP_6: return "oKB_KP_6";
		case oKB_KP_7: return "oKB_KP_7";
		case oKB_KP_8: return "oKB_KP_8";
		case oKB_KP_9: return "oKB_KP_9";
		 /*
		 * Auxilliary Functions; note the duplicate definitions for left and right
		 * function keys; Sun keyboards and a few other manufactures have such
		 * function key groups on the left and/or right sides of the keyboard.
		 * We've not found a keyboard with more than 35 function keys total.
		 */
		case oKB_F1: return "oKB_F1";
		case oKB_F2: return "oKB_F2";
		case oKB_F3: return "oKB_F3";
		case oKB_F4: return "oKB_F4";
		case oKB_F5: return "oKB_F5";
		case oKB_F6: return "oKB_F6";
		case oKB_F7: return "oKB_F7";
		case oKB_F8: return "oKB_F8";
		case oKB_F9: return "oKB_F9";
		case oKB_F10: return "oKB_F10";
		case oKB_F11: return "oKB_F11";
		//case oKB_L1: return "oKB_L1";
		case oKB_F12: return "oKB_F12";
		//case oKB_L2: return "oKB_L2";
		case oKB_F13: return "oKB_F13";
		//case oKB_L3: return "oKB_L3";
		case oKB_F14: return "oKB_F14";
		//case oKB_L4: return "oKB_L4";
		case oKB_F15: return "oKB_F15";
		//case oKB_L5: return "oKB_L5";
		case oKB_F16: return "oKB_F16";
		//case oKB_L6: return "oKB_L6";
		case oKB_F17: return "oKB_F17";
		//case oKB_L7: return "oKB_L7";
		case oKB_F18: return "oKB_F18";
		//case oKB_L8: return "oKB_L8";
		case oKB_F19: return "oKB_F19";
		//case oKB_L9: return "oKB_L9";
		case oKB_F20: return "oKB_F20";
		//case oKB_L10: return "oKB_L10";
		case oKB_F21: return "oKB_F21";
		//case oKB_R1: return "oKB_R1";
		case oKB_F22: return "oKB_F22";
		//case oKB_R2: return "oKB_R2";
		case oKB_F23: return "oKB_F23";
		//case oKB_R3: return "oKB_R3";
		case oKB_F24: return "oKB_F24";
		//case oKB_R4: return "oKB_R4";
		case oKB_F25: return "oKB_F25";
		//case oKB_R5: return "oKB_R5";
		case oKB_F26: return "oKB_F26";
		//case oKB_R6: return "oKB_R6";
		case oKB_F27: return "oKB_F27";
		//case oKB_R7: return "oKB_R7";
		case oKB_F28: return "oKB_F28";
		//case oKB_R8: return "oKB_R8";
		case oKB_F29: return "oKB_F29";
		//case oKB_R9: return "oKB_R9";
		case oKB_F30: return "oKB_F30";
		//case oKB_R10: return "oKB_R10";
		case oKB_F31: return "oKB_F31";
		//case oKB_R11: return "oKB_R11";
		case oKB_F32: return "oKB_F32";
		//case oKB_R12: return "oKB_R12";
		case oKB_F33: return "oKB_F33";
		//case oKB_R13: return "oKB_R13";
		case oKB_F34: return "oKB_F34";
		//case oKB_R14: return "oKB_R14";
		case oKB_F35: return "oKB_F35";
		//case oKB_R15: return "oKB_R15";
		/* Modifiers*/ 
		case oKB_Shift_L: return "oKB_Shift_L";
		case oKB_Shift_R: return "oKB_Shift_R";
		case oKB_Control_L: return "oKB_Control_L";
		case oKB_Control_R: return "oKB_Control_R";
		case oKB_Caps_Lock: return "oKB_Caps_Lock";
		case oKB_Shift_Lock: return "oKB_Shift_Lock";
		case oKB_Meta_L: return "oKB_Meta_L";
		case oKB_Meta_R: return "oKB_Meta_R";
		case oKB_Alt_L: return "oKB_Alt_L";
		case oKB_Alt_R: return "oKB_Alt_R";
		case oKB_Super_L: return "oKB_Super_L";
		case oKB_Super_R: return "oKB_Super_R";
		case oKB_Hyper_L: return "oKB_Hyper_L";
		case oKB_Hyper_R: return "oKB_Hyper_R";
		#endif/* XK_MISCELLANY*/ 
		/*
		 * ISO 9995 Function and Modifier Keys
		 * Byte 3 = 0xFE
		 */
		#ifdef XK_XKB_KEYS
		case oKB_ISO_Lock: return "oKB_ISO_Lock";
		case oKB_ISO_Level2_Latch: return "oKB_ISO_Level2_Latch";
		case oKB_ISO_Level3_Shift: return "oKB_ISO_Level3_Shift";
		case oKB_ISO_Level3_Latch: return "oKB_ISO_Level3_Latch";
		case oKB_ISO_Level3_Lock: return "oKB_ISO_Level3_Lock";
		//case oKB_ISO_Group_Shift: return "oKB_ISO_Group_Shift";
		case oKB_ISO_Group_Latch: return "oKB_ISO_Group_Latch";
		case oKB_ISO_Group_Lock: return "oKB_ISO_Group_Lock";
		case oKB_ISO_Next_Group: return "oKB_ISO_Next_Group";
		case oKB_ISO_Next_Group_Lock: return "oKB_ISO_Next_Group_Lock";
		case oKB_ISO_Prev_Group: return "oKB_ISO_Prev_Group";
		case oKB_ISO_Prev_Group_Lock: return "oKB_ISO_Prev_Group_Lock";
		case oKB_ISO_First_Group: return "oKB_ISO_First_Group";
		case oKB_ISO_First_Group_Lock: return "oKB_ISO_First_Group_Lock";
		case oKB_ISO_Last_Group: return "oKB_ISO_Last_Group";
		case oKB_ISO_Last_Group_Lock: return "oKB_ISO_Last_Group_Lock";
		case oKB_ISO_Left_Tab: return "oKB_ISO_Left_Tab";
		case oKB_ISO_Move_Line_Up: return "oKB_ISO_Move_Line_Up";
		case oKB_ISO_Move_Line_Down: return "oKB_ISO_Move_Line_Down";
		case oKB_ISO_Partial_Line_Up: return "oKB_ISO_Partial_Line_Up";
		case oKB_ISO_Partial_Line_Down: return "oKB_ISO_Partial_Line_Down";
		case oKB_ISO_Partial_Space_Left: return "oKB_ISO_Partial_Space_Left";
		case oKB_ISO_Partial_Space_Right: return "oKB_ISO_Partial_Space_Right";
		case oKB_ISO_Set_Margin_Left: return "oKB_ISO_Set_Margin_Left";
		case oKB_ISO_Set_Margin_Right: return "oKB_ISO_Set_Margin_Right";
		case oKB_ISO_Release_Margin_Left: return "oKB_ISO_Release_Margin_Left";
		case oKB_ISO_Release_Margin_Right: return "oKB_ISO_Release_Margin_Right";
		case oKB_ISO_Release_Both_Margins: return "oKB_ISO_Release_Both_Margins";
		case oKB_ISO_Fast_Cursor_Left: return "oKB_ISO_Fast_Cursor_Left";
		case oKB_ISO_Fast_Cursor_Right: return "oKB_ISO_Fast_Cursor_Right";
		case oKB_ISO_Fast_Cursor_Up: return "oKB_ISO_Fast_Cursor_Up";
		case oKB_ISO_Fast_Cursor_Down: return "oKB_ISO_Fast_Cursor_Down";
		case oKB_ISO_Continuous_Underline: return "oKB_ISO_Continuous_Underline";
		case oKB_ISO_Discontinuous_Underline: return "oKB_ISO_Discontinuous_Underline";
		case oKB_ISO_Emphasize: return "oKB_ISO_Emphasize";
		case oKB_ISO_Center_Object: return "oKB_ISO_Center_Object";
		case oKB_ISO_Enter: return "oKB_ISO_Enter";
		case oKB_dead_grave: return "oKB_dead_grave";
		case oKB_dead_acute: return "oKB_dead_acute";
		case oKB_dead_circumflex: return "oKB_dead_circumflex";
		case oKB_dead_tilde: return "oKB_dead_tilde";
		case oKB_dead_macron: return "oKB_dead_macron";
		case oKB_dead_breve: return "oKB_dead_breve";
		case oKB_dead_abovedot: return "oKB_dead_abovedot";
		case oKB_dead_diaeresis: return "oKB_dead_diaeresis";
		case oKB_dead_abovering: return "oKB_dead_abovering";
		case oKB_dead_doubleacute: return "oKB_dead_doubleacute";
		case oKB_dead_caron: return "oKB_dead_caron";
		case oKB_dead_cedilla: return "oKB_dead_cedilla";
		case oKB_dead_ogonek: return "oKB_dead_ogonek";
		case oKB_dead_iota: return "oKB_dead_iota";
		case oKB_dead_voiced_sound: return "oKB_dead_voiced_sound";
		case oKB_dead_semivoiced_sound: return "oKB_dead_semivoiced_sound";
		case oKB_First_Virtual_Screen: return "oKB_First_Virtual_Screen";
		case oKB_Prev_Virtual_Screen: return "oKB_Prev_Virtual_Screen";
		case oKB_Next_Virtual_Screen: return "oKB_Next_Virtual_Screen";
		case oKB_Last_Virtual_Screen: return "oKB_Last_Virtual_Screen";
		case oKB_Terminate_Server: return "oKB_Terminate_Server";
		case oKB_Pointer_Left: return "oKB_Pointer_Left";
		case oKB_Pointer_Right: return "oKB_Pointer_Right";
		case oKB_Pointer_Up: return "oKB_Pointer_Up";
		case oKB_Pointer_Down: return "oKB_Pointer_Down";
		case oKB_Pointer_UpLeft: return "oKB_Pointer_UpLeft";
		case oKB_Pointer_UpRight: return "oKB_Pointer_UpRight";
		case oKB_Pointer_DownLeft: return "oKB_Pointer_DownLeft";
		case oKB_Pointer_DownRight: return "oKB_Pointer_DownRight";

		// @oooii-tony: Use more visually pleasing versions of the same values

		//case oKB_Pointer_Button_Dflt: return "oKB_Pointer_Button_Dflt";
		case oKB_Pointer_Button_Left: return "oKB_Pointer_Button_Left";
		case oKB_Pointer_Button_Right: return "oKB_Pointer_Button_Right";
		case oKB_Pointer_Button_Middle: return "oKB_Pointer_Button_Middle";
		case oKB_Pointer_Button_Back: return "oKB_Pointer_Button_Back";
		case oKB_Pointer_Button_Forward: return "oKB_Pointer_Button_Forward";
		case oKB_Pointer_Button5: return "oKB_Pointer_Button5";
		case oKB_Pointer_DblClick_Left: return "oKB_Pointer_DblClick_Left";
		case oKB_Pointer_DblClick_Right: return "oKB_Pointer_DblClick_Right";
		case oKB_Pointer_DblClick_Middle: return "oKB_Pointer_DblClick_Middle";
		case oKB_Pointer_DblClick_Back: return "oKB_Pointer_DblClick_Back";
		case oKB_Pointer_DblClick_Forward: return "oKB_Pointer_DblClick_Forward";
		case oKB_Pointer_DblClick5: return "oKB_Pointer_DblClick5";
		case oKB_Pointer_Drag_Left: return "oKB_Pointer_Drag_Left";
		case oKB_Pointer_Drag_Right: return "oKB_Pointer_Drag_Right";
		case oKB_Pointer_Drag_Middle: return "oKB_Pointer_Drag_Middle";
		case oKB_Pointer_Drag_Back: return "oKB_Pointer_Drag_Back";
		case oKB_Pointer_Drag_Forward: return "oKB_Pointer_Drag_Forward";
		case oKB_Pointer_EnableKeys: return "oKB_Pointer_EnableKeys";
		case oKB_Pointer_Accelerate: return "oKB_Pointer_Accelerate";
		case oKB_Pointer_DfltBtnNext: return "oKB_Pointer_DfltBtnNext";
		case oKB_Pointer_DfltBtnPrev: return "oKB_Pointer_DfltBtnPrev";
		#endif
		/*
		 * 3270 Terminal Keys
		 * Byte 3 = 0xFD
		 */
		#ifdef XK_3270
		case oKB_3270_Duplicate: return "oKB_3270_Duplicate";
		case oKB_3270_FieldMark: return "oKB_3270_FieldMark";
		case oKB_3270_Right2: return "oKB_3270_Right2";
		case oKB_3270_Left2: return "oKB_3270_Left2";
		case oKB_3270_BackTab: return "oKB_3270_BackTab";
		case oKB_3270_EraseEOF: return "oKB_3270_EraseEOF";
		case oKB_3270_EraseInput: return "oKB_3270_EraseInput";
		case oKB_3270_Reset: return "oKB_3270_Reset";
		case oKB_3270_Quit: return "oKB_3270_Quit";
		case oKB_3270_PA1: return "oKB_3270_PA1";
		case oKB_3270_PA2: return "oKB_3270_PA2";
		case oKB_3270_PA3: return "oKB_3270_PA3";
		case oKB_3270_Test: return "oKB_3270_Test";
		case oKB_3270_Attn: return "oKB_3270_Attn";
		case oKB_3270_CursorBlink: return "oKB_3270_CursorBlink";
		case oKB_3270_AltCursor: return "oKB_3270_AltCursor";
		case oKB_3270_KeyClick: return "oKB_3270_KeyClick";
		case oKB_3270_Jump: return "oKB_3270_Jump";
		case oKB_3270_Ident: return "oKB_3270_Ident";
		case oKB_3270_Rule: return "oKB_3270_Rule";
		case oKB_3270_Copy: return "oKB_3270_Copy";
		case oKB_3270_Play: return "oKB_3270_Play";
		case oKB_3270_Setup: return "oKB_3270_Setup";
		case oKB_3270_Record: return "oKB_3270_Record";
		case oKB_3270_ChangeScreen: return "oKB_3270_ChangeScreen";
		case oKB_3270_DeleteWord: return "oKB_3270_DeleteWord";
		case oKB_3270_ExSelect: return "oKB_3270_ExSelect";
		case oKB_3270_CursorSelect: return "oKB_3270_CursorSelect";
		case oKB_3270_PrintScreen: return "oKB_3270_PrintScreen";
		case oKB_3270_Enter: return "oKB_3270_Enter";
		#endif
		/*
		 * Latin 1
		 * Byte 3 = 0
		 */
		#ifdef XK_LATIN1
		case oKB_space: return "oKB_space";
		case oKB_exclam: return "oKB_exclam";
		case oKB_quotedbl: return "oKB_quotedbl";
		case oKB_numbersign: return "oKB_numbersign";
		case oKB_dollar: return "oKB_dollar";
		case oKB_percent: return "oKB_percent";
		case oKB_ampersand: return "oKB_ampersand";
		case oKB_apostrophe: return "oKB_apostrophe";
		//case oKB_quoteright: return "oKB_quoteright";
		case oKB_parenleft: return "oKB_parenleft";
		case oKB_parenright: return "oKB_parenright";
		case oKB_asterisk: return "oKB_asterisk";
		case oKB_plus: return "oKB_plus";
		case oKB_comma: return "oKB_comma";
		case oKB_minus: return "oKB_minus";
		case oKB_period: return "oKB_period";
		case oKB_slash: return "oKB_slash";
		case oKB_0: return "oKB_0";
		case oKB_1: return "oKB_1";
		case oKB_2: return "oKB_2";
		case oKB_3: return "oKB_3";
		case oKB_4: return "oKB_4";
		case oKB_5: return "oKB_5";
		case oKB_6: return "oKB_6";
		case oKB_7: return "oKB_7";
		case oKB_8: return "oKB_8";
		case oKB_9: return "oKB_9";
		case oKB_colon: return "oKB_colon";
		case oKB_semicolon: return "oKB_semicolon";
		case oKB_less: return "oKB_less";
		case oKB_equal: return "oKB_equal";
		case oKB_greater: return "oKB_greater";
		case oKB_question: return "oKB_question";
		case oKB_at: return "oKB_at";
		case oKB_A: return "oKB_A";
		case oKB_B: return "oKB_B";
		case oKB_C: return "oKB_C";
		case oKB_D: return "oKB_D";
		case oKB_E: return "oKB_E";
		case oKB_F: return "oKB_F";
		case oKB_G: return "oKB_G";
		case oKB_H: return "oKB_H";
		case oKB_I: return "oKB_I";
		case oKB_J: return "oKB_J";
		case oKB_K: return "oKB_K";
		case oKB_L: return "oKB_L";
		case oKB_M: return "oKB_M";
		case oKB_N: return "oKB_N";
		case oKB_O: return "oKB_O";
		case oKB_P: return "oKB_P";
		case oKB_Q: return "oKB_Q";
		case oKB_R: return "oKB_R";
		case oKB_S: return "oKB_S";
		case oKB_T: return "oKB_T";
		case oKB_U: return "oKB_U";
		case oKB_V: return "oKB_V";
		case oKB_W: return "oKB_W";
		case oKB_X: return "oKB_X";
		case oKB_Y: return "oKB_Y";
		case oKB_Z: return "oKB_Z";
		case oKB_bracketleft: return "oKB_bracketleft";
		case oKB_backslash: return "oKB_backslash";
		case oKB_bracketright: return "oKB_bracketright";
		case oKB_asciicircum: return "oKB_asciicircum";
		case oKB_underscore: return "oKB_underscore";
		case oKB_grave: return "oKB_grave";
		//case oKB_quoteleft: return "oKB_quoteleft";
		case oKB_a: return "oKB_a";
		case oKB_b: return "oKB_b";
		case oKB_c: return "oKB_c";
		case oKB_d: return "oKB_d";
		case oKB_e: return "oKB_e";
		case oKB_f: return "oKB_f";
		case oKB_g: return "oKB_g";
		case oKB_h: return "oKB_h";
		case oKB_i: return "oKB_i";
		case oKB_j: return "oKB_j";
		case oKB_k: return "oKB_k";
		case oKB_l: return "oKB_l";
		case oKB_m: return "oKB_m";
		case oKB_n: return "oKB_n";
		case oKB_o: return "oKB_o";
		case oKB_p: return "oKB_p";
		case oKB_q: return "oKB_q";
		case oKB_r: return "oKB_r";
		case oKB_s: return "oKB_s";
		case oKB_t: return "oKB_t";
		case oKB_u: return "oKB_u";
		case oKB_v: return "oKB_v";
		case oKB_w: return "oKB_w";
		case oKB_x: return "oKB_x";
		case oKB_y: return "oKB_y";
		case oKB_z: return "oKB_z";
		case oKB_braceleft: return "oKB_braceleft";
		case oKB_bar: return "oKB_bar";
		case oKB_braceright: return "oKB_braceright";
		case oKB_asciitilde: return "oKB_asciitilde";
		case oKB_nobreakspace: return "oKB_nobreakspace";
		case oKB_exclamdown: return "oKB_exclamdown";
		case oKB_cent: return "oKB_cent";
		case oKB_sterling: return "oKB_sterling";
		case oKB_currency: return "oKB_currency";
		case oKB_yen: return "oKB_yen";
		case oKB_brokenbar: return "oKB_brokenbar";
		case oKB_section: return "oKB_section";
		case oKB_diaeresis: return "oKB_diaeresis";
		case oKB_copyright: return "oKB_copyright";
		case oKB_ordfeminine: return "oKB_ordfeminine";
		case oKB_guillemotleft: return "oKB_guillemotleft";
		case oKB_notsign: return "oKB_notsign";
		case oKB_hyphen: return "oKB_hyphen";
		case oKB_registered: return "oKB_registered";
		case oKB_macron: return "oKB_macron";
		case oKB_degree: return "oKB_degree";
		case oKB_plusminus: return "oKB_plusminus";
		case oKB_twosuperior: return "oKB_twosuperior";
		case oKB_threesuperior: return "oKB_threesuperior";
		case oKB_acute: return "oKB_acute";
		case oKB_mu: return "oKB_mu";
		case oKB_paragraph: return "oKB_paragraph";
		case oKB_periodcentered: return "oKB_periodcentered";
		case oKB_cedilla: return "oKB_cedilla";
		case oKB_onesuperior: return "oKB_onesuperior";
		case oKB_masculine: return "oKB_masculine";
		case oKB_guillemotright: return "oKB_guillemotright";
		case oKB_onequarter: return "oKB_onequarter";
		case oKB_onehalf: return "oKB_onehalf";
		case oKB_threequarters: return "oKB_threequarters";
		case oKB_questiondown: return "oKB_questiondown";
		case oKB_Agrave: return "oKB_Agrave";
		case oKB_Aacute: return "oKB_Aacute";
		case oKB_Acircumflex: return "oKB_Acircumflex";
		case oKB_Atilde: return "oKB_Atilde";
		case oKB_Adiaeresis: return "oKB_Adiaeresis";
		case oKB_Aring: return "oKB_Aring";
		case oKB_AE: return "oKB_AE";
		case oKB_Ccedilla: return "oKB_Ccedilla";
		case oKB_Egrave: return "oKB_Egrave";
		case oKB_Eacute: return "oKB_Eacute";
		case oKB_Ecircumflex: return "oKB_Ecircumflex";
		case oKB_Ediaeresis: return "oKB_Ediaeresis";
		case oKB_Igrave: return "oKB_Igrave";
		case oKB_Iacute: return "oKB_Iacute";
		case oKB_Icircumflex: return "oKB_Icircumflex";
		case oKB_Idiaeresis: return "oKB_Idiaeresis";
		case oKB_ETH: return "oKB_ETH";
		//case oKB_Eth: return "oKB_Eth";
		case oKB_Ntilde: return "oKB_Ntilde";
		case oKB_Ograve: return "oKB_Ograve";
		case oKB_Oacute: return "oKB_Oacute";
		case oKB_Ocircumflex: return "oKB_Ocircumflex";
		case oKB_Otilde: return "oKB_Otilde";
		case oKB_Odiaeresis: return "oKB_Odiaeresis";
		case oKB_multiply: return "oKB_multiply";
		case oKB_Ooblique: return "oKB_Ooblique";
		case oKB_Ugrave: return "oKB_Ugrave";
		case oKB_Uacute: return "oKB_Uacute";
		case oKB_Ucircumflex: return "oKB_Ucircumflex";
		case oKB_Udiaeresis: return "oKB_Udiaeresis";
		case oKB_Yacute: return "oKB_Yacute";
		case oKB_THORN: return "oKB_THORN";
		//case oKB_Thorn: return "oKB_Thorn";
		case oKB_ssharp: return "oKB_ssharp";
		case oKB_agrave: return "oKB_agrave";
		case oKB_aacute: return "oKB_aacute";
		case oKB_acircumflex: return "oKB_acircumflex";
		case oKB_atilde: return "oKB_atilde";
		case oKB_adiaeresis: return "oKB_adiaeresis";
		case oKB_aring: return "oKB_aring";
		case oKB_ae: return "oKB_ae";
		case oKB_ccedilla: return "oKB_ccedilla";
		case oKB_egrave: return "oKB_egrave";
		case oKB_eacute: return "oKB_eacute";
		case oKB_ecircumflex: return "oKB_ecircumflex";
		case oKB_ediaeresis: return "oKB_ediaeresis";
		case oKB_igrave: return "oKB_igrave";
		case oKB_iacute: return "oKB_iacute";
		case oKB_icircumflex: return "oKB_icircumflex";
		case oKB_idiaeresis: return "oKB_idiaeresis";
		case oKB_eth: return "oKB_eth";
		case oKB_ntilde: return "oKB_ntilde";
		case oKB_ograve: return "oKB_ograve";
		case oKB_oacute: return "oKB_oacute";
		case oKB_ocircumflex: return "oKB_ocircumflex";
		case oKB_otilde: return "oKB_otilde";
		case oKB_odiaeresis: return "oKB_odiaeresis";
		case oKB_division: return "oKB_division";
		case oKB_oslash: return "oKB_oslash";
		case oKB_ugrave: return "oKB_ugrave";
		case oKB_uacute: return "oKB_uacute";
		case oKB_ucircumflex: return "oKB_ucircumflex";
		case oKB_udiaeresis: return "oKB_udiaeresis";
		case oKB_yacute: return "oKB_yacute";
		case oKB_thorn: return "oKB_thorn";
		case oKB_ydiaeresis: return "oKB_ydiaeresis";
		#endif/* XK_LATIN1*/ 
		/*
		 * Latin 2
		 * Byte 3 = 1
		 */ 
		#ifdef XK_LATIN2
		case oKB_Aogonek: return "oKB_Aogonek";
		case oKB_breve: return "oKB_breve";
		case oKB_Lstroke: return "oKB_Lstroke";
		case oKB_Lcaron: return "oKB_Lcaron";
		case oKB_Sacute: return "oKB_Sacute";
		case oKB_Scaron: return "oKB_Scaron";
		case oKB_Scedilla: return "oKB_Scedilla";
		case oKB_Tcaron: return "oKB_Tcaron";
		case oKB_Zacute: return "oKB_Zacute";
		case oKB_Zcaron: return "oKB_Zcaron";
		case oKB_Zabovedot: return "oKB_Zabovedot";
		case oKB_aogonek: return "oKB_aogonek";
		case oKB_ogonek: return "oKB_ogonek";
		case oKB_lstroke: return "oKB_lstroke";
		case oKB_lcaron: return "oKB_lcaron";
		case oKB_sacute: return "oKB_sacute";
		case oKB_caron: return "oKB_caron";
		case oKB_scaron: return "oKB_scaron";
		case oKB_scedilla: return "oKB_scedilla";
		case oKB_tcaron: return "oKB_tcaron";
		case oKB_zacute: return "oKB_zacute";
		case oKB_doubleacute: return "oKB_doubleacute";
		case oKB_zcaron: return "oKB_zcaron";
		case oKB_zabovedot: return "oKB_zabovedot";
		case oKB_Racute: return "oKB_Racute";
		case oKB_Abreve: return "oKB_Abreve";
		case oKB_Lacute: return "oKB_Lacute";
		case oKB_Cacute: return "oKB_Cacute";
		case oKB_Ccaron: return "oKB_Ccaron";
		case oKB_Eogonek: return "oKB_Eogonek";
		case oKB_Ecaron: return "oKB_Ecaron";
		case oKB_Dcaron: return "oKB_Dcaron";
		case oKB_Dstroke: return "oKB_Dstroke";
		case oKB_Nacute: return "oKB_Nacute";
		case oKB_Ncaron: return "oKB_Ncaron";
		case oKB_Odoubleacute: return "oKB_Odoubleacute";
		case oKB_Rcaron: return "oKB_Rcaron";
		case oKB_Uring: return "oKB_Uring";
		case oKB_Udoubleacute: return "oKB_Udoubleacute";
		case oKB_Tcedilla: return "oKB_Tcedilla";
		case oKB_racute: return "oKB_racute";
		case oKB_abreve: return "oKB_abreve";
		case oKB_lacute: return "oKB_lacute";
		case oKB_cacute: return "oKB_cacute";
		case oKB_ccaron: return "oKB_ccaron";
		case oKB_eogonek: return "oKB_eogonek";
		case oKB_ecaron: return "oKB_ecaron";
		case oKB_dcaron: return "oKB_dcaron";
		case oKB_dstroke: return "oKB_dstroke";
		case oKB_nacute: return "oKB_nacute";
		case oKB_ncaron: return "oKB_ncaron";
		case oKB_odoubleacute: return "oKB_odoubleacute";
		case oKB_udoubleacute: return "oKB_udoubleacute";
		case oKB_rcaron: return "oKB_rcaron";
		case oKB_uring: return "oKB_uring";
		case oKB_tcedilla: return "oKB_tcedilla";
		case oKB_abovedot: return "oKB_abovedot";
		#endif/* XK_LATIN2*/
		/*
		 * Latin 3
		 * Byte 3 = 2
		 */
		#ifdef XK_LATIN3
		case oKB_Hstroke: return "oKB_Hstroke";
		case oKB_Hcircumflex: return "oKB_Hcircumflex";
		case oKB_Iabovedot: return "oKB_Iabovedot";
		case oKB_Gbreve: return "oKB_Gbreve";
		case oKB_Jcircumflex: return "oKB_Jcircumflex";
		case oKB_hstroke: return "oKB_hstroke";
		case oKB_hcircumflex: return "oKB_hcircumflex";
		case oKB_idotless: return "oKB_idotless";
		case oKB_gbreve: return "oKB_gbreve";
		case oKB_jcircumflex: return "oKB_jcircumflex";
		case oKB_Cabovedot: return "oKB_Cabovedot";
		case oKB_Ccircumflex: return "oKB_Ccircumflex";
		case oKB_Gabovedot: return "oKB_Gabovedot";
		case oKB_Gcircumflex: return "oKB_Gcircumflex";
		case oKB_Ubreve: return "oKB_Ubreve";
		case oKB_Scircumflex: return "oKB_Scircumflex";
		case oKB_cabovedot: return "oKB_cabovedot";
		case oKB_ccircumflex: return "oKB_ccircumflex";
		case oKB_gabovedot: return "oKB_gabovedot";
		case oKB_gcircumflex: return "oKB_gcircumflex";
		case oKB_ubreve: return "oKB_ubreve";
		case oKB_scircumflex: return "oKB_scircumflex";
		#endif/* XK_LATIN3*/
		/*
		 * Latin 4
		 * Byte 3 = 3
		 */
		#ifdef XK_LATIN4
		case oKB_kra: return "oKB_kra";
		case oKB_kappa: return "oKB_kappa";
		case oKB_Rcedilla: return "oKB_Rcedilla";
		case oKB_Itilde: return "oKB_Itilde";
		case oKB_Lcedilla: return "oKB_Lcedilla";
		case oKB_Emacron: return "oKB_Emacron";
		case oKB_Gcedilla: return "oKB_Gcedilla";
		case oKB_Tslash: return "oKB_Tslash";
		case oKB_rcedilla: return "oKB_rcedilla";
		case oKB_itilde: return "oKB_itilde";
		case oKB_lcedilla: return "oKB_lcedilla";
		case oKB_emacron: return "oKB_emacron";
		case oKB_gcedilla: return "oKB_gcedilla";
		case oKB_tslash: return "oKB_tslash";
		case oKB_ENG: return "oKB_ENG";
		case oKB_eng: return "oKB_eng";
		case oKB_Amacron: return "oKB_Amacron";
		case oKB_Iogonek: return "oKB_Iogonek";
		case oKB_Eabovedot: return "oKB_Eabovedot";
		case oKB_Imacron: return "oKB_Imacron";
		case oKB_Ncedilla: return "oKB_Ncedilla";
		case oKB_Omacron: return "oKB_Omacron";
		case oKB_Kcedilla: return "oKB_Kcedilla";
		case oKB_Uogonek: return "oKB_Uogonek";
		case oKB_Utilde: return "oKB_Utilde";
		case oKB_Umacron: return "oKB_Umacron";
		case oKB_amacron: return "oKB_amacron";
		case oKB_iogonek: return "oKB_iogonek";
		case oKB_eabovedot: return "oKB_eabovedot";
		case oKB_imacron: return "oKB_imacron";
		case oKB_ncedilla: return "oKB_ncedilla";
		case oKB_omacron: return "oKB_omacron";
		case oKB_kcedilla: return "oKB_kcedilla";
		case oKB_uogonek: return "oKB_uogonek";
		case oKB_utilde: return "oKB_utilde";
		case oKB_umacron: return "oKB_umacron";
		#endif/* XK_LATIN4*/
		/*
		 * Katakana
		 * Byte 3 = 4
		 */
		#ifdef XK_KATAKANA
		case oKB_overline: return "oKB_overline";
		case oKB_kana_fullstop: return "oKB_kana_fullstop";
		case oKB_kana_openingbracket: return "oKB_kana_openingbracket";
		case oKB_kana_closingbracket: return "oKB_kana_closingbracket";
		case oKB_kana_comma: return "oKB_kana_comma";
		case oKB_kana_conjunctive: return "oKB_kana_conjunctive";
		case oKB_kana_middledot: return "oKB_kana_middledot";
		case oKB_kana_WO: return "oKB_kana_WO";
		case oKB_kana_a: return "oKB_kana_a";
		case oKB_kana_i: return "oKB_kana_i";
		case oKB_kana_u: return "oKB_kana_u";
		case oKB_kana_e: return "oKB_kana_e";
		case oKB_kana_o: return "oKB_kana_o";
		case oKB_kana_ya: return "oKB_kana_ya";
		case oKB_kana_yu: return "oKB_kana_yu";
		case oKB_kana_yo: return "oKB_kana_yo";
		case oKB_kana_tsu: return "oKB_kana_tsu";
		case oKB_kana_tu: return "oKB_kana_tu";
		case oKB_prolongedsound: return "oKB_prolongedsound";
		case oKB_kana_A: return "oKB_kana_A";
		case oKB_kana_I: return "oKB_kana_I";
		case oKB_kana_U: return "oKB_kana_U";
		case oKB_kana_E: return "oKB_kana_E";
		case oKB_kana_O: return "oKB_kana_O";
		case oKB_kana_KA: return "oKB_kana_KA";
		case oKB_kana_KI: return "oKB_kana_KI";
		case oKB_kana_KU: return "oKB_kana_KU";
		case oKB_kana_KE: return "oKB_kana_KE";
		case oKB_kana_KO: return "oKB_kana_KO";
		case oKB_kana_SA: return "oKB_kana_SA";
		case oKB_kana_SHI: return "oKB_kana_SHI";
		case oKB_kana_SU: return "oKB_kana_SU";
		case oKB_kana_SE: return "oKB_kana_SE";
		case oKB_kana_SO: return "oKB_kana_SO";
		case oKB_kana_TA: return "oKB_kana_TA";
		case oKB_kana_CHI: return "oKB_kana_CHI";
		case oKB_kana_TI: return "oKB_kana_TI";
		case oKB_kana_TSU: return "oKB_kana_TSU";
		case oKB_kana_TU: return "oKB_kana_TU";
		case oKB_kana_TE: return "oKB_kana_TE";
		case oKB_kana_TO: return "oKB_kana_TO";
		case oKB_kana_NA: return "oKB_kana_NA";
		case oKB_kana_NI: return "oKB_kana_NI";
		case oKB_kana_NU: return "oKB_kana_NU";
		case oKB_kana_NE: return "oKB_kana_NE";
		case oKB_kana_NO: return "oKB_kana_NO";
		case oKB_kana_HA: return "oKB_kana_HA";
		case oKB_kana_HI: return "oKB_kana_HI";
		case oKB_kana_FU: return "oKB_kana_FU";
		case oKB_kana_HU: return "oKB_kana_HU";
		case oKB_kana_HE: return "oKB_kana_HE";
		case oKB_kana_HO: return "oKB_kana_HO";
		case oKB_kana_MA: return "oKB_kana_MA";
		case oKB_kana_MI: return "oKB_kana_MI";
		case oKB_kana_MU: return "oKB_kana_MU";
		case oKB_kana_ME: return "oKB_kana_ME";
		case oKB_kana_MO: return "oKB_kana_MO";
		case oKB_kana_YA: return "oKB_kana_YA";
		case oKB_kana_YU: return "oKB_kana_YU";
		case oKB_kana_YO: return "oKB_kana_YO";
		case oKB_kana_RA: return "oKB_kana_RA";
		case oKB_kana_RI: return "oKB_kana_RI";
		case oKB_kana_RU: return "oKB_kana_RU";
		case oKB_kana_RE: return "oKB_kana_RE";
		case oKB_kana_RO: return "oKB_kana_RO";
		case oKB_kana_WA: return "oKB_kana_WA";
		case oKB_kana_N: return "oKB_kana_N";
		case oKB_voicedsound: return "oKB_voicedsound";
		case oKB_semivoicedsound: return "oKB_semivoicedsound";
		case oKB_kana_switch: return "oKB_kana_switch";
		#endif/* XK_KATAKANA*/
		/*
		 * Arabic
		 * Byte 3 = 5
		 */
		#ifdef XK_ARABIC
		case oKB_Arabic_comma: return "oKB_Arabic_comma";
		case oKB_Arabic_semicolon: return "oKB_Arabic_semicolon";
		case oKB_Arabic_question_mark: return "oKB_Arabic_question_mark";
		case oKB_Arabic_hamza: return "oKB_Arabic_hamza";
		case oKB_Arabic_maddaonalef: return "oKB_Arabic_maddaonalef";
		case oKB_Arabic_hamzaonalef: return "oKB_Arabic_hamzaonalef";
		case oKB_Arabic_hamzaonwaw: return "oKB_Arabic_hamzaonwaw";
		case oKB_Arabic_hamzaunderalef: return "oKB_Arabic_hamzaunderalef";
		case oKB_Arabic_hamzaonyeh: return "oKB_Arabic_hamzaonyeh";
		case oKB_Arabic_alef: return "oKB_Arabic_alef";
		case oKB_Arabic_beh: return "oKB_Arabic_beh";
		case oKB_Arabic_tehmarbuta: return "oKB_Arabic_tehmarbuta";
		case oKB_Arabic_teh: return "oKB_Arabic_teh";
		case oKB_Arabic_theh: return "oKB_Arabic_theh";
		case oKB_Arabic_jeem: return "oKB_Arabic_jeem";
		case oKB_Arabic_hah: return "oKB_Arabic_hah";
		case oKB_Arabic_khah: return "oKB_Arabic_khah";
		case oKB_Arabic_dal: return "oKB_Arabic_dal";
		case oKB_Arabic_thal: return "oKB_Arabic_thal";
		case oKB_Arabic_ra: return "oKB_Arabic_ra";
		case oKB_Arabic_zain: return "oKB_Arabic_zain";
		case oKB_Arabic_seen: return "oKB_Arabic_seen";
		case oKB_Arabic_sheen: return "oKB_Arabic_sheen";
		case oKB_Arabic_sad: return "oKB_Arabic_sad";
		case oKB_Arabic_dad: return "oKB_Arabic_dad";
		case oKB_Arabic_tah: return "oKB_Arabic_tah";
		case oKB_Arabic_zah: return "oKB_Arabic_zah";
		case oKB_Arabic_ain: return "oKB_Arabic_ain";
		case oKB_Arabic_ghain: return "oKB_Arabic_ghain";
		case oKB_Arabic_tatweel: return "oKB_Arabic_tatweel";
		case oKB_Arabic_feh: return "oKB_Arabic_feh";
		case oKB_Arabic_qaf: return "oKB_Arabic_qaf";
		case oKB_Arabic_kaf: return "oKB_Arabic_kaf";
		case oKB_Arabic_lam: return "oKB_Arabic_lam";
		case oKB_Arabic_meem: return "oKB_Arabic_meem";
		case oKB_Arabic_noon: return "oKB_Arabic_noon";
		case oKB_Arabic_ha: return "oKB_Arabic_ha";
		case oKB_Arabic_heh: return "oKB_Arabic_heh";
		case oKB_Arabic_waw: return "oKB_Arabic_waw";
		case oKB_Arabic_alefmaksura: return "oKB_Arabic_alefmaksura";
		case oKB_Arabic_yeh: return "oKB_Arabic_yeh";
		case oKB_Arabic_fathatan: return "oKB_Arabic_fathatan";
		case oKB_Arabic_dammatan: return "oKB_Arabic_dammatan";
		case oKB_Arabic_kasratan: return "oKB_Arabic_kasratan";
		case oKB_Arabic_fatha: return "oKB_Arabic_fatha";
		case oKB_Arabic_damma: return "oKB_Arabic_damma";
		case oKB_Arabic_kasra: return "oKB_Arabic_kasra";
		case oKB_Arabic_shadda: return "oKB_Arabic_shadda";
		case oKB_Arabic_sukun: return "oKB_Arabic_sukun";
		case oKB_Arabic_switch: return "oKB_Arabic_switch";
		#endif/* XK_ARABIC*/
		/*
		 * Cyrillic
		 * Byte 3 = 6
		 */
		#ifdef XK_CYRILLIC
		case oKB_Serbian_dje: return "oKB_Serbian_dje";
		case oKB_Macedonia_gje: return "oKB_Macedonia_gje";
		case oKB_Cyrillic_io: return "oKB_Cyrillic_io";
		case oKB_Ukrainian_ie: return "oKB_Ukrainian_ie";
		case oKB_Ukranian_je: return "oKB_Ukranian_je";
		case oKB_Macedonia_dse: return "oKB_Macedonia_dse";
		case oKB_Ukrainian_i: return "oKB_Ukrainian_i";
		case oKB_Ukranian_i: return "oKB_Ukranian_i";
		case oKB_Ukrainian_yi: return "oKB_Ukrainian_yi";
		case oKB_Ukranian_yi: return "oKB_Ukranian_yi";
		case oKB_Cyrillic_je: return "oKB_Cyrillic_je";
		case oKB_Serbian_je: return "oKB_Serbian_je";
		case oKB_Cyrillic_lje: return "oKB_Cyrillic_lje";
		case oKB_Serbian_lje: return "oKB_Serbian_lje";
		case oKB_Cyrillic_nje: return "oKB_Cyrillic_nje";
		case oKB_Serbian_nje: return "oKB_Serbian_nje";
		case oKB_Serbian_tshe: return "oKB_Serbian_tshe";
		case oKB_Macedonia_kje: return "oKB_Macedonia_kje";
		case oKB_Byelorussian_shortu: return "oKB_Byelorussian_shortu";
		case oKB_Cyrillic_dzhe: return "oKB_Cyrillic_dzhe";
		case oKB_Serbian_dze: return "oKB_Serbian_dze";
		case oKB_numerosign: return "oKB_numerosign";
		case oKB_Serbian_DJE: return "oKB_Serbian_DJE";
		case oKB_Macedonia_GJE: return "oKB_Macedonia_GJE";
		case oKB_Cyrillic_IO: return "oKB_Cyrillic_IO";
		case oKB_Ukrainian_IE: return "oKB_Ukrainian_IE";
		case oKB_Ukranian_JE: return "oKB_Ukranian_JE";
		case oKB_Macedonia_DSE: return "oKB_Macedonia_DSE";
		case oKB_Ukrainian_I: return "oKB_Ukrainian_I";
		case oKB_Ukranian_I: return "oKB_Ukranian_I";
		case oKB_Ukrainian_YI: return "oKB_Ukrainian_YI";
		case oKB_Ukranian_YI: return "oKB_Ukranian_YI";
		case oKB_Cyrillic_JE: return "oKB_Cyrillic_JE";
		case oKB_Serbian_JE: return "oKB_Serbian_JE";
		case oKB_Cyrillic_LJE: return "oKB_Cyrillic_LJE";
		case oKB_Serbian_LJE: return "oKB_Serbian_LJE";
		case oKB_Cyrillic_NJE: return "oKB_Cyrillic_NJE";
		case oKB_Serbian_NJE: return "oKB_Serbian_NJE";
		case oKB_Serbian_TSHE: return "oKB_Serbian_TSHE";
		case oKB_Macedonia_KJE: return "oKB_Macedonia_KJE";
		case oKB_Byelorussian_SHORTU: return "oKB_Byelorussian_SHORTU";
		case oKB_Cyrillic_DZHE: return "oKB_Cyrillic_DZHE";
		case oKB_Serbian_DZE: return "oKB_Serbian_DZE";
		case oKB_Cyrillic_yu: return "oKB_Cyrillic_yu";
		case oKB_Cyrillic_a: return "oKB_Cyrillic_a";
		case oKB_Cyrillic_be: return "oKB_Cyrillic_be";
		case oKB_Cyrillic_tse: return "oKB_Cyrillic_tse";
		case oKB_Cyrillic_de: return "oKB_Cyrillic_de";
		case oKB_Cyrillic_ie: return "oKB_Cyrillic_ie";
		case oKB_Cyrillic_ef: return "oKB_Cyrillic_ef";
		case oKB_Cyrillic_ghe: return "oKB_Cyrillic_ghe";
		case oKB_Cyrillic_ha: return "oKB_Cyrillic_ha";
		case oKB_Cyrillic_i: return "oKB_Cyrillic_i";
		case oKB_Cyrillic_shorti: return "oKB_Cyrillic_shorti";
		case oKB_Cyrillic_ka: return "oKB_Cyrillic_ka";
		case oKB_Cyrillic_el: return "oKB_Cyrillic_el";
		case oKB_Cyrillic_em: return "oKB_Cyrillic_em";
		case oKB_Cyrillic_en: return "oKB_Cyrillic_en";
		case oKB_Cyrillic_o: return "oKB_Cyrillic_o";
		case oKB_Cyrillic_pe: return "oKB_Cyrillic_pe";
		case oKB_Cyrillic_ya: return "oKB_Cyrillic_ya";
		case oKB_Cyrillic_er: return "oKB_Cyrillic_er";
		case oKB_Cyrillic_es: return "oKB_Cyrillic_es";
		case oKB_Cyrillic_te: return "oKB_Cyrillic_te";
		case oKB_Cyrillic_u: return "oKB_Cyrillic_u";
		case oKB_Cyrillic_zhe: return "oKB_Cyrillic_zhe";
		case oKB_Cyrillic_ve: return "oKB_Cyrillic_ve";
		case oKB_Cyrillic_softsign: return "oKB_Cyrillic_softsign";
		case oKB_Cyrillic_yeru: return "oKB_Cyrillic_yeru";
		case oKB_Cyrillic_ze: return "oKB_Cyrillic_ze";
		case oKB_Cyrillic_sha: return "oKB_Cyrillic_sha";
		case oKB_Cyrillic_e: return "oKB_Cyrillic_e";
		case oKB_Cyrillic_shcha: return "oKB_Cyrillic_shcha";
		case oKB_Cyrillic_che: return "oKB_Cyrillic_che";
		case oKB_Cyrillic_hardsign: return "oKB_Cyrillic_hardsign";
		case oKB_Cyrillic_YU: return "oKB_Cyrillic_YU";
		case oKB_Cyrillic_A: return "oKB_Cyrillic_A";
		case oKB_Cyrillic_BE: return "oKB_Cyrillic_BE";
		case oKB_Cyrillic_TSE: return "oKB_Cyrillic_TSE";
		case oKB_Cyrillic_DE: return "oKB_Cyrillic_DE";
		case oKB_Cyrillic_IE: return "oKB_Cyrillic_IE";
		case oKB_Cyrillic_EF: return "oKB_Cyrillic_EF";
		case oKB_Cyrillic_GHE: return "oKB_Cyrillic_GHE";
		case oKB_Cyrillic_HA: return "oKB_Cyrillic_HA";
		case oKB_Cyrillic_I: return "oKB_Cyrillic_I";
		case oKB_Cyrillic_SHORTI: return "oKB_Cyrillic_SHORTI";
		case oKB_Cyrillic_KA: return "oKB_Cyrillic_KA";
		case oKB_Cyrillic_EL: return "oKB_Cyrillic_EL";
		case oKB_Cyrillic_EM: return "oKB_Cyrillic_EM";
		case oKB_Cyrillic_EN: return "oKB_Cyrillic_EN";
		case oKB_Cyrillic_O: return "oKB_Cyrillic_O";
		case oKB_Cyrillic_PE: return "oKB_Cyrillic_PE";
		case oKB_Cyrillic_YA: return "oKB_Cyrillic_YA";
		case oKB_Cyrillic_ER: return "oKB_Cyrillic_ER";
		case oKB_Cyrillic_ES: return "oKB_Cyrillic_ES";
		case oKB_Cyrillic_TE: return "oKB_Cyrillic_TE";
		case oKB_Cyrillic_U: return "oKB_Cyrillic_U";
		case oKB_Cyrillic_ZHE: return "oKB_Cyrillic_ZHE";
		case oKB_Cyrillic_VE: return "oKB_Cyrillic_VE";
		case oKB_Cyrillic_SOFTSIGN: return "oKB_Cyrillic_SOFTSIGN";
		case oKB_Cyrillic_YERU: return "oKB_Cyrillic_YERU";
		case oKB_Cyrillic_ZE: return "oKB_Cyrillic_ZE";
		case oKB_Cyrillic_SHA: return "oKB_Cyrillic_SHA";
		case oKB_Cyrillic_E: return "oKB_Cyrillic_E";
		case oKB_Cyrillic_SHCHA: return "oKB_Cyrillic_SHCHA";
		case oKB_Cyrillic_CHE: return "oKB_Cyrillic_CHE";
		case oKB_Cyrillic_HARDSIGN: return "oKB_Cyrillic_HARDSIGN";
		#endif/* XK_CYRILLIC*/
		/*
		 * Greek
		 * Byte 3 = 7
		 */
		#ifdef XK_GREEK
		case oKB_Greek_ALPHAaccent: return "oKB_Greek_ALPHAaccent";
		case oKB_Greek_EPSILONaccent: return "oKB_Greek_EPSILONaccent";
		case oKB_Greek_ETAaccent: return "oKB_Greek_ETAaccent";
		case oKB_Greek_IOTAaccent: return "oKB_Greek_IOTAaccent";
		case oKB_Greek_IOTAdiaeresis: return "oKB_Greek_IOTAdiaeresis";
		case oKB_Greek_OMICRONaccent: return "oKB_Greek_OMICRONaccent";
		case oKB_Greek_UPSILONaccent: return "oKB_Greek_UPSILONaccent";
		case oKB_Greek_UPSILONdieresis: return "oKB_Greek_UPSILONdieresis";
		case oKB_Greek_OMEGAaccent: return "oKB_Greek_OMEGAaccent";
		case oKB_Greek_accentdieresis: return "oKB_Greek_accentdieresis";
		case oKB_Greek_horizbar: return "oKB_Greek_horizbar";
		case oKB_Greek_alphaaccent: return "oKB_Greek_alphaaccent";
		case oKB_Greek_epsilonaccent: return "oKB_Greek_epsilonaccent";
		case oKB_Greek_etaaccent: return "oKB_Greek_etaaccent";
		case oKB_Greek_iotaaccent: return "oKB_Greek_iotaaccent";
		case oKB_Greek_iotadieresis: return "oKB_Greek_iotadieresis";
		case oKB_Greek_iotaaccentdieresis: return "oKB_Greek_iotaaccentdieresis";
		case oKB_Greek_omicronaccent: return "oKB_Greek_omicronaccent";
		case oKB_Greek_upsilonaccent: return "oKB_Greek_upsilonaccent";
		case oKB_Greek_upsilondieresis: return "oKB_Greek_upsilondieresis";
		case oKB_Greek_upsilonaccentdieresis: return "oKB_Greek_upsilonaccentdieresis";
		case oKB_Greek_omegaaccent: return "oKB_Greek_omegaaccent";
		case oKB_Greek_ALPHA: return "oKB_Greek_ALPHA";
		case oKB_Greek_BETA: return "oKB_Greek_BETA";
		case oKB_Greek_GAMMA: return "oKB_Greek_GAMMA";
		case oKB_Greek_DELTA: return "oKB_Greek_DELTA";
		case oKB_Greek_EPSILON: return "oKB_Greek_EPSILON";
		case oKB_Greek_ZETA: return "oKB_Greek_ZETA";
		case oKB_Greek_ETA: return "oKB_Greek_ETA";
		case oKB_Greek_THETA: return "oKB_Greek_THETA";
		case oKB_Greek_IOTA: return "oKB_Greek_IOTA";
		case oKB_Greek_KAPPA: return "oKB_Greek_KAPPA";
		case oKB_Greek_LAMDA: return "oKB_Greek_LAMDA";
		case oKB_Greek_LAMBDA: return "oKB_Greek_LAMBDA";
		case oKB_Greek_MU: return "oKB_Greek_MU";
		case oKB_Greek_NU: return "oKB_Greek_NU";
		case oKB_Greek_XI: return "oKB_Greek_XI";
		case oKB_Greek_OMICRON: return "oKB_Greek_OMICRON";
		case oKB_Greek_PI: return "oKB_Greek_PI";
		case oKB_Greek_RHO: return "oKB_Greek_RHO";
		case oKB_Greek_SIGMA: return "oKB_Greek_SIGMA";
		case oKB_Greek_TAU: return "oKB_Greek_TAU";
		case oKB_Greek_UPSILON: return "oKB_Greek_UPSILON";
		case oKB_Greek_PHI: return "oKB_Greek_PHI";
		case oKB_Greek_CHI: return "oKB_Greek_CHI";
		case oKB_Greek_PSI: return "oKB_Greek_PSI";
		case oKB_Greek_OMEGA: return "oKB_Greek_OMEGA";
		case oKB_Greek_alpha: return "oKB_Greek_alpha";
		case oKB_Greek_beta: return "oKB_Greek_beta";
		case oKB_Greek_gamma: return "oKB_Greek_gamma";
		case oKB_Greek_delta: return "oKB_Greek_delta";
		case oKB_Greek_epsilon: return "oKB_Greek_epsilon";
		case oKB_Greek_zeta: return "oKB_Greek_zeta";
		case oKB_Greek_eta: return "oKB_Greek_eta";
		case oKB_Greek_theta: return "oKB_Greek_theta";
		case oKB_Greek_iota: return "oKB_Greek_iota";
		case oKB_Greek_kappa: return "oKB_Greek_kappa";
		case oKB_Greek_lamda: return "oKB_Greek_lamda";
		case oKB_Greek_lambda: return "oKB_Greek_lambda";
		case oKB_Greek_mu: return "oKB_Greek_mu";
		case oKB_Greek_nu: return "oKB_Greek_nu";
		case oKB_Greek_xi: return "oKB_Greek_xi";
		case oKB_Greek_omicron: return "oKB_Greek_omicron";
		case oKB_Greek_pi: return "oKB_Greek_pi";
		case oKB_Greek_rho: return "oKB_Greek_rho";
		case oKB_Greek_sigma: return "oKB_Greek_sigma";
		case oKB_Greek_finalsmallsigma: return "oKB_Greek_finalsmallsigma";
		case oKB_Greek_tau: return "oKB_Greek_tau";
		case oKB_Greek_upsilon: return "oKB_Greek_upsilon";
		case oKB_Greek_phi: return "oKB_Greek_phi";
		case oKB_Greek_chi: return "oKB_Greek_chi";
		case oKB_Greek_psi: return "oKB_Greek_psi";
		case oKB_Greek_omega: return "oKB_Greek_omega";
		case oKB_Greek_switch: return "oKB_Greek_switch";
		#endif/* XK_GREEK*/
		/*
		 * Technical
		 * Byte 3 = 8
		 */
		#ifdef XK_TECHNICAL
		case oKB_leftradical: return "oKB_leftradical";
		case oKB_topleftradical: return "oKB_topleftradical";
		case oKB_horizconnector: return "oKB_horizconnector";
		case oKB_topintegral: return "oKB_topintegral";
		case oKB_botintegral: return "oKB_botintegral";
		case oKB_vertconnector: return "oKB_vertconnector";
		case oKB_topleftsqbracket: return "oKB_topleftsqbracket";
		case oKB_botleftsqbracket: return "oKB_botleftsqbracket";
		case oKB_toprightsqbracket: return "oKB_toprightsqbracket";
		case oKB_botrightsqbracket: return "oKB_botrightsqbracket";
		case oKB_topleftparens: return "oKB_topleftparens";
		case oKB_botleftparens: return "oKB_botleftparens";
		case oKB_toprightparens: return "oKB_toprightparens";
		case oKB_botrightparens: return "oKB_botrightparens";
		case oKB_leftmiddlecurlybrace: return "oKB_leftmiddlecurlybrace";
		case oKB_rightmiddlecurlybrace: return "oKB_rightmiddlecurlybrace";
		case oKB_topleftsummation: return "oKB_topleftsummation";
		case oKB_botleftsummation: return "oKB_botleftsummation";
		case oKB_topvertsummationconnector: return "oKB_topvertsummationconnector";
		case oKB_botvertsummationconnector: return "oKB_botvertsummationconnector";
		case oKB_toprightsummation: return "oKB_toprightsummation";
		case oKB_botrightsummation: return "oKB_botrightsummation";
		case oKB_rightmiddlesummation: return "oKB_rightmiddlesummation";
		case oKB_lessthanequal: return "oKB_lessthanequal";
		case oKB_notequal: return "oKB_notequal";
		case oKB_greaterthanequal: return "oKB_greaterthanequal";
		case oKB_integral: return "oKB_integral";
		case oKB_therefore: return "oKB_therefore";
		case oKB_variation: return "oKB_variation";
		case oKB_infinity: return "oKB_infinity";
		case oKB_nabla: return "oKB_nabla";
		case oKB_approximate: return "oKB_approximate";
		case oKB_similarequal: return "oKB_similarequal";
		case oKB_ifonlyif: return "oKB_ifonlyif";
		case oKB_implies: return "oKB_implies";
		case oKB_identical: return "oKB_identical";
		case oKB_radical: return "oKB_radical";
		case oKB_includedin: return "oKB_includedin";
		case oKB_includes: return "oKB_includes";
		case oKB_intersection: return "oKB_intersection";
		case oKB_union: return "oKB_union";
		case oKB_logicaland: return "oKB_logicaland";
		case oKB_logicalor: return "oKB_logicalor";
		case oKB_partialderivative: return "oKB_partialderivative";
		case oKB_function: return "oKB_function";
		case oKB_leftarrow: return "oKB_leftarrow";
		case oKB_uparrow: return "oKB_uparrow";
		case oKB_rightarrow: return "oKB_rightarrow";
		case oKB_downarrow: return "oKB_downarrow";
		#endif/* XK_TECHNICAL*/
		/*
		 * Special
		 * Byte 3 = 9
		 */
		#ifdef XK_SPECIAL
		case oKB_blank: return "oKB_blank";
		case oKB_soliddiamond: return "oKB_soliddiamond";
		case oKB_checkerboard: return "oKB_checkerboard";
		case oKB_ht: return "oKB_ht";
		case oKB_ff: return "oKB_ff";
		case oKB_cr: return "oKB_cr";
		case oKB_lf: return "oKB_lf";
		case oKB_nl: return "oKB_nl";
		case oKB_vt: return "oKB_vt";
		case oKB_lowrightcorner: return "oKB_lowrightcorner";
		case oKB_uprightcorner: return "oKB_uprightcorner";
		case oKB_upleftcorner: return "oKB_upleftcorner";
		case oKB_lowleftcorner: return "oKB_lowleftcorner";
		case oKB_crossinglines: return "oKB_crossinglines";
		case oKB_horizlinescan1: return "oKB_horizlinescan1";
		case oKB_horizlinescan3: return "oKB_horizlinescan3";
		case oKB_horizlinescan5: return "oKB_horizlinescan5";
		case oKB_horizlinescan7: return "oKB_horizlinescan7";
		case oKB_horizlinescan9: return "oKB_horizlinescan9";
		case oKB_leftt: return "oKB_leftt";
		case oKB_rightt: return "oKB_rightt";
		case oKB_bott: return "oKB_bott";
		case oKB_topt: return "oKB_topt";
		case oKB_vertbar: return "oKB_vertbar";
		#endif/* XK_SPECIAL*/
		/*
		 * Publishing
		 * Byte 3 = a
		 */
		#ifdef XK_PUBLISHING
		case oKB_emspace: return "oKB_emspace";
		case oKB_enspace: return "oKB_enspace";
		case oKB_em3space: return "oKB_em3space";
		case oKB_em4space: return "oKB_em4space";
		case oKB_digitspace: return "oKB_digitspace";
		case oKB_punctspace: return "oKB_punctspace";
		case oKB_thinspace: return "oKB_thinspace";
		case oKB_hairspace: return "oKB_hairspace";
		case oKB_emdash: return "oKB_emdash";
		case oKB_endash: return "oKB_endash";
		case oKB_signifblank: return "oKB_signifblank";
		case oKB_ellipsis: return "oKB_ellipsis";
		case oKB_doubbaselinedot: return "oKB_doubbaselinedot";
		case oKB_onethird: return "oKB_onethird";
		case oKB_twothirds: return "oKB_twothirds";
		case oKB_onefifth: return "oKB_onefifth";
		case oKB_twofifths: return "oKB_twofifths";
		case oKB_threefifths: return "oKB_threefifths";
		case oKB_fourfifths: return "oKB_fourfifths";
		case oKB_onesixth: return "oKB_onesixth";
		case oKB_fivesixths: return "oKB_fivesixths";
		case oKB_careof: return "oKB_careof";
		case oKB_figdash: return "oKB_figdash";
		case oKB_leftanglebracket: return "oKB_leftanglebracket";
		case oKB_decimalpoint: return "oKB_decimalpoint";
		case oKB_rightanglebracket: return "oKB_rightanglebracket";
		case oKB_marker: return "oKB_marker";
		case oKB_oneeighth: return "oKB_oneeighth";
		case oKB_threeeighths: return "oKB_threeeighths";
		case oKB_fiveeighths: return "oKB_fiveeighths";
		case oKB_seveneighths: return "oKB_seveneighths";
		case oKB_trademark: return "oKB_trademark";
		case oKB_signaturemark: return "oKB_signaturemark";
		case oKB_trademarkincircle: return "oKB_trademarkincircle";
		case oKB_leftopentriangle: return "oKB_leftopentriangle";
		case oKB_rightopentriangle: return "oKB_rightopentriangle";
		case oKB_emopencircle: return "oKB_emopencircle";
		case oKB_emopenrectangle: return "oKB_emopenrectangle";
		case oKB_leftsinglequotemark: return "oKB_leftsinglequotemark";
		case oKB_rightsinglequotemark: return "oKB_rightsinglequotemark";
		case oKB_leftdoublequotemark: return "oKB_leftdoublequotemark";
		case oKB_rightdoublequotemark: return "oKB_rightdoublequotemark";
		case oKB_prescription: return "oKB_prescription";
		case oKB_minutes: return "oKB_minutes";
		case oKB_seconds: return "oKB_seconds";
		case oKB_latincross: return "oKB_latincross";
		case oKB_hexagram: return "oKB_hexagram";
		case oKB_filledrectbullet: return "oKB_filledrectbullet";
		case oKB_filledlefttribullet: return "oKB_filledlefttribullet";
		case oKB_filledrighttribullet: return "oKB_filledrighttribullet";
		case oKB_emfilledcircle: return "oKB_emfilledcircle";
		case oKB_emfilledrect: return "oKB_emfilledrect";
		case oKB_enopencircbullet: return "oKB_enopencircbullet";
		case oKB_enopensquarebullet: return "oKB_enopensquarebullet";
		case oKB_openrectbullet: return "oKB_openrectbullet";
		case oKB_opentribulletup: return "oKB_opentribulletup";
		case oKB_opentribulletdown: return "oKB_opentribulletdown";
		case oKB_openstar: return "oKB_openstar";
		case oKB_enfilledcircbullet: return "oKB_enfilledcircbullet";
		case oKB_enfilledsqbullet: return "oKB_enfilledsqbullet";
		case oKB_filledtribulletup: return "oKB_filledtribulletup";
		case oKB_filledtribulletdown: return "oKB_filledtribulletdown";
		case oKB_leftpointer: return "oKB_leftpointer";
		case oKB_rightpointer: return "oKB_rightpointer";
		case oKB_club: return "oKB_club";
		case oKB_diamond: return "oKB_diamond";
		case oKB_heart: return "oKB_heart";
		case oKB_maltesecross: return "oKB_maltesecross";
		case oKB_dagger: return "oKB_dagger";
		case oKB_doubledagger: return "oKB_doubledagger";
		case oKB_checkmark: return "oKB_checkmark";
		case oKB_ballotcross: return "oKB_ballotcross";
		case oKB_musicalsharp: return "oKB_musicalsharp";
		case oKB_musicalflat: return "oKB_musicalflat";
		case oKB_malesymbol: return "oKB_malesymbol";
		case oKB_femalesymbol: return "oKB_femalesymbol";
		case oKB_telephone: return "oKB_telephone";
		case oKB_telephonerecorder: return "oKB_telephonerecorder";
		case oKB_phonographcopyright: return "oKB_phonographcopyright";
		case oKB_caret: return "oKB_caret";
		case oKB_singlelowquotemark: return "oKB_singlelowquotemark";
		case oKB_doublelowquotemark: return "oKB_doublelowquotemark";
		case oKB_cursor: return "oKB_cursor";
		#endif/* XK_PUBLISHING*/
		/*
		 * APL
		 * Byte 3 = b
		 */
		#ifdef XK_APL
		case oKB_leftcaret: return "oKB_leftcaret";
		case oKB_rightcaret: return "oKB_rightcaret";
		case oKB_downcaret: return "oKB_downcaret";
		case oKB_upcaret: return "oKB_upcaret";
		case oKB_overbar: return "oKB_overbar";
		case oKB_downtack: return "oKB_downtack";
		case oKB_upshoe: return "oKB_upshoe";
		case oKB_downstile: return "oKB_downstile";
		case oKB_underbar: return "oKB_underbar";
		case oKB_jot: return "oKB_jot";
		case oKB_quad: return "oKB_quad";
		case oKB_uptack: return "oKB_uptack";
		case oKB_circle: return "oKB_circle";
		case oKB_upstile: return "oKB_upstile";
		case oKB_downshoe: return "oKB_downshoe";
		case oKB_rightshoe: return "oKB_rightshoe";
		case oKB_leftshoe: return "oKB_leftshoe";
		case oKB_lefttack: return "oKB_lefttack";
		case oKB_righttack: return "oKB_righttack";
		#endif/* XK_APL*/
		/*
		 * Hebrew
		 * Byte 3 = c
		 */
		#ifdef XK_HEBREW
		case oKB_hebrew_doublelowline: return "oKB_hebrew_doublelowline";
		case oKB_hebrew_aleph: return "oKB_hebrew_aleph";
		case oKB_hebrew_bet: return "oKB_hebrew_bet";
		case oKB_hebrew_beth: return "oKB_hebrew_beth";
		case oKB_hebrew_gimel: return "oKB_hebrew_gimel";
		case oKB_hebrew_gimmel: return "oKB_hebrew_gimmel";
		case oKB_hebrew_dalet: return "oKB_hebrew_dalet";
		case oKB_hebrew_daleth: return "oKB_hebrew_daleth";
		case oKB_hebrew_he: return "oKB_hebrew_he";
		case oKB_hebrew_waw: return "oKB_hebrew_waw";
		case oKB_hebrew_zain: return "oKB_hebrew_zain";
		case oKB_hebrew_zayin: return "oKB_hebrew_zayin";
		case oKB_hebrew_chet: return "oKB_hebrew_chet";
		case oKB_hebrew_het: return "oKB_hebrew_het";
		case oKB_hebrew_tet: return "oKB_hebrew_tet";
		case oKB_hebrew_teth: return "oKB_hebrew_teth";
		case oKB_hebrew_yod: return "oKB_hebrew_yod";
		case oKB_hebrew_finalkaph: return "oKB_hebrew_finalkaph";
		case oKB_hebrew_kaph: return "oKB_hebrew_kaph";
		case oKB_hebrew_lamed: return "oKB_hebrew_lamed";
		case oKB_hebrew_finalmem: return "oKB_hebrew_finalmem";
		case oKB_hebrew_mem: return "oKB_hebrew_mem";
		case oKB_hebrew_finalnun: return "oKB_hebrew_finalnun";
		case oKB_hebrew_nun: return "oKB_hebrew_nun";
		case oKB_hebrew_samech: return "oKB_hebrew_samech";
		case oKB_hebrew_samekh: return "oKB_hebrew_samekh";
		case oKB_hebrew_ayin: return "oKB_hebrew_ayin";
		case oKB_hebrew_finalpe: return "oKB_hebrew_finalpe";
		case oKB_hebrew_pe: return "oKB_hebrew_pe";
		case oKB_hebrew_finalzade: return "oKB_hebrew_finalzade";
		case oKB_hebrew_finalzadi: return "oKB_hebrew_finalzadi";
		case oKB_hebrew_zade: return "oKB_hebrew_zade";
		case oKB_hebrew_zadi: return "oKB_hebrew_zadi";
		case oKB_hebrew_qoph: return "oKB_hebrew_qoph";
		case oKB_hebrew_kuf: return "oKB_hebrew_kuf";
		case oKB_hebrew_resh: return "oKB_hebrew_resh";
		case oKB_hebrew_shin: return "oKB_hebrew_shin";
		case oKB_hebrew_taw: return "oKB_hebrew_taw";
		case oKB_hebrew_taf: return "oKB_hebrew_taf";
		case oKB_Hebrew_switch: return "oKB_Hebrew_switch";
		#endif/* XK_HEBREW*/ 
		/*
		 * Thai
		 * Byte 3 = d
		 */ 
		#ifdef XK_THAI
		case oKB_Thai_kokai: return "oKB_Thai_kokai";
		case oKB_Thai_khokhai: return "oKB_Thai_khokhai";
		case oKB_Thai_khokhuat: return "oKB_Thai_khokhuat";
		case oKB_Thai_khokhwai: return "oKB_Thai_khokhwai";
		case oKB_Thai_khokhon: return "oKB_Thai_khokhon";
		case oKB_Thai_khorakhang: return "oKB_Thai_khorakhang";
		case oKB_Thai_ngongu: return "oKB_Thai_ngongu";
		case oKB_Thai_chochan: return "oKB_Thai_chochan";
		case oKB_Thai_choching: return "oKB_Thai_choching";
		case oKB_Thai_chochang: return "oKB_Thai_chochang";
		case oKB_Thai_soso: return "oKB_Thai_soso";
		case oKB_Thai_chochoe: return "oKB_Thai_chochoe";
		case oKB_Thai_yoying: return "oKB_Thai_yoying";
		case oKB_Thai_dochada: return "oKB_Thai_dochada";
		case oKB_Thai_topatak: return "oKB_Thai_topatak";
		case oKB_Thai_thothan: return "oKB_Thai_thothan";
		case oKB_Thai_thonangmontho: return "oKB_Thai_thonangmontho";
		case oKB_Thai_thophuthao: return "oKB_Thai_thophuthao";
		case oKB_Thai_nonen: return "oKB_Thai_nonen";
		case oKB_Thai_dodek: return "oKB_Thai_dodek";
		case oKB_Thai_totao: return "oKB_Thai_totao";
		case oKB_Thai_thothung: return "oKB_Thai_thothung";
		case oKB_Thai_thothahan: return "oKB_Thai_thothahan";
		case oKB_Thai_thothong: return "oKB_Thai_thothong";
		case oKB_Thai_nonu: return "oKB_Thai_nonu";
		case oKB_Thai_bobaimai: return "oKB_Thai_bobaimai";
		case oKB_Thai_popla: return "oKB_Thai_popla";
		case oKB_Thai_phophung: return "oKB_Thai_phophung";
		case oKB_Thai_fofa: return "oKB_Thai_fofa";
		case oKB_Thai_phophan: return "oKB_Thai_phophan";
		case oKB_Thai_fofan: return "oKB_Thai_fofan";
		case oKB_Thai_phosamphao: return "oKB_Thai_phosamphao";
		case oKB_Thai_moma: return "oKB_Thai_moma";
		case oKB_Thai_yoyak: return "oKB_Thai_yoyak";
		case oKB_Thai_rorua: return "oKB_Thai_rorua";
		case oKB_Thai_ru: return "oKB_Thai_ru";
		case oKB_Thai_loling: return "oKB_Thai_loling";
		case oKB_Thai_lu: return "oKB_Thai_lu";
		case oKB_Thai_wowaen: return "oKB_Thai_wowaen";
		case oKB_Thai_sosala: return "oKB_Thai_sosala";
		case oKB_Thai_sorusi: return "oKB_Thai_sorusi";
		case oKB_Thai_sosua: return "oKB_Thai_sosua";
		case oKB_Thai_hohip: return "oKB_Thai_hohip";
		case oKB_Thai_lochula: return "oKB_Thai_lochula";
		case oKB_Thai_oang: return "oKB_Thai_oang";
		case oKB_Thai_honokhuk: return "oKB_Thai_honokhuk";
		case oKB_Thai_paiyannoi: return "oKB_Thai_paiyannoi";
		case oKB_Thai_saraa: return "oKB_Thai_saraa";
		case oKB_Thai_maihanakat: return "oKB_Thai_maihanakat";
		case oKB_Thai_saraaa: return "oKB_Thai_saraaa";
		case oKB_Thai_saraam: return "oKB_Thai_saraam";
		case oKB_Thai_sarai: return "oKB_Thai_sarai";
		case oKB_Thai_saraii: return "oKB_Thai_saraii";
		case oKB_Thai_saraue: return "oKB_Thai_saraue";
		case oKB_Thai_sarauee: return "oKB_Thai_sarauee";
		case oKB_Thai_sarau: return "oKB_Thai_sarau";
		case oKB_Thai_sarauu: return "oKB_Thai_sarauu";
		case oKB_Thai_phinthu: return "oKB_Thai_phinthu";
		case oKB_Thai_maihanakat_maitho: return "oKB_Thai_maihanakat_maitho";
		case oKB_Thai_baht: return "oKB_Thai_baht";
		case oKB_Thai_sarae: return "oKB_Thai_sarae";
		case oKB_Thai_saraae: return "oKB_Thai_saraae";
		case oKB_Thai_sarao: return "oKB_Thai_sarao";
		case oKB_Thai_saraaimaimuan: return "oKB_Thai_saraaimaimuan";
		case oKB_Thai_saraaimaimalai: return "oKB_Thai_saraaimaimalai";
		case oKB_Thai_lakkhangyao: return "oKB_Thai_lakkhangyao";
		case oKB_Thai_maiyamok: return "oKB_Thai_maiyamok";
		case oKB_Thai_maitaikhu: return "oKB_Thai_maitaikhu";
		case oKB_Thai_maiek: return "oKB_Thai_maiek";
		case oKB_Thai_maitho: return "oKB_Thai_maitho";
		case oKB_Thai_maitri: return "oKB_Thai_maitri";
		case oKB_Thai_maichattawa: return "oKB_Thai_maichattawa";
		case oKB_Thai_thanthakhat: return "oKB_Thai_thanthakhat";
		case oKB_Thai_nikhahit: return "oKB_Thai_nikhahit";
		case oKB_Thai_leksun: return "oKB_Thai_leksun";
		case oKB_Thai_leknung: return "oKB_Thai_leknung";
		case oKB_Thai_leksong: return "oKB_Thai_leksong";
		case oKB_Thai_leksam: return "oKB_Thai_leksam";
		case oKB_Thai_leksi: return "oKB_Thai_leksi";
		case oKB_Thai_lekha: return "oKB_Thai_lekha";
		case oKB_Thai_lekhok: return "oKB_Thai_lekhok";
		case oKB_Thai_lekchet: return "oKB_Thai_lekchet";
		case oKB_Thai_lekpaet: return "oKB_Thai_lekpaet";
		case oKB_Thai_lekkao: return "oKB_Thai_lekkao";
		#endif/* XK_THAI*/
		/*
		 * Korean
		 * Byte 3 = e
		 */
		#ifdef XK_KOREAN
		case oKB_Hangul: return "oKB_Hangul";
		case oKB_Hangul_Start: return "oKB_Hangul_Start";
		case oKB_Hangul_End: return "oKB_Hangul_End";
		case oKB_Hangul_Hanja: return "oKB_Hangul_Hanja";
		case oKB_Hangul_Jamo: return "oKB_Hangul_Jamo";
		case oKB_Hangul_Romaja: return "oKB_Hangul_Romaja";
		case oKB_Hangul_Codeinput: return "oKB_Hangul_Codeinput";
		case oKB_Hangul_Jeonja: return "oKB_Hangul_Jeonja";
		case oKB_Hangul_Banja: return "oKB_Hangul_Banja";
		case oKB_Hangul_PreHanja: return "oKB_Hangul_PreHanja";
		case oKB_Hangul_PostHanja: return "oKB_Hangul_PostHanja";
		case oKB_Hangul_SingleCandidate: return "oKB_Hangul_SingleCandidate";
		case oKB_Hangul_MultipleCandidate: return "oKB_Hangul_MultipleCandidate";
		case oKB_Hangul_PreviousCandidate: return "oKB_Hangul_PreviousCandidate";
		case oKB_Hangul_Special: return "oKB_Hangul_Special";
		case oKB_Hangul_switch: return "oKB_Hangul_switch";
		/* Hangul Consonant Characters*/
		case oKB_Hangul_Kiyeog: return "oKB_Hangul_Kiyeog";
		case oKB_Hangul_SsangKiyeog: return "oKB_Hangul_SsangKiyeog";
		case oKB_Hangul_KiyeogSios: return "oKB_Hangul_KiyeogSios";
		case oKB_Hangul_Nieun: return "oKB_Hangul_Nieun";
		case oKB_Hangul_NieunJieuj: return "oKB_Hangul_NieunJieuj";
		case oKB_Hangul_NieunHieuh: return "oKB_Hangul_NieunHieuh";
		case oKB_Hangul_Dikeud: return "oKB_Hangul_Dikeud";
		case oKB_Hangul_SsangDikeud: return "oKB_Hangul_SsangDikeud";
		case oKB_Hangul_Rieul: return "oKB_Hangul_Rieul";
		case oKB_Hangul_RieulKiyeog: return "oKB_Hangul_RieulKiyeog";
		case oKB_Hangul_RieulMieum: return "oKB_Hangul_RieulMieum";
		case oKB_Hangul_RieulPieub: return "oKB_Hangul_RieulPieub";
		case oKB_Hangul_RieulSios: return "oKB_Hangul_RieulSios";
		case oKB_Hangul_RieulTieut: return "oKB_Hangul_RieulTieut";
		case oKB_Hangul_RieulPhieuf: return "oKB_Hangul_RieulPhieuf";
		case oKB_Hangul_RieulHieuh: return "oKB_Hangul_RieulHieuh";
		case oKB_Hangul_Mieum: return "oKB_Hangul_Mieum";
		case oKB_Hangul_Pieub: return "oKB_Hangul_Pieub";
		case oKB_Hangul_SsangPieub: return "oKB_Hangul_SsangPieub";
		case oKB_Hangul_PieubSios: return "oKB_Hangul_PieubSios";
		case oKB_Hangul_Sios: return "oKB_Hangul_Sios";
		case oKB_Hangul_SsangSios: return "oKB_Hangul_SsangSios";
		case oKB_Hangul_Ieung: return "oKB_Hangul_Ieung";
		case oKB_Hangul_Jieuj: return "oKB_Hangul_Jieuj";
		case oKB_Hangul_SsangJieuj: return "oKB_Hangul_SsangJieuj";
		case oKB_Hangul_Cieuc: return "oKB_Hangul_Cieuc";
		case oKB_Hangul_Khieuq: return "oKB_Hangul_Khieuq";
		case oKB_Hangul_Tieut: return "oKB_Hangul_Tieut";
		case oKB_Hangul_Phieuf: return "oKB_Hangul_Phieuf";
		case oKB_Hangul_Hieuh: return "oKB_Hangul_Hieuh"; 
		/* Hangul Vowel Characters*/
		case oKB_Hangul_A: return "oKB_Hangul_A";
		case oKB_Hangul_AE: return "oKB_Hangul_AE";
		case oKB_Hangul_YA: return "oKB_Hangul_YA";
		case oKB_Hangul_YAE: return "oKB_Hangul_YAE";
		case oKB_Hangul_EO: return "oKB_Hangul_EO";
		case oKB_Hangul_E: return "oKB_Hangul_E";
		case oKB_Hangul_YEO: return "oKB_Hangul_YEO";
		case oKB_Hangul_YE: return "oKB_Hangul_YE";
		case oKB_Hangul_O: return "oKB_Hangul_O";
		case oKB_Hangul_WA: return "oKB_Hangul_WA";
		case oKB_Hangul_WAE: return "oKB_Hangul_WAE";
		case oKB_Hangul_OE: return "oKB_Hangul_OE";
		case oKB_Hangul_YO: return "oKB_Hangul_YO";
		case oKB_Hangul_U: return "oKB_Hangul_U";
		case oKB_Hangul_WEO: return "oKB_Hangul_WEO";
		case oKB_Hangul_WE: return "oKB_Hangul_WE";
		case oKB_Hangul_WI: return "oKB_Hangul_WI";
		case oKB_Hangul_YU: return "oKB_Hangul_YU";
		case oKB_Hangul_EU: return "oKB_Hangul_EU";
		case oKB_Hangul_YI: return "oKB_Hangul_YI";
		case oKB_Hangul_I: return "oKB_Hangul_I";
		/* Hangul syllable-final (JongSeong) Characters*/
		case oKB_Hangul_J_Kiyeog: return "oKB_Hangul_J_Kiyeog";
		case oKB_Hangul_J_SsangKiyeog: return "oKB_Hangul_J_SsangKiyeog";
		case oKB_Hangul_J_KiyeogSios: return "oKB_Hangul_J_KiyeogSios";
		case oKB_Hangul_J_Nieun: return "oKB_Hangul_J_Nieun";
		case oKB_Hangul_J_NieunJieuj: return "oKB_Hangul_J_NieunJieuj";
		case oKB_Hangul_J_NieunHieuh: return "oKB_Hangul_J_NieunHieuh";
		case oKB_Hangul_J_Dikeud: return "oKB_Hangul_J_Dikeud";
		case oKB_Hangul_J_Rieul: return "oKB_Hangul_J_Rieul";
		case oKB_Hangul_J_RieulKiyeog: return "oKB_Hangul_J_RieulKiyeog";
		case oKB_Hangul_J_RieulMieum: return "oKB_Hangul_J_RieulMieum";
		case oKB_Hangul_J_RieulPieub: return "oKB_Hangul_J_RieulPieub";
		case oKB_Hangul_J_RieulSios: return "oKB_Hangul_J_RieulSios";
		case oKB_Hangul_J_RieulTieut: return "oKB_Hangul_J_RieulTieut";
		case oKB_Hangul_J_RieulPhieuf: return "oKB_Hangul_J_RieulPhieuf";
		case oKB_Hangul_J_RieulHieuh: return "oKB_Hangul_J_RieulHieuh";
		case oKB_Hangul_J_Mieum: return "oKB_Hangul_J_Mieum";
		case oKB_Hangul_J_Pieub: return "oKB_Hangul_J_Pieub";
		case oKB_Hangul_J_PieubSios: return "oKB_Hangul_J_PieubSios";
		case oKB_Hangul_J_Sios: return "oKB_Hangul_J_Sios";
		case oKB_Hangul_J_SsangSios: return "oKB_Hangul_J_SsangSios";
		case oKB_Hangul_J_Ieung: return "oKB_Hangul_J_Ieung";
		case oKB_Hangul_J_Jieuj: return "oKB_Hangul_J_Jieuj";
		case oKB_Hangul_J_Cieuc: return "oKB_Hangul_J_Cieuc";
		case oKB_Hangul_J_Khieuq: return "oKB_Hangul_J_Khieuq";
		case oKB_Hangul_J_Tieut: return "oKB_Hangul_J_Tieut";
		case oKB_Hangul_J_Phieuf: return "oKB_Hangul_J_Phieuf";
		case oKB_Hangul_J_Hieuh: return "oKB_Hangul_J_Hieuh";
		/* Ancient Hangul Consonant Characters*/
		case oKB_Hangul_RieulYeorinHieuh: return "oKB_Hangul_RieulYeorinHieuh";
		case oKB_Hangul_SunkyeongeumMieum: return "oKB_Hangul_SunkyeongeumMieum";
		case oKB_Hangul_SunkyeongeumPieub: return "oKB_Hangul_SunkyeongeumPieub";
		case oKB_Hangul_PanSios: return "oKB_Hangul_PanSios";
		case oKB_Hangul_KkogjiDalrinIeung: return "oKB_Hangul_KkogjiDalrinIeung";
		case oKB_Hangul_SunkyeongeumPhieuf: return "oKB_Hangul_SunkyeongeumPhieuf";
		case oKB_Hangul_YeorinHieuh: return "oKB_Hangul_YeorinHieuh";
		/* Ancient Hangul Vowel Characters*/
		case oKB_Hangul_AraeA: return "oKB_Hangul_AraeA";
		case oKB_Hangul_AraeAE: return "oKB_Hangul_AraeAE";
		/* Ancient Hangul syllable-final (JongSeong) Characters*/
		case oKB_Hangul_J_PanSios: return "oKB_Hangul_J_PanSios";
		case oKB_Hangul_J_KkogjiDalrinIeung: return "oKB_Hangul_J_KkogjiDalrinIeung";
		case oKB_Hangul_J_YeorinHieuh: return "oKB_Hangul_J_YeorinHieuh";
		/* Korean currency symbol*/
		case oKB_Korean_Won: return "oKB_Korean_Won";
		#endif/* XK_KOREAN*/
		default: return "Key Not Found";
	}
}