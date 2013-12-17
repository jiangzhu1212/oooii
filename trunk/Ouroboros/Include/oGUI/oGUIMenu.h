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
// API for creating GUI menus.
// Menu: A handle that contains menu items and submenus
// Item: an item in a menu that has an ID, but is accessed relative to its 
// parent menu.
#pragma once
#ifndef oGUIMenu_h
#define oGUIMenu_h

#include <oBasis/oGUI.h>
#include <oBasis/oRTTI.h>
#include <oBase/fixed_string.h>
#include <vector>

// If _IsTopLevelMenu is true, the returned oGUI_MENU can be associated with an
// oWindow. If false, "popup" menu will be created fit use as a submenu or 
// right-click menus.
oGUI_MENU oGUIMenuCreate(bool _IsTopLevelMenu = false);

// This only needs to be called on oGUI_MENUs with no parents, such as those 
// detached from a window or parent menu. If an oGUI_MENU is attached, then the 
// parent manages the lifetime.
void oGUIMenuDestroy(oGUI_MENU _hMenu);

// Transfer ownership of oGUI_MENU lifetime to a window and show it. Setting a 
// null _hMenu will detach the oGUI_MENU and client code is responsible for its
// lifetime.
void oGUIMenuAttach(oGUI_WINDOW _hWindow, oGUI_MENU _hMenu);

// Returns the number of items in the specified menu
int oGUIMenuGetNumItems(oGUI_MENU _hMenu);

// The parent menu can be a top-level window. Once a submenu is attached it's 
// lifetime is managed by the parent menu.
void oGUIMenuAppendSubmenu(oGUI_MENU _hParentMenu, oGUI_MENU _hSubmenu, const char* _Text);

// Detaches the specified submenu without destroying it.
void oGUIMenuRemoveSubmenu(oGUI_MENU _hParentMenu, oGUI_MENU _hSubmenu);

// Convert the specified menuitem into a submenu, preserving the text. This is 
// primarily intended for code to take a disabled, empty submenu to having a 
// menu to be populated.
void oGUIMenuItemToSubmenu(oGUI_MENU _hParentMenu, int _ItemID, oGUI_MENU _hSubmenu);

// Converts the specified submenu into a menuitem with the specified ID. By 
// default it is created disabled because the primary use of this utility is to 
// disable the top-level entry for a submenu when it has zero entries.
void oGUIMenuSubmenuToItem(oGUI_MENU _hParentMenu, oGUI_MENU _hSubmenu, int _ItemID, bool _Enabled = false);

void oGUIMenuAppendItem(oGUI_MENU _hParentMenu, int _ItemID, const char* _Text);
void oGUIMenuRemoveItem(oGUI_MENU _hParentMenu, int _ItemID);

// Any submenus will be destroyed. Take care not to use dangling handles to 
// submenus once this is called.
void oGUIMenuRemoveAllItems(oGUI_MENU _hMenu);

void oGUIMenuAppendSeparator(oGUI_MENU _hParentMenu);

void oGUIMenuEnable(oGUI_MENU _hMenu, int _ItemID, bool _Enabled = true);
bool oGUIMenuIsEnabled(oGUI_MENU _hMenu, int _ItemID);

void oGUIMenuCheck(oGUI_MENU _hMenu, int _ItemID, bool _Checked = true);
bool oGUIMenuIsChecked(oGUI_MENU _hMenu, int _ItemID);

// Radio API ensures only zero or one is selected in a range. Use oGUIMenuCheck
// to zero out a selected one.
void oGUIMenuCheckRadio(oGUI_MENU _hMenu, int _ItemIDRadioRangeFirst, int _ItemIDRadioRangeLast, int _ItemIDToCheck);

// If none are checked, this returns oInvalid.
int oGUIMenuGetCheckedRadio(oGUI_MENU _hMenu, int _ItemIDRadioRangeFirst, int _ItemIDRadioRangeLast);

