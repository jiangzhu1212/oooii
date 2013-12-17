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
#include <oGUI/window.h>
#include <oGUI/oGUIMenu.h>
#include <oGUI/Windows/oGDI.h>
#include <oGUI/Windows/oWinAsString.h>
#include <oGUI/Windows/oWinCursor.h>
#include <oGUI/Windows/oWinKey.h>
#include <oGUI/Windows/oWinRect.h>
#include <oGUI/Windows/oWinStatusBar.h>
#include <oGUI/Windows/oWinWindowing.h>
#include <oSurface/codec.h>
#include <oBase/backoff.h>
#include <vector>
#include <windowsx.h>
#include <Shellapi.h>

namespace ouro {

static bool kForceDebug = false;

#define DISPATCH(_SimpleFunction) do { dispatch_internal(std::move([=] { _SimpleFunction; })); } while(false)

template<typename HookT, typename ParamT>
class HookManager
{
public:
	typedef HookT hook_type;
	typedef ParamT param_type;
	typedef oStd::recursive_mutex mutex_t;
	typedef oStd::lock_guard<mutex_t> lock_t;

	HookManager() { Hooks.reserve(8); }

	int Hook(const hook_type& _Hook)
	{
		lock_t lock(HooksMutex);
		return static_cast<int>(sparse_set(Hooks, _Hook));
	}

	void Unhook(int _hHook)
	{
		lock_t lock(HooksMutex);
		ranged_set(Hooks, _hHook, nullptr);
	}

	void Visit(const param_type& _Param)
	{
		lock_t lock(HooksMutex);
		oFOR(hook_type& hook, Hooks)
			if (hook)
				hook(_Param);
	}

private:
	mutex_t HooksMutex;
	std::vector<hook_type> Hooks;
};

struct window_impl : window
{
	oDECLARE_WNDPROC(window_impl);

	window_impl(const init& _Init);
	~window_impl();
	
	// environmental API
	oGUI_WINDOW native_handle() const override;
	display::id display_id() const override;
	bool is_window_thread() const override;
	void debug(bool _Debug = true) override;
	bool debug() const override;
	void flush_messages(bool _WaitForNext = false) override;
	void quit() override;


	// shape API
	void shape(const oGUI_WINDOW_SHAPE_DESC& _Shape) override;
	oGUI_WINDOW_SHAPE_DESC shape() const override;

	// border/decoration API
	void icon(oGUI_ICON _hIcon) override;
	oGUI_ICON icon() const override;
	void user_cursor(oGUI_CURSOR _hCursor) override;
	oGUI_CURSOR user_cursor() const override;
	void client_cursor_state(oGUI_CURSOR_STATE _State) override;
	oGUI_CURSOR_STATE client_cursor_state() const override;
	void set_titlev(const char* _Format, va_list _Args) override;
	char* get_title(char* _StrDestination, size_t _SizeofStrDestination) const override;
	void set_num_status_sections(const int* _pStatusSectionWidths, size_t _NumStatusSections) override;
	int get_num_status_sections(int* _pStatusSectionWidths = nullptr, size_t _MaxNumStatusSectionWidths = 0) const override;
	void set_status_textv(int _StatusSectionIndex, const char* _Format, va_list _Args) override;
	char* get_status_text(char* _StrDestination, size_t _SizeofStrDestination, int _StatusSectionIndex) const override;
	void status_icon(int _StatusSectionIndex, oGUI_ICON _hIcon) override;
	oGUI_ICON status_icon(int _StatusSectionIndex) const override;

	// Draw Order/Dependency API
	void parent(const std::shared_ptr<basic_window>& _Parent) override;
	std::shared_ptr<basic_window> parent() const override;
	void owner(const std::shared_ptr<basic_window>& _Owner) override;
	std::shared_ptr<basic_window> owner() const override;
	void sort_order(oGUI_WINDOW_SORT_ORDER _SortOrder) override;
	oGUI_WINDOW_SORT_ORDER sort_order() const override;
	void focus(bool _Focus = true) override;
	bool has_focus() const override;


