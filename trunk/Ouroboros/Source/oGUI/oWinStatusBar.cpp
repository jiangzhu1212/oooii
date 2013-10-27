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
#include <oGUI/Windows/oWinStatusBar.h>
#include <oGUI/Windows/oWinRect.h>
#include <oGUI/Windows/oWinWindowing.h>
#include <CommCtrl.h>

using namespace ouro;

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

bool oWinIsStatusBar(HWND _hStatusBar)
{
	sstring ClassName;
	if (!GetClassName(_hStatusBar, ClassName.c_str(), oInt(ClassName.capacity())))
		return false;

	return !strcmp(ClassName, STATUSCLASSNAME);
}

void oWinStatusBarSetMinHeight(HWND _hStatusBar, int _MinHeight)
{
	SendMessage(_hStatusBar, SB_SETMINHEIGHT, (WPARAM)_MinHeight, 0);
}

void oWinStatusBarSetNumItems(HWND _hStatusBar, const int* _pItemWidths, size_t _NumItems)
{
	std::array<int, 256> CoordOfRight;

	int LastLeft = 0;
	for (int i = 0; i < oInt(_NumItems); i++)
	{
		LastLeft += _pItemWidths[i];
		
		if (_pItemWidths[i] == -1)
		{
			CoordOfRight[i] = -1;
			break;
		}
		CoordOfRight[i] = LastLeft;
	}

	oVB(SendMessage(_hStatusBar, SB_SETPARTS, (WPARAM)_NumItems, (LPARAM)CoordOfRight.data()));
}

int oWinStatusBarGetNumItems(HWND _hStatusBar, int* _pItemWidths, size_t _MaxNumItemWidths)
{
	return (int)SendMessage(_hStatusBar, SB_GETPARTS, (WPARAM)(_MaxNumItemWidths == 0 ? INT_MAX : _MaxNumItemWidths), (LPARAM)_pItemWidths);
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
	lstring s;
	vsnprintf(s, _Format, _Args);
	ellipsize(s);
	WPARAM w = (_ItemIndex & 0xff) | oWinStatusBarGetStyle(_BorderStyle);
	if (!SendMessage(_hStatusBar, SB_SETTEXT, w, (LPARAM)s.c_str()))
	{
		int nParts = (int)SendMessage(_hStatusBar, SB_GETPARTS, INT_MAX, 0);
		if (_ItemIndex >= nParts)
		{
			oErrorSetLast(std::errc::no_buffer_space, "The specified status bar item index %d is out of range (number of items %d)", _ItemIndex, nParts);
			oVERIFY(false);
		}

		else
			oVB(false);
	}
}

char* oWinStatusBarGetText(char* _StrDestination, size_t _SizeofStrDestination, HWND _hStatusBar, int _ItemIndex)
{
	LRESULT lResult = SendMessage(_hStatusBar, SB_GETTEXTLENGTH, (WPARAM)_ItemIndex, 0);
	size_t len = LOWORD(lResult);
	if (len >= _SizeofStrDestination)
		return (char*)oErrorSetLast(std::errc::no_buffer_space);
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
