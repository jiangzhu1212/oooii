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
#include <oGUI/oMsgBox.h>
#include <oBase/byte.h>
#include <oGUI/Windows/oWinCursor.h>
#include <oGUI/Windows/oWinDialog.h>
#include <oGUI/Windows/oWinRect.h>
#include <oGUI/Windows/oWinTray.h>
#include <oGUI/Windows/oWinWindowing.h>
#include <oCore/windows/win_error.h>

// Secret function that is not normally exposed in headers.
// Typically pass 0 for wLanguageId, and specify a timeout
// for the dialog in milliseconds, returns MB_TIMEDOUT if
// the timeout is reached.
int MessageBoxTimeoutA(IN HWND hWnd, IN LPCSTR lpText, IN LPCSTR lpCaption, IN UINT uType, IN WORD wLanguageId, IN DWORD dwMilliseconds);
int MessageBoxTimeoutW(IN HWND hWnd, IN LPCWSTR lpText, IN LPCWSTR lpCaption, IN UINT uType, IN WORD wLanguageId, IN DWORD dwMilliseconds);

#ifdef UNICODE
	#define MessageBoxTimeout MessageBoxTimeoutW
#else
	#define MessageBoxTimeout MessageBoxTimeoutA
#endif 

#define MB_TIMEDOUT 32000

// Link to MessageBoxTimeout based on code from:
// http://www.codeproject.com/KB/cpp/MessageBoxTimeout.aspx

//Functions & other definitions required-->
typedef int (__stdcall *MSGBOXAAPI)(IN HWND hWnd, 
        IN LPCSTR lpText, IN LPCSTR lpCaption, 
        IN UINT uType, IN WORD wLanguageId, IN DWORD dwMilliseconds);
typedef int (__stdcall *MSGBOXWAPI)(IN HWND hWnd, 
        IN LPCWSTR lpText, IN LPCWSTR lpCaption, 
        IN UINT uType, IN WORD wLanguageId, IN DWORD dwMilliseconds);

int MessageBoxTimeoutA(HWND hWnd, LPCSTR lpText, 
    LPCSTR lpCaption, UINT uType, WORD wLanguageId, 
    DWORD dwMilliseconds)
{
    static MSGBOXAAPI MsgBoxTOA = nullptr;

    if (!MsgBoxTOA)
    {
        HMODULE hUser32 = GetModuleHandle("user32.dll");
        if (hUser32)
        {
            MsgBoxTOA = (MSGBOXAAPI)GetProcAddress(hUser32, 
                                      "MessageBoxTimeoutA");
            //fall through to 'if (MsgBoxTOA)...'
        }
        else
        {
            //stuff happened, add code to handle it here 
            //(possibly just call MessageBox())
            return 0;
        }
    }

    if (MsgBoxTOA)
    {
        return MsgBoxTOA(hWnd, lpText, lpCaption, 
              uType, wLanguageId, dwMilliseconds);
    }

    return 0;
}

int MessageBoxTimeoutW(HWND hWnd, LPCWSTR lpText, 
    LPCWSTR lpCaption, UINT uType, WORD wLanguageId, DWORD dwMilliseconds)
{
    static MSGBOXWAPI MsgBoxTOW = nullptr;

    if (!MsgBoxTOW)
    {
        HMODULE hUser32 = GetModuleHandle("user32.dll");
        if (hUser32)
        {
            MsgBoxTOW = (MSGBOXWAPI)GetProcAddress(hUser32, 
                                      "MessageBoxTimeoutW");
            //fall through to 'if (MsgBoxTOW)...'
        }
        else
        {
            //stuff happened, add code to handle it here 
            //(possibly just call MessageBox())
            return 0;
        }
    }

    if (MsgBoxTOW)
    {
        return MsgBoxTOW(hWnd, lpText, lpCaption, 
               uType, wLanguageId, dwMilliseconds);
    }

    return 0;
}

#define IDMESSAGE 20
#define IDCOPYTOCLIPBOARD 21
#define IDICON 22

oMSGBOX_RESULT AsAction(WORD _ID)
{
	switch (_ID)
	{
		case IDCANCEL: return oMSGBOX_ABORT;
		case IDRETRY: return oMSGBOX_BREAK;
		case IDCONTINUE: return oMSGBOX_CONTINUE;
		case IDIGNORE: return oMSGBOX_IGNORE;
		default: return oMSGBOX_CONTINUE;
	}
}

HICON LoadIcon(oMSGBOX_TYPE _Type)
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
		_Type = oMSGBOX_ERR;
	return LoadIcon(0, map[_Type]);
}

