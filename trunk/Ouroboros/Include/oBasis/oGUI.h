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
// Common header for Graphical User Interface (GUI) concepts. This should be 
// used as the basis of a cross-platform interface and conversion functions from 
// these generic concepts to platform-specific concepts.
#pragma once
#ifndef oGUI_h
#define oGUI_h

#include <oBase/assert.h>
#include <oBase/color.h>
#include <oBase/fixed_string.h>
#include <oCompute/oAABox.h>
#include <array>
#include <functional>

#define oGUI_IN_RANGE(_EnumPrefix, _Value) ouro::in_range(ouro::input_key::_EnumPrefix##_first, ouro::input_key::_EnumPrefix##_last, _Value)

namespace ouro {

// Commonly used when creating enums of GUI functionality to have associated
// runs in the enum - such as for a radio selection list.
inline bool in_range(int _First, int _Last, int _Value) { return _Value >= _First && _Value <= _Last; }

namespace input_device_type
{	enum value {

	unknown,
	keyboard,
	mouse,
	joystick,
	control, // i.e. a button or scrollbar
	skeleton,
	voice,
	touch,

	count,

};}

namespace input_device_status
{	enum value {

	ready,
	initializing,
	not_connected,
	is_clone,
	not_supported,
	insufficient_bandwidth,
	low_power,
	not_powered,
	not_ready,

	count,

};}

namespace input_key
{	enum value {

	none,

	// Mouse keys
	mouse_left,
	mouse_first = mouse_left,

	standard_first = mouse_first, 
	mouse_right,
	mouse_middle,
	mouse_side1,
	mouse_side2,
	// Double-click seems to be ubiquitous. In X11 and thus RFB it's its own
	// button and in Windows the button events get eaten and transformed into 
	// different events for double-click. So here favor the X11 model.
	mouse_left_double,
	mouse_right_double,
	mouse_middle_double,
	mouse_side1_double,
	mouse_side2_double,
	mouse_last = mouse_side2_double,

	// Joystick keys
	joy_lleft,
	joystick_first = joy_lleft, joy_lup, joy_lright, joy_ldown,
	joy_rleft, joy_rup, joy_rright, joy_rdown,
	joy_lshoulder1, joy_lshoulder2, joy_rshoulder1, joy_rshoulder2,
	joy_lthumb, joy_rthumb, joy_system, joy_start, joy_select,
	joystick_last = joy_select,

	// Control keys
	lctrl,
	keyboard_first = lctrl, rctrl,
	lalt, ralt,
	lshift, rshift,
	lwin, rwin,
	app_cycle, app_context,

	// Toggle keys
	capslock,
	scrolllock,
	numlock,

	// Typing keys
	space, backtick, dash, equal_, lbracket, rbracket, backslash, semicolon, apostrophe, comma, period, slash,
	_0, _1, _2, _3, _4, _5, _6, _7, _8, _9,
	a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u, v, w, x, y, z,

	// Numpad keys
	num0, num1, num2, num3, num4, num5, num6, num7, num8, num9, 
	nummul, numadd, numsub, numdecimal, numdiv, 

	// Typing control keys
	esc,
	backspace,
	tab,
	enter,
	ins, del,
	home, end,
	pgup, pgdn,

	// System control keys
	f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12, f13, f14, f15, f16, f17, f18, f19, f20, f21, f22, f23, f24,
	pause,
	sleep,
	printscreen,

	// Directional keys
	left, up, right, down,

	standard_last = down,

	// Browser keys
	mail, 
	back, 
	forward, 
	refresh, 
	stop, 
	search, 
	favs,

	// Media keys
	media,
	mute,
	volup,
	voldn,
	prev_track,
	next_track,
	stop_track,
	play_pause_track,

	// Misc keys
	app1, app2,

	keyboard_last = app2,

	// Touch
	touch1,
	touch_first = touch1, touch2, touch3, touch4, touch5, touch6, touch7, touch8, touch9, touch10,
	touch_last = touch10,

	count,

};}

namespace skeleton_bone
{	enum value {
	
	hip_center,
	spine,
	shoulder_center,
	head,
	shoulder_left,
	elbow_left,
	wrist_left,
	hand_left,
	shoulder_right,
	elbow_right,
	wrist_right,
	hand_right,
	hip_left,
	knee_left,
	ankle_left,
	foot_left,
	hip_right,
	knee_right,
	ankle_right,
	foot_right,
	
	count,
	invalid = count,

};}

namespace alignment
{ enum value {

	// Position is relative to parent's top-left corner
	top_left,

	// Position is centered horizontally and vertically relative to the parent's 
	// top
	top_center,

	// Position is relative to parent's top-right corner minus the child's width
	top_right,

	// Position is centered vertically and horizontally relative to parent's left 
	// corner
	middle_left,
	
	// Position is relative to that which centers the child in the parent's bounds
	middle_center,
	
	// Position is centered vertically and horizontally relative to parent's right 
	// corner minus the child's width
	middle_right,
	
	// Position is relative to parent's bottom-left corner
	bottom_left,

	// Position is centered horizontally and vertically relative to the parent's 
	// bottom minus the child's height
	bottom_center,
	
	// Position is relative to parent's bottom-right corner minus the child's width
	bottom_right,
	
	// Child is sized and positioned to match parent: aspect ration is not 
	// respected. Alignment will be center-middle and respect any offset value.
	fit_parent,

	// Retain aspect ratio and choose the child's largest dimension (width or 
	// height) and leave letterbox (vertical or horizontal) areas. Alignment will 
	// be center-middle and respect any offset value.
	fit_largest_axis,

	// Retain aspect ration and choose the child's smallest dimension (width or 
	// height) and crop any overflow. (cropping of the result rectangle is 
	// dependent on a separate cropping flag.
	fit_smallest_axis,

	count,

};}

namespace window_state
{	enum value {

	// A GUI window is a rectangular client area with an operating system-specific
	// border. It can exist in one of the several states listed below.
	
	// Window does not exist, or when specifying a new state, use this to indicate 
	// no-change.
	invalid,
	
	// Window is invisible.
	hidden,

	// Window is reduces to iconic or taskbar size. When setting this size
	// and position are ignored but style is preserved.
	minimized,
	
	// Window is in normal sub-screen-size mode. When setting this state size,  
	// position and style are respected.
	restored,

	// Window takes up the entire screen but still has borders. When setting this 
	// state size and position are ignored but style is preserved.
	maximized, 

	// Window borders are removed and the client area fills the whole screen but 
	// does not do special direct-access or HW-syncing - it still behaves like a 
	// window thus enabling fast application switching or multi-full-screen
	// support.
	fullscreen,
	
	count,

};}

namespace window_style
{	enum value {

	// Use this value for "noop" or "previous" value.
	default_style,

	// There is no border around the client area and the window remains a top-
	// level window. This is the best for splash screens and fullscreen modes.
	borderless,

	// There is a border but closing the window from the decoration is not allowed
	dialog,

	// There is a border but no user resize is allowed
	fixed,

	// Same as fixed with a menu
	fixed_with_menu,

	// Same as fixed with a status bar
	fixed_with_statusbar,

	// Same as fixed with a menu and status bar
	fixed_with_menu_and_statusbar,

	// There is a border and user can resize window
	sizable,

	// Same as sizable with a menu
	sizable_with_menu,

	// Same as sizable with a status bar
	sizable_with_statusbar,

	// Same as sizable with a menu and a status bar
	sizable_with_menu_and_statusbar,

	count,

};}

namespace window_sort_order
{	enum value {

	// normal/default behavior
	sorted,

	// "normal" always-on-top
	always_on_top,

	// Meant for final shipping, this fights hard to ensure any Windows popups or
	// even the taskbar stays hidden.
	always_on_top_with_focus,

	count,

};}

namespace cursor_state
{	enum value {

	// No cursor appears (hidden)
	none,

	// Default OS arrow
	arrow,
	
	// A hand for translating/scrolling
	hand,

	// A question-mark-like icon
	help,

	// Use this to indicate mouse interaction is not allowed
	not_allowed,

	// User input is blocked while the operation is occurring
	wait_foreground,

	// A performance-degrading action is under way but user input is not blocked
	wait_background,
	
	// A user-provided cursor is displayed
	user,

	count,
};}

namespace control_type
{	enum value {

	unknown,
	groupbox,
	button,
	checkbox,
	radiobutton,
	label,
	label_centered,
	hyperlabel, // supports multiple markups of <a href="<somelink>">MyLink</a> or <a id="someID">MyID</a>. There can be multiple in one string.
	label_selectable,
	icon,
	textbox,
	textbox_scrollable,
	floatbox, // textbox that only allows floating point (single-precision) numbers to be entered. Specify oDEFAULT for Size values to use icon's values.
	floatbox_spinner,
	combobox, // Supports specifying contents all at once, delimited by '|'. i.e. "Text1|Text2|Text3"
	combotextbox, // Supports specifying contents all at once, delimited by '|'. i.e. "Text1|Text2|Text3"
	progressbar,
	progressbar_unknown, // (marquee)
	tab, // Supports specifying contents all at once, delimited by '|'. i.e. "Text1|Text2|Text3"
	slider,
	slider_selectable, // displays a portion of the slider as selected
	slider_with_ticks, // Same as Slider but tick marks can be added
	listbox, // Supports specifying contents all at once, delimited by '|'. i.e. "Text1|Text2|Text3"
	count,

};}

namespace border_style
{	enum value {

	sunken,
	flat,
	raised,

	count,

};}

namespace gui_event
{	enum value {

	// An event is something that the window issues to client code. Events can be
	// triggered as a side-effect of other actions. Rarely does client code have
	// direct control over events. All events should be downcast to AsShape() 
	// unless described otherwise below.

	// Called when a timer is triggered. Use timers only for infrequent one-shot
	// actions. Do not use this mechanism for consistent updates such as ring
	// buffer monitors or for main loop execution or swap buffer presentation.
	// Use AsTimer() on the event.
	timer,

	activated, // called just after a window comes to be in focus
	deactivated, // called just after a window is no longer in focus

	// called before during window or control creation. This should throw an 
	// exception if there is a failure. It will be caught in the window's factory
	// function and made into an API-consistent error message or rethrown as 
	// appropriate.
	creating, 
	
	// called when the window wants to redraw itself. NOTE: On Windows the user 
	// must call BeginPaint/EndPaint on the event's hSource as documented in 
	// WM_PAINT.
	paint,
	display_changed, // called when the desktop/screens change
	moving, // called just before a window moves
	moved, // called just after a window moves
	sizing, // called just before a window's client area changes size
	sized, // called just after a window's client area changes size
	
	// called before the window is closed. Really this is where client code should 
	// decide whether or not to exit the window's message loop. ouro::gui_event::closed is 
	// too late. All control of the message loop is in client code, so if there
	// is nothing done in ouro::gui_event::closing, then the default behavior is to leave the 
	// window as-is, no closing is actually done.
	closing, 
	
	// called after a commitment to closing the window has been made.
	closed,
	
	// called when a request to become fullscreen is made. Hook can return false 
	// on failure.
	to_fullscreen,
	
	// called when a request to go from fullscreen to windowed is made
	from_fullscreen,

	// Sent if a window had input capture, but something else in the system caused 
	// it to lose capture
	lost_capture, 
	
	// sent if a window gets files drag-dropped on it. Use AsDrop() on event.
	drop_files,

	// Sent when an input device's status has changed. Use AsInputDevice() on 
	// event.
	input_device_changed,

	// The user can trigger a custom event with a user-specified sub-type. 
	// Use AsCustom() on event.
	custom_event,

	count,

};}

namespace gui_action
{	enum value {

	// Actions are similar to events, except client code or the user can trigger 
	// them explicitly and not as the artifact of some other system activity.

	unknown,
	menu,
	control_activated,
	control_deactivated,
	control_selection_changing,
	control_selection_changed,
	hotkey,
	key_down,
	key_up,
	pointer_move,
	skeleton,
	skeleton_acquired,
	skeleton_lost,

	count,

};}

inline input_device_type::value get_type(const input_key::value& _Key)
{
	if (oGUI_IN_RANGE(keyboard, _Key)) return input_device_type::keyboard;
	if (oGUI_IN_RANGE(mouse, _Key)) return input_device_type::mouse;
	if (oGUI_IN_RANGE(joystick, _Key)) return input_device_type::joystick;
	if (oGUI_IN_RANGE(touch, _Key)) return input_device_type::touch;
	return input_device_type::unknown;
}

// A standard key issues both an oGUI_ACTION_KEYDOWN and oGUI_ACTION_KEYUP 
// without special handling.
// Non-standard keys can behave poorly. For example on Windows they are hooked
// by the OS/driver to do something OS-specific and thus do not come through
// as key events, but rather as an app event that is singular, not a down/up.
inline bool is_standard_key(const input_key::value& _Key)
{
	return in_range(input_key::standard_first, input_key::standard_last, _Key);
}

// Invalid, hidden and minimized windows are not visible
inline bool is_visible(const window_state::value& _State) { return _State >= window_state::restored; }

inline bool has_statusbar(const window_style::value& _Style)
{
	switch (_Style)
	{
		case window_style::fixed_with_statusbar:
		case window_style::fixed_with_menu_and_statusbar:
		case window_style::sizable_with_statusbar:
		case window_style::sizable_with_menu_and_statusbar: return true;
		default: break;
	}
	return false;
}

inline bool has_menu(const window_style::value& _Style)
{
	switch (_Style)
	{
		case window_style::fixed_with_menu:
		case window_style::fixed_with_menu_and_statusbar:
		case window_style::sizable_with_menu:
		case window_style::sizable_with_menu_and_statusbar: return true;
		default: break;
	}
	return false;
}

inline bool is_shape_event(const gui_event::value& _Event)
{
	switch (_Event)
	{
		case gui_event::creating: case gui_event::timer: case gui_event::drop_files: 
		case gui_event::input_device_changed: case gui_event::custom_event: return false;
		default: break;
	}
	return true;
}

} // namespace ouro

