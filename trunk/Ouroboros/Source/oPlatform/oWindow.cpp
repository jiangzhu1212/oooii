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
#include <oPlatform/oWindow.h>
#include <oBasis/oEvent.h>
#include <oBasis/oLockThis.h>
#include <oPlatform/oDisplay.h>
#include <oPlatform/Windows/oGDI.h>
#include <oPlatform/Windows/oWinAsString.h>
#include <oPlatform/Windows/oWinCursor.h>
#include <oPlatform/Windows/oWinMenu.h>
#include <oPlatform/Windows/oWinStatusBar.h>
#include <oPlatform/Windows/oWinWindowing.h>
#include <oPlatform/Windows/oWinRect.h>
#include "oVKToX11Keyboard.h"
#include <windowsx.h>

class oWinWindow : public oWindow
{
public:
	oWinWindow(const oWINDOW_INIT& _Init, bool* _pSuccess);
	~oWinWindow();
	oDECLARE_WNDPROC(oWinWindow);

	int Reference() threadsafe override
	{
		return RefCount.Reference();
	}
	
	void Release() threadsafe override
	{
		// If a window's action handler decides to destroy itself, there is a 
		// deadlock where the message queue wants to flush/joint from a call from 
		// itself. Outstanding messages do need to flush - largely relating to 
		// derived window types that share state management such as DXGI + windows
		// for 3D rendering. So make a new thread that basically waits for the 
		// close then deletes this object from a different thread that better allows 
		// for a join().

		if (RefCount.Release())
		{
			if (oWinIsWindowThread(hWnd))
			{
				oThread t([=] {
					WaitUntilClosed();
					delete this;
				});
				t.detach(); // don't stop, move onto closing the window.
				Close();
			}
			else
				delete this;
		}
	}

	bool QueryInterface(const oGUID& _InterfaceID, threadsafe void** _ppInterface) threadsafe override;

	bool IsOpen() const threadsafe override;
	bool WaitUntilClosed(unsigned int _TimeoutMS = oInfiniteWait) const threadsafe override;
	bool WaitUntilOpaque(unsigned int _TimeoutMS = oInfiniteWait) const threadsafe override;
	bool Close(bool _AskFirst = true) threadsafe override;
	bool IsWindowThread() const threadsafe override;
	bool HasFocus() const threadsafe override;
	void SetFocus() threadsafe override;
	void GetDesc(oGUI_WINDOW_DESC* _pDesc) const threadsafe override;
	bool Map(oGUI_WINDOW_DESC** _ppDesc) threadsafe override;
	void Unmap() threadsafe override;
	char* GetTitle(char* _StrDestination, size_t _SizeofStrDestination) const threadsafe override;
	void SetTitleV(const char* _Format, va_list _Args) threadsafe override;
	char* GetStatusText(char* _StrDestination, size_t _SizeofStrDestination, int _StatusSectionIndex) const threadsafe override;
	void SetStatusText(int _StatusSectionIndex, const char* _Format, va_list _Args) threadsafe override;

	void SetHotKeys(const oGUI_HOTKEY_DESC_NO_CTOR* _pHotKeys, size_t _NumHotKeys) threadsafe override;
	int GetHotKeys(oGUI_HOTKEY_DESC_NO_CTOR* _pHotKeys, size_t _MaxNumHotKeys) const threadsafe override;
	oStd::future<oRef<oImage>> CreateSnapshot(int _Frame = oInvalid,  bool _IncludeBorder = false) const threadsafe override;
	void Trigger(const oGUI_ACTION_DESC& _Action) threadsafe override;
	void Dispatch(const oTASK& _Task) threadsafe override;
	oDEFINE_CALLABLE_WRAPPERS(Dispatch, threadsafe, Dispatch);
	int HookActions(const oGUI_ACTION_HOOK& _Hook) threadsafe override;
	void UnhookActions(int _ActionHookID) threadsafe override;
	int HookEvents(const oGUI_EVENT_HOOK& _Hook) threadsafe override;
	void UnhookEvents(int _EventHookID) threadsafe override;

protected:
	HWND hWnd;
	HMENU hMenu;
	HWND hStatusBar;
	HACCEL hAccel;
	HCURSOR hUserCursor;
	oEvent Closed;
	double ShowTimestamp;
	oSharedMutex DescMutex;
	oMutex PendingDescMutex;
	oRecursiveMutex ActionHooksMutex;
	oRecursiveMutex EventHooksMutex;
	oRecursiveMutex DestroyMutex;
	oGUI_WINDOW_DESC Desc;
	oGUI_WINDOW_DESC TestPendingDesc; // copy of PendingDesc before user maps it. If it's the same as the incoming buffer, then skip update
	oGUI_WINDOW_DESC PendingDesc;
	oGUI_WINDOW_DESC PreFullscreenDesc;
	oTASK CachedWTRun;
	oRef<threadsafe oDispatchQueuePrivate> MessageQueue;
	oThread::id MessagePumpThreadID;
	oRefCount RefCount;
	std::vector<oGUI_ACTION_HOOK> ActionHooks;
	std::vector<oGUI_EVENT_HOOK> EventHooks;
	bool CursorVisible; // override set by Alt-F1

	inline oWinWindow* This() const threadsafe { return (oWinWindow*)this; }

	// Run a function of the oError pattern (returns true for success) and waits
	// for it to be executed.
	bool DispatchAndWait(const oFUNCTION<bool()>& _Task) const threadsafe;

	void SetNumStatusItems(int* _pItemWidths, size_t _NumItems) threadsafe;

	// returns true if the opacity transition when a window is show is finished
	// or false if it's still going or the window is hidden.
	bool IsOpaque() const threadsafe;

	void Destroy() threadsafe;

	// This is threadsafe in the sense it can be called from a different thread,
	// but appropriate locking is required if the desc here has to do with 
	// map/unmap implementations.
	void SetDesc(const oGUI_WINDOW_DESC& _Desc, bool _Force = false) threadsafe;

	// Sets the cursor state according to current state of this oWinWindow 
	// including the Desc and CursorVisible. Because of how SetCapture() works, 
	// this needs to be called from more than one place, so that's why it's broken
	// out as a function.
	void WTSetCursor(bool _IsInClientArea);
	void WTTriggerAction(const oGUI_ACTION_DESC& _Action);
	void WTTriggerActionByCopy(oGUI_ACTION_DESC _Action); // by copy so dipatching one of these retains the event
	void WTTriggerEvent(const oGUI_EVENT_DESC& _Event);
	void WTTriggerEventByCopy(oGUI_EVENT_DESC _Event); // by copy
	bool WTTriggerEventAndWait(oGUI_EVENT_DESC _Event); // by copy
	void WTRun();

