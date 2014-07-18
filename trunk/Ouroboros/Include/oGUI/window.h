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
// An abstraction for an operating system's window concept. This is fit for 
// containing child controls and doing other typical event-based Windowing. It 
// can also be made a primary render target for a GPU device. See oGPU for more
// details.
#pragma once
#ifndef oGUI_window_h
#define oGUI_window_h

#include <oGUI/oGUI.h>
#include <oBase/future.h>
#include <oCore/display.h>
#include <oSurface/buffer.h>

namespace ouro {

class basic_window
{
public:
	// environmental API
	virtual window_handle native_handle() const = 0;
	virtual display::id display_id() const = 0;
	virtual bool is_window_thread() const = 0;
  
	// Flushes the window's message queue. This should be called in a loop on the
	// same thread where the window was created. If _WaitForNext is false, this 
	// will loop until the messasge queue is empty and exit. If _WaitForNext is 
	// true this will block and process messages until Quit() is called.
	virtual void flush_messages(bool _WaitForNext = false) = 0;

	// Breaks out of a blocking flush_messages routine.
	virtual void quit() = 0;

	// If true, platform oTRACEs of every event and action will be enabled. This
	// is false by default.
	virtual void debug(bool _Debug) = 0;
	virtual bool debug() const = 0;


	// shape API
	virtual void state(window_state::value _State) = 0;
	virtual window_state::value state() const = 0;
	inline void show(window_state::value _State = window_state::restored) { state(_State); }
	inline void hide() { state(window_state::hidden); }
	inline void minimize() { state(window_state::minimized); }
	inline void maximize() { state(window_state::maximized); }
	inline void restore() { state(window_state::restored); }

	virtual void client_position(const int2& _ClientPosition) = 0;
	virtual int2 client_position() const = 0;
	virtual int2 client_size() const = 0;


	// border/decoration API
	virtual void icon(icon_handle _hIcon) = 0;
	virtual icon_handle icon() const = 0;

	// sets the cursor to use when cursor_state::user is specified in client_cursor_state
	virtual void user_cursor(cursor_handle _hCursor) = 0;
	virtual cursor_handle user_cursor() const = 0;

	virtual void client_cursor_state(cursor_state::value _State) = 0;
	virtual cursor_state::value client_cursor_state() const = 0;

	virtual void set_titlev(const char* _Format, va_list _Args) = 0;
	virtual char* get_title(char* _StrDestination, size_t _SizeofStrDestination) const = 0;

	inline void set_title(const char* _Format, ...) { va_list args; va_start(args, _Format); set_titlev(_Format, args); va_end(args); }
	template<size_t size> char* get_title(char (&_StrDestination)[size]) const { return get_title(_StrDestination, size); }
	template<size_t capacity> char* get_title(fixed_string<char, capacity>& _StrDestination) const { return get_title(_StrDestination, _StrDestination.capacity()); }

	// draw order/dependency API

	// Overrides Shape and STYLE to be RESTORED and NONE. If this exclusive 
	// fullscreen is true, then this throws an exception. A parent is 
	// automatically an owner of this window. If there is already an owner, this 
	// will throw an exception.
	virtual void parent(const std::shared_ptr<basic_window>& _Parent) = 0;
	virtual std::shared_ptr<basic_window> parent() const = 0;

	// An owner is more like a sibling who does all the work. Use this for the 
	// association between dialog boxes and the windows on top of which they are
	// applied. If there is a parent, this will throw an exception.
	virtual void owner(const std::shared_ptr<basic_window>& _Owner) = 0;
	virtual std::shared_ptr<basic_window> owner() const = 0;

	virtual void sort_order(window_sort_order::value _SortOrder) = 0;
	virtual window_sort_order::value sort_order() const = 0;

	virtual void focus(bool _Focus) = 0;
	virtual bool has_focus() const = 0;
};

class window : public basic_window
{
public:

	// events
	struct create_event;
	struct shape_event;
	struct timer_event;
	struct drop_event;
	struct input_device_event;
	struct custom_event;

	struct basic_event
	{
		basic_event(window_handle _hWindow, event_type::value _Type)
			: window(_hWindow)
			, type(_Type)
		{}

		// Native window handle
		window_handle window;

		// Event type. Use this to choose a downcaster below.
		event_type::value type;

		// union doesn't work because of int2's copy ctor so use downcasting instead
		inline const create_event& as_create() const;
		inline const shape_event& as_shape() const;
		inline const timer_event& as_timer() const;
		inline const drop_event& as_drop() const;
		inline const input_device_event& as_input_device() const;
		inline const custom_event& as_custom() const;
	};

	struct create_event : basic_event
	{
		create_event(window_handle _hWindow
			, statusbar_handle _hStatusBar
			, menu_handle _hMenu
			, const window_shape& _Shape
			, void* _pUser)
			: basic_event(_hWindow, event_type::creating)
			, statusbar(_hStatusBar)
			, menu(_hMenu)
			, shape(_Shape)
			, user(_pUser)
		{}

		// Native handle of the window's status bar.
		statusbar_handle statusbar;

