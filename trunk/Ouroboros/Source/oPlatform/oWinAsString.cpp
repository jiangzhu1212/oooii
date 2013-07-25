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
#include <oPlatform/Windows/oWinAsString.h>
#include <oPlatform/Windows/oWinWindowing.h>
#include <oBasis/oString.h>
#include <oBasis/oError.h>
#include <Windowsx.h>
#include <DShow.h>
#include <Shellapi.h>
#include <CDErr.h>
#include <Dbt.h>
#include <SetupAPI.h>

const char* oWinAsStringHT(int _HTCode)
{
	switch (_HTCode)
	{
		case HTERROR: return "HTERROR";
		case HTTRANSPARENT: return "HTTRANSPARENT";
		case HTNOWHERE: return "HTNOWHERE";
		case HTCLIENT: return "HTCLIENT";
		case HTCAPTION: return "HTCAPTION";
		case HTSYSMENU: return "HTSYSMENU";
		//case HTGROWBOX: return "HTGROWBOX"; // same as HTSIZE
		case HTSIZE: return "HTSIZE";
		case HTMENU: return "HTMENU";
		case HTHSCROLL: return "HTHSCROLL";
		case HTVSCROLL: return "HTVSCROLL";
		case HTMINBUTTON: return "HTMINBUTTON";
		case HTMAXBUTTON: return "HTMAXBUTTON";
		case HTLEFT: return "HTLEFT";
		case HTRIGHT: return "HTRIGHT";
		case HTTOP: return "HTTOP";
		case HTTOPLEFT: return "HTTOPLEFT";
		case HTTOPRIGHT: return "HTTOPRIGHT";
		case HTBOTTOM: return "HTBOTTOM";
		case HTBOTTOMLEFT: return "HTBOTTOMLEFT";
		case HTBOTTOMRIGHT: return "HTBOTTOMRIGHT";
		case HTBORDER: return "HTBORDER";
		//case HTREDUCE: return "HTREDUCE"; // same as HTMINBUTTON
		//case HTZOOM: return "HTZOOM"; // same as HTMAXBUTTON
		//case HTSIZEFIRST: return "HTSIZEFIRST"; // same as HTLEFT
		//case HTSIZELAST: return "HTSIZELAST"; // same as HTBOTTOMRIGHT
		#if(WINVER >= 0x0400)
			case HTOBJECT: return "HTOBJECT";
			case HTCLOSE: return "HTCLOSE";
			case HTHELP: return "HTHELP";
		#endif
		default: break;
	}
	return "unrecognized HTCODE";
}

const char* oWinAsStringSC(int _SCCode)
{
#define oSC_DRAGMOVE 0xf012

	switch (_SCCode)
	{
		case SC_SIZE: return "SC_SIZE";
		case SC_MOVE: return "SC_MOVE";
		case SC_MINIMIZE: return "SC_MINIMIZE";
		case SC_MAXIMIZE: return "SC_MAXIMIZE";
		case SC_NEXTWINDOW: return "SC_NEXTWINDOW";
		case SC_PREVWINDOW: return "SC_PREVWINDOW";
		case SC_CLOSE: return "SC_CLOSE";
		case SC_VSCROLL: return "SC_VSCROLL";
		case SC_HSCROLL: return "SC_HSCROLL";
		case SC_MOUSEMENU: return "SC_MOUSEMENU";
		case SC_KEYMENU: return "SC_KEYMENU";
		case SC_ARRANGE: return "SC_ARRANGE";
		case SC_RESTORE: return "SC_RESTORE";
		case SC_TASKLIST: return "SC_TASKLIST";
		case SC_SCREENSAVE: return "SC_SCREENSAVE";
		case SC_HOTKEY: return "SC_HOTKEY";
		#if(WINVER >= 0x0400)
			case SC_DEFAULT: return "SC_DEFAULT";
			case SC_MONITORPOWER: return "SC_MONITORPOWER";
			case SC_CONTEXTHELP: return "SC_CONTEXTHELP";
			case SC_SEPARATOR: return "SC_SEPARATOR";
		#endif
		case oSC_DRAGMOVE: return "SC_DRAGMOVE";
		default: break;
	}

	return "unrecognized SCCODE";
}

const char* oWinAsStringSW(int _SWCode)
{
	switch (_SWCode)
	{
		case SW_OTHERUNZOOM: return "SW_OTHERUNZOOM";
		case SW_OTHERZOOM: return "SW_OTHERZOOM";
		case SW_PARENTCLOSING: return "SW_PARENTCLOSING";
		case SW_PARENTOPENING: return "SW_PARENTOPENING";
		default: break;
	}
	return "unrecognized SWCODE";
}