char* oGUIMenuGetText(char* _StrDestination, size_t _SizeofStrDestination, oGUI_MENU _hMenu, int _ItemID);
template<size_t size> char* oGUIMenuGetText(char (&_StrDestination)[size], oGUI_MENU _hMenu, int _ItemID) { return oGUIMenuGetText(_StrDestination, size, _hMenu, _ItemID); }
template<size_t capacity> char* oGUIMenuGetText(ouro::fixed_string<char, capacity>& _StrDestination, oGUI_MENU _hMenu, int _ItemID) { return oGUIMenuGetText(_StrDestination, _StrDestination.capacity(), _hMenu, _ItemID); }

char* oGUIMenuGetText(char* _StrDestination, size_t _SizeofStrDestination, oGUI_MENU _hMenu, oGUI_MENU _hSubmenu);
template<size_t size> char* oGUIMenuGetText(char (&_StrDestination)[size], oGUI_MENU _hParentMenu, oGUI_MENU _hSubmenu) { return oGUIMenuGetText(_StrDestination, size, _hParentMenu, _hSubmenu); }
template<size_t capacity> char* oGUIMenuGetText(ouro::fixed_string<char, capacity>& _StrDestination, oGUI_MENU _hParentMenu, oGUI_MENU _hSubmenu) { return oGUIMenuGetText(_StrDestination, _StrDestination.capacity(), _hParentMenu, _hSubmenu); }

// Create a menu populated with all values of an enum in the specified range.
// This is useful to pair with oGUIMenuEnumRadioListHandler below for quick
// menu construction for radio selection groups. _InitialItem will evaluate to
// _FirstMenuItem by default.
bool oGUIMenuAppendEnumItems(oGUI_MENU _hMenu, int _FirstMenuItem, int _LastMenuItem, const oRTTI& _Enum, int _InitialItem = oInvalid);
template<typename enumT> bool oGUIMenuAppendEnumItems(oGUI_MENU _hMenu, int _FirstMenuItem, int _LastMenuItem, const oRTTI& _Enum, const enumT& _InitialEnumValue) { return oGUIMenuAppendEnumItems(_hMenu, _FirstMenuItem, _LastMenuItem, _Enum, int(_FirstMenuItem + _InitialEnumValue)); }

// A utility class to help handle enums that manifest as radio selection groups
// in menus. Basically this allows a range of IDs to be associated with a 
// callback.
class oGUIMenuEnumRadioListHandler
{
public:
	// _RebasedMenuItem is inputvalue - _FirstMenuItem, so the first menuitem will
	// have a rebased value of 0.
	typedef oFUNCTION<void(int _RebasedMenuItem)> callback_t;

	inline void Register(oGUI_MENU _hMenu, int _FirstMenuItem, int _LastMenuItem, const callback_t& _Callback)
	{
		ENTRY e;
		e.hMenu = _hMenu;
		e.First = _FirstMenuItem;
		e.Last = _LastMenuItem;
		e.Callback = _Callback;

		auto it = std::find_if(Callbacks.begin(), Callbacks.end(), [&](const ENTRY& _Entry)
		{
			return (e.hMenu == _Entry.hMenu)
				|| (e.First >= _Entry.First && e.First <= _Entry.Last) 
				|| (e.Last >= _Entry.First && e.Last <= _Entry.Last);
		});

		if (it != Callbacks.end())
			throw std::invalid_argument("The specified menu/range has already been registered or overlaps a previously registered range");

		Callbacks.push_back(e);
	}

	inline void OnAction(const oGUI_ACTION_DESC& _Action)
	{
		if (_Action.Action == oGUI_ACTION_MENU)
		{
			auto it = std::find_if(Callbacks.begin(), Callbacks.end(), [&](const ENTRY& _Entry)
			{
				return _Action.DeviceID >= _Entry.First && _Action.DeviceID <= _Entry.Last;
			});

			if (it != Callbacks.end())
			{
				oGUIMenuCheckRadio(it->hMenu, it->First, it->Last, _Action.DeviceID);
				it->Callback(_Action.DeviceID - it->First);
			}
		}
	}

private:
	struct ENTRY
	{
		oGUI_MENU hMenu;
		int First;
		int Last;
		callback_t Callback;
	};

	std::vector<ENTRY> Callbacks;
};

#endif
