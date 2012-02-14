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
#include <oPlatform/oWinWindowing.h>
#include <oBasis/oAssert.h>
#include <oBasis/oFixedString.h>
#include <oBasis/oStdChrono.h>
#include <oPlatform/oWinRect.h>
#include <oPlatform/oWinAsString.h>
#include <WindowsX.h>

bool oWinCreate(HWND* _pHwnd, WNDPROC _Wndproc, void* _pThis, bool _SupportDoubleClicks, unsigned int _ClassUniqueID)
{
	if (!_pHwnd)
		return oErrorSetLast(oERROR_INVALID_PARAMETER);

	char className[32];
	sprintf_s(className, "OOOii.Window%s%u", _SupportDoubleClicks ? "DblClks" : "", _ClassUniqueID);

	WNDCLASSEXA wc;
	ZeroMemory(&wc, sizeof(WNDCLASSEX));
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.hInstance = GetModuleHandle(0);
	wc.lpfnWndProc = _Wndproc;                    
	wc.lpszClassName = className;                        
	wc.style = CS_BYTEALIGNCLIENT|CS_HREDRAW|CS_VREDRAW|CS_OWNDC|(_SupportDoubleClicks ? CS_DBLCLKS : 0);
	wc.hCursor = LoadCursor(0, IDC_ARROW);
	wc.hbrBackground = GetSysColorBrush(COLOR_3DFACE);
	if (0 == RegisterClassEx(&wc) && GetLastError() != ERROR_CLASS_ALREADY_EXISTS)
		return oWinSetLastError();

	*_pHwnd = CreateWindowEx(WS_EX_ACCEPTFILES|WS_EX_APPWINDOW, className, "", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, 0, 0, 0, _pThis);
	if (!*_pHwnd)
		return false; // pass through error

	return true;
}

HBRUSH oWinGetBackgroundBrush(HWND _hWnd)
{
	return (HBRUSH)GetClassLongPtr(_hWnd, GCLP_HBRBACKGROUND);
}

void* oWinGetThis(HWND _hWnd, UINT _uMsg, WPARAM _wParam, LPARAM _lParam)
{
	if (!_hWnd)
		return 0;

	void* context = 0;

	if (_uMsg == WM_CREATE)
	{
		// a context (probably some 'this' pointer) was passed during the call 
		// to CreateWindow, so put that in userdata.
		CREATESTRUCTA* cs = (CREATESTRUCTA*)_lParam;
		SetWindowLongPtr(_hWnd, GWLP_USERDATA, (LONG_PTR)cs->lpCreateParams);
		context = (void*)cs->lpCreateParams;
	}

	else if (_uMsg == WM_INITDIALOG)
	{
		// dialogs don't use CREATESTRUCT, so assume it's directly the context
		SetWindowLongPtr(_hWnd, GWLP_USERDATA, (LONG_PTR)_lParam);
		context = (void*)_lParam;
	}

	else
	{
		// For any other message, grab the context and return it
		context = _hWnd ? (void*)GetWindowLongPtr(_hWnd, GWLP_USERDATA) : 0;
	}

	return context;
}

#define oWINV(_hWnd) \
	if (!oWinExists(_hWnd)) \
	{	oErrorSetLast(oERROR_INVALID_PARAMETER, "Invalid HWND %p specified", _hWnd); \
		return false; \
	}

#define oWINVP(_hWnd) \
	if (!oWinExists(_hWnd)) \
	{	oErrorSetLast(oERROR_INVALID_PARAMETER, "Invalid HWND %p specified", _hWnd); \
		return nullptr; \
	}

