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
#include <oGUI/oGUIMenu.h>
#include <oBase/assert.h>
#include <oCore/windows/win_error.h>
#include <oBasis/oError.h> // @tony fixme

using namespace ouro;

#if 0
// not in use (yet?)
static int oGUIMenuFindPosition(ouro::menu_handle _hParentMenu, int _ItemID)
{
	const int n = oGUIMenuGetNumItems(_hParentMenu);
	for (int i = 0; i < n; i++)
	{
		int ID = GetMenuItemID(_hParentMenu, i);
		if (ID == _ItemID)
			return i;
	}
	return oInvalid;
}
#endif

static int oGUIMenuFindPosition(ouro::menu_handle _hParentMenu, ouro::menu_handle _hSubmenu)
{
	const int n = GetMenuItemCount((HMENU)_hParentMenu);
	for (int i = 0; i < n; i++)
	{
		ouro::menu_handle hSubmenu = (ouro::menu_handle)GetSubMenu((HMENU)_hParentMenu, i);
		if (hSubmenu == _hSubmenu)
			return i;
	}
	return oInvalid;
}

#ifdef oENABLE_ASSERTS
// Returns true if the specified menu contains all IDs [first,last]
static bool oGUIMenuContainsRange(ouro::menu_handle _hMenu, int _ItemIDRangeFirst, int _ItemIDRangeLast)
{
	MENUITEMINFO mii;
	mii.cbSize = sizeof(MENUITEMINFO);
	mii.fMask = MIIM_ID;

	int nFound = 0;
	const int n = oGUIMenuGetNumItems(_hMenu);
	for (int i = 0; i < n; i++)
	{
		UINT uID = GetMenuItemID((HMENU)_hMenu, i);
		if (uID == oInvalid)
			return false;
		int ID = oInt(uID);
		if (ID >= _ItemIDRangeFirst && ID <= _ItemIDRangeLast)
			nFound++;
	}

	return nFound == (_ItemIDRangeLast - _ItemIDRangeFirst + 1);
}
#endif

char* oGUIMenuGetTextByPosition(char* _StrDestination, size_t _SizeofStrDestination, ouro::menu_handle _hMenu, int _MenuItemPosition)
{
	if (!GetMenuStringA((HMENU)_hMenu, _MenuItemPosition, _StrDestination, oInt(_SizeofStrDestination), MF_BYPOSITION))
		return nullptr;
	return _StrDestination;
}

ouro::menu_handle oGUIMenuCreate(bool _IsTopLevelMenu)
{
	return (ouro::menu_handle)(_IsTopLevelMenu ? CreateMenu() : CreatePopupMenu());
}

void oGUIMenuDestroy(ouro::menu_handle _hMenu)
{
	if (IsMenu((HMENU)_hMenu))
		oVB(DestroyMenu((HMENU)_hMenu));
}

void oGUIMenuAttach(ouro::window_handle _hWindow, ouro::menu_handle _hMenu)
{
	oVB(SetMenu((HWND)_hWindow, (HMENU)_hMenu));
}

int oGUIMenuGetNumItems(ouro::menu_handle _hMenu)
{
	return oInt(GetMenuItemCount((HMENU)_hMenu));
}

void oGUIMenuAppendSubmenu(ouro::menu_handle _hParentMenu, ouro::menu_handle _hSubmenu, const char* _Text)
{
	oVB(AppendMenu((HMENU)_hParentMenu, MF_STRING|MF_POPUP, (UINT_PTR)_hSubmenu, _Text));
}

void oGUIMenuRemoveSubmenu(ouro::menu_handle _hParentMenu, ouro::menu_handle _hSubmenu)
{
	int p = oGUIMenuFindPosition(_hParentMenu, _hSubmenu);
	if (p != oInvalid && !RemoveMenu((HMENU)_hParentMenu, p, MF_BYPOSITION))
	{
		DWORD hr = GetLastError();
		if (GetLastError() != ERROR_RESOURCE_TYPE_NOT_FOUND)
			oV(hr);
	}
}

void oGUIMenuAppendItem(ouro::menu_handle _hParentMenu, int _ItemID, const char* _Text)
{
	oVB(AppendMenu((HMENU)_hParentMenu, MF_STRING, (UINT_PTR)_ItemID, _Text));
}

void oGUIMenuRemoveItem(ouro::menu_handle _hParentMenu, int _ItemID)
{
	if (!RemoveMenu((HMENU)_hParentMenu, _ItemID, MF_BYCOMMAND))
	{
		DWORD hr = GetLastError();
		if (GetLastError() != ERROR_RESOURCE_TYPE_NOT_FOUND)
			oV(hr);
	}
}

void oGUIMenuRemoveAllItems(ouro::menu_handle _hMenu)
{
	int n = GetMenuItemCount((HMENU)_hMenu);
	while (n)
	{
		DeleteMenu((HMENU)_hMenu, n-1, MF_BYPOSITION);
		n = GetMenuItemCount((HMENU)_hMenu);
	}
}

void oGUIMenuItemToSubmenu(ouro::menu_handle _hParentMenu, int _ItemID, ouro::menu_handle _hSubmenu)
{
	mstring text;
	oVERIFY(oGUIMenuGetText(text, _hParentMenu, _ItemID));
	oVB(RemoveMenu((HMENU)_hParentMenu, _ItemID, MF_BYCOMMAND));
	oVB(InsertMenu((HMENU)_hParentMenu, _ItemID, MF_STRING|MF_POPUP, (UINT_PTR)_hSubmenu, text));
}

