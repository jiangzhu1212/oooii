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
#include <oPlatform/oWindow.h>
#include <oConcurrency/event.h>
#include <oBasis/oLockThis.h>
#include <oConcurrency/backoff.h>
#include <oPlatform/oDisplay.h>
#include <oPlatform/oGUIMenu.h>
#include <oPlatform/oMsgBox.h>
#include <oPlatform/oProcessHeap.h>
#include <oPlatform/Windows/oGDI.h>
#include <oPlatform/Windows/oWinAsString.h>
#include <oPlatform/Windows/oWinCursor.h>
#include <oPlatform/Windows/oWinKey.h>
#include <oPlatform/Windows/oWinStatusBar.h>
#include <oPlatform/Windows/oWinWindowing.h>
#include <oPlatform/Windows/oWinRect.h>
#include "oWinMessagePump.h"
#include <windowsx.h>

using namespace oConcurrency;

//#define oUSE_RAW_INPUT

static const bool kForceDebug = false;

class oWinWindow : public oWindow
{
public:
	oWinWindow(const oWINDOW_INIT& _Init, bool* _pSuccess);
	~oWinWindow();
	oDECLARE_WNDPROC(oWinWindow);
	oDEFINE_REFCOUNT_INTERFACE(RefCount);

	bool QueryInterface(const oGUID& _InterfaceID, threadsafe void** _ppInterface) threadsafe override;

	// This should be issued to the thread that will handle the message pump.
	bool ConstructOnThread(const oWINDOW_INIT& _Init);

	oGUI_WINDOW GetNativeHandle() const threadsafe { return (oGUI_WINDOW)hWnd; }
	bool IsOpen() const threadsafe override;
	bool WaitUntilClosed(unsigned int _TimeoutMS = oInfiniteWait) const threadsafe override;
	bool WaitUntilOpaque(unsigned int _TimeoutMS = oInfiniteWait) const threadsafe override;
	bool Close() threadsafe override;
	bool IsWindowThread() const threadsafe override;
	bool HasFocus() const threadsafe override;
	void SetFocus() threadsafe override;
	void GetDesc(oGUI_WINDOW_DESC* _pDesc) const threadsafe override;
	bool Map(oGUI_WINDOW_DESC** _ppDesc) threadsafe override;
	void Unmap() threadsafe override;
	void SetDesc(const oGUI_WINDOW_CURSOR_DESC& _CursorDesc) threadsafe override;
	void GetDesc(oGUI_WINDOW_CURSOR_DESC* _pCursorDesc) const threadsafe override;
	char* GetTitle(char* _StrDestination, size_t _SizeofStrDestination) const threadsafe override { return SendMessageA(hWnd, WM_GETTEXT, (WPARAM)_SizeofStrDestination, (LPARAM)_StrDestination) ? _StrDestination : nullptr; }
	void SetTitleV(const char* _Format, va_list _Args) threadsafe override { oStd::lstring* pTitle = new oStd::lstring(); vsnprintf(*pTitle, _Format, _Args); Dispatch([=] { SendMessage(hWnd, WM_SETTEXT, 0, (LPARAM)pTitle); delete pTitle; }); }

	char* GetStatusText(char* _StrDestination, size_t _SizeofStrDestination, int _StatusSectionIndex) const threadsafe override;
	void SetStatusText(int _StatusSectionIndex, const char* _Format, va_list _Args) threadsafe override;

	void SetHotKeys(const oGUI_HOTKEY_DESC_NO_CTOR* _pHotKeys, size_t _NumHotKeys) threadsafe override;
	int GetHotKeys(oGUI_HOTKEY_DESC_NO_CTOR* _pHotKeys, size_t _MaxNumHotKeys) const threadsafe override;
	oStd::future<oRef<oImage>> CreateSnapshot(int _Frame = oInvalid,  bool _IncludeBorder = false) const threadsafe override;
	void Trigger(const oGUI_ACTION_DESC& _Action) threadsafe override;
	void Dispatch(const oTASK& _Task) threadsafe override;
	void SetTimer(uintptr_t _Context, unsigned int _RelativeTimeMS) threadsafe override;
	oDEFINE_CALLABLE_WRAPPERS(Dispatch, threadsafe, Dispatch);
	int HookActions(const oGUI_ACTION_HOOK& _Hook) threadsafe override { return oInt(SendMessage(hWnd, oWM_ACTION_HOOK, 0, (LPARAM)&_Hook)); }
	void UnhookActions(int _ActionHookID) threadsafe override { oVB(PostMessage(hWnd, oWM_ACTION_UNHOOK, 0, (LPARAM)_ActionHookID)); }
	int HookEvents(const oGUI_EVENT_HOOK& _Hook) threadsafe override { return oInt(SendMessage(hWnd, oWM_EVENT_HOOK, 0, (LPARAM)&_Hook)); }
	void UnhookEvents(int _EventHookID) threadsafe override { oVB(PostMessage(hWnd, oWM_EVENT_UNHOOK, 0, (LPARAM)_EventHookID)); }

protected:
	HWND hWnd;
	oGUI_MENU hMenu;
	HWND hStatusBar;
	HACCEL hAccel;
	HCURSOR hUserCursor;

	event Closed;
	double ShowTimestamp;
	shared_mutex DescMutex;
	shared_mutex CursorDescMutex;
	mutex PendingDescMutex;
	oGUI_WINDOW_DESC Desc;
	oGUI_WINDOW_DESC PendingDesc;
	oGUI_WINDOW_DESC PreFullscreenDesc;
	oGUI_WINDOW_CURSOR_DESC CursorDesc;
	oRefCount RefCount;
	std::vector<oGUI_ACTION_HOOK> ActionHooks;
	std::vector<oGUI_EVENT_HOOK> EventHooks;
	bool AltF1CursorVisible; // override set by Alt-F1
	int2 CursorClientPosAtMouseDown;
	oWINKEY_CONTROL_STATE ControlKeyState;

	void SetNumStatusItems(int* _pItemWidths, size_t _NumItems) threadsafe;

	// returns true if the opacity transition when a window is show is finished
	// or false if it's still going or the window is hidden.
	bool IsOpaque() const threadsafe;

	// This is threadsafe in the sense it can be called from a different thread,
	// but appropriate locking is required if the desc here has to do with 
	// map/unmap implementations.
	void SetDesc(const oGUI_WINDOW_DESC& _Desc, bool _Force = false) threadsafe;

	// Sets the cursor state according to current state of this oWinWindow 
	// including the Desc and AltF1CursorVisible. Because of how SetCapture() works, 
	// this needs to be called from more than one place, so that's why it's broken
	// out as a function.
	void WTSetCursor(bool _IsInClientArea);
	void WTOnSize(const int2& _NewSize, oGUI_WINDOW_STATE _NewState);
	void WTTriggerAction(const oGUI_ACTION_DESC& _Action);
	bool WTTriggerEvent(const oGUI_EVENT_DESC& _Event);

	// Main call for modifying window state as executed on windows thread
	void WTSetDesc(const oGUI_WINDOW_DESC& _NewDesc, bool _Force = false);
	void WTSetCursorDesc(const oGUI_WINDOW_CURSOR_DESC& _NewDesc, bool _Force = false);
};

#define oKEEP_FOCUS(_hWnd) ::SetTimer(_hWnd, 0x12300011, 1500, PeriodicallySetFocus)
static VOID CALLBACK PeriodicallySetFocus(HWND _hWnd, UINT _uMsg, UINT_PTR _idEvent, DWORD _dwTime)
{
	if (oWinGetState(_hWnd) == oGUI_WINDOW_FULLSCREEN_COOPERATIVE && oWinIsAlwaysOnTop(_hWnd))
	{
		oWinSetFocus(_hWnd);
		oKEEP_FOCUS(_hWnd);
	}
}

