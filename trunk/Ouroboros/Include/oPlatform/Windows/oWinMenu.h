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
// Syntactic sugar on top of native win32 APIs. This is not
// intended to abstract a cross-platform implementation, 
// merely this just attempts to centralize, document, 
// simplify and standardize the feature-creeped win32 API.
// Specifically this header contains HMENU-related API for
// creating the typical File-Edit-View-Help drop-down menus
// found in many applications.
#pragma once
#ifndef oWinMenu_h
#define oWinMenu_h

#include <oPlatform/Windows/oWindows.h>

// If _IsTopLevelMenu is true, the returned HMENU can be passed to the 
// creation parameter for an HWND (as in oWinCreate above). If false, this 
// creates a "popup" menu, the kind used in submenus or right-click menus.
// NOTE: A menu should be created and associated with an HWND before calling 
// oWinSetStyle because all adjustments to RECT size are client rect-based, and
// having a menu can alter that. To fix the situation, it's possible to call
// oWinSetStyle again after a menu has been added to ensure client dimensions 
// are as-specified.
HMENU oWinMenuCreate(bool _IsTopLevelMenu = false);

// This only needs to be called on HMENUs with no parents, such as those 
// detached from a window or parent menu. If an HMENU is connected, then
// the parent manages the lifetime.
void oWinMenuDestroy(HMENU _hMenu);

// Transfer ownership of HMENU lifetime to HWND and show it. Setting a null 
// _hMenu will deassociate the HMENU and client code is responsible for its
// lifetime.
void oWinMenuSet(HWND _hWnd, HMENU _hMenu);

// Attach the specified submenu to the specified parent menu. The parent
// menu can be a top-level window. If a submenu is attached, it's lifetime
// need no longer be managed.
void oWinMenuAddSubmenu(HMENU _hParentMenu, HMENU _hSubmenu, const char* _Text);

void oWinMenuAddMenuItem(HMENU _hParentMenu, int _MenuItemID, const char* _Text);
void oWinMenuRemoveMenuItem(HMENU _hParentMenu, int _MenuItemID);

void oWinMenuAddSeparator(HMENU _hParentMenu);

void oWinMenuCheck(HMENU _hMenu, int _MenuItemID, bool _Checked = true);
bool oWinMenuIsChecked(HMENU _hMenu, int _MenuItemID);

void oWinMenuEnable(HMENU _hMenu, int _MenuItemID, bool _Enabled = true);
bool oWinMenuIsEnabled(HMENU _hMenu, int _MenuItemID);

#endif