	// Main call for modifying window state as executed on windows thread
	void WTSetDesc(const oGUI_WINDOW_DESC& _NewDesc, bool _Force = false);
};

#define oKEEP_FOCUS(_hWnd) SetTimer(_hWnd, 0x12300011, 1500, PeriodicallySetFocus)
static VOID CALLBACK PeriodicallySetFocus(HWND _hWnd, UINT _uMsg, UINT_PTR _idEvent, DWORD _dwTime)
{
	if (oWinGetState(_hWnd) == oGUI_WINDOW_FULLSCREEN_COOPERATIVE && oWinIsAlwaysOnTop(_hWnd))
	{
		oWinSetFocus(_hWnd);
		oKEEP_FOCUS(_hWnd);
	}
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
	, CursorVisible(true)
{
	*_pSuccess = false;
	ActionHooks.reserve(8);
	EventHooks.reserve(8);

	CachedWTRun = oBIND(&oWinWindow::WTRun, this);

	if (!oDispatchQueueCreatePrivate(_Init.WindowThreadDebugName, 500, &MessageQueue))
		return; // pass through error

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
		oSparseSet(EventHooks, _Init.EventHook);

	if (_Init.ActionHook)
		oSparseSet(ActionHooks, _Init.ActionHook);

	if (!DispatchAndWait([=]() -> bool
	{
		MessagePumpThreadID = oStd::this_thread::get_id();
		if (!oWinCreate(&hWnd, _Init.WinDesc.ClientPosition, _Init.WinDesc.ClientSize, StaticWndProc, this))
			return false; // pass through error

		if (Desc.AllowTouch)
			oVB(RegisterTouchWindow(hWnd, 0));

		void* oLoadInvisibleIcon();
		void* oLoadStandardIcon();

		if (!oWinGetIcon(hWnd))
		{
			// @oooii-tony: Why does this 0-alpha image come out opaque?
			//oWinSetIcon(hWnd, (HICON)oLoadInvisibleIcon(), true);
		}

		if (!oWinGetIcon(hWnd, true))
			oWinSetIcon(hWnd, (HICON)oLoadStandardIcon(), true);

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

		return true;
	}))
		return; // pass through error

	// Kick off the message pump, the window and public API is "live" at this 
	// point.
	MessageQueue->Dispatch(CachedWTRun);

	if (oSTRVALID(_Init.WindowTitle))
		SetTitle("%s", _Init.WindowTitle);

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
	Destroy();
}

bool oWindowCreate(const oWINDOW_INIT& _Init, threadsafe oWindow** _ppWindow)
{
	bool success = false;
	oCONSTRUCT(_ppWindow, oWinWindow(_Init, &success));
	return success;
}

const oGUID& oGetGUID(const threadsafe oWindow* const threadsafe*)
{
	// {0AA1837F-CAFD-427B-BF50-307D100974EE}
	static const oGUID guid = { 0xaa1837f, 0xcafd, 0x427b, { 0xbf, 0x50, 0x30, 0x7d, 0x10, 0x9, 0x74, 0xee } };
	return guid;
}

const oGUID& oGetGUID(const threadsafe oWinWindow* const threadsafe*)
{
	// {302277C1-B1D5-44FD-82F8-30115401A6FA}
	static const oGUID guid = { 0x302277c1, 0xb1d5, 0x44fd, { 0x82, 0xf8, 0x30, 0x11, 0x54, 0x1, 0xa6, 0xfa } };
	return guid;
}

bool oWinWindow::QueryInterface(const oGUID& _InterfaceID, threadsafe void** _ppInterface) threadsafe
{
	*_ppInterface = nullptr;

	if (_InterfaceID == oGetGUID<oInterface>() || _InterfaceID == oGetGUID<oWindow>() || _InterfaceID == oGetGUID<oWinWindow>())
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

	return !!*_ppInterface ? true : oErrorSetLast(oERROR_NOT_FOUND);
}

bool oWinWindow::IsOpen() const threadsafe
{
	return !Closed.IsSet();
}

bool oWinWindow::WaitUntilClosed(unsigned int _TimeoutMS) const threadsafe
{
	return Closed.Wait(_TimeoutMS);
}

bool oWinWindow::WaitUntilOpaque(unsigned int _TimeoutMS) const threadsafe
{
	oBackoff bo;
	unsigned int Now = oTimerMS();
	unsigned int Then = Now + _TimeoutMS;
	while (!IsOpaque())
	{
		bo.Pause();
		Now = oTimerMS();
		if (_TimeoutMS != oInfiniteWait && Now > Then)
			return oErrorSetLast(oERROR_TIMEOUT);
	}

	return true;
}

bool oWinWindow::Close(bool _AskFirst) threadsafe
{
	if (_AskFirst)
	{
		oStd::promise<bool> Promise;
		oStd::future<bool> Future = Promise.get_future();

		auto f = [&]
		{
			SendMessage(hWnd, WM_CLOSE, 0, 0);
			Promise.set_value(Closed.IsSet());
		};

		if (IsWindowThread())
			f();
		else
			Dispatch(f);
		bool ShouldClose = false;
		Future.get(&ShouldClose);
		return ShouldClose;
	}

	// Lock out anyone destroying until closed is in a stable state.
	oLockGuard<oRecursiveMutex> lock(DestroyMutex);
	Closed.Set();
	return true;
}

bool oWinWindow::IsWindowThread() const threadsafe
{
	return oStd::this_thread::get_id() == This()->MessagePumpThreadID; // safe because the value is immutable
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
	memcpy(&This()->TestPendingDesc, &This()->PendingDesc, sizeof(This()->TestPendingDesc));
	*_ppDesc = &This()->PendingDesc;
	return true;
}

void oWinWindow::Unmap() threadsafe
{
	if (memcmp(&This()->TestPendingDesc, &This()->PendingDesc, sizeof(This()->TestPendingDesc)))
		SetDesc(This()->PendingDesc);
	PendingDescMutex.unlock();
}

char* oWinWindow::GetTitle(char* _StrDestination, size_t _SizeofStrDestination) const threadsafe
{
	return DispatchAndWait([=]() -> bool { return !!oWinGetText(_StrDestination, _SizeofStrDestination, hWnd); }) ? _StrDestination : nullptr;
}

void oWinWindow::SetTitleV(const char* _Format, va_list _Args) threadsafe
{
	oStringL title;
	oVPrintf(title, _Format, _Args);
	Dispatch([=] { oWinSetText(hWnd, title); });
}

char* oWinWindow::GetStatusText(char* _StrDestination, size_t _SizeofStrDestination, int _StatusSectionIndex) const threadsafe
{
	return DispatchAndWait([=] { return !!oWinStatusBarGetText(_StrDestination, _SizeofStrDestination, hStatusBar, _StatusSectionIndex); }) ? _StrDestination : nullptr;
}

void oWinWindow::SetStatusText(int _StatusSectionIndex, const char* _Format, va_list _Args) threadsafe
{
	oStringL title;
	oVPrintf(title, _Format, _Args);
	Dispatch([=] { oWinStatusBarSetText(hStatusBar, _StatusSectionIndex, oGUI_BORDER_FLAT, title); });
}

static void ConvertAcceleratorTable(ACCEL* _pAccels, const oGUI_HOTKEY_DESC_NO_CTOR* _pHotKeys, size_t _NumHotKeys)
{
	for (size_t i = 0; i < _NumHotKeys; i++)
	{
		ACCEL& a = _pAccels[i];
		const oGUI_HOTKEY_DESC_NO_CTOR& h = _pHotKeys[i];
		a.fVirt = FVIRTKEY;
		if (h.AltDown) a.fVirt |= FALT;
		if (h.CtrlDown) a.fVirt |= FCONTROL;
		if (h.ShiftDown) a.fVirt |= FSHIFT;
		a.key = TranslateX11KeyboardToVK(h.HotKey) & 0xffff;
		a.cmd = h.ID;
	}
}

static void ConvertAcceleratorTable(oGUI_HOTKEY_DESC_NO_CTOR* _pHotKeys, const ACCEL* _pAccels, size_t _NumHotKeys)
{
	for (size_t i = 0; i < _NumHotKeys; i++)
	{
		const ACCEL& a = _pAccels[i];
		oGUI_HOTKEY_DESC_NO_CTOR& h = _pHotKeys[i];
		oASSERT(a.fVirt & FVIRTKEY, "");
		h.HotKey = TranslateKeyToX11(a.key);
		h.AltDown = !!(a.fVirt & FALT);
		h.CtrlDown = !!(a.fVirt & FCONTROL);
		h.ShiftDown = !!(a.fVirt & FSHIFT);
		h.ID = a.cmd;
	}
}

void oWinWindow::SetHotKeys(const oGUI_HOTKEY_DESC_NO_CTOR* _pHotKeys, size_t _NumHotKeys) threadsafe
{
	oGUI_HOTKEY_DESC_NO_CTOR* pCopy = nullptr;
	if (_pHotKeys && _NumHotKeys)
	{
		pCopy = new oGUI_HOTKEY_DESC_NO_CTOR[_NumHotKeys];
		memcpy(pCopy, _pHotKeys, sizeof(oGUI_HOTKEY_DESC_NO_CTOR) * _NumHotKeys);
	}
	
	Dispatch([=]
	{
	if (hAccel)
	{
		DestroyAcceleratorTable(hAccel);
		hAccel = nullptr;
	}

	if (_pHotKeys)
	{
		ACCEL* pAccels = new ACCEL[_NumHotKeys];
			ConvertAcceleratorTable(pAccels, pCopy, _NumHotKeys);
		hAccel = CreateAcceleratorTable((LPACCEL)pAccels, oUInt(_NumHotKeys));
		delete [] pAccels;
			delete [] pCopy;
	}
	});
}

int oWinWindow::GetHotKeys(oGUI_HOTKEY_DESC_NO_CTOR* _pHotKeys, size_t _MaxNumHotKeys) const threadsafe
{
	oStd::promise<int> Promise;
	oStd::future<int> Future = Promise.get_future();

	This()->Dispatch([&]
	{
	if (!hAccel)
	{
		Promise.set_value(0);
		return;
	}

	int nHotKeys = CopyAcceleratorTable(hAccel, nullptr, 0);
	if (_pHotKeys && _MaxNumHotKeys)
	{
		if ((size_t)nHotKeys >= _MaxNumHotKeys)
		{
			Promise.set_error(oERROR_AT_CAPACITY, "Buffer not large enough");
			return;
		}

		ACCEL* pAccels = new ACCEL[nHotKeys];
		ConvertAcceleratorTable(_pHotKeys, pAccels, nHotKeys);
		delete [] pAccels;
	}

		Promise.set_value(nHotKeys);
	});
	
	int nHotKeys = 0;
	Future.get(&nHotKeys);
	return nHotKeys;
}

oStd::future<oRef<oImage>> oWinWindow::CreateSnapshot(int _Frame, bool _IncludeBorder) const threadsafe
{
	// @oooii-tony: Probably there's a better way to make promises std::move around
	// and thus not need to explicitly allocate it, but first get oWindow settled
	// and then revisit this.

	auto PromisedImage = std::make_shared<oStd::promise<oRef<oImage>>>();
	oStd::future<oRef<oImage>> Image = PromisedImage->get_future();
	
	This()->Dispatch([=]
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
				PromisedImage->take_last_error();
	});

	return Image;
}