void oGUIMenuSubmenuToItem(ouro::menu_handle _hParentMenu, ouro::menu_handle _hSubmenu, int _ItemID, bool _Enabled)
{
	int p = oGUIMenuFindPosition(_hParentMenu, _hSubmenu);
	oASSERT(p != oInvalid, "the specified submenu is not under the specified parent menu");
	
	mstring text;	
	oVERIFY(oGUIMenuGetTextByPosition(text, text.capacity(), _hParentMenu, p));
	oVB(DeleteMenu((HMENU)_hParentMenu, p, MF_BYPOSITION));

	UINT uFlags = MF_BYPOSITION|MF_STRING;
	if (!_Enabled)
		uFlags |= MF_GRAYED;

	oVB(InsertMenu((HMENU)_hParentMenu, p, uFlags, (UINT_PTR)_ItemID, text.c_str()));
}

void oGUIMenuAppendSeparator(ouro::menu_handle _hParentMenu)
{
	oVB(AppendMenu((HMENU)_hParentMenu, MF_SEPARATOR, 0, nullptr));
}

void oGUIMenuCheck(ouro::menu_handle _hMenu, int _ItemID, bool _Checked)
{
	if (-1 == CheckMenuItem((HMENU)_hMenu, oUInt(_ItemID), MF_BYCOMMAND | (_Checked ? MF_CHECKED : MF_UNCHECKED)))
		oASSERT(false, "MenuItemID not found in the specified menu");
}

bool oGUIMenuIsChecked(ouro::menu_handle _hMenu, int _ItemID)
{
	MENUITEMINFO mii;
	ZeroMemory(&mii, sizeof(mii));
	mii.cbSize = sizeof(mii);
	mii.fMask = MIIM_STATE;
	if (!GetMenuItemInfo((HMENU)_hMenu, oUInt(_ItemID), FALSE, &mii))
		return false;

	if (mii.fState & MFS_CHECKED)
		return true;

	oErrorSetLast(0);
	return false;
}

void oGUIMenuCheckRadio(ouro::menu_handle _hMenu, int _ItemIDRadioRangeFirst, int _ItemIDRadioRangeLast, int _ItemIDToCheck)
{
	// CheckMenuRadioItem returns false if the menu is wrong, but doesn't set a 
	// useful last error (S_OK is returned when I ran into this) so add our own
	// check here.
	oASSERT(oGUIMenuGetNumItems(_hMenu) >= (_ItemIDRadioRangeLast-_ItemIDRadioRangeFirst+1), "A radio range was specified that is larger than the number of elements in the list (menu count=%d, range implies %d items)", oGUIMenuGetNumItems(_hMenu), (_ItemIDRadioRangeLast-_ItemIDRadioRangeFirst+1));
	oASSERT(oGUIMenuContainsRange(_hMenu, _ItemIDRadioRangeFirst, _ItemIDRadioRangeLast), "The specified menu 0x%p does not include the specified range [%d,%d] with selected %d. Most API works with an ancestor menu but this requires the immediate parent, so if the ranges look correct check the specified _hMenu.", _hMenu, _ItemIDRadioRangeFirst, _ItemIDRadioRangeLast, _ItemIDToCheck);
	oVB(CheckMenuRadioItem((HMENU)_hMenu, _ItemIDRadioRangeFirst, _ItemIDRadioRangeLast, _ItemIDToCheck, MF_BYCOMMAND));
}

int oGUIMenuGetCheckedRadio(ouro::menu_handle _hMenu, int _ItemIDRadioRangeFirst, int _ItemIDRadioRangeLast)
{
	for (int i = _ItemIDRadioRangeFirst; i <= _ItemIDRadioRangeLast; i++)
		if (oGUIMenuIsChecked(_hMenu, i))
			return i;
	return oInvalid;
}

void oGUIMenuEnable(ouro::menu_handle _hMenu, int _ItemID, bool _Enabled)
{
	if (-1 == EnableMenuItem((HMENU)_hMenu, oUInt(_ItemID), MF_BYCOMMAND | (_Enabled ? MF_ENABLED : MF_GRAYED)))
		oASSERT(false, "MenuItemID not found in the specified menu");
}

bool oGUIMenuIsEnabled(ouro::menu_handle _hMenu, int _ItemID)
{
	MENUITEMINFO mii;
	ZeroMemory(&mii, sizeof(mii));
	mii.cbSize = sizeof(mii);
	mii.fMask = MIIM_STATE;
	oVB(GetMenuItemInfo((HMENU)_hMenu, oUInt(_ItemID), FALSE, &mii));
	if (mii.fState & (MF_GRAYED|MF_DISABLED))
		return false;
	return true;
}

char* oGUIMenuGetText(char* _StrDestination, size_t _SizeofStrDestination, ouro::menu_handle _hMenu, int _ItemID)
{
	if (!GetMenuStringA((HMENU)_hMenu, _ItemID, _StrDestination, oInt(_SizeofStrDestination), MF_BYCOMMAND))
		return nullptr;
	return _StrDestination;
}

char* oGUIMenuGetText(char* _StrDestination, size_t _SizeofStrDestination, ouro::menu_handle _hParentMenu, ouro::menu_handle _hSubmenu)
{
	int pos = oGUIMenuFindPosition(_hParentMenu, _hSubmenu);
	if (pos == oInvalid)
		return nullptr;
	return oGUIMenuGetTextByPosition(_StrDestination, _SizeofStrDestination, _hParentMenu, pos);
}
