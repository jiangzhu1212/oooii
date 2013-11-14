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

#include <oBasis/oGUI.h>
#include <oConcurrency/oConcurrency.h>
#include <oStd/future.h>
#include <oCore/display.h>
#include <oSurface/buffer.h>

namespace ouro {

class basic_window
{
public:
	// environmental API
	virtual oGUI_WINDOW native_handle() const = 0;
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
	virtual void state(oGUI_WINDOW_STATE _State) = 0;
	virtual oGUI_WINDOW_STATE state() const = 0;
	inline void show(oGUI_WINDOW_STATE _State = oGUI_WINDOW_RESTORED) { state(_State); }
	inline void hide() { state(oGUI_WINDOW_HIDDEN); }
	inline void minimize() { state(oGUI_WINDOW_MINIMIZED); }
	inline void maximize() { state(oGUI_WINDOW_MAXIMIZED); }
	inline void restore() { state(oGUI_WINDOW_RESTORED); }

	virtual void client_position(const int2& _ClientPosition) = 0;
	virtual int2 client_position() const = 0;
	virtual int2 client_size() const = 0;


	// border/decoration API
	virtual void icon(oGUI_ICON _hIcon) = 0;
	virtual oGUI_ICON icon() const = 0;

	// sets the cursor to use when oGUI_CURSOR_USER is specified in client_cursor_state
	virtual void user_cursor(oGUI_CURSOR _hCursor) = 0;
	virtual oGUI_CURSOR user_cursor() const = 0;

	virtual void client_cursor_state(oGUI_CURSOR_STATE _State) = 0;
	virtual oGUI_CURSOR_STATE client_cursor_state() const = 0;

	virtual void set_titlev(const char* _Format, va_list _Args) = 0;
	virtual char* get_title(char* _StrDestination, size_t _SizeofStrDestination) const = 0;

	inline void set_title(const char* _Format, ...) { va_list args; va_start(args, _Format); set_titlev(_Format, args); va_end(args); }
	template<size_t size> char* get_title(char (&_StrDestination)[size]) const { return get_title(_StrDestination, size); }
	template<size_t capacity> char* get_title(ouro::fixed_string<char, capacity>& _StrDestination) const { return get_title(_StrDestination, _StrDestination.capacity()); }

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

	virtual void sort_order(oGUI_WINDOW_SORT_ORDER _SortOrder) = 0;
	virtual oGUI_WINDOW_SORT_ORDER sort_order() const = 0;

	virtual void focus(bool _Focus) = 0;
	virtual bool has_focus() const = 0;
};

class window : public basic_window
{
public:

	struct init
	{
		init()
		: title("")
		, icon(nullptr)
		, cursor(nullptr)
		, cursor_state(oGUI_CURSOR_ARROW)
		, sort_order(oGUI_WINDOW_SORTED)
		, debug(false)
		, allow_touch(false)
		, client_drag_to_move(false)
		, alt_f4_closes(false)
	{}
  
		const char* title;
		oGUI_ICON icon;
		oGUI_CURSOR cursor;
		oGUI_CURSOR_STATE cursor_state;
		oGUI_WINDOW_SORT_ORDER sort_order;
		bool debug;
		bool allow_touch;
		bool client_drag_to_move;
		bool alt_f4_closes;

		// NOTE: The oGUI_CREATING event gets fired during construction, so if that 
		// event is to be hooked it needs to be passed and hooked up during 
		// construction.
		oGUI_EVENT_HOOK event_hook;
		oGUI_ACTION_HOOK action_hook;
		oGUI_WINDOW_SHAPE_DESC shape;
	};
  
	static std::shared_ptr<window> make(const init& _Init);


	// shape API
	virtual void shape(const oGUI_WINDOW_SHAPE_DESC& _Shape) = 0;
	virtual oGUI_WINDOW_SHAPE_DESC shape() const = 0;
	void state(oGUI_WINDOW_STATE _State) override { oGUI_WINDOW_SHAPE_DESC s; s.State = _State; shape(s); }
	oGUI_WINDOW_STATE state() const override { oGUI_WINDOW_SHAPE_DESC s = shape(); return s.State; }
	void style(oGUI_WINDOW_STYLE _Style) { oGUI_WINDOW_SHAPE_DESC s; s.Style = _Style; shape(s); }
	oGUI_WINDOW_STYLE style() const { oGUI_WINDOW_SHAPE_DESC s = shape(); return s.Style; }

