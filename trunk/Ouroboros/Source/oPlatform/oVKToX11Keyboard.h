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
#pragma once
#ifndef oVKToX11Keyboard_h
#define oVKToX11Keyboard_h

#include <oBasis/oX11KeyboardSymbols.h>

const int MAX_TOUCHES = 10;

typedef struct X11KeyMapping_Struct
{
	UINT VKCode;
	UINT XCode;
} X11KeyMapping;

static const X11KeyMapping keymap[] = 
{
  {VK_BACK, oKB_BackSpace},
  {VK_TAB, oKB_Tab},
  {VK_CLEAR, oKB_Clear},
  {VK_RETURN, oKB_Return},
  {VK_LSHIFT, oKB_Shift_L},
  {VK_RSHIFT, oKB_Shift_R},
  {VK_SHIFT, oKB_Shift_L},
  {VK_LCONTROL, oKB_Control_L},
  {VK_RCONTROL, oKB_Control_R},
  {VK_CONTROL, oKB_Control_L},
  {VK_LMENU, oKB_Alt_L},
  {VK_RMENU, oKB_Alt_R},
  {VK_MENU, oKB_Alt_L},
	{VK_MENU, oKB_Menu},
  {VK_PAUSE, oKB_Pause},
  {VK_CAPITAL, oKB_Caps_Lock},
  {VK_ESCAPE, oKB_Escape},
  {VK_SPACE, oKB_space},
  {VK_PRIOR, oKB_Page_Up},
  {VK_NEXT, oKB_Page_Down},
  {VK_END, oKB_End},
  {VK_HOME, oKB_Home},
  {VK_LEFT, oKB_Left},
  {VK_UP, oKB_Up},
  {VK_RIGHT, oKB_Right},
  {VK_DOWN, oKB_Down},
  {VK_SELECT, oKB_Select},
  {VK_EXECUTE, oKB_Execute},
  {VK_SNAPSHOT, oKB_Print},
  {VK_INSERT, oKB_Insert},
  {VK_DELETE, oKB_Delete},
  {VK_HELP, oKB_Help},
  {VK_NUMPAD0, oKB_KP_0},
  {VK_NUMPAD1, oKB_KP_1},
  {VK_NUMPAD2, oKB_KP_2},
  {VK_NUMPAD3, oKB_KP_3},
  {VK_NUMPAD4, oKB_KP_4},
  {VK_NUMPAD5, oKB_KP_5},
  {VK_NUMPAD6, oKB_KP_6},
  {VK_NUMPAD7, oKB_KP_7},
  {VK_NUMPAD8, oKB_KP_8},
  {VK_NUMPAD9, oKB_KP_9},
  {VK_MULTIPLY, oKB_KP_Multiply},
  {VK_ADD, oKB_KP_Add},
  {VK_SEPARATOR, oKB_KP_Separator},   // often comma
  {VK_SUBTRACT, oKB_KP_Subtract},
  {VK_DECIMAL, oKB_KP_Decimal},
  {VK_DIVIDE, oKB_KP_Divide},
  {VK_F1, oKB_F1},
  {VK_F2, oKB_F2},
  {VK_F3, oKB_F3},
  {VK_F4, oKB_F4},
  {VK_F5, oKB_F5},
  {VK_F6, oKB_F6},
  {VK_F7, oKB_F7},
  {VK_F8, oKB_F8},
  {VK_F9, oKB_F9},
  {VK_F10, oKB_F10},
  {VK_F11, oKB_F11},
  {VK_F12, oKB_F12},
  {VK_F13, oKB_F13},
  {VK_F14, oKB_F14},
  {VK_F15, oKB_F15},
  {VK_F16, oKB_F16},
  {VK_F17, oKB_F17},
  {VK_F18, oKB_F18},
  {VK_F19, oKB_F19},
  {VK_F20, oKB_F20},
  {VK_F21, oKB_F21},
  {VK_F22, oKB_F22},
  {VK_F23, oKB_F23},
  {VK_F24, oKB_F24},
  {VK_NUMLOCK, oKB_Num_Lock},
  {VK_SCROLL, oKB_Scroll_Lock},
  {VK_CANCEL, oKB_Break},
	{VK_BROWSER_BACK, oKB_Browser_Back},
	{VK_BROWSER_FORWARD, oKB_Browser_Forward},
	{VK_BROWSER_REFRESH, oKB_Browser_Refresh},
	{VK_BROWSER_STOP, oKB_Browser_Stop},
	{VK_BROWSER_SEARCH, oKB_Browser_Search},
	{VK_BROWSER_FAVORITES, oKB_Browser_Favorites},
	{VK_BROWSER_HOME, oKB_Browser_Home},
	{VK_VOLUME_MUTE, oKB_Volume_Mute},
	{VK_VOLUME_DOWN, oKB_Volume_Down},
	{VK_VOLUME_UP, oKB_Volume_Up},
	{VK_MEDIA_NEXT_TRACK, oKB_Media_Next_Track},
	{VK_MEDIA_PREV_TRACK, oKB_Media_Prev_Track},
	{VK_MEDIA_STOP, oKB_Media_Stop},
	{VK_MEDIA_PLAY_PAUSE, oKB_Media_Play_Pause},
	{VK_LAUNCH_MAIL, oKB_Launch_Mail},
	{VK_LAUNCH_MEDIA_SELECT, oKB_Launch_Media_Select}
};