const char* oWinAsStringWM(int _uMsg)
{
	switch (_uMsg)
	{
		case WM_ACTIVATE: return "WM_ACTIVATE";
		case WM_ACTIVATEAPP: return "WM_ACTIVATEAPP";
		case WM_APPCOMMAND: return "WM_APPCOMMAND";
		case WM_CANCELMODE: return "WM_CANCELMODE";
		case WM_CAPTURECHANGED: return "WM_CAPTURECHANGED";
		case WM_CHAR: return "WM_CHAR";
		case WM_CHILDACTIVATE: return "WM_CHILDACTIVATE";
		case WM_CLOSE: return "WM_CLOSE";
		case WM_COMMAND: return "WM_COMMAND";
		case WM_COMPACTING: return "WM_COMPACTING";
		case WM_COMPAREITEM: return "WM_COMPAREITEM";
		case WM_CREATE: return "WM_CREATE";
		case WM_DEADCHAR: return "WM_DEADCHAR";
		case WM_DESTROY: return "WM_DESTROY";
		case WM_DISPLAYCHANGE: return "WM_DISPLAYCHANGE";
		case WM_DEVICECHANGE: return "WM_DEVICECHANGE";
		case WM_DRAWITEM: return "WM_DRAWITEM";
		case WM_DROPFILES: return "WM_DROPFILES";
		case WM_DWMCOLORIZATIONCOLORCHANGED: return "WM_DWMCOLORIZATIONCOLORCHANGED";
		case WM_DWMCOMPOSITIONCHANGED: return "WM_DWMCOMPOSITIONCHANGED";
		case WM_DWMNCRENDERINGCHANGED: return "WM_DWMNCRENDERINGCHANGED";
		case WM_DWMWINDOWMAXIMIZEDCHANGE: return "WM_DWMWINDOWMAXIMIZEDCHANGE";
		case WM_ENABLE: return "WM_ENABLE";
		case WM_ENTERIDLE: return "WM_ENTERIDLE";
		case WM_ENTERSIZEMOVE: return "WM_ENTERSIZEMOVE";
		case WM_ERASEBKGND: return "WM_ERASEBKGND";
		case WM_EXITSIZEMOVE: return "WM_EXITSIZEMOVE";
		case WM_GETDLGCODE: return "WM_GETDLGCODE";
		case WM_GETICON: return "WM_GETICON";
		case WM_GETMINMAXINFO: return "WM_GETMINMAXINFO";
		case WM_GETTEXT: return "WM_GETTEXT";
		case WM_GETTEXTLENGTH: return "WM_GETTEXTLENGTH";
		case WM_HELP: return "WM_HELP";
		case WM_HOTKEY: return "WM_HOTKEY";
		case WM_ICONERASEBKGND: return "WM_ICONERASEBKGND";
		case WM_INITMENUPOPUP: return "WM_INITMENUPOPUP";
		case WM_INPUTLANGCHANGE: return "WM_INPUTLANGCHANGE";
		case WM_INPUTLANGCHANGEREQUEST: return "WM_INPUTLANGCHANGEREQUEST";
		case WM_KEYDOWN: return "WM_KEYDOWN";
		case WM_KEYUP: return "WM_KEYUP";
		case WM_KILLFOCUS: return "WM_KILLFOCUS";
		case WM_MEASUREITEM: return "WM_MEASUREITEM";
		case WM_MENUSELECT: return "WM_MENUSELECT";
		case WM_MOVE: return "WM_MOVE";
		case WM_MOVING: return "WM_MOVING";
		case WM_NCMOUSEMOVE: return "WM_NCMOUSEMOVE";
		case WM_NCMOUSELEAVE: return "WM_NCMOUSELEAVE";
		case WM_NCACTIVATE: return "WM_NCACTIVATE";
		case WM_NCCALCSIZE: return "WM_NCCALCSIZE";
		case WM_NCCREATE: return "WM_NCCREATE";
		case WM_NCDESTROY: return "WM_NCDESTROY";
		case WM_NCPAINT: return "WM_NCPAINT";
		case WM_NCLBUTTONDOWN: return "WM_NCLBUTTONDOWN";
		case WM_NOTIFY: return "WM_NOTIFY";
		case WM_NOTIFYFORMAT: return "WM_NOTIFYFORMAT";
		case WM_NULL: return "WM_NULL";
		case WM_PAINT: return "WM_PAINT";
		case WM_PRINT: return "WM_PRINT";
		case WM_PRINTCLIENT: return "WM_PRINTCLIENT";
		case WM_PAINTICON: return "WM_PAINTICON";
		case WM_PARENTNOTIFY: return "WM_PARENTNOTIFY";
		case WM_QUERYDRAGICON: return "WM_QUERYDRAGICON";
		case WM_QUERYOPEN: return "WM_QUERYOPEN";
		case WM_NCHITTEST: return "WM_NCHITTEST";
		case WM_SETCURSOR: return "WM_SETCURSOR";
		case WM_SETFOCUS: return "WM_SETFOCUS";
		case WM_SETICON: return "WM_SETICON";
		case WM_SETREDRAW: return "WM_SETREDRAW";
		case WM_SETTEXT: return "WM_SETTEXT";
		case WM_SHOWWINDOW: return "WM_SHOWWINDOW";
		case WM_SIZE: return "WM_SIZE";
		case WM_SIZING: return "WM_SIZING";
		case WM_STYLECHANGED: return "WM_STYLECHANGED";
		case WM_STYLECHANGING: return "WM_STYLECHANGING";
		case WM_SYNCPAINT: return "WM_SYNCPAINT";
		case WM_SYSDEADCHAR: return "WM_SYSDEADCHAR";
		case WM_SYSKEYDOWN: return "WM_SYSKEYDOWN";
		case WM_SYSKEYUP: return "WM_SYSKEYUP";
		case WM_SYSCOMMAND: return "WM_SYSCOMMAND";
		case WM_THEMECHANGED: return "WM_THEMECHANGED";
		case WM_UNICHAR: return "WM_UNICHAR";
		case WM_USERCHANGED: return "WM_USERCHANGED";
		case WM_WINDOWPOSCHANGED: return "WM_WINDOWPOSCHANGED";
		case WM_WINDOWPOSCHANGING: return "WM_WINDOWPOSCHANGING";
		case WM_MOUSEACTIVATE: return "WM_MOUSEACTIVATE";
		case WM_MOUSEMOVE: return "WM_MOUSEMOVE";
		case WM_LBUTTONDOWN: return "WM_LBUTTONDOWN";
		case WM_LBUTTONUP: return "WM_LBUTTONUP";
		case WM_MBUTTONDOWN: return "WM_MBUTTONDOWN";
		case WM_MBUTTONUP: return "WM_MBUTTONUP";
		case WM_RBUTTONDOWN: return "WM_RBUTTONDOWN";
		case WM_RBUTTONUP: return "WM_RBUTTONUP";
		case WM_XBUTTONDOWN: return "WM_XBUTTONDOWN";
		case WM_XBUTTONUP: return "WM_XBUTTONUP";
		case WM_MOUSEWHEEL: return "WM_MOUSEWHEEL";
		case WM_MOUSEHWHEEL: return "WM_MOUSEHWHEEL";
		case WM_CONTEXTMENU: return "WM_CONTEXTMENU";
		case WM_IME_SETCONTEXT: return "WM_IME_SETCONTEXT";
		case WM_IME_NOTIFY: return "WM_IME_NOTIFY";
		case WM_INITMENU: return "WM_INITMENU";
		case WM_ENTERMENULOOP: return "WM_ENTERMENULOOP";
		case WM_EXITMENULOOP: return "WM_EXITMENULOOP";
		case WM_INITDIALOG: return "WM_INITDIALOG";
		case WM_CTLCOLORBTN: return "WM_CTLCOLORBTN";
		case WM_CTLCOLORDLG: return "WM_CTLCOLORDLG";
		case WM_CTLCOLOREDIT: return "WM_CTLCOLOREDIT";
		case WM_CTLCOLORLISTBOX: return "WM_CTLCOLORLISTBOX";
		case WM_CTLCOLORMSGBOX: return "WM_CTLCOLORMSGBOX";
		case WM_CTLCOLORSCROLLBAR: return "WM_CTLCOLORSCROLLBAR";
		case WM_CTLCOLORSTATIC: return "WM_CTLCOLORSTATIC";
		case WM_UPDATEUISTATE: return "WM_UPDATEUISTATE";
		case WM_CHANGEUISTATE: return "WM_CHANGEUISTATE";
		case WM_QUERYUISTATE: return "WM_QUERYUISTATE";
		case 0x91: return "Theme Service internal message 0x91";
		case 0x92: return "Theme Service internal message 0x92";
		case 0x93: return "Theme Service internal message 0x93";

		// Ouroboros-custom messages
		case oWM_MAINLOOP: return "oWM_MAINLOOP";
		case oWM_DISPATCH: return "oWM_DISPATCH";
		case oWM_ACTION_HOOK: return "oWM_ACTION_HOOK";
		case oWM_ACTION_UNHOOK: return "oWM_ACTION_UNHOOK";
		case oWM_ACTION_TRIGGER: return "oWM_ACTION_TRIGGER";
		case oWM_EVENT_HOOK: return "oWM_EVENT_HOOK";
		case oWM_EVENT_UNHOOK: return "oWM_EVENT_UNHOOK";
		case oWM_SETHOTKEYS: return "oWM_SETHOTKEYS";
		case oWM_GETHOTKEYS: return "oWM_GETHOTKEYS";
		case oWM_STATUS_SETPARTS: return "oWM_STATUS_SETPARTS";
		case oWM_STATUS_SETTEXT: return "oWM_STATUS_SETTEXT";
		case oWM_STATUS_GETTEXT: return "oWM_STATUS_GETTEXT";
		case oWM_STATUS_GETTEXTLENGTH: return "oWM_STATUS_GETTEXTLENGTH";
		case oWM_SKELETON: return "oWM_SKELETON";
		case oWM_INPUT_DEVICE_CHANGE: return "oWM_INPUT_DEVICE_CHANGE";

		default: break;
	}
	return "unrecognized WMCODE";
}

const char* oWinAsStringWS(int _WSFlag)
{
	switch (_WSFlag)
	{
		case WS_OVERLAPPED: return "WS_OVERLAPPED";
		case WS_POPUP: return "WS_POPUP";
		case WS_CHILD: return "WS_CHILD";
		case WS_MINIMIZE: return "WS_MINIMIZE";
		case WS_VISIBLE: return "WS_VISIBLE";
		case WS_DISABLED: return "WS_DISABLED";
		case WS_CLIPSIBLINGS: return "WS_CLIPSIBLINGS";
		case WS_CLIPCHILDREN: return "WS_CLIPCHILDREN";
		case WS_MAXIMIZE: return "WS_MAXIMIZE";
		case WS_CAPTION: return "WS_CAPTION";
		case WS_BORDER: return "WS_BORDER";
		case WS_DLGFRAME: return "WS_DLGFRAME";
		case WS_VSCROLL: return "WS_VSCROLL";
		case WS_HSCROLL: return "WS_HSCROLL";
		case WS_SYSMENU: return "WS_SYSMENU";
		case WS_THICKFRAME: return "WS_THICKFRAME";
		case WS_GROUP: return "WS_GROUP";
		case WS_TABSTOP: return "WS_TABSTOP";
		//case WS_MINIMIZEBOX: return "WS_MINIMIZEBOX"; // already used
		//case WS_MAXIMIZEBOX: return "WS_MAXIMIZEBOX"; // already used
		default: break;
	}

	return "unrecognized WS flag";
}