using namespace oStd::chrono;
bool oWinPumpMessages(HWND _hWnd, unsigned int _TimeoutMS)
{
	oWINV(_hWnd);
	MSG msg;
	high_resolution_clock::time_point start = high_resolution_clock::now();
	while (PeekMessage(&msg, _hWnd, 0, 0, PM_REMOVE))
	{
		if (_TimeoutMS == oINFINITE_WAIT || oSeconds(high_resolution_clock::now() - start) < milliseconds(_TimeoutMS))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		else
		{
			oErrorSetLast(oERROR_TIMEOUT);
			return false;
		}
	}

	return true;
}

bool oWinWake(HWND _hWnd)
{
	oWINV(_hWnd);
	return !!PostMessage(_hWnd, WM_NULL, 0, 0);
}

bool oWinExists(HWND _hWnd)
{
	return !!::IsWindow(_hWnd);
}

bool oWinHasFocus(HWND _hWnd)
{
	return oWinExists(_hWnd) && _hWnd == ::GetForegroundWindow();
}

bool oWinSetFocus(HWND _hWnd, bool _Focus)
{
	oWINV(_hWnd);
	if (_Focus)
	{
		::SetForegroundWindow(_hWnd);
		::SetActiveWindow(_hWnd);
		::SetFocus(_hWnd);
		return true;
	}
	
	else
		return oWinSetFocus(GetNextWindow(_hWnd, GW_HWNDNEXT), true);
}

bool oWinGetEnabled(HWND _hWnd)
{
	oWINV(_hWnd);
	oVB_RETURN(IsWindowEnabled(_hWnd));
	return true;
}

bool oWinSetEnabled(HWND _hWnd, bool _Enabled)
{
	oWINV(_hWnd);
	oVB_RETURN(EnableWindow(_hWnd, BOOL(_Enabled)));
	return true;
}

oWINDOW_STATE oWinGetState(HWND _hWnd)
{
	if (!oWinExists(_hWnd)) return oWINDOW_NONEXISTANT;
	else if (!IsWindowVisible(_hWnd)) return oWINDOW_HIDDEN;
	else if (IsIconic(_hWnd)) return oWINDOW_MINIMIZED;
	else if (IsZoomed(_hWnd)) return oWINDOW_MAXIMIZED;
	else return oWINDOW_RESTORED;
}

// Returns a value fit to be passed to ShowWindow()
static int oWinGetShowCommand(oWINDOW_STATE _State, bool _TakeFocus)
{
	switch (_State)
	{
		case oWINDOW_NONEXISTANT: return SW_HIDE;
		case oWINDOW_HIDDEN: return SW_HIDE;
		case oWINDOW_MINIMIZED: return _TakeFocus ? SW_SHOWMINIMIZED : SW_SHOWMINNOACTIVE;
		case oWINDOW_MAXIMIZED: return SW_SHOWMAXIMIZED;
		case oWINDOW_RESTORED: return _TakeFocus ? SW_SHOWNORMAL : SW_SHOWNOACTIVATE;
		oNODEFAULT;
	}
}

bool oWinRestore(HWND _hWnd)
{
	oWINV(_hWnd);
	HWND hProgMan = FindWindow(0, "Program Manager");
	oASSERT(hProgMan, "Program Manager not found");
	oWinSetFocus(hProgMan);
	oWinSetFocus(_hWnd);
	ShowWindow(_hWnd, SW_SHOWDEFAULT);
	return true;
}

bool oWinSetState(HWND _hWnd, oWINDOW_STATE _State, bool _TakeFocus)
{
	oWINV(_hWnd);

	// There's a known issue that a simple ShowWindow doesn't always work on 
	// some minimized apps. The WAR seems to be to set focus to anything else, 
	// then try to restore the app.
	if (_TakeFocus && oWinGetState(_hWnd) == oWINDOW_MINIMIZED && _State > oWINDOW_MINIMIZED)
	{
		HWND hProgMan = FindWindow(nullptr, "Program Manager");
		oASSERT(hProgMan, "Program Manager not found");
		oWinSetFocus(hProgMan);
	}

	if (!ShowWindow(_hWnd, oWinGetShowCommand(_State, _TakeFocus)))
		oVB_RETURN(RedrawWindow(_hWnd, nullptr, nullptr, RDW_INVALIDATE|RDW_UPDATENOW));
	return true;
}

