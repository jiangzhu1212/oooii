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
#include <oPlatform/oWindow.h>
#include <oPlatform/oGUIMenu.h>
#include <oPlatform/Windows/oGDI.h>
#include <oPlatform/Windows/oWinAsString.h>
#include <oPlatform/Windows/oWinCursor.h>
#include <oPlatform/Windows/oWinRect.h>
#include <oPlatform/Windows/oWinStatusBar.h>
#include <oPlatform/Windows/oWinWindowing.h>
#include <oConcurrency/event.h>
#include <oConcurrency/mutex.h>
#include <oStd/backoff.h>
#include <vector>
#include <windowsx.h>

static bool kForceDebug = false;

#define DISPATCH(_SimpleFunction) do { DispatchInternal(std::move([=] { _SimpleFunction; })); } while(false)

template<typename HookT, typename ParamT>
class HookManager
{
public:
	typedef HookT hook_type;
	typedef ParamT param_type;
	typedef oConcurrency::recursive_mutex mutex_t;
	typedef oConcurrency::lock_guard<mutex_t> lock_t;

	HookManager() { Hooks.reserve(8); }

	int Hook(const hook_type& _Hook) threadsafe
	{
		lock_t lock(HooksMutex);
		return static_cast<int>(oStd::sparse_set(oThreadsafe(Hooks), _Hook));
	}

	void Unhook(int _hHook) threadsafe
	{
		lock_t lock(HooksMutex);
		oStd::ranged_set(oThreadsafe(Hooks), _hHook, nullptr);
	}

	void Visit(const param_type& _Param) threadsafe
	{
		lock_t lock(HooksMutex);
		oFOR(hook_type& hook, oThreadsafe(Hooks))
			if (hook)
				hook(_Param);
	}

private:
	mutex_t HooksMutex;
	std::vector<hook_type> Hooks;
};

struct oWinWindow : oWindow
{
	oDEFINE_REFCOUNT_INTERFACE(RefCount);
	oDEFINE_NOOP_QUERYINTERFACE();
	oDECLARE_WNDPROC(oWinWindow);

	oWinWindow(const oWINDOW_INIT& _Init, bool* _pSuccess);
	~oWinWindow();
	
	// Basic API
	oGUI_WINDOW GetNativeHandle() const threadsafe override;
	int GetDisplayIndex() const override;
	bool IsWindowThread() const threadsafe override;

	// Client Position/Size API
	void SetShape(const oGUI_WINDOW_SHAPE_DESC& _Shape) threadsafe override;
	oGUI_WINDOW_SHAPE_DESC GetShape() const override;

	// Border/Decoration API
	void SetIcon(oGUI_ICON _hIcon) threadsafe override;
	oGUI_ICON GetIcon() const override;
	void SetUserCursor(oGUI_CURSOR _hCursor) threadsafe override;
	oGUI_CURSOR GetUserCursor() const override;
	void SetClientCursorState(oGUI_CURSOR_STATE _State) threadsafe override;
	oGUI_CURSOR_STATE GetClientCursorState() const override;
	void SetTitleV(const char* _Format, va_list _Args) threadsafe override;
	char* GetTitle(char* _StrDestination, size_t _SizeofStrDestination) const override;
	void SetNumStatusSections(const int* _pStatusSectionWidths, size_t _NumStatusSections) threadsafe override;
	int GetNumStatusSections(int* _pStatusSectionWidths = nullptr, size_t _MaxNumStatusSectionWidths = 0) const override;
	void SetStatusTextV(int _StatusSectionIndex, const char* _Format, va_list _Args) threadsafe override;
	char* GetStatusText(char* _StrDestination, size_t _SizeofStrDestination, int _StatusSectionIndex) const override;
	void SetStatusIcon(int _StatusSectionIndex, oGUI_ICON _hIcon) threadsafe override;
	oGUI_ICON GetStatusIcon(int _StatusSectionIndex) const override;

	// Draw Order/Dependency API
	void SetParent(oWindow* _pParent) threadsafe override;
	oWindow* GetParent() const override;
	void SetOwner(oWindow* _pOwner) threadsafe override;
	oWindow* GetOwner() const override;
	void SetSortOrder(oGUI_WINDOW_SORT_ORDER _SortOrder) threadsafe override;
	oGUI_WINDOW_SORT_ORDER GetSortOrder() const override;
	void SetFocus(bool _Focus = true) threadsafe override;
	bool HasFocus() const override;


	// Extended Input API
	
	void SetDebug(bool _Debug = true) threadsafe override;
	bool GetDebug() const override;
	void SetAllowTouchActions(bool _Allow = true) threadsafe override;
	bool GetAllowTouchActions() const override;
	void SetClientDragToMove(bool _DragMoves = true) threadsafe override;
	bool GetClientDragToMove() const override;
	void SetAltF4Closes(bool _AltF4Closes = true) threadsafe override;
	bool GetAltF4Closes() const override;
	void SetEnabled(bool _Enabled) threadsafe override;
	bool GetEnabled() const override;
	void SetCapture(bool _Capture) threadsafe override;
	bool HasCapture() const override;
	void SetHotKeys(const oGUI_HOTKEY_DESC_NO_CTOR* _pHotKeys, size_t _NumHotKeys) threadsafe override;
	int GetHotKeys(oGUI_HOTKEY_DESC_NO_CTOR* _pHotKeys, size_t _MaxNumHotKeys) const override;