const char* oWinAsStringWSEX(int _WSEXFlag)
{
	switch (_WSEXFlag)
	{
		case WS_EX_DLGMODALFRAME: return "WS_EX_DLGMODALFRAME";
		case WS_EX_NOPARENTNOTIFY: return "WS_EX_NOPARENTNOTIFY";
		case WS_EX_TOPMOST: return "WS_EX_TOPMOST";
		case WS_EX_ACCEPTFILES: return "WS_EX_ACCEPTFILES";
		case WS_EX_TRANSPARENT: return "WS_EX_TRANSPARENT";
		case WS_EX_MDICHILD: return "WS_EX_MDICHILD";
		case WS_EX_TOOLWINDOW: return "WS_EX_TOOLWINDOW";
		case WS_EX_WINDOWEDGE: return "WS_EX_WINDOWEDGE";
		case WS_EX_CLIENTEDGE: return "WS_EX_CLIENTEDGE";
		case WS_EX_CONTEXTHELP: return "WS_EX_CONTEXTHELP";
		case WS_EX_RIGHT: return "WS_EX_RIGHT";
		case WS_EX_LEFT: return "WS_EX_LEFT";
		case WS_EX_RTLREADING: return "WS_EX_RTLREADING";
		//case WS_EX_LTRREADING: return "WS_EX_LTRREADING"; // dup b/c val is 0
		case WS_EX_LEFTSCROLLBAR: return "WS_EX_LEFTSCROLLBAR";
		//case WS_EX_RIGHTSCROLLBAR: return "WS_EX_RIGHTSCROLLBAR"; // dup b/c val is 0
		case WS_EX_CONTROLPARENT: return "WS_EX_CONTROLPARENT";
		case WS_EX_STATICEDGE: return "WS_EX_STATICEDGE";
		case WS_EX_APPWINDOW: return "WS_EX_APPWINDOW";
		case WS_EX_OVERLAPPEDWINDOW: return "WS_EX_OVERLAPPEDWINDOW";
		case WS_EX_PALETTEWINDOW: return "WS_EX_PALETTEWINDOW";
		case WS_EX_LAYERED: return "WS_EX_LAYERED";
		case WS_EX_NOINHERITLAYOUT: return "WS_EX_NOINHERITLAYOUT";
		case WS_EX_LAYOUTRTL: return "WS_EX_LAYOUTRTL";
		case WS_EX_COMPOSITED: return "WS_EX_COMPOSITED";
		case WS_EX_NOACTIVATE: return "WS_EX_NOACTIVATE";
		default: break;
	}
	
	return "unrecognized WS_EX flag";
}

const char* oWinAsStringWA(int _WACode)
{
	switch (_WACode)
	{
		case WA_ACTIVE: return "WA_ACTIVE";
		case WA_CLICKACTIVE: return "WA_CLICKACTIVE";
		case WA_INACTIVE: return "WA_INACTIVE";
		default: break;
	}

	return "unrecognized WACode";
}

const char* oWinAsStringBST(int _BSTCode)
{
	switch (_BSTCode)
	{
		case BST_CHECKED: return "BST_CHECKED";
		case BST_DROPDOWNPUSHED: return "BST_DROPDOWNPUSHED";
		case BST_FOCUS: return "BST_FOCUS";
		case BST_HOT: return "BST_HOT";
		case BST_INDETERMINATE: return "BST_INDETERMINATE";
		case BST_PUSHED: return "BST_PUSHED";
		case BST_UNCHECKED: return "BST_UNCHECKED";
		default: break;
	}

	return "unrecognized BSTCode";
}

const char* oWinAsStringNM(int _NMCode)
{
	switch (_NMCode)
	{
		case NM_CHAR: return "NM_CHAR";
		case NM_CUSTOMDRAW: return "NM_CUSTOMDRAW";
		case NM_CUSTOMTEXT: return "NM_CUSTOMTEXT";
		case NM_FONTCHANGED: return "NM_FONTCHANGED";
		case NM_GETCUSTOMSPLITRECT: return "NM_GETCUSTOMSPLITRECT";
		case NM_HOVER: return "NM_HOVER";
		case NM_KEYDOWN: return "NM_KEYDOWN";
		case NM_KILLFOCUS: return "NM_KILLFOCUS";
		case NM_LDOWN: return "NM_LDOWN";
		case NM_NCHITTEST: return "NM_NCHITTEST";
		case NM_OUTOFMEMORY: return "NM_OUTOFMEMORY";
		case NM_RDOWN: return "NM_RDOWN";
		case NM_RELEASEDCAPTURE: return "NM_RELEASEDCAPTURE";
		case NM_RETURN: return "NM_RETURN";
		case NM_SETCURSOR: return "NM_SETCURSOR";
		case NM_SETFOCUS: return "NM_SETFOCUS";
		case NM_THEMECHANGED: return "NM_THEMECHANGED";
		case NM_TOOLTIPSCREATED: return "NM_TOOLTIPSCREATED";
		//case NM_TVSTATEIMAGECHANGING: return "NM_TVSTATEIMAGECHANGING"; // same as NM_CUSTOMTEXT
		case NM_CLICK: return "NM_CLICK";
		case NM_DBLCLK: return "NM_DBLCLK";
		case NM_RCLICK: return "NM_RCLICK";
		case NM_RDBLCLK: return "NM_RDBLCLK";
		default: break;
	}

	return "unrecognized NMCode";
}

const char* oWinAsStringSWP(int _SWPCode)
{
	switch (_SWPCode)
	{
		case SWP_NOSIZE: return "SWP_NOSIZE";
		case SWP_NOMOVE: return "SWP_NOMOVE";
		case SWP_NOZORDER: return "SWP_NOZORDER";
		case SWP_NOREDRAW: return "SWP_NOREDRAW";
		case SWP_NOACTIVATE: return "SWP_NOACTIVATE";
		case SWP_FRAMECHANGED: return "SWP_FRAMECHANGED";
		case SWP_SHOWWINDOW: return "SWP_SHOWWINDOW";
		case SWP_HIDEWINDOW: return "SWP_HIDEWINDOW";
		case SWP_NOCOPYBITS: return "SWP_NOCOPYBITS";
		case SWP_NOOWNERZORDER: return "SWP_NOOWNERZORDER";
		case SWP_NOSENDCHANGING: return "SWP_NOSENDCHANGING";
		//case SWP_DRAWFRAME: return "SWP_DRAWFRAME"; // same as DWP_FRAMECHANGED
		//case SWP_NOREPOSITION: return "SWP_NOREPOSITION"; // same as SWP_NOOWNERZORDER
		#if (WINVER >= 0x0400)
			case SWP_DEFERERASE: return "SWP_DEFERERASE";
			case SWP_ASYNCWINDOWPOS: return "SWP_ASYNCWINDOWPOS";
		#endif
		default: break;
	}

	return "unrecognized SWPCode";
}

const char* oWinAsStringGWL(int _GWLCode)
{
	switch (_GWLCode)
	{
		#ifndef _WIN64
			case GWL_WNDPROC: return "GWL_WNDPROC";
			case GWL_HINSTANCE: return "GWL_HINSTANCE";
			case GWL_HWNDPARENT: return "GWL_HWNDPARENT";
			case GWL_USERDATA: return "GWL_USERDATA";
		#endif
		case GWL_STYLE: return "GWL_STYLE";
		case GWL_EXSTYLE: return "GWL_EXSTYLE";
		case GWL_ID: return "GWL_ID";
		default: break;
	}

	return "unrecognized GWLCode";
}

const char* oWinAsStringGWLP(int _GWLPCode)
{
	switch (_GWLPCode)
	{
		case GWLP_WNDPROC: return "GWLP_WNDPROC";
		case GWLP_HINSTANCE: return "GWLP_HINSTANCE";
		case GWLP_HWNDPARENT: return "GWLP_HWNDPARENT";
		case GWLP_USERDATA: return "GWLP_USERDATA";
		case GWLP_ID: return "GWLP_ID";
		default: break;
	}

	return "unrecognized GWLPCode";
}

const char* oWinAsStringTCN(int _TCNCode)
{
	switch (_TCNCode)
	{
		case TCN_KEYDOWN: return "TCN_KEYDOWN";
		case TCN_SELCHANGE: return "TCN_SELCHANGE";
		case TCN_SELCHANGING: return "TCN_SELCHANGING";
		case TCN_GETOBJECT: return "TCN_GETOBJECT";
		case TCN_FOCUSCHANGE: return "TCN_FOCUSCHANGE";
		default: break;
	}

	return "unrecognized TCNCode";
}

