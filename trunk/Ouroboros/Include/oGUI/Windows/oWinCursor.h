// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.
// Utilities for working with HCURSOR objects on Windows
#pragma once
#ifndef oWinCursor_h
#define oWinCursor_h

#include <oGUI/oGUI.h>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

// Modifies the default cursor associated with the client area of a Window
HCURSOR oWinGetCursor(HWND _hWnd);
bool oWinSetCursor(HWND _hWnd, HCURSOR _hCursor);

// Returns a preloaded windows HCURSOR. There is not need to free the returned
// value. This returns nullptr for ouro::cursor_state::none. The value specified
// in _hUserCursor is returned of ouro::cursor_state::user is specified.
HCURSOR oWinGetCursor(ouro::cursor_state::value _State, HCURSOR _hUserCursor = nullptr);

// Sets/hides/shows a cursor over the specified HWND according to the state.
void oWinCursorSetState(HWND _hWnd, ouro::cursor_state::value _CursorState, HCURSOR _hUserCursor = nullptr);

// Returns the state based on which cursor is set.
ouro::cursor_state::value oWinCursorGetState(HWND _hWnd);

bool oWinCursorGetClipped(HWND _hWnd);
bool oWinCursorSetClipped(HWND _hWnd, bool _Clipped = true);

bool oWinCursorIsVisible();
void oWinCursorSetVisible(bool _Visible = true);

// Returns the coordinates of the cursor's hotspot. If _hWnd is nullptr then the 
// position will be in virtual desktop space. If _hWnd is valid, then the 
// coordinates will be in client space.
int2 oWinCursorGetPosition(HWND _hWnd);

// Set the coordinates of the cursor's hotspot. If _hWnd is nullptr then the 
// position will be in virtual desktop space. If _hWnd is valid then the 
// position will be in client space.
void oWinCursorSetPosition(HWND _hWnd, const int2& _Position);

#endif
