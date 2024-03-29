// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.
#include <oGUI/Windows/oWinCursor.h>
#include <oGUI/Windows/oWinRect.h>
#include <oGUI/Windows/oWinWindowing.h>
#include <oCore/windows/win_error.h>

#define oWINV(_hWnd) if (!oWinExists(_hWnd)) oTHROW_INVARG("Invalid HWND %p specified", _hWnd);

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
	oWINV(_hWnd);
	return (HCURSOR)GetClassLongPtr(_hWnd, GCLP_HCURSOR);
}

bool oWinSetCursor(HWND _hWnd, HCURSOR _hCursor)
{
	oWINV(_hWnd);
	oVB(SetClassLongPtr(_hWnd, GCLP_HCURSOR, (LONG_PTR)_hCursor));
	return true;
}

HCURSOR oWinGetCursor(ouro::cursor_state::value _State, HCURSOR _hUserCursor)
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

	return _State == ouro::cursor_state::user ? _hUserCursor : LoadCursor(nullptr, cursors[_State]);
}

void oWinCursorSetState(HWND _hWnd, ouro::cursor_state::value _CursorState, HCURSOR _hUserCursor)
{
	HCURSOR hCursor = oWinGetCursor(_CursorState, _hUserCursor);
	oVB(oWinSetCursor(_hWnd, hCursor));
	oWinCursorSetVisible(_CursorState != ouro::cursor_state::none);
}

ouro::cursor_state::value oWinCursorGetState(HWND _hWnd)
{
	oASSERT(oWinExists(_hWnd), "Invalid _hWnd specified");
	if (!oWinCursorIsVisible())
		return ouro::cursor_state::none;
	HCURSOR hCursor = (HCURSOR)GetClassLongPtr(_hWnd, GCLP_HCURSOR);
	for (int i = ouro::cursor_state::arrow; i < ouro::cursor_state::user; i++)
	{
		HCURSOR hStateCursor = oWinGetCursor((ouro::cursor_state::value)i);
		if (hCursor == hStateCursor)
			return (ouro::cursor_state::value)i;
	}
	return ouro::cursor_state::user;
}
