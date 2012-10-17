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
#include <oPlatform/Windows/oWinStatusBar.h>
#include <oPlatform/Windows/oWinRect.h>

HWND oWinStatusBarCreate(HWND _hParent, HMENU _ID, int _MinHeight)
{
	HWND hWnd = CreateWindowEx(
		0
		, STATUSCLASSNAME
		, ""
		, WS_VISIBLE|WS_CHILD|SBARS_SIZEGRIP|SBARS_TOOLTIPS
		, 0
		, 0
		, 0
		, 0
		, (HWND)_hParent
		, _ID
		, nullptr
		, nullptr);

	if (_MinHeight != oDEFAULT)
		SendMessage(hWnd, SB_SETMINHEIGHT, (WPARAM)_MinHeight, 0);
	SendMessage(hWnd, WM_SIZE, 0, 0);
	return hWnd;
}

void oWinStatusBarAdjustClientRect(HWND _hParent, RECT* _pRect)
{
	HWND hStatusBar = FindWindowEx(_hParent, nullptr, STATUSCLASSNAME, nullptr);
	if (hStatusBar)
	{
		RECT rStatusBar;
		GetClientRect(hStatusBar, &rStatusBar);
		_pRect->bottom += oWinRectH(rStatusBar);
	}
}

void oWinStatusBarSyncOnSize(HWND _hStatusBar)
{
	SendMessage(_hStatusBar, WM_SIZE, 0, 0);
}

void oWinStatusBarSetMinHeight(HWND _hStatusBar, int _MinHeight)
{
	SendMessage(_hStatusBar, SB_SETMINHEIGHT, (WPARAM)_MinHeight, 0);
}

void oWinStatusBarSetNumItems(HWND _hStatusBar, const int* _pItemWidths, size_t _NumItems)
{
	oVB(SendMessage(_hStatusBar, SB_SETPARTS, (WPARAM)_NumItems, (LPARAM)_pItemWidths));
}

RECT oWinStatusBarGetItemRect(HWND _hStatusBar, int _ItemIndex)
{
	RECT r;
	oVB(SendMessage(_hStatusBar, SB_GETRECT, (WPARAM)_ItemIndex, (LPARAM)&r));
	return r;
}

static int oWinStatusBarGetStyle(oGUI_BORDER_STYLE _Style)
{
	switch (_Style)
	{
		case oGUI_BORDER_SUNKEN: return 0;
		case oGUI_BORDER_FLAT: return SBT_NOBORDERS;
		case oGUI_BORDER_RAISED: return SBT_POPOUT;
		oNODEFAULT;
	}
}

void oWinStatusBarSetText(HWND _hStatusBar, int _ItemIndex, oGUI_BORDER_STYLE _BorderStyle, const char* _Format, va_list _Args)
{
	oStringL s;
	oVPrintf(s, _Format, _Args);
	oAddTruncationElipse(s);
	WPARAM w = (_ItemIndex & 0xff) | oWinStatusBarGetStyle(_BorderStyle);
	oVB(SendMessage(_hStatusBar, SB_SETTEXT, w, (LPARAM)s.c_str()));
}

char* oWinStatusBarGetText(char* _StrDestination, size_t _SizeofStrDestination, HWND _hStatusBar, int _ItemIndex)
{
	LRESULT lResult = SendMessage(_hStatusBar, SB_GETTEXTLENGTH, (WPARAM)_ItemIndex, 0);
	size_t len = LOWORD(lResult);
	if (len >= _SizeofStrDestination)
		return (char*)oErrorSetLast(oERROR_AT_CAPACITY);
	SendMessage(_hStatusBar, SB_GETTEXT, (WPARAM)_ItemIndex, (LPARAM)_StrDestination);
	return _StrDestination;
}

void oWinStatusBarSetTipText(HWND _hStatusBar, int _ItemIndex, const char* _Text)
{
	SendMessage(_hStatusBar, SB_SETTEXT, (WPARAM)_ItemIndex, (LPARAM)_Text);
}

char* oWinStatusBarGetTipText(char* _StrDestination, size_t _SizeofStrDestination, HWND _hStatusBar, int _ItemIndex)
{
	UINT w = (UINT)(oUShort(_SizeofStrDestination) << 16) | (UINT)(_ItemIndex & 0xffff);
	SendMessage(_hStatusBar, SB_GETTIPTEXT, (WPARAM)w, (LPARAM)_StrDestination);
	return _StrDestination;
}

void oWinStatusBarSetIcon(HWND _hStatusBar, int _ItemIndex, HICON _hIcon)
{
	oVB(SendMessage(_hStatusBar, SB_SETICON, (WPARAM)_ItemIndex, (LPARAM)_hIcon));
}

HICON oWinStatusBarGetIcon(HWND _hStatusBar, int _ItemIndex)
{
	return (HICON)SendMessage(_hStatusBar, SB_GETICON, (WPARAM)_ItemIndex, 0);
}