const char* oWinAsStringCDERR(int _CDERRCode)
{
	switch (_CDERRCode)
	{
		case CDERR_DIALOGFAILURE: return "CDERR_DIALOGFAILURE";
		case CDERR_GENERALCODES: return "CDERR_GENERALCODES";
		case CDERR_STRUCTSIZE: return "CDERR_STRUCTSIZE";
		case CDERR_INITIALIZATION: return "CDERR_INITIALIZATION";
		case CDERR_NOTEMPLATE: return "CDERR_NOTEMPLATE";
		case CDERR_NOHINSTANCE: return "CDERR_NOHINSTANCE";
		case CDERR_LOADSTRFAILURE: return "CDERR_LOADSTRFAILURE";
		case CDERR_FINDRESFAILURE: return "CDERR_FINDRESFAILURE";
		case CDERR_LOADRESFAILURE: return "CDERR_LOADRESFAILURE";
		case CDERR_LOCKRESFAILURE: return "CDERR_LOCKRESFAILURE";
		case CDERR_MEMALLOCFAILURE: return "CDERR_MEMALLOCFAILURE";
		case CDERR_MEMLOCKFAILURE: return "CDERR_MEMLOCKFAILURE";
		case CDERR_NOHOOK: return "CDERR_NOHOOK";
		case CDERR_REGISTERMSGFAIL: return "CDERR_REGISTERMSGFAIL";
		case PDERR_PRINTERCODES: return "PDERR_PRINTERCODES";
		case PDERR_SETUPFAILURE: return "PDERR_SETUPFAILURE";
		case PDERR_PARSEFAILURE: return "PDERR_PARSEFAILURE";
		case PDERR_RETDEFFAILURE: return "PDERR_RETDEFFAILURE";
		case PDERR_LOADDRVFAILURE: return "PDERR_LOADDRVFAILURE";
		case PDERR_GETDEVMODEFAIL: return "PDERR_GETDEVMODEFAIL";
		case PDERR_INITFAILURE: return "PDERR_INITFAILURE";
		case PDERR_NODEVICES: return "PDERR_NODEVICES";
		case PDERR_NODEFAULTPRN: return "PDERR_NODEFAULTPRN";
		case PDERR_DNDMMISMATCH: return "PDERR_DNDMMISMATCH";
		case PDERR_CREATEICFAILURE: return "PDERR_CREATEICFAILURE";
		case PDERR_PRINTERNOTFOUND: return "PDERR_PRINTERNOTFOUND";
		case PDERR_DEFAULTDIFFERENT: return "PDERR_DEFAULTDIFFERENT";
		case CFERR_CHOOSEFONTCODES: return "CFERR_CHOOSEFONTCODES";
		case CFERR_NOFONTS: return "CFERR_NOFONTS";
		case CFERR_MAXLESSTHANMIN: return "CFERR_MAXLESSTHANMIN";
		case FNERR_FILENAMECODES: return "FNERR_FILENAMECODES";
		case FNERR_SUBCLASSFAILURE: return "FNERR_SUBCLASSFAILURE";
		case FNERR_INVALIDFILENAME: return "FNERR_INVALIDFILENAME";
		case FNERR_BUFFERTOOSMALL: return "FNERR_BUFFERTOOSMALL";
		case FRERR_FINDREPLACECODES: return "FRERR_FINDREPLACECODES";
		case FRERR_BUFFERLENGTHZERO: return "FRERR_BUFFERLENGTHZERO";
		case CCERR_CHOOSECOLORCODES: return "CCERR_CHOOSECOLORCODES";
		default: break;
	}

	return "unrecognized CDERR";
}

const char* oWinAsStringExceptionCode(int _ExceptionCode)
{
	switch (_ExceptionCode)
	{
		case EXCEPTION_ACCESS_VIOLATION: return "EXCEPTION_ACCESS_VIOLATION";
		case EXCEPTION_DATATYPE_MISALIGNMENT: return "EXCEPTION_DATATYPE_MISALIGNMENT";
		case EXCEPTION_BREAKPOINT: return "EXCEPTION_BREAKPOINT";
		case EXCEPTION_SINGLE_STEP: return "EXCEPTION_SINGLE_STEP";
		case EXCEPTION_ARRAY_BOUNDS_EXCEEDED: return "EXCEPTION_ARRAY_BOUNDS_EXCEEDED";
		case EXCEPTION_FLT_DENORMAL_OPERAND: return "EXCEPTION_FLT_DENORMAL_OPERAND";
		case EXCEPTION_FLT_DIVIDE_BY_ZERO: return "EXCEPTION_FLT_DIVIDE_BY_ZERO";
		case EXCEPTION_FLT_INEXACT_RESULT: return "EXCEPTION_FLT_INEXACT_RESULT";
		case EXCEPTION_FLT_INVALID_OPERATION: return "EXCEPTION_FLT_INVALID_OPERATION";
		case EXCEPTION_FLT_OVERFLOW: return "EXCEPTION_FLT_OVERFLOW";
		case EXCEPTION_FLT_STACK_CHECK: return "EXCEPTION_FLT_STACK_CHECK";
		case EXCEPTION_FLT_UNDERFLOW: return "EXCEPTION_FLT_UNDERFLOW";
		case EXCEPTION_INT_DIVIDE_BY_ZERO: return "EXCEPTION_INT_DIVIDE_BY_ZERO";
		case EXCEPTION_INT_OVERFLOW: return "EXCEPTION_INT_OVERFLOW";
		case EXCEPTION_PRIV_INSTRUCTION: return "EXCEPTION_PRIV_INSTRUCTION";
		case EXCEPTION_IN_PAGE_ERROR: return "EXCEPTION_IN_PAGE_ERROR";
		case EXCEPTION_ILLEGAL_INSTRUCTION: return "EXCEPTION_ILLEGAL_INSTRUCTION";
		case EXCEPTION_NONCONTINUABLE_EXCEPTION: return "EXCEPTION_NONCONTINUABLE_EXCEPTION";
		case EXCEPTION_STACK_OVERFLOW: return "EXCEPTION_STACK_OVERFLOW";
		case EXCEPTION_INVALID_DISPOSITION: return "EXCEPTION_INVALID_DISPOSITION";
		case EXCEPTION_GUARD_PAGE: return "EXCEPTION_GUARD_PAGE";
		case EXCEPTION_INVALID_HANDLE: return "EXCEPTION_INVALID_HANDLE";
		//case EXCEPTION_POSSIBLE_DEADLOCK: return "EXCEPTION_POSSIBLE_DEADLOCK"; // @oooii-tony: STATUS_POSSIBLE_DEADLOCK is not defined, which is what this evaluates to
		default: break;
	}

	return "unrecognized Exception Code";
}

const char* oWinAsStringDBT(int _DBTEvent)
{
	switch (_DBTEvent)
	{
		case DBT_CONFIGCHANGECANCELED: return "DBT_CONFIGCHANGECANCELED";
		case DBT_CONFIGCHANGED: return "DBT_CONFIGCHANGED";
		case DBT_CUSTOMEVENT: return "DBT_CUSTOMEVENT";
		case DBT_DEVICEARRIVAL: return "DBT_DEVICEARRIVAL";
		case DBT_DEVICEQUERYREMOVE: return "DBT_DEVICEQUERYREMOVE";
		case DBT_DEVICEQUERYREMOVEFAILED: return "DBT_DEVICEQUERYREMOVEFAILED";
		case DBT_DEVICEREMOVECOMPLETE: return "DBT_DEVICEREMOVECOMPLETE";
		case DBT_DEVICEREMOVEPENDING: return "DBT_DEVICEREMOVEPENDING";
		case DBT_DEVICETYPESPECIFIC: return "DBT_DEVICETYPESPECIFIC";
		case DBT_DEVNODES_CHANGED: return "DBT_DEVNODES_CHANGED";
		case DBT_QUERYCHANGECONFIG: return "DBT_QUERYCHANGECONFIG";
		case DBT_USERDEFINED: return "DBT_USERDEFINED";
		default: break;
	}

	return "unrecognized DBT event";
}