struct oGUI_TRACKING_CLIPPING
{
	oGUI_TRACKING_CLIPPING()
		: Left(false)
		, Right(false)
		, Top(false)
		, Bottom(false)
		, Front(false)
		, Back(false)
	{}

	bool Left : 1;
	bool Right : 1;
	bool Top : 1;
	bool Bottom : 1;
	bool Front : 1;
	bool Back : 1;
};

struct oGUI_BONE_DESC
{
	oGUI_BONE_DESC(unsigned int _SourceID = 0)
		: SourceID(_SourceID)
	{
		Positions.fill(float4(0.0f, 0.0f, 0.0f, -1.0f));
	}

	unsigned int SourceID;
	oGUI_TRACKING_CLIPPING Clipping;
	std::array<float4, ouro::skeleton_bone::count> Positions;
};

struct oGUI_MENU_ITEM_DESC_NO_CTOR
{
	const char* Text;
	bool Enabled;
	bool Checked;
};

struct oGUI_MENU_ITEM_DESC : oGUI_MENU_ITEM_DESC_NO_CTOR
{
	oGUI_MENU_ITEM_DESC()
	{
		Text = "";
		Enabled = true;
		Checked = false;
	}
};

// Abstractions for native platform handles
oDECLARE_HANDLE(oGUI_DRAW_CONTEXT);
oDECLARE_HANDLE(oGUI_WINDOW);
oDECLARE_HANDLE(oGUI_SKELETON);
oDECLARE_HANDLE(oGUI_MENU);
oDECLARE_DERIVED_HANDLE(oGUI_WINDOW, oGUI_STATUSBAR);
oDECLARE_DERIVED_HANDLE(oGUI_WINDOW, oGUI_CONTROL);
oDECLARE_HANDLE(oGUI_FONT);
oDECLARE_HANDLE(oGUI_ICON);
oDECLARE_HANDLE(oGUI_CURSOR);

