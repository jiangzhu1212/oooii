// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.
#include <oGUI/window.h>
#include <oGUI/menu.h>
#include <oGUI/Windows/win_as_string.h>
#include <oGUI/Windows/oWinCursor.h>
#include <oGUI/Windows/oWinKey.h>
#include <oGUI/Windows/oWinRect.h>
#include <oGUI/Windows/oWinStatusBar.h>
#include <oGUI/Windows/oWinWindowing.h>
#include <oCore/windows/win_error.h>
#include <oSurface/codec.h>
#include <oConcurrency/backoff.h>
#include <oConcurrency/event.h>
#include <vector>
#include <commctrl.h>
#include <windowsx.h>
#include <Shellapi.h>

namespace ouro {

static bool kForceDebug = false;

#define DISPATCH(_SimpleFunction) do { dispatch_internal(std::move([=] { _SimpleFunction; })); } while(false)

// Converts a Windows control message to an action. This calls 
// oWinControlDefaultOnNotify if there is no other appropriate handling. This
// returns true if the output action and lresults are valid and should be 
// respected or false if this was not a control message.
bool control_to_action(HWND _hWnd, UINT _uMsg, WPARAM _wParam, LPARAM _lParam, input::action* _pAction, LRESULT* _pLResult)
{
	*_pLResult = 0;
	bool Handled = true;

	switch (_uMsg)
	{
		case WM_COMMAND:
		{
			_pAction->device_type = input::type::control;
			_pAction->device_id = LOWORD(_wParam);
			_pAction->window = (window_handle)_hWnd;
			_pAction->key = input::none;
			_pAction->position(0.0f);

			if (!_lParam)
			{
				_pAction->action_type = (HIWORD(_wParam) == 1) ? input::action_type::hotkey : input::action_type::menu;
				_pAction->action_code = invalid;
			}
			else
			{
				_pAction->window = (window_handle)_lParam;
				_pAction->action_type = input::action_type::control_activated;
				_pAction->action_code = HIWORD(_wParam);
			}

			break;
		}

		case WM_HSCROLL:
		{
			if (_lParam != 0)
			{
				_pAction->window = (window_handle)(_lParam);
				_pAction->device_type = input::type::control;
				_pAction->device_id = invalid;
				_pAction->key = input::key::none;
				_pAction->position(0.0f);
				_pAction->action_code = as_int(_wParam);
				switch (LOWORD(_wParam))
				{
					case TB_ENDTRACK: _pAction->action_type = input::action_type::control_deactivated; break;
					default: _pAction->action_type = input::action_type::control_activated; break;
				}
				break;
			}

			Handled = false;
			break;
		}

		case WM_NOTIFY:
		{
			LRESULT lResult = FALSE;
			const NMHDR& nmhdr = *(const NMHDR*)_lParam;
			*_pAction = input::action();
			_pAction->device_type = input::type::control;
			_pAction->device_id = as_int(nmhdr.idFrom);
			_pAction->window = nmhdr.hwndFrom;
			_pAction->position(0.0f);
			_pAction->action_code = nmhdr.code;

			control_type::value type = oWinControlGetType(nmhdr.hwndFrom);
			switch (type)
			{
				case control_type::tab:
				{
					switch (_pAction->action_code)
					{
						case TCN_SELCHANGING: _pAction->action_type = input::action_type::control_selection_changing; break;
						case TCN_SELCHANGE: _pAction->action_type = input::action_type::control_selection_changed; break;
						default: break;
					}

					break;
				}

				case control_type::button:
				{
					switch (_pAction->action_code)
					{
						case BN_CLICKED: _pAction->action_type = input::action_type::control_activated; break;
						default: break;
					}

					break;
				}

				default: break;
			}

			Handled = !oWinControlDefaultOnNotify(_hWnd, nmhdr, _pLResult, type);
			break;
		}

		default: Handled = false; break;
	}

	return Handled;
}

template<typename HookT, typename ParamT>
class HookManager
{
public:
	typedef HookT hook_type;
	typedef ParamT param_type;
	typedef std::recursive_mutex mutex_t;
	typedef std::lock_guard<mutex_t> lock_t;

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
		for (hook_type& hook : Hooks)
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
	window_handle native_handle() const override;
	display::id display_id() const override;
	bool is_window_thread() const override;
	void render_target(bool _RenderTarget) override;
	bool render_target() const override;
	void debug(bool _Debug = true) override;
	bool debug() const override;
	void flush_messages(bool _WaitForNext = false) override;
	void quit() override;