	// Extended Input API
	
	void allow_touch_actions(bool _Allow = true) override;
	bool allow_touch_actions() const override;
	void client_drag_to_move(bool _DragMoves = true) override;
	bool client_drag_to_move() const override;
	void alt_f4_closes(bool _AltF4Closes = true) override;
	bool alt_f4_closes() const override;
	void enabled(bool _Enabled) override;
	bool enabled() const override;
	void capture(bool _Capture) override;
	bool has_capture() const override;
	void set_hotkeys(const oGUI_HOTKEY_DESC_NO_CTOR* _pHotKeys, size_t _NumHotKeys) override;
	int get_hotkeys(oGUI_HOTKEY_DESC_NO_CTOR* _pHotKeys, size_t _MaxNumHotKeys) const override;

	// Observer API
	int hook_actions(const oGUI_ACTION_HOOK& _Hook) override;
	void unhook_actions(int _ActionHookID) override;
	int hook_events(const oGUI_EVENT_HOOK& _Hook) override;
	void unhook_events(int _EventHookID) override;

	// Execution API
	void trigger(const oGUI_ACTION_DESC& _Action) override;
	void post(int _CustomEventCode, uintptr_t _Context) override;
	void dispatch(const oTASK& _Task) override;
	oStd::future<std::shared_ptr<surface::buffer>> snapshot(int _Frame = oInvalid, bool _IncludeBorder = false) const override;
	void start_timer(uintptr_t _Context, unsigned int _RelativeTimeMS) override;
	void stop_timer(uintptr_t _Context) override;

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

	event Destroyed;

	std::shared_ptr<basic_window> Owner;
	std::shared_ptr<basic_window> Parent;

private:

	void dispatch_internal(oTASK&& _Task) const { PostMessage(hWnd, oWM_DISPATCH, 0, (LPARAM)const_cast<window_impl*>(this)->new_object<oTASK>(std::move(_Task))); }

	struct oScopedHeapLock
	{
		oScopedHeapLock(HANDLE _hHeap) : hHeap(_hHeap) { HeapLock(hHeap); }
		~oScopedHeapLock() { HeapUnlock(hHeap); }
		private: HANDLE hHeap;
	};

	template<typename T>
	T* new_object(const T& _Object) { oScopedHeapLock lock(hHeap); return new (HeapAlloc(hHeap, HEAP_GENERATE_EXCEPTIONS, sizeof(T))) T(_Object); }
	
	template<typename T>
	T* new_object(T&& _Object) { oScopedHeapLock lock(hHeap); return new (HeapAlloc(hHeap, HEAP_GENERATE_EXCEPTIONS, sizeof(T))) T(std::move(_Object)); }

	template<typename T>
	void delete_object(T* _pObject) { _pObject->~T(); oScopedHeapLock lock(hHeap); HeapFree(hHeap, HEAP_GENERATE_EXCEPTIONS, _pObject); }

	template<typename T>
	T* new_array(size_t _NumObjects)
	{
		void* p = nullptr;
		{
			oScopedHeapLock lock(hHeap);
			p = HeapAlloc(hHeap, HEAP_GENERATE_EXCEPTIONS, sizeof(T) * _NumObjects + sizeof(size_t));
		}
		*(size_t*)p = _NumObjects;

		T* pObjects = (T*)byte_add(p, sizeof(size_t));
		for (size_t i = 0; i < _NumObjects; i++)
			pObjects[i] = std::move(T());
		return pObjects;
	}

	template<typename T>
	void delete_array(T* _pObjects)
	{
		size_t* pNumObjects = (size_t*)(byte_sub(_pObjects, sizeof(size_t)));
		*pNumObjects;
		for (size_t i = 0; i < *pNumObjects; i++)
			_pObjects[i].~T();
		oScopedHeapLock lock(hHeap);
		HeapFree(hHeap, HEAP_GENERATE_EXCEPTIONS, pNumObjects);
	}