struct oGUI_WINDOW_DESC
{
	oGUI_WINDOW_DESC()
		: hParent(nullptr)
		, hOwner(nullptr)
		, hIcon(nullptr)
		, ClientPosition(oDEFAULT, oDEFAULT)
		, ClientSize(oDEFAULT, oDEFAULT)
		, State(ouro::window_state::restored)
		, Style(ouro::window_style::fixed)
		, Debug(false)
		, Enabled(true)
		, HasFocus(true)
		, AlwaysOnTop(false)
		, ShowMenu(false)
		, ShowStatusBar(false)
		, DefaultEraseBackground(true)
		, AllowClientDragToMove(false)
		, AllowAltF1(true)
		, AllowAltF4(true)
		, AllowAltEnter(true)
		, AllowTouch(true)
		, EnableMainLoopEvent(true)
		, EnableDeviceChangeEvent(true)
	{
		StatusWidths.fill(-1);
	}

	// Specifying a parent makes this window a child window, meaning it becomes a 
	// control (see oGUI_CONTROL code below). This means top-level decoration such
	// as a border or min/max/restore controls will not be shown.
	oGUI_WINDOW hParent;

	// Owner is the relationship of this window as a (modal) dialog box to that 
	// which it has disabled. By specifying an owner the relationship will be 
	// preserved.
	oGUI_WINDOW hOwner;

