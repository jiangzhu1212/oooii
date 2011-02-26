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
#include "pch.h"
#include <oooii/oMsgBox.h>
#include <oooii/oByte.h>
#include <oooii/oStddef.h>
#include <oooii/oStdio.h>
#include <oooii/oString.h>
#include <oooii/oWindows.h>
#include <oooii/oTempString.h>

#if defined(_WIN32) || defined(_WIN64)
	// Use the Windows Vista UI look. If this causes issues or the dialog not to appear, try other values from processorAchitecture { x86 ia64 amd64 * }
	#pragma comment(linker, "\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif

static const size_t _INFINITE_WAIT = ~0u;

static const size_t kVeryLargeStringLength = 60 * 1024;

namespace Control
{
enum Class
{
	kButton = 0x0080,
	kEdit = 0x0081,
	kStatic = 0x0082,
	kStaticIcon = SS_ICON,
	kListBox = 0x0083,
	kScrollBar = 0x0084,
	kComboBox = 0x0085,
};

enum Id
{
	// ID* is assigned in order as found in WinUser.h
	// hopefully user Ids will just increment after
	// the default ones
	kOk = IDOK,
	kCancel = IDCANCEL,
	kAbort = IDABORT,
	kRetry = IDRETRY, 
	kIgnore = IDIGNORE,
	kYes = IDYES,
	kNo = IDNO,
	kClose = IDCLOSE,
	kHelp = IDHELP,
	kTryAgain = IDTRYAGAIN,
	kContinue = IDCONTINUE,
	kIgnoreFuture = kContinue + 1,
	kIcon,
	kMessageText,
	kCopyToClipboard,
};

oMsgBox::RESULT toAction(Id id)
{
	switch (id)
	{
	case kAbort: return oMsgBox::ABORT;
	case kRetry: return oMsgBox::BREAK;
	case kIgnore: return oMsgBox::CONTINUE;
	case kIgnoreFuture: return oMsgBox::IGNORE_ALWAYS;
	default: break;
	}

	return oMsgBox::CONTINUE;
}

} // namespace Control

LPCSTR toIcon(oMsgBox::TYPE type)
{
	static LPCSTR map[] =
	{
		IDI_INFORMATION,
		IDI_WARNING,
		IDI_QUESTION,
		IDI_ERROR,
		IDI_ERROR,
	};

	if (type >= oCOUNTOF(map))
		return IDI_ERROR;
	return map[type];
}

static VOID CALLBACK WndDialogTimerProc(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
{
	switch (idEvent)
	{
	case 1:
		EndDialog(hwnd, Control::kIgnore);
		break;
	case 2:
		EnableWindow(GetDlgItem(hwnd, Control::kIgnore), TRUE);
		EnableWindow(GetDlgItem(hwnd, Control::kIgnoreFuture), TRUE);
		KillTimer(hwnd, idEvent);
		break;
	}
}

struct DialogParams
{
	DialogParams(HICON hIcon_, unsigned int timeoutMS_, unsigned int disableTimeoutMS_)
		: hIcon(hIcon_)
		, timeoutMS(static_cast<UINT>(timeoutMS_))
		, disableTimeoutMS(static_cast<UINT>(disableTimeoutMS_))
	{}

	HICON hIcon;
	UINT timeoutMS;
	UINT disableTimeoutMS;
};

static INT_PTR CALLBACK WndDialogProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_INITDIALOG:
	{
		DialogParams& p = *(DialogParams*)lParam;
		if (p.timeoutMS != _INFINITE_WAIT)
			SetTimer(hwnd, 1, p.timeoutMS, WndDialogTimerProc);
		if (p.disableTimeoutMS != _INFINITE_WAIT)
			SetTimer(hwnd, 2, p.disableTimeoutMS, WndDialogTimerProc);
		SendDlgItemMessage(hwnd, Control::kIcon, STM_SETICON, (WPARAM)p.hIcon, 0);
		return false;
	}

	case WM_COMMAND:
		switch (wParam)
		{
		case Control::kAbort:
		case Control::kRetry:
		case Control::kIgnore:
		case Control::kIgnoreFuture:
			EndDialog(hwnd, wParam);
			return true;

		case Control::kCopyToClipboard:
			SendDlgItemMessage(hwnd, Control::kMessageText, EM_SETSEL, 0, -1);
			SendDlgItemMessage(hwnd, Control::kMessageText, WM_COPY, 0, 0);
			return true;
		default:
			break;
		}

	default:
		break;
	}

	return false;
}