void oWinStatusBarInitialize(HWND _hStatusBar, const oGUI_WINDOW_DESC& _Desc)
{
	std::array<int, oCOUNTOF(_Desc.StatusWidths)> itemWidths;
	itemWidths.fill(oInvalid);
	size_t i = 0; 
	for (; i < oCOUNTOF(_Desc.StatusWidths); i++)
	{
		if (_Desc.StatusWidths[i] != oInvalid)
			itemWidths[i] = _Desc.StatusWidths[i];

		if (_Desc.StatusWidths[i] == oInvalid)
		{
			i++;
			break;
		}
	}

	oWinStatusBarSetNumItems(_hStatusBar, itemWidths.data(), i);
}

bool oWinWindow::ConstructOnThread(const oWINDOW_INIT& _Init)
{
	if (!oWinCreate(&hWnd, _Init.WinDesc.ClientPosition, _Init.WinDesc.ClientSize, StaticWndProc, this))
		return false; // pass through error

	if (oSTRVALID(_Init.WindowTitle))
		SendMessage(hWnd, WM_SETTEXT, 0, (LPARAM)_Init.WindowTitle);

	if (Desc.AllowTouch)
	{
		oWINDOWS_VERSION v = oGetWindowsVersion();

		#ifdef oWINDOWS_HAS_REGISTERTOUCHWINDOW
			if (v >= oWINDOWS_7)
				oVB(RegisterTouchWindow(hWnd, 0));
			else
		#endif
			return oErrorSetLast(std::errc::not_supported, "touch input not supported pre-Windows7");
	}

	// Set the initial position with alignment considerations
	POINT p = {0,0};
	HWND hParent = (HWND)_Init.WinDesc.hParent;
	if (!hParent)
	{
		hParent = (HWND)_Init.WinDesc.hOwner;
		ClientToScreen(hParent, &p); // if there is no parent-child relationship, then the coords are in desktop space, not local/client space
	}

	RECT rParent = oWinGetParentRect(hWnd, hParent);
	rParent = oWinRectTranslate(rParent, p);
	RECT rClient = oWinRectResolve(rParent, _Init.WinDesc.ClientPosition, _Init.WinDesc.ClientSize, _Init.InitialAlignment, !!_Init.WinDesc.hParent);

	if (!oWinSetStyle(hWnd, _Init.WinDesc.Style, _Init.WinDesc.ShowStatusBar, &rClient))
		return false; // pass through error

	#ifdef oUSE_RAW_INPUT
		RAWINPUTDEVICE RID[2] =
		{
			{ oUS_USAGE_PAGE_GENERIC_DESKTOP, oUS_USAGE_KEYBOARD, 0, nullptr },
			{ oUS_USAGE_PAGE_GENERIC_DESKTOP, oUS_USAGE_MOUSE, 0, nullptr },
		};
		oVB(RegisterRawInputDevices(RID, 2, sizeof(RAWINPUTDEVICE)));
	#endif

	return true;
}

oWinWindow::oWinWindow(const oWINDOW_INIT& _Init, bool* _pSuccess)
	: hWnd(nullptr)
	, hMenu(nullptr)
	, hStatusBar(nullptr)
	, hAccel(nullptr)
	, hUserCursor(nullptr)
	, ShowTimestamp(-1.0)
	, Desc(_Init.WinDesc)
	, PendingDesc(_Init.WinDesc)
	, PreFullscreenDesc(_Init.WinDesc)
	, AltF1CursorVisible(true)
	, CursorClientPosAtMouseDown(oDEFAULT, oDEFAULT)
{
	*_pSuccess = false;

	ActionHooks.reserve(8);
	EventHooks.reserve(8);

	// Never create a window immediately exclusive fullscreen.
	PreFullscreenDesc.State = oGUI_WINDOW_RESTORED;
	if (Desc.State == oGUI_WINDOW_FULLSCREEN_EXCLUSIVE)
	{
		Desc.State = oGUI_WINDOW_RESTORED;
		PendingDesc.State = oGUI_WINDOW_RESTORED;
	}

	// oWinCreate starts off as a top-level window, so represent that here
	PreFullscreenDesc.hParent = Desc.hParent = nullptr;

	// Attach the hook now so that oGUI_CREATING can be hooked
	if (_Init.EventHook)
		oStd::sparse_set(EventHooks, _Init.EventHook);

	if (_Init.ActionHook)
		oStd::sparse_set(ActionHooks, _Init.ActionHook);

	oStd::packaged_task<bool(void)> task(oBIND(&oWinWindow::ConstructOnThread, this, oBINDREF(_Init)));

	if (!oWinMessagePump::Singleton()->IsMessagePumpThread())
	{
		oStd::future<bool> taskfuture = task.get_future();
		oWinMessagePump::Singleton()->Dispatch(std::move(task));
		try { taskfuture.get(); } catch(std::exception& e) { oErrorSetLast(e); return; }
	}

	else
		task();

	SetDesc(PendingDesc, true);

	// If fullscreen was the request, do it now
	if (_Init.WinDesc.State == oGUI_WINDOW_FULLSCREEN_EXCLUSIVE)
	{
		oGUI_WINDOW_DESC* pDesc = nullptr;
		Map(&pDesc);
		*pDesc = _Init.WinDesc;
		Unmap();
	}

	*_pSuccess = true;
}

oWinWindow::~oWinWindow()
{
	Close();
}

bool oWindowCreate(const oWINDOW_INIT& _Init, threadsafe oWindow** _ppWindow)
{
	bool success = false;
	oCONSTRUCT(_ppWindow, oWinWindow(_Init, &success));
	return success;
}

bool oWinWindow::QueryInterface(const oGUID& _InterfaceID, threadsafe void** _ppInterface) threadsafe
{
	*_ppInterface = nullptr;

	if (_InterfaceID == oGUID_oInterface || _InterfaceID == oGUID_oWindow)
	{
		Reference();
		*_ppInterface = this;
	}

	else if (_InterfaceID == oGetGUID<oGUI_WINDOW>())
		*_ppInterface = hWnd;
	else if (_InterfaceID == oGetGUID<oGUI_MENU>())
		*_ppInterface = hMenu;
	else if (_InterfaceID == oGetGUID<oGUI_STATUSBAR>())
		*_ppInterface = hStatusBar;

	return !!*_ppInterface ? true : oErrorSetLast(std::errc::function_not_supported);
}

bool oWinWindow::IsOpen() const threadsafe
{
	return !Closed.is_set();
}

bool oWinWindow::WaitUntilClosed(unsigned int _TimeoutMS) const threadsafe
{
	return Closed.wait_for(oStd::chrono::milliseconds(_TimeoutMS));
}

bool oWinWindow::WaitUntilOpaque(unsigned int _TimeoutMS) const threadsafe
{
	backoff bo;
	unsigned int Now = oTimerMS();
	unsigned int Then = Now + _TimeoutMS;
	while (!IsOpaque())
	{
		bo.pause();
		Now = oTimerMS();
		if (_TimeoutMS != oInfiniteWait && Now > Then)
			return oErrorSetLast(std::errc::timed_out);
	}

	return true;
}

bool oWinWindow::Close() threadsafe
{
	SendMessage(hWnd, WM_CLOSE, 0, 0);
	return Closed.is_set();
}

bool oWinWindow::IsWindowThread() const threadsafe
{
	return oWinMessagePump::Singleton()->IsMessagePumpThread();
}

