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
// Syntactic sugar on top of native win32 APIs. This is not
// intended to abstract a cross-platform implementation, 
// merely this just attempts to centralize, document, 
// simplify and standardize the feature-creeped win32 API.
// Specifically this header contains API for manipulating
// StatusBar objects: cells of simple data that often appear
// at the bottom of many applications.

// NOTE1: StatusBars can contain only icons and text. Anything 
// more complex is another control drawn over the status bar.

// NOTE2: StatusBars are actually drawn in the client area, and
// also require sync with the parent window's client area if it
// is resized. Call oWinStatusBarSyncOnSize() from the parent's
// WM_SIZE handler to keep the StatusBar positioned properly. 
// Usually a StatusBar appears as a thicker bottom border, not a
// control in the client area, so often usage of this will want
// to shorten the dimensions of GetClientRect on the parent to
// prevent typical client area draws from overdrawing the 
// StatusBar.
#pragma once
#ifndef oWinStatusBar_h
#define oWinStatusBar_h

#include <oBasis/oGUI.h>
#include <oPlatform/Windows/oWindows.h>

HWND oWinStatusBarCreate(HWND _hParent, HMENU _ID, int _MinHeight = oDEFAULT);

// The StatusBar needs to be relocated as its parent window
// changes, so call this in the parent's WM_SIZE handler.
void oWinStatusBarSyncOnSize(HWND _hStatusBar);

// Returns the height of the status bar if the specified window has one, or 
// oInvalid if it does not. (0 indicates there is a status bar and it is hidden)
int oWinStatusBarGetHeight(HWND _hParent);

// Other oWin* API treats the status bar of a window as not part of the 
// client area. Windows disagrees, so be able to truncate the true client 
// area to not overlap the area handled by the status bar. This assumes
// that there is only one StatusBar associated with the specified window.
void oWinStatusBarAdjustClientRect(HWND _hParent, RECT* _pRect);

RECT oWinStatusBarGetItemRect(HWND _hStatusBar, int _ItemIndex);

void oWinStatusBarSetNumItems(HWND _hStatusBar, const int* _pItemWidths, size_t _NumItems);
template<size_t size> void oWinStatusBarSetNumItems(HWND _hStatusBar, const int (&_pItemWidths)[size]) { oWinStatusBarSetNumItems(_hStatusBar, _pItemWidths, size); }

void oWinStatusBarSetMinHeight(HWND _hStatusBar, int _MinHeight);

void oWinStatusBarSetText(HWND _hStatusBar, int _ItemIndex, oGUI_BORDER_STYLE _BorderStyle, const char* _Format, va_list _Args);
inline void oWinStatusBarSetText(HWND _hStatusBar, int _ItemIndex, oGUI_BORDER_STYLE _BorderStyle, const char* _Format, ...) { va_list args; va_start(args, _Format); oWinStatusBarSetText(_hStatusBar, _ItemIndex, _BorderStyle, _Format, args); va_end(args); }

char* oWinStatusBarGetText(char* _StrDestination, size_t _SizeofStrDestination, HWND _hStatusBar, int _ItemIndex);
template<size_t size> char* oWinStatusBarGetText(char (&_StrDestination)[size], HWND _hStatusBar, int _ItemIndex) { return oWinStatusBarGetText(_StrDestination, size, _hStatusBar, _ItemIndex); }

void oWinStatusBarSetTipText(HWND _hStatusBar, int _ItemIndex, const char* _Text);
char* oWinStatusBarGetTipText(char* _StrDestination, size_t _SizeofStrDestination, HWND _hStatusBar, int _ItemIndex);
template<size_t size> char* oWinStatusBarTipText(char (&_StrDestination)[size], HWND _hStatusBar, int _ItemIndex) { return oWinStatusBarTipText(_StrDestination, size, _hStatusBar, _ItemIndex); }

void oWinStatusBarSetIcon(HWND _hStatusBar, int _ItemIndex, HICON _hIcon);
HICON oWinStatusBarGetIcon(HWND _hStatusBar, int _ItemIndex);

#endif