	char* new_string(const char* _Format, va_list _Args)
	{
		const static size_t kStartLength = 64;
		char* s = nullptr;
		{
			oScopedHeapLock lock(hHeap);
			s = (char*)HeapAlloc(hHeap, HEAP_GENERATE_EXCEPTIONS, kStartLength);
		}

		int len = ouro::vsnprintf(s, kStartLength, _Format, _Args);
		if (len >= kStartLength)
		{
			oScopedHeapLock lock(hHeap);
			delete_object(s);
			s = (char*)HeapAlloc(hHeap, HEAP_GENERATE_EXCEPTIONS, len + 1);
			ouro::vsnprintf(s, len + 1, _Format, _Args);
		}

		return s;
	}

	void delete_string(char* _String)
	{
		oScopedHeapLock lock(hHeap);
		HeapFree(hHeap, HEAP_GENERATE_EXCEPTIONS, _String);
	}

	void init_window(const init& _Init);
	void trigger_generic_event(oGUI_EVENT _Event, oGUI_WINDOW_SHAPE_DESC* _pShape = nullptr);
	void set_cursor();
	bool handle_input(HWND _hWnd, UINT _uMsg, WPARAM _wParam, LPARAM _lParam, LRESULT* _pLResult);
	bool handle_sizing(HWND _hWnd, UINT _uMsg, WPARAM _wParam, LPARAM _lParam, LRESULT* _pLResult);
	bool handle_misc(HWND _hWnd, UINT _uMsg, WPARAM _wParam, LPARAM _lParam, LRESULT* _pLResult);
	bool handle_lifetime(HWND _hWnd, UINT _uMsg, WPARAM _wParam, LPARAM _lParam, LRESULT* _pLResult);
};

void window_impl::init_window(const init& _Init)
{
	// this->hWnd assigned in WM_CREATE
	if (!oWinCreate(nullptr, _Init.title, _Init.shape.Style, _Init.shape.ClientPosition, _Init.shape.ClientSize, StaticWndProc, (void*)&_Init, this))
		oThrowLastError();
	
	PriorShape = oWinGetShape(hWnd);

	// Initialize decoration

	if (_Init.icon)
		oWinSetIcon(hWnd, (HICON)_Init.icon);

	// Still have to set style here since oWinCreate is still unaware of menu/status bar
	oGUI_WINDOW_SHAPE_DESC InitShape;
	InitShape.State = _Init.shape.State;
	InitShape.Style = _Init.shape.Style;

	if (!oWinSetShape(hWnd, InitShape))
		oThrowLastError();
}

window_impl::window_impl(const init& _Init)
	: hWnd(nullptr)
	, hAccel(nullptr)
	, hHeap(HeapCreate(HEAP_GENERATE_EXCEPTIONS, oMB(1), 0))
	, hUserCursor(nullptr)
	, ClientCursorState(_Init.cursor_state)
	, SortOrder(_Init.sort_order)
	, Captured(false)
	, ClientDragToMove(false)
	, Debug(false)
	, AllowTouch(false)
	, CursorClientPosAtMouseDown(oDEFAULT, oDEFAULT)
{
	if (_Init.action_hook)
		ActionHooks.Hook(_Init.action_hook);

	if (_Init.event_hook)
		EventHooks.Hook(_Init.event_hook);

	init_window(_Init);
	window_impl::sort_order(SortOrder);
	window_impl::debug(_Init.debug);
	window_impl::allow_touch_actions(_Init.allow_touch);
	window_impl::client_drag_to_move(_Init.client_drag_to_move);
	window_impl::alt_f4_closes(_Init.alt_f4_closes);
}

window_impl::~window_impl()
{
	if (hWnd)
	{
		oWinDestroy(hWnd);
		Destroyed.wait();
	}

	if (hHeap)
		HeapDestroy(hHeap);
}

std::shared_ptr<window> window::make(const init& _Init)
{
	return std::make_shared<window_impl>(_Init);
}

oGUI_WINDOW window_impl::native_handle() const
{
	return (oGUI_WINDOW)hWnd;
}

ouro::display::id window_impl::display_id() const
{
	oGUI_WINDOW_SHAPE_DESC s = shape();
	int2 center = s.ClientPosition + s.ClientSize / 2;
	return ouro::display::find(center.x, center.y);
}

bool window_impl::is_window_thread() const
{
	return oWinIsWindowThread(hWnd);
}

void window_impl::shape(const oGUI_WINDOW_SHAPE_DESC& _Shape)
{
	dispatch_internal(std::move([=]
	{
		if (!oWinSetShape(hWnd, _Shape))
			oTRACE("ERROR: oWinSetShape: %s", oErrorGetLastString());
	}));
}

oGUI_WINDOW_SHAPE_DESC window_impl::shape() const
{
	return oWinGetShape(hWnd);
}

void window_impl::icon(oGUI_ICON _hIcon)
{
	DISPATCH(oWinSetIcon(hWnd, (HICON)_hIcon, true));
	DISPATCH(oWinSetIcon(hWnd, (HICON)_hIcon, false));
}

oGUI_ICON window_impl::icon() const
{
	return (oGUI_ICON)oWinGetIcon(hWnd);
}

void window_impl::user_cursor(oGUI_CURSOR _hCursor)
{
	dispatch_internal(std::move([=]
	{
		if (hUserCursor)
			DestroyCursor((HCURSOR)hUserCursor);
		hUserCursor = _hCursor;
	}));
}

oGUI_CURSOR window_impl::user_cursor() const
{
	return (oGUI_CURSOR)hUserCursor;
}

void window_impl::client_cursor_state(oGUI_CURSOR_STATE _State)
{
	DISPATCH(ClientCursorState = _State);
}

oGUI_CURSOR_STATE window_impl::client_cursor_state() const
{
	return ClientCursorState;
}

void window_impl::set_titlev(const char* _Format, va_list _Args)
{
	char* pString = new_string(_Format, _Args);
	dispatch_internal(std::move([=]
	{
		oWinSetText(hWnd, pString);
		if (pString)
			delete_string(pString);
	}));
}

char* window_impl::get_title(char* _StrDestination, size_t _SizeofStrDestination) const
{
	return oWinGetText(_StrDestination, _SizeofStrDestination, hWnd);
}

void window_impl::set_num_status_sections(const int* _pStatusSectionWidths, size_t _NumStatusSections)
{
	window_impl* w = const_cast<window_impl*>(this);
	int* pCopy = w->new_array<int>(_NumStatusSections);
	memcpy(pCopy, _pStatusSectionWidths, _NumStatusSections * sizeof(int));
	dispatch_internal(std::move([=]
	{
		oWinStatusBarSetNumItems(oWinGetStatusBar(hWnd), pCopy, _NumStatusSections);
		if (pCopy)
			delete_array(pCopy);
	}));
}

int window_impl::get_num_status_sections(int* _pStatusSectionWidths, size_t _MaxNumStatusSectionWidths) const
{
	return oWinStatusBarGetNumItems(oWinGetStatusBar(hWnd), _pStatusSectionWidths, _MaxNumStatusSectionWidths);
}

void window_impl::set_status_textv(int _StatusSectionIndex, const char* _Format, va_list _Args)
{
	char* pString = new_string(_Format, _Args);
	dispatch_internal(std::move([=]
	{
		oWinStatusBarSetText(oWinGetStatusBar(hWnd), _StatusSectionIndex, oGUI_BORDER_FLAT, pString);
		if (pString)
			delete_string(pString);
	}));
}

char* window_impl::get_status_text(char* _StrDestination, size_t _SizeofStrDestination, int _StatusSectionIndex) const
{
	size_t len = 0;
	window_impl* w = const_cast<window_impl*>(this);
	oTASK* pTask = w->new_object<oTASK>(std::move([=,&len]
	{
		if (oWinStatusBarGetText(_StrDestination, _SizeofStrDestination, oWinGetStatusBar(hWnd), _StatusSectionIndex))
			len = strlen(_StrDestination);
	}));

	SendMessage(hWnd, oWM_DISPATCH, 0, (LPARAM)pTask);
	return (len && len <= _SizeofStrDestination) ? _StrDestination : nullptr;
}

void window_impl::status_icon(int _StatusSectionIndex, oGUI_ICON _hIcon)
{
	DISPATCH(oWinStatusBarSetIcon(oWinGetStatusBar(hWnd), _StatusSectionIndex, (HICON)_hIcon));
}

oGUI_ICON window_impl::status_icon(int _StatusSectionIndex) const
{
	return (oGUI_ICON)oWinStatusBarGetIcon(oWinGetStatusBar(hWnd), _StatusSectionIndex);
}

void window_impl::parent(const std::shared_ptr<basic_window>& _Parent)
{
	dispatch_internal(std::move([=]
	{
		oCHECK(!Owner, "Can't have owner at same time as parent");
		Parent = _Parent;
		oVERIFY(oWinSetParent(hWnd, Parent ? (HWND)Parent->native_handle() : nullptr));
	}));
}

std::shared_ptr<basic_window> window_impl::parent() const
{
	return const_cast<window_impl*>(this)->Parent;
}

void window_impl::owner(const std::shared_ptr<basic_window>& _Owner)
{
	dispatch_internal(std::move([=]
	{
		oCHECK(!Parent, "Can't have parent at same time as owner");
		Owner = _Owner;
		oWinSetOwner(hWnd, Owner ? (HWND)Owner->native_handle() : nullptr);
	}));
}

std::shared_ptr<basic_window> window_impl::owner() const
{
	return const_cast<window_impl*>(this)->Owner;
}

void window_impl::sort_order(oGUI_WINDOW_SORT_ORDER _SortOrder)
{
	dispatch_internal(std::move([=]
	{
		this->SortOrder = _SortOrder;
		oVERIFY(oWinSetAlwaysOnTop(hWnd, _SortOrder != oGUI_WINDOW_SORTED));
		if (_SortOrder == oGUI_WINDOW_ALWAYS_ON_TOP_WITH_FOCUS)
			::SetTimer(hWnd, (UINT_PTR)&SortOrder, 500, nullptr);
		else
			::KillTimer(hWnd, (UINT_PTR)&SortOrder);
	}));
}

oGUI_WINDOW_SORT_ORDER window_impl::sort_order() const
{
	return SortOrder;
}

void window_impl::focus(bool _Focus)
{
	DISPATCH(oWinSetFocus(hWnd));
}

bool window_impl::has_focus() const
{
	return oWinHasFocus(hWnd);
}

void window_impl::debug(bool _Debug)
{
	dispatch_internal([=] { Debug = _Debug; });
}

bool window_impl::debug() const
{
	return Debug;
}

void window_impl::allow_touch_actions(bool _Allow)
{
	dispatch_internal([=]
	{
		oVERIFY(oWinRegisterTouchEvents(hWnd, _Allow));
		AllowTouch = _Allow;
	});
}

bool window_impl::allow_touch_actions() const
{
	return AllowTouch;
}

void window_impl::client_drag_to_move(bool _DragMoves)
{
	dispatch_internal([=] { ClientDragToMove = _DragMoves; });
}

bool window_impl::client_drag_to_move() const
{
	return ClientDragToMove;
}

void window_impl::alt_f4_closes(bool _AltF4Closes)
{
	DISPATCH(oVERIFY(oWinAltF4Enable(hWnd, _AltF4Closes)));
}

bool window_impl::alt_f4_closes() const
{
	return oWinAltF4IsEnabled(hWnd);
}

void window_impl::enabled(bool _Enabled)
{
	DISPATCH(oWinEnable(hWnd, _Enabled));
}

bool window_impl::enabled() const
{
	return oWinIsEnabled(hWnd);
}

void window_impl::capture(bool _Capture)
{
	dispatch_internal([=]
	{
		if (_Capture)
			::SetCapture(hWnd);
		else
			ReleaseCapture();
	});
}

bool window_impl::has_capture() const
{
	return hWnd == GetCapture();
}

void window_impl::set_hotkeys(const oGUI_HOTKEY_DESC_NO_CTOR* _pHotKeys, size_t _NumHotKeys)
{
	oGUI_HOTKEY_DESC_NO_CTOR* pCopy = nullptr;
	if (_pHotKeys && _NumHotKeys)
	{
		pCopy = new_array<oGUI_HOTKEY_DESC_NO_CTOR>(_NumHotKeys);
		memcpy(pCopy, _pHotKeys, sizeof(oGUI_HOTKEY_DESC_NO_CTOR) * _NumHotKeys);
	}

	dispatch_internal(std::move([=]
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
			ACCEL* pAccels = this->new_array<ACCEL>(_NumHotKeys);
			oWinAccelFromHotKeys(pAccels, pCopy, _NumHotKeys);
			this->hAccel = CreateAcceleratorTable((LPACCEL)pAccels, oUInt(_NumHotKeys));
			this->delete_array(pAccels);
			this->delete_array(pCopy);
		}
	}));
}