	// The icon displayed when the window is minimized and appears in the upper
	// left of the window's border on some systems. When set, the window owns the
	// lifetime of the hIcon. If the icon is then changed, the prior icon is still
	// the responsibility of client code, so if changing an icon, grab the old one
	// first and free it after it has been disassociated from the window.
	oGUI_ICON hIcon;

	int2 ClientPosition;
	int2 ClientSize;
	ouro::window_state::value State;
	ouro::window_style::value Style;
	std::array<short, 8> StatusWidths;

	// If true, platform-specific action and event information will be spewed to 
	// the debug window and log file.
	bool Debug;
	
	// If true the window accepts actions.
	bool Enabled;

	// Only one window on an operating system can have focus. If true, this window
	// is the one.
	bool HasFocus;

	// If true this is in a class of windows that self-sort separately from normal
	// windows. There may still be other operating system windows with even higher
	// sort priority (such as the Windows task bar) so this may be inadequate in
	// final fullscreen deployments.
	bool AlwaysOnTop;

	bool ShowMenu;
	bool ShowStatusBar;

	// By default this should be true to allow the operating system to do its 
	// normal painting. For media windows that use complex paint algorithms (such
	// as video update or 3D rendering) this should be set false to avoid conflict 
	// with more robust clearing handled by the media algorithm.
	bool DefaultEraseBackground;

	// If true, the window can be moved by left-clicking anywhere in the client
	// area and dragging.
	bool AllowClientDragToMove;

	// allow toggle of mouse cursor visibility
	bool AllowAltF1;

	// allow immediate close of window
	bool AllowAltF4;