static void calcStringRect(RECT& rString, const char* string, LONG minW, LONG minH, LONG maxW, LONG maxH)
{
	rString.left = 0;
	rString.top = 0;
	rString.right = 1;
	rString.bottom = 1;
	HDC hDC = CreateCompatibleDC(0);
	DrawTextA(hDC, string, -1, &rString, DT_CALCRECT|DT_LEFT|DT_NOPREFIX|DT_WORDBREAK);
	DeleteDC(hDC);
	// use a fudge-factor for the not-so-nice numbers that come out of DrawText
	rString.bottom = __min(maxH, __max(minH, rString.bottom * 3/4));
	rString.right = __min(maxW, __max(minW, rString.right * 3/4));

	if (rString.bottom == maxH)
		rString.right = maxW;
}

static size_t calcTemplateSize(size_t captionTextLen, const char* fontName)
{
	// Accumulate buffer information. menu, class, and caption need to be WORD
	// aligned, but since the first few are 0, don't worry about it. If either
	// of these changes, alignment may have to be enforced.
	size_t size = sizeof(DLGTEMPLATE) +
		sizeof(WORD) + // this will be 0x0000 because there is no menu
		sizeof(WORD) + // this will be 0x0000 because the default class will do
		sizeof(WCHAR) * (captionTextLen + 1); // caption string

	if (fontName && strlen(fontName) > 0)
	{
		size = oByteAlign(size, sizeof(WORD));
		size += strlen(fontName) + 1;
	}

	return oByteAlign(size, sizeof(DWORD));
}

static size_t calcTemplateItemSize(size_t itemTextLen)
{
	return oByteAlign(sizeof(DLGITEMTEMPLATE) +
		2 * sizeof(WORD) + // will keep it simple 0xFFFF and the ControlClass
		sizeof(WCHAR) * (itemTextLen + 1) + // text
		sizeof(WORD), // 0x0000. This is used for data to be passed to WM_CREATE, bypass this for now
		sizeof(DWORD)); // in size calculations, ensure everything is DWORD-aligned
}

void initializeItem(void*& buffer, const char* text, Control::Id id, Control::Class controlClass, DWORD style, const RECT& r)
{
	buffer = (void*)oByteAlign(buffer, sizeof(DWORD));
	LPDLGITEMTEMPLATE it = (LPDLGITEMTEMPLATE)buffer;
	it->style = style;
	it->dwExtendedStyle = 4;
	it->x = short(r.left);
	it->y = short(r.top);
	it->cx = short(r.right - r.left);
	it->cy = short(r.bottom - r.top);
	it->id = WORD(id);
	WORD* cur = (WORD*)((BYTE*)buffer + sizeof(DLGITEMTEMPLATE));
	*cur++ = 0xFFFF; // flag that a simple ControlClass is to follow
	*cur++ = WORD(controlClass);
	size_t textLen = oStrConvert((WCHAR*)cur, kVeryLargeStringLength, text);
	cur = (WORD*)((BYTE*)cur + (textLen+1) * sizeof(WCHAR));
	*cur++ = 0x0000; // no WM_CREATE data
	buffer = (void*)oByteAlign(cur, sizeof(DWORD));
}