oWINDOW_STYLE oWinGetStyle(HWND _hWnd)
{
	#define oFIXED_STYLE (WS_CAPTION|WS_SYSMENU|WS_MINIMIZEBOX)

	LONG_PTR style = GetWindowLongPtr(_hWnd, GWL_STYLE);
	if ((style & WS_OVERLAPPEDWINDOW) == WS_OVERLAPPEDWINDOW) return oWINDOW_SIZEABLE;
	else if ((style & oFIXED_STYLE) == oFIXED_STYLE) return oWINDOW_FIXED;
	else if ((style & WS_POPUP) == WS_POPUP) return oWINDOW_BORDERLESS;
	return oWINDOW_DIALOG;
}

static DWORD oWinGetStyle(oWINDOW_STYLE _Style)
{
	switch (_Style)
	{
		case oWINDOW_BORDERLESS: return WS_POPUP;
		case oWINDOW_DIALOG: return WS_CAPTION;
		case oWINDOW_FIXED: return WS_CAPTION|WS_SYSMENU|WS_MINIMIZEBOX;
		default: return WS_OVERLAPPEDWINDOW;
	}
}

bool oWinSetStyle(HWND _hWnd, oWINDOW_STYLE _Style, const RECT* _prClient)
{
	oWINV(_hWnd);

	// Basically only change the bits we mean to change and preserve the others
	DWORD dwCurrentStyleFlags = oWinGetStyle(oWinGetStyle(_hWnd));
	DWORD dwAllFlags = (DWORD)GetWindowLongPtr(_hWnd, GWL_STYLE);
	dwAllFlags &=~ dwCurrentStyleFlags;
	dwAllFlags |= oWinGetStyle(_Style);

	UINT uFlags = SWP_NOZORDER|SWP_FRAMECHANGED;
	if (dwAllFlags & (WS_MAXIMIZE|WS_MINIMIZE)) // ignore user size/move settings if maximized or minimized
		uFlags |= SWP_NOMOVE|SWP_NOSIZE;

	RECT r;
	if (_prClient)
	{
		r = *_prClient;
		if (r.right == oDEFAULT || r.bottom == oDEFAULT)
			uFlags |= SWP_NOSIZE;
	}

	else
		oVB_RETURN(oWinGetClientScreenRect(_hWnd, &r));

	oVB_RETURN(AdjustWindowRect(&r, dwAllFlags, FALSE));

	SetLastError(0); // http://msdn.microsoft.com/en-us/library/ms644898(VS.85).aspx
	oVB_RETURN(SetWindowLongPtr(_hWnd, GWL_STYLE, dwAllFlags));
	oVB_RETURN(SetWindowPos(_hWnd, 0, r.left, r.top, oWinRectW(r), oWinRectH(r), uFlags));
	return true;
}

int2 oWinGetPosition(HWND _hWnd)
{
	if (!IsWindow(_hWnd))
		return int2(oDEFAULT, oDEFAULT);
	POINT p = {0,0};
	ClientToScreen(_hWnd, &p);
	return int2(p.x, p.y);
}

bool oWinSetPosition(HWND _hWnd, const int2& _ScreenPosition)
{
	oWINV(_hWnd);
	RECT r = oWinRectWH(_ScreenPosition, int2(100,100));
	oVB_RETURN(AdjustWindowRect(&r, oWinGetStyle(oWinGetStyle(_hWnd)), FALSE));
	oVB_RETURN(SetWindowPos(_hWnd, 0, r.left, r.top, 0, 0, SWP_NOSIZE|SWP_NOZORDER));
	return true;
}