	// Observer API
	int HookActions(const oGUI_ACTION_HOOK& _Hook) threadsafe override;
	void UnhookActions(int _ActionHookID) threadsafe override;
	int HookEvents(const oGUI_EVENT_HOOK& _Hook) threadsafe override;
	void UnhookEvents(int _EventHookID) threadsafe override;

	// Execution API
	void Trigger(const oGUI_ACTION_DESC& _Action) threadsafe override;
	void Post(int _CustomEventCode, uintptr_t _Context) threadsafe override;
	void Dispatch(const oTASK& _Task) threadsafe override;
	oStd::future<oStd::intrusive_ptr<oImage>> CreateSnapshot(int _Frame = oInvalid, bool _IncludeBorder = false) threadsafe const override;
	void SetTimer(uintptr_t _Context, unsigned int _RelativeTimeMS) threadsafe override;
	void StopTimer(uintptr_t _Context) threadsafe override;
	void FlushMessages(bool _WaitForNext = false) override;
	void Quit() threadsafe override;

private:

	HWND hWnd;
	HACCEL hAccel;
	HANDLE hHeap;
	oGUI_CURSOR hUserCursor;
	oWINKEY_CONTROL_STATE ControlKeyState;

	oGUI_CURSOR_STATE ClientCursorState;
	oGUI_WINDOW_SORT_ORDER SortOrder;
	bool Captured;
	bool ClientDragToMove;
	bool Debug;
	bool AllowTouch;
	int2 CursorClientPosAtMouseDown;

	oGUI_WINDOW_SHAPE_DESC PriorShape;

	typedef HookManager<oGUI_ACTION_HOOK, oGUI_ACTION_DESC> ActionManager_t;
	typedef HookManager<oGUI_EVENT_HOOK, oGUI_EVENT_DESC> EventManager_t;

	ActionManager_t ActionHooks;
	EventManager_t EventHooks;

	oConcurrency::event Destroyed;
	oRefCount RefCount;

	oStd::intrusive_ptr<oWindow> Owner;
	oStd::intrusive_ptr<oWindow> Parent;

private:

	void DispatchInternal(oTASK&& _Task) threadsafe const { PostMessage(hWnd, oWM_DISPATCH, 0, (LPARAM)const_cast<threadsafe oWinWindow*>(this)->New<oTASK>(std::move(_Task))); }

	struct oScopedHeapLock
	{
		oScopedHeapLock(HANDLE _hHeap) : hHeap(_hHeap) { HeapLock(hHeap); }
		~oScopedHeapLock() { HeapUnlock(hHeap); }
		private: HANDLE hHeap;
	};

	template<typename T>
	T* New(const T& _Object) threadsafe { oScopedHeapLock lock(hHeap); return new (HeapAlloc(hHeap, HEAP_GENERATE_EXCEPTIONS, sizeof(T))) T(_Object); }
	
	template<typename T>
	T* New(T&& _Object) threadsafe { oScopedHeapLock lock(hHeap); return new (HeapAlloc(hHeap, HEAP_GENERATE_EXCEPTIONS, sizeof(T))) T(std::move(_Object)); }

	template<typename T>
	void Delete(T* _pObject) threadsafe { _pObject->~T(); oScopedHeapLock lock(hHeap); HeapFree(hHeap, HEAP_GENERATE_EXCEPTIONS, _pObject); }

	template<typename T>
	T* NewArray(size_t _NumObjects) threadsafe
	{
		void* p = nullptr;
		{
			oScopedHeapLock lock(hHeap);
			p = HeapAlloc(hHeap, HEAP_GENERATE_EXCEPTIONS, sizeof(T) * _NumObjects + sizeof(size_t));
		}
		*(size_t*)p = _NumObjects;

		T* pObjects = (T*)oStd::byte_add(p, sizeof(size_t));
		for (size_t i = 0; i < _NumObjects; i++)
			pObjects[i] = std::move(T());
		return pObjects;
	}

	template<typename T>
	void DeleteArray(T* _pObjects) threadsafe
	{
		size_t* pNumObjects = (size_t*)(oStd::byte_sub(_pObjects, sizeof(size_t)));
		*pNumObjects;
		for (size_t i = 0; i < *pNumObjects; i++)
			_pObjects[i].~T();
		oScopedHeapLock lock(hHeap);
		HeapFree(hHeap, HEAP_GENERATE_EXCEPTIONS, pNumObjects);
	}

	char* NewString(const char* _Format, va_list _Args) threadsafe
	{
		const static size_t kStartLength = 64;
		char* s = nullptr;
		{
			oScopedHeapLock lock(hHeap);
			s = (char*)HeapAlloc(hHeap, HEAP_GENERATE_EXCEPTIONS, kStartLength);
		}

		int len = oStd::vsnprintf(s, kStartLength, _Format, _Args);
		if (len >= kStartLength)
		{
			oScopedHeapLock lock(hHeap);
			Delete(s);
			s = (char*)HeapAlloc(hHeap, HEAP_GENERATE_EXCEPTIONS, len + 1);
			oStd::vsnprintf(s, len + 1, _Format, _Args);
		}

		return s;
	}