bool oWinWindow::HasFocus() const threadsafe
{
	return hWnd ? oWinHasFocus(hWnd) : false;
}

void oWinWindow::SetFocus() threadsafe
{
	if (hWnd)
		oWinSetFocus(hWnd);
}

void oWinWindow::GetDesc(oGUI_WINDOW_DESC* _pDesc) const threadsafe
{
	*_pDesc = oLockSharedThis(DescMutex)->Desc;
}

bool oWinWindow::Map(oGUI_WINDOW_DESC** _ppDesc) threadsafe
{
	PendingDescMutex.lock();
	*_ppDesc = &oThreadsafe(this)->PendingDesc;
	return true;
}

void oWinWindow::Unmap() threadsafe
{
	SetDesc(oThreadsafe(this)->PendingDesc);
	PendingDescMutex.unlock();
}

void oWinWindow::SetDesc(const oGUI_WINDOW_CURSOR_DESC& _CursorDesc) threadsafe
{
	Dispatch(&oWinWindow::WTSetCursorDesc, oThreadsafe(this), _CursorDesc, false); // bind _CursorDesc by-copy
}

void oWinWindow::GetDesc(oGUI_WINDOW_CURSOR_DESC* _pCursorDesc) const threadsafe
{
	shared_lock<shared_mutex> lock(CursorDescMutex);
	*_pCursorDesc = oThreadsafe(this)->CursorDesc;
}

char* oWinWindow::GetStatusText(char* _StrDestination, size_t _SizeofStrDestination, int _StatusSectionIndex) const threadsafe
{
	return SendMessage(hWnd, oWM_STATUS_GETTEXT, MAKEWPARAM(_StatusSectionIndex, oShort(_SizeofStrDestination)), (LPARAM)_StrDestination) ? _StrDestination : nullptr;
}

void oWinWindow::SetStatusText(int _StatusSectionIndex, const char* _Format, va_list _Args) threadsafe
{
	oStd::lstring* pText = new oStd::lstring();
	vsnprintf(*pText, _Format, _Args);
	PostMessage(hWnd, oWM_STATUS_SETTEXT, _StatusSectionIndex, (LPARAM)pText);
}

void oWinWindow::SetHotKeys(const oGUI_HOTKEY_DESC_NO_CTOR* _pHotKeys, size_t _NumHotKeys) threadsafe
{
	oGUI_HOTKEY_DESC_NO_CTOR* pCopy = nullptr;
	if (_pHotKeys && _NumHotKeys)
	{
		pCopy = new oGUI_HOTKEY_DESC_NO_CTOR[_NumHotKeys];
		memcpy(pCopy, _pHotKeys, sizeof(oGUI_HOTKEY_DESC_NO_CTOR) * _NumHotKeys);
	}
	PostMessage(hWnd, oWM_SETHOTKEYS, (WPARAM)_NumHotKeys, (LPARAM)pCopy);
}

int oWinWindow::GetHotKeys(oGUI_HOTKEY_DESC_NO_CTOR* _pHotKeys, size_t _MaxNumHotKeys) const threadsafe
{
	return oInt(SendMessage(hWnd, oWM_GETHOTKEYS, _MaxNumHotKeys, (LPARAM)_pHotKeys));
}

oStd::future<oRef<oImage>> oWinWindow::CreateSnapshot(int _Frame, bool _IncludeBorder) const threadsafe
{
	// @oooii-tony: Probably there's a better way to make promises std::move around
	// and thus not need to explicitly allocate it, but first get oWindow settled
	// and then revisit this.

	auto PromisedImage = std::make_shared<oStd::promise<oRef<oImage>>>();
	oStd::future<oRef<oImage>> Image = PromisedImage->get_future();
	
	const_cast<threadsafe oWinWindow*>(this)->Dispatch([=]() mutable
	{
		bool success = WaitUntilOpaque(20000);
		if (!success)
		{
			if (oGUIIsVisible(Desc.State))
				oVERIFY(false); // pass through verification of Wait
			else
				oASSERT(false, "A non-hidden window timed out waiting to become opaque");
		}

		oRef<oImage> Image;
		void* buf = nullptr;
		size_t size = 0;
		oWinSetFocus(hWnd); // Windows doesn't do well with hidden contents.
		if (oGDIScreenCaptureWindow(hWnd, _IncludeBorder, malloc, &buf, &size, false))
		{
			oImageCreate("Screen capture", buf, size, &Image);
			free(buf);
		}

		if (!!Image)
			PromisedImage->set_value(Image);
		else
			PromisedImage->set_exception(std::make_exception_ptr(std::system_error(std::make_error_code((std::errc::errc)oErrorGetLast()), oErrorGetLastString())));
	});

	return Image;
}

void oWinWindow::Trigger(const oGUI_ACTION_DESC& _Action) threadsafe
{
	PostMessage(hWnd, oWM_ACTION_TRIGGER, 0, (LPARAM)new oGUI_ACTION_DESC(_Action));
}

void oWinWindow::WTTriggerAction(const oGUI_ACTION_DESC& _Action)
{
	#ifdef _DEBUG
		if (Desc.Debug || kForceDebug)
			oTRACE("hWindow 0x%x %s", _Action.hWindow, oStd::as_string(_Action.Action));
	#endif

	oFOR(oGUI_ACTION_HOOK& hook, ActionHooks)
	{
		if (hook)
			hook(_Action);
	}
}

bool oWinWindow::WTTriggerEvent(const oGUI_EVENT_DESC& _Event)
{
	#ifdef _DEBUG
		if ((Desc.Debug || kForceDebug) && _Event.Event != oGUI_MAINLOOP)
			oTRACE("hSource 0x%x %s pos=%d,%d size=%d,%d", _Event.hWindow, oStd::as_string(_Event.Event), _Event.ClientPosition.x, _Event.ClientPosition.y, _Event.ClientSize.x, _Event.ClientSize.y);
	#endif

	bool result = true;
	oFOR(oGUI_EVENT_HOOK& hook, EventHooks)
	{
		if (hook)
		{
			if (!hook(_Event))
				result = false;
		}
	}

	return result;
}

void oWinWindow::Dispatch(const oTASK& _Task) threadsafe
{
	oWinMessagePump::Singleton()->Dispatch(_Task);
}

void oWinWindow::SetTimer(uintptr_t _Context, unsigned int _RelativeTimeMS) threadsafe
{
	PostMessage(hWnd, oWM_START_TIMER, (WPARAM)_Context, _RelativeTimeMS);
}

bool oWinWindow::IsOpaque() const threadsafe
{
	// @oooii-tony: I can't find API to ask about the opacity of an HWND, so just
	// wait for a while.

	static const double kFadeInTime = 0.2;

	if (ShowTimestamp < 0.0)
		return false;
	double Now = oTimer();
	return (ShowTimestamp + kFadeInTime) < Now;
}

void oWinWindow::SetDesc(const oGUI_WINDOW_DESC& _Desc, bool _Force) threadsafe
{
	Dispatch(&oWinWindow::WTSetDesc, oThreadsafe(this), _Desc, _Force); // bind _Desc by-copy
}

void oWinWindow::SetNumStatusItems(int* _pItemWidths, size_t _NumItems) threadsafe
{
	int* w = new int[_NumItems];
	memcpy(w, _pItemWidths, sizeof(int) * _NumItems);
	PostMessage(hWnd, oWM_STATUS_SETPARTS, (WPARAM)_NumItems, (LPARAM)w);
}

//#define oDEBUG_WTSETDESC
#ifdef oDEBUG_WTSETDESC
#define WTSD_TRACE(_Format, ...) do { if (_NewDesc.Debug || kForceDebug) { oTRACE(_Format, ## __VA_ARGS__); } } while(false)
#else
	#define WTSD_TRACE(_Format, ...)