bool oWinAnimate(HWND _hWnd, const RECT& _From, const RECT& _To)
{
	oWINV(_hWnd);
	ANIMATIONINFO ai;
	ai.cbSize = sizeof(ai);
	SystemParametersInfo(SPI_GETANIMATION, sizeof(ai), &ai, 0);
	if (ai.iMinAnimate)
		return !!DrawAnimatedRects(_hWnd, IDANI_CAPTION, &_From, &_To);
	return true;
}

bool oWinSetTitle(HWND _hWnd, const char* _Title)
{
	oWINV(_hWnd);
	if (!SetWindowText(_hWnd, _Title))
		return oWinSetLastError();
	return true;
}

bool oWinGetTitle(HWND _hWnd, char* _Title, size_t _SizeofTitle)
{
	oWINV(_hWnd);
	if (!GetWindowText(_hWnd, _Title, static_cast<int>(_SizeofTitle)))
		return oWinSetLastError();
	return true;
}

bool oWinGetAlwaysOnTop(HWND _hWnd)
{
	oWINV(_hWnd);
	return !!(GetWindowLong(_hWnd, GWL_EXSTYLE) & WS_EX_TOPMOST);
}

bool oWinSetAlwaysOnTop(HWND _hWnd, bool _AlwaysOnTop)
{
	oWINV(_hWnd);
	RECT r;
	oVB_RETURN(GetWindowRect(_hWnd, &r));
	return !!::SetWindowPos(_hWnd, _AlwaysOnTop ? HWND_TOPMOST : HWND_TOP, r.left, r.top, oWinRectW(r), oWinRectH(r), IsWindowVisible(_hWnd) ? SWP_SHOWWINDOW : SWP_HIDEWINDOW);
}

HICON oWinGetIcon(HWND _hWnd, bool _BigIcon)
{
	oWINV(_hWnd);
	return (HICON)SendMessage(_hWnd, WM_GETICON, (WPARAM)(_BigIcon ? ICON_BIG : ICON_SMALL), 0);
}

bool oWinSetIcon(HWND _hWnd, HICON _hIcon, bool _BigIcon)
{
	oWINV(_hWnd);
	return !!SendMessage(_hWnd, WM_SETICON, (WPARAM)(_BigIcon ? ICON_BIG : ICON_SMALL), (LPARAM)_hIcon);
}

unsigned int oWinGetDisplayIndex(HWND _hWnd)
{
	DISPLAY_DEVICE dev;
	return oWinGetDisplayDevice(MonitorFromWindow(_hWnd, MONITOR_DEFAULTTONEAREST), &dev);
}

struct WFIND
{
	DWORD ThreadID;
	const char* Title;
	HWND hWnd;
};

bool oWinGetClientScreenRect(HWND _hWnd, RECT* _pRect)
{
	oWINV(_hWnd);
	if (!_pRect)
	{
		oErrorSetLast(oERROR_INVALID_PARAMETER);
		return false;
	}
	
	oVB_RETURN(GetClientRect(_hWnd, _pRect));
	POINT p = { _pRect->left, _pRect->top };
	oVB_RETURN(ClientToScreen(_hWnd, &p));
	_pRect->left = p.x;
	_pRect->top = p.y;
	_pRect->right += p.x;
	_pRect->bottom += p.y;
	return true;
}

bool oWinCursorSetClipped(HWND _hWnd, bool _Clipped)
{
	oWINV(_hWnd);

	if (_Clipped)
	{
		RECT r;
		oWinGetClientScreenRect(_hWnd, &r);
		oVB_RETURN(ClipCursor(&r));
	}

	else
		oVB_RETURN(ClipCursor(0));

	return true;
}