	void DeleteString(char* _String) threadsafe
	{
		oScopedHeapLock lock(hHeap);
		HeapFree(hHeap, HEAP_GENERATE_EXCEPTIONS, _String);
	}

	bool InitWindow(const oWINDOW_INIT& _Init);
	void TriggerGenericEvent(oGUI_EVENT _Event, oGUI_WINDOW_SHAPE_DESC* _pShape = nullptr);
	void SetCursor();
	bool HandleInput(HWND _hWnd, UINT _uMsg, WPARAM _wParam, LPARAM _lParam, LRESULT* _pLResult);
	bool HandleSizing(HWND _hWnd, UINT _uMsg, WPARAM _wParam, LPARAM _lParam, LRESULT* _pLResult);
	bool HandleMisc(HWND _hWnd, UINT _uMsg, WPARAM _wParam, LPARAM _lParam, LRESULT* _pLResult);
	bool HandleLifetime(HWND _hWnd, UINT _uMsg, WPARAM _wParam, LPARAM _lParam, LRESULT* _pLResult);
};

bool oWinWindow::InitWindow(const oWINDOW_INIT& _Init)
{
	// this->hWnd assigned in WM_CREATE
	if (!oWinCreate(nullptr, _Init.Title, _Init.Shape.Style, _Init.Shape.ClientPosition, _Init.Shape.ClientSize, StaticWndProc, (void*)&_Init, this))
		return false; // pass through error
	
	PriorShape = oWinGetShape(hWnd);

	// Initialize decoration

	if (_Init.hIcon)
		oWinSetIcon(hWnd, (HICON)_Init.hIcon);

	// Still have to set style here since oWinCreate is still unaware of menu/status bar
	oGUI_WINDOW_SHAPE_DESC InitShape;
	InitShape.State = _Init.Shape.State;
	InitShape.Style = _Init.Shape.Style;

	if (!oWinSetShape(hWnd, InitShape))
		return false; // pass through error

	return true;
}

oWinWindow::oWinWindow(const oWINDOW_INIT& _Init, bool* _pSuccess)
	: hWnd(nullptr)
	, hAccel(nullptr)
	, hHeap(HeapCreate(HEAP_GENERATE_EXCEPTIONS, oMB(1), 0))
	, hUserCursor(nullptr)
	, ClientCursorState(_Init.ClientCursorState)
	, SortOrder(_Init.SortOrder)
	, Captured(false)
	, ClientDragToMove(false)
	, Debug(false)
	, AllowTouch(false)
	, CursorClientPosAtMouseDown(oDEFAULT, oDEFAULT)
{
	*_pSuccess = false;

	if (_Init.ActionHook)
		ActionHooks.Hook(_Init.ActionHook);

	if (_Init.EventHook)
		EventHooks.Hook(_Init.EventHook);

	if (!InitWindow(_Init))
		return; // pass through error

	oWinWindow::SetSortOrder(SortOrder);
	oWinWindow::SetDebug(_Init.Debug);
	oWinWindow::SetAllowTouchActions(_Init.AllowTouch);
	oWinWindow::SetClientDragToMove(_Init.ClientDragToMove);
	oWinWindow::SetAltF4Closes(_Init.AltF4Closes);

	*_pSuccess = true;
}

oWinWindow::~oWinWindow()
{
	if (hWnd)
	{
		oWinDestroy(hWnd);
		Destroyed.wait();
	}

	if (hHeap)
		HeapDestroy(hHeap);
}

bool oWindowCreate(const oWINDOW_INIT& _Init, oWindow** _ppWindow)
{
	bool success = false;
	oCONSTRUCT(_ppWindow, oWinWindow(_Init, &success));
	return success;
}

oGUI_WINDOW oWinWindow::GetNativeHandle() const threadsafe
{
	return (oGUI_WINDOW)hWnd;
}

int oWinWindow::GetDisplayIndex() const
{
	oGUI_WINDOW_SHAPE_DESC s = GetShape();
	return oDisplayFindIndex(s.ClientPosition + s.ClientSize / 2);
}

bool oWinWindow::IsWindowThread() const threadsafe
{
	return oWinIsWindowThread(hWnd);
}

void oWinWindow::SetShape(const oGUI_WINDOW_SHAPE_DESC& _Shape) threadsafe
{
	DispatchInternal(std::move([=]
	{
		if (!oWinSetShape(hWnd, _Shape))
			oTRACE("ERROR: oWinSetShape: %s", oErrorGetLastString());
	}));
}

oGUI_WINDOW_SHAPE_DESC oWinWindow::GetShape() const
{
	return oWinGetShape(hWnd);
}

void oWinWindow::SetIcon(oGUI_ICON _hIcon) threadsafe
{
	DISPATCH(oWinSetIcon(hWnd, (HICON)_hIcon));
}

oGUI_ICON oWinWindow::GetIcon() const
{
	return (oGUI_ICON)oWinGetIcon(hWnd);
}

void oWinWindow::SetUserCursor(oGUI_CURSOR _hCursor) threadsafe
{
	DispatchInternal(std::move([=]
	{
		if (hUserCursor)
			DestroyCursor((HCURSOR)hUserCursor);
		hUserCursor = _hCursor;
	}));
}