		// Native handle of the top-level window's menu.
		menu_handle menu;
		window_shape shape;

		// The user can pass a value to this for usage during window creation.
		void* user;
	};

	struct shape_event : basic_event
	{
		shape_event(window_handle _hWindow
			, event_type::value _Type
			, const window_shape& _Shape)
			: basic_event(_hWindow, _Type)
			, shape(_Shape)
		{}

		window_shape shape;
	};

	struct timer_event : basic_event
	{
		timer_event(window_handle _hWindow, uintptr_t _Context)
			: basic_event(_hWindow, event_type::timer)
			, context(_Context)
		{}

		// Any pointer-sized value. It is recommended this be the address of a field
		// in the App's class that is the struct context for the timer event.
		uintptr_t context;
	};

	struct drop_event : basic_event
	{
		drop_event(window_handle _hWindow
			, const path_string* _pPaths
			, int _NumPaths
			, const int2& _ClientDropPosition)
			: basic_event(_hWindow, event_type::drop_files)
			, paths(_pPaths)
			, num_paths(_NumPaths)
			, client_drop_position(_ClientDropPosition)
		{}
		const path_string* paths;
		int num_paths;
		int2 client_drop_position;
	};

	struct input_device_event : basic_event
	{
		input_device_event(window_handle _hWindow
			, input::type _Type
			, input::status _Status
			, const char* _InstanceName)
			: basic_event(_hWindow, event_type::input_device_changed)
			, type(_Type)
			, status(_Status)
			, instance_name(_InstanceName)
		{}

		input::type type;
		input::status status;
		const char* instance_name;
	};

	struct custom_event : basic_event
	{
		custom_event(window_handle _hWindow, int _EventCode, uintptr_t _Context)
			: basic_event(_hWindow, event_type::custom_event)
			, code(_EventCode)
			, context(_Context)
		{}

		int code;
		uintptr_t context;
	};

	typedef std::function<void(const basic_event& _Event)> event_hook;

	
	// lifetime API
	struct init
	{
		init()
		: title("")
		, icon(nullptr)
		, create_user_data(nullptr)
		, cursor(nullptr)
		, cursor_state(cursor_state::arrow)
		, sort_order(window_sort_order::sorted)
		, debug(false)
		, allow_touch(false)
		, client_drag_to_move(false)
		, alt_f4_closes(false)
	{}
  
		const char* title;
		icon_handle icon;
		void* create_user_data; // user data accessible in the create event
		cursor_handle cursor;
		cursor_state::value cursor_state;
		window_sort_order::value sort_order;
		bool debug;
		bool allow_touch;
		bool client_drag_to_move;
		bool alt_f4_closes;

		// NOTE: The event_type::creating event gets fired during construction, so if that 
		// event is to be hooked it needs to be passed and hooked up during 
		// construction.
		event_hook on_event;
		input::action_hook on_action;
		window_shape shape;
	};

	static std::shared_ptr<window> make(const init& _Init);


	// shape API
	virtual void shape(const window_shape& _Shape) = 0;
	virtual window_shape shape() const = 0;
	void state(window_state::value _State) override { window_shape s; s.state = _State; shape(s); }
	window_state::value state() const override { window_shape s = shape(); return s.state; }
	void style(window_style::value _Style) { window_shape s; s.style = _Style; shape(s); }
	window_style::value style() const { window_shape s = shape(); return s.style; }

	void client_position(const int2& _ClientPosition) override { window_shape s; s.client_position = _ClientPosition; shape(s); }
	int2 client_position() const override { window_shape s = shape(); return s.client_position; }
	void client_size(const int2& _ClientSize) { window_shape s; s.client_size = _ClientSize; shape(s); }
	int2 client_size() const override { window_shape s = shape(); return s.client_size; }
 
  
	// border/decoration API

	// For widths, if the last width is -1, it will be interpreted as "take up 
	// rest of width of window".
	virtual void set_num_status_sections(const int* _pStatusSectionWidths, size_t _NumStatusSections) = 0;
	virtual int get_num_status_sections(int* _pStatusSectionWidths = nullptr, size_t _MaxNumStatusSectionWidths = 0) const = 0;
	template<size_t size> void set_num_status_sections(const int (&_pStatusSectionWidths)[size]) { set_num_status_sections(_pStatusSectionWidths, size); }
	template<size_t size> int get_num_status_sections(int (&_pStatusSectionWidths)[size]) { return get_num_status_sections(_pStatusSectionWidths, size); }

	virtual void set_status_textv(int _StatusSectionIndex, const char* _Format, va_list _Args) = 0;
	virtual char* get_status_text(char* _StrDestination, size_t _SizeofStrDestination, int _StatusSectionIndex) const = 0;

	inline void set_status_text(int _StatusSectionIndex, const char* _Format, ...) { va_list args; va_start(args, _Format); set_status_textv(_StatusSectionIndex, _Format, args); va_end(args); }
	template<size_t size> char* get_status_text(char (&_StrDestination)[size], int _StatusSectionIndex) const { return get_status_text(_StrDestination, size, _StatusSectionIndex); }
	template<size_t capacity> char* get_status_text(fixed_string<char, capacity>& _StrDestination, int _StatusSectionIndex) const { return get_status_text(_StrDestination, _StrDestination.capacity(), _StatusSectionIndex); }