int window_impl::get_hotkeys(oGUI_HOTKEY_DESC_NO_CTOR* _pHotKeys, size_t _MaxNumHotKeys) const
{
	int N = 0;
	window_impl* w = const_cast<window_impl*>(this);
	oTASK* pTask = w->new_object<oTASK>(std::move([=,&N]
	{
		if (hAccel && _pHotKeys && _MaxNumHotKeys)
		{
			int nHotKeys = CopyAcceleratorTable(hAccel, nullptr, 0);
			ACCEL* pAccels = w->new_array<ACCEL>(nHotKeys);
			CopyAcceleratorTable(hAccel, pAccels, nHotKeys);
			int NumCopied = __min(nHotKeys, oInt(_MaxNumHotKeys));
			oWinAccelToHotKeys(_pHotKeys, pAccels, NumCopied);
			w->delete_array(pAccels);
			N = NumCopied;
		}
	}));

	SendMessage(hWnd, oWM_DISPATCH, 0, (LPARAM)pTask);
	return N;
}

int window_impl::hook_actions(const oGUI_ACTION_HOOK& _Hook)
{
	return ActionHooks.Hook(_Hook);
}

void window_impl::unhook_actions(int _ActionHookID)
{
	ActionHooks.Unhook(_ActionHookID);
}