void oWinWindow::Trigger(const oGUI_ACTION_DESC& _Action) threadsafe
{
	Dispatch(&oWinWindow::WTTriggerActionByCopy, This(), _Action);
}

void oWinWindow::WTTriggerAction(const oGUI_ACTION_DESC& _Action)
{
	if (DestroyMutex.try_lock())
	{
		if (IsOpen())
		{
			#ifdef _DEBUG
				if (Desc.Debug)
					oTRACE("hSource 0x%x %s", _Action.hSource, oAsString(_Action.Action));
			#endif

			oLockGuard<oRecursiveMutex> lock(ActionHooksMutex);
			oFOR(oGUI_ACTION_HOOK& hook, ActionHooks)
			{
				if (hook)
					hook(_Action);
			}
		}
		DestroyMutex.unlock();
	}
}

void oWinWindow::WTTriggerActionByCopy(oGUI_ACTION_DESC _Action)
{
	WTTriggerAction(_Action);
}

void oWinWindow::WTTriggerEvent(const oGUI_EVENT_DESC& _Event)
{
	if (_Event.Event == oGUI_CLOSED || DestroyMutex.try_lock())
	{
		if (IsOpen())
		{
			#ifdef _DEBUG
				if (Desc.Debug && _Event.Event != oGUI_IDLE)
					oTRACE("hSource 0x%p %s pos=%d,%d size=%d,%d", _Event.hSource, oAsString(_Event.Event), _Event.ClientPosition.x, _Event.ClientPosition.y, _Event.ClientSize.x, _Event.ClientSize.y);
			#endif

			oLockGuard<oRecursiveMutex> lock(EventHooksMutex);
			oFOR(oGUI_EVENT_HOOK& hook, EventHooks)
			{
				if (hook)
					hook(_Event);
			}
		}

		if (_Event.Event != oGUI_CLOSED)
			DestroyMutex.unlock();
	}
}