static VOID CALLBACK WndDialogTimerProc(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
{
	switch (idEvent)
	{
		case 1:
			EndDialog(hwnd, IDCANCEL);
			break;
		case 2:
			EnableWindow(GetDlgItem(hwnd, IDCONTINUE), TRUE);
			EnableWindow(GetDlgItem(hwnd, IDIGNORE), TRUE);
			KillTimer(hwnd, idEvent);
			break;
	}
}

struct DLGAssert
{
	DLGAssert(HICON _hIcon, unsigned int _TimeoutMS, unsigned int _DisableTimeoutMS)
		: hIcon(_hIcon)
		, TimeoutMS(static_cast<UINT>(_TimeoutMS))
		, DisableTimeoutMS(static_cast<UINT>(_DisableTimeoutMS))
		, hBGBrush(nullptr/*CreateSolidBrush(RGB(255,255,255))*/) // I want to make the top part of the dialog white and bottom gray like in Win7, but I can't change the BG color of a read-only editbox since the WM_CTLCOLOREDIT doesn't get sent
	{}

	~DLGAssert()
	{
		DeleteObject(hBGBrush);
	}

	oDECLARE_DLGPROC(, WndProc);
	oDECLARE_DLGPROC(static, StaticWndProc);

	HICON hIcon;
	UINT TimeoutMS;
	UINT DisableTimeoutMS;
	HBRUSH hBGBrush;
};

oDEFINE_DLGPROC(DLGAssert, StaticWndProc);

INT_PTR CALLBACK DLGAssert::WndProc(HWND _hWnd, UINT _uMsg, WPARAM _wParam, LPARAM _lParam)
{
	//char desc[512];
	//oGetWMDesc(desc, _hWnd, _uMsg, _wParam, _lParam);
	//oTRACE("%s", desc);

	switch (_uMsg)
	{
		case WM_INITDIALOG:
		{
			if (TimeoutMS != oInfiniteWait)
				SetTimer(_hWnd, 1, TimeoutMS, WndDialogTimerProc);
			if (DisableTimeoutMS != 0 && DisableTimeoutMS != oInfiniteWait)
				SetTimer(_hWnd, 2, DisableTimeoutMS, WndDialogTimerProc);
			SendDlgItemMessage(_hWnd, IDICON, STM_SETICON, (WPARAM)hIcon, 0);
			return false;
		}

		case WM_SETCURSOR:
			oWinCursorSetVisible();
			SetCursor(LoadCursor(nullptr, IDC_ARROW));
			return true;

		case WM_COMMAND:
			switch (_wParam)
			{
				case IDCANCEL:
				case IDRETRY:
				case IDCONTINUE:
				case IDIGNORE:
					EndDialog(_hWnd, _wParam);
					return true;
				case IDCOPYTOCLIPBOARD:
					SendDlgItemMessage(_hWnd, IDMESSAGE, EM_SETSEL, 0, -1);
					SendDlgItemMessage(_hWnd, IDMESSAGE, WM_COPY, 0, 0);
					return true;
				default:
					break;
			}

		//case WM_CTLCOLORDLG:
		//	return (INT_PTR)hBGBrush;

		//case WM_CTLCOLOREDIT:
		//{
		//	if (HWND(_lParam) == GetDlgItem(_hWnd, IDMESSAGE))
		//	{
		//		SetBkMode((HDC)_wParam, TRANSPARENT);
		//		return (INT_PTR)hBGBrush;
		//	}
		//	break;
		//}

		//case WM_CTLCOLORBTN:
		//	break;

		//case WM_CTLCOLORSTATIC:
		//	break;

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

oMSGBOX_RESULT AssertDialog(oMSGBOX_TYPE _Type, const char* _Caption, const char* _String, unsigned int _DialogTimeoutMS, unsigned int _IgnoreDisableTimeoutMS)
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

	ouro::replace(ouro::data(string), ouro::size(string), _String, "\n", "\r\n");
	calcStringRect(rString, ouro::data(string), MinW, MinH, MaxW, MaxH);

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
		{ ouro::data(string), oDLG_LARGELABEL, IDMESSAGE, rString, true, true, true },
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

	DLGAssert Dialog(hIcon, _DialogTimeoutMS, _IgnoreDisableTimeoutMS);
	INT_PTR int_ptr = DialogBoxIndirectParam(GetModuleHandle(0), lpDlgTemplate, GetDesktopWindow(), DLGAssert::StaticWndProc, (LPARAM)&Dialog);
	
	oDlgDeleteTemplate(lpDlgTemplate);

	if (int_ptr == -1)
	{
		std::string msg = ouro::windows::category().message(GetLastError());
		oTRACE("DialogBoxIndirectParam failed. %s\n", msg.c_str());
		__debugbreak(); // debug msgbox called from oASSERTs, so don't recurse into it
	}

	DeleteObject(hIcon);

	oMSGBOX_RESULT result = AsAction(static_cast<WORD>(int_ptr));
	return result;
}

UINT AsFlags(oMSGBOX_TYPE type)
{
	switch (type)
	{
		default:
		case oMSGBOX_INFO: return MB_ICONINFORMATION|MB_OK;
		case oMSGBOX_WARN: return MB_ICONWARNING|MB_OK;
		case oMSGBOX_YESNO: return MB_ICONQUESTION|MB_YESNO|MB_DEFBUTTON1;
		case oMSGBOX_ERR: return MB_ICONERROR|MB_OK|MB_TASKMODAL;
		case oMSGBOX_DEBUG: return MB_ICONERROR|MB_ABORTRETRYIGNORE;
	}
}

oMSGBOX_RESULT GetResult(int messageBoxResult)
{
	switch (messageBoxResult)
	{
		default:
		case IDOK: return oMSGBOX_YES;
		case IDABORT: return oMSGBOX_ABORT;
		case IDIGNORE: return oMSGBOX_CONTINUE; 
		case IDRETRY: return oMSGBOX_BREAK;
		case IDYES: return oMSGBOX_YES;
		case IDNO: return oMSGBOX_NO;
	}
}

namespace WFNWCV
{
	static thread_local HWND hParent = nullptr;
	static thread_local WNDPROC OrigWndProc = nullptr;
	static thread_local HHOOK Hook = nullptr;

	LRESULT CALLBACK WndProc(HWND _hWnd, UINT _uMsg, WPARAM _wParam, LPARAM _lParam)
	{
		LRESULT result = CallWindowProc(OrigWndProc, _hWnd, _uMsg, _wParam, _lParam);

		switch (_uMsg)
		{
			case WM_ACTIVATE:
			{
				if (hParent)
				{
					RECT rParent, rChild;
					GetWindowRect(_hWnd, &rChild);
					GetWindowRect(hParent, &rParent);
					POINT p = {0,0};
					ClientToScreen(hParent, &p);
					int2 MsgBoxPosition = int2(p.x, p.y) + (oWinRectSize(rParent) - oWinRectSize(rChild)) / 2;
					SetWindowPos(_hWnd, nullptr, MsgBoxPosition.x, MsgBoxPosition.y, 0, 0, SWP_NOZORDER|SWP_NOSIZE);
				}
				break;
			}

			case WM_SETCURSOR:
			{
				oWinSetCursor(_hWnd, LoadCursor(nullptr, IDC_ARROW));
				oWinCursorSetVisible();
				break;
			}

			case WM_NCDESTROY:
			{
				oVB(UnhookWindowsHookEx(Hook));
				hParent = nullptr;
				Hook = nullptr;
				OrigWndProc = nullptr;
				break;
			}
		}

		return result;
	}

	LRESULT Install(int _nCode, WPARAM _wParam, LPARAM _lParam)
	{
		CWPSTRUCT& cwp = *(CWPSTRUCT*)_lParam;
		if (_nCode == HC_ACTION && cwp.message == WM_INITDIALOG && !OrigWndProc)
			OrigWndProc = (WNDPROC)SetWindowLongPtr(cwp.hwnd, GWLP_WNDPROC, (LONG_PTR)WndProc);
		return CallNextHookEx(WFNWCV::Hook, _nCode, _wParam, _lParam);
	}

} // WFNWCV

void oSetupNextMessageBoxWndProc(HWND _hParent)
{
	WFNWCV::hParent = _hParent;
	if (WFNWCV::hParent)
		WFNWCV::Hook = SetWindowsHookEx(WH_CALLWNDPROC, (HOOKPROC)WFNWCV::Install, nullptr,  GetWindowThreadProcessId(_hParent, nullptr));
}

oMSGBOX_RESULT oMsgBoxV(const oMSGBOX_DESC& _Desc, const char* _Format, va_list _Args)
{
	std::vector<char> msg(oKB(128));
	ouro::vsnprintf(ouro::data(msg), ouro::size(msg), _Format, _Args);
	oMSGBOX_RESULT result = oMSGBOX_YES;
	HICON hIcon = nullptr;

	HWND hWnd = (HWND)_Desc.hParent;
	oStd::thread::id ThreadID;

	if (!hWnd)
		oWinGetProcessTopWindowAndThread(ouro::this_process::get_id(), &hWnd, &ThreadID);

	switch (_Desc.Type)
	{
		case oMSGBOX_DEBUG:
			result = AssertDialog(_Desc.Type, _Desc.Title, ouro::data(msg), _Desc.TimeoutMS, 0);
			break;
		
		case oMSGBOX_NOTIFY_INFO:
		case oMSGBOX_NOTIFY_WARN:
		case oMSGBOX_NOTIFY_ERR:
			hIcon = LoadIcon(_Desc.Type);
			// pass thru

		case oMSGBOX_NOTIFY:
			ouro::notification_area::show_message((oGUI_WINDOW)hWnd, 0, (oGUI_ICON)hIcon, __max(2000, _Desc.TimeoutMS), _Desc.Title, ouro::data(msg));
			result = oMSGBOX_CONTINUE;
			break;

		default:
		{
			oSetupNextMessageBoxWndProc(hWnd);
			result = GetResult(MessageBoxTimeout(hWnd, ouro::data(msg), _Desc.Title, AsFlags(_Desc.Type), 0, _Desc.TimeoutMS));
			break;
		}
	}

	return result;
}