#endif

void oWinWindow::WTSetDesc(const oGUI_WINDOW_DESC& _NewDesc, bool _Force)
{
	#define CHANGED(x) ((_NewDesc.x != Desc.x) || _Force)

	if (CHANGED(hIcon))
	{
		WTSD_TRACE("== oWinSetIcon(0x%x (this), 0x%x) ==", hWnd, _NewDesc.hIcon);
		oWinSetIcon(hWnd, (HICON)_NewDesc.hIcon);
	}

	if (CHANGED(AlwaysOnTop))
	{
		WTSD_TRACE("== oWinSetAlwaysOnTop(0x%x (this), %s) ==", hWnd, oStd::as_string(_NewDesc.AlwaysOnTop));
		oVERIFY(oWinSetAlwaysOnTop(hWnd, _NewDesc.AlwaysOnTop));
	}

	if (CHANGED(hOwner))
	{
		PendingDesc.hOwner = _NewDesc.hOwner;
		WTSD_TRACE("== oWinSetOwner(0x%x (this), %p) ==", hWnd, _NewDesc.hOwner);
		oWinSetOwner(hWnd, (HWND)_NewDesc.hOwner);
	}

	if (CHANGED(Enabled))
	{
		WTSD_TRACE("== oWinSetEnabled(%s) ==", oStd::as_string(_NewDesc.Enabled));
		oVERIFY(oWinEnable(hWnd, _NewDesc.Enabled));
	}

	const bool kGoingToFullscreen = oGUIIsFullscreen(_NewDesc.State) && !oGUIIsFullscreen(Desc.State);
	const bool kGoingToWindowed = oGUIIsFullscreen(Desc.State) && !oGUIIsFullscreen(_NewDesc.State);

	// Extended Frame is Menu, StatusBar, (maybe toolbar or coolbar soon)
	const bool kHasExtendedFrame = _NewDesc.Style >= oGUI_WINDOW_FIXED && !oGUIIsFullscreen(_NewDesc.State);
	const bool kShowStatusBar = kHasExtendedFrame && _NewDesc.ShowStatusBar;

	if (!kHasExtendedFrame)
		oWinSetState(hStatusBar, oGUI_WINDOW_HIDDEN);

	// going to fullscreen completely discards state and style. Position is only
	// used to get a screen onto which fullscreen will be applied.
	if (kGoingToFullscreen)
	{
		if (GetParent(hWnd))
		{
			oASSERT(!_NewDesc.hParent && !Desc.hParent, "Going to fullscreen while parented is not yet handled");
			WTSD_TRACE("== SetParent(nullptr) ==");
			SetParent(hWnd, nullptr);
		}

		{
			// Move to the monitor we're going to go fullscreen on. Doing it here rather
			// than directly going fullscreen on an adapter allows us to call DXGI's
			// ResizeTarget in the MS-recommended way.
			// Don't record this move, let the window ignore position for any reason 
			// other than placing the window on the right monitor.
			int2 Pos = _NewDesc.ClientPosition;
			if (Pos.x == oDEFAULT)
				Pos.x = Desc.ClientPosition.x;
			if (Pos.y == oDEFAULT)
				Pos.y = Desc.ClientPosition.y;
			WTSD_TRACE("== oWinSetPosition(%dx%d) ==", Pos.x, Pos.y);
			oWinSetPosition(hWnd, Pos);
		}

		PreFullscreenDesc = Desc;
		if (oGUIIsFullscreen(PreFullscreenDesc.State))
			PreFullscreenDesc.State = oGUI_WINDOW_RESTORED;
		
		// @oooii-tony: Is this still necessary when it's set up above?
		if (_NewDesc.AlwaysOnTop)
		{
			SetWindowLongPtr(hWnd, GWL_EXSTYLE, WS_EX_TOPMOST);
		}

		if (_NewDesc.State == oGUI_WINDOW_FULLSCREEN_COOPERATIVE && _NewDesc.AlwaysOnTop)
		{
			// AlwaysOnTop solves top-ness for all windows except the taskbar, so 
			// keep focus, which covers the taskbar, but only do it for windows that
			// are fullscreen on the taskbar.
			if (oWinGetDisplayIndex(hWnd) == oDisplayGetPrimaryIndex())
				oKEEP_FOCUS(hWnd);
		}

		WTSD_TRACE("== oGUI_TO_FULLSCREEN ==");
		oGUI_EVENT_DESC ed;
		ed.Event = oGUI_TO_FULLSCREEN;
		ed.hWindow = (oGUI_WINDOW)hWnd;
		ed.ClientPosition = _NewDesc.ClientPosition;
		ed.ClientSize = _NewDesc.ClientSize;
		//ed.ScreenSize = ??; // read from a reg setting... if not use cur disp val
		ed.State = _NewDesc.State;
		if (!WTTriggerEvent(ed))
		{
			oGUI_WINDOW_DESC copy = _NewDesc;
			copy.State = oGUI_WINDOW_RESTORED;
			WTSD_TRACE("== Recursive WTSetDesc() ==");
			WTSetDesc(copy);
			return;
		}
	}

	else 
	{
		if (kGoingToWindowed)
		{
			WTSD_TRACE("== oGUI_FROM_FULLSCREEN ==");
			oGUI_EVENT_DESC ed;
			ed.Event = oGUI_FROM_FULLSCREEN;
			ed.hWindow = (oGUI_WINDOW)hWnd;
			ed.ClientPosition = _NewDesc.ClientPosition;
			ed.ClientSize = _NewDesc.ClientSize;
			// read from a reg setting... if not use cur disp val (though this should 
			// be current client size)
			//ed.ScreenSize = ??;
			ed.State = _NewDesc.State;
			WTTriggerEvent(ed);
			
			Desc.State = PreFullscreenDesc.State; // prevents infinite recursion
			WTSD_TRACE("== Recursive WTSetDesc() ==");
			WTSetDesc(PreFullscreenDesc);
		}

		if (kHasExtendedFrame)
		{
			if (CHANGED(ShowMenu))
			{
				WTSD_TRACE("== SetMenu() ==", oStd::as_string(_NewDesc.ShowMenu));
				oGUIMenuAttach((oGUI_WINDOW)hWnd, _NewDesc.ShowMenu ? hMenu : nullptr);
			}

			if (CHANGED(ShowStatusBar))
			{
				WTSD_TRACE("== oWinSetState(%s) ==", oStd::as_string(kShowStatusBar ? oGUI_WINDOW_RESTORED : oGUI_WINDOW_HIDDEN));
				oWinSetState(hStatusBar, kShowStatusBar ? oGUI_WINDOW_RESTORED : oGUI_WINDOW_HIDDEN);
			}
		}

		// As documented in MS's SetParent() API, set style flags before
		// SetParent call
		bool EmbeddedStyle = oGUI_WINDOW_EMBEDDED == oWinGetStyle(hWnd);
		if (_NewDesc.hParent && Desc.Style != oGUI_WINDOW_EMBEDDED && _NewDesc.Style != oGUI_WINDOW_EMBEDDED)
		{
			RECT rParentLocal = oWinRectWH(_NewDesc.ClientPosition, _NewDesc.ClientSize);
			WTSD_TRACE("== oWinSetStyle(oGUI_WINDOW_EMBEDDED, ShowStatusBar=%s) xy=%d,%d wh=%dx%d ==", oStd::as_string(kShowStatusBar), _NewDesc.ClientPosition.x, _NewDesc.ClientPosition.y, _NewDesc.ClientSize.x, _NewDesc.ClientSize.y);
			oVERIFY(oWinSetStyle(hWnd, oGUI_WINDOW_EMBEDDED,kShowStatusBar, &rParentLocal));
			EmbeddedStyle = true;
		}

		if (CHANGED(hParent))
		{
			WTSD_TRACE("== SetParent(%p) ==", _NewDesc.hParent);
			oVB(SetParent(hWnd, (HWND)_NewDesc.hParent));

			WTSD_TRACE("== oWinSetStyle(%s, ShowStatusBar=%s) xy=%d,%d wh=%dx%d ==", oStd::as_string(_NewDesc.Style), oStd::as_string(kShowStatusBar), _NewDesc.ClientPosition.x, _NewDesc.ClientPosition.y, _NewDesc.ClientSize.x, _NewDesc.ClientSize.y);
			RECT rParentLocal = oWinRectWH(_NewDesc.ClientPosition, _NewDesc.ClientSize);
			oVERIFY(oWinSetStyle(hWnd, _NewDesc.Style, kShowStatusBar, &rParentLocal));

			if (!Desc.hParent && _NewDesc.hParent)
			{
				RECT r;
				GetClientRect((HWND)_NewDesc.hParent, &r);
				WTSD_TRACE("== PostMessage(%p, WM_SIZE, SIZE_RESTORED, %dx%d) ==", _NewDesc.hParent, oWinRectW(r), oWinRectH(r));
				PostMessage((HWND)_NewDesc.hParent, WM_SIZE, SIZE_RESTORED, (LPARAM)MAKELONG(oWinRectW(r), oWinRectH(r)));
			}
			
			EmbeddedStyle = oGUI_WINDOW_EMBEDDED == oWinGetStyle(hWnd);
		}

		RECT rClient;
		if (_NewDesc.ClientPosition.x == oDEFAULT || _NewDesc.ClientPosition.y == oDEFAULT || _NewDesc.ClientSize.x == oDEFAULT || _NewDesc.ClientSize.y == oDEFAULT)
		{
			// When WTSetDesc is called, either it is a PendingDesc that had values from
			// the current Desc, or its a PreFullscreenDesc that had values from the 
			// window at that time. So no matter what the "exactly prior values" case
			// is handled by using _NewDesc values. If any of those have been changed
			// to oDEFAULT, Use the monitor the window is on

			RECT rParent = oWinGetParentRect(hWnd);
			rClient = oWinRectResolve(rParent, _NewDesc.ClientPosition, _NewDesc.ClientSize, oGUI_ALIGNMENT_MIDDLE_CENTER, !!_NewDesc.hParent);
		}

		else
			rClient = oWinRectWH(_NewDesc.ClientPosition, _NewDesc.ClientSize);

		if (!EmbeddedStyle)
		{
			RECT rOld;
			GetClientRect(hWnd, &rOld);

			if (oWinGetState(hStatusBar) != oGUI_WINDOW_HIDDEN)
			{
				RECT rStatus;
				GetClientRect(hStatusBar, &rStatus);
				rOld.bottom -= oWinRectH(rStatus);
			}

			rOld = oWinRectTranslate(rOld, oWinGetPosition(hWnd));

			bool SizeChanged = !!memcmp(&rOld, &rClient, sizeof(RECT));
			if (CHANGED(Style) || CHANGED(ShowStatusBar) || SizeChanged)
			{
				WTSD_TRACE("== oWinSetStyle(%s, ShowStatusBar=%s, xy=%d,%d wh=%dx%d) ==", oStd::as_string(_NewDesc.Style), oStd::as_string(kShowStatusBar), rClient.left, rClient.top, oWinRectW(rClient), oWinRectH(rClient));
				oVERIFY(oWinSetStyle(hWnd, _NewDesc.Style, kShowStatusBar, &rClient));

				// If this is true, AND the window is maximized, oWinSetStyle is going 
				// to realize that a resize is not necessary for the parent window, and 
				// not send an event out. But because the status bar in the maximized 
				// case cannot grow the window AND when maximized the user is saying "I
				// don't care about the client area size, just that it is large minus
				// OS decoration", then the status bar area doesn't grow the decorated 
				// window to maintain an explicit client size, it instead shrinks the 
				// max client size. So handle that here...
				if (CHANGED(ShowStatusBar) && oGUI_WINDOW_MAXIMIZED == oWinGetState(hWnd))
					SendMessage(hWnd, WM_SIZE, SIZE_MAXIMIZED, MAKELPARAM(oWinRectW(rOld), oWinRectH(rOld)));
			}
		}

		if (CHANGED(State))
		{
			WTSD_TRACE("== oWinSetState(%s, HasFocus=%s) ==", oStd::as_string(_NewDesc.State), oStd::as_string(_NewDesc.HasFocus));
			
			// oWinSetState calls back into the WndProc and gets to a WM_SIZE, which
			// will attempt to preserve the current Desc's value if not minimized, so
			// set that up now: (this is hacky, there should be a better way to do 
			// this)
			Desc.State = _NewDesc.State;
			oVERIFY(oWinSetState(hWnd, _NewDesc.State, _NewDesc.HasFocus));

			ShowTimestamp = oGUIIsVisible(_NewDesc.State) ? oTimer() : -1.0;
		}

		else if ((oWinHasFocus(hWnd) && !_NewDesc.HasFocus) || (!oWinHasFocus(hWnd) && _NewDesc.HasFocus) || CursorDesc.HasCapture)
		{
			WTSD_TRACE("== oWinSetFocus(%s) ==", oStd::as_string(_NewDesc.HasFocus));
			oWinSetFocus(hWnd, _NewDesc.HasFocus);
		}
	}

	if (CHANGED(EnableMainLoopEvent))
		oWinMessagePump::Singleton()->RegisterWindow(hWnd, _NewDesc.EnableMainLoopEvent);

	lock_guard<shared_mutex> lock(DescMutex);
	WTSD_TRACE("== WriteDesc ==");
	Desc = _NewDesc;
	#undef CHANGED
}

