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
#include <oPlatform/oProgressBar.h>
#include <oBasis/oDispatchQueuePrivate.h>
#include <oBasis/oError.h>
#include <oBasis/oFixedString.h>
#include <oBasis/oInterface.h>
#include <oBasis/oMutex.h>
#include <oBasis/oRef.h>
#include <oBasis/oRefCount.h>
#include <oBasis/oString.h>
#include <oPlatform/oInterprocessEvent.h>
#include <oPlatform/oSingleton.h>
#include <oPlatform/Windows/oWinWindowing.h>

// Careful, DM_GETDEFID DM_SETDEFID DM_REPOSITION use WM_USER values
static const UINT oWM_SETMARQUEE = WM_USER+600; // _wParam is TRUE for marquee, FALSE for regular progress bar
static const UINT oWM_SHOWSTOP = WM_USER+601; // _wParam is TRUE for showing the stop button, FALSE for hiding it
static const UINT oWM_SETSTOP = WM_USER+602;
static const UINT oWM_SETPOS = WM_USER+603; // _wParam is an INT [0,100] for percentage complete
static const UINT oWM_ADDPOS = WM_USER+604;
static const LONG InsetX = 7;
static const LONG InsetY = 5;

static bool oSetDlgItemTextTruncated(HWND _hDlg, int _nIDDlgItem, LPCSTR _lpString)
{
	char buf[256];
	oStrcpy(buf, _lpString);
	oAddTruncationElipse(buf);

	bool result = !!SetDlgItemText(_hDlg, _nIDDlgItem, buf);
	if (!result)
		oWinSetLastError();
	return result;
};

struct oWinProgressBar : oProgressBar
{
	oDEFINE_REFCOUNT_INTERFACE(RefCount);
	oDEFINE_NOOP_QUERYINTERFACE();

	oWinProgressBar(const DESC& _Desc, void* _WindowNativeHandle, bool* _pSuccess);
	~oWinProgressBar();

	// _____________________________________________________________________________
	// Public API

	void GetDesc(DESC* _pDesc) threadsafe override;
	DESC* Map() threadsafe override;
	void Unmap() threadsafe override;
	void SetTitle(const char* _Title) threadsafe override;
	void SetText(const char* _Text, const char* _Subtext = nullptr) threadsafe override;
	void SetPercentage(int _Percent) threadsafe override;
	void AddPercentage(int _Percentage) threadsafe override;
	bool Wait(unsigned int _TimeoutMS = oInfiniteWait) threadsafe override;

protected:
	enum ITEM
	{
		STOP_BUTTON,
		PERCENTAGE,
		TEXT,
		SUBTEXT,
		PROGRESSBAR,
	};

	LPDLGTEMPLATE PB_NewTemplate(char* _Title, const char* _Text, const char* _Subtext, bool _AlwaysOnTop, RECT* _pOutProgressBarRect, RECT* _pOutMarqueeBarRect);
	HWND PB_NewControl(HWND _hDialog, const RECT& _Rect, bool _Visible, bool _Marquee, short _ProgressMax = 100);

	oDECLARE_DLGPROC(, WndProc);
	oDECLARE_DLGPROC(static, StaticWndProc);

	void Initialize(bool* _pSuccess);
	void Run();
	void Deinitialize();
	void WTSetText(bool _TextValid, oStringL _Text, bool _SubtextValid, oStringL _Subtext); // by copy so oBIND retains string in Enqueue
	void WTSetTitle(oStringL _Title); // by copy so oBIND retains string in Enqueue
	void SetDesc(DESC _Desc); // by copy so oBIND retains string in Enqueue

	DESC Desc;
	DESC PendingDesc;
	
	LPDLGTEMPLATE lpDlgTemplate; // needed while dialog is alive
	RECT INIT_rProgressBar;
	RECT INIT_rMarqueeBar;
	HWND INIT_hParent;
	HWND hDialog;
	HWND hProgressBar;
	HWND hMarqueeBar;
	oRefCount RefCount;
	oSharedMutex DescMutex;
	oInterprocessEvent Complete;
	oInterprocessEvent Stopped;

	oRef<threadsafe oDispatchQueuePrivate> MessageQueue;
};