bool oWinCursorGetClipped(HWND _hWnd)
{
	oWINV(_hWnd);
	RECT rClip, rClient;
	oVB_RETURN(GetClipCursor(&rClip));
	oVB_RETURN(GetClientRect(_hWnd, &rClient));
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

bool oWinCursorGetPosition(HWND _hWnd, int2* _pClientPosition)
{
	oWINV(_hWnd);
	if (!_pClientPosition)
	{
		oErrorSetLast(oERROR_INVALID_PARAMETER);
		return false;
	}

	POINT p;
	oVB_RETURN(GetCursorPos(&p));
	oVB_RETURN(ScreenToClient(_hWnd, &p));
	_pClientPosition->x = p.x;
	_pClientPosition->y = p.y;
	return true;
}

bool oWinCursorSetPosition(HWND _hWnd, const int2& _ClientPosition)
{
	oWINV(_hWnd);
	POINT p = { _ClientPosition.x, _ClientPosition.y };
	oVB_RETURN(ClientToScreen(_hWnd, &p));
	oVB_RETURN(SetCursorPos(p.x, p.y));
	return true;
}

HCURSOR oWinGetCursor(HWND _hWnd)
{
	oWINVP(_hWnd);
	return (HCURSOR)GetClassLongPtr(_hWnd, GCLP_HCURSOR);
}

bool oWinSetCursor(HWND _hWnd, HCURSOR _hCursor)
{
	oWINVP(_hWnd);
	if (0 == SetClassLongPtr(_hWnd, GCLP_HCURSOR, (LONG_PTR)_hCursor))
	{
		oWinSetLastError();
		return false;
	}
	
	return true;
}

void oWinButtonSetChecked(HWND _hWndButton, bool _Checked)
{
	Button_SetCheck(_hWndButton, _Checked ? BST_CHECKED : BST_UNCHECKED);
}

bool oWinButtonGetChecked(HWND _hWndButton)
{
	return BST_CHECKED == Button_GetState(_hWndButton);
}

void oWinElementSetText(HWND _hWndElement, const char* _Text)
{
	oVB(SetWindowText(_hWndElement, _Text));
}

int oWinComboboxGetTextIndex(HWND _hWndCombobox, const char* _Text)
{
	int index = ComboBox_FindStringExact(_hWndCombobox, -1, _Text);
	if (index == CB_ERR)
		oErrorSetLast(oERROR_NOT_FOUND, "Text %s was not found in combobox", oSAFESTRN(_Text));
	return index;
}

bool oWinComboboxSetSelection(HWND _hWndCombobox, int _Index)
{
	if (CB_ERR == ComboBox_SetCurSel(_hWndCombobox, _Index))
		return oErrorSetLast(oERROR_NOT_FOUND, "Index %d was not found in combobox", _Index);
	return true;
}

bool oWinComboboxSetSelection(HWND _hWndCombobox, const char* _Text)
{
	int index = oWinComboboxGetTextIndex(_hWndCombobox, _Text);
	if (-1 == index)
		return false; // pass through error
	return oWinComboboxSetSelection(_hWndCombobox, index);
}

bool oWinComboboxGetSelectedText(HWND _hWndCombobox, char* _StrDestination, size_t _SizeofStrDestination)
{
	size_t len = ComboBox_GetTextLength(_hWndCombobox);
	if (len >= _SizeofStrDestination)
		return oErrorSetLast(oERROR_AT_CAPACITY, "Buffer too small to receive string");
	ComboBox_GetText(_hWndCombobox, _StrDestination, static_cast<int>(_SizeofStrDestination));
	return true;
}

void oWinComboboxClear(HWND _hWndCombobox)
{
	ComboBox_ResetContent(_hWndCombobox);
}

bool oWinComboboxAppendStrings(HWND _hWndCombobox, const char* _Tokens, const char* _Separator)
{
	char* ctx = nullptr;
	const char* tok = oStrTok(_Tokens, _Separator, &ctx);
	while (tok)
	{
		ComboBox_AddString(_hWndCombobox, tok);
		tok = oStrTok(nullptr, _Separator, &ctx);
	}

	if (!oStrTokFinishedSuccessfully(&ctx))
		return oErrorSetLast(oERROR_CORRUPT, "Failed to parse tokenized string for combobox values");
	return true;
}

static DWORD GetStyle(oWINDOW_CONTROL_TYPE _Type, bool _StartsNewGroup)
{
	static const DWORD sStyle[] =
	{
		0,
		WS_VISIBLE|WS_CHILD|BS_GROUPBOX,
		WS_VISIBLE|WS_CHILD|WS_TABSTOP|BS_PUSHBUTTON,
		WS_VISIBLE|WS_CHILD|WS_TABSTOP|BS_DEFPUSHBUTTON,
		WS_VISIBLE|WS_CHILD|WS_TABSTOP|BS_AUTOCHECKBOX,
		WS_VISIBLE|WS_CHILD|WS_TABSTOP|BS_AUTORADIOBUTTON,
		WS_VISIBLE|WS_CHILD|SS_SIMPLE|SS_WORDELLIPSIS,
		WS_VISIBLE|WS_CHILD|WS_TABSTOP|ES_READONLY|ES_NOHIDESEL|ES_AUTOHSCROLL,
		WS_VISIBLE|WS_CHILD|WS_TABSTOP|ES_NOHIDESEL|ES_AUTOHSCROLL,
		WS_VISIBLE|WS_CHILD|WS_TABSTOP|CBS_DROPDOWNLIST|CBS_HASSTRINGS,
		WS_VISIBLE|WS_CHILD|WS_TABSTOP|CBS_DROPDOWN|CBS_HASSTRINGS,
		WS_VISIBLE|WS_CHILD
	};

	DWORD dwStyle = sStyle[_Type];
	if (_StartsNewGroup)
		dwStyle |= WS_GROUP;
	return dwStyle;
}

static const char* GetClass(oWINDOW_CONTROL_TYPE _Type)
{
	static const char* sClass[] =
	{
		"Unknown",
		"Button",
		"Button",
		"Button",
		"Button",
		"Button",
		"Static",
		"Edit",
		"Edit",
		"Combobox",
		"Combobox",
		PROGRESS_CLASS,
	};

	return sClass[_Type];
}

HWND oWinControlCreate(const oWINDOW_CONTROL_DESC& _Desc)
{
	if (!_Desc.hParent || _Desc.Type == oWINDOW_CONTROL_UNKNOWN)
		return (HWND)oErrorSetLast(oERROR_INVALID_PARAMETER);

	const bool IsListControl = _Desc.Type == oWINDOW_CONTROL_COMBOBOX || _Desc.Type == oWINDOW_CONTROL_COMBOTEXTBOX;

	HWND hWnd = CreateWindowEx(
		_Desc.Type == oWINDOW_CONTROL_TEXTBOX ? WS_EX_CLIENTEDGE : 0
		, GetClass(_Desc.Type)
		, IsListControl ? "" : oSAFESTR(_Desc.Text)
		, GetStyle(_Desc.Type, _Desc.StartsNewGroup)
		, _Desc.Position.x
		, _Desc.Position.y
		, _Desc.Dimensions.x
		, _Desc.Dimensions.y
		, _Desc.hParent
		, (HMENU)_Desc.ID
		, nullptr
		, nullptr);

	if (hWnd)
	{
		SendMessage(hWnd, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), 1);

		if (IsListControl)
		{
			if (!oWinComboboxAppendStrings(hWnd, _Desc.Text, "|"))
				return nullptr; // pass through last error

			oWinComboboxSetSelection(hWnd, 0);
		}
	}

	else
		oWinSetLastError();

	return hWnd;
}

