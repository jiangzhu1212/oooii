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
#include <oPlatform/Windows/oWinAsString.h>
#include <oBasis/oString.h>
#include <oBasis/oError.h>
#include <Windowsx.h>
#include <DShow.h>
#include <CDErr.h>
#include "oVKToX11Keyboard.h"

const char* oWinAsStringHT(unsigned int _HTCode)
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

const char* oWinAsStringSC(unsigned int _SCCode)
{
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
		default: break;
	}

	return "unrecognized SCCODE";
}

const char* oWinAsStringSW(unsigned int _SWCode)
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

const char* oWinAsStringWM(unsigned int _uMsg)
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
		case WM_DRAWITEM: return "WM_DRAWITEM";
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
		default: break;
	}
	return "unrecognized WMCODE";
}

const char* oWinAsStringWS(unsigned int _WSFlag)
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

const char* oWinAsStringWA(unsigned int _WACode)
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

const char* oWinAsStringBST(unsigned int _BSTCode)
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

const char* oWinAsStringNM(unsigned int _NMCode)
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

const char* oWinAsStringSWP(unsigned int _SWPCode)
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

const char* oWinAsStringGWL(unsigned int _GWLCode)
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

const char* oWinAsStringGWLP(unsigned int _GWLPCode)
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

const char* oWinAsStringTCN(unsigned int _TCNCode)
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

const char* oWinAsStringCDERR(unsigned int _CDERRCode)
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

const char* oWinAsStringExceptionCode(unsigned int _ExceptionCode)
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

char* oWinParseStyleFlags(char* _StrDestination, size_t _SizeofStrDestination, UINT _WSFlags)
{
	return oAsStringFlags(_StrDestination, _SizeofStrDestination, _WSFlags, oWinAsStringWS(WS_OVERLAPPED), [&](unsigned int _Flag) { return oWinAsStringWS(_Flag); });
}

char* oWinParseSWPFlags(char* _StrDestination, size_t _SizeofStrDestination, UINT _SWPFlags)
{
	return oAsStringFlags(_StrDestination, _SizeofStrDestination, _SWPFlags & 0x07ff, "0", [&](unsigned int _Flag) { return oWinAsStringSWP(_Flag); });
}

char* oWinParseWMMessage(char* _StrDestination, size_t _SizeofStrDestination, HWND _hWnd, UINT _uMsg, WPARAM _wParam, LPARAM _lParam)
{
	// http://www.autoitscript.com/autoit3/docs/appendix/WinMsgCodes.htm

	switch (_uMsg)
	{ 
		case WM_ACTIVATE: oPrintf(_StrDestination, _SizeofStrDestination, "HWND 0x%x WM_ACTIVATE %s, other HWND 0x%x %sactivated", _hWnd, oWinAsStringWA((UINT)_wParam), _lParam, _wParam == WA_INACTIVE ? "" : "de"); break;
		case WM_ACTIVATEAPP: oPrintf(_StrDestination, _SizeofStrDestination, "HWND 0x%x WM_ACTIVATEAPP %sactivated, other thread %d %sactivated", _hWnd, _wParam ? "" : "de", _lParam, _wParam ? "de" : ""); break;
		case WM_NCACTIVATE: oPrintf(_StrDestination, _SizeofStrDestination, "HWND 0x%x WM_NCACTIVATE titlebar/icon needs to be changed: %s lParam=%x", _hWnd, _wParam ? "true" : "false", _lParam); break;
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
		case WM_SYSCOMMAND: oPrintf(_StrDestination, _SizeofStrDestination, "HWND 0x%x WM_SYSCOMMAND: wParam=%s screenpos=%d,%d", _hWnd, oWinAsStringSC((UINT)_wParam), GET_X_LPARAM(_lParam), GET_Y_LPARAM(_lParam)); break;
		case WM_SYSKEYDOWN: oPrintf(_StrDestination, _SizeofStrDestination, "HWND 0x%x WM_SYSKEYDOWN: wParam=%s lParam=%0x", _hWnd, oAsString(TranslateKeyToX11(_wParam)), _lParam); break;
		case WM_SYSKEYUP: oPrintf(_StrDestination, _SizeofStrDestination, "HWND 0x%x WM_SYSKEYUP: wParam=%s lParam=%0x", _hWnd, oAsString(TranslateKeyToX11(_wParam)), _lParam); break;
		case WM_KEYDOWN: oPrintf(_StrDestination, _SizeofStrDestination, "HWND 0x%x WM_KEYDOWN: wParam=%s lParam=%0x", _hWnd, oAsString(TranslateKeyToX11(_wParam)), _lParam); break;
		case WM_KEYUP: oPrintf(_StrDestination, _SizeofStrDestination, "HWND 0x%x WM_KEYUP: wParam=%s lParam=%0x", _hWnd, oAsString(TranslateKeyToX11(_wParam)), _lParam); break;
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
		return oErrorSetLast(oERROR_AT_CAPACITY);

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
