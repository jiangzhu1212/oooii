/**************************************************************************
 * The MIT License                                                        *
 * Copyright (c) 2014 Antony Arciuolo.                                    *
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
#pragma once
#ifndef oBase_input_h
#define oBase_input_h

#include <oBase/invalid.h>
#include <oBase/macros.h>
#include <oBase/resized_type.h>
#include <oHLSL/oHLSLTypes.h>
#include <array>

namespace ouro {

	namespace input {

enum type
{
	unknown,
	keyboard,
	mouse,
	joystick,
	control, // i.e. a button or scrollbar
	skeleton,
	voice,
	touch,

	type_count,
};

enum status
{
	ready,
	initializing,
	not_connected,
	is_clone,
	not_supported,
	insufficient_bandwidth,
	low_power,
	not_powered,
	not_ready,

	status_count,
};

enum key
{
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

	key_count,
};

enum skeleton_bone
{	
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
	
	bone_count,
	invalid_bone = bone_count,
};

enum action_type
{
	unhandled,
	
	// GUI elements
	menu,
	control_activated,
	control_deactivated,
	control_selection_changing,
	control_selection_changed,
	hotkey,
	
	// joystick/keyboard/mouse
	key_down,
	key_up,
	pointer_move,

	// human form tracking
	skeleton_update,
	skeleton_acquired,
	skeleton_lost,

	action_type_count,
};

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
	std::array<float4, bone_count> positions;
};

struct action
{
	// All input communication should provide this struct. This can also be 
	// populated and sent to an action handler to spoof hardware events, for 
	// example from a network stream thus enabling remote access.

	action()
		: window(nullptr)
		, timestamp_ms(0)
		, device_id(invalid)
		, device_type(unknown)
		, action_type(unhandled)
		, key(none)
		, action_code(invalid)
		, position_x(0.0f)
		, position_y(0.0f)
		, position_z(0.0f)
		, position_w(0.0f)
	{ /*skeleton = nullptr;*/ }

	action(
		void* _hWindow
		, unsigned int _TimestampMS
		, unsigned int _DeviceID
		, type _DeviceType
		, action_type _ActionType
		, key _Key = none
		, unsigned int _ActionCode = invalid)
			: window(_hWindow)
			, timestamp_ms(_TimestampMS)
			, device_id(_DeviceID)
			, device_type(_DeviceType)
			, action_type(_ActionType)
			, key(_Key)
			, action_code(_ActionCode)
			, position_x(0.0f)
			, position_y(0.0f)
			, position_z(0.0f)
			, position_w(0.0f)
	{ /*skeleton = nullptr;*/ }

	// For operating systems that support multiple applications this gives the 
	// handle of the context handling input actions. If a control action, this is 
	// the control on which the action occurred.
	void* window;

	// Time since the start of the app when the message was sent in milliseconds.
	unsigned int timestamp_ms;

	// When there are multiple devices of the same type, this differentiates. For
	// example if this is a gesture, then this would be the tracking/skeleton ID.
	// A mouse or keyboard usually only have one associated, so this is typically
	// not used there. Joysticks would be the ID of each individual one, and for 
	// controls it is the ID associated with the control.
	unsigned int device_id;

	// Describes the device generating this action.
	resized_type<type, char> device_type;

	// Describes the type of action generated.
	resized_type<action_type, char> action_type;
	
	// Any binary (up/down) key such as from a joystick, keyboard or mouse.
	resized_type<key, unsigned short> key;

	// An additional action value for certain types of controls.
	unsigned int action_code;

	union
	{
		// For touch and mouse XY are typical coords and Z is the mouse wheel.
		// For other pointer types gesture it is the 3D position whose W can 
		// typically be ignored but might be indicative of validity.
		// For joysticks xy is the left-most axis and zw is the right-most axis.
		// This is not valid for skeleton actions - use the handle below to retreive 
		// more robust data.
		// Note: float4's copy ctor keeps it out of the union, so work around that
		// by declaring the simple types here and declaring an accessor.
		struct { float position_x, position_y, position_z, position_w; };

		// For human form tracking this handle can be used to access more robust
		// data.
		// Valid for: skeleton, skeleton_acquired, skeleton_list.
		void* skeleton;
	};

	float4 position() const { return *(float4*)&position_x; }
	void position(const float4& _Position) { *(float4*)&position_x = _Position; }
};

typedef std::function<void(const action& _Action)> action_hook;

inline type get_type(const key& _Key)
{
	#define IF_IS(_DeviceType) do { if (_Key >= _DeviceType##_first && _Key <= _DeviceType##_last) return _DeviceType; } while(false)
	IF_IS(keyboard); IF_IS(mouse); IF_IS(joystick); IF_IS(touch);
	return unknown;
	#undef IF_IS
}

// A standard key issues both an keydown and keyup without special handling.
// Non-standard keys can behave poorly. For example on Windows they are hooked
// by the OS/driver to do something OS-specific and thus do not come through
// as key events, but rather as an app event that is singular, not a down/up.
inline bool is_standard(const key& _Key) { return _Key >= standard_first && _Key <= standard_last; }

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
void record_state(const ouro::input::action& _Action, const ouro::input::key* _pKeys, size_t _NumKeys, bool* _pKeyStates, size_t _NumKeyStates, float3* _pPointerPosition);
template<size_t NumKeys, size_t NumKeyStates> void record_state(const ouro::input::action& _Action, const ouro::input::key (&_pKeys)[NumKeys], bool (&_pKeyStates)[NumKeyStates], float3* _pPointerPosition) { record_state(_Action, _pKeys, NumKeys, _pKeyStates, NumKeyStates, _pPointerPosition); }

	} // namespace input
}

#endif