	// allow toggle of fullscreen (exclusive or cooperative)
	bool AllowAltEnter;
	
	// Touch often causes mouse messages to behave atypically, so only enable 
	// touch if the application is specifically ready for it.
	bool AllowTouch;

	// By default this should be false as most operating system GUIs are event-
	// driven. For media windows or other "main loop" simulations that use complex 
	// paint algorithms (such as video update or 3D rendering) this should be set 
	// true and logic to refresh the client area should be handled in the main 
	// loop event.
	bool EnableMainLoopEvent;

	// Enable extra bookkeeping and registration for the window to respond to the
	// oGUI_EVENT_DEVICE_CHANGE message. If this is false, this message may not 
	// occur.
	bool EnableDeviceChangeEvent;
};

struct oGUI_WINDOW_SHAPE_DESC
{
	oGUI_WINDOW_SHAPE_DESC()
		: State(ouro::window_state::invalid)
		, Style(ouro::window_style::default_style)
		, ClientPosition(oDEFAULT, oDEFAULT)
		, ClientSize(oDEFAULT, oDEFAULT)
	{}

	// The desired Shape of the window. Minimize and maximize will override/ignore  
	// ClientPosition and ClientSize values. Use ouro::window_state::invalid to indicate
	// that the state should remain untouched (only respect ClientPosition and 
	// ClientSize values).
	ouro::window_state::value State;

	// The desired style of the non-client area of the window (the OS decoration)
	// Use ouro::window_style::default_style to indicate that the state should remain 
	// untouched. Changing style will not affect client size/position.
	ouro::window_style::value Style;

	// This always refers to non-minimized, non-maximized Shapes. oDEFAULT values
	// imply "use whatever was there before". For example, if the state is set to 
	// maximized, and some non-default value is applied to ClientSize, then the 
	// next time the state is set to restored. Changing the style while maximized 
	// can also change the dimensions, but since the state's goal is to maximize
	// client area, these values will be ignored.
	int2 ClientPosition;
	int2 ClientSize;
};

// This only describes the cursor when over a particular window, not the global
// cursor.
struct oGUI_WINDOW_CURSOR_DESC
{
	oGUI_WINDOW_CURSOR_DESC()
		: ClientState(ouro::cursor_state::arrow)
		, NonClientState(ouro::cursor_state::arrow)
		, HasCapture(false)
	{}

	ouro::cursor_state::value ClientState; // state of cursor when in the client area
	ouro::cursor_state::value NonClientState; // state of cursor when in the OS decoration area
	bool HasCapture; // forces messages to the window even if the focus/cursor is outside the window. Setting this to true will also bring the window to the foreground.
};

// _____________________________________________________________________________
// oGUI_WINDOW events

struct oGUI_EVENT_CREATE_DESC;
struct oGUI_EVENT_SHAPE_DESC;
struct oGUI_EVENT_TIMER_DESC;
struct oGUI_EVENT_DROP_DESC;
struct oGUI_EVENT_INPUT_DEVICE_DESC;
struct oGUI_EVENT_CUSTOM_DESC;

struct oGUI_EVENT_DESC
{
	oGUI_EVENT_DESC(oGUI_WINDOW _hWindow, ouro::gui_event::value _Type)
		: hWindow(_hWindow)
		, Type(_Type)
	{}

	// Native window handle
	oGUI_WINDOW hWindow;

	// Type of event. This is the base class; use the downcasting accessors below
	// based on this type value.
	ouro::gui_event::value Type;

	// union doesn't work because of int2's copy ctor, so use downcasting 
	// accessors.
	inline const oGUI_EVENT_CREATE_DESC& AsCreate() const;
	inline const oGUI_EVENT_SHAPE_DESC& AsShape() const;
	inline const oGUI_EVENT_TIMER_DESC& AsTimer() const;
	inline const oGUI_EVENT_DROP_DESC& AsDrop() const;
	inline const oGUI_EVENT_INPUT_DEVICE_DESC& AsInputDevice() const;
	inline const oGUI_EVENT_CUSTOM_DESC& AsCustom() const;
};

struct oGUI_EVENT_CREATE_DESC : oGUI_EVENT_DESC
{
	oGUI_EVENT_CREATE_DESC(oGUI_WINDOW _hWindow
		, oGUI_STATUSBAR _hStatusBar
		, oGUI_MENU _hMenu
		, const oGUI_WINDOW_SHAPE_DESC& _Shape
		, void* _pUser)
		: oGUI_EVENT_DESC(_hWindow, ouro::gui_event::creating)
		, hStatusBar(_hStatusBar)
		, hMenu(_hMenu)
		, Shape(_Shape)
		, pUser(_pUser)
	{}

	// Native handle of the window's status bar.
	oGUI_STATUSBAR hStatusBar;