oWINDOW_CONTROL_TYPE oWinControlGetType(HWND _hWnd)
{
	oStringM ClassName;
	if (!GetClassName(_hWnd, ClassName, static_cast<int>(ClassName.capacity())))
		return oWINDOW_CONTROL_UNKNOWN;
	
	if (!_stricmp("Button", ClassName))
	{
		LONG dwStyle = 0xff & GetWindowLong(_hWnd, GWL_STYLE);
		switch (dwStyle)
		{
			case BS_GROUPBOX: return oWINDOW_CONTROL_GROUPBOX;
			case BS_PUSHBUTTON: return oWINDOW_CONTROL_BUTTON;
			case BS_DEFPUSHBUTTON: return oWINDOW_CONTROL_BUTTON_DEFAULT;
			case BS_AUTOCHECKBOX: return oWINDOW_CONTROL_CHECKBOX;
			case BS_AUTORADIOBUTTON: return oWINDOW_CONTROL_RADIOBUTTON;
			default: return oWINDOW_CONTROL_UNKNOWN;
		}
	}

	else if (!_stricmp("Static", ClassName))
	{
		LONG dwStyle = 0xff & GetWindowLong(_hWnd, GWL_STYLE);
		switch (dwStyle)
		{
			case SS_SIMPLE: return oWINDOW_CONTROL_LABEL;
			default: return oWINDOW_CONTROL_UNKNOWN;
		}
	}

	else if (!_stricmp("Edit", ClassName))
	{
		DWORD dwStyle = 0xffff & (DWORD)GetWindowLong(_hWnd, GWL_STYLE);

		if (dwStyle == (dwStyle & GetStyle(oWINDOW_CONTROL_LABEL_SELECTABLE, false))) return oWINDOW_CONTROL_LABEL_SELECTABLE;
		else if (dwStyle == (dwStyle & GetStyle(oWINDOW_CONTROL_TEXTBOX, false))) return oWINDOW_CONTROL_TEXTBOX;
		else return oWINDOW_CONTROL_UNKNOWN;
	}

	else if (!_stricmp("ComboBox", ClassName))
	{
		LONG dwStyle = 0xf & GetWindowLong(_hWnd, GWL_STYLE);
		switch (dwStyle)
		{
			case CBS_DROPDOWNLIST: return oWINDOW_CONTROL_COMBOBOX;
			case CBS_DROPDOWN: return oWINDOW_CONTROL_COMBOTEXTBOX;
			default: return oWINDOW_CONTROL_UNKNOWN;
		}
	}

	else if (!_stricmp(PROGRESS_CLASS, ClassName))
	{
		return oWINDOW_CONTROL_PROGRESSBAR;
	}

	return oWINDOW_CONTROL_UNKNOWN;
}