	void client_position(const int2& _ClientPosition) override { oGUI_WINDOW_SHAPE_DESC s; s.ClientPosition = _ClientPosition; shape(s); }
	int2 client_position() const override { oGUI_WINDOW_SHAPE_DESC s = shape(); return s.ClientPosition; }
	void client_size(const int2& _ClientSize) { oGUI_WINDOW_SHAPE_DESC s; s.ClientSize = _ClientSize; shape(s); }
	int2 client_size() const override { oGUI_WINDOW_SHAPE_DESC s = shape(); return s.ClientSize; }
 
  
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
	template<size_t capacity> char* get_status_text(ouro::fixed_string<char, capacity>& _StrDestination, int _StatusSectionIndex) const { return get_status_text(_StrDestination, _StrDestination.capacity(), _StatusSectionIndex); }

	virtual void status_icon(int _StatusSectionIndex, oGUI_ICON _hIcon) = 0;
	virtual oGUI_ICON status_icon(int _StatusSectionIndex) const = 0;

 
	// extended input API

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

	// If set to true, Alt-F4 will trigger an oGUI_CLOSING event. This is true by 
	// default.
	virtual void alt_f4_closes(bool _AltF4Closes) = 0;
	virtual bool alt_f4_closes() const = 0;

	virtual void enabled(bool _Enabled) = 0;
	virtual bool enabled() const = 0;

	virtual void capture(bool _Capture) = 0;
	virtual bool has_capture() const = 0;

	virtual void set_hotkeys(const oGUI_HOTKEY_DESC_NO_CTOR* _pHotKeys, size_t _NumHotKeys) = 0;
	template<size_t size> void set_hotkeys(const oGUI_HOTKEY_DESC_NO_CTOR (&_pHotKeys)[size]) { set_hotkeys(_pHotKeys, size); }
	template<size_t size> void set_hotkeys(const oGUI_HOTKEY_DESC (&_pHotKeys)[size]) { set_hotkeys(_pHotKeys, size); }

	virtual int get_hotkeys(oGUI_HOTKEY_DESC_NO_CTOR* _pHotKeys, size_t _MaxNumHotKeys) const = 0;
	template<size_t size> int get_hotkeys(oGUI_HOTKEY_DESC_NO_CTOR (&_pHotKeys)[size]) { return get_hotkeys(_pHotKeys, size); }
	template<size_t size> int get_hotkeys(oGUI_HOTKEY_DESC (&_pHotKeys)[size]) { return get_hotkeys(_pHotKeys, size); }


	// observer API

	virtual int hook_actions(const oGUI_ACTION_HOOK& _Hook) = 0;
	virtual void unhook_actions(int _ActionHookID) = 0;

	virtual int hook_events(const oGUI_EVENT_HOOK& _Hook) = 0;
	virtual void unhook_events(int _EventHookID) = 0;


	// execution API

	// Appends a broadcast of an action as if it came from user input.
	virtual void trigger(const oGUI_ACTION_DESC& _Action) = 0;

	// Post an event that is specified by the user here.
	virtual void post(int _CustomEventCode, uintptr_t _Context) = 0;

	// Posts the specified task in the window's message queue and executes it in 
	// order with other events. This is useful for wrapping platform-specific 
	// window/control calls.
	virtual void dispatch(const oTASK& _Task) = 0;
	oDEFINE_CALLABLE_WRAPPERS(dispatch,, dispatch);

	// Schedules an oImage to be generated from the window. In the simple case,
	// _Frame is not used and the front buffer is captured. Due to platform rules
	// this may involve bringing the specified window to focus.
	virtual oStd::future<std::shared_ptr<ouro::surface::buffer>> snapshot(int _Frame = oInvalid, bool _IncludeBorder = false) const = 0;

	// Causes an oGUI_TIMER event to occur with the specified context after the 
	// specified time. This will be called every specified milliseconds until 
	// stop_timer is called with the same context.
	virtual void start_timer(uintptr_t _Context, unsigned int _RelativeTimeMS) = 0;
	virtual void stop_timer(uintptr_t _Context) = 0;
};

} // namespace ouro

#endif