int window_impl::hook_events(const oGUI_EVENT_HOOK& _Hook)
{
	return EventHooks.Hook(_Hook);
}

void window_impl::unhook_events(int _EventHookID)
{
	EventHooks.Unhook(_EventHookID);
}

void window_impl::trigger(const oGUI_ACTION_DESC& _Action)
{
	dispatch_internal(std::move(oBIND(&ActionManager_t::Visit, &ActionHooks, _Action))); // bind by copy
}

void window_impl::post(int _CustomEventCode, uintptr_t _Context)
{
	oGUI_EVENT_CUSTOM_DESC e((oGUI_WINDOW)hWnd, _CustomEventCode, _Context);
	dispatch_internal(oBIND(&EventManager_t::Visit, &EventHooks, e)); // bind by copy
}

void window_impl::dispatch(const oTASK& _Task)
{
	oTASK* pTask = new_object<oTASK>(_Task);
	PostMessage(hWnd, oWM_DISPATCH, 0, (LPARAM)pTask);
}

static bool oWinWaitUntilOpaque(HWND _hWnd, unsigned int _TimeoutMS)
{
	backoff bo;
	unsigned int Now = ouro::timer::now_ms();
	unsigned int Then = Now + _TimeoutMS;
	while (!oWinIsOpaque(_hWnd))
	{
		bo.pause();
		Now = ouro::timer::now_ms();
		if (_TimeoutMS != oInfiniteWait && Now > Then)
			return oErrorSetLast(std::errc::timed_out);
	}

	return true;
}