void oWinWindow::WTSetCursorDesc(const oGUI_WINDOW_CURSOR_DESC& _NewDesc, bool _Force)
{
	#define CHANGED(x) ((_NewDesc.x != CursorDesc.x) || _Force)

	if (CHANGED(HasCapture))
	{
		if (_NewDesc.HasCapture)
		{
			oASSERT(!Desc.hParent, "For HasCapture to be true, the window must not have a parent (it must be a top-level window).");
			SetCapture(hWnd);
			oWinSetFocus(hWnd, Desc.HasFocus);
		}

		else
			ReleaseCapture();
	}

	bool CallWTSetCursor = CHANGED(ClientState) || CHANGED(NonClientState);
	{
		lock_guard<shared_mutex> lock(CursorDescMutex);
		CursorDesc = _NewDesc;
	}

	if (CallWTSetCursor)
	{
		int2 p;
		GetCursorPos((POINT*)&p);
		oVB(ScreenToClient(hWnd, (POINT*)&p));
		WTSetCursor(greater_than_equal(p, int2(0,0)) && less_than_equal(p, Desc.ClientSize));
	}
	#undef CHANGED
}

void oWinWindow::WTSetCursor(bool _IsInClientArea)
{
	oGUI_CURSOR_STATE NewState = oGUI_CURSOR_NONE;
	if (AltF1CursorVisible)
		NewState = _IsInClientArea ? CursorDesc.ClientState : CursorDesc.NonClientState;
	oWinCursorSetState(hWnd, NewState, hUserCursor);
}