oMsgBox::RESULT AssertDialog(oMsgBox::TYPE type, const char* caption, const char* string, unsigned int dialogTimeoutMS, unsigned int ignoreDisableTimeoutMS)
{
	WORD fontSize = 8;
	const char* font = "Tahoma";
	const char* textBtn0 = "&Abort";
	const char* textBtn1 = "&Break";
	const char* textBtn2 = "&Continue";
	const char* textBtn3 = "I&gnore";
	const char* textBtn4 = "Copy &To Clipboard";
	const char* textIcon = "";

	const LONG FrameSpacingX = 5;
	const LONG FrameSpacingY = 6;
	const LONG BtnSpacingX = 3;
	const LONG BtnW = 42;
	const LONG BtnH = 13;
	const LONG BtnCopyW = 65;
	const LONG BtnPanelW = 4 * BtnW + 3 * BtnSpacingX;
	const LONG MinW = BtnCopyW + BtnSpacingX + BtnPanelW + 2 * FrameSpacingX;
	const LONG MinH = 50;
	const LONG MaxW = MinW + 150;
	const LONG MaxH = MinH + 150;

	RECT rString;
	oTempString _string(oTempString::BigSize);

	calcStringRect(rString, oNewlinesToDos(_string, string), MinW, MinH, MaxW, MaxH);

	// Figure out where interface goes based on string RECT
	const LONG BtnPanelLeft = (rString.right - BtnPanelW) - FrameSpacingX;
	const LONG BtnPanelTop = rString.bottom;
	const LONG BtnPanelBottom = BtnPanelTop + BtnH;

	// Offset string box to make room for icon on left
	RECT rIcon = { FrameSpacingX, FrameSpacingY, rIcon.left + (GetSystemMetrics(SM_CXICON)/2), rIcon.top + (GetSystemMetrics(SM_CYICON)/2) };
	rString.left += rIcon.right + FrameSpacingX + 2;
	rString.top += FrameSpacingY;
	rString.right -= FrameSpacingX;
	rString.bottom -= FrameSpacingY;
	
	// Assign the bounds for the rest of the dialog items

	// 4 main control options are right-aligned, but spaced evenly
	RECT rAbort = { BtnPanelLeft, BtnPanelTop, rAbort.left + BtnW, BtnPanelBottom };
	RECT rDebug = { rAbort.right + BtnSpacingX, BtnPanelTop, rDebug.left + BtnW, BtnPanelBottom }; 
	RECT rIgnore = { rDebug.right + BtnSpacingX, BtnPanelTop, rIgnore.left + BtnW, BtnPanelBottom };
	RECT rIgnoreFuture = { rIgnore.right + BtnSpacingX, BtnPanelTop, rIgnoreFuture.left + BtnW, BtnPanelBottom };

	// copy to clipboard is left-aligned
	RECT rCopyToClipboard = { FrameSpacingX, BtnPanelTop, rCopyToClipboard.left + BtnCopyW, rCopyToClipboard.top + BtnH };

	// Make the overall dialog
	RECT rDialog = { 0, 0, __max(rString.right, rIgnoreFuture.right + FrameSpacingX), __max(rString.bottom, rIgnoreFuture.bottom + FrameSpacingY) };

	// Prepare a buffer large enough to hold the template definition
	size_t captionLen = strlen(caption);
	size_t templateBufferSize = calcTemplateSize(captionLen, font);
	templateBufferSize += calcTemplateItemSize(strlen(_string));
	templateBufferSize += calcTemplateItemSize(strlen(textBtn0));
	templateBufferSize += calcTemplateItemSize(strlen(textBtn1));
	templateBufferSize += calcTemplateItemSize(strlen(textBtn2));
	templateBufferSize += calcTemplateItemSize(strlen(textBtn3));
	templateBufferSize += calcTemplateItemSize(strlen(textIcon));
	templateBufferSize += calcTemplateItemSize(strlen(textBtn4));

//	if (templateBufferSize > (sizeof(_string) + 1024)) __debugbreak(); // don't use Assert to avoid recursion
	oTempString _buffer(oTempString::BigSize);
	char* buffer = _buffer;

	// Allocate and set up the template header
	LPDLGTEMPLATE lpTemplate = (LPDLGTEMPLATE)buffer;
	lpTemplate->style = WS_CAPTION|WS_TABSTOP|WS_VISIBLE|WS_POPUP|DS_MODALFRAME|DS_CENTER|DS_SETFONT;
	lpTemplate->dwExtendedStyle = 0; // WS_EX_TOPMOST to make always-on-top
	lpTemplate->cdit = 7;
	lpTemplate->cx = short(rDialog.right - rDialog.left);
	lpTemplate->cy = short(rDialog.bottom - rDialog.top);

	WORD* cur = (WORD*)((BYTE*)lpTemplate + sizeof(DLGTEMPLATE));
	*cur++ = 0x0000; // no menu
	*cur++ = 0x0000; // use default class
	captionLen = oStrConvert((WCHAR*)cur, kVeryLargeStringLength, caption);
	cur = (WORD*)((BYTE*)cur + (captionLen+1) * sizeof(WCHAR));

	*cur++ = fontSize;
	size_t titleLen = oStrConvert((WCHAR*)cur, kVeryLargeStringLength, font);
	cur = (WORD*)((BYTE*)cur + (titleLen+1) * sizeof(WCHAR));
	
	// Now fill in all the items
	void* vcur = (void*)cur;

	DWORD BtnStyle = WS_CHILD|WS_VISIBLE|WS_TABSTOP;

	initializeItem(vcur, _string.c_str(), Control::kMessageText, Control::kEdit, WS_CHILD|WS_VISIBLE|ES_LEFT|ES_MULTILINE|ES_READONLY|WS_VSCROLL, rString);
	initializeItem(vcur, textBtn0, Control::kAbort, Control::kButton, BtnStyle, rAbort);
	initializeItem(vcur, textBtn1, Control::kRetry, Control::kButton, BtnStyle|BS_DEFPUSHBUTTON, rDebug);
	
	DWORD dwStyleIgnore = BtnStyle;
	if (ignoreDisableTimeoutMS != _INFINITE_WAIT)
		dwStyleIgnore |= WS_DISABLED;
	
	initializeItem(vcur, textBtn2, Control::kIgnore, Control::kButton, dwStyleIgnore, rIgnore);
	initializeItem(vcur, textBtn3, Control::kIgnoreFuture, Control::kButton, dwStyleIgnore, rIgnoreFuture);
	initializeItem(vcur, textIcon, Control::kIcon, Control::kStatic, WS_CHILD|WS_VISIBLE|SS_ICON, rIcon);
	initializeItem(vcur, textBtn4, Control::kCopyToClipboard, Control::kButton, BtnStyle, rCopyToClipboard);

	HICON hIcon = LoadIcon(0, toIcon(type));

	DialogParams p(hIcon, dialogTimeoutMS, ignoreDisableTimeoutMS);
	INT_PTR int_ptr = DialogBoxIndirectParam(GetModuleHandle(0), lpTemplate, GetDesktopWindow(), WndDialogProc, (LPARAM)&p);
	if (int_ptr == -1)
	{
		char s[2048];
		oGetNativeErrorDesc(s, GetLastError());
		sprintf_s(s, "DialogBoxIndirectParam failed. %s", s);
		__debugbreak();
	}

	DeleteObject(hIcon);

	oMsgBox::RESULT result = Control::toAction(static_cast<Control::Id>(int_ptr));
	return result;
}