oStd::future<std::shared_ptr<surface::buffer>> window_impl::snapshot(int _Frame, bool _IncludeBorder) const
{
	auto PromisedSnap = std::make_shared<oStd::promise<std::shared_ptr<surface::buffer>>>();
	auto Image = PromisedSnap->get_future();

	const_cast<window_impl*>(this)->dispatch([=]() mutable
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

		std::shared_ptr<surface::buffer> snap;
		void* buf = nullptr;
		size_t size = 0;
		oWinSetFocus(hWnd); // Windows doesn't do well with hidden contents.
		if (oGDIScreenCaptureWindow(hWnd, _IncludeBorder, malloc, &buf, &size, false))
		{
			snap = surface::decode(buf, size);
			free(buf);
		}

		if (!!snap)
			PromisedSnap->set_value(snap);
		else
			PromisedSnap->set_exception(std::make_exception_ptr(std::system_error(std::make_error_code((std::errc::errc)oErrorGetLast()), oErrorGetLastString())));
	});

	return Image;
}

void window_impl::start_timer(uintptr_t _Context, unsigned int _RelativeTimeMS)
{
	DISPATCH(::SetTimer(hWnd, (UINT_PTR)_Context, _RelativeTimeMS, nullptr));
}

void window_impl::stop_timer(uintptr_t _Context)
{
	DISPATCH(::KillTimer(hWnd, (UINT_PTR)_Context));
}

