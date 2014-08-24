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
// Syntactic sugar on top of native win32 APIs. This is not intended to abstract 
// a cross-platform implementation, merely this just attempts to centralize, 
// document, simplify and standardize the feature-creeped win32 API. 
// Specifically this header contains API for manipulating StatusBar objects: 
// cells of simple data that often appear at the bottom of many applications.
// NOTE: StatusBars can contain only icons and text. Anything more complex is 
// another control drawn over the status bar.
#pragma once
#ifndef oWinStatusBar_h
#define oWinStatusBar_h

#include <oGUI/oGUI.h>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

HWND oWinStatusBarCreate(HWND _hParent, HMENU _ID, int _MinHeight = oDEFAULT);
bool oWinIsStatusBar(HWND _hStatusBar);
RECT oWinStatusBarGetItemRect(HWND _hStatusBar, int _ItemIndex);
void oWinStatusBarSetNumItems(HWND _hStatusBar, const int* _pItemWidths, size_t _NumItems);
template<size_t size> void oWinStatusBarSetNumItems(HWND _hStatusBar, const int (&_pItemWidths)[size]) { oWinStatusBarSetNumItems(_hStatusBar, _pItemWidths, size); }

// Returns the actual number of items
int oWinStatusBarGetNumItems(HWND _hStatusBar, int* _pItemWidths = nullptr, size_t _MaxNumItemWidths = 0);

void oWinStatusBarSetMinHeight(HWND _hStatusBar, int _MinHeight);

void oWinStatusBarSetText(HWND _hStatusBar, int _ItemIndex, ouro::border_style::value _BorderStyle, const char* _Format, va_list _Args);
inline void oWinStatusBarSetText(HWND _hStatusBar, int _ItemIndex, ouro::border_style::value _BorderStyle, const char* _Format, ...) { va_list args; va_start(args, _Format); oWinStatusBarSetText(_hStatusBar, _ItemIndex, _BorderStyle, _Format, args); va_end(args); }

char* oWinStatusBarGetText(char* _StrDestination, size_t _SizeofStrDestination, HWND _hStatusBar, int _ItemIndex);
template<size_t size> char* oWinStatusBarGetText(char (&_StrDestination)[size], HWND _hStatusBar, int _ItemIndex) { return oWinStatusBarGetText(_StrDestination, size, _hStatusBar, _ItemIndex); }

void oWinStatusBarSetTipText(HWND _hStatusBar, int _ItemIndex, const char* _Text);
char* oWinStatusBarGetTipText(char* _StrDestination, size_t _SizeofStrDestination, HWND _hStatusBar, int _ItemIndex);
template<size_t size> char* oWinStatusBarTipText(char (&_StrDestination)[size], HWND _hStatusBar, int _ItemIndex) { return oWinStatusBarTipText(_StrDestination, size, _hStatusBar, _ItemIndex); }

void oWinStatusBarSetIcon(HWND _hStatusBar, int _ItemIndex, HICON _hIcon);
HICON oWinStatusBarGetIcon(HWND _hStatusBar, int _ItemIndex);

#endif