	// shape API
	void shape(const window_shape& _Shape) override;
	window_shape shape() const override;

	// border/decoration API
	void icon(icon_handle _hIcon) override;
	icon_handle icon() const override;
	void user_cursor(cursor_handle _hCursor) override;
	cursor_handle user_cursor() const override;
	void client_cursor_state(cursor_state::value _State) override;
	cursor_state::value client_cursor_state() const override;
	void set_titlev(const char* _Format, va_list _Args) override;
	char* get_title(char* _StrDestination, size_t _SizeofStrDestination) const override;
	void set_num_status_sections(const int* _pStatusSectionWidths, size_t _NumStatusSections) override;
	int get_num_status_sections(int* _pStatusSectionWidths = nullptr, size_t _MaxNumStatusSectionWidths = 0) const override;
	void set_status_textv(int _StatusSectionIndex, const char* _Format, va_list _Args) override;
	char* get_status_text(char* _StrDestination, size_t _SizeofStrDestination, int _StatusSectionIndex) const override;
	void status_icon(int _StatusSectionIndex, icon_handle _hIcon) override;
	icon_handle status_icon(int _StatusSectionIndex) const override;

	// Draw Order/Dependency API
	void parent(const std::shared_ptr<basic_window>& _Parent) override;
	std::shared_ptr<basic_window> parent() const override;
	void owner(const std::shared_ptr<basic_window>& _Owner) override;
	std::shared_ptr<basic_window> owner() const override;
	void sort_order(window_sort_order::value _SortOrder) override;
	window_sort_order::value sort_order() const override;
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
	void set_hotkeys(const ouro::basic_hotkey_info* _pHotKeys, size_t _NumHotKeys) override;
	int get_hotkeys(ouro::basic_hotkey_info* _pHotKeys, size_t _MaxNumHotKeys) const override;

	// Observer API
	int hook_actions(const input::action_hook& _Hook) override;
	void unhook_actions(int _ActionHookID) override;
	int hook_events(const event_hook& _Hook) override;
	void unhook_events(int _EventHookID) override;

	// Execution API
	void trigger(const input::action& _Action) override;
	void post(int _CustomEventCode, uintptr_t _Context) override;
	void dispatch(const std::function<void()>& _Task) override;
	future<surface::image> snapshot(int _Frame = ouro::invalid, bool _IncludeBorder = false) const override;
	void start_timer(uintptr_t _Context, unsigned int _RelativeTimeMS) override;
	void stop_timer(uintptr_t _Context) override;

private:

	HWND hWnd;
	HACCEL hAccel;
	HANDLE hHeap;
	cursor_handle hUserCursor;
	oWINKEY_CONTROL_STATE ControlKeyState;

	cursor_state::value ClientCursorState;
	window_sort_order::value SortOrder;
	bool Captured;
	bool ClientDragToMove;
	bool Debug;
	bool AllowTouch;
	int2 CursorClientPosAtMouseDown;

	window_shape PriorShape;

	typedef HookManager<input::action_hook, input::action> ActionManager_t;
	typedef HookManager<event_hook, basic_event> EventManager_t;

	ActionManager_t ActionHooks;
	EventManager_t EventHooks;

	event Destroyed;