typedef struct AppCommandKeyMapping_Struct
{
	UINT AppCmd;
	UINT XCode;
} AppCommandKeyMapping;

static const AppCommandKeyMapping appKeyMap[] = 
{
	{APPCOMMAND_VOLUME_UP,				oKB_Volume_Up},
	{APPCOMMAND_VOLUME_MUTE,			oKB_Volume_Mute},
	{APPCOMMAND_VOLUME_DOWN,			oKB_Volume_Down},
	{APPCOMMAND_MEDIA_STOP,				oKB_Media_Stop},
	{APPCOMMAND_MEDIA_PREVIOUSTRACK,	oKB_Media_Prev_Track},
	{APPCOMMAND_MEDIA_NEXTTRACK,		oKB_Media_Next_Track},
	{APPCOMMAND_MEDIA_PLAY_PAUSE,		oKB_Media_Play_Pause},
	{APPCOMMAND_LAUNCH_MEDIA_SELECT,	oKB_Launch_Media_Select},
	{APPCOMMAND_LAUNCH_MAIL,			oKB_Launch_Mail},
	{APPCOMMAND_BROWSER_STOP,			oKB_Browser_Stop},
	{APPCOMMAND_BROWSER_SEARCH,			oKB_Browser_Search},
	{APPCOMMAND_BROWSER_REFRESH,		oKB_Browser_Refresh},
	{APPCOMMAND_BROWSER_HOME,			oKB_Browser_Home},
	{APPCOMMAND_BROWSER_FORWARD,		oKB_Browser_Forward},
	{APPCOMMAND_BROWSER_FAVORITES,		oKB_Browser_Favorites},
	{APPCOMMAND_BROWSER_BACKWARD,		oKB_Browser_Back}
};

// Given a message such as WM_LBUTTONDOWN (or up), return which button was 
// activated as an X11 code. Returns true if the out params are valid, false if 
// this was not a mouse button message.
bool TranslateMouseButtonToX11(UINT _uMsg, WPARAM _wParam, oKEYBOARD_KEY* _pKey, oGUI_ACTION* _pAction);

oKEYBOARD_KEY TranslateTouchToX11(int _TouchIdx);
// We cannot process or send a key up event for media keys
oKEYBOARD_KEY TranslateAppCommandToX11Key(LPARAM _lParam);
oKEYBOARD_KEY TranslateKeyToX11(WPARAM _wParam);
UINT TranslateX11KeyboardToVK(oKEYBOARD_KEY _Key);
const char* oAsString(oKEYBOARD_KEY _Key);

#endif