oDEFINE_DLGPROC(oWinProgressBar, StaticWndProc);

bool oProgressBarCreate(const oProgressBar::DESC& _Desc, void* _WindowNativeHandle, threadsafe oProgressBar** _ppProgressBar)
{
	if (!_ppProgressBar)
	{
		oErrorSetLast(oERROR_INVALID_PARAMETER);
		return false;
	}

	bool success = false;
	oCONSTRUCT(_ppProgressBar, oWinProgressBar(_Desc, _WindowNativeHandle, &success));
	return success;
}

oWinProgressBar::oWinProgressBar(const DESC& _Desc, void* _WindowNativeHandle, bool* _pSuccess)
	: Desc(_Desc)
	, PendingDesc(_Desc)
	, INIT_hParent((HWND)_WindowNativeHandle)
	, lpDlgTemplate(nullptr)
	, hDialog(nullptr)
	, hProgressBar(nullptr)
	, hMarqueeBar(nullptr)
{
	*_pSuccess = oDispatchQueueCreatePrivate("oWinProgressBar Message Thread", 2000, &MessageQueue);
	if (*_pSuccess)
	{
		MessageQueue->Dispatch(&oWinProgressBar::Initialize, this, _pSuccess);
		MessageQueue->Flush();
		MessageQueue->Dispatch(&oWinProgressBar::Run, this);
	}
}

oWinProgressBar::~oWinProgressBar()
{
	MessageQueue->Dispatch(&oWinProgressBar::Deinitialize, this);
	oWinWake(hDialog);
	MessageQueue->Join();
}

void oWinProgressBar::Initialize(bool* _pSuccess)
{
	*_pSuccess = false;
	lpDlgTemplate = PB_NewTemplate("", "", "", Desc.AlwaysOnTop, &INIT_rProgressBar, &INIT_rMarqueeBar);
	if (lpDlgTemplate)
	{
		hDialog = CreateDialogIndirectParam(GetModuleHandle(0), lpDlgTemplate, INIT_hParent, oWinProgressBar::StaticWndProc, (LPARAM)this);
		SetDesc(Desc);
		*_pSuccess = true;
	}
}

void oWinProgressBar::Deinitialize()
{
	if (hDialog)
	{
		DestroyWindow(hDialog);
		hDialog = nullptr;
	}

	if (lpDlgTemplate)
	{
		oDlgDeleteTemplate(lpDlgTemplate);
		lpDlgTemplate = nullptr;
	}
}

LPDLGTEMPLATE oWinProgressBar::PB_NewTemplate(char* _Title, const char* _Text, const char* _Subtext, bool _AlwaysOnTop, RECT* _pOutProgressBarRect, RECT* _pOutMarqueeBarRect)
{
	const RECT rDialog = { 0, 0, 197, 65 };
	const RECT rStop = { 140, 45, rDialog.right - InsetX, rDialog.bottom - InsetY };
	const RECT rPercentage = { 170, 30, 192, 40 };
	const RECT rText = { InsetX, 3 + InsetY, rDialog.right - InsetX, 13 + InsetY };
	const RECT rSubtext = { InsetX, 44, 140, 54 };
	const RECT rProgressBarInit = { InsetX, 27, 167, 41 };
	const RECT rMarqueeBarInit = { InsetX, 27, rDialog.right - InsetX, 41 };
	*_pOutProgressBarRect = rProgressBarInit;
	*_pOutMarqueeBarRect = rMarqueeBarInit;

	const oWINDOWS_DIALOG_ITEM items[] = 
	{
		{ "&Stop", oDLG_BUTTON, STOP_BUTTON, rStop, true, true, true },
		{ "", oDLG_LABEL_LEFT_ALIGNED, PERCENTAGE, rPercentage, true, true, false },
		{ oSAFESTR(_Text), oDLG_LABEL_CENTERED, TEXT, rText, true, true, false },
		{ oSAFESTR(_Subtext), oDLG_LABEL_LEFT_ALIGNED, SUBTEXT, rSubtext, true, true, false },
	};

	oWINDOWS_DIALOG_DESC dlg;
	dlg.Font = "Tahoma";
	dlg.Caption = oSAFESTR(_Title);
	dlg.pItems = items;
	dlg.NumItems = oCOUNTOF(items);
	dlg.FontPointSize = 8;
	dlg.Rect = rDialog;
	dlg.Center = true;
	dlg.SetForeground = true;
	dlg.Enabled = true;
	dlg.Visible = false; // always show later (once full init is finished)
	dlg.AlwaysOnTop = false;

	return oDlgNewTemplate(dlg);
}

