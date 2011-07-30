// $(header)
#include <oooii/oMsgBox.h>
#include <oooii/oByte.h>
#include <oooii/oStddef.h>
#include <oooii/oStdio.h>
#include <oooii/oSTL.h>
#include <oooii/oString.h>
#include <oooii/oThreading.h>
#include <oooii/oWindows.h>

#if defined(_WIN32) || defined(_WIN64)
	// Use the Windows Vista UI look. If this causes issues or the dialog not to appear, try other values from processorAchitecture { x86 ia64 amd64 * }
	#pragma comment(linker, "\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif

#define IDMESSAGE 20
#define IDCOPYTOCLIPBOARD 21
#define IDICON 22

oMsgBox::RESULT AsAction(WORD _ID)
{
	switch (_ID)
	{
		case IDCANCEL: return oMsgBox::ABORT;
		case IDRETRY: return oMsgBox::BREAK;
		case IDCONTINUE: return oMsgBox::CONTINUE;
		case IDIGNORE: return oMsgBox::IGNORE_ALWAYS;
		default: return oMsgBox::CONTINUE;
	}
}

HICON LoadIcon(oMsgBox::TYPE _Type)
{
	static LPCSTR map[] =
	{
		IDI_INFORMATION,
		IDI_WARNING,
		IDI_QUESTION,
		IDI_ERROR,
		IDI_ERROR,
		IDI_INFORMATION,
		IDI_INFORMATION,
		IDI_WARNING,
		IDI_ERROR,
	};

	if (_Type >= oCOUNTOF(map))
		_Type = oMsgBox::ERR;
	return LoadIcon(0, map[_Type]);
}

