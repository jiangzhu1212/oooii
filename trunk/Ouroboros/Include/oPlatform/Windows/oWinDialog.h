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
// Utilities for working with Window's common dialogs.
#pragma once
#ifndef oWinDialog_h
#define oWinDialog_h

#include <oBasis/oGUI.h>
#include <oPlatform/Windows/oWindows.h>

// Filter pairs are delimited by '|' and are the description, and the wildcard:
// i.e. "Text Files|*.txt|Bitmap Files|*.bmp"
// If _Path is not empty it will be used as the initial starting folder.
// If there is an error this will return false. If the error was due to a user
// canceling the dialog box, last error will be std::errc::operation_canceled. 
// Most likely any other error will be std::errc::protocol_error with a Windows 
// common dialog error to follow.
bool oWinDialogGetOpenPath(ouro::path& _Path, const char* _DialogTitle, const char* _FilterPairs, HWND _hParent = nullptr);
bool oWinDialogGetSavePath(ouro::path& _Path, const char* _DialogTitle, const char* _FilterPairs, HWND _hParent = nullptr);

// *_pColor is used as the initial value of the dialog
bool oWinDialogGetColor(ouro::color* _pColor, HWND _hParent = nullptr);

// *_pLogicalFont is used to initialize the dialog
bool oWinDialogGetFont(LOGFONT* _pLogicalFont, ouro::color* _pColor, HWND _hParent);

#endif
