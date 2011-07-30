// $(header)

#include <oooii/oErrno.h>
#include <oooii/oEvent.h>
#include <oooii/oInterface.h>
#include <oooii/oMutex.h>
#include <oooii/oRef.h>
#include <oooii/oRefCount.h>
#include <oooii/oSingleton.h>
#include <oooii/oString.h>
#include <oooii/oThread.h>
#include <oooii/oWindows.h>

static const UINT oWM_SETMARQUEE = WM_USER+600; // Careful, DM_GETDEFID DM_SETDEFID DM_REPOSITION use WM_USER values
static const LONG InsetX = 7;
static const LONG InsetY = 5;

struct oWinProgressBar : oThread::Proc
{
	oDEFINE_REFCOUNT_INTERFACE(RefCount);
	oDEFINE_NOOP_QUERYINTERFACE();

	enum ITEM
	{
		STOP_BUTTON,
		PERCENTAGE,
		TEXT,
		SUBTEXT,
		PROGRESSBAR,
	};

	struct DESC
	{
		DESC()
			: Percent(0)
			, Marquee(false)
			, Visible(false)
			, AlwaysOnTop(true)
		{}

		unsigned int Percent; // [0,100]
		bool Marquee;
		bool Visible;
		bool AlwaysOnTop;
	};

	oWinProgressBar(const DESC& _Desc, void* _ParentNativeHandle, bool* _pSuccess);
	~oWinProgressBar();

	static bool Create(const DESC& _Desc, void* _ParentNativeHandle, oWinProgressBar** _ppProgressBar);

	void RunIteration() override;
	bool OnBegin() override;
	void OnEnd() override;

	// nullptr means "don't touch prior value"
	void SetText(const char* _Text, const char* _Subtext = nullptr, const char* _Title = nullptr) threadsafe;

	void SetPercent(unsigned int _Percent) threadsafe;
	void AddPercent(unsigned int _Percent) threadsafe;

	void SetPercentNOLOCK(unsigned int _Percent);

	void SetVisible(bool _Visible) threadsafe;
	void SetAlwaysOnTop(bool _AlwaysOnTop) threadsafe;
	void SetMarquee(bool _Marquee) threadsafe;
	void SetStopped(bool _Stopped) threadsafe;

	inline bool WaitReady(unsigned int _TimeoutMS = oINFINITE_WAIT) threadsafe { return Initialized.Wait(_TimeoutMS); }

	// "Complete" means either we got to 100% (returns true), or someone stopped 
	// it or it timed out. Check oGetLastError for ECANCELED or ETIMEDOUT.
	bool WaitComplete(unsigned int _TimeoutMS = oINFINITE_WAIT) threadsafe;

	void GetDesc(DESC* _pDesc) const threadsafe { oRWMutex::ScopedLockRead lock(Mutex); *_pDesc = thread_cast<DESC&>(Desc); /* Safe because of mutex */ }

protected:
	LPDLGTEMPLATE PB_NewTemplate(char* _Title, const char* _Text, const char* _Subtext, bool _AlwaysOnTop, RECT* _pOutProgressBarRect, RECT* _pOutMarqueeBarRect);
	HWND PB_NewControl(HWND _hDialog, const RECT& _Rect, bool _Visible, bool _Marquee, short _ProgressMax = 100);

	static INT_PTR CALLBACK StaticDlgProc(HWND _hWnd, UINT _uMsg, WPARAM _wParam, LPARAM _lParam);
	INT_PTR DlgProc(HWND _hWnd, UINT _uMsg, WPARAM _wParam, LPARAM _lParam);

	INT_PTR OnInitDialog(HWND _hDialog);
	INT_PTR OnSetMarquee(bool _SetMarquee);

	bool PostSetDlgItemText(HWND _hDlg, int _nIDDlgItem, LPCTSTR _lpString) threadsafe;

	bool PostProgressBarMessage(UINT _Msg, WPARAM _wParam, LPARAM _lParam) threadsafe;
	bool PostMarqueeBarMessage(UINT _Msg, WPARAM _wParam, LPARAM _lParam) threadsafe;

	DESC Desc;
	LPDLGTEMPLATE lpDlgTemplate; // needed while dialog is alive
	RECT rProgressBar;
	RECT rMarqueeBar;
	HWND hParent;
	HWND hDialog;
	HWND hProgressBar;
	HWND hMarqueeBar;

	oRefCount RefCount;
	oRWMutex Mutex;
	oRWMutex PBMutex;
	oEvent Initialized;
	oEvent Complete;
	oEvent Stopped;
};

bool oWinProgressBar::Create(const DESC& _Desc, void* _ParentNativeHandle, oWinProgressBar** _ppProgressBar)
{
	if (!_ppProgressBar)
	{
		oSetLastError(EINVAL);
		return false;
	}

	bool success = false;
	oCONSTRUCT(_ppProgressBar, oWinProgressBar(_Desc, _ParentNativeHandle, &success));
	return success;
}