	std::shared_ptr<basic_window> Owner;
	std::shared_ptr<basic_window> Parent;

private:

	void dispatch_internal(std::function<void()>&& _Task) const { PostMessage(hWnd, oWM_DISPATCH, 0, (LPARAM)const_cast<window_impl*>(this)->new_object<std::function<void()>>(std::move(_Task))); }

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
		const static size_t kStartLength = 1024;
		const static size_t kStopLength = 128*1024;
		char* s = nullptr;
		int len = -1;
		size_t cap = kStartLength;
		while (len < 0 && cap < kStopLength)
		{
			oScopedHeapLock lock(hHeap);
			s = (char*)HeapAlloc(hHeap, HEAP_GENERATE_EXCEPTIONS, cap);
			len = vsnprintf(s, cap, _Format, _Args);
			if (len >= 0 && size_t(len) < cap)
				break;
			HeapFree(hHeap, HEAP_GENERATE_EXCEPTIONS, s);
			cap *= 2;
		}

		oCHECK(len >= 0, "formatting failed for string \"%s\"", _Format);
		return s;
	}

	void delete_string(char* _String)
	{
		oScopedHeapLock lock(hHeap);
		HeapFree(hHeap, HEAP_GENERATE_EXCEPTIONS, _String);
	}

	void init_window(const init& _Init);
	void trigger_generic_event(event_type::value _Event, window_shape* _pShape = nullptr);
	void set_cursor();
	bool handle_input(HWND _hWnd, UINT _uMsg, WPARAM _wParam, LPARAM _lParam, LRESULT* _pLResult);
	bool handle_sizing(HWND _hWnd, UINT _uMsg, WPARAM _wParam, LPARAM _lParam, LRESULT* _pLResult);
	bool handle_misc(HWND _hWnd, UINT _uMsg, WPARAM _wParam, LPARAM _lParam, LRESULT* _pLResult);
	bool handle_lifetime(HWND _hWnd, UINT _uMsg, WPARAM _wParam, LPARAM _lParam, LRESULT* _pLResult);
};

void window_impl::init_window(const init& _Init)
{
	// this->hWnd assigned in WM_CREATE
	oWinCreate(nullptr, _Init.title, _Init.shape.style, _Init.shape.client_position, _Init.shape.client_size, StaticWndProc, (void*)&_Init, this);
	
	PriorShape = oWinGetShape(hWnd);

	// Initialize decoration

	if (_Init.icon)
		oWinSetIcon(hWnd, (HICON)_Init.icon);

	// Still have to set style here since oWinCreate is still unaware of menu/status bar
	window_shape InitShape;
	InitShape.state = _Init.shape.state;
	InitShape.style = _Init.shape.style;

	oWinSetShape(hWnd, InitShape);
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
	if (_Init.on_action)
		ActionHooks.Hook(_Init.on_action);

	if (_Init.on_event)
		EventHooks.Hook(_Init.on_event);

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

window_handle window_impl::native_handle() const
{
	return (window_handle)hWnd;
}

display::id window_impl::display_id() const
{
	window_shape s = shape();
	int2 center = s.client_position + s.client_size / 2;
	return display::find(center.x, center.y);
}

bool window_impl::is_window_thread() const
{
	return oWinIsWindowThread(hWnd);
}

void window_impl::shape(const window_shape& _Shape)
{
	dispatch_internal(std::move([=]
	{
		try { oWinSetShape(hWnd, _Shape); }
		catch (std::exception& e)
		{ oTRACEA("ERROR: oWinSetShape: %s", e.what()); }
	}));
}

window_shape window_impl::shape() const
{
	return oWinGetShape(hWnd);
}

void window_impl::icon(icon_handle _hIcon)
{
	DISPATCH(oWinSetIcon(hWnd, (HICON)_hIcon, true));
	DISPATCH(oWinSetIcon(hWnd, (HICON)_hIcon, false));
}

icon_handle window_impl::icon() const
{
	return (icon_handle)oWinGetIcon(hWnd);
}

void window_impl::user_cursor(cursor_handle _hCursor)
{
	dispatch_internal(std::move([=]
	{
		if (hUserCursor)
			DestroyCursor((HCURSOR)hUserCursor);
		hUserCursor = _hCursor;
	}));
}

cursor_handle window_impl::user_cursor() const
{
	return (cursor_handle)hUserCursor;
}

void window_impl::client_cursor_state(cursor_state::value _State)
{
	DISPATCH(ClientCursorState = _State);
}

cursor_state::value window_impl::client_cursor_state() const
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
		oWinStatusBarSetText(oWinGetStatusBar(hWnd), _StatusSectionIndex, border_style::flat, pString);
		if (pString)
			delete_string(pString);
	}));
}