unsigned short oWinControlGetID(HWND _hWnd)
{
	return static_cast<unsigned short>(GetDlgCtrlID(_hWnd));
}

HWND oWinControlGetFromID(HWND _hParent, unsigned short _ID)
{
	return GetDlgItem(_hParent, _ID);
}

void oWinControlSetText(HWND _hWnd, const char* _Text)
{
	oVB(SetWindowText(_hWnd, _Text));
}

char* oWinControlGetText(char* _StrDestination, size_t _SizeofStrDestination, HWND _hWnd, bool _OnlySelected)
{
	unsigned int start = 0, end = 0;

	size_t len = 0;

	if (_OnlySelected)
	{
		SendMessage(_hWnd, EM_GETSEL, (WPARAM)&start, (LPARAM)&end);
		len = end - start;
	}

	else
		len = GetWindowTextLength(_hWnd);

	if (len >= _SizeofStrDestination)
	{
		oErrorSetLast(oERROR_AT_CAPACITY, "Buffer too small to receive string");
		return nullptr;
	}

	if (!GetWindowText(_hWnd, _StrDestination, static_cast<int>(_SizeofStrDestination)))
	{
		oWinSetLastError();
		return nullptr;
	}

	if (_OnlySelected && start != end)
	{
		if (start)
			strcpy_s(_StrDestination, _SizeofStrDestination, _StrDestination + start);
		_StrDestination[len] = 0;
	}

	return _StrDestination;
}