oWinProgressBar::oWinProgressBar(const DESC& _Desc, void* _ParentNativeHandle, bool* _pSuccess)
	: Desc(_Desc)
	, hParent((HWND)_ParentNativeHandle)
	, lpDlgTemplate(nullptr)
	, hDialog(0)
	, hProgressBar(0)
	, hMarqueeBar(0)
{
	*_pSuccess = true;
}

oWinProgressBar::~oWinProgressBar()
{
	OnEnd();
}

bool oWinProgressBar::OnBegin()
{
	lpDlgTemplate = PB_NewTemplate("Test", "Test1", "Test2", Desc.AlwaysOnTop, &rProgressBar, &rMarqueeBar);
	if (lpDlgTemplate)
	{
		hDialog = CreateDialogIndirectParam(
			GetModuleHandle(0)
			, lpDlgTemplate
			, hParent
			, StaticDlgProc, (LPARAM)this);

		Initialized.Set();
		return true;
	}

	return false;
}

void oWinProgressBar::OnEnd()
{
	if (hDialog)
	{
		DestroyWindow(hDialog);
		hDialog = 0;
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
	dlg.AlwaysOnTop = false;//_AlwaysOnTop;

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

void oWinProgressBar::RunIteration()
{
	MSG msg;
	if (GetMessage(&msg, hDialog, 0, 0) <= 0) //either an error or WM_QUIT
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
}

void oWinProgressBar::SetPercentNOLOCK(unsigned int _Percent)
{
	Desc.Percent = __min(100, _Percent);
	char buf[16];
	sprintf_s(buf, "%u%%", Desc.Percent);
	PostProgressBarMessage(PBM_SETPOS, Desc.Percent, 0);
	PostSetDlgItemText(hDialog, PERCENTAGE, buf);

	if (Desc.Percent == 100)
		Complete.Set();
	else
		Complete.Reset();
}

void oWinProgressBar::SetText(const char* _Text, const char* _Subtext, const char* _Title) threadsafe
{
	if (_Text)
		PostSetDlgItemText(hDialog, TEXT, _Text);

	if (_Subtext)
		PostSetDlgItemText(hDialog, SUBTEXT, _Subtext);

	if (_Title)
		oSetTitle(hDialog, _Title); // threadsafe?
}

void oWinProgressBar::SetPercent(unsigned int _Percent) threadsafe
{
	oRWMutex::ScopedLock lock(Mutex);
	thread_cast<oWinProgressBar*>(this)->SetPercentNOLOCK(_Percent);
}

void oWinProgressBar::AddPercent(unsigned int _Percent) threadsafe
{
	oRWMutex::ScopedLock lock(Mutex);
	
	// @oooii-tony: DANGER might not be threadsafe because of the SendMessage v. PostMessage
	UINT Pos = (UINT)SendMessage(hProgressBar, PBM_GETPOS, 0, 0);
	thread_cast<oWinProgressBar*>(this)->SetPercentNOLOCK(Pos + _Percent);
}

void oWinProgressBar::SetVisible(bool _Visible) threadsafe
{
	oRWMutex::ScopedLock lock(Mutex);
	Desc.Visible = _Visible;
	ShowWindow(hDialog, SW_HIDE);
	ShowWindow(hDialog, Desc.Visible ? SW_SHOW : SW_HIDE);
	UpdateWindow(hDialog);
}

void oWinProgressBar::SetAlwaysOnTop(bool _AlwaysOnTop) threadsafe
{
	oRWMutex::ScopedLock lock(Mutex);
	Desc.AlwaysOnTop = _AlwaysOnTop;
	oSetAlwaysOnTop(hDialog, Desc.AlwaysOnTop);
}

void oWinProgressBar::SetMarquee(bool _Marquee) threadsafe
{
	oRWMutex::ScopedLock lock(Mutex);
	Desc.Marquee = _Marquee;
	PostMessage(hDialog, oWM_SETMARQUEE, WPARAM(_Marquee ? 1 : 0), 0);
}

void oWinProgressBar::SetStopped(bool _Stopped) threadsafe
{
	PostProgressBarMessage(PBM_SETSTATE, _Stopped ? PBST_ERROR : PBST_NORMAL, 0);
	SetText(nullptr, "Stopped...");

	if (_Stopped)
		Stopped.Set();
	else
		Stopped.Reset();
}

bool oWinProgressBar::WaitComplete(unsigned int _TimeoutMS) threadsafe
{
	size_t bStopped = 0;
	threadsafe oEvent* pEvents[] = { &Complete, &Stopped };
	bool result = oEvent::WaitMultiple(pEvents, oCOUNTOF(pEvents), &bStopped, _TimeoutMS);
	if (bStopped)
		oSetLastError(ECANCELED);
	return result;
}

INT_PTR oWinProgressBar::StaticDlgProc(HWND _hWnd, UINT _uMsg, WPARAM _wParam, LPARAM _lParam)
{
	oWinProgressBar* pThis = (oWinProgressBar*)oGetWindowContext(_hWnd, _uMsg, _wParam, _lParam);
	return pThis ? pThis->DlgProc(_hWnd, _uMsg, _wParam, _lParam) : false;
}

INT_PTR oWinProgressBar::DlgProc(HWND _hWnd, UINT _uMsg, WPARAM _wParam, LPARAM _lParam)
{
	switch (_uMsg)
	{
		case WM_INITDIALOG:
			return OnInitDialog(_hWnd);

		case oWM_SETMARQUEE:
			return OnSetMarquee(!!_wParam);

		case WM_DESTROY:
			DestroyWindow(_hWnd);
			break;

		case WM_COMMAND:
			switch (_wParam)
			{
				case oWinProgressBar::STOP_BUTTON:
					SetStopped(true);
					return true;
				
				default:
					break;
			}

		default:
			break;
	}

	return false;
}

INT_PTR oWinProgressBar::OnInitDialog(HWND _hDialog)
{
	hDialog = _hDialog;

	// Mark the stop button with the default style
	SendMessage(hDialog, DM_SETDEFID, 0, 0);
	hProgressBar = PB_NewControl(hDialog, rProgressBar, true, false);
	hMarqueeBar = PB_NewControl(hDialog, rMarqueeBar, false, true);
	Initialized.Set();
	return false;
}

INT_PTR oWinProgressBar::OnSetMarquee(bool _SetMarquee)
{
	HWND hHide, hShow;
	if (_SetMarquee)
	{
		hHide = hProgressBar;
		hShow = hMarqueeBar;
		SetPercentNOLOCK(0);
	}

	else
	{
		hHide = hMarqueeBar;
		hShow = hProgressBar;
	}

	HWND hPercentage = GetDlgItem(hDialog, PERCENTAGE);
	ShowWindow(hPercentage, _SetMarquee ? SW_HIDE : SW_SHOW);

	if (!_SetMarquee)
		UpdateWindow(hPercentage);

	ShowWindow(hHide, SW_HIDE);
	ShowWindow(hShow, SW_SHOWNORMAL);
	UpdateWindow(hShow);
	return false;
}

bool oWinProgressBar::PostSetDlgItemText(HWND _hDlg, int _nIDDlgItem, LPCTSTR _lpString) threadsafe
{
	// @oooii-tony: TODO: Make this threadsafe. Eric addressed this type of thing
	// by PostMessage'ing a user message and handling it in the message pump. That
	// means for strings we'd need to allocate persistent memory and free it in the
	// pump. Just a PITA at the moment, so punt for now.

	char buf[256];
	strcpy_s(buf, _lpString);
	oAddTruncationElipse(buf);

	bool result = !!SetDlgItemText(_hDlg, _nIDDlgItem, buf);
	if (!result)
		oWinSetLastError();
	return result;
}

bool oWinProgressBar::PostProgressBarMessage(UINT _Msg, WPARAM _wParam, LPARAM _lParam) threadsafe
{
	bool result = false;
	{
		oRWMutex::ScopedLock lock(PBMutex);
		result = !!SendMessage(hProgressBar, _Msg, _wParam, _lParam);
	}
	if (!result)
		oWinSetLastError();
	return result;
};

bool oWinProgressBar::PostMarqueeBarMessage(UINT _Msg, WPARAM _wParam, LPARAM _lParam) threadsafe
{
	bool result = false;
	{
		oRWMutex::ScopedLock lock(PBMutex);
		result = !!SendMessage(hProgressBar, _Msg, _wParam, _lParam);
	}
	if (!result)
		oWinSetLastError();
	return result;
};

struct oProgressBarContext : oProcessSingleton<oProgressBarContext>
{
	oProgressBarContext();
	oRef<oWinProgressBar> ProgressBar;
	oRef<threadsafe oThread> Thread;
};

oProgressBarContext::oProgressBarContext()
{
	oWinProgressBar::DESC d;
	oVERIFY(oWinProgressBar::Create(d, nullptr, &ProgressBar));
	oVERIFY(oThread::Create("oWinProgressBar Thread", oKB(64), false, ProgressBar, &Thread));
}

#define WPB() threadsafe oWinProgressBar* PB = oProgressBarContext::Singleton()->ProgressBar; oVERIFY(PB->WaitReady(20000))

void oProgressBarShow(const char* _Title, bool _Show, bool _AlwaysOnTop, bool _UnknownProgress)
{
	WPB();
	PB->SetText(nullptr, nullptr, _Title);
	PB->SetAlwaysOnTop(_AlwaysOnTop);
	PB->SetMarquee(_UnknownProgress);
	PB->SetVisible(_Show);
}

void oProgressBarSetText(const char* _Text, const char* _Subtext)
{
	WPB();
	PB->SetText(_Text, _Subtext);
}

void oProgressBarSetStopped(bool _Stopped)
{
	WPB();
	PB->SetStopped(_Stopped);
}

void oProgressBarSet(int _Percentage)
{
	WPB();
	PB->SetPercent(_Percentage);
}

void oProgressBarAdd(int _Percentage)
{
	WPB();
	PB->AddPercent(_Percentage);
}

bool oProgressBarWait(unsigned int _TimeoutMS)
{
	WPB();
	return PB->WaitComplete(_TimeoutMS);
}
