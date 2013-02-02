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
// API for creating GUI menus.
// Menu: A handle that contains menu items and submenus
// Item: an item in a menu that has an ID, but is accessed relative to its 
// parent menu.
#pragma once
#ifndef oGUIMenu_h
#define oGUIMenu_h

#include <oBasis/oGUI.h>
#include <oBasis/oFixedString.h>

// If _IsTopLevelMenu is true, the returned oGUI_MENU can be associated with an
// oWindow. If false, "popup" menu will be created fit use as a submenu or 
// right-click menus.
oAPI oGUI_MENU oGUIMenuCreate(bool _IsTopLevelMenu = false);

// This only needs to be called on oGUI_MENUs with no parents, such as those 
// detached from a window or parent menu. If an oGUI_MENU is attached, then the 
// parent manages the lifetime.
oAPI void oGUIMenuDestroy(oGUI_MENU _hMenu);

// Transfer ownership of oGUI_MENU lifetime to a window and show it. Setting a 
// null _hMenu will detach the oGUI_MENU and client code is responsible for its
// lifetime.
oAPI void oGUIMenuAttach(oGUI_WINDOW _hWindow, oGUI_MENU _hMenu);

// Returns the number of items in the specified menu
oAPI int oGUIMenuGetNumItems(oGUI_MENU _hMenu);

// The parent menu can be a top-level window. Once a submenu is attached it's 
// lifetime is managed by the parent menu.
oAPI void oGUIMenuAppendSubmenu(oGUI_MENU _hParentMenu, oGUI_MENU _hSubmenu, const char* _Text);

// Detaches the specified submenu without destroying it.
oAPI void oGUIMenuRemoveSubmenu(oGUI_MENU _hParentMenu, oGUI_MENU _hSubmenu);

// Convert the specified menuitem into a submenu, preserving the text. This is 
// primarily intended for code to take a disabled, empty submenu to having a 
// menu to be populated.
oAPI void oGUIMenuItemToSubmenu(oGUI_MENU _hParentMenu, int _ItemID, oGUI_MENU _hSubmenu);

// Converts the specified submenu into a menuitem with the specified ID. By 
// default it is created disabled because the primary use of this utility is to 
// disable the top-level entry for a submenu when it has zero entries.
oAPI void oGUIMenuSubmenuToItem(oGUI_MENU _hParentMenu, oGUI_MENU _hSubmenu, int _ItemID, bool _Enabled = false);

oAPI void oGUIMenuAppendItem(oGUI_MENU _hParentMenu, int _ItemID, const char* _Text);
oAPI void oGUIMenuRemoveItem(oGUI_MENU _hParentMenu, int _ItemID);

// Any submenus will be destroyed. Take care not to use dangling handles to 
// submenus once this is called.
oAPI void oGUIMenuRemoveAllItems(oGUI_MENU _hMenu);

oAPI void oGUIMenuAppendSeparator(oGUI_MENU _hParentMenu);

oAPI void oGUIMenuEnable(oGUI_MENU _hMenu, int _ItemID, bool _Enabled = true);
oAPI bool oGUIMenuIsEnabled(oGUI_MENU _hMenu, int _ItemID);

oAPI void oGUIMenuCheck(oGUI_MENU _hMenu, int _ItemID, bool _Checked = true);
oAPI bool oGUIMenuIsChecked(oGUI_MENU _hMenu, int _ItemID);

// Radio API ensures only zero or one is selected in a range. Use oGUIMenuCheck
// to zero out a selected one.
oAPI void oGUIMenuCheckRadio(oGUI_MENU _hMenu, int _ItemIDRadioRangeFirst, int _ItemIDRadioRangeLast, int _ItemIDToCheck);

// If none are checked, this returns oInvalid.
oAPI int oGUIMenuGetCheckedRadio(oGUI_MENU _hMenu, int _ItemIDRadioRangeFirst, int _ItemIDRadioRangeLast);

oAPI char* oGUIMenuGetText(char* _StrDestination, size_t _SizeofStrDestination, oGUI_MENU _hMenu, int _ItemID);
template<size_t size> char* oGUIMenuGetText(char (&_StrDestination)[size], oGUI_MENU _hMenu, int _ItemID) { return oGUIMenuGetText(_StrDestination, size, _hMenu, _ItemID); }
template<size_t capacity> char* oGUIMenuGetText(oFixedString<char, capacity>& _StrDestination, oGUI_MENU _hMenu, int _ItemID) { return oGUIMenuGetText(_StrDestination, _StrDestination.capacity(), _hMenu, _ItemID); }

oAPI char* oGUIMenuGetText(char* _StrDestination, size_t _SizeofStrDestination, oGUI_MENU _hMenu, oGUI_MENU _hSubmenu);
template<size_t size> char* oGUIMenuGetText(char (&_StrDestination)[size], oGUI_MENU _hParentMenu, oGUI_MENU _hSubmenu) { return oGUIMenuGetText(_StrDestination, size, _hParentMenu, _hSubmenu); }
template<size_t capacity> char* oGUIMenuGetText(oFixedString<char, capacity>& _StrDestination, oGUI_MENU _hParentMenu, oGUI_MENU _hSubmenu) { return oGUIMenuGetText(_StrDestination, _StrDestination.capacity(), _hParentMenu, _hSubmenu); }

#endif