void oWinWindow::WTTriggerEventByCopy(oGUI_EVENT_DESC _Event)
{
	WTTriggerEvent(_Event);
}

bool oWinWindow::WTTriggerEventAndWait(oGUI_EVENT_DESC _Event)
{
	#ifdef _DEBUG
		if (Desc.Debug)
			oTRACE("hSource 0x%p %s pos=%d,%d size=%d,%d", _Event.hSource, oAsString(_Event.Event), _Event.ClientPosition.x, _Event.ClientPosition.y, _Event.ClientSize.x, _Event.ClientSize.y);
	#endif

	bool result = true;
	oLockGuard<oRecursiveMutex> lock(EventHooksMutex);
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
	MessageQueue->Dispatch(_Task);
	if (hWnd && !oWinIsWindowThread(hWnd))
		oWinWake(hWnd);
}

int oWinWindow::HookActions(const oGUI_ACTION_HOOK& _Hook) threadsafe
{
	oStd::promise<int> Promise;
	oStd::future<int> Future = Promise.get_future();
	Dispatch([&]
	{
		size_t index = 0;
		{
			oLockGuard<oRecursiveMutex> lock(ActionHooksMutex);
			index = oSparseSet(This()->ActionHooks, _Hook);
		}
		Promise.set_value(oUInt(index));

	});

	int HookID = oInvalid;
	Future.get(&HookID);
	return HookID;
}

void oWinWindow::UnhookActions(int _ActionHookID) threadsafe
{
	Dispatch([=]
	{
		oLockGuard<oRecursiveMutex> lock(ActionHooksMutex);
		if (_ActionHookID < oUInt(This()->ActionHooks.size()))
			This()->ActionHooks[_ActionHookID] = nullptr;
	});
}

int oWinWindow::HookEvents(const oGUI_EVENT_HOOK& _Hook) threadsafe
{
	oStd::promise<int> Promise;
	oStd::future<int> Future = Promise.get_future();
	
	Dispatch([&]
	{
		size_t index = 0;
		{
			oLockGuard<oRecursiveMutex> lock(EventHooksMutex);
			index = oSparseSet(This()->EventHooks, _Hook);
		}
		Promise.set_value(oUInt(index));
	});
	
	
	int HookID = oInvalid;
	Future.get(&HookID);
	return HookID;
}

void oWinWindow::UnhookEvents(int _EventHookID) threadsafe
{
	Dispatch([=]
	{
		oLockGuard<oRecursiveMutex> lock(EventHooksMutex);
		if (_EventHookID < oUInt(This()->EventHooks.size()))
			This()->EventHooks[_EventHookID] = nullptr;
	});
}

void oWinWindow::Destroy() threadsafe
{
	oLockGuard<oRecursiveMutex> lock(DestroyMutex);

	if (MessageQueue && MessageQueue->Joinable())
	{
		This()->Dispatch([=]
		{
			if (oGUIIsFullscreen(Desc.State))
			{
				oGUI_WINDOW_DESC copy = This()->Desc;
				copy.State = oGUI_WINDOW_RESTORED;
				This()->WTSetDesc(copy);
			}

			if (hWnd)
			{
				oWinSetOwner(hWnd, nullptr); // minimizes "randomly set focus to some window other than parent"
				DestroyWindow(hWnd);
				hWnd = nullptr;
			}
		});

		MessageQueue->Join();
	}
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
	Dispatch(&oWinWindow::WTSetDesc, This(), oBINDREF(_Desc), _Force);
}

void oWinWindow::SetNumStatusItems(int* _pItemWidths, size_t _NumItems) threadsafe
{
	size_t n = _NumItems;
	int* w = new int[n];
	memcpy(w, _pItemWidths, sizeof(int) * n);
	Dispatch([=]
	{
		oWinStatusBarSetNumItems(hStatusBar, w, n);
		delete [] _pItemWidths;
	});
}

bool oWinWindow::DispatchAndWait(const oFUNCTION<bool()>& _Task) const threadsafe
{
	oStd::promise<bool> Promise;
	oStd::future<bool> Future = Promise.get_future();

	This()->Dispatch([&]
	{
	if (_Task())
			Promise.set_value(true);
	else
			Promise.take_last_error();
	});

	bool Success = false;
	Future.get(&Success);
	return Success;
}

void oWinWindow::WTRun()
{
	oASSERT(IsWindowThread(), "Wrong thread");
	if (hWnd)
	{
		if (!oWinDispatchMessage(hWnd, hAccel, !Desc.EnableIdleEvent) && IsOpen() && Desc.EnableIdleEvent)
		{
			oGUI_EVENT_DESC ed;
			ed.Event = oGUI_IDLE;
			ed.hSource = (oGUI_WINDOW)hWnd;
			ed.ClientPosition = Desc.ClientPosition;
			ed.ClientSize = Desc.ClientSize;
			ed.State = Desc.State;
			WTTriggerEvent(ed);
		}

		MessageQueue->Dispatch(CachedWTRun);
	}

	else
		oTRACE("hWnd invalidated");
}

#define oDEBUG_WTSETDESC
#ifdef oDEBUG_WTSETDESC
#define WTSD_TRACE(_Format, ...) do { if (_NewDesc.Debug) { oTRACE(_Format, ## __VA_ARGS__); } } while(false)
#else
	#define WTSD_TRACE(_Format, ...)
#endif