void oWinWindow::WTOnSize(const int2& _NewSize, oGUI_WINDOW_STATE _NewState)
{
	oASSERT(IsWindowThread(), "Desc access isn't protected because it only occurs on the window thread. If that's not true, then more protection is needed");

	oGUI_EVENT_DESC ed;
	ed.hWindow = (oGUI_WINDOW)hWnd;
	ed.Event = oGUI_SIZING;
	ed.ClientPosition = Desc.ClientPosition;
	ed.ClientSize = Desc.ClientSize;
	ed.State = Desc.State;
	WTTriggerEvent(ed);

	int2 NewSize = _NewSize;
	if (oGUI_WINDOW_HIDDEN != oWinGetState(hStatusBar))
	{
		SendMessage(hStatusBar, WM_SIZE, 0, 0);
		RECT rStatusBar;
		GetClientRect(hStatusBar, &rStatusBar);
		NewSize.y -= oWinRectH(rStatusBar);
	}

	oDISPLAY_DESC dd;
	oVERIFY(oDisplayEnum(oWinGetDisplayIndex(hWnd), &dd));
	ed.ScreenSize = dd.WorkareaSize;

	ed.Event = oGUI_SIZED;
	ed.ClientSize = NewSize;
	ed.State = _NewState;
	WTTriggerEvent(ed);

	// @oooii-tony: the desc is protected because we're on the desc thread. 
	// The pending desc is another story. Look into this more and if this 
	// changes, review all WndProc for other instances of the need for 
	// protection.
	// Should desc be updated before the event so if someone calls GetDesc() it's
	// consistent? Depending on locking, that could deadlock... but that's what
	// PendingDesc is for.
	lock_guard<shared_mutex> lock(DescMutex);
	PendingDesc.State = Desc.State = ed.State;
	PendingDesc.ClientSize = Desc.ClientSize = NewSize;
}

// Decodes the wParam for WM_SYSCOMMAND. If this returns true, _pNewState 
// contains the new state of the window, else the value is left untouched.
bool oWinStateChanged(WPARAM _wParam, oGUI_WINDOW_STATE* _pNewState)
{
	bool StateChanged = true;
	switch (_wParam & 0xfff0)
	{
		case SC_RESTORE:  *_pNewState = oGUI_WINDOW_RESTORED; break;
		case SC_MINIMIZE: *_pNewState = oGUI_WINDOW_MINIMIZED; break;
		case SC_MAXIMIZE: *_pNewState = oGUI_WINDOW_MAXIMIZED; break;
		default: StateChanged = false; break;
	}
	return StateChanged;
}