	// Native handle of the top-level window's menu.
	oGUI_MENU hMenu;
	oGUI_WINDOW_SHAPE_DESC Shape;

	// The user can pass a value to this for usage.
	void* pUser;
};

struct oGUI_EVENT_SHAPE_DESC : oGUI_EVENT_DESC
{
	oGUI_EVENT_SHAPE_DESC(oGUI_WINDOW _hWindow
		, ouro::gui_event::value _Type
		, const oGUI_WINDOW_SHAPE_DESC& _Shape)
		: oGUI_EVENT_DESC(_hWindow, _Type)
		, Shape(_Shape)
	{}

	oGUI_WINDOW_SHAPE_DESC Shape;
};

struct oGUI_EVENT_TIMER_DESC : oGUI_EVENT_DESC
{
	oGUI_EVENT_TIMER_DESC(oGUI_WINDOW _hWindow, uintptr_t _Context)
		: oGUI_EVENT_DESC(_hWindow, ouro::gui_event::timer)
		, Context(_Context)
	{}

	// Any pointer-sized value. It is recommended this be the address of a field
	// in the App's class that is the struct context for the timer event.
	uintptr_t Context;
};

struct oGUI_EVENT_DROP_DESC : oGUI_EVENT_DESC
{
	oGUI_EVENT_DROP_DESC(oGUI_WINDOW _hWindow
		, const ouro::path_string* _pPaths
		, int _NumPaths
		, const int2& _ClientDropPosition)
		: oGUI_EVENT_DESC(_hWindow, ouro::gui_event::drop_files)
		, pPaths(_pPaths)
		, NumPaths(_NumPaths)
		, ClientDropPosition(_ClientDropPosition)
	{}
	const ouro::path_string* pPaths;
	int NumPaths;
	int2 ClientDropPosition;
};

struct oGUI_EVENT_INPUT_DEVICE_DESC : oGUI_EVENT_DESC
{
	oGUI_EVENT_INPUT_DEVICE_DESC(oGUI_WINDOW _hWindow
		, ouro::input_device_type::value _Type
		, ouro::input_device_status::value _Status
		, const char* _InstanceName)
		: oGUI_EVENT_DESC(_hWindow, ouro::gui_event::input_device_changed)
		, Type(_Type)
		, Status(_Status)
		, InstanceName(_InstanceName)
	{}

	ouro::input_device_type::value Type;
	ouro::input_device_status::value Status;
	const char* InstanceName;
};

struct oGUI_EVENT_CUSTOM_DESC : oGUI_EVENT_DESC
{
	oGUI_EVENT_CUSTOM_DESC(oGUI_WINDOW _hWindow, int _EventCode, uintptr_t _Context)
		: oGUI_EVENT_DESC(_hWindow, ouro::gui_event::custom_event)
		, EventCode(_EventCode)
		, Context(_Context)
	{}

	int EventCode;
	uintptr_t Context;
};

const oGUI_EVENT_CREATE_DESC& oGUI_EVENT_DESC::AsCreate() const { oASSERT(Type == ouro::gui_event::creating, "wrong type"); return *static_cast<const oGUI_EVENT_CREATE_DESC*>(this); }
const oGUI_EVENT_SHAPE_DESC& oGUI_EVENT_DESC::AsShape() const { oASSERT(ouro::is_shape_event(Type), "wrong type"); return *static_cast<const oGUI_EVENT_SHAPE_DESC*>(this); }
const oGUI_EVENT_TIMER_DESC& oGUI_EVENT_DESC::AsTimer() const { oASSERT(Type == ouro::gui_event::timer, "wrong type"); return *static_cast<const oGUI_EVENT_TIMER_DESC*>(this); }
const oGUI_EVENT_DROP_DESC& oGUI_EVENT_DESC::AsDrop() const { oASSERT(Type == ouro::gui_event::drop_files, "wrong type"); return *static_cast<const oGUI_EVENT_DROP_DESC*>(this); }
const oGUI_EVENT_INPUT_DEVICE_DESC& oGUI_EVENT_DESC::AsInputDevice() const { oASSERT(Type == ouro::gui_event::input_device_changed, "wrong type"); return *static_cast<const oGUI_EVENT_INPUT_DEVICE_DESC*>(this); }
const oGUI_EVENT_CUSTOM_DESC& oGUI_EVENT_DESC::AsCustom() const { oASSERT(Type == ouro::gui_event::custom_event, "wrong type"); return *static_cast<const oGUI_EVENT_CUSTOM_DESC*>(this); }

typedef std::function<void(const oGUI_EVENT_DESC& _Event)> oGUI_EVENT_HOOK;

struct oGUI_ACTION_DESC
{
	// All actions use this desc. This can be filled out manually and submitted
	// to an action handler to spoof hardware events, for example from a network
	// stream thus enabling remote access.