HWND oWinProgressBar::PB_NewControl(HWND _hDialog, const RECT& _Rect, bool _Visible, bool _Marquee, short _ProgressMax)
{
	DWORD dwStyle = WS_CHILD;
	if (_Visible) dwStyle |= WS_VISIBLE;
	if (_Marquee) dwStyle |= PBS_MARQUEE;

	RECT r = _Rect;
	MapDialogRect(_hDialog, &r);
	HWND hControl = CreateWindowEx(
		0
		, PROGRESS_CLASS
		, "OOOii.ProgressBarControl"
		, dwStyle
		, r.left
		, r.top
		, r.right - r.left
		, r.bottom - r.top
		,	_hDialog
		, 0
		, 0
		, nullptr);

	SendMessage(hControl, PBM_SETRANGE, 0, MAKELPARAM(0, _ProgressMax));

	if (_Marquee)
		SendMessage(hControl, PBM_SETMARQUEE, 1, 0);

	return hControl;
}

void oWinProgressBar::GetDesc(DESC* _pDesc) threadsafe
{
	oSharedLock<oSharedMutex> lock(DescMutex);
	*_pDesc = thread_cast<DESC&>(Desc);
}

oProgressBar::DESC* oWinProgressBar::Map() threadsafe
{
	DescMutex.lock();
	return thread_cast<DESC*>(&PendingDesc);
}

void oWinProgressBar::Unmap() threadsafe
{
	DescMutex.unlock();
	MessageQueue->Dispatch(&oWinProgressBar::SetDesc, thread_cast<oWinProgressBar*>(this), thread_cast<DESC&>(PendingDesc));
	oWinWake(hDialog);
}

void oWinProgressBar::WTSetTitle(oStringL _Title)
{
	oWinSetText(hDialog, _Title);
}

void oWinProgressBar::SetTitle(const char* _Title) threadsafe
{
	MessageQueue->Dispatch(&oWinProgressBar::SetTitle, thread_cast<oWinProgressBar*>(this), oStringL(oSAFESTR(_Title)));
	oWinWake(hDialog);
}

void oWinProgressBar::WTSetText(bool _TextValid, oStringL _Text, bool _SubtextValid, oStringL _Subtext)
{
	if (_TextValid)
		oSetDlgItemTextTruncated(hDialog, TEXT, _Text);
	if (_SubtextValid)
		oSetDlgItemTextTruncated(hDialog, SUBTEXT, _Subtext);
}

void oWinProgressBar::SetText(const char* _Text, const char* _Subtext) threadsafe
{
	if (_Text || _Subtext)
	{
		MessageQueue->Dispatch(&oWinProgressBar::WTSetText, thread_cast<oWinProgressBar*>(this), !!_Text, oStringL(oSAFESTRN(_Text)), !!_Subtext, oStringL(oSAFESTRN(_Subtext)));
		oWinWake(hDialog);
	}
}

void oWinProgressBar::SetPercentage(int _Percentage) threadsafe
{
	oVB(PostMessage(hDialog, oWM_SETPOS, _Percentage, 0));
}

void oWinProgressBar::AddPercentage(int _Percentage) threadsafe
{
	oVB(PostMessage(hDialog, oWM_ADDPOS, _Percentage, 0));
}

bool oWinProgressBar::Wait(unsigned int _TimeoutMS) threadsafe
{
	size_t bStopped = 0;
	threadsafe oInterprocessEvent* pEvents[] = { &Complete, &Stopped };
	bool result = oInterprocessEvent::WaitMultiple(pEvents, oCOUNTOF(pEvents), &bStopped, _TimeoutMS);
	if (bStopped)
	{
		oErrorSetLast(oERROR_CANCELED);
		return false;
	}
	return result;
}

