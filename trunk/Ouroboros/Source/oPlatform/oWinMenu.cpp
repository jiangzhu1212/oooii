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
#include <oPlatform/Windows/oWinMenu.h>
#include <oBasis/oAssert.h>

HMENU oWinMenuCreate(bool _IsTopLevelMenu)
{
	return _IsTopLevelMenu ? CreateMenu() : CreatePopupMenu();
}

void oWinMenuDestroy(HMENU _hMenu)
{
	if (IsMenu(_hMenu))
		oVB(DestroyMenu(_hMenu));
}

void oWinMenuSet(HWND _hWnd, HMENU _hMenu)
{
	oVB(SetMenu(_hWnd, _hMenu));
}

void oWinMenuAddSubmenu(HMENU _hParentMenu, HMENU _hSubmenu, const char* _Text)
{
	oVB(AppendMenu(_hParentMenu, MF_STRING|MF_POPUP, (UINT_PTR)_hSubmenu, _Text));
}

void oWinMenuAddMenuItem(HMENU _hParentMenu, int _MenuItemID, const char* _Text)
{
	oVB(AppendMenu(_hParentMenu, MF_STRING, (UINT_PTR)_MenuItemID, _Text));
}

void oWinMenuRemoveMenuItem(HMENU _hParentMenu, int _MenuItemID)
{
	if (!RemoveMenu(_hParentMenu, _MenuItemID, MF_BYCOMMAND))
	{
		DWORD hr = GetLastError();
		if (GetLastError() != ERROR_RESOURCE_TYPE_NOT_FOUND)
			oV(hr);
	}
}

void oWinMenuAddSeparator(HMENU _hParentMenu)
{
	oVB(AppendMenu(_hParentMenu, MF_SEPARATOR, 0, nullptr));
}

void oWinMenuCheck(HMENU _hMenu, int _MenuItemID, bool _Checked)
{
	if (-1 == CheckMenuItem(_hMenu, (UINT)_MenuItemID, MF_BYCOMMAND | (_Checked ? MF_CHECKED : MF_UNCHECKED)))
		oASSERT(false, "MenuItemID not found in the specified menu");
}

bool oWinMenuIsChecked(HMENU _hMenu, int _MenuItemID)
{
	MENUITEMINFO mii;
	ZeroMemory(&mii, sizeof(mii));
	mii.cbSize = sizeof(mii);
	mii.fMask = MIIM_STATE;
	oVB(GetMenuItemInfo(_hMenu, (UINT)_MenuItemID, FALSE, &mii));
	if (mii.fState & MFS_CHECKED)
		return true;
	return false;
}

void oWinMenuEnable(HMENU _hMenu, int _MenuItemID, bool _Enabled)
{
	if (-1 == EnableMenuItem(_hMenu, (UINT)_MenuItemID, MF_BYCOMMAND | (_Enabled ? MF_ENABLED : MF_GRAYED)))
		oASSERT(false, "MenuItemID not found in the specified menu");
}

bool oWinMenuIsEnabled(HMENU _hMenu, int _MenuItemID)
{
	MENUITEMINFO mii;
	ZeroMemory(&mii, sizeof(mii));
	mii.cbSize = sizeof(mii);
	mii.fMask = MIIM_STATE;
	oVB(GetMenuItemInfo(_hMenu, (UINT)_MenuItemID, FALSE, &mii));
	if (mii.fState & (MF_GRAYED|MF_DISABLED))
		return false;
	return true;
}