oGUI_CURSOR oWinWindow::GetUserCursor() const
{
	return (oGUI_CURSOR)hUserCursor;
}

void oWinWindow::SetClientCursorState(oGUI_CURSOR_STATE _State) threadsafe
{
	DISPATCH(ClientCursorState = _State);
}

oGUI_CURSOR_STATE oWinWindow::GetClientCursorState() const
{
	return ClientCursorState;
}

void oWinWindow::SetTitleV(const char* _Format, va_list _Args) threadsafe
{
	char* pString = NewString(_Format, _Args);
	DispatchInternal(std::move([=]
	{
		oWinSetText(hWnd, pString);
		if (pString)
			DeleteString(pString);
	}));
}

char* oWinWindow::GetTitle(char* _StrDestination, size_t _SizeofStrDestination) const
{
	return oWinGetText(_StrDestination, _SizeofStrDestination, hWnd);
}

void oWinWindow::SetNumStatusSections(const int* _pStatusSectionWidths, size_t _NumStatusSections) threadsafe
{
	oWinWindow* w = const_cast<oWinWindow*>(this);
	int* pCopy = w->NewArray<int>(_NumStatusSections);
	memcpy(pCopy, _pStatusSectionWidths, _NumStatusSections * sizeof(int));
	DispatchInternal(std::move([=]
	{
		oWinStatusBarSetNumItems(oWinGetStatusBar(hWnd), pCopy, _NumStatusSections);
		if (pCopy)
			DeleteArray(pCopy);
	}));
}

int oWinWindow::GetNumStatusSections(int* _pStatusSectionWidths, size_t _MaxNumStatusSectionWidths) const
{
	return oWinStatusBarGetNumItems(oWinGetStatusBar(hWnd), _pStatusSectionWidths, _MaxNumStatusSectionWidths);
}

void oWinWindow::SetStatusTextV(int _StatusSectionIndex, const char* _Format, va_list _Args) threadsafe
{
	char* pString = NewString(_Format, _Args);
	DispatchInternal(std::move([=]
	{
		oWinStatusBarSetText(oWinGetStatusBar(hWnd), _StatusSectionIndex, oGUI_BORDER_FLAT, pString);
		if (pString)
			DeleteString(pString);
	}));
}

char* oWinWindow::GetStatusText(char* _StrDestination, size_t _SizeofStrDestination, int _StatusSectionIndex) const
{
	size_t len = 0;
	oWinWindow* w = const_cast<oWinWindow*>(this);
	oTASK* pTask = w->New<oTASK>(std::move([=,&len]
	{
		if (oWinStatusBarGetText(_StrDestination, _SizeofStrDestination, oWinGetStatusBar(hWnd), _StatusSectionIndex))
			len = strlen(_StrDestination);
	}));

	SendMessage(hWnd, oWM_DISPATCH, 0, (LPARAM)pTask);
	return (len && len <= _SizeofStrDestination) ? _StrDestination : nullptr;
}

void oWinWindow::SetStatusIcon(int _StatusSectionIndex, oGUI_ICON _hIcon) threadsafe
{
	DISPATCH(oWinStatusBarSetIcon(oWinGetStatusBar(hWnd), _StatusSectionIndex, (HICON)_hIcon));
}

oGUI_ICON oWinWindow::GetStatusIcon(int _StatusSectionIndex) const
{
	return (oGUI_ICON)oWinStatusBarGetIcon(oWinGetStatusBar(hWnd), _StatusSectionIndex);
}

void oWinWindow::SetParent(oWindow* _pParent) threadsafe
{
	if (_pParent)
		_pParent->Reference();

	DispatchInternal(std::move([=]
	{
		oASSERT(!Owner, "Can't have owner at same time as parent");
		Parent = std::move(oStd::intrusive_ptr<oWindow>(_pParent, false));
		oVERIFY(oWinSetParent(hWnd, _pParent ? (HWND)_pParent->GetNativeHandle() : nullptr));
	}));
}

oWindow* oWinWindow::GetParent() const
{
	return const_cast<oWinWindow*>(this)->Parent;
}

void oWinWindow::SetOwner(oWindow* _pOwner) threadsafe
{
	if (_pOwner)
		_pOwner->Reference();
	DispatchInternal(std::move([=]
	{
		oASSERT(!Parent, "Can't have parent at same time as owner");
		Owner = std::move(oStd::intrusive_ptr<oWindow>(_pOwner, false));
		oWinSetOwner(hWnd, _pOwner ? (HWND)_pOwner->GetNativeHandle() : nullptr);
	}));
}

oWindow* oWinWindow::GetOwner() const
{
	return const_cast<oWinWindow*>(this)->Owner;
}

void oWinWindow::SetSortOrder(oGUI_WINDOW_SORT_ORDER _SortOrder) threadsafe
{
	DispatchInternal(std::move([=]
	{
		this->SortOrder = _SortOrder;
		oVERIFY(oWinSetAlwaysOnTop(hWnd, _SortOrder != oGUI_WINDOW_SORTED));
		if (_SortOrder == oGUI_WINDOW_ALWAYS_ON_TOP_WITH_FOCUS)
			::SetTimer(hWnd, (UINT_PTR)&SortOrder, 500, nullptr);
		else
			::KillTimer(hWnd, (UINT_PTR)&SortOrder);
	}));
}

