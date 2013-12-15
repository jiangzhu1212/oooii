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
// Utilities for working with the Window's Tray
#pragma once
#ifndef oGUI_win_tray_h
#define oGUI_win_tray_h

#include <oBasis/oGUI.h>

namespace ouro {
	namespace notification_area {

// Return the oGUI_WINDOW of the "system tray" or "notification area"	
oGUI_WINDOW native_handle();

// Sets the focus on the tray itself, not any icon in it
void focus();

// Get the rectangle of the specified icon. Returns true if the rect is valid, 
// or false if the icon doesn't exist.
void icon_rect(oGUI_WINDOW _hWnd, unsigned int _ID, int* _pX, int* _pY, int* _pWidth, int* _pHeight);

// Returns true of the tray icon exists
bool exists(oGUI_WINDOW _hWnd, unsigned int _ID);

// Icons are identified by the oGUI_WINDOW and the ID. If _CallbackMessage is not zero 
// then this is a message that can be handled in the oGUI_WINDOW's WNDPROC. Use 
// WM_USER+n for the custom code. If oGUI_ICON is nullptr the icon from the oGUI_WINDOW 
// will be used. All lifetime management of a valid oGUI_ICON must be handled by 
// client code.
void show_icon(oGUI_WINDOW _hWnd, unsigned int _ID, unsigned int _CallbackMessage, oGUI_ICON _hIcon, bool _Show);

// Once an icon has been created with show_icon, use this to display a message 
// on it. If _hIcon is nullptr then the _hWnd's icon is used.
void show_message(oGUI_WINDOW _hWnd, unsigned int _ID, oGUI_ICON _hIcon, unsigned int _TimeoutMS, const char* _Title, const char* _Message);

// Minimize a window to the tray (animates a window to the tray)
void minimize(oGUI_WINDOW _hWnd, unsigned int _CallbackMessage, oGUI_ICON _hIcon);

// Animates an existing tray icon to a restored window
void restore(oGUI_WINDOW _hWnd);

// Use this in a windows message handler to decode the parameters for a callback
// message as passed to one of the above APIs.
void decode_callback_message_params(uintptr_t _wParam, uintptr_t _lParam, unsigned int* _pNotificationEvent, unsigned int* _pID, int* _pX, int* _pY);

	} // namespace notification_area
} // namespace ouro

#endif
