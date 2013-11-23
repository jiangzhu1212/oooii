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
#include <oGUI/Windows/oWinCursor.h>
#include <oGUI/Windows/oWinRect.h>
#include <oGUI/Windows/oWinWindowing.h>
#include <oCore/windows/win_error.h>

#define oWINV(_hWnd) \
	if (!oWinExists(_hWnd)) \
	{	oErrorSetLast(std::errc::invalid_argument, "Invalid HWND %p specified", _hWnd); \
	return false; \
	}

#define oWINVP(_hWnd) \
	if (!oWinExists(_hWnd)) \
	{	oErrorSetLast(std::errc::invalid_argument, "Invalid HWND %p specified", _hWnd); \
	return nullptr; \
	}

static bool oWinGetHonestClientScreenRect(HWND _hWnd, RECT* _pRect)
{
	oVB(GetClientRect(_hWnd, _pRect));
	POINT p = { _pRect->left, _pRect->top };
	oVB(ClientToScreen(_hWnd, &p));
	*_pRect = oWinRectTranslate(*_pRect, p);
	return true;
}

bool oWinCursorSetClipped(HWND _hWnd, bool _Clipped)
{
	oWINV(_hWnd);

	if (_Clipped)
	{
		// allow cursor over status bar, so clip to honest client size, not spoofed
		RECT r;
		oVB(oWinGetHonestClientScreenRect(_hWnd, &r));
		oVB(ClipCursor(&r));
	}

	else
		oVB(ClipCursor(0));

	return true;
}

bool oWinCursorGetClipped(HWND _hWnd)
{
	oWINV(_hWnd);
	RECT rClip, rClient;
	oVB(GetClipCursor(&rClip));
	oVB(oWinGetHonestClientScreenRect(_hWnd, &rClient));
	return !memcmp(&rClip, &rClient, sizeof(RECT));
}

bool oWinCursorIsVisible()
{
	CURSORINFO ci;
	ci.cbSize = sizeof(CURSORINFO);
	GetCursorInfo(&ci);
	return ci.flags == CURSOR_SHOWING;
}

void oWinCursorSetVisible(bool _Visible)
{
	if (_Visible)
		while (ShowCursor(true) < 0) {}
	else
		while (ShowCursor(false) > -1) {}
}

int2 oWinCursorGetPosition(HWND _hWnd)
{
	POINT p;
	oVB(GetCursorPos(&p));
	if (::IsWindow(_hWnd))
		oVB(ScreenToClient(_hWnd, &p));
	return int2(p.x, p.y);
}

void oWinCursorSetPosition(HWND _hWnd, const int2& _Position)
{
	POINT p = { _Position.x, _Position.y };
	if (::IsWindow(_hWnd))
		oVB(ClientToScreen(_hWnd, &p));
	oVB(SetCursorPos(p.x, p.y));
}

HCURSOR oWinGetCursor(HWND _hWnd)
{
	oWINVP(_hWnd);
	return (HCURSOR)GetClassLongPtr(_hWnd, GCLP_HCURSOR);
}

bool oWinSetCursor(HWND _hWnd, HCURSOR _hCursor)
{
	oWINVP(_hWnd);
	oVB(SetClassLongPtr(_hWnd, GCLP_HCURSOR, (LONG_PTR)_hCursor));
	return true;
}

HCURSOR oWinGetCursor(oGUI_CURSOR_STATE _State, HCURSOR _hUserCursor)
{
	LPSTR cursors[] =
	{
		nullptr,
		IDC_ARROW,
		IDC_HAND,
		IDC_HELP,
		IDC_NO,
		IDC_WAIT,
		IDC_APPSTARTING,
		nullptr,
	};

	return _State == oGUI_CURSOR_USER ? _hUserCursor : LoadCursor(nullptr, cursors[_State]);
}

void oWinCursorSetState(HWND _hWnd, oGUI_CURSOR_STATE _CursorState, HCURSOR _hUserCursor)
{
	HCURSOR hCursor = oWinGetCursor(_CursorState, _hUserCursor);
	oVB(oWinSetCursor(_hWnd, hCursor));
	oWinCursorSetVisible(_CursorState != oGUI_CURSOR_NONE);
}

oGUI_CURSOR_STATE oWinCursorGetState(HWND _hWnd)
{
	oASSERT(oWinExists(_hWnd), "Invalid _hWnd specified");
	if (!oWinCursorIsVisible())
		return oGUI_CURSOR_NONE;
	HCURSOR hCursor = (HCURSOR)GetClassLongPtr(_hWnd, GCLP_HCURSOR);
	for (int i = oGUI_CURSOR_ARROW; i < oGUI_CURSOR_USER; i++)
	{
		HCURSOR hStateCursor = oWinGetCursor((oGUI_CURSOR_STATE)i);
		if (hCursor == hStateCursor)
			return (oGUI_CURSOR_STATE)i;
	}
	return oGUI_CURSOR_USER;
}
