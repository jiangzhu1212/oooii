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
// Wrappers for creating a Windows dialog.
#pragma once
#ifndef oWinDialog_h
#define oWinDialog_h

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

enum oWINDOWS_DIALOG_ITEM_TYPE
{
	oDLG_BUTTON,
	oDLG_EDITBOX,
	oDLG_LABEL_LEFT_ALIGNED,
	oDLG_LABEL_CENTERED,
	oDLG_LABEL_RIGHT_ALIGNED,
	oDLG_LARGELABEL,
	oDLG_ICON,
	oDLG_LISTBOX,
	oDLG_SCROLLBAR,
	oDLG_COMBOBOX,
};

struct oWINDOWS_DIALOG_ITEM
{
	const char* Text;
	oWINDOWS_DIALOG_ITEM_TYPE Type;
	WORD ItemID;
	RECT Rect;
	bool Enabled;
	bool Visible;
	bool TabStop;
};

struct oWINDOWS_DIALOG_DESC
{
	const char* Font;
	const char* Caption;
	const oWINDOWS_DIALOG_ITEM* pItems;
	UINT NumItems;
	UINT FontPointSize;
	RECT Rect;
	bool Center; // if true, ignores Rect.left, Rect.top positioning
	bool SetForeground;
	bool Enabled;
	bool Visible;
	bool AlwaysOnTop;
};

LPDLGTEMPLATE oDlgNewTemplate(const oWINDOWS_DIALOG_DESC& _Desc);
void oDlgDeleteTemplate(LPDLGTEMPLATE _lpDlgTemplate);

#endif