void oWinProgressBar::SetDesc(DESC _Desc)
{
	SendMessage(hDialog, oWM_SHOWSTOP, _Desc.ShowStopButton, 0);
	oWinSetAlwaysOnTop(hDialog, _Desc.AlwaysOnTop);
	SendMessage(hDialog, oWM_SETMARQUEE, _Desc.UnknownProgress, 0);
	SendMessage(hDialog, oWM_SETSTOP, _Desc.Stopped, 0);
	ShowWindow(hDialog, _Desc.Show ? SW_SHOW : SW_HIDE);
	if (_Desc.Show) oWinSetFocus(hDialog);
	oLockGuard<oSharedMutex> lock(DescMutex);
	Desc = _Desc;
}

void oWinProgressBar::Run()
{
	if (hDialog)
	{
		MSG msg;
		if (GetMessage(&msg, hDialog, 0, 0) <= 0) // either an error or WM_QUIT
		{
			// close
		}
		else
		{
			if (!IsDialogMessage(hDialog, &msg))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}

		MessageQueue->Dispatch(&oWinProgressBar::Run, this);
	}
}

INT_PTR oWinProgressBar::WndProc(HWND _hWnd, UINT _uMsg, WPARAM _wParam, LPARAM _lParam)
{
	switch (_uMsg)
	{
		case WM_INITDIALOG:
		{
			hDialog = _hWnd;
			// Mark the stop button with the default style
			SendMessage(hDialog, DM_SETDEFID, STOP_BUTTON, 0);
			hProgressBar = PB_NewControl(hDialog, INIT_rProgressBar, true, false);
			hMarqueeBar = PB_NewControl(hDialog, INIT_rMarqueeBar, false, true);
			return false;
		}
			
		case oWM_SETMARQUEE:
		{
			HWND hHide = _wParam ? hProgressBar : hMarqueeBar;
			HWND hShow = _wParam ? hMarqueeBar : hProgressBar;
			HWND hPercentage = GetDlgItem(hDialog, PERCENTAGE);
			ShowWindow(hPercentage, hHide == hProgressBar ? SW_HIDE : SW_SHOW);
			UpdateWindow(hPercentage);
			ShowWindow(hHide, SW_HIDE);
			ShowWindow(hShow, SW_SHOW);
			UpdateWindow(hShow);
			return false;
		}

		case oWM_SHOWSTOP: 
		{
			HWND hStop = GetDlgItem(hDialog, STOP_BUTTON);
			ShowWindow(hStop, _wParam ? SW_SHOW : SW_HIDE);
			UpdateWindow(hStop);
			return false;
		}
			
		case oWM_SETPOS:
		{
			UINT p = (UINT)__max(0, __min(100, (UINT)_wParam));
			SendMessage(hProgressBar, PBM_SETPOS, p, 0);
			char buf[16];
			oPrintf(buf, "%u%%", p);
			oAddTruncationElipse(buf);
			oVB(SetDlgItemText(hDialog, PERCENTAGE, buf));
			if (p == 100) Complete.Set();
			else Complete.Reset();
			return false;
		}

		case oWM_ADDPOS:
		{
			UINT Pos = (UINT)SendMessage(hProgressBar, PBM_GETPOS, 0, 0);
			oV((HRESULT)SendMessage(_hWnd, oWM_SETPOS, Pos + _wParam, 0));
			return false;
		}

		case oWM_SETSTOP:
		{
			if (_wParam) Stopped.Set();
			else Stopped.Reset();
			SendMessage(hProgressBar, PBM_SETSTATE, _wParam ? PBST_ERROR : PBST_NORMAL, 0);
			if (_wParam) oSetDlgItemTextTruncated(hDialog, SUBTEXT, "Stopped...");
			oLockGuard<oSharedMutex> lock(DescMutex);
			Desc.Stopped = !!_wParam;
			return false;
		}

		case WM_CLOSE:
			DestroyWindow(_hWnd);
			break;

		case WM_COMMAND:
			switch (_wParam)
			{
				case STOP_BUTTON:
					SendMessage(_hWnd, oWM_SETSTOP, TRUE, 0);
					return true;
				
				default:
					break;
			}

		default:
			break;
	}

	return false;
}