static VOID CALLBACK WndDialogTimerProc(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
{
	switch (idEvent)
	{
		case 1:
			EndDialog(hwnd, IDCONTINUE);
			break;
		case 2:
			EnableWindow(GetDlgItem(hwnd, IDCONTINUE), TRUE);
			EnableWindow(GetDlgItem(hwnd, IDIGNORE), TRUE);
			KillTimer(hwnd, idEvent);
			break;
	}
}

struct DialogParams
{
	DialogParams(HICON _hIcon, unsigned int _TimeoutMS, unsigned int _DisableTimeoutMS)
		: hIcon(_hIcon)
		, TimeoutMS(static_cast<UINT>(_TimeoutMS))
		, DisableTimeoutMS(static_cast<UINT>(_DisableTimeoutMS))
	{}

	HICON hIcon;
	UINT TimeoutMS;
	UINT DisableTimeoutMS;
};

static INT_PTR CALLBACK WndDialogProc(HWND _Hwnd, UINT _uMsg, WPARAM _wParam, LPARAM _lParam)
{
	switch (_uMsg)
	{
		case WM_INITDIALOG:
		{
			DialogParams& p = *(DialogParams*)_lParam;
			if (p.TimeoutMS != oINFINITE_WAIT)
				SetTimer(_Hwnd, 1, p.TimeoutMS, WndDialogTimerProc);
			if (p.DisableTimeoutMS != 0 && p.DisableTimeoutMS != oINFINITE_WAIT)
				SetTimer(_Hwnd, 2, p.DisableTimeoutMS, WndDialogTimerProc);
			SendDlgItemMessage(_Hwnd, IDICON, STM_SETICON, (WPARAM)p.hIcon, 0);
			return false;
		}

	case WM_COMMAND:
		switch (_wParam)
		{
			case IDCANCEL:
			case IDRETRY:
			case IDCONTINUE:
			case IDIGNORE:
				EndDialog(_Hwnd, _wParam);
				return true;
			case IDCOPYTOCLIPBOARD:
				SendDlgItemMessage(_Hwnd, IDMESSAGE, EM_SETSEL, 0, -1);
				SendDlgItemMessage(_Hwnd, IDMESSAGE, WM_COPY, 0, 0);
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

oMsgBox::RESULT AssertDialog(oMsgBox::TYPE _Type, const char* _Caption, const char* _String, unsigned int _DialogTimeoutMS, unsigned int _IgnoreDisableTimeoutMS)
{
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
	std::vector<char> string(oKB(128));

	calcStringRect(rString, oNewlinesToDos(oGetData(string), oGetDataSize(string), _String), MinW, MinH, MaxW, MaxH);

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
	RECT rBreak = { rAbort.right + BtnSpacingX, BtnPanelTop, rBreak.left + BtnW, BtnPanelBottom }; 
	RECT rContinue = { rBreak.right + BtnSpacingX, BtnPanelTop, rContinue.left + BtnW, BtnPanelBottom };
	RECT rIgnore = { rContinue.right + BtnSpacingX, BtnPanelTop, rIgnore.left + BtnW, BtnPanelBottom };

	// copy to clipboard is left-aligned
	RECT rCopyToClipboard = { FrameSpacingX, BtnPanelTop, rCopyToClipboard.left + BtnCopyW, rCopyToClipboard.top + BtnH };

	// Make the overall dialog
	RECT rDialog = { 0, 0, __max(rString.right, rIgnore.right + FrameSpacingX), __max(rString.bottom, rIgnore.bottom + FrameSpacingY) };

	bool TimedoutControlledEnable = (_IgnoreDisableTimeoutMS == 0);
	const oWINDOWS_DIALOG_ITEM items[] = 
	{
		{ "&Abort", oDLG_BUTTON, IDCANCEL, rAbort, true, true, true },
		{ "&Break", oDLG_BUTTON, IDRETRY, rBreak, true, true, true },
		{ "&Continue", oDLG_BUTTON, IDCONTINUE, rContinue, TimedoutControlledEnable, true, true },
		{ "I&gnore", oDLG_BUTTON, IDIGNORE, rIgnore, TimedoutControlledEnable, true, true },
		{ "Copy &To Clipboard", oDLG_BUTTON, IDCOPYTOCLIPBOARD, rCopyToClipboard, true, true, true },
		{ oGetData(string), oDLG_LARGELABEL, IDMESSAGE, rString, true, true, true },
		{ "", oDLG_ICON, IDICON, rIcon, true, true, false },
	};

	oWINDOWS_DIALOG_DESC dlg;
	dlg.Font = "Tahoma";
	dlg.Caption = _Caption;
	dlg.pItems = items;
	dlg.NumItems = oCOUNTOF(items);
	dlg.FontPointSize = 8;
	dlg.Rect = rDialog;
	dlg.Center = true;
	dlg.SetForeground = true;
	dlg.Enabled = true;
	dlg.Visible = true;
	dlg.AlwaysOnTop = true;

	LPDLGTEMPLATE lpDlgTemplate = oDlgNewTemplate(dlg);

	HICON hIcon = LoadIcon(_Type);

	DialogParams p(hIcon, _DialogTimeoutMS, _IgnoreDisableTimeoutMS);
	INT_PTR int_ptr = DialogBoxIndirectParam(GetModuleHandle(0), lpDlgTemplate, GetDesktopWindow(), WndDialogProc, (LPARAM)&p);
	
	oDlgDeleteTemplate(lpDlgTemplate);

	if (int_ptr == -1)
	{
		char s[2048];
		oGetWindowsErrorDescription(s, GetLastError());
		sprintf_s(s, "DialogBoxIndirectParam failed. %s", s);
		__debugbreak(); // debug msgbox called from oASSERTs, so don't recurse into it
	}

	DeleteObject(hIcon);

	oMsgBox::RESULT result = AsAction(static_cast<WORD>(int_ptr));
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
	std::vector<char> msg(oKB(128));
	vsprintf_s(oGetData(msg), oGetDataSize(msg), _Format, args);
	oMsgBox::RESULT result = YES;
	HICON hIcon = 0;

	switch (_Type)
	{
		case oMsgBox::DEBUG:
			result = AssertDialog(_Type, _Title, oGetData(msg), _Timeout, 0);
			break;
		
		case NOTIFY_INFO:
		case NOTIFY_WARN:
		case NOTIFY_ERR:
			hIcon = LoadIcon(_Type);
			// pass thru

		case oMsgBox::NOTIFY:
			oVERIFY(oTrayShowMessage(oGetWindowFromThreadID(oThreadGetMainID()), 0, hIcon, __max(2000, _Timeout), _Title, oGetData(msg)));
			result = CONTINUE;
			break;

		default:
			result = GetResult(MessageBoxTimeout(oGetWindowFromThreadID(oThreadGetMainID()), oGetData(msg), _Title, AsFlags(_Type), 0, _Timeout));
			break;
	}

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
	return tvprintf(_Type, oINFINITE_WAIT, _Title, _Format, args);
}

oMsgBox::RESULT oMsgBox::printf(oMsgBox::TYPE _Type, const char* _Title, const char* _Format, ...)
{
	va_list args;
	va_start(args, _Format);
	return vprintf(_Type, _Title, _Format, args);
}