oGUI_WINDOW_SORT_ORDER oWinWindow::GetSortOrder() const
{
	return SortOrder;
}

void oWinWindow::SetFocus(bool _Focus) threadsafe
{
	DISPATCH(oWinSetFocus(hWnd));
}

bool oWinWindow::HasFocus() const
{
	return oWinHasFocus(hWnd);
}

void oWinWindow::SetDebug(bool _Debug) threadsafe
{
	DispatchInternal([=] { Debug = _Debug; });
}

bool oWinWindow::GetDebug() const
{
	return Debug;
}

void oWinWindow::SetAllowTouchActions(bool _Allow) threadsafe
{
	DispatchInternal([=]
	{
		oVERIFY(oWinRegisterTouchEvents(hWnd, _Allow));
		AllowTouch = _Allow;
	});
}

bool oWinWindow::GetAllowTouchActions() const
{
	return AllowTouch;
}

void oWinWindow::SetClientDragToMove(bool _DragMoves) threadsafe
{
	DispatchInternal([=] { ClientDragToMove = _DragMoves; });
}

bool oWinWindow::GetClientDragToMove() const
{
	return ClientDragToMove;
}

void oWinWindow::SetAltF4Closes(bool _AltF4Closes) threadsafe
{
	DISPATCH(oVERIFY(oWinAltF4Enable(hWnd, _AltF4Closes)));
}

bool oWinWindow::GetAltF4Closes() const
{
	return oWinAltF4IsEnabled(hWnd);
}

void oWinWindow::SetEnabled(bool _Enabled) threadsafe
{
	DISPATCH(oWinEnable(hWnd, _Enabled));
}

bool oWinWindow::GetEnabled() const
{
	return oWinIsEnabled(hWnd);
}

void oWinWindow::SetCapture(bool _Capture) threadsafe
{
	DispatchInternal([=]
	{
		if (_Capture)
			::SetCapture(hWnd);
		else
			ReleaseCapture();
	});
}

bool oWinWindow::HasCapture() const
{
	return hWnd == GetCapture();
}

void oWinWindow::SetHotKeys(const oGUI_HOTKEY_DESC_NO_CTOR* _pHotKeys, size_t _NumHotKeys) threadsafe
{
	oGUI_HOTKEY_DESC_NO_CTOR* pCopy = nullptr;
	if (_pHotKeys && _NumHotKeys)
	{
		pCopy = NewArray<oGUI_HOTKEY_DESC_NO_CTOR>(_NumHotKeys);
		memcpy(pCopy, _pHotKeys, sizeof(oGUI_HOTKEY_DESC_NO_CTOR) * _NumHotKeys);
	}

	DispatchInternal(std::move([=]
	{
		// Be explicit in member values because these are to modify values at time
		// of message processing, not message dispatch
		if (this->hAccel)
		{
			DestroyAcceleratorTable(this->hAccel);
			this->hAccel = nullptr;
		}

		if (pCopy)
		{
			ACCEL* pAccels = this->NewArray<ACCEL>(_NumHotKeys);
			oWinAccelFromHotKeys(pAccels, pCopy, _NumHotKeys);
			this->hAccel = CreateAcceleratorTable((LPACCEL)pAccels, oUInt(_NumHotKeys));
			this->DeleteArray(pAccels);
			this->DeleteArray(pCopy);
		}
	}));
}

int oWinWindow::GetHotKeys(oGUI_HOTKEY_DESC_NO_CTOR* _pHotKeys, size_t _MaxNumHotKeys) const
{
	int N = 0;
	oWinWindow* w = const_cast<oWinWindow*>(this);
	oTASK* pTask = w->New<oTASK>(std::move([=,&N]
	{
		if (hAccel && _pHotKeys && _MaxNumHotKeys)
		{
			int nHotKeys = CopyAcceleratorTable(hAccel, nullptr, 0);
			ACCEL* pAccels = w->NewArray<ACCEL>(nHotKeys);
			CopyAcceleratorTable(hAccel, pAccels, nHotKeys);
			int NumCopied = __min(nHotKeys, oInt(_MaxNumHotKeys));
			oWinAccelToHotKeys(_pHotKeys, pAccels, NumCopied);
			w->DeleteArray(pAccels);
			N = NumCopied;
		}
	}));

	SendMessage(hWnd, oWM_DISPATCH, 0, (LPARAM)pTask);
	return N;
}

int oWinWindow::HookActions(const oGUI_ACTION_HOOK& _Hook) threadsafe
{
	return ActionHooks.Hook(_Hook);
}

void oWinWindow::UnhookActions(int _ActionHookID) threadsafe
{
	ActionHooks.Unhook(_ActionHookID);
}

int oWinWindow::HookEvents(const oGUI_EVENT_HOOK& _Hook) threadsafe
{
	return EventHooks.Hook(_Hook);
}

void oWinWindow::UnhookEvents(int _EventHookID) threadsafe
{
	EventHooks.Unhook(_EventHookID);
}

