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
// Utilities for working with Window's common dialogs.
#pragma once
#ifndef oGUI_win_common_dialog_h
#define oGUI_win_common_dialog_h

#include <oBase/color.h>
#include <oBase/path.h>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

namespace ouro {
	namespace windows {
		namespace common_dialog {

// All functions return true if successful or false if the user canceled the dialog
// and thus there is no error but values are not valid.
// If there is a system error an exception is thrown.

// Filter pairs are delimited by '|' and are the description, and the wildcard:
// i.e. "Text Files|*.txt|Bitmap Files|*.bmp"
// If _Path is not empty it will be used as the initial starting folder.
bool open_path(path& _Path, const char* _DialogTitle, const char* _FilterPairs, HWND _hParent = nullptr);
bool save_path(path& _Path, const char* _DialogTitle, const char* _FilterPairs, HWND _hParent = nullptr);

// *_pColor is used as the initial value of the dialog
bool pick_color(color* _pColor, HWND _hParent = nullptr);

// *_pLogicalFont is used to initialize the dialog
bool pick_font(LOGFONT* _pLogicalFont, color* _pColor, HWND _hParent);

		} // namespace common_dialog
	} // namespace windows
} // namespace ouro

#endif