char* window_impl::get_status_text(char* _StrDestination, size_t _SizeofStrDestination, int _StatusSectionIndex) const
{
	size_t len = 0;
	window_impl* w = const_cast<window_impl*>(this);
	std::function<void()>* pTask = w->new_object<std::function<void()>>(std::move([=,&len]
	{
		if (oWinStatusBarGetText(_StrDestination, _SizeofStrDestination, oWinGetStatusBar(hWnd), _StatusSectionIndex))
			len = strlen(_StrDestination);
	}));

	SendMessage(hWnd, oWM_DISPATCH, 0, (LPARAM)pTask);
	return (len && len <= _SizeofStrDestination) ? _StrDestination : nullptr;
}

void window_impl::status_icon(int _StatusSectionIndex, icon_handle _hIcon)
{
	DISPATCH(oWinStatusBarSetIcon(oWinGetStatusBar(hWnd), _StatusSectionIndex, (HICON)_hIcon));
}

icon_handle window_impl::status_icon(int _StatusSectionIndex) const
{
	return (icon_handle)oWinStatusBarGetIcon(oWinGetStatusBar(hWnd), _StatusSectionIndex);
}

void window_impl::parent(const std::shared_ptr<basic_window>& _Parent)
{
	dispatch_internal(std::move([=]
	{
		oCHECK(!Owner, "Can't have owner at same time as parent");
		Parent = _Parent;
		oWinSetParent(hWnd, Parent ? (HWND)Parent->native_handle() : nullptr);
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

void window_impl::sort_order(window_sort_order::value _SortOrder)
{
	dispatch_internal(std::move([=]
	{
		this->SortOrder = _SortOrder;
		oWinSetAlwaysOnTop(hWnd, _SortOrder != window_sort_order::sorted);
		if (_SortOrder == window_sort_order::always_on_top_with_focus)
			::SetTimer(hWnd, (UINT_PTR)&SortOrder, 500, nullptr);
		else
			::KillTimer(hWnd, (UINT_PTR)&SortOrder);
	}));
}

window_sort_order::value window_impl::sort_order() const
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

void window_impl::render_target(bool _RenderTarget)
{
	oWinSetIsRenderTarget(hWnd, _RenderTarget);
}

bool window_impl::render_target() const
{
	return oWinIsRenderTarget(hWnd);
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
		oWinRegisterTouchEvents(hWnd, _Allow);
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
	DISPATCH(oWinAltF4Enable(hWnd, _AltF4Closes));
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

void window_impl::set_hotkeys(const ouro::basic_hotkey_info* _pHotKeys, size_t _NumHotKeys)
{
	ouro::basic_hotkey_info* pCopy = nullptr;
	if (_pHotKeys && _NumHotKeys)
	{
		pCopy = new_array<ouro::basic_hotkey_info>(_NumHotKeys);
		memcpy(pCopy, _pHotKeys, sizeof(ouro::basic_hotkey_info) * _NumHotKeys);
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
			this->hAccel = CreateAcceleratorTable((LPACCEL)pAccels, as_uint(_NumHotKeys));
			this->delete_array(pAccels);
			this->delete_array(pCopy);
		}
	}));
}

int window_impl::get_hotkeys(ouro::basic_hotkey_info* _pHotKeys, size_t _MaxNumHotKeys) const
{
	int N = 0;
	window_impl* w = const_cast<window_impl*>(this);
	std::function<void()>* pTask = w->new_object<std::function<void()>>(std::move([=,&N]
	{
		if (hAccel && _pHotKeys && _MaxNumHotKeys)
		{
			int nHotKeys = CopyAcceleratorTable(hAccel, nullptr, 0);
			ACCEL* pAccels = w->new_array<ACCEL>(nHotKeys);
			CopyAcceleratorTable(hAccel, pAccels, nHotKeys);
			int NumCopied = __min(nHotKeys, as_int(_MaxNumHotKeys));
			oWinAccelToHotKeys(_pHotKeys, pAccels, NumCopied);
			w->delete_array(pAccels);
			N = NumCopied;
		}
	}));

	SendMessage(hWnd, oWM_DISPATCH, 0, (LPARAM)pTask);
	return N;
}

int window_impl::hook_actions(const input::action_hook& _Hook)
{
	return ActionHooks.Hook(_Hook);
}

void window_impl::unhook_actions(int _ActionHookID)
{
	ActionHooks.Unhook(_ActionHookID);
}

int window_impl::hook_events(const event_hook& _Hook)
{
	return EventHooks.Hook(_Hook);
}

void window_impl::unhook_events(int _EventHookID)
{
	EventHooks.Unhook(_EventHookID);
}

void window_impl::trigger(const input::action& _Action)
{
	dispatch_internal(std::move(std::bind(&ActionManager_t::Visit, &ActionHooks, _Action))); // bind by copy
}

void window_impl::post(int _CustomEventCode, uintptr_t _Context)
{
	custom_event e((window_handle)hWnd, _CustomEventCode, _Context);
	dispatch_internal(std::bind(&EventManager_t::Visit, &EventHooks, e)); // bind by copy
}

void window_impl::dispatch(const std::function<void()>& _Task)
{
	std::function<void()>* pTask = new_object<std::function<void()>>(_Task);
	PostMessage(hWnd, oWM_DISPATCH, 0, (LPARAM)pTask);
}

static bool oWinWaitUntilOpaque(HWND _hWnd, unsigned int _TimeoutMS)
{
	backoff bo;
	unsigned int Now = timer::nowmsi();
	unsigned int Then = Now + _TimeoutMS;
	while (!oWinIsOpaque(_hWnd))
	{
		bo.pause();
		Now = timer::nowmsi();
		if (_TimeoutMS != ouro::infinite && Now > Then)
			return false;
	}

	return true;
}

future<surface::image> window_impl::snapshot(int _Frame, bool _IncludeBorder) const
{
	auto PromisedSnap = std::make_shared<ouro::promise<surface::image>>();
	auto Image = PromisedSnap->get_future();

	const_cast<window_impl*>(this)->dispatch([=]() mutable
	{
		bool success = oWinWaitUntilOpaque(hWnd, 20000);
		if (!success)
		{
			window_shape s = oWinGetShape(hWnd);
			if (is_visible(s.state))
				oCHECK0(false); // pass through verification of wait
			else
				oCHECK(false, "A non-hidden window timed out waiting to become opaque");
		}

		surface::image snap;
		void* buf = nullptr;
		size_t size = 0;
		oWinSetFocus(hWnd); // Windows doesn't do well with hidden contents.
		try
		{
			oGDIScreenCaptureWindow(hWnd, _IncludeBorder, malloc, &buf, &size, false, false);
			snap = surface::decode(buf, size);
			free(buf);
		}

		catch (std::exception)
		{
			PromisedSnap->set_exception(std::current_exception());
			snap.deinitialize();
		}

		if (!!snap)
			PromisedSnap->set_value(std::move(snap));
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
		int r = oWinDispatchMessage(hWnd, hAccel, _WaitForNext);
		if (r == -1)
			return;
		else if (r == 0 && !_WaitForNext)
			return;
	}
}

void window_impl::quit()
{
	PostMessage(hWnd, oWM_QUIT, 0, 0);
	//dispatch_internal([=] { PostQuitMessage(0); });
};

void window_impl::trigger_generic_event(event_type::value _Event, window_shape* _pShape)
{
	shape_event e((window_handle)hWnd, _Event, oWinGetShape(hWnd));
	EventHooks.Visit(e);
	if (_pShape)
		*_pShape = e.shape;
}

void window_impl::set_cursor()
{
	cursor_state::value NewState = ClientCursorState;
	HCURSOR hCursor = oWinGetCursor(NewState, (HCURSOR)hUserCursor);
	::SetCursor(hCursor);
	oWinCursorSetVisible(NewState != cursor_state::none);
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
			// event_type::creating, where it's known to be only semi-ready.
			hWnd = _hWnd;

			create_event e((window_handle)_hWnd
				, (statusbar_handle)oWinGetStatusBar(_hWnd)
				, (menu_handle)oWinGetMenu(_hWnd)
				, wcs->Shape, pInit->create_user_data);
			EventHooks.Visit(e);
			break;
		}
	
		case WM_CLOSE:
		{
			// Don't allow DefWindowProc to destroy the window, put it all on client 
			// code.
			shape_event e((window_handle)hWnd, event_type::closing, oWinGetShape(hWnd));
			EventHooks.Visit(e);
			*_pLResult = 0;
			return true;
		}

		case oWM_DESTROY:
			DestroyWindow(_hWnd);
			break;

		case WM_DESTROY:
		{
			trigger_generic_event(event_type::closed);
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
			trigger_generic_event((_wParam == WA_INACTIVE) ? event_type::deactivated : event_type::activated);
			break;

		// All these should be treated the same if there's any reason to override 
		// painting.
		case WM_PAINT: case WM_PRINT: case WM_PRINTCLIENT:
			trigger_generic_event(event_type::paint);
			break;

		case WM_DISPLAYCHANGE:
			trigger_generic_event(event_type::display_changed);
			break;

		case oWM_DISPATCH:
		{
			std::function<void()>* pTask = (std::function<void()>*)_lParam;
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
				timer_event e((window_handle)_hWnd, (uintptr_t)_wParam);
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
			trigger_generic_event((_wParam == SC_MOVE) ? event_type::moving : event_type::sizing);
			return true;

		case WM_MOVE:
		{
			//oTRACE("HWND 0x%x WM_MOVE: %dx%d", _hWnd, GET_X_LPARAM(_lParam), GET_Y_LPARAM(_lParam));

			window_shape s;
			trigger_generic_event(event_type::moved, &s);
			PriorShape.client_position = s.client_position;
			return true;
		}

		case WM_SIZE:
		{
			//oTRACE("WM_SIZE: %dx%d", GET_X_LPARAM(_lParam), GET_Y_LPARAM(_lParam));

			if (!oWinIsTempChange(_hWnd))
			{
				shape_event e((window_handle)hWnd, event_type::sizing, PriorShape);
				EventHooks.Visit(e);

				e.type = event_type::sized;
				e.shape = oWinGetShape(_hWnd);
				EventHooks.Visit(e);
				PriorShape = e.shape;
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

	input::action Action;
	if (oWinKeyDispatchMessage(_hWnd, _uMsg, _wParam, _lParam, Timestamp, &ControlKeyState, &Action))
	{
		if (ClientDragToMove)
		{
			const input::key CheckKey = GetSystemMetrics(SM_SWAPBUTTON) ? input::mouse_right : input::mouse_left;

			if (Action.key == CheckKey)
			{
				if (Action.action_type == input::action_type::key_down)
				{
					CursorClientPosAtMouseDown = oWinCursorGetPosition(_hWnd);
					::SetCapture(_hWnd);
				}

				else if (Action.action_type == input::action_type::key_up)
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

	if (control_to_action(_hWnd, _uMsg, _wParam, _lParam, &Action, _pLResult))
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
				window_shape Shape;
				Shape.client_position = int2(p.x, p.y) - CursorClientPosAtMouseDown;
				try { oWinSetShape(_hWnd, Shape); }
				catch (std::exception& e) { oTRACEA("ERROR: oWinSetShape: %s", e.what()); }
			}

			break;
		}

		case WM_CANCELMODE:
		{
			if (GetCapture() == hWnd)
			{
				ReleaseCapture();
				trigger_generic_event(event_type::lost_capture);
			}
			break;
		}

		case WM_CAPTURECHANGED:
		{
			if ((HWND)_lParam != _hWnd)
			{
				if (GetCapture() == hWnd)
					ReleaseCapture();
				trigger_generic_event(event_type::lost_capture);
			}
			break;
		}
	
		// WM_INPUT_DEVICE_CHANGE is parsed into a more reasonable messaging system
		// so use oWM_INPUT_DEVICE_CHANGE as its proxy.
		case oWM_INPUT_DEVICE_CHANGE:
		{
			input_device_event e((window_handle)_hWnd
				, input::type(LOWORD(_wParam))
				, input::status(HIWORD(_wParam))
				, (const char*)_lParam);
			EventHooks.Visit(e);
			return true;
		}

		case oWM_SKELETON:
		{
			input::action a(_hWnd, Timestamp, LOWORD(_wParam), input::type::skeleton, input::action_type::skeleton_update, input::none, invalid);
			a.skeleton = (void*)_lParam;
			ActionHooks.Visit(a);
			return 0;
		}

		case oWM_USER_CAPTURED:
			ActionHooks.Visit(input::action(_hWnd, Timestamp, static_cast<unsigned int>(_wParam), input::type::skeleton, input::action_type::skeleton_acquired));
			return 0;

		case oWM_USER_LOST:
			ActionHooks.Visit(input::action(_hWnd, Timestamp, static_cast<unsigned int>(_wParam), input::type::skeleton, input::action_type::skeleton_lost));
			return 0;

		#ifdef oWINDOWS_HAS_REGISTERTOUCHWINDOW
			case WM_TOUCH:
			{
				TOUCHINPUT inputs[(int)input::key::touch_last - (int)input::key::touch_first];
				const UINT nTouches = __min(LOWORD(_wParam), oCOUNTOF(inputs));
				if (nTouches)
				{
					if (GetTouchInputInfo((HTOUCHINPUT)_lParam, nTouches, inputs, sizeof(TOUCHINPUT)))
					{
						input::action a(_hWnd, Timestamp, 0, input::type::touch, input::action_type::key_down);
						for (UINT i = 0; i < nTouches; i++)
						{
							a.device_id = i;
							a.key = (input::key)((int)input::key::touch1 + i);
							a.position(float4(inputs[i].x / 100.0f, inputs[i].y / 100.0f, 0.0f, 0.0f));
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
			const int NumPaths = DragQueryFile((HDROP)_wParam, invalid, nullptr, 0); 
			path_string* pPaths = new path_string[NumPaths];
			for (int i = 0; i < NumPaths; i++)
				DragQueryFile((HDROP)_wParam, i, const_cast<char*>(pPaths[i].c_str()), as_uint(pPaths[i].capacity()));
			DragFinish((HDROP)_wParam);

			drop_event e((window_handle)_hWnd, pPaths, NumPaths, p);
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
		oTRACE("%s", windows::parse_wm_message(s, s.capacity(), &ControlKeyState, _hWnd, _uMsg, _wParam, _lParam));
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
