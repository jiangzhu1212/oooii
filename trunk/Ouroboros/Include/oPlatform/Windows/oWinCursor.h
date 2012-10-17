/**************************************************************************
 * The MIT License                                                        *
 * Copyright (c) 2011 Antony Arciuolo & Kevin Myers                       *
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
// Utilities for working with HCURSOR objects on Windows
#pragma once
#ifndef oWinCursor_h
#define oWinCursor_h

#include <oBasis/oGUI.h>
#include <oPlatform/Windows/oWindows.h>

// Modifies the default cursor associated with the client area of a Window
HCURSOR oWinGetCursor(HWND _hWnd);
bool oWinSetCursor(HWND _hWnd, HCURSOR _hCursor);

// Returns a preloaded windows HCURSOR. There is not need to free the returned
// value. This returns nullptr for oGUI_CURSOR_NONE. The value specified
// in _hUserCursor is returned of oGUI_CURSOR_USER is specified.
HCURSOR oWinGetCursor(oGUI_CURSOR_STATE _State, HCURSOR _hUserCursor = nullptr);

// Sets/hides/shows a cursor over the specified HWND according to the state.
void oWinCursorSetState(HWND _hWnd, oGUI_CURSOR_STATE _CursorState, HCURSOR _hUserCursor = nullptr);

// Returns the state based on which cursor is set.
oGUI_CURSOR_STATE oWinCursorGetState(HWND _hWnd);

bool oWinCursorGetClipped(HWND _hWnd);
bool oWinCursorSetClipped(HWND _hWnd, bool _Clipped = true);

bool oWinCursorIsVisible();
void oWinCursorSetVisible(bool _Visible = true);

bool oWinCursorGetPosition(HWND _hWnd, int2* _ClientPosition);
bool oWinCursorSetPosition(HWND _hWnd, const int2& _ClientPosition);

#endif