const char* oWinAsStringDBTDT(int _DBTDevType)
{
	switch (_DBTDevType)
	{
		case DBT_DEVTYP_DEVICEINTERFACE: return "DBT_DEVTYP_DEVICEINTERFACE";
		case DBT_DEVTYP_HANDLE: return "DBT_DEVTYP_HANDLE";
		case DBT_DEVTYP_OEM: return "DBT_DEVTYP_OEM";
		case DBT_DEVTYP_PORT: return "DBT_DEVTYP_PORT";
		case DBT_DEVTYP_VOLUME: return "DBT_DEVTYP_VOLUME";
		default: break;
	}

	return "unrecognized DBT devtype event";
}

const char* oWinAsStringSPDRP(int _SPDRPValue)
{
	switch (_SPDRPValue)
	{
		case SPDRP_DEVICEDESC: return "SPDRP_DEVICEDESC";
		case SPDRP_HARDWAREID: return "SPDRP_HARDWAREID";
		case SPDRP_COMPATIBLEIDS: return "SPDRP_COMPATIBLEIDS";
		case SPDRP_UNUSED0: return "SPDRP_UNUSED0";
		case SPDRP_SERVICE: return "SPDRP_SERVICE";
		case SPDRP_UNUSED1: return "SPDRP_UNUSED1";
		case SPDRP_UNUSED2: return "SPDRP_UNUSED2";
		case SPDRP_CLASS: return "SPDRP_CLASS";
		case SPDRP_CLASSGUID: return "SPDRP_CLASSGUID";
		case SPDRP_DRIVER: return "SPDRP_DRIVER";
		case SPDRP_CONFIGFLAGS: return "SPDRP_CONFIGFLAGS";
		case SPDRP_MFG: return "SPDRP_MFG";
		case SPDRP_FRIENDLYNAME: return "SPDRP_FRIENDLYNAME";
		case SPDRP_LOCATION_INFORMATION: return "SPDRP_LOCATION_INFORMATION";
		case SPDRP_PHYSICAL_DEVICE_OBJECT_NAME: return "SPDRP_PHYSICAL_DEVICE_OBJECT_NAME";
		case SPDRP_CAPABILITIES: return "SPDRP_CAPABILITIES";
		case SPDRP_UI_NUMBER: return "SPDRP_UI_NUMBER";
		case SPDRP_UPPERFILTERS: return "SPDRP_UPPERFILTERS";
		case SPDRP_LOWERFILTERS: return "SPDRP_LOWERFILTERS";
		case SPDRP_BUSTYPEGUID: return "SPDRP_BUSTYPEGUID";
		case SPDRP_LEGACYBUSTYPE: return "SPDRP_LEGACYBUSTYPE";
		case SPDRP_BUSNUMBER: return "SPDRP_BUSNUMBER";
		case SPDRP_ENUMERATOR_NAME: return "SPDRP_ENUMERATOR_NAME";
		case SPDRP_SECURITY: return "SPDRP_SECURITY";
		case SPDRP_SECURITY_SDS: return "SPDRP_SECURITY_SDS";
		case SPDRP_DEVTYPE: return "SPDRP_DEVTYPE";
		case SPDRP_EXCLUSIVE: return "SPDRP_EXCLUSIVE";
		case SPDRP_CHARACTERISTICS: return "SPDRP_CHARACTERISTICS";
		case SPDRP_ADDRESS: return "SPDRP_ADDRESS";
		case SPDRP_UI_NUMBER_DESC_FORMAT: return "SPDRP_UI_NUMBER_DESC_FORMAT";
		case SPDRP_DEVICE_POWER_DATA: return "SPDRP_DEVICE_POWER_DATA";
		case SPDRP_REMOVAL_POLICY: return "SPDRP_REMOVAL_POLICY";
		case SPDRP_REMOVAL_POLICY_HW_DEFAULT: return "SPDRP_REMOVAL_POLICY_HW_DEFAULT";
		case SPDRP_REMOVAL_POLICY_OVERRIDE: return "SPDRP_REMOVAL_POLICY_OVERRIDE";
		case SPDRP_INSTALL_STATE: return "SPDRP_INSTALL_STATE";
		case SPDRP_LOCATION_PATHS: return "SPDRP_LOCATION_PATHS";
		case SPDRP_BASE_CONTAINERID: return "SPDRP_BASE_CONTAINERID";
		default: break;
	}
	return "unrecognized SPDRP value";
}

char* oWinParseStyleFlags(char* _StrDestination, size_t _SizeofStrDestination, UINT _WSFlags)
{
	return oStd::strbitmask(_StrDestination, _SizeofStrDestination, *(int*)&_WSFlags, oWinAsStringWS(WS_OVERLAPPED), oWinAsStringWS);
}

char* oWinParseStyleExFlags(char* _StrDestination, size_t _SizeofStrDestination, UINT _WSEXFlags)
{
	return oStd::strbitmask(_StrDestination, _SizeofStrDestination, *(int*)&_WSEXFlags, oWinAsStringWSEX(0), oWinAsStringWSEX);
}

char* oWinParseSWPFlags(char* _StrDestination, size_t _SizeofStrDestination, UINT _SWPFlags)
{
	return oStd::strbitmask(_StrDestination, _SizeofStrDestination, *(int*)&_SWPFlags & 0x07ff, "0", oWinAsStringSWP);
}