#define CHANGED(x) ((_NewDesc.x != Desc.x) || _Force)
void oWinWindow::WTSetDesc(const oGUI_WINDOW_DESC& _NewDesc, bool _Force)
{
	if (CHANGED(AlwaysOnTop))
	{
		WTSD_TRACE("== oWinSetAlwaysOnTop(%s) ==", oAsString(_NewDesc.AlwaysOnTop));
		oVERIFY(oWinSetAlwaysOnTop(hWnd, _NewDesc.AlwaysOnTop));
	}

	if (CHANGED(hOwner))
	{
		PendingDesc.hOwner = _NewDesc.hOwner;
		WTSD_TRACE("== oWinSetOwner(this, %p) ==", _NewDesc.hOwner);
		oWinSetOwner(hWnd, (HWND)_NewDesc.hOwner);
	}

	if (CHANGED(Enabled))
	{
		WTSD_TRACE("== oWinSetEnabled(%s) ==", oAsString(_NewDesc.Enabled));
		oVERIFY(oWinEnable(hWnd, _NewDesc.Enabled));
	}

	const bool kGoingToFullscreen = oGUIIsFullscreen(_NewDesc.State) && !oGUIIsFullscreen(Desc.State);
	const bool kGoingToWindowed = oGUIIsFullscreen(Desc.State) && !oGUIIsFullscreen(_NewDesc.State);

	// Extended Frame is Menu, StatusBar, (maybe toolbar or coolbar soon)
	const bool kHasExtendedFrame = _NewDesc.Style >= oGUI_WINDOW_FIXED && !oGUIIsFullscreen(_NewDesc.State);

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
		
		WTSD_TRACE("== oGUI_TO_FULLSCREEN ==");
		oGUI_EVENT_DESC ed;
		ed.Event = oGUI_TO_FULLSCREEN;
		ed.hSource = (oGUI_WINDOW)hWnd;
		ed.ClientPosition = _NewDesc.ClientPosition;
		ed.ClientSize = _NewDesc.ClientSize;
		//ed.ScreenSize = ??; // read from a reg setting... if not use cur disp val
		ed.State = _NewDesc.State;
		if (!WTTriggerEventAndWait(ed))
		{
			oGUI_WINDOW_DESC copy = _NewDesc;
			copy.State = oGUI_WINDOW_RESTORED;
			WTSD_TRACE("== Recursive WTSetDesc() ==");
			WTSetDesc(copy);
			return;
		}

		if (_NewDesc.State == oGUI_WINDOW_FULLSCREEN_COOPERATIVE && _NewDesc.AlwaysOnTop)
		{
			// AlwaysOnTop solves top-ness for all windows except the taskbar, so 
			// keep focus, which covers the taskbar, but only do it for windows that
			// are fullscreen on the taskbar.
			if (oWinGetDisplayIndex(hWnd) == oDisplayGetPrimaryIndex())
				oKEEP_FOCUS(hWnd);
		}

		// Because oDXGISetFullscreenState flushes the message queue, we could end
		// up eating an oWinWake() message from another thread, so send another 
		// just in case...
		WTSD_TRACE("== oWinWake() ==");
		oWinWake(hWnd);

		oASSERT(!_NewDesc.HasCapture, "Not yet implemented");
	}

	else 
	{
		if (kGoingToWindowed)
		{
			WTSD_TRACE("== oGUI_FROM_FULLSCREEN ==");
			oGUI_EVENT_DESC ed;
			ed.Event = oGUI_FROM_FULLSCREEN;
			ed.hSource = (oGUI_WINDOW)hWnd;
			ed.ClientPosition = _NewDesc.ClientPosition;
			ed.ClientSize = _NewDesc.ClientSize;
			//ed.ScreenSize = ??; // read from a reg setting... if not use cur disp val (though this should be current client size)
			ed.State = _NewDesc.State;
			WTTriggerEvent(ed);
			
			WTSD_TRACE("== oWinWake() ==");
			oWinWake(hWnd);

			Desc.State = PreFullscreenDesc.State; // prevents infinite recursion
			WTSD_TRACE("== Recursive WTSetDesc() ==");
			WTSetDesc(PreFullscreenDesc);
		} 

		if (kHasExtendedFrame)
		{
			if (CHANGED(ShowMenu))
			{
				WTSD_TRACE("== SetMenu() ==", oAsString(_NewDesc.ShowMenu));
				oVB(SetMenu(hWnd, _NewDesc.ShowMenu ? hMenu : nullptr));
			}

			if (CHANGED(ShowStatusBar))
			{
				WTSD_TRACE("== oWinSetState(%s) ==", oAsString(_NewDesc.ShowStatusBar ? oGUI_WINDOW_RESTORED : oGUI_WINDOW_HIDDEN));
				oWinSetState(hStatusBar, _NewDesc.ShowStatusBar ? oGUI_WINDOW_RESTORED : oGUI_WINDOW_HIDDEN);
			}
		}

		// As documented in MS's SetParent() API, set style flags before
		// SetParent call
		bool EmbeddedStyle = oGUI_WINDOW_EMBEDDED == oWinGetStyle(hWnd);
		if (_NewDesc.hParent && Desc.Style != oGUI_WINDOW_EMBEDDED && _NewDesc.Style != oGUI_WINDOW_EMBEDDED)
		{
			RECT rParentLocal = oWinRectWH(_NewDesc.ClientPosition, _NewDesc.ClientSize);
			WTSD_TRACE("== oWinSetStyle(oGUI_WINDOW_EMBEDDED, ShowStatusBar=%s) xy=%d,%d wh=%dx%d ==", oAsString(_NewDesc.ShowStatusBar), _NewDesc.ClientPosition.x, _NewDesc.ClientPosition.y, _NewDesc.ClientSize.x, _NewDesc.ClientSize.y);
			oVERIFY(oWinSetStyle(hWnd, oGUI_WINDOW_EMBEDDED, _NewDesc.ShowStatusBar, &rParentLocal));
			EmbeddedStyle = true;
		}

		if (CHANGED(hParent))
		{
			WTSD_TRACE("== SetParent(%p) ==", _NewDesc.hParent);
			oVB(SetParent(hWnd, (HWND)_NewDesc.hParent));
			if (!Desc.hParent && _NewDesc.hParent)
			{
				RECT r;
				GetClientRect((HWND)_NewDesc.hParent, &r);
				WTSD_TRACE("== PostMessage(%p, WM_SIZE, SIZE_RESTORED, %dx%d) ==", _NewDesc.hParent, oWinRectW(r), oWinRectH(r));
				PostMessage((HWND)_NewDesc.hParent, WM_SIZE, SIZE_RESTORED, (LPARAM)MAKELONG(oWinRectW(r), oWinRectH(r)));
			}
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

			bool SizeChanged = !!memcmp(&rOld, &rClient, sizeof(RECT));
			if (CHANGED(Style) || CHANGED(ShowStatusBar) || SizeChanged)
			{
				WTSD_TRACE("== oWinSetStyle(%s, ShowStatusBar=%s, xy=%d,%d wh=%dx%d) ==", oAsString(_NewDesc.Style), oAsString(_NewDesc.ShowStatusBar), rClient.left, rClient.top, oWinRectW(rClient), oWinRectH(rClient));
				oVERIFY(oWinSetStyle(hWnd, _NewDesc.Style, _NewDesc.ShowStatusBar, &rClient));

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
			WTSD_TRACE("== oWinSetState(%s, HasFocus=%s) ==", oAsString(_NewDesc.State), oAsString(_NewDesc.HasFocus));
			oVERIFY(oWinSetState(hWnd, _NewDesc.State, _NewDesc.HasFocus));

			ShowTimestamp = oGUIIsVisible(_NewDesc.State) ? oTimer() : -1.0;
		}

		else if ((oWinHasFocus(hWnd) && !_NewDesc.HasFocus) || (!oWinHasFocus(hWnd) && _NewDesc.HasFocus) || _NewDesc.HasCapture)
		{
			WTSD_TRACE("== oWinSetFocus(%s) ==", oAsString(_NewDesc.HasFocus));
			oWinSetFocus(hWnd, _NewDesc.HasFocus);
		}

		if (CHANGED(HasCapture))
		{
			if (_NewDesc.HasCapture)
			{
				oASSERT(!_NewDesc.hParent, "For HasCapture to be true, the window must not have a parent (it must be a top-level window).");
				WTSD_TRACE("== SetCapture() ==");
				SetCapture(hWnd);
			}

			else
			{
				WTSD_TRACE("== ReleaseCapture() ==");
				ReleaseCapture();
			}
		}
	}

	if (CHANGED(CursorState))
	{
		int2 p;
		GetCursorPos((POINT*)&p);
		oVB(ScreenToClient(hWnd, (POINT*)&p));

		WTSetCursor(greater_than_equal(p, int2(0,0)) && less_than_equal(p, _NewDesc.ClientSize));
	}

	oLockGuard<oSharedMutex> lock(DescMutex);
	WTSD_TRACE("== WriteDesc ==");
	Desc = _NewDesc;
}