	oGUI_ACTION_DESC()
		: hWindow(nullptr)
		, Action(ouro::gui_action::unknown)
		, DeviceType(ouro::input_device_type::unknown)
		, DeviceID(-1)
		, Position(0.0f)
		//, Key(NONE)
		//, ActionCode(0)
	{ hSkeleton = nullptr; }

	oGUI_ACTION_DESC(
		oGUI_WINDOW _hWindow
		, unsigned int _TimestampMS
		, ouro::gui_action::value _Action
		, ouro::input_device_type::value _DeviceType
		, int _DeviceID)
			: hWindow(_hWindow)
			, TimestampMS(_TimestampMS)
			, Action(_Action)
			, DeviceType(_DeviceType)
			, DeviceID(_DeviceID)
			, Position(0.0f)
			//, Key(NONE)
			//, ActionCode(0)
	{ hSkeleton = nullptr; }

	// _____________________________________________________________________________
	// Common across all actions

	// Control devices have their own handle and sometimes their own sub-action.
	oGUI_WINDOW hWindow;

	// Time at which the message was sent in milliseconds.
	unsigned int TimestampMS;

	ouro::gui_action::value Action;
	ouro::input_device_type::value DeviceType;

	// When there are multiple devices of the same type, this differentiates. For
	// example if this is a gesture, then this would be the tracking/skeleton ID.
	// A mouse or keyboard usually only have one associated, so this is typically
	// not used there. Joysticks would be the ID of each individual one, and for 
	// controls it is the ID associated with the control.
	int DeviceID;

	// For touch and mouse XY are typical coords and Z is the mouse wheel.
	// For gesture it is the 3D position whose W can typically be ignored, but 
	// might be indicative of validity.
	// For joysticks xy is the left-most axis and zw is the right-most axis.
	float4 Position;
	
	union
	{
		struct 
		{
			// Any binary key is described by this, from a keyboard key to a touch event 
			// to a mouse button to a gesture volume activation (air-key).
			ouro::input_key::value Key;

			// For some specific types of controls, this is an additional action value.
			int ActionCode;
		};

		// if Action is an ouro::gui_action::skeleton, the hSkeleton is  a handle fit for 
		// use with a platform-specific accessor to the actual skeleton data.
		oGUI_SKELETON hSkeleton;
	};
};

typedef std::function<void(const oGUI_ACTION_DESC& _Action)> oGUI_ACTION_HOOK;

// So that hotkeys can be statically defined without a "non-aggregates cannot be 
// initialized" warning
struct oGUI_HOTKEY_DESC_NO_CTOR
{
	ouro::input_key::value HotKey;
	unsigned short ID;
	bool AltDown;
	bool CtrlDown;
	bool ShiftDown;
};

struct oGUI_HOTKEY_DESC : oGUI_HOTKEY_DESC_NO_CTOR
{
	oGUI_HOTKEY_DESC()
	{
		HotKey = ouro::input_key::none;
		ID = 0;
		AltDown = false;
		CtrlDown = false;
		ShiftDown = false;
	}

	oGUI_HOTKEY_DESC(const oGUI_HOTKEY_DESC_NO_CTOR& _That) { operator=(_That); }
	oGUI_HOTKEY_DESC(const oGUI_HOTKEY_DESC& _That) { operator=(_That); }
	const oGUI_HOTKEY_DESC& operator=(const oGUI_HOTKEY_DESC_NO_CTOR& _That) { *(oGUI_HOTKEY_DESC_NO_CTOR*)this = _That; }
	const oGUI_HOTKEY_DESC& operator=(const oGUI_HOTKEY_DESC& _That) { *this = _That; }
};

struct oGUI_CONTROL_DESC
{
	oGUI_CONTROL_DESC()
		: hParent(nullptr)
		, hFont(nullptr)
		, Type(ouro::control_type::unknown)
		, Text("")
		, Position(-1, -1)
		, Size(oDEFAULT, oDEFAULT)
		, ID(0xffff)
		, StartsNewGroup(false)
	{}

	// All controls must have a parent
	oGUI_WINDOW hParent;

	// If nullptr is specified, then the system default font will be used.
	oGUI_FONT hFont;

	// Type of control to create
	ouro::control_type::value Type;
	
	// Any item that contains a list of strings can set this to be
	// a '|'-terminated set of strings to immediately populate all
	// items. ("Item1|Item2|Item3"). This is valid for:
	// COMBOBOX, COMBOTEXTBOX, TAB
	union
	{
		const char* Text;
		oGUI_ICON Icon;
	};

	int2 Position;
	int2 Size;
	unsigned short ID;
	bool StartsNewGroup;
};

struct oGUI_FONT_DESC
{
	oGUI_FONT_DESC()
		: FontName("Tahoma")
		, PointSize(10.0f)
		, Bold(false)
		, Italic(false)
		, Underline(false)
		, StrikeOut(false)
		, AntiAliased(true)
	{}