void oWinWindow::Trigger(const oGUI_ACTION_DESC& _Action) threadsafe
{
	DispatchInternal(std::move(oBIND(&ActionManager_t::Visit, &ActionHooks, _Action))); // bind by copy
}

void oWinWindow::Post(int _CustomEventCode, uintptr_t _Context) threadsafe
{
	oGUI_EVENT_CUSTOM_DESC e((oGUI_WINDOW)hWnd, _CustomEventCode, _Context);
	DispatchInternal(oBIND(&EventManager_t::Visit, &EventHooks, e)); // bind by copy
}

void oWinWindow::Dispatch(const oTASK& _Task) threadsafe
{
	oTASK* pTask = New<oTASK>(_Task);
	PostMessage(hWnd, oWM_DISPATCH, 0, (LPARAM)pTask);
}

static bool oWinWaitUntilOpaque(HWND _hWnd, unsigned int _TimeoutMS)
{
	oStd::backoff bo;
	unsigned int Now = oTimerMS();
	unsigned int Then = Now + _TimeoutMS;
	while (!oWinIsOpaque(_hWnd))
	{
		bo.pause();
		Now = oTimerMS();
		if (_TimeoutMS != oInfiniteWait && Now > Then)
			return oErrorSetLast(std::errc::timed_out);
	}

	return true;
}