	virtual void status_icon(int _StatusSectionIndex, icon_handle _hIcon) = 0;
	virtual icon_handle status_icon(int _StatusSectionIndex) const = 0;

 
	// extended input API

	// Returns true if this window has been associated with a rendering API that
	// may take control and reformat the window. This is manually flagged/specified
	// to keep GUI and rendering code orthogonal.
	virtual void render_target(bool _RenderTarget) = 0;
	virtual bool render_target() const = 0;

	// If true, platform oTRACEs of every event and action will be enabled. This
	// is false by default.
	virtual void debug(bool _Debug) = 0;
	virtual bool debug() const = 0;

	// Many touch screen drivers emulated mouse events in a way that is not 
	// consistent with regular mouse behavior, so if that affects application 
	// logic, here's a switch. This is false by default.
	virtual void allow_touch_actions(bool _Allow) = 0;
	virtual bool allow_touch_actions() const = 0;

	// If set to true, any mouse action in the client area will move the window.
	// This is useful for borderless windows like splash screens. This is false by 
	// default.
	virtual void client_drag_to_move(bool _DragMoves) = 0;
	virtual bool client_drag_to_move() const = 0;

	// If set to true, Alt-F4 will trigger an event_type::closing event. This is true by 
	// default.
	virtual void alt_f4_closes(bool _AltF4Closes) = 0;
	virtual bool alt_f4_closes() const = 0;

	virtual void enabled(bool _Enabled) = 0;
	virtual bool enabled() const = 0;

	virtual void capture(bool _Capture) = 0;
	virtual bool has_capture() const = 0;

	virtual void set_hotkeys(const basic_hotkey_info* _pHotKeys, size_t _NumHotKeys) = 0;
	template<size_t size> void set_hotkeys(const basic_hotkey_info (&_pHotKeys)[size]) { set_hotkeys(_pHotKeys, size); }
	template<size_t size> void set_hotkeys(const hotkey_info (&_pHotKeys)[size]) { set_hotkeys(_pHotKeys, size); }

	virtual int get_hotkeys(basic_hotkey_info* _pHotKeys, size_t _MaxNumHotKeys) const = 0;
	template<size_t size> int get_hotkeys(basic_hotkey_info (&_pHotKeys)[size]) { return get_hotkeys(_pHotKeys, size); }
	template<size_t size> int get_hotkeys(hotkey_info (&_pHotKeys)[size]) { return get_hotkeys(_pHotKeys, size); }


	// observer API

	virtual int hook_actions(const input::action_hook& _Hook) = 0;
	virtual void unhook_actions(int _ActionHookID) = 0;

	virtual int hook_events(const event_hook& _Hook) = 0;
	virtual void unhook_events(int _EventHookID) = 0;


	// execution API

	// Appends a broadcast of an action as if it came from user input.
	virtual void trigger(const input::action& _Action) = 0;

	// Post an event that is specified by the user here.
	virtual void post(int _CustomEventCode, uintptr_t _Context) = 0;

	// Posts the specified task in the window's message queue and executes it in 
	// order with other events. This is useful for wrapping platform-specific 
	// window/control calls.
	virtual void dispatch(const std::function<void()>& _Task) = 0;
	oDEFINE_CALLABLE_WRAPPERS(,dispatch,, dispatch);

	// Schedules an oImage to be generated from the window. In the simple case,
	// _Frame is not used and the front buffer is captured. Due to platform rules
	// this may involve bringing the specified window to focus.
	virtual future<std::shared_ptr<surface::buffer>> snapshot(int _Frame = -1, bool _IncludeBorder = false) const = 0;

	// Causes an event_type::timer event to occur with the specified context after the 
	// specified time. This will be called every specified milliseconds until 
	// stop_timer is called with the same context.
	virtual void start_timer(uintptr_t _Context, unsigned int _RelativeTimeMS) = 0;
	virtual void stop_timer(uintptr_t _Context) = 0;
};

const window::create_event& window::basic_event::as_create() const { oASSERT(type == event_type::creating, "wrong type"); return *static_cast<const create_event*>(this); }
const window::shape_event& window::basic_event::as_shape() const { oASSERT(is_shape_event(type), "wrong type"); return *static_cast<const shape_event*>(this); }
const window::timer_event& window::basic_event::as_timer() const { oASSERT(type == event_type::timer, "wrong type"); return *static_cast<const timer_event*>(this); }
const window::drop_event& window::basic_event::as_drop() const { oASSERT(type == event_type::drop_files, "wrong type"); return *static_cast<const drop_event*>(this); }
const window::input_device_event& window::basic_event::as_input_device() const { oASSERT(type == event_type::input_device_changed, "wrong type"); return *static_cast<const input_device_event*>(this); }
const window::custom_event& window::basic_event::as_custom() const { oASSERT(type == event_type::custom_event, "wrong type"); return *static_cast<const custom_event*>(this); }

} // namespace ouro

#endif