LRESULT oWinWindow::WndProc(HWND _hWnd, UINT _uMsg, WPARAM _wParam, LPARAM _lParam)
{
	if ((Desc.Debug || kForceDebug) && _uMsg != oWM_MAINLOOP)
	{
		oStd::xlstring s;
		oTRACE("%s", oWinParseWMMessage(s, s.capacity(), &ControlKeyState, _hWnd, _uMsg, _wParam, _lParam));
	}

	double Timestamp = oWinGetDispatchMessageTime();

	LRESULT ActionLResult = 0;
	oGUI_ACTION_DESC Action;
	bool Handled = false;

	if (oWinKeyDispatchMessage(_hWnd, _uMsg, _wParam, _lParam, Timestamp, &ControlKeyState, &Action))
	{
		if (Desc.AllowClientDragToMove)
		{
			const oGUI_KEY CheckKey = GetSystemMetrics(SM_SWAPBUTTON) ? oGUI_KEY_MOUSE_RIGHT : oGUI_KEY_MOUSE_LEFT;

			if (Action.Key == CheckKey)
			{
				if (Action.Action == oGUI_ACTION_KEY_DOWN)
				{
					CursorClientPosAtMouseDown = oWinCursorGetPosition(_hWnd);
					SetCapture(_hWnd);
				}

				else if (Action.Action == oGUI_ACTION_KEY_UP)
				{
					ReleaseCapture();
					CursorClientPosAtMouseDown = int2(oDEFAULT, oDEFAULT);
				}
			}
		}

		WTTriggerAction(Action);

		Handled = _uMsg != WM_MOUSEMOVE;
	}

	if (oWinControlToAction(_hWnd, _uMsg, _wParam, _lParam, &Action, &ActionLResult))
	{
		WTTriggerAction(Action);
		return ActionLResult;
	}

	if (!Handled)
	{
		switch (_uMsg)
		{
			// _____________________________________________________________________________
			// Standard windows handling that doesn't broadcast user events or actions

			// @oooii-tony: This message isn't coming through when I plug/unplug the
			// USB stick that controls both mouse and KB.
			#ifdef oUSE_RAW_INPUT
				case WM_INPUT_DEVICE_CHANGE:
				{
					oGUI_INPUT_DEVICE_EVENT_DESC e((oGUI_WINDOW)_hWnd, oGUI_INPUT_DEVICE_CHANGED, Desc);
					e.Status = _wParam == GIDC_ARRIVAL ? oGUI_INPUT_DEVICE_READY : oGUI_INPUT_DEVICE_NOT_READY;

					RID_DEVICE_INFO RIDDI;
					UINT Size = sizeof(RIDDI);
					GetRawInputDeviceInfo((HANDLE)_lParam, RIDI_DEVICEINFO, &RIDDI, &Size);
					switch (RIDDI.dwType)
					{
						case RIM_TYPEKEYBOARD:
							e.Type = oGUI_INPUT_DEVICE_KEYBOARD;
							break;
						case RIM_TYPEMOUSE:
							e.Type = oGUI_INPUT_DEVICE_MOUSE;
							break;
						default:
						case RIM_TYPEHID:
							e.Type = oGUI_INPUT_DEVICE_UNKNOWN;
							break;
					}

					oTRACE("%s: %s", oStd::as_string(e.Type), oStd::as_string(e.Status));
					WTTriggerEvent(e);
					break;
				}

				// @oooii-tony: I thought this might give me WM_KEYUP messages for media
				// keys, but it doesn't get any event at all!
				case WM_INPUT:
				{
					UINT RISize = 0;
					if (0 == GetRawInputData((HRAWINPUT)_lParam, RID_INPUT, nullptr, &RISize, sizeof(RAWINPUTHEADER)))
					{
						BYTE* pBuffer = new BYTE[RISize];
						GetRawInputData((HRAWINPUT)_lParam, RID_INPUT, pBuffer, &RISize, sizeof(RAWINPUTHEADER));
						RAWINPUT* pRaw = (RAWINPUT*)pBuffer;
						switch (pRaw->header.dwType)
						{
							case RIM_TYPEKEYBOARD:
								pRaw = pRaw;
								break;
							case RIM_TYPEMOUSE:
								pRaw = pRaw;
								break;
							case RIM_TYPEHID:
								pRaw = pRaw;
								break;
							default:
								break;
						}
					
						delete [] pBuffer;
					}

					break;
				}
			#endif

			case WM_SETCURSOR:
				WTSetCursor(LOWORD(_lParam) == HTCLIENT);
				break;

			case WM_MOUSEMOVE:
			{
				if (CursorDesc.HasCapture)
				{
					int2 P(GET_X_LPARAM(_lParam), GET_Y_LPARAM(_lParam));
					WTSetCursor(greater_than_equal(P, int2(0,0)) && less_than_equal(P, Desc.ClientSize));
				}

				if (Desc.AllowClientDragToMove && CursorClientPosAtMouseDown.x != oDEFAULT)
				{
					POINT p;
					p.x = GET_X_LPARAM(_lParam);
					p.y = GET_Y_LPARAM(_lParam);
					ClientToScreen(_hWnd, &p);
					oWinSetPosition(_hWnd, int2(p.x, p.y) - CursorClientPosAtMouseDown);
				}

				break;
			}

			case WM_ERASEBKGND:
				if (!Desc.DefaultEraseBackground)
					return 1;
				break;

			case WM_GETDLGCODE:
				// this window is not a sub-class of any window, so allow all keys to come 
				// through.
				return DLGC_WANTALLKEYS;

			case WM_ACTIVATE:
				oWinMessagePump::Singleton()->SetAccel(_wParam == WA_INACTIVE ? nullptr : hAccel);
				WTTriggerEvent(oGUI_EVENT_DESC((oGUI_WINDOW)_hWnd, (_wParam == WA_INACTIVE) ? oGUI_DEACTIVATED : oGUI_ACTIVATED, Desc));
				break;

			// @oooii-tony: All these should be treated the same if there's any reason
			// to override painting.
			case WM_PAINT:
			case WM_PRINT:
			case WM_PRINTCLIENT:
				WTTriggerEvent(oGUI_EVENT_DESC((oGUI_WINDOW)_hWnd, oGUI_PAINT, Desc));
				break;

			case WM_DISPLAYCHANGE:
				WTTriggerEvent(oGUI_EVENT_DESC((oGUI_WINDOW)_hWnd, oGUI_DISPLAY_CHANGED, Desc, int2(LOWORD(_lParam), HIWORD(_lParam))));
				break;

			case WM_CAPTURECHANGED:
			{
				if ((HWND)_lParam != _hWnd)
				{
					if (CursorClientPosAtMouseDown.x != oDEFAULT)
						ReleaseCapture();

					WTTriggerEvent(oGUI_EVENT_DESC((oGUI_WINDOW)_hWnd, oGUI_LOST_CAPTURE, Desc));
				}
				break;
			}

			case WM_TIMER:
			{
				oGUI_TIMER_EVENT_DESC e((oGUI_WINDOW)_hWnd, oGUI_TIMER, Desc);
				e.Context = (uintptr_t)_wParam;
				WTTriggerEvent(e);
				return 1;
			}

			// NOTE: It's hard to refactor WM_DROPFILES since it loops on trigger event
			case WM_DROPFILES:
			{
				oGUI_DROP_EVENT_DESC ed((oGUI_WINDOW)_hWnd, oGUI_DROP_FILES, Desc);
				DragQueryPoint((HDROP)_wParam, (POINT*)&ed.ClientDropPosition);
				ed.NumPaths = DragQueryFile((HDROP)_wParam, ~0u, nullptr, 0);
				ed.pPaths = new oStd::path_string[ed.NumPaths];
				for (uint i = 0; i < ed.NumPaths; i++)
					DragQueryFile((HDROP)_wParam, i, const_cast<char*>(ed.pPaths[i].c_str()), oUInt(ed.pPaths[i].capacity()));
				WTTriggerEvent(ed);
				delete [] ed.pPaths;
				DragFinish((HDROP)_wParam);
				return 0;
			}

			// NOTE: It's hard to refactor WM_TOUCH since it loops on trigger action
			#ifdef oWINDOWS_HAS_REGISTERTOUCHWINDOW
			case WM_TOUCH:
			{
				TOUCHINPUT inputs[oGUI_KEY_TOUCH_LAST - oGUI_KEY_TOUCH_FIRST];
				const UINT nTouches = __min(LOWORD(_wParam), oCOUNTOF(inputs));
				if (nTouches)
				{
					if (GetTouchInputInfo((HTOUCHINPUT)_lParam, nTouches, inputs, sizeof(TOUCHINPUT)))
					{
						oGUI_ACTION_DESC a((oGUI_WINDOW)_hWnd, Timestamp, oGUI_ACTION_KEY_DOWN, oGUI_INPUT_DEVICE_TOUCH, 0);
						for (UINT i = 0; i < nTouches; i++)
						{
							// @oooii-tony: Maybe touch doesn't need to pollute X11? The idea
							// one day is to support wacky interface devices through RFB 
							// protocol which forces all boolean things down X11... should 
							// this comply?
							a.DeviceID = i;
							a.Key = (oGUI_KEY)(oGUI_KEY_TOUCH1 + i);
							a.Position = float4(inputs[i].x / 100.0f, inputs[i].y / 100.0f, 0.0f, 0.0f);
							WTTriggerAction(a);
						}
						CloseTouchInputHandle((HTOUCHINPUT)_lParam);
					}
				}
				break;
			}
			#endif
		
			// _____________________________________________________________________________
			// Ouroboros custom messages

			case oWM_MAINLOOP:
				WTTriggerEvent(oGUI_EVENT_DESC((oGUI_WINDOW)_hWnd, oGUI_MAINLOOP, Desc));
				return 0;

			case oWM_START_TIMER:
				::SetTimer(_hWnd, (UINT_PTR)_wParam, (UINT)_lParam, nullptr);
				break;

			case oWM_ACTION_HOOK:
				return (LRESULT)oStd::sparse_set(ActionHooks, *(oGUI_ACTION_HOOK*)_lParam);

			case oWM_ACTION_UNHOOK:
				return oStd::ranged_set(ActionHooks, _lParam, nullptr) ? 1 : 0;

			case oWM_ACTION_TRIGGER:
				WTTriggerAction(*(oGUI_ACTION_DESC*)_lParam);
				delete (oGUI_ACTION_DESC*)_lParam;
				return 0;

			case oWM_EVENT_HOOK:
				return (LRESULT)oStd::sparse_set(EventHooks, *(oGUI_EVENT_HOOK*)_lParam);

			case oWM_EVENT_UNHOOK:
				return oStd::ranged_set(EventHooks, _lParam, nullptr) ? 1 : 0;

			case oWM_SETHOTKEYS:
			{
				if (hAccel)
				{
					DestroyAcceleratorTable(hAccel);
					hAccel = nullptr;
				}

				const oGUI_HOTKEY_DESC_NO_CTOR* pHotKeys = (const oGUI_HOTKEY_DESC_NO_CTOR*)_lParam;
				if (pHotKeys)
				{
					ACCEL* pAccels = new ACCEL[_wParam];
					oWinAccelFromHotKeys(pAccels, pHotKeys, _wParam);
					hAccel = CreateAcceleratorTable((LPACCEL)pAccels, oUInt(_wParam));
					delete [] pAccels;
					delete [] pHotKeys;
				}

				if (oWinHasFocus(_hWnd))
					oWinMessagePump::Singleton()->SetAccel(hAccel);

				return 0;
			}

			case oWM_GETHOTKEYS:
			{
				if (hAccel && _lParam && _wParam)
				{
					int nHotKeys = CopyAcceleratorTable(hAccel, nullptr, 0);
					ACCEL* pAccels = new ACCEL[nHotKeys];
					CopyAcceleratorTable(hAccel, pAccels, nHotKeys);
					size_t NumCopied = __min(nHotKeys, oInt(_wParam));
					oWinAccelToHotKeys((oGUI_HOTKEY_DESC_NO_CTOR*)_lParam, pAccels, NumCopied);
					delete [] pAccels;
					return (LRESULT)NumCopied;
				}

				return 0;
			}

			case oWM_STATUS_SETTEXT:
				if (hStatusBar)
				{
					oStd::lstring* pString = (oStd::lstring*)_lParam;
					oWinStatusBarSetText(hStatusBar, oInt(_wParam), oGUI_BORDER_FLAT, pString->c_str());
					delete pString;
				}
				return 0;

			case oWM_STATUS_GETTEXT:
			{
				if (hStatusBar)
				{
					int Index = GET_X_LPARAM(_wParam);
					int MaxSize = GET_Y_LPARAM(_wParam);
					if (oWinStatusBarGetText((char*)_lParam, MaxSize, hStatusBar, Index))
						return strlen((char*)_lParam);
				}

				return 0;
			}

			case oWM_STATUS_SETPARTS:
			{
				int* pItemWidths = (int*)_lParam;
				oWinStatusBarSetNumItems(hStatusBar, pItemWidths, _wParam);
				if (pItemWidths)
					delete [] pItemWidths;
				return 0;
			}

			case oWM_SKELETON:
			{
				oGUI_ACTION_DESC a((oGUI_WINDOW)_hWnd, Timestamp, oGUI_ACTION_SKELETON, oGUI_INPUT_DEVICE_SKELETON, LOWORD(_wParam));
				a.hSkeleton = (oGUI_SKELETON)_lParam;
				WTTriggerAction(a);
				return 0;
			}

			case oWM_USER_CAPTURED:
				WTTriggerAction(oGUI_ACTION_DESC((oGUI_WINDOW)_hWnd, Timestamp, oGUI_ACTION_SKELETON_ACQUIRED, oGUI_INPUT_DEVICE_SKELETON, oInt(_wParam)));
				return 0;

			case oWM_USER_LOST:
				WTTriggerAction(oGUI_ACTION_DESC((oGUI_WINDOW)_hWnd, Timestamp, oGUI_ACTION_SKELETON_LOST, oGUI_INPUT_DEVICE_SKELETON, oInt(_wParam)));
				return 0;

			case oWM_INPUT_DEVICE_STATUS:
			{
				oGUI_INPUT_DEVICE_EVENT_DESC e((oGUI_WINDOW)_hWnd, oGUI_INPUT_DEVICE_CHANGED, Desc);
				e.Type = oGUI_INPUT_DEVICE_TYPE(LOWORD(_wParam));
				e.Status = oGUI_INPUT_DEVICE_STATUS(HIWORD(_wParam));
				oStd::mstring* pStr = (oStd::mstring*)_lParam;
				e.InstanceName = *pStr;
				WTTriggerEvent(e);
				delete pStr;
				return 0;
			}

			// _____________________________________________________________________________
			// Lifetime/Sizing/Moving/State handlers

			// size/move stuff uses mutexes... probably bad. All size handling may have
			// to be reconsidered/rewritten...

			case WM_ENTERSIZEMOVE:
				WTTriggerEvent(oGUI_EVENT_DESC((oGUI_WINDOW)hWnd, (_wParam == SC_MOVE) ? oGUI_MOVING : oGUI_SIZING, Desc));
				return 0;

			case WM_MOVE:
			{
				oGUI_EVENT_DESC e((oGUI_WINDOW)hWnd, oGUI_MOVED, Desc);
				e.ClientPosition = int2(GET_X_LPARAM(_lParam), GET_Y_LPARAM(_lParam));
				WTTriggerEvent(e);
				lock_guard<shared_mutex> lock(DescMutex);
				PendingDesc.ClientPosition = Desc.ClientPosition = e.ClientPosition;
				return 0;
			}

			case WM_SIZE:
			{
				oGUI_WINDOW_STATE NewState = (_wParam == SIZE_MINIMIZED) ? oGUI_WINDOW_MINIMIZED : Desc.State;
				WTOnSize(int2(GET_X_LPARAM(_lParam), GET_Y_LPARAM(_lParam)), NewState);
				return 0;
			}

			case WM_SYSCOMMAND:
			{
				oGUI_WINDOW_STATE NewState = Desc.State;
				if (oWinStateChanged(_wParam, &NewState))
					WTOnSize(Desc.ClientSize, NewState); // @oooii-tony: What should _NewSize be here?
				break; // fall through to default handler
			}

			// Handle non-pass-thru syskeys
			case WM_SYSKEYDOWN:
			{
				if (_wParam == VK_F1 && !Desc.AllowAltF1) return 0;
				if (_wParam == VK_F4 && !Desc.AllowAltF4) return 0;
				if (_wParam == VK_RETURN && !Desc.AllowAltEnter) return 0;

				// No additional handling required for Alt-F4... Windows knows what to do
				switch (_wParam)
				{
					case VK_F1:
						AltF1CursorVisible = !AltF1CursorVisible;
						oWinCursorSetVisible(AltF1CursorVisible);
						break;

					case VK_RETURN:
						if (oGUIIsFullscreen(Desc.State))
							WTSetDesc(PreFullscreenDesc);
						else
						{
							oGUI_WINDOW_DESC NewDesc = PreFullscreenDesc = Desc;
							NewDesc.State = oGUI_WINDOW_FULLSCREEN_EXCLUSIVE;
							WTSetDesc(NewDesc);
						}
						break;

					default:
						break;
				}

				break;
			}
	
			case WM_CREATE:
			{
				CREATESTRUCT CS = *(CREATESTRUCT*)_lParam;
				hStatusBar = oWinStatusBarCreate(_hWnd, (HMENU)0x00005747);
				oWinSetState(hStatusBar, Desc.ShowStatusBar ? oGUI_WINDOW_RESTORED : oGUI_WINDOW_HIDDEN);
				oWinStatusBarInitialize(hStatusBar, Desc);

				hMenu = oGUIMenuCreate(true);
				if (Desc.ShowMenu)
					oGUIMenuAttach((oGUI_WINDOW)_hWnd, hMenu);

				// this is a bit dangerous because it's not really true this hWnd is 
				// ready for use, but we need to expose it consistently as a valid 
				// return value from oWindow::QueryInterface so it can be accessed from 
				// oGUI_CREATING, where it's known to be only semi-ready.
				hWnd = _hWnd; 

				oGUI_CREATING_EVENT_DESC e((oGUI_WINDOW)hWnd, oGUI_CREATING, Desc);
				e.State = oWinGetState(_hWnd);
				e.hMenu = hMenu;
				e.hStatusBar = (oGUI_STATUSBAR)hStatusBar;

				if (!WTTriggerEvent(e))
					return -1;
				if (Desc.ShowMenu)
					oGUIMenuAttach((oGUI_WINDOW)_hWnd, hMenu);

				ShowTimestamp = oGUIIsVisible(e.State) ? oTimer() : -1.0;
				break;
			}
			case WM_CLOSE:
			{
				oGUI_EVENT_DESC ed((oGUI_WINDOW)_hWnd, oGUI_CLOSING, Desc);
			
				// the closed test is threadsafe because this message pump occurs on one 
				// thread and Closed is only set here.
				if (WTTriggerEvent(ed))
				{
					Closed.set();
					DestroyWindow(_hWnd);
				}

				return 0;
			}
			case WM_DESTROY:
			{
				oWinMessagePump::Singleton()->UnregisterWindow(_hWnd);

				oWinSetOwner(_hWnd, nullptr); // minimizes "randomly set focus to some window other than parent"

				oGUI_EVENT_DESC ed((oGUI_WINDOW)_hWnd, oGUI_CLOSED, Desc);
				WTTriggerEvent(ed);
				if (!GetParent(_hWnd)) // squelches failure about child windows not being allowed to have menus, even null menus
					oGUIMenuAttach((oGUI_WINDOW)_hWnd, nullptr);
				oGUIMenuDestroy(hMenu);
				if (hAccel)
					DestroyAcceleratorTable(hAccel);

				if (hAccel == oWinMessagePump::Singleton()->GetAccel())
					oWinMessagePump::Singleton()->SetAccel(nullptr);

				return 0;
			}

			default:
				break;
		}
	}

	return DefWindowProc(_hWnd, _uMsg, _wParam, _lParam);
}
