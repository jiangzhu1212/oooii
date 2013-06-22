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
// Utilities for working with Window's common dialogs.
#pragma once
#ifndef oWinDialog_h
#define oWinDialog_h

#include <oBasis/oGUI.h>
#include <oPlatform/Windows/oWindows.h>

// Filter pairs are delimited by '|' and are the description, and the wildcard:
// i.e. "Text Files|*.txt|Bitmap Files|*.bmp"
// If _StrDestination contains a valid path, that path will be used
// as the initial starting folder.
// If there is an error this will return false. If the error was due to a user
// canceling the dialog box, last error will be std::errc::operation_canceled. Most likely
// any other error will be std::errc::protocol_error with a Windows common dialog error
// to follow.
bool oWinDialogGetOpenPath(char* _StrDestination, size_t _SizeofStrDestination, const char* _DialogTitle, const char* _FilterPairs, HWND _hParent = nullptr);
template<size_t size> bool oWinDialogGetOpenPath(char (&_StrDestination)[size], const char* _DialogTitle, const char* _FilterPairs, HWND _hParent = nullptr) { return oWinDialogGetOpenPath(_StrDestination, size, _DialogTitle, _FilterPairs, _hParent); }
template<size_t capacity> bool oWinDialogGetOpenPath(oStd::fixed_string<char, capacity>& _StrDestination, const char* _DialogTitle, const char* _FilterPairs, HWND _hParent = nullptr) { return oWinDialogGetOpenPath(_StrDestination, _StrDestination.capacity(), _DialogTitle, _FilterPairs, _hParent); }

bool oWinDialogGetSavePath(char* _StrDestination, size_t _SizeofStrDestination, const char* _DialogTitle, const char* _FilterPairs, HWND _hParent = nullptr);
template<size_t size> bool oWinDialogGetSavePath(char (&_StrDestination)[size], const char* _DialogTitle, const char* _FilterPairs, HWND _hParent = nullptr) { return oWinDialogGetSavePath(_StrDestination, size, _DialogTitle, _FilterPairs, _hParent); }
template<size_t capacity> bool oWinDialogGetSavePath(oStd::fixed_string<char, capacity>& _StrDestination, const char* _DialogTitle, const char* _FilterPairs, HWND _hParent = nullptr) { return oWinDialogGetSavePath(_StrDestination, _StrDestination.capacity(), _DialogTitle, _FilterPairs, _hParent); }

// *_pColor is used as the initial value of the dialog
bool oWinDialogGetColor(oStd::color* _pColor, HWND _hParent = nullptr);

// *_pLogicalFont is used to initialize the dialog
bool oWinDialogGetFont(LOGFONT* _pLogicalFont, oStd::color* _pColor, HWND _hParent);

#endif