bool oWinControlSetChecked(HWND _hWnd, bool _Checked)
{
	oWINDOW_CONTROL_TYPE type = oWinControlGetType(_hWnd);
	if (type != oWINDOW_CONTROL_CHECKBOX && type != oWINDOW_CONTROL_RADIOBUTTON)
		return oErrorSetLast(oERROR_INVALID_PARAMETER);
	Button_SetCheck(_hWnd, _Checked ? BST_CHECKED : BST_UNCHECKED);
	return true;
}

bool oWinControlIsChecked(HWND _hWnd)
{
	oWINDOW_CONTROL_TYPE type = oWinControlGetType(_hWnd);
	if (type != oWINDOW_CONTROL_CHECKBOX && type != oWINDOW_CONTROL_RADIOBUTTON)
		return oErrorSetLast(oERROR_INVALID_PARAMETER);

	oErrorSetLast(oERROR_NONE);
	LRESULT State = Button_GetState(_hWnd);
	return (State & BST_CHECKED) == BST_CHECKED;
}

bool oWinControlAddListItem(HWND _hWnd, const char* _ListItemText)
{
	oWINDOW_CONTROL_TYPE type = oWinControlGetType(_hWnd);
	if (type != oWINDOW_CONTROL_COMBOBOX && type != oWINDOW_CONTROL_COMBOTEXTBOX)
		return oErrorSetLast(oERROR_INVALID_PARAMETER);

	int result = ComboBox_AddString(_hWnd, _ListItemText);
	if (result == CB_ERRSPACE)
		return oErrorSetLast(oERROR_AT_CAPACITY, "String too long");
	return true;
}

bool oWinControlClearListItems(HWND _hWnd)
{
	oWINDOW_CONTROL_TYPE type = oWinControlGetType(_hWnd);
	if (type != oWINDOW_CONTROL_COMBOBOX && type != oWINDOW_CONTROL_COMBOTEXTBOX)
		return oErrorSetLast(oERROR_INVALID_PARAMETER);

	ComboBox_ResetContent(_hWnd);
	return true;
}

int oWinControlFindListItem(HWND _hWnd, const char* _ListItemText)
{
	oWINDOW_CONTROL_TYPE type = oWinControlGetType(_hWnd);
	if (type != oWINDOW_CONTROL_COMBOBOX && type != oWINDOW_CONTROL_COMBOTEXTBOX)
	{
		oErrorSetLast(oERROR_INVALID_PARAMETER);
		return CB_ERR;
	}

	int index = ComboBox_FindStringExact(_hWnd, 0, _ListItemText);
	if (index == CB_ERR)
		oErrorSetLast(oERROR_NOT_FOUND, "Text %s was not found in combobox", oSAFESTRN(_ListItemText));

	return index;
}

bool oWinControlSelect(HWND _hWnd, int _Index)
{
	oWINDOW_CONTROL_TYPE type = oWinControlGetType(_hWnd);
	if (type != oWINDOW_CONTROL_COMBOBOX && type != oWINDOW_CONTROL_COMBOTEXTBOX)
		return oErrorSetLast(oERROR_INVALID_PARAMETER);

	if (CB_ERR == ComboBox_SetCurSel(_hWnd, _Index))
		return oErrorSetLast(oERROR_NOT_FOUND, "Index %d was not found in list control", _Index);
	return true;
}

bool oWinControlSelect(HWND _hWnd, int _StartIndex, int _Length)
{
	oWINDOW_CONTROL_TYPE type = oWinControlGetType(_hWnd);
	if (type != oWINDOW_CONTROL_TEXTBOX && type != oWINDOW_CONTROL_COMBOTEXTBOX && type != oWINDOW_CONTROL_LABEL_SELECTABLE)
		return oErrorSetLast(oERROR_INVALID_PARAMETER);

	Edit_SetSel(_hWnd, _StartIndex, _StartIndex+_Length);
	return true;
}