void window_impl::flush_messages(bool _WaitForNext)
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

void window_impl::quit()
{
	PostMessage(hWnd, oWM_QUIT, 0, 0);
	//dispatch_internal([=] { PostQuitMessage(0); });
};

void window_impl::trigger_generic_event(oGUI_EVENT _Event, oGUI_WINDOW_SHAPE_DESC* _pShape)
{
	oGUI_EVENT_SHAPE_DESC e((oGUI_WINDOW)hWnd, _Event, oWinGetShape(hWnd));
	EventHooks.Visit(e);
	if (_pShape)
		*_pShape = e.Shape;
}

void window_impl::set_cursor()
{
	oGUI_CURSOR_STATE NewState = ClientCursorState;
	HCURSOR hCursor = oWinGetCursor(NewState, (HCURSOR)hUserCursor);
	::SetCursor(hCursor);
	oWinCursorSetVisible(NewState != oGUI_CURSOR_NONE);
}

bool window_impl::handle_lifetime(HWND _hWnd, UINT _uMsg, WPARAM _wParam, LPARAM _lParam, LRESULT* _pLResult)
{
	switch (_uMsg)
	{
		case WM_CREATE:
		{
			CREATESTRUCTA* cs = (CREATESTRUCTA*)_lParam;
			oWIN_CREATESTRUCT* wcs = (oWIN_CREATESTRUCT*)cs->lpCreateParams;
			const init* pInit = (const init*)wcs->pInit;
			oASSERT(pInit, "invalid init struct");

			// this is a bit dangerous because it's not really true this hWnd is 
			// ready for use, but we need to expose it consistently as a valid 
			// return value from native_handle() so it can be accessed from 
			// oGUI_CREATING, where it's known to be only semi-ready.
			hWnd = _hWnd;

			oGUI_EVENT_CREATE_DESC e((oGUI_WINDOW)_hWnd
				, (oGUI_STATUSBAR)oWinGetStatusBar(_hWnd)
				, (oGUI_MENU)oWinGetMenu(_hWnd)
				, wcs->Shape, pInit->create_user_data);
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
			trigger_generic_event(oGUI_CLOSED);
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

bool window_impl::handle_misc(HWND _hWnd, UINT _uMsg, WPARAM _wParam, LPARAM _lParam, LRESULT* _pLResult)
{
	switch (_uMsg)
	{
		case WM_SETCURSOR:
			if (LOWORD(_lParam) == HTCLIENT)
			{
				set_cursor();
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
			trigger_generic_event((_wParam == WA_INACTIVE) ? oGUI_DEACTIVATED : oGUI_ACTIVATED);
			break;

		// All these should be treated the same if there's any reason to override 
		// painting.
		case WM_PAINT: case WM_PRINT: case WM_PRINTCLIENT:
			trigger_generic_event(oGUI_PAINT);
			break;

		case WM_DISPLAYCHANGE:
			trigger_generic_event(oGUI_DISPLAY_CHANGED);
			break;

		case oWM_DISPATCH:
		{
			oTASK* pTask = (oTASK*)_lParam;
			(*pTask)();
			delete_object(pTask);
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

bool window_impl::handle_sizing(HWND _hWnd, UINT _uMsg, WPARAM _wParam, LPARAM _lParam, LRESULT* _pLResult)
{
	*_pLResult = 0;

	switch (_uMsg)
	{
		case WM_ENTERSIZEMOVE:
			trigger_generic_event((_wParam == SC_MOVE) ? oGUI_MOVING : oGUI_SIZING);
			return true;

		case WM_MOVE:
		{
			//oTRACE("HWND 0x%x WM_MOVE: %dx%d", _hWnd, GET_X_LPARAM(_lParam), GET_Y_LPARAM(_lParam));

			oGUI_WINDOW_SHAPE_DESC s;
			trigger_generic_event(oGUI_MOVED, &s);
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

bool window_impl::handle_input(HWND _hWnd, UINT _uMsg, WPARAM _wParam, LPARAM _lParam, LRESULT* _pLResult)
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
				if (all(P >= int2(0,0)) && all(P <= oWinRectSize(rClient)))
					set_cursor();
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
				trigger_generic_event(oGUI_LOST_CAPTURE);
			}
			break;
		}

		case WM_CAPTURECHANGED:
		{
			if ((HWND)_lParam != _hWnd)
			{
				if (GetCapture() == hWnd)
					ReleaseCapture();
				trigger_generic_event(oGUI_LOST_CAPTURE);
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
							// @tony: Maybe touch doesn't need to pollute X11? The idea one 
							// day is to support wacky interface devices through RFB protocol 
							// which forces all boolean things down X11... should this comply?
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
			path_string* pPaths = new path_string[NumPaths];
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

LRESULT window_impl::WndProc(HWND _hWnd, UINT _uMsg, WPARAM _wParam, LPARAM _lParam)
{
	if ((Debug || kForceDebug))
	{
		xlstring s;
		oTRACE("%s", oWinParseWMMessage(s, s.capacity(), &ControlKeyState, _hWnd, _uMsg, _wParam, _lParam));
	}

	LRESULT lResult = -1;
	if (handle_input(_hWnd, _uMsg, _wParam, _lParam, &lResult))
		return lResult;

	if (handle_sizing(_hWnd, _uMsg, _wParam, _lParam, &lResult))
		return lResult;

	if (handle_misc(_hWnd, _uMsg, _wParam, _lParam, &lResult))
		return lResult;

	if (handle_lifetime(_hWnd, _uMsg, _wParam, _lParam, &lResult))
		return lResult;

	return DefWindowProc(_hWnd, _uMsg, _wParam, _lParam);
}

} // namespace ouro