void oWinWindow::WTSetCursor(bool _IsInClientArea)
{
	if (_IsInClientArea || !Desc.ShowCursorOutsideClientArea)
		oWinCursorSetState(hWnd, CursorVisible ? Desc.CursorState : oGUI_CURSOR_NONE, hUserCursor);
	else
		oWinCursorSetVisible();
}

// X11 doesn't seem to support the idea of multiple mouse buttons down at 
// time of drag, so create a priority system, favoring the smallest index
// of button over large buttons.
static oKEYBOARD_KEY GetTopPriorityMouseButton(WPARAM _wParam)
{
	WPARAM Keys = GET_KEYSTATE_WPARAM(_wParam);
	if (Keys & MK_LBUTTON) return oKB_Pointer_Button_Left;
	if (Keys & MK_RBUTTON) return oKB_Pointer_Button_Right;
	if (Keys & MK_MBUTTON) return oKB_Pointer_Button_Middle;
	if (Keys & MK_XBUTTON1) return oKB_Pointer_Button_Back;
	if (Keys & MK_XBUTTON2) return oKB_Pointer_Button_Forward;
	return oKB_VoidSymbol;
}

static inline float3 Float3(LPARAM _lParam) { return float3((float)GET_X_LPARAM(_lParam), (float)GET_Y_LPARAM(_lParam), 0.0f); }
static inline float3 Float3(LPARAM _lParam, WPARAM _wParam) { return float3((float)GET_X_LPARAM(_lParam), (float)GET_Y_LPARAM(_lParam), (float)GET_WHEEL_DELTA_WPARAM(_wParam)); }
LRESULT oWinWindow::WndProc(HWND _hWnd, UINT _uMsg, WPARAM _wParam, LPARAM _lParam)
{
	if (Desc.Debug)
	{
		oStringXL s;
		oTRACE("%s", oWinParseWMMessage(s, s.capacity(), _hWnd, _uMsg, _wParam, _lParam));
	}

	// Call out the mouse events separately because there are so many of them that
	// do so similar a task.
	oKEYBOARD_KEY Key = oKB_VoidSymbol;
	oGUI_ACTION KeyAction;
	if (TranslateMouseButtonToX11(_uMsg, _wParam, &Key, &KeyAction))
	{
		oGUI_ACTION_DESC ad;
		ad.hSource = (oGUI_WINDOW)hWnd;
		ad.Action = KeyAction;
		ad.Key = Key;
		ad.PointerPosition = Float3(_lParam);
		WTTriggerAction(ad);
	}

	else switch (_uMsg)
	{
		case WM_ACTIVATE:
		{
			oGUI_EVENT_DESC ed;
			ed.hSource = (oGUI_WINDOW)hWnd;
			ed.Event = (_wParam == WA_INACTIVE) ? oGUI_DEACTIVATED : oGUI_ACTIVATED;
			ed.ClientPosition = Desc.ClientPosition;
			ed.ClientSize = Desc.ClientSize;
			ed.State = Desc.State;
			WTTriggerEvent(ed);
			break;
		}

		case WM_ERASEBKGND:
			if (!Desc.DefaultEraseBackground)
				return 1;
			break;
		
		case WM_ENTERSIZEMOVE:
		{
			oGUI_EVENT_DESC ed;
			ed.hSource = (oGUI_WINDOW)hWnd;
			ed.Event = (_wParam == SC_MOVE) ? oGUI_MOVING : oGUI_SIZING;
			ed.ClientPosition = Desc.ClientPosition;
			ed.ClientSize = Desc.ClientSize;
			ed.State = Desc.State;
			WTTriggerEvent(ed);
			return 0;
		}
		case WM_MOVE:
		{
			int2 NewPosition(GET_X_LPARAM(_lParam), GET_Y_LPARAM(_lParam));
			oGUI_EVENT_DESC ed;
			ed.hSource = (oGUI_WINDOW)hWnd;
			ed.Event = oGUI_MOVED;
			ed.ClientPosition = NewPosition;
			ed.ClientSize = Desc.ClientSize;
			ed.State = Desc.State;
			WTTriggerEvent(ed);
			oLockGuard<oSharedMutex> lock(DescMutex);
			PendingDesc.ClientPosition = Desc.ClientPosition = NewPosition;
			return 0;
		}
		case WM_SIZE:
		{
			oGUI_EVENT_DESC ed;
			ed.hSource = (oGUI_WINDOW)hWnd;
			ed.Event = oGUI_SIZING;
			ed.ClientPosition = Desc.ClientPosition;
			ed.ClientSize = Desc.ClientSize;
			ed.State = Desc.State;
			WTTriggerEvent(ed);

			int2 NewSize(GET_X_LPARAM(_lParam), GET_Y_LPARAM(_lParam));
			if (oGUI_WINDOW_HIDDEN != oWinGetState(hStatusBar))
			{
				SendMessage(hStatusBar, WM_SIZE, 0, 0);
				RECT rStatusBar;
				GetClientRect(hStatusBar, &rStatusBar);
				NewSize.y -= oWinRectH(rStatusBar);
			}

			ed.Event = oGUI_SIZED;
			ed.ClientSize = NewSize;
			ed.State = (_wParam == SIZE_MINIMIZED) ? oGUI_WINDOW_MINIMIZED : Desc.State;
			WTTriggerEvent(ed);
			oLockGuard<oSharedMutex> lock(DescMutex);
			PendingDesc.ClientSize = Desc.ClientSize = NewSize;
			return 0;
		}

		// @oooii-tony: All these should be treated the same if there's any reason
		// to override painting.
		case WM_PAINT:
		case WM_PRINT:
		case WM_PRINTCLIENT:
		{
			// Not doing the BeginPaint/EndPaint
			oGUI_EVENT_DESC ed;
			ed.hSource = (oGUI_WINDOW)hWnd;
			ed.Event = oGUI_PAINT;
			ed.ClientPosition = Desc.ClientPosition;
			ed.ClientSize = Desc.ClientSize;
			ed.State = Desc.State;
			WTTriggerEvent(ed);
			break;
		}
		case WM_SETCURSOR:
			WTSetCursor(LOWORD(_lParam) == HTCLIENT);
			break;

		case WM_COMMAND:
		{
			oGUI_ACTION_DESC ad;
			ad.hSource = (oGUI_WINDOW)hWnd;
			ad.SourceID = LOWORD(_wParam);

			if (!_lParam)
			{
				if (HIWORD(_wParam) == 1)
					ad.Action = oGUI_ACTION_HOTKEY;
				else
					ad.Action = oGUI_ACTION_MENU;
			}
			else
			{
				ad.hSource = (oGUI_WINDOW)_lParam;
				ad.Action = oGUI_ACTION_CONTROL_ACTIVATED;
				ad.ActionCode = HIWORD(_wParam);
			}

			WTTriggerAction(ad);
			return 0;
		}

		case WM_HSCROLL:
		{
			if (_lParam != 0)
			{
				oGUI_ACTION_DESC ad;
				ad.hSource = (oGUI_WINDOW)(_lParam);
				ad.SourceID = 0;
				switch (LOWORD(_wParam))
				{
				case TB_ENDTRACK:
					ad.Action = oGUI_ACTION_CONTROL_DEACTIVATED;
					break;
				default:
					ad.Action = oGUI_ACTION_CONTROL_ACTIVATED;
					break;
				}
				WTTriggerAction(ad);
			}
			return 0;
		}
		case WM_NOTIFY:
		{
			LRESULT lResult = FALSE;
			const NMHDR& nmhdr = *(const NMHDR*)_lParam;

			oGUI_ACTION_DESC ad;
			ad.hSource = (oGUI_WINDOW)nmhdr.hwndFrom;
			ad.SourceID = oInt(nmhdr.idFrom);
			ad.ActionCode = nmhdr.code;

			oGUI_CONTROL_TYPE type = oWinControlGetType(nmhdr.hwndFrom);
			switch (type)
			{
				case oGUI_CONTROL_TAB:
				{
					switch (ad.ActionCode)
					{
						case TCN_SELCHANGING: ad.Action = oGUI_ACTION_CONTROL_SELECTION_CHANGING; break;
						case TCN_SELCHANGE: ad.Action = oGUI_ACTION_CONTROL_SELECTION_CHANGED; break;
						default: break;
					}

					break;
				}

				case oGUI_CONTROL_BUTTON:
				{
					switch (ad.ActionCode)
					{
						case BN_CLICKED:
							ad.Action = oGUI_ACTION_CONTROL_ACTIVATED;
							break;
						default:
							break;
					}

					break;
				}

				default:
					break;
			}


			if (!oWinControlDefaultOnNotify(_hWnd, nmhdr, &lResult, type))
				WTTriggerAction(ad);
			return lResult;
		}

		case WM_GETDLGCODE:
			// this window is not a sub-class of any window, so allow all keys to come 
			// through.
			return DLGC_WANTALLKEYS;

		case WM_SYSKEYDOWN:
		{
			if (_wParam == VK_F1 && !Desc.AllowAltF1)
				return 0;
			if (_wParam == VK_F4 && !Desc.AllowAltF4)
				return 0;
			if (_wParam == VK_RETURN && Desc.AllowAltEnter)
				return 0;

			// No additional handling required for Alt-F4... Windows knows what to do
			switch (_wParam)
			{
				case VK_F1:
					CursorVisible = !CursorVisible;
					oWinCursorSetVisible(CursorVisible);
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

				// Pass ALT buttons through to regular key handling
				case VK_LMENU:
				case VK_MENU:
				case VK_RMENU:
				{
					oGUI_ACTION_DESC ad;
					ad.hSource = (oGUI_WINDOW)hWnd;
					ad.Action = oGUI_ACTION_KEY_DOWN;
					ad.Key = TranslateKeyToX11(_wParam);
					WTTriggerAction(ad);
					break;
				}

				default:
					break;
			}

			break;
		}
		case WM_SYSKEYUP:
			switch (_wParam)
			{
				// Pass ALT buttons through to regular key handling
				case VK_LMENU:
				case VK_MENU:
				case VK_RMENU:
				{
					oGUI_ACTION_DESC ad;
					ad.hSource = (oGUI_WINDOW)hWnd;
					ad.Action = oGUI_ACTION_KEY_UP;
					ad.Key = TranslateKeyToX11(_wParam);
					WTTriggerAction(ad);
					break;
				}

				default:
					break;
			}

			break;

		case WM_KEYDOWN:
		{
			oGUI_ACTION_DESC ad;
			ad.hSource = (oGUI_WINDOW)hWnd;
			ad.Action = oGUI_ACTION_KEY_DOWN;
			ad.Key = TranslateKeyToX11(_wParam);
			WTTriggerAction(ad);
			break;
		}
		case WM_KEYUP:
		{
			oGUI_ACTION_DESC ad;
			ad.hSource = (oGUI_WINDOW)hWnd;
			ad.Action = oGUI_ACTION_KEY_UP;
			ad.Key = TranslateKeyToX11(_wParam);
			WTTriggerAction(ad);
			break;
		}
		case WM_APPCOMMAND: // fired when a media KB button is pressed - does not get keyups
			if (GET_DEVICE_LPARAM(_lParam) == FAPPCOMMAND_KEY)
			{
				oGUI_ACTION_DESC ad;
				ad.hSource = (oGUI_WINDOW)hWnd;
				ad.Action = oGUI_ACTION_KEY_DOWN;
				ad.Key = TranslateAppCommandToX11Key(_lParam);
				WTTriggerAction(ad);
			}
			break;
		case WM_MOUSEMOVE:
		{	
			// NOTE: With Dell Touch Screen drivers installed WM_MOUSEMOVE is called continuously, even if the mouse
			// doesn't move. Uninstalling the drivers fixes this.
			oGUI_ACTION_DESC ad;
			ad.hSource = (oGUI_WINDOW)hWnd;
			ad.Action = oGUI_ACTION_POINTER_MOVE;
			ad.Key = GetTopPriorityMouseButton(_wParam);
			ad.PointerPosition = Float3(_lParam);

			if (Desc.HasCapture)
			{
				int2 P(GET_X_LPARAM(_lParam), GET_Y_LPARAM(_lParam));
				WTSetCursor(greater_than_equal(P, int2(0,0)) && less_than_equal(P, Desc.ClientSize));
			}

			WTTriggerAction(ad);
			break;
		}
		case WM_MOUSEWHEEL:
		{	// Treat the mouse wheel delta as the z axis (relative, not absolute position)
			oGUI_ACTION_DESC ad;
			ad.hSource = (oGUI_WINDOW)hWnd;
			ad.Action = oGUI_ACTION_POINTER_MOVE;
			ad.Key = GetTopPriorityMouseButton(_wParam);
			ad.PointerPosition = Float3(_lParam, _wParam);
			WTTriggerAction(ad);
			break;
		}
		case WM_TOUCH:
		{
			const UINT nTouches = __min(LOWORD(_wParam), MAX_TOUCHES);
			TOUCHINPUT inputs[MAX_TOUCHES];
			if (nTouches)
			{
				if (GetTouchInputInfo((HTOUCHINPUT)_lParam, nTouches, inputs, sizeof(TOUCHINPUT)))
				{
					oGUI_ACTION_DESC ad;
					ad.hSource = (oGUI_WINDOW)hWnd;
					ad.Action = oGUI_ACTION_KEY_DOWN;
					for (UINT i = 0; i < nTouches; i++)
					{
						ad.Key = TranslateTouchToX11(i);
						ad.PointerPosition = float3(inputs[i].x / 100.0f, inputs[i].y / 100.0f, 0.0f);
						WTTriggerAction(ad);
					}
					CloseTouchInputHandle((HTOUCHINPUT)_lParam);
				}
			}
			break;
		}
		case WM_CREATE:
		{
			CREATESTRUCT CS = *(CREATESTRUCT*)_lParam;
			hStatusBar = oWinStatusBarCreate(_hWnd, (HMENU)0x00005747); // 1337 for 'stat'
			oWinSetState(hStatusBar, Desc.ShowStatusBar ? oGUI_WINDOW_RESTORED : oGUI_WINDOW_HIDDEN);
			
			{
				int itemWidths[oCOUNTOF(Desc.StatusWidths)];
				oINIT_ARRAY(itemWidths, oInvalid);
				size_t i = 0; 
				for (; i < oCOUNTOF(Desc.StatusWidths); i++)
				{
					if (Desc.StatusWidths[i] != oInvalid)
						itemWidths[i] = Desc.StatusWidths[i];

					if (Desc.StatusWidths[i] == oInvalid)
					{
						i++;
						break;
					}
				}

				oWinStatusBarSetNumItems(hStatusBar, itemWidths, i);
			}

			hMenu = oWinMenuCreate(true);
			if (Desc.ShowMenu)
				SetMenu(_hWnd, hMenu);

			// this is a bit dangerous because it's not really true this hWnd is ready
			// for use, but we need to expose it consistently as a valid return value
			// from oWindow::QueryInterface so it can be accessed from oGUI_CREATING,
			// where it's known to be only semi-ready.
			hWnd = _hWnd; 

			oGUI_EVENT_DESC ed;
			ed.hSource = (oGUI_WINDOW)hWnd;
			ed.hMenu = (oGUI_MENU)hMenu;
			ed.hStatusBar = (oGUI_STATUSBAR)hStatusBar;
			ed.Event = oGUI_CREATING;
			ed.ClientPosition = Desc.ClientPosition;
			ed.ClientSize = Desc.ClientSize;
			ed.State = oWinGetState(_hWnd);
			if (!WTTriggerEventAndWait(ed))
				return -1;
			if (Desc.ShowMenu)
				SetMenu(_hWnd, hMenu);

			ShowTimestamp = oGUIIsVisible(ed.State) ? oTimer() : -1.0;
			break;
		}
		case WM_DISPLAYCHANGE:
		{
			oGUI_EVENT_DESC ed;
			ed.hSource = (oGUI_WINDOW)hWnd;
			ed.Event = oGUI_DISPLAY_CHANGED;
			ed.ClientPosition = Desc.ClientPosition;
			ed.ClientSize = Desc.ClientSize;
			ed.ScreenSize = int2(LOWORD(_lParam), HIWORD(_lParam));
			WTTriggerEvent(ed);
			break;
		}
		case WM_CAPTURECHANGED:
		{
			if ((HWND)_lParam != hWnd)
			{
				oGUI_EVENT_DESC ed;
				ed.hSource = (oGUI_WINDOW)hWnd;
				ed.Event = oGUI_LOST_CAPTURE;
				ed.ClientPosition = Desc.ClientPosition;
				ed.ClientSize = Desc.ClientSize;
				WTTriggerEvent(ed);
			}
			break;
		}
		case WM_CLOSE:
		{
			oGUI_EVENT_DESC ed;
			ed.hSource = (oGUI_WINDOW)hWnd;
			ed.Event = oGUI_CLOSING;
			ed.ClientPosition = Desc.ClientPosition;
			ed.ClientSize = Desc.ClientSize;
			if (WTTriggerEventAndWait(ed))
				Close(false);
			return 0;
		}
		case WM_DESTROY:
		{
			oGUI_EVENT_DESC ed;
			ed.hSource = (oGUI_WINDOW)hWnd;
			ed.Event = oGUI_CLOSED;
			ed.ClientPosition = Desc.ClientPosition;
			ed.ClientSize = Desc.ClientSize;
			WTTriggerEvent(ed);
			SetMenu(_hWnd, nullptr);
			oWinMenuDestroy(hMenu);
			if (hAccel)
				oVB(DestroyAcceleratorTable(hAccel));
			return 0;
		}
		default:
			break;
	}

	return DefWindowProc(_hWnd, _uMsg, _wParam, _lParam);
}
