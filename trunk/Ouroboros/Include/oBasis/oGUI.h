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
#include <oBase/macros.h>
#include <oCompute/oAABox.h>
#include <array>
#include <functional>

namespace ouro {

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

oDECLARE_HANDLE(draw_context_handle);
oDECLARE_HANDLE(window_handle);
oDECLARE_HANDLE(skeleton_handle);
oDECLARE_HANDLE(menu_handle);
oDECLARE_DERIVED_HANDLE(window_handle, statusbar_handle);
oDECLARE_DERIVED_HANDLE(window_handle, control_handle);
oDECLARE_HANDLE(font_handle);
oDECLARE_HANDLE(icon_handle);
oDECLARE_HANDLE(cursor_handle);

struct tracking_clipping
{
	tracking_clipping()
		: left(false)
		, right(false)
		, top(false)
		, bottom(false)
		, front(false)
		, back(false)
	{}

	bool left : 1;
	bool right : 1;
	bool top : 1;
	bool bottom : 1;
	bool front : 1;
	bool back : 1;
};

struct tracking_skeleton
{
	tracking_skeleton(unsigned int _SourceID = 0)
		: source_id(_SourceID)
	{ positions.fill(float4(0.0f, 0.0f, 0.0f, -1.0f)); }

	unsigned int source_id;
	tracking_clipping clipping;
	std::array<float4, skeleton_bone::count> positions;
};

struct window_shape
{
	window_shape()
		: state(window_state::invalid)
		, style(window_style::default_style)
		, client_position(oDEFAULT, oDEFAULT)
		, client_size(oDEFAULT, oDEFAULT)
	{}

	// Minimize and maximize will override/ignore client_position and client_size 
	// values. Use window_state::invalid to indicate the state should remain 
	// untouched (only respect client_position and client_size values).
	window_state::value state;

	// Use window_style::default_style to indicate that the style should remain 
	// untouched. Changing style will not affect client size/position.
	window_style::value style;

	// This always refers to non-minimized, non-maximized states. oDEFAULT values 
	// imply "use whatever was there before". For example if the state is set to 
	// maximized and some non-default value is applied to client_size then the 
	// next time the state is set to restored the window will have that new size. 
	int2 client_position;
	int2 client_size;
};

struct window_cursor_shape
{
	// This only describes the cursor when over a particular window, not the 
	// global cursor.

	window_cursor_shape()
		: client_state(cursor_state::arrow)
		, nonclient_state(cursor_state::arrow)
		, has_capture(false)
	{}

	// state of cursor when in the client area
	cursor_state::value client_state;
	
	// state of cursor when in the OS decoration area
	cursor_state::value nonclient_state; 
	
	// forces messages to the window even if the focus/cursor is outside the 
	// window. Setting this to true will also bring the window to the foreground.
	bool has_capture;
};

struct action_info
{
	// All actions use this desc. This can be filled out manually and submitted
	// to an action handler to spoof hardware events, for example from a network
	// stream thus enabling remote access.

	action_info()
		: window(nullptr)
		, action(gui_action::unknown)
		, device_type(input_device_type::unknown)
		, device_id(-1)
		, position(0.0f)
		//, Key(NONE)
		//, ActionCode(0)
	{ skeleton = nullptr; }

	action_info(
		window_handle _hWindow
		, unsigned int _TimestampMS
		, gui_action::value _Action
		, input_device_type::value _DeviceType
		, int _DeviceID)
			: window(_hWindow)
			, timestamp_ms(_TimestampMS)
			, action(_Action)
			, device_type(_DeviceType)
			, device_id(_DeviceID)
			, position(0.0f)
			//, Key(NONE)
			//, ActionCode(0)
	{ skeleton = nullptr; }

	// Control devices have their own handle and sometimes their own sub-action.
	window_handle window;

	// Time at which the message was sent in milliseconds.
	unsigned int timestamp_ms;

	gui_action::value action;
	input_device_type::value device_type;

	// When there are multiple devices of the same type, this differentiates. For
	// example if this is a gesture, then this would be the tracking/skeleton ID.
	// A mouse or keyboard usually only have one associated, so this is typically
	// not used there. Joysticks would be the ID of each individual one, and for 
	// controls it is the ID associated with the control.
	int device_id;

	// For touch and mouse XY are typical coords and Z is the mouse wheel.
	// For gesture it is the 3D position whose W can typically be ignored, but 
	// might be indicative of validity.
	// For joysticks xy is the left-most axis and zw is the right-most axis.
	float4 position;
	
	union
	{
		struct 
		{
			// Any binary key is described by this, from a keyboard key to a touch event 
			// to a mouse button to a gesture volume activation (air-key).
			input_key::value key;

			// For some specific types of controls, this is an additional action 
			// value.
			int action_code;
		};

		// if Action is an gui_action::skeleton, the hSkeleton is  a handle fit for 
		// use with a platform-specific accessor to the actual skeleton data.
		skeleton_handle skeleton;
	};
};

typedef std::function<void(const action_info& _Action)> action_hook;


inline input_device_type::value get_type(const input_key::value& _Key)
{
	#define IF_IS(_DeviceType) do { if (_Key >= input_key::_DeviceType##_first && _Key <= input_key::_DeviceType##_last) return input_device_type::_DeviceType; } while(false)
	IF_IS(keyboard); IF_IS(mouse); IF_IS(joystick); IF_IS(touch);
	return input_device_type::unknown;
	#undef IF_IS
}

// A standard key issues both an keydown and keyup without special handling.
// Non-standard keys can behave poorly. For example on Windows they are hooked
// by the OS/driver to do something OS-specific and thus do not come through
// as key events, but rather as an app event that is singular, not a down/up.
inline bool is_standard_key(const input_key::value& _Key)
{
	return _Key >= input_key::standard_first && _Key <= input_key::standard_last;
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
	ouro::window_handle hParent;

	// If nullptr is specified, then the system default font will be used.
	ouro::font_handle hFont;

	// Type of control to create
	ouro::control_type::value Type;
	
	// Any item that contains a list of strings can set this to be
	// a '|'-terminated set of strings to immediately populate all
	// items. ("Item1|Item2|Item3"). This is valid for:
	// COMBOBOX, COMBOTEXTBOX, TAB
	union
	{
		const char* Text;
		ouro::icon_handle Icon;
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
void oGUIRecordInputState(const ouro::action_info& _Action, const ouro::input_key::value* _pKeys, size_t _NumKeys, bool* _pKeyStates, size_t _NumKeyStates, float3* _pPointerPosition);
template<size_t NumKeys, size_t NumKeyStates> void oGUIRecordInputState(const ouro::action_info& _Action, const ouro::input_key::value (&_pKeys)[NumKeys], bool (&_pKeyStates)[NumKeyStates], float3* _pPointerPosition) { oGUIRecordInputState(_Action, _pKeys, NumKeys, _pKeyStates, NumKeyStates, _pPointerPosition); }

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