char* oWinParseWMMessage(char* _StrDestination, size_t _SizeofStrDestination, oWINKEY_CONTROL_STATE* _pState, HWND _hWnd, UINT _uMsg, WPARAM _wParam, LPARAM _lParam)
{
	// http://www.autoitscript.com/autoit3/docs/appendix/WinMsgCodes.htm

	#define KEYSTR oStd::as_string(oWinKeyToKey((DWORD)oWinKeyTranslate(_wParam, _pState)))

	switch (_uMsg)
	{ 
		case WM_ACTIVATE: oPrintf(_StrDestination, _SizeofStrDestination, "HWND 0x%x WM_ACTIVATE %s, other HWND 0x%x %sactivated", _hWnd, oWinAsStringWA((UINT)_wParam), _lParam, _wParam == WA_INACTIVE ? "" : "de"); break;
		case WM_ACTIVATEAPP: oPrintf(_StrDestination, _SizeofStrDestination, "HWND 0x%x WM_ACTIVATEAPP %sactivated, other thread %d %sactivated", _hWnd, _wParam ? "" : "de", _lParam, _wParam ? "de" : ""); break;
		case WM_NCACTIVATE: oPrintf(_StrDestination, _SizeofStrDestination, "HWND 0x%x WM_NCACTIVATE titlebar/icon needs to be changed: %s lParam=%x", _hWnd, _wParam ? "true" : "false", _lParam); break;
		case WM_SETTEXT: oPrintf(_StrDestination, _SizeofStrDestination, "HWND 0x%x WM_SETTEXT: lParam=%s", _hWnd, _lParam); break;
		case WM_WINDOWPOSCHANGING: { char tmp[1024]; WINDOWPOS& wp = *(WINDOWPOS*)_lParam; oPrintf(_StrDestination, _SizeofStrDestination, "HWND 0x%x WM_WINDOWPOSCHANGING hwndInsertAfter=%x xy=%d,%d wh=%dx%d flags=%s", _hWnd, wp.hwndInsertAfter, wp.x, wp.y, wp.cx, wp.cy, oWinParseSWPFlags(tmp, wp.flags)); break; }
		case WM_WINDOWPOSCHANGED: { char tmp[1024]; WINDOWPOS& wp = *(WINDOWPOS*)_lParam; oPrintf(_StrDestination, _SizeofStrDestination, "HWND 0x%x WM_WINDOWPOSCHANGED hwndInsertAfter=%x xy=%d,%d wh=%dx%d flags=%s", _hWnd, wp.hwndInsertAfter, wp.x, wp.y, wp.cx, wp.cy, oWinParseSWPFlags(tmp, wp.flags)); break; }
		case WM_STYLECHANGING: { char tmp[1024]; char tmp2[1024]; oPrintf(_StrDestination, _SizeofStrDestination, "HWND 0x%x WM_STYLECHANGING %s %s -> %s", _hWnd, _wParam == GWL_EXSTYLE ? "GWL_EXSTYLE" : "GWL_STYLE", oWinParseSWPFlags(tmp, ((STYLESTRUCT*)_lParam)->styleOld), oWinParseSWPFlags(tmp2, ((STYLESTRUCT*)_lParam)->styleNew)); break; }
		case WM_STYLECHANGED: { char tmp[1024]; char tmp2[1024]; oPrintf(_StrDestination, _SizeofStrDestination, "HWND 0x%x WM_STYLECHANGED %s", _hWnd, _wParam == GWL_EXSTYLE ? "GWL_EXSTYLE" : "GWL_STYLE", oWinParseSWPFlags(tmp, ((STYLESTRUCT*)_lParam)->styleOld), oWinParseSWPFlags(tmp2, ((STYLESTRUCT*)_lParam)->styleNew)); break; }
		case WM_DISPLAYCHANGE: oPrintf(_StrDestination, _SizeofStrDestination, "HWND 0x%x WM_DISPLAYCHANGE %dx%dx%d", _hWnd, static_cast<int>(GET_X_LPARAM(_lParam)), static_cast<int>(GET_Y_LPARAM(_lParam)), _wParam); break;
		case WM_DWMCOLORIZATIONCOLORCHANGED: oPrintf(_StrDestination, _SizeofStrDestination, "HWND 0x%x WM_DWMCOLORIZATIONCOLORCHANGED", _hWnd); break;
		case WM_DWMCOMPOSITIONCHANGED: oPrintf(_StrDestination, _SizeofStrDestination, "HWND 0x%x WM_DWMCOMPOSITIONCHANGED", _hWnd); break;
		case WM_DWMNCRENDERINGCHANGED: oPrintf(_StrDestination, _SizeofStrDestination, "HWND 0x%x WM_DWMNCRENDERINGCHANGED Desktop Window Manager (DWM) %s", _hWnd, (BOOL)_wParam ? "enabled" : "disabled"); break;
		case WM_DWMWINDOWMAXIMIZEDCHANGE: oPrintf(_StrDestination, _SizeofStrDestination, "HWND 0x%x WM_DWMWINDOWMAXIMIZEDCHANGE", _hWnd); break;
		case WM_MOVE: oPrintf(_StrDestination, _SizeofStrDestination, "HWND 0x%x WM_MOVE %d,%d", _hWnd, GET_X_LPARAM(_lParam), GET_Y_LPARAM(_lParam)); break;
		case WM_SETCURSOR: oPrintf(_StrDestination, _SizeofStrDestination, "HWND 0x%x WM_SETCURSOR %s hit=%s, id=%s", _hWnd, (HWND)_wParam == _hWnd ? "In Window" : "Out of Window", oWinAsStringHT(GET_X_LPARAM(_lParam)), oWinAsStringWM(GET_Y_LPARAM(_lParam))); break;
		case WM_SHOWWINDOW: oPrintf(_StrDestination, _SizeofStrDestination, "HWND 0x%x WM_SHOWWINDOW: wParam=%s lParam=%s", _hWnd, _wParam ? "shown" : "hidden", oWinAsStringSW((UINT)_lParam));  break;
		case WM_SIZE: oPrintf(_StrDestination, _SizeofStrDestination, "HWND 0x%x WM_SIZE %dx%d", _hWnd, GET_X_LPARAM(_lParam), GET_Y_LPARAM(_lParam)); break;
		case WM_SYSCOMMAND: oPrintf(_StrDestination, _SizeofStrDestination, "HWND 0x%x WM_SYSCOMMAND: wParam=%s screenpos=%d,%d", _hWnd, oWinAsStringSC((UINT)(_wParam&0xfff0)), GET_X_LPARAM(_lParam), GET_Y_LPARAM(_lParam)); break;
		case WM_SYSKEYDOWN: oPrintf(_StrDestination, _SizeofStrDestination, "HWND 0x%x WM_SYSKEYDOWN: wParam=%s lParam=%0x", _hWnd, KEYSTR, _lParam); break;
		case WM_SYSKEYUP: oPrintf(_StrDestination, _SizeofStrDestination, "HWND 0x%x WM_SYSKEYUP: wParam=%s lParam=%0x", _hWnd, KEYSTR, _lParam); break;
		case WM_KEYDOWN: oPrintf(_StrDestination, _SizeofStrDestination, "HWND 0x%x WM_KEYDOWN: wParam=%s lParam=%0x", _hWnd, KEYSTR, _lParam); break;
		case WM_KEYUP: oPrintf(_StrDestination, _SizeofStrDestination, "HWND 0x%x WM_KEYUP: wParam=%s lParam=%0x", _hWnd, KEYSTR, _lParam); break;
		case WM_MOUSEMOVE: oPrintf(_StrDestination, _SizeofStrDestination, "HWND 0x%x WM_MOUSEMOVE %d,%d", _hWnd, GET_X_LPARAM(_lParam), GET_Y_LPARAM(_lParam)); break;
		case WM_LBUTTONDOWN: oPrintf(_StrDestination, _SizeofStrDestination, "HWND 0x%x WM_LBUTTONDOWN %d,%d", _hWnd, GET_X_LPARAM(_lParam), GET_Y_LPARAM(_lParam)); break;
		case WM_LBUTTONUP: oPrintf(_StrDestination, _SizeofStrDestination, "HWND 0x%x WM_LBUTTONUP %d,%d", _hWnd, GET_X_LPARAM(_lParam), GET_Y_LPARAM(_lParam)); break;
		case WM_MBUTTONDOWN: oPrintf(_StrDestination, _SizeofStrDestination, "HWND 0x%x WM_MBUTTONDOWN %d,%d", _hWnd, GET_X_LPARAM(_lParam), GET_Y_LPARAM(_lParam)); break;
		case WM_MBUTTONUP: oPrintf(_StrDestination, _SizeofStrDestination, "HWND 0x%x WM_MBUTTONUP %d,%d", _hWnd, GET_X_LPARAM(_lParam), GET_Y_LPARAM(_lParam)); break;
		case WM_RBUTTONDOWN: oPrintf(_StrDestination, _SizeofStrDestination, "HWND 0x%x WM_RBUTTONDOWN %d,%d", _hWnd, GET_X_LPARAM(_lParam), GET_Y_LPARAM(_lParam)); break;
		case WM_RBUTTONUP: oPrintf(_StrDestination, _SizeofStrDestination, "HWND 0x%x WM_RBUTTONUP %d,%d", _hWnd, GET_X_LPARAM(_lParam), GET_Y_LPARAM(_lParam)); break;
		case WM_XBUTTONDOWN: oPrintf(_StrDestination, _SizeofStrDestination, "HWND 0x%x WM_XBUTTONDOWN(%d) %d,%d", _hWnd, GET_XBUTTON_WPARAM(_wParam), GET_X_LPARAM(_lParam), GET_Y_LPARAM(_lParam)); break;
		case WM_XBUTTONUP: oPrintf(_StrDestination, _SizeofStrDestination, "HWND 0x%x WM_XBUTTONUP(%d) %d,%d", _hWnd, GET_XBUTTON_WPARAM(_wParam), GET_X_LPARAM(_lParam), GET_Y_LPARAM(_lParam)); break;
		case WM_MOUSEWHEEL: oPrintf(_StrDestination, _SizeofStrDestination, "HWND 0x%x WM_MOUSEWHEEL %d", _hWnd, GET_WHEEL_DELTA_WPARAM(_wParam)); break;
		case WM_MOUSEHWHEEL: oPrintf(_StrDestination, _SizeofStrDestination, "HWND 0x%x WM_MOUSEHWHEEL %d", _hWnd, GET_WHEEL_DELTA_WPARAM(_wParam)); break;
		case WM_IME_SETCONTEXT: oPrintf(_StrDestination, _SizeofStrDestination, "HWND 0x%x WM_IME_SETCONTEXT window %sactive... display flags 0x%x", _hWnd, _wParam ? "" : "in", _lParam); break;
		case WM_IME_NOTIFY: oPrintf(_StrDestination, _SizeofStrDestination, "HWND 0x%x WM_IME_NOTIFY wParam == 0x%x lParam = 0x%x", _hWnd, _wParam, _lParam); break;
		case WM_CTLCOLORBTN: oPrintf(_StrDestination, _SizeofStrDestination, "HWND 0x%x WM_CTLCOLORBTN HDC 0x%x DlgItemhwnd = %p", _hWnd, _wParam, _lParam); break;
		case WM_CTLCOLORDLG: oPrintf(_StrDestination, _SizeofStrDestination, "HWND 0x%x WM_CTLCOLORDLG HDC 0x%x Dlghwnd = %p", _hWnd, _wParam, _lParam); break;
		case WM_CTLCOLOREDIT: oPrintf(_StrDestination, _SizeofStrDestination, "HWND 0x%x WM_CTLCOLOREDIT HDC 0x%x DlgItemhwnd = %p", _hWnd, _wParam, _lParam); break;
		case WM_CTLCOLORLISTBOX: oPrintf(_StrDestination, _SizeofStrDestination, "HWND 0x%x WM_CTLCOLORLISTBOX HDC 0x%x DlgItemhwnd = %p", _hWnd, _wParam, _lParam); break;
		case WM_CTLCOLORMSGBOX: oPrintf(_StrDestination, _SizeofStrDestination, "HWND 0x%x WM_CTLCOLORMSGBOX HDC 0x%x DlgItemhwnd = %p", _hWnd, _wParam, _lParam); break;
		case WM_CTLCOLORSCROLLBAR: oPrintf(_StrDestination, _SizeofStrDestination, "HWND 0x%x WM_CTLCOLORSCROLLBAR HDC 0x%x DlgItemhwnd = %p", _hWnd, _wParam, _lParam); break;
		case WM_CTLCOLORSTATIC: oPrintf(_StrDestination, _SizeofStrDestination, "HWND 0x%x WM_CTLCOLORSTATIC HDC 0x%x DlgItemhwnd = %p", _hWnd, _wParam, _lParam); break;
		case WM_NOTIFY: { const NMHDR& h = *(NMHDR*)_lParam; oPrintf(_StrDestination, _SizeofStrDestination, "HWND 0x%x WM_NOTIFY WPARAM=%u from hwndFrom=0x%x idFrom=%d notification code=%s", _hWnd, _wParam, h.hwndFrom, h.idFrom, oWinAsStringNM(h.code)); break; }
		case WM_DROPFILES:
		{
			oStd::path_string p;
			UINT nFiles = DragQueryFile((HDROP)_wParam, ~0u, p, oUInt(p.capacity()));
			DragQueryFile((HDROP)_wParam, 0, p, oUInt(p.capacity()));
			oPrintf(_StrDestination, _SizeofStrDestination, "HWND 0x%x WM_DROPFILES hDrop=0x%x %u files starting with \"%s\"", _hWnd, _wParam, nFiles, p.c_str());
			break;
		}
		case WM_INPUT_DEVICE_CHANGE:
		{
			const char* type = _wParam == GIDC_ARRIVAL ? "arrival" : "removal";

			oStd::mstring Name;
			UINT Size = oUInt(Name.capacity() * sizeof(oStd::mstring::char_type));
			GetRawInputDeviceInfo((HANDLE)_lParam, RIDI_DEVICENAME, Name.c_str(), &Size);

			RID_DEVICE_INFO RIDDI;
			Size = sizeof(RIDDI);
			GetRawInputDeviceInfo((HANDLE)_lParam, RIDI_DEVICEINFO, &RIDDI, &Size);
			oGUI_INPUT_DEVICE_TYPE InpType = oGUI_INPUT_DEVICE_UNKNOWN;
			switch (RIDDI.dwType)
			{
				case RIM_TYPEKEYBOARD:
					InpType = oGUI_INPUT_DEVICE_KEYBOARD;
					break;
				case RIM_TYPEMOUSE:
					InpType = oGUI_INPUT_DEVICE_MOUSE;
					break;
				default:
				case RIM_TYPEHID:
					InpType = oGUI_INPUT_DEVICE_UNKNOWN;
					break;
			}

			oStd::sstring StrType;
			oPrintf(_StrDestination, _SizeofStrDestination, "HWND 0x%x WM_INPUT_DEVICE_CHANGE %s type=%s devname=%s", _hWnd, type, oStd::to_string(StrType, InpType), Name.c_str());
			break;
		}

		case WM_DEVICECHANGE: 
		{ 
			DEV_BROADCAST_HDR* pDBHdr = (DEV_BROADCAST_HDR*)_lParam;
			const char* devtype = pDBHdr ? oWinAsStringDBTDT(pDBHdr->dbch_devicetype) : "(null)";

			const oStd::guid* pGUID = &oStd::null_guid;
			const char* name = "(null)";

			if (pDBHdr)
			{
				switch (pDBHdr->dbch_devicetype)
				{
					case DBT_DEVTYP_DEVICEINTERFACE:
					{
						DEV_BROADCAST_DEVICEINTERFACE_A* pDBDI = (DEV_BROADCAST_DEVICEINTERFACE_A*)_lParam;
						pGUID = (const oStd::guid*)&pDBDI->dbcc_classguid;
						name = pDBDI->dbcc_name;
						break;
					}
					case DBT_DEVTYP_HANDLE:
					{
						DEV_BROADCAST_HANDLE* pDBH = (DEV_BROADCAST_HANDLE*)_lParam;
						pGUID = (const oStd::guid*)&pDBH->dbch_eventguid;
						break;
					}
					case DBT_DEVTYP_OEM:
					{
						DEV_BROADCAST_OEM* pDBOEM = (DEV_BROADCAST_OEM*)_lParam;
						name = "OEM";
						break;
					}
					case DBT_DEVTYP_PORT:
					{
						DEV_BROADCAST_PORT* pDBPort = (DEV_BROADCAST_PORT*)_lParam;
						name = pDBPort->dbcp_name;
						break;
					}
					case DBT_DEVTYP_VOLUME:
					{
						DEV_BROADCAST_VOLUME* pDBVol = (DEV_BROADCAST_VOLUME*)_lParam;
						switch (pDBVol->dbcv_flags)
						{
							case DBTF_MEDIA: name = "physical media device"; break;
							case DBTF_NET: name = "network device"; break;
							case DBTF_MEDIA|DBTF_NET: name = "network media device"; break;
							default: name = "physical device";
						}
						break;
					}
					default: break;
				}
			}

			oStd::sstring StrGUID;
			oPrintf(_StrDestination, _SizeofStrDestination, "HWND 0x%x WM_DEVICECHANGE %s devtype=%s GUID=%s name=%s", _hWnd, oWinAsStringDBT(oInt(_wParam)), devtype, oStd::to_string(StrGUID, *pGUID), name);
			break;
		}
		default:
		{
			const char* WMStr = oWinAsStringWM(_uMsg);
			oPrintf(_StrDestination, _SizeofStrDestination, "HWND 0x%x %s", _hWnd, WMStr);
			if (*WMStr == 'u') // unrecognized
				oPrintf(_StrDestination, _SizeofStrDestination, "HWND 0x%x Unrecognized uMsg=0x%x (%u)", _hWnd, _uMsg, _uMsg); break;
			break;
		}
	}

	return _StrDestination;
}