	ouro::sstring FontName;
	float PointSize;
	bool Bold;
	bool Italic;
	bool Underline;
	bool StrikeOut;
	bool AntiAliased;
};

struct oGUI_TEXT_DESC
{
	oGUI_TEXT_DESC()
		: Position(int2(oDEFAULT, oDEFAULT))
		, Size(int2(oDEFAULT, oDEFAULT))
		, Alignment(ouro::alignment::top_left)
		, Foreground(ouro::White)
		, Background(0)
		, Shadow(0)
		, ShadowOffset(int2(2,2))
		, SingleLine(false)
	{}

	// Position and Size define a rectangle in which text is drawn. Alignment 
	// provides addition offsetting within that rectangle. So to center text on
	// the screen, the position would be 0,0 and size would be the resolution of 
	// the screen, and alignment would be middle-center.
	int2 Position;
	int2 Size;
	ouro::alignment::value Alignment;
	// Any non-1.0 (non-0xff) alpha will be not-drawn
	ouro::color Foreground;
	ouro::color Background;
	ouro::color Shadow;
	int2 ShadowOffset;
	bool SingleLine;
};

// @tony: Can oInputMapper replace this?
// A utility function to analyze the specified action and compare it for keydown
// and keyup events from keys specified in the keys array. If there's a match,
// the corresponding keystate is marked as true if down, and false if up. NOTE:
// _NumKeys can be a multiple of _NumKeyStates to support multiple key bindings
// for the same KeyState. i.e. if you have two states LEFT and RIGHT and you 
// want to bind A and D, but also left-arrow and right-arrow, that can be done
// with an array of [A,D,left,right] and the Keystates will be written correctly
// for LEFT and RIGHT. If the action is a pointer move, then the position is 
// recorded to _pPointerPosition.
void oGUIRecordInputState(const oGUI_ACTION_DESC& _Action, const ouro::input_key::value* _pKeys, size_t _NumKeys, bool* _pKeyStates, size_t _NumKeyStates, float3* _pPointerPosition);
template<size_t NumKeys, size_t NumKeyStates> void oGUIRecordInputState(const oGUI_ACTION_DESC& _Action, const ouro::input_key::value (&_pKeys)[NumKeys], bool (&_pKeyStates)[NumKeyStates], float3* _pPointerPosition) { oGUIRecordInputState(_Action, _pKeys, NumKeys, _pKeyStates, NumKeyStates, _pPointerPosition); }

// _____________________________________________________________________________
// Rectangle utils

// Clips the child (to-be-clipped) rectangle against the parent.
inline oRECT oGUIClipRect(const oRECT& _Parent, const oRECT& _ToBeClipped) { oRECT r = _ToBeClipped; r.Min.x = __max(r.Min.x, _Parent.Min.x); r.Min.y = __max(r.Min.y, _Parent.Min.y); r.Max.x = __min(r.Max.x, _Parent.Max.x); r.Max.y = __min(r.Max.y, _Parent.Max.y); return r; }

// Replaces any oDEFAULT values with the specified default value.
inline int2 oGUIResolveRectSize(const int2& _Size, const int2& _DefaultSize) { int2 result(_Size); if (result.x == oDEFAULT) result.x = _DefaultSize.x; if (result.y == oDEFAULT) result.y = _DefaultSize.y; return result; }

// Replaces any oDEFAULT values with 0.
inline int2 oGUIResolveRectPosition(const int2& _Position, const int2& _DefaultPosition = int2(0, 0)) { int2 result(_Position); if (result.x == oDEFAULT) result.x = _DefaultPosition.x; if (result.y == oDEFAULT) result.y = _DefaultPosition.y; return result; }

// Positions a child rectangle "inside" (parent can be smaller than the child)
// the specified parent according to the specified alignment and clipping. In 
// non-fit alignments, position will be respected as an offset from the anchor
// calculated for the specified alignment. For example, if a rect is right-
// aligned, and there is a position of x=-10, then the rectangle is inset an
// additional 10 units. oDEFAULT for position most often evaluates to a zero 
// offset.
oRECT oGUIResolveRect(const oRECT& _Parent, const oRECT& _UnadjustedChild, ouro::alignment::value _Alignment, bool _Clip);
inline oRECT oGUIResolveRect(const oRECT& _Parent, const int2& _UnadjustedChildPosition, const int2& _UnadjustedChildSize, ouro::alignment::value _Alignment, bool _Clip) { oRECT r; r.Min = oGUIResolveRectPosition(_UnadjustedChildPosition); r.Max = r.Min + _UnadjustedChildSize; return oGUIResolveRect(_Parent, r, _Alignment, _Clip); }

#endif