UINT AsFlags(oMsgBox::TYPE type)
{
	switch (type)
	{
		default:
		case oMsgBox::INFO: return MB_ICONINFORMATION|MB_OK;
		case oMsgBox::WARN: return MB_ICONWARNING|MB_OK;
		case oMsgBox::YESNO: return MB_ICONQUESTION|MB_YESNO|MB_DEFBUTTON1;
		case oMsgBox::ERR: return MB_ICONERROR|MB_OK;
		case oMsgBox::DEBUG: return MB_ICONERROR|MB_ABORTRETRYIGNORE;
	}
}

oMsgBox::RESULT GetResult(int messageBoxResult)
{
	switch (messageBoxResult)
	{
		default:
		case IDOK: return oMsgBox::YES;
		case IDABORT: return oMsgBox::ABORT;
		case IDIGNORE: return oMsgBox::CONTINUE; 
		case IDRETRY: return oMsgBox::BREAK;
		case IDYES: return oMsgBox::YES;
		case IDNO: return oMsgBox::NO;
	}
}

oMsgBox::RESULT oMsgBox::tvprintf(oMsgBox::TYPE _Type, unsigned int _Timeout, const char* _Title, const char* _Format, va_list args)
{
	oTempString msg(oTempString::BigSize);
	vsprintf_s(msg, _Format, args);
	oMsgBox::RESULT result;
	if (_Type == oMsgBox::DEBUG)
		result = AssertDialog(_Type, _Title, msg, _Timeout, 0);
	else
		result = GetResult(MessageBoxTimeout(0, msg, _Title, AsFlags(_Type), 0, _Timeout));
	return result;
}

oMsgBox::RESULT oMsgBox::tprintf(oMsgBox::TYPE _Type, unsigned int _Timeout, const char* _Title, const char* _Format, ...)
{
	va_list args;
	va_start(args, _Format);
	return tvprintf(_Type, _Timeout, _Title, _Format, args);
}

oMsgBox::RESULT oMsgBox::vprintf(oMsgBox::TYPE _Type, const char* _Title, const char* _Format, va_list args)
{
	return tvprintf(_Type, _INFINITE_WAIT, _Title, _Format, args);
}

oMsgBox::RESULT oMsgBox::printf(oMsgBox::TYPE _Type, const char* _Title, const char* _Format, ...)
{
	va_list args;
	va_start(args, _Format);
	return vprintf(_Type, _Title, _Format, args);
}