const char* oWinAsStringHR(HRESULT _hResult)
{
	switch (_hResult)
	{
		// From WinError.h
		//case ERROR_INVALID_FUNCTION: return "ERROR_INVALID_FUNCTION"; // same value as S_FALSE
		case ERROR_FILE_NOT_FOUND: return "ERROR_FILE_NOT_FOUND";
		case ERROR_PATH_NOT_FOUND: return "ERROR_PATH_NOT_FOUND";
		case ERROR_TOO_MANY_OPEN_FILES: return "ERROR_TOO_MANY_OPEN_FILES";
		case ERROR_ACCESS_DENIED: return "ERROR_ACCESS_DENIED";
		case ERROR_INVALID_HANDLE: return "ERROR_INVALID_HANDLE";
		case ERROR_ARENA_TRASHED: return "ERROR_ARENA_TRASHED";
		case ERROR_NOT_ENOUGH_MEMORY: return "ERROR_NOT_ENOUGH_MEMORY";
		case ERROR_INVALID_BLOCK: return "ERROR_INVALID_BLOCK";
		case ERROR_BAD_ENVIRONMENT: return "ERROR_BAD_ENVIRONMENT";
		case ERROR_BAD_FORMAT: return "ERROR_BAD_FORMAT";
		case ERROR_INVALID_ACCESS: return "ERROR_INVALID_ACCESS";
		case ERROR_INVALID_DATA: return "ERROR_INVALID_DATA";
		case ERROR_OUTOFMEMORY: return "ERROR_OUTOFMEMORY";
		case ERROR_INVALID_DRIVE: return "ERROR_INVALID_DRIVE";
		case ERROR_CURRENT_DIRECTORY: return "ERROR_CURRENT_DIRECTORY";
		case ERROR_NOT_SAME_DEVICE: return "ERROR_NOT_SAME_DEVICE";
		case ERROR_NO_MORE_FILES: return "ERROR_NO_MORE_FILES";
		case ERROR_WRITE_PROTECT: return "ERROR_WRITE_PROTECT";
		case ERROR_BAD_UNIT: return "ERROR_BAD_UNIT";
		case ERROR_NOT_READY: return "ERROR_NOT_READY";
		case ERROR_BAD_COMMAND: return "ERROR_BAD_COMMAND";
		case ERROR_CRC: return "ERROR_CRC";
		case ERROR_BAD_LENGTH: return "ERROR_BAD_LENGTH";
		case ERROR_SEEK: return "ERROR_SEEK";
		case ERROR_NOT_DOS_DISK: return "ERROR_NOT_DOS_DISK";
		case ERROR_WRITE_FAULT: return "ERROR_WRITE_FAULT";
		case ERROR_READ_FAULT: return "ERROR_READ_FAULT";
		case ERROR_SHARING_VIOLATION: return "ERROR_SHARING_VIOLATION";
		case ERROR_LOCK_VIOLATION: return "ERROR_LOCK_VIOLATION";
		case ERROR_WRONG_DISK: return "ERROR_WRONG_DISK";
		case E_NOINTERFACE: return "E_NOINTERFACE";
		case E_OUTOFMEMORY: return "E_OUTOFMEMORY";
		case E_FAIL: return "E_FAIL";
		case E_ACCESSDENIED: return "E_ACCESSDENIED";
		case E_INVALIDARG: return "E_INVALIDARG";
		case S_OK: return "S_OK";
		case S_FALSE: return "S_FALSE";
		default: return "unrecognized HRESULT";
	}
}