oStd::future<oStd::intrusive_ptr<oImage>> oWinWindow::CreateSnapshot(int _Frame, bool _IncludeBorder) threadsafe const
{
	auto PromisedImage = std::make_shared<oStd::promise<oStd::intrusive_ptr<oImage>>>();
	auto Image = PromisedImage->get_future();

	const_cast<threadsafe oWinWindow*>(this)->Dispatch([=]() mutable
	{
		bool success = oWinWaitUntilOpaque(hWnd, 20000);
		if (!success)
		{
			oGUI_WINDOW_SHAPE_DESC s = oWinGetShape(hWnd);
			if (oGUIIsVisible(s.State))
				oVERIFY(false); // pass through verification of Wait
			else
				oASSERT(false, "A non-hidden window timed out waiting to become opaque");
		}

		oStd::intrusive_ptr<oImage> Image;
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

void oWinWindow::SetTimer(uintptr_t _Context, unsigned int _RelativeTimeMS) threadsafe
{
	DISPATCH(::SetTimer(hWnd, (UINT_PTR)_Context, _RelativeTimeMS, nullptr));
}

void oWinWindow::StopTimer(uintptr_t _Context) threadsafe
{
	DISPATCH(::KillTimer(hWnd, (UINT_PTR)_Context));
}

void oWinWindow::FlushMessages(bool _WaitForNext)
{
	while (hWnd)
	{
		if (!oWinDispatchMessage(hWnd, hAccel, _WaitForNext))
		{
			switch (oErrorGetLast())
			{
				case std::errc::operation_canceled:
					return;
				case std::errc::operation_not_permitted:
					oThrowLastError();
					break;
				default:
					if (!_WaitForNext)
						return;
					break;
			}
		}
	}
}

void oWinWindow::Quit() threadsafe
{
	DispatchInternal([=] { PostQuitMessage(0); });
};

void oWinWindow::TriggerGenericEvent(oGUI_EVENT _Event, oGUI_WINDOW_SHAPE_DESC* _pShape)
{
	oGUI_EVENT_SHAPE_DESC e((oGUI_WINDOW)hWnd, _Event, oWinGetShape(hWnd));
	EventHooks.Visit(e);
	if (_pShape)
		*_pShape = e.Shape;
}

void oWinWindow::SetCursor()
{
	oGUI_CURSOR_STATE NewState = ClientCursorState;
	HCURSOR hCursor = oWinGetCursor(NewState, (HCURSOR)hUserCursor);
	::SetCursor(hCursor);
	oWinCursorSetVisible(NewState != oGUI_CURSOR_NONE);
}

bool oWinWindow::HandleLifetime(HWND _hWnd, UINT _uMsg, WPARAM _wParam, LPARAM _lParam, LRESULT* _pLResult)
{
	switch (_uMsg)
	{
		case WM_CREATE:
		{
			CREATESTRUCTA* cs = (CREATESTRUCTA*)_lParam;
			oWIN_CREATESTRUCT* wcs = (oWIN_CREATESTRUCT*)cs->lpCreateParams;
			const oWINDOW_INIT* pInit = (const oWINDOW_INIT*)wcs->pInit;
			oASSERT(pInit, "invalid init struct");

			// this is a bit dangerous because it's not really true this hWnd is 
			// ready for use, but we need to expose it consistently as a valid 
			// return value from GetNativeHandle() so it can be accessed from 
			// oGUI_CREATING, where it's known to be only semi-ready.
			hWnd = _hWnd;

			oGUI_EVENT_CREATE_DESC e((oGUI_WINDOW)_hWnd
				, (oGUI_STATUSBAR)oWinGetStatusBar(_hWnd)
				, (oGUI_MENU)oWinGetMenu(_hWnd)
				, wcs->Shape);
			EventHooks.Visit(e);
			break;
		}
	
		case WM_CLOSE:
		{
			// Don't allow DefWindowProc to destroy the window, put it all on client 
			// code.
			oGUI_EVENT_SHAPE_DESC e((oGUI_WINDOW)hWnd, oGUI_CLOSING, oWinGetShape(hWnd));
			EventHooks.Visit(e);
			*_pLResult = 0;
			return true;
		}

		case oWM_DESTROY:
			DestroyWindow(_hWnd);
			break;

		case WM_DESTROY:
		{
			TriggerGenericEvent(oGUI_CLOSED);
			hWnd = nullptr;

			if (hAccel)
				DestroyAcceleratorTable(hAccel);

			*_pLResult = 0;
			Destroyed.set();
			return true;
		}

		default:
			break;
	}

	return false;
}

bool oWinWindow::HandleMisc(HWND _hWnd, UINT _uMsg, WPARAM _wParam, LPARAM _lParam, LRESULT* _pLResult)
{
	switch (_uMsg)
	{
		case WM_SETCURSOR:
			if (LOWORD(_lParam) == HTCLIENT)
			{
				SetCursor();
				*_pLResult = TRUE;
				return true;
			}
			break;

		case WM_GETDLGCODE:
			// this window is not a sub-class of any window, so allow all keys to come 
			// through.
			*_pLResult = DLGC_WANTALLKEYS;
			return true;

		case WM_ACTIVATE:
			TriggerGenericEvent((_wParam == WA_INACTIVE) ? oGUI_DEACTIVATED : oGUI_ACTIVATED);
			break;

		// All these should be treated the same if there's any reason to override 
		// painting.
		case WM_PAINT: case WM_PRINT: case WM_PRINTCLIENT:
			TriggerGenericEvent(oGUI_PAINT);
			break;

		case WM_DISPLAYCHANGE:
			TriggerGenericEvent(oGUI_DISPLAY_CHANGED);
			break;

		case oWM_DISPATCH:
		{
			oTASK* pTask = (oTASK*)_lParam;
			(*pTask)();
			Delete(pTask);
			*_pLResult = 0;
			return true;
		}

		case WM_TIMER:
		{
			if (_wParam == (WPARAM)(&SortOrder))
				oWinSetFocus(_hWnd);
			else
			{
				oGUI_EVENT_TIMER_DESC e((oGUI_WINDOW)_hWnd, (uintptr_t)_wParam);
				EventHooks.Visit(e);
			}
			*_pLResult = 1;
			return true;
		}

		default:
			break;
	}

	return false;
}

bool oWinWindow::HandleSizing(HWND _hWnd, UINT _uMsg, WPARAM _wParam, LPARAM _lParam, LRESULT* _pLResult)
{
	*_pLResult = 0;

	switch (_uMsg)
	{
		case WM_ENTERSIZEMOVE:
			TriggerGenericEvent((_wParam == SC_MOVE) ? oGUI_MOVING : oGUI_SIZING);
			return true;

		case WM_MOVE:
		{
			//oTRACE("HWND 0x%x WM_MOVE: %dx%d", _hWnd, GET_X_LPARAM(_lParam), GET_Y_LPARAM(_lParam));

			oGUI_WINDOW_SHAPE_DESC s;
			TriggerGenericEvent(oGUI_MOVED, &s);
			PriorShape.ClientPosition = s.ClientPosition;
			return true;
		}

		case WM_SIZE:
		{
			//oTRACE("WM_SIZE: %dx%d", GET_X_LPARAM(_lParam), GET_Y_LPARAM(_lParam));

			if (!oWinIsTempChange(_hWnd))
			{
				oGUI_EVENT_SHAPE_DESC e((oGUI_WINDOW)hWnd, oGUI_SIZING, PriorShape);
				EventHooks.Visit(e);

				e.Type = oGUI_SIZED;
				e.Shape = oWinGetShape(_hWnd);
				EventHooks.Visit(e);
				PriorShape = e.Shape;
			}
			return true;
		}

		default:
			break;
	}

	return false;
}

bool oWinWindow::HandleInput(HWND _hWnd, UINT _uMsg, WPARAM _wParam, LPARAM _lParam, LRESULT* _pLResult)
{
	*_pLResult = 0;
	unsigned int Timestamp = (unsigned int)GetMessageTime();

	oGUI_ACTION_DESC Action;
	if (oWinKeyDispatchMessage(_hWnd, _uMsg, _wParam, _lParam, Timestamp, &ControlKeyState, &Action))
	{
		if (ClientDragToMove)
		{
			const oGUI_KEY CheckKey = GetSystemMetrics(SM_SWAPBUTTON) ? oGUI_KEY_MOUSE_RIGHT : oGUI_KEY_MOUSE_LEFT;

			if (Action.Key == CheckKey)
			{
				if (Action.Action == oGUI_ACTION_KEY_DOWN)
				{
					CursorClientPosAtMouseDown = oWinCursorGetPosition(_hWnd);
					::SetCapture(_hWnd);
				}

				else if (Action.Action == oGUI_ACTION_KEY_UP)
				{
					ReleaseCapture();
					CursorClientPosAtMouseDown = int2(oDEFAULT, oDEFAULT);
				}
			}
		}

		ActionHooks.Visit(Action);
		if (_uMsg != WM_MOUSEMOVE)
			return false;
	}

	if (oWinControlToAction(_hWnd, _uMsg, _wParam, _lParam, &Action, _pLResult))
	{
		ActionHooks.Visit(Action);
		return true;
	}

	switch (_uMsg)
	{
		case WM_MOUSEMOVE:
		{
			if (Captured)
			{
				RECT rClient;
				GetClientRect(_hWnd, &rClient);
				int2 P(GET_X_LPARAM(_lParam), GET_Y_LPARAM(_lParam));
				if (greater_than_equal(P, int2(0,0)) && less_than_equal(P, oWinRectSize(rClient)))
					SetCursor();
			}

			if (ClientDragToMove && CursorClientPosAtMouseDown.x != oDEFAULT)
			{
				POINT p;
				p.x = GET_X_LPARAM(_lParam);
				p.y = GET_Y_LPARAM(_lParam);
				ClientToScreen(_hWnd, &p);
				oGUI_WINDOW_SHAPE_DESC Shape;
				Shape.ClientPosition = int2(p.x, p.y) - CursorClientPosAtMouseDown;
				if (!oWinSetShape(_hWnd, Shape))
					oTRACE("ERROR: oWinSetShape: %s", oErrorGetLastString());
			}

			break;
		}

		case WM_CANCELMODE:
		{
			if (GetCapture() == hWnd)
			{
				ReleaseCapture();
				TriggerGenericEvent(oGUI_LOST_CAPTURE);
			}
			break;
		}

		case WM_CAPTURECHANGED:
		{
			if ((HWND)_lParam != _hWnd)
			{
				if (GetCapture() == hWnd)
					ReleaseCapture();
				TriggerGenericEvent(oGUI_LOST_CAPTURE);
			}
			break;
		}
	
		// WM_INPUT_DEVICE_CHANGE is parsed into a more reasonable messaging system
		// so use oWM_INPUT_DEVICE_CHANGE as its proxy.
		case oWM_INPUT_DEVICE_CHANGE:
		{
			oGUI_EVENT_INPUT_DEVICE_DESC e((oGUI_WINDOW)_hWnd
				, oGUI_INPUT_DEVICE_TYPE(LOWORD(_wParam))
				, oGUI_INPUT_DEVICE_STATUS(HIWORD(_wParam))
				, (const char*)_lParam);
			EventHooks.Visit(e);
			return true;
		}

		case oWM_SKELETON:
		{
			oGUI_ACTION_DESC a((oGUI_WINDOW)_hWnd, Timestamp, oGUI_ACTION_SKELETON, oGUI_INPUT_DEVICE_SKELETON, LOWORD(_wParam));
			a.hSkeleton = (oGUI_SKELETON)_lParam;
			ActionHooks.Visit(a);
			return 0;
		}

		case oWM_USER_CAPTURED:
			ActionHooks.Visit(oGUI_ACTION_DESC((oGUI_WINDOW)_hWnd, Timestamp, oGUI_ACTION_SKELETON_ACQUIRED, oGUI_INPUT_DEVICE_SKELETON, oInt(_wParam)));
			return 0;

		case oWM_USER_LOST:
			ActionHooks.Visit(oGUI_ACTION_DESC((oGUI_WINDOW)_hWnd, Timestamp, oGUI_ACTION_SKELETON_LOST, oGUI_INPUT_DEVICE_SKELETON, oInt(_wParam)));
			return 0;

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
							ActionHooks.Visit(a);
						}
						CloseTouchInputHandle((HTOUCHINPUT)_lParam);
					}
				}
				break;
			}
		#endif

		case WM_DROPFILES:
		{
			int2 p(0, 0);
			DragQueryPoint((HDROP)_wParam, (POINT*)&p);
			const int NumPaths = DragQueryFile((HDROP)_wParam, ~0u, nullptr, 0); 
			oStd::path_string* pPaths = new oStd::path_string[NumPaths];
			for (int i = 0; i < NumPaths; i++)
				DragQueryFile((HDROP)_wParam, i, const_cast<char*>(pPaths[i].c_str()), oUInt(pPaths[i].capacity()));
			DragFinish((HDROP)_wParam);

			oGUI_EVENT_DROP_DESC e((oGUI_WINDOW)_hWnd, pPaths, NumPaths, p);
			EventHooks.Visit(e);
			delete [] pPaths;
			return true;
		}

		default:
			break;
	}

	return false;
}

LRESULT oWinWindow::WndProc(HWND _hWnd, UINT _uMsg, WPARAM _wParam, LPARAM _lParam)
{
	if ((Debug || kForceDebug))
	{
		oStd::xlstring s;
		oTRACE("%s", oWinParseWMMessage(s, s.capacity(), &ControlKeyState, _hWnd, _uMsg, _wParam, _lParam));
	}

	LRESULT lResult = -1;
	if (HandleInput(_hWnd, _uMsg, _wParam, _lParam, &lResult))
		return lResult;

	if (HandleSizing(_hWnd, _uMsg, _wParam, _lParam, &lResult))
		return lResult;

	if (HandleMisc(_hWnd, _uMsg, _wParam, _lParam, &lResult))
		return lResult;

	if (HandleLifetime(_hWnd, _uMsg, _wParam, _lParam, &lResult))
		return lResult;

	return DefWindowProc(_hWnd, _uMsg, _wParam, _lParam);
}