const char* oWinAsStringHR_DXGI(HRESULT _hResult)
{
	switch (_hResult)
	{
		case DXGI_ERROR_INVALID_CALL: return "DXGI_ERROR_INVALID_CALL";
		case DXGI_ERROR_NOT_FOUND: return "DXGI_ERROR_NOT_FOUND";
		case DXGI_ERROR_MORE_DATA: return "DXGI_ERROR_MORE_DATA";
		case DXGI_ERROR_UNSUPPORTED: return "DXGI_ERROR_UNSUPPORTED";
		case DXGI_ERROR_DEVICE_REMOVED: return "DXGI_ERROR_DEVICE_REMOVED";
		case DXGI_ERROR_DEVICE_HUNG: return "DXGI_ERROR_DEVICE_HUNG";
		case DXGI_ERROR_DEVICE_RESET: return "DXGI_ERROR_DEVICE_RESET";
		case DXGI_ERROR_WAS_STILL_DRAWING: return "DXGI_ERROR_WAS_STILL_DRAWING";
		case DXGI_ERROR_FRAME_STATISTICS_DISJOINT: return "DXGI_ERROR_FRAME_STATISTICS_DISJOINT";
		case DXGI_ERROR_GRAPHICS_VIDPN_SOURCE_IN_USE: return "DXGI_ERROR_GRAPHICS_VIDPN_SOURCE_IN_USE";
		case DXGI_ERROR_DRIVER_INTERNAL_ERROR: return "DXGI_ERROR_DRIVER_INTERNAL_ERROR";
		case DXGI_ERROR_NONEXCLUSIVE: return "DXGI_ERROR_NONEXCLUSIVE";
		case DXGI_ERROR_NOT_CURRENTLY_AVAILABLE: return "DXGI_ERROR_NOT_CURRENTLY_AVAILABLE";
		case DXGI_ERROR_REMOTE_CLIENT_DISCONNECTED: return "DXGI_ERROR_REMOTE_CLIENT_DISCONNECTED";
		case DXGI_ERROR_REMOTE_OUTOFMEMORY: return "DXGI_ERROR_REMOTE_OUTOFMEMORY";
		default: return "unrecognized DXGI HRESULT";
	}
}

const char* oWinAsStringHR_DX11(HRESULT _hResult)
{
	switch (_hResult)
	{
		case D3D11_ERROR_FILE_NOT_FOUND: return "D3D11_ERROR_FILE_NOT_FOUND";
		case D3D11_ERROR_TOO_MANY_UNIQUE_STATE_OBJECTS: return "D3D11_ERROR_TOO_MANY_UNIQUE_STATE_OBJECTS";
		case D3D11_ERROR_TOO_MANY_UNIQUE_VIEW_OBJECTS: return "D3D11_ERROR_TOO_MANY_UNIQUE_VIEW_OBJECTS";
		case D3D11_ERROR_DEFERRED_CONTEXT_MAP_WITHOUT_INITIAL_DISCARD: return "D3D11_ERROR_DEFERRED_CONTEXT_MAP_WITHOUT_INITIAL_DISCARD";
		case D3DERR_INVALIDCALL: return "D3DERR_INVALIDCALL";
		case D3DERR_WASSTILLDRAWING: return "D3DERR_WASSTILLDRAWING";
		default: return "unrecognized DX11 HRESULT";
	}
}

const char* oWinAsStringHR_VFW(HRESULT _hResult)
{
	switch (_hResult)
	{
		case VFW_E_INVALIDMEDIATYPE: return "VFW_E_INVALIDMEDIATYPE";
		case VFW_E_NOT_CONNECTED: return "VFW_E_NOT_CONNECTED";
		case VFW_E_NOT_STOPPED: return "VFW_E_NOT_STOPPED";
		case VFW_E_WRONG_STATE: return "VFW_E_WRONG_STATE";
		default: return "unrecognized VFW HRESULT";
	}
}

bool oWinParseHRESULT(char* _StrDestination, size_t _SizeofStrDestination, HRESULT _hResult)
{
	int len = 0;
	*_StrDestination = 0;
	FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM, 0, static_cast<DWORD>(_hResult), 0, _StrDestination, static_cast<DWORD>(_SizeofStrDestination), 0);
	if (!*_StrDestination || !memcmp(_StrDestination, "???", 3))
	{
		const char* s = oWinAsStringHR_DXGI(_hResult);

		#define UNRECOG(_Str) (!memcmp(_Str, "unre", 4))

		if (UNRECOG(s))
			s = oWinAsStringHR_VFW(_hResult);

		if (UNRECOG(s))
			s = oWinAsStringHR_DX11(_hResult);

		if (UNRECOG(s))
		{
			#if defined(_DX9) || defined(_DX10) || defined(_DX11)
				oStrcpy(_StrDestination, _SizeofStrDestination, DXGetErrorStringA(_hResult));
			#else
				len = oPrintf(_StrDestination, _SizeofStrDestination, "unrecognized error code 0x%08x", _hResult);
			#endif
		}

		else
			len = oPrintf(_StrDestination, _SizeofStrDestination, s);
	}

	if (len == -1)
		return oErrorSetLast(std::errc::no_buffer_space);

	return true;
}

const char* oWinAsStringDISP(UINT _DISPCode)
{
	switch (_DISPCode)
	{
		case DISP_CHANGE_BADDUALVIEW: return "DISP_CHANGE_BADDUALVIEW";
		case DISP_CHANGE_BADFLAGS: return "DISP_CHANGE_BADFLAGS";
		case DISP_CHANGE_BADMODE: return "DISP_CHANGE_BADMODE";
		case DISP_CHANGE_BADPARAM: return "DISP_CHANGE_BADPARAM";
		case DISP_CHANGE_FAILED: return "DISP_CHANGE_FAILED";
		case DISP_CHANGE_NOTUPDATED: return "DISP_CHANGE_NOTUPDATED";
		case DISP_CHANGE_RESTART: return "DISP_CHANGE_RESTART";
		case DISP_CHANGE_SUCCESSFUL: return "DISP_CHANGE_SUCCESSFUL";
		default: return "unrecognized DISPCode";
	}
}
