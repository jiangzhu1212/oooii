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

#include <oBase/fixed_string.h>
#include <oBasis/oStddef.h>
#include <oBasis/oRTTI.h>
#include <array>

// Commonly used when creating enums of GUI functionality to have associated
// runs in the enum - such as for a radio selection list.
inline bool oGUIInRange(int _First, int _Last, int _Value) { return _Value >= _First && _Value <= _Last; }
#define oGUI_IN_RANGE(_EnumPrefix, _Value) oGUIInRange(_EnumPrefix##_FIRST, _EnumPrefix##_LAST, _Value)

enum oGUI_INPUT_DEVICE_TYPE
{
	oGUI_INPUT_DEVICE_UNKNOWN,
	oGUI_INPUT_DEVICE_KEYBOARD,
	oGUI_INPUT_DEVICE_MOUSE,
	oGUI_INPUT_DEVICE_JOYSTICK,
	oGUI_INPUT_DEVICE_CONTROL, // i.e. A button or scrollbar
	oGUI_INPUT_DEVICE_SKELETON,
	oGUI_INPUT_DEVICE_VOICE,
	oGUI_INPUT_DEVICE_TOUCH,

	oGUI_INPUT_DEVICE_TYPE_COUNT,
};
oRTTI_ENUM_DECLARATION(oRTTI_CAPS_ARRAY, oGUI_INPUT_DEVICE_TYPE)

enum oGUI_INPUT_DEVICE_STATUS
{
	oGUI_INPUT_DEVICE_READY,
	oGUI_INPUT_DEVICE_INITIALIZING,
	oGUI_INPUT_DEVICE_NOT_CONNECTED,
	oGUI_INPUT_DEVICE_IS_CLONE,
	oGUI_INPUT_DEVICE_NOT_SUPPORTED,
	oGUI_INPUT_DEVICE_INSUFFICIENT_BANDWIDTH,
	oGUI_INPUT_DEVICE_LOW_POWER,
	oGUI_INPUT_DEVICE_NOT_POWERED,
	oGUI_INPUT_DEVICE_NOT_READY,

	oGUI_INPUT_DEVICE_STATUS_COUNT,
};
oRTTI_ENUM_DECLARATION(oRTTI_CAPS_ARRAY, oGUI_INPUT_DEVICE_STATUS)

enum oGUI_KEY
{
	oGUI_KEY_NONE,

	// Mouse keys
	oGUI_KEY_MOUSE_LEFT,
	oGUI_KEY_MOUSE_FIRST = oGUI_KEY_MOUSE_LEFT,

	oGUI_KEY_STANDARD_FIRST = oGUI_KEY_MOUSE_FIRST, 
	oGUI_KEY_MOUSE_RIGHT,
	oGUI_KEY_MOUSE_MIDDLE,
	oGUI_KEY_MOUSE_SIDE1,
	oGUI_KEY_MOUSE_SIDE2,
	// Double-click seems to be ubiquitous. In X11 and thus RFB it's its own
	// button and in Windows the button events get eaten and transformed into 
	// different events for double-click. So here favor the X11 model.
	oGUI_KEY_MOUSE_LEFT_DOUBLE,
	oGUI_KEY_MOUSE_RIGHT_DOUBLE,
	oGUI_KEY_MOUSE_MIDDLE_DOUBLE,
	oGUI_KEY_MOUSE_SIDE1_DOUBLE,
	oGUI_KEY_MOUSE_SIDE2_DOUBLE,
	oGUI_KEY_MOUSE_LAST = oGUI_KEY_MOUSE_SIDE2_DOUBLE,

	// Joystick keys
	oGUI_KEY_JOY_LLEFT,
	oGUI_KEY_JOYSTICK_FIRST = oGUI_KEY_JOY_LLEFT, oGUI_KEY_JOY_LUP, oGUI_KEY_JOY_LRIGHT, oGUI_KEY_JOY_LDOWN,
	oGUI_KEY_JOY_RLEFT, oGUI_KEY_JOY_RUP, oGUI_KEY_JOY_RRIGHT, oGUI_KEY_JOY_RDOWN,
	oGUI_KEY_JOY_LSHOULDER1, oGUI_KEY_JOY_LSHOULDER2, oGUI_KEY_JOY_RSHOULDER1, oGUI_KEY_JOY_RSHOULDER2,
	oGUI_KEY_JOY_LTHUMB, oGUI_KEY_JOY_RTHUMB, oGUI_KEY_JOY_SYSTEM, oGUI_KEY_JOY_START, oGUI_KEY_JOY_SELECT,
	oGUI_KEY_JOYSTICK_LAST = oGUI_KEY_JOY_SELECT,

	// Control keys
	oGUI_KEY_LCTRL,
	oGUI_KEY_KEYBOARD_FIRST = oGUI_KEY_LCTRL, oGUI_KEY_RCTRL,
	oGUI_KEY_LALT, oGUI_KEY_RALT,
	oGUI_KEY_LSHIFT, oGUI_KEY_RSHIFT,
	oGUI_KEY_LWIN, oGUI_KEY_RWIN,
	oGUI_KEY_APP_CYCLE, oGUI_KEY_APP_CONTEXT,

	// Toggle keys
	oGUI_KEY_CAPSLOCK,
	oGUI_KEY_SCROLLLOCK,
	oGUI_KEY_NUMLOCK,

	// Typing keys
	oGUI_KEY_SPACE, oGUI_KEY_BACKTICK, oGUI_KEY_DASH, oGUI_KEY_EQUAL, oGUI_KEY_LBRACKET, oGUI_KEY_RBRACKET, oGUI_KEY_BACKSLASH, oGUI_KEY_SEMICOLON, oGUI_KEY_APOSTROPHE, oGUI_KEY_COMMA, oGUI_KEY_PERIOD, oGUI_KEY_SLASH,
	oGUI_KEY_0, oGUI_KEY_1, oGUI_KEY_2, oGUI_KEY_3, oGUI_KEY_4, oGUI_KEY_5, oGUI_KEY_6, oGUI_KEY_7, oGUI_KEY_8, oGUI_KEY_9,
	oGUI_KEY_A, oGUI_KEY_B, oGUI_KEY_C, oGUI_KEY_D, oGUI_KEY_E, oGUI_KEY_F, oGUI_KEY_G, oGUI_KEY_H, oGUI_KEY_I, oGUI_KEY_J, oGUI_KEY_K, oGUI_KEY_L, oGUI_KEY_M, oGUI_KEY_N, oGUI_KEY_O, oGUI_KEY_P, oGUI_KEY_Q, oGUI_KEY_R, oGUI_KEY_S, oGUI_KEY_T, oGUI_KEY_U, oGUI_KEY_V, oGUI_KEY_W, oGUI_KEY_X, oGUI_KEY_Y, oGUI_KEY_Z,

	// Numpad keys
	oGUI_KEY_NUM0, oGUI_KEY_NUM1, oGUI_KEY_NUM2, oGUI_KEY_NUM3, oGUI_KEY_NUM4, oGUI_KEY_NUM5, oGUI_KEY_NUM6, oGUI_KEY_NUM7, oGUI_KEY_NUM8, oGUI_KEY_NUM9, 
	oGUI_KEY_NUMMUL, oGUI_KEY_NUMADD, oGUI_KEY_NUMSUB, oGUI_KEY_NUMDECIMAL, oGUI_KEY_NUMDIV, 

	// Typing control keys
	oGUI_KEY_ESC,
	oGUI_KEY_BACKSPACE,
	oGUI_KEY_TAB,
	oGUI_KEY_ENTER,
	oGUI_KEY_INS, oGUI_KEY_DEL,
	oGUI_KEY_HOME, oGUI_KEY_END,
	oGUI_KEY_PGUP, oGUI_KEY_PGDN,

	// System control keys
	oGUI_KEY_F1, oGUI_KEY_F2, oGUI_KEY_F3, oGUI_KEY_F4, oGUI_KEY_F5, oGUI_KEY_F6, oGUI_KEY_F7, oGUI_KEY_F8, oGUI_KEY_F9, oGUI_KEY_F10, oGUI_KEY_F11, oGUI_KEY_F12, oGUI_KEY_F13, oGUI_KEY_F14, oGUI_KEY_F15, oGUI_KEY_F16, oGUI_KEY_F17, oGUI_KEY_F18, oGUI_KEY_F19, oGUI_KEY_F20, oGUI_KEY_F21, oGUI_KEY_F22, oGUI_KEY_F23, oGUI_KEY_F24,
	oGUI_KEY_PAUSE,
	oGUI_KEY_SLEEP,
	oGUI_KEY_PRINTSCREEN,

	// Directional keys
	oGUI_KEY_LEFT, oGUI_KEY_UP, oGUI_KEY_RIGHT, oGUI_KEY_DOWN,

	oGUI_KEY_STANDARD_LAST = oGUI_KEY_DOWN,

	// Browser keys
	oGUI_KEY_MAIL, 
	oGUI_KEY_BACK, 
	oGUI_KEY_FORWARD, 
	oGUI_KEY_REFRESH, 
	oGUI_KEY_STOP, 
	oGUI_KEY_SEARCH, 
	oGUI_KEY_FAVS,

	// Media keys
	oGUI_KEY_MEDIA,
	oGUI_KEY_MUTE,
	oGUI_KEY_VOLUP,
	oGUI_KEY_VOLDN,
	oGUI_KEY_PREV_TRACK,
	oGUI_KEY_NEXT_TRACK,
	oGUI_KEY_STOP_TRACK,
	oGUI_KEY_PLAY_PAUSE_TRACK,

	// Misc keys
	oGUI_KEY_APP1, oGUI_KEY_APP2,

	oGUI_KEY_KEYBOARD_LAST = oGUI_KEY_APP2,

	// Touch
	oGUI_KEY_TOUCH1,
	oGUI_KEY_TOUCH_FIRST = oGUI_KEY_TOUCH1, oGUI_KEY_TOUCH2, oGUI_KEY_TOUCH3, oGUI_KEY_TOUCH4, oGUI_KEY_TOUCH5, oGUI_KEY_TOUCH6, oGUI_KEY_TOUCH7, oGUI_KEY_TOUCH8, oGUI_KEY_TOUCH9, oGUI_KEY_TOUCH10,
	oGUI_KEY_TOUCH_LAST = oGUI_KEY_TOUCH10,

	oGUI_KEY_COUNT,
};
oRTTI_ENUM_DECLARATION(oRTTI_CAPS_ARRAY, oGUI_KEY)

inline oGUI_INPUT_DEVICE_TYPE oGUIDeviceFromKey(oGUI_KEY _Key)
{
	if (oGUI_IN_RANGE(oGUI_KEY_KEYBOARD, _Key)) return oGUI_INPUT_DEVICE_KEYBOARD;
	if (oGUI_IN_RANGE(oGUI_KEY_MOUSE, _Key)) return oGUI_INPUT_DEVICE_MOUSE;
	if (oGUI_IN_RANGE(oGUI_KEY_JOYSTICK, _Key)) return oGUI_INPUT_DEVICE_JOYSTICK;
	if (oGUI_IN_RANGE(oGUI_KEY_TOUCH, _Key)) return oGUI_INPUT_DEVICE_TOUCH;
	return oGUI_INPUT_DEVICE_UNKNOWN;
}

// A standard key issues both an oGUI_ACTION_KEYDOWN and oGUI_ACTION_KEYUP 
// without special handling.
// Non-standard keys can behave poorly. For example on Windows they are hooked
// by the OS/driver to do something OS-specific and thus do not come through
// as key events, but rather as an app event that is singular, not a down/up.
inline bool oGUIIsStandardKey(oGUI_KEY _Key)
{
	return _Key >= oGUI_KEY_STANDARD_FIRST && _Key <= oGUI_KEY_STANDARD_LAST;
}

enum oGUI_BONE
{
	oGUI_BONE_HIP_CENTER,
	oGUI_BONE_SPINE,
	oGUI_BONE_SHOULDER_CENTER,
	oGUI_BONE_HEAD,
	oGUI_BONE_SHOULDER_LEFT,
	oGUI_BONE_ELBOW_LEFT,
	oGUI_BONE_WRIST_LEFT,
	oGUI_BONE_HAND_LEFT,
	oGUI_BONE_SHOULDER_RIGHT,
	oGUI_BONE_ELBOW_RIGHT,
	oGUI_BONE_WRIST_RIGHT,
	oGUI_BONE_HAND_RIGHT,
	oGUI_BONE_HIP_LEFT,
	oGUI_BONE_KNEE_LEFT,
	oGUI_BONE_ANKLE_LEFT,
	oGUI_BONE_FOOT_LEFT,
	oGUI_BONE_HIP_RIGHT,
	oGUI_BONE_KNEE_RIGHT,
	oGUI_BONE_ANKLE_RIGHT,
	oGUI_BONE_FOOT_RIGHT,
	
	oGUI_BONE_COUNT,
	oGUI_BONE_INVALID = oGUI_BONE_COUNT,
};
oRTTI_ENUM_DECLARATION(oRTTI_CAPS_ARRAY, oGUI_BONE)

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
	std::array<float4, oGUI_BONE_COUNT> Positions;
};

enum oGUI_WINDOW_STATE
{
	// A GUI window is a rectangular client area with an operating system-specific
	// border. It can exist in one of the several states listed below.
	
	// Window does not exist, or when specifying a new state, use this to indicate 
	// no-change.
	oGUI_WINDOW_INVALID,
	
	// Window is invisible.
	oGUI_WINDOW_HIDDEN,

	// Window is reduces to iconic or taskbar size. When setting this size
	// and position are ignored but style is preserved.
	oGUI_WINDOW_MINIMIZED,
	
	// Window is in normal sub-screen-size mode. When setting this state size,  
	// position and style are respected.
	oGUI_WINDOW_RESTORED,

	// Window takes up the entire screen but still has borders. When setting this 
	// state size and position are ignored but style is preserved.
	oGUI_WINDOW_MAXIMIZED, 

	// Window borders are removed and the client area fills the whole screen but 
	// does not do special direct-access or HW-syncing - it still behaves like a 
	// window thus enabling fast application switching or multi-full-screen
	// support.
	oGUI_WINDOW_FULLSCREEN,
	
	oGUI_WINDOW_STATE_COUNT,
};
oRTTI_ENUM_DECLARATION(oRTTI_CAPS_ARRAY, oGUI_WINDOW_STATE)

// Invalid, hidden and minimized windows are not visible
inline bool oGUIIsVisible(oGUI_WINDOW_STATE _State) { return _State >= oGUI_WINDOW_RESTORED; }

enum oGUI_WINDOW_STYLE
{
	// Use this value for "noop" or "previous" value.
	oGUI_WINDOW_DEFAULT,

	// There is no border around the client area and the window remains a top-
	// level window. This is the best for splash screens and fullscreen modes.
	oGUI_WINDOW_BORDERLESS,

	// There is a border but closing the window from the decoration is not allowed
	oGUI_WINDOW_DIALOG,

	// There is a border but no user resize is allowed
	oGUI_WINDOW_FIXED,

	// Same as fixed with a menu
	oGUI_WINDOW_FIXED_WITH_MENU,

	// Same as fixed with a status bar
	oGUI_WINDOW_FIXED_WITH_STATUSBAR,

	// Same as fixed with a menu and status bar
	oGUI_WINDOW_FIXED_WITH_MENU_AND_STATUSBAR,

	// There is a border and user can resize window
	oGUI_WINDOW_SIZABLE,

	// Same as sizable with a menu
	oGUI_WINDOW_SIZABLE_WITH_MENU,

	// Same as sizable with a status bar
	oGUI_WINDOW_SIZABLE_WITH_STATUSBAR,

	// Same as sizable with a menu and a status bar
	oGUI_WINDOW_SIZABLE_WITH_MENU_AND_STATUSBAR,

	oGUI_WINDOW_STYLE_COUNT,
};
oRTTI_ENUM_DECLARATION(oRTTI_CAPS_ARRAY, oGUI_WINDOW_STYLE)

inline bool oGUIStyleHasStatusBar(oGUI_WINDOW_STYLE _Style)
{
	switch (_Style)
	{
		case oGUI_WINDOW_FIXED_WITH_STATUSBAR:
		case oGUI_WINDOW_FIXED_WITH_MENU_AND_STATUSBAR:
		case oGUI_WINDOW_SIZABLE_WITH_STATUSBAR:
		case oGUI_WINDOW_SIZABLE_WITH_MENU_AND_STATUSBAR: return true;
		default: break;
	}
	return false;
}

inline bool oGUIStyleHasMenu(oGUI_WINDOW_STYLE _Style)
{
	switch (_Style)
	{
		case oGUI_WINDOW_FIXED_WITH_MENU:
		case oGUI_WINDOW_FIXED_WITH_MENU_AND_STATUSBAR:
		case oGUI_WINDOW_SIZABLE_WITH_MENU:
		case oGUI_WINDOW_SIZABLE_WITH_MENU_AND_STATUSBAR: return true;
		default: break;
	}
	return false;
}

enum oGUI_WINDOW_SORT_ORDER
{
	// normal/default behavior
	oGUI_WINDOW_SORTED,

	// "normal" always-on-top
	oGUI_WINDOW_ALWAYS_ON_TOP,

	// Meant for final shipping, this fights hard to ensure any Windows popups or
	// even the taskbar stays hidden.
	oGUI_WINDOW_ALWAYS_ON_TOP_WITH_FOCUS,

	oGUI_WINDOW_SORT_ORDER_COUNT,
};

enum oGUI_CURSOR_STATE
{
	// No cursor appears (hidden)
	oGUI_CURSOR_NONE,

	// Default OS arrow
	oGUI_CURSOR_ARROW,
	
	// A hand for translating/scrolling
	oGUI_CURSOR_HAND,

	// A question-mark-like icon
	oGUI_CURSOR_HELP,

	// Use this to indicate mouse interaction is not allowed
	oGUI_CURSOR_NOTALLOWED,

	// User input is blocked while the operation is occurring
	oGUI_CURSOR_WAIT_FOREGROUND,

	// A performance-degrading action is under way but user input is not blocked
	oGUI_CURSOR_WAIT_BACKGROUND,
	
	// A user-provided cursor is displayed
	oGUI_CURSOR_USER,

	oGUI_CURSOR_STATE_COUNT,
};
oRTTI_ENUM_DECLARATION(oRTTI_CAPS_ARRAY, oGUI_CURSOR_STATE)

enum oGUI_ALIGNMENT
{
	// Position is relative to parent's top-left corner
	oGUI_ALIGNMENT_TOP_LEFT,

	// Position is centered horizontally and vertically relative to the parent's 
	// top
	oGUI_ALIGNMENT_TOP_CENTER,

	// Position is relative to parent's top-right corner minus the child's width
	oGUI_ALIGNMENT_TOP_RIGHT,

	// Position is centered vertically and horizontally relative to parent's left 
	// corner
	oGUI_ALIGNMENT_MIDDLE_LEFT,
	
	// Position is relative to that which centers the child in the parent's bounds
	oGUI_ALIGNMENT_MIDDLE_CENTER,
	
	// Position is centered vertically and horizontally relative to parent's right 
	// corner minus the child's width
	oGUI_ALIGNMENT_MIDDLE_RIGHT,
	
	// Position is relative to parent's bottom-left corner
	oGUI_ALIGNMENT_BOTTOM_LEFT,

	// Position is centered horizontally and vertically relative to the parent's 
	// bottom minus the child's height
	oGUI_ALIGNMENT_BOTTOM_CENTER,
	
	// Position is relative to parent's bottom-right corner minus the child's width
	oGUI_ALIGNMENT_BOTTOM_RIGHT,
	
	// Child is sized and positioned to match parent: aspect ration is not 
	// respected. Alignment will be center-middle and respect any offset value.
	oGUI_ALIGNMENT_FIT_PARENT,

	// Retain aspect ratio and choose the child's largest dimension (width or 
	// height) and leave letterbox (vertical or horizontal) areas. Alignment will 
	// be center-middle and respect any offset value.
	oGUI_ALIGNMENT_FIT_LARGEST_AXIS,

	// Retain aspect ration and choose the child's smallest dimension (width or 
	// height) and crop any overflow. (cropping of the result rectangle is 
	// dependent on a separate cropping flag.
	oGUI_ALIGNMENT_FIT_SMALLEST_AXIS,

	oGUI_ALIGNMENT_COUNT,
};
oRTTI_ENUM_DECLARATION(oRTTI_CAPS_ARRAY, oGUI_ALIGNMENT)

enum oGUI_CONTROL_TYPE
{
	oGUI_CONTROL_UNKNOWN,
	oGUI_CONTROL_GROUPBOX,
	oGUI_CONTROL_BUTTON,
	oGUI_CONTROL_CHECKBOX,
	oGUI_CONTROL_RADIOBUTTON,
	oGUI_CONTROL_LABEL,
	oGUI_CONTROL_LABEL_CENTERED,
	oGUI_CONTROL_HYPERLABEL, // supports multiple markups of <a href="<somelink>">MyLink</a> or <a id="someID">MyID</a>. There can be multiple in one string.
	oGUI_CONTROL_LABEL_SELECTABLE,
	oGUI_CONTROL_ICON, // oGUI_CONTROL_DESC::Text should be the native handle to the icon resource (HICON on Windows)
	oGUI_CONTROL_TEXTBOX,
	oGUI_CONTROL_TEXTBOX_SCROLLABLE,
	oGUI_CONTROL_FLOATBOX, // textbox that only allows floating point (single-precision) numbers to be entered. Specify oDEFAULT for Size values to use icon's values.
	oGUI_CONTROL_FLOATBOX_SPINNER,
	oGUI_CONTROL_COMBOBOX, // Supports specifying contents all at once, delimited by '|'. i.e. "Text1|Text2|Text3"
	oGUI_CONTROL_COMBOTEXTBOX, // Supports specifying contents all at once, delimited by '|'. i.e. "Text1|Text2|Text3"
	oGUI_CONTROL_PROGRESSBAR,
	oGUI_CONTROL_PROGRESSBAR_UNKNOWN, // (marquee)
	oGUI_CONTROL_TAB, // Supports specifying contents all at once, delimited by '|'. i.e. "Text1|Text2|Text3"
	oGUI_CONTROL_SLIDER,
	oGUI_CONTROL_SLIDER_SELECTABLE, // displays a portion of the slider as selected
	oGUI_CONTROL_SLIDER_WITH_TICKS, // Same as Slider but tick marks can be added
	oGUI_CONTROL_TYPE_COUNT,
};
oRTTI_ENUM_DECLARATION(oRTTI_CAPS_ARRAY, oGUI_CONTROL_TYPE)

enum oGUI_BORDER_STYLE
{
	oGUI_BORDER_SUNKEN,
	oGUI_BORDER_FLAT,
	oGUI_BORDER_RAISED,
	oGUI_BORDER_STYLE_COUNT,
};
oRTTI_ENUM_DECLARATION(oRTTI_CAPS_ARRAY, oGUI_BORDER_STYLE)

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

enum oGUI_EVENT
{
	// An event is something that the window issues to client code. Events can be
	// triggered as a side-effect of other actions. Rarely does client code have
	// direct control over events. All events should be downcast to AsShape() 
	// unless described otherwise below.

	// Called when a timer is triggered. Use timers only for infrequent one-shot
	// actions. Do not use this mechanism for consistent updates such as ring
	// buffer monitors or for main loop execution or swap buffer presentation.
	// Use AsTimer() on the event.
	oGUI_TIMER,

	oGUI_ACTIVATED, // called just after a window comes to be in focus
	oGUI_DEACTIVATED, // called just after a window is no longer in focus

	// called before during window or control creation. This should throw an 
	// exception if there is a failure. It will be caught in the window's factory
	// function and made into an API-consistent error message or rethrown as 
	// appropriate.
	oGUI_CREATING, 
	
	// called when the window wants to redraw itself. NOTE: On Windows the user 
	// must call BeginPaint/EndPaint on the event's hSource as documented in 
	// WM_PAINT.
	oGUI_PAINT,
	oGUI_DISPLAY_CHANGED, // called when the desktop/screens change
	oGUI_MOVING, // called just before a window moves
	oGUI_MOVED, // called just after a window moves
	oGUI_SIZING, // called just before a window's client area changes size
	oGUI_SIZED, // called just after a window's client area changes size
	
	// called before the window is closed. Really this is where client code should 
	// decide whether or not to exit the window's message loop. oGUI_CLOSED is 
	// too late. All control of the message loop is in client code, so if there
	// is nothing done in oGUI_CLOSING, then the default behavior is to leave the 
	// window as-is, no closing is actually done.
	oGUI_CLOSING, 
	
	// called after a commitment to closing the window has been made.
	oGUI_CLOSED,
	
	// called when a request to become fullscreen is made. Hook can return false 
	// on failure.
	oGUI_TO_FULLSCREEN,
	
	// called when a request to go from fullscreen to windowed is made
	oGUI_FROM_FULLSCREEN,

	// Sent if a window had input capture, but something else in the system caused 
	// it to lose capture
	oGUI_LOST_CAPTURE, 
	
	// sent if a window gets files drag-dropped on it. Use AsDrop() on event.
	oGUI_DROP_FILES,

	// Sent when an input device's status has changed. Use AsInputDevice() on 
	// event.
	oGUI_INPUT_DEVICE_CHANGED,

	// The user can trigger a custom event with a user-specified sub-type. 
	// Use AsCustom() on event.
	oGUI_CUSTOM_EVENT,

	oGUI_EVENT_COUNT,
};
oRTTI_ENUM_DECLARATION(oRTTI_CAPS_ARRAY, oGUI_EVENT)

inline bool oGUIISShapeEvent(oGUI_EVENT _Event)
{
	switch (_Event)
	{
		case oGUI_CREATING: case oGUI_TIMER: case oGUI_DROP_FILES: 
		case oGUI_INPUT_DEVICE_CHANGED: case oGUI_CUSTOM_EVENT: return false;
		default: break;
	}
	return true;
}

enum oGUI_ACTION
{
	// Actions are similar to events, except client code or the user can trigger 
	// them explicitly and not as the artifact of some other action.

	oGUI_ACTION_UNKNOWN,
	oGUI_ACTION_MENU,
	oGUI_ACTION_CONTROL_ACTIVATED,
	oGUI_ACTION_CONTROL_DEACTIVATED,
	oGUI_ACTION_CONTROL_SELECTION_CHANGING,
	oGUI_ACTION_CONTROL_SELECTION_CHANGED,
	oGUI_ACTION_HOTKEY,
	oGUI_ACTION_KEY_DOWN,
	oGUI_ACTION_KEY_UP,
	oGUI_ACTION_POINTER_MOVE,
	oGUI_ACTION_SKELETON,
	oGUI_ACTION_SKELETON_ACQUIRED,
	oGUI_ACTION_SKELETON_LOST,

	oGUI_ACTION_COUNT,
};
oRTTI_ENUM_DECLARATION(oRTTI_CAPS_ARRAY, oGUI_ACTION)

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
		, State(oGUI_WINDOW_RESTORED)
		, Style(oGUI_WINDOW_FIXED)
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
		StatusWidths.fill(oInvalid);
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
	oGUI_WINDOW_STATE State;
	oGUI_WINDOW_STYLE Style;
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
		: State(oGUI_WINDOW_INVALID)
		, Style(oGUI_WINDOW_DEFAULT)
		, ClientPosition(oDEFAULT, oDEFAULT)
		, ClientSize(oDEFAULT, oDEFAULT)
	{}

	// The desired Shape of the window. Minimize and maximize will override/ignore  
	// ClientPosition and ClientSize values. Use oGUI_WINDOW_INVALID to indicate
	// that the state should remain untouched (only respect ClientPosition and 
	// ClientSize values).
	oGUI_WINDOW_STATE State;

	// The desired style of the non-client area of the window (the OS decoration)
	// Use oGUI_WINDOW_DEFAULT to indicate that the state should remain 
	// untouched. Changing style will not affect client size/position.
	oGUI_WINDOW_STYLE Style;

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
		: ClientState(oGUI_CURSOR_ARROW)
		, NonClientState(oGUI_CURSOR_ARROW)
		, HasCapture(false)
	{}

	oGUI_CURSOR_STATE ClientState; // state of cursor when in the client area
	oGUI_CURSOR_STATE NonClientState; // state of cursor when in the OS decoration area
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
	oGUI_EVENT_DESC(oGUI_WINDOW _hWindow, oGUI_EVENT _Type)
		: hWindow(_hWindow)
		, Type(_Type)
	{}

	// Native window handle
	oGUI_WINDOW hWindow;

	// Type of event. This is the base class; use the downcasting accessors below
	// based on this type value.
	oGUI_EVENT Type;

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
		, const oGUI_WINDOW_SHAPE_DESC& _Shape)
		: oGUI_EVENT_DESC(_hWindow, oGUI_CREATING)
		, hStatusBar(_hStatusBar)
		, hMenu(_hMenu)
		, Shape(_Shape)
	{}

	// Native handle of the window's status bar.
	oGUI_STATUSBAR hStatusBar;

	// Native handle of the top-level window's menu.
	oGUI_MENU hMenu;
	oGUI_WINDOW_SHAPE_DESC Shape;
};

struct oGUI_EVENT_SHAPE_DESC : oGUI_EVENT_DESC
{
	oGUI_EVENT_SHAPE_DESC(oGUI_WINDOW _hWindow
		, oGUI_EVENT _Type
		, const oGUI_WINDOW_SHAPE_DESC& _Shape)
		: oGUI_EVENT_DESC(_hWindow, _Type)
		, Shape(_Shape)
	{}

	oGUI_WINDOW_SHAPE_DESC Shape;
};

struct oGUI_EVENT_TIMER_DESC : oGUI_EVENT_DESC
{
	oGUI_EVENT_TIMER_DESC(oGUI_WINDOW _hWindow, uintptr_t _Context)
		: oGUI_EVENT_DESC(_hWindow, oGUI_TIMER)
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
		: oGUI_EVENT_DESC(_hWindow, oGUI_DROP_FILES)
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
		, oGUI_INPUT_DEVICE_TYPE _Type
		, oGUI_INPUT_DEVICE_STATUS _Status
		, const char* _InstanceName)
		: oGUI_EVENT_DESC(_hWindow, oGUI_INPUT_DEVICE_CHANGED)
		, Type(_Type)
		, Status(_Status)
		, InstanceName(_InstanceName)
	{}

	oGUI_INPUT_DEVICE_TYPE Type;
	oGUI_INPUT_DEVICE_STATUS Status;
	const char* InstanceName;
};

struct oGUI_EVENT_CUSTOM_DESC : oGUI_EVENT_DESC
{
	oGUI_EVENT_CUSTOM_DESC(oGUI_WINDOW _hWindow, int _EventCode, uintptr_t _Context)
		: oGUI_EVENT_DESC(_hWindow, oGUI_CUSTOM_EVENT)
		, EventCode(_EventCode)
		, Context(_Context)
	{}

	int EventCode;
	uintptr_t Context;
};

const oGUI_EVENT_CREATE_DESC& oGUI_EVENT_DESC::AsCreate() const { oASSERT(Type == oGUI_CREATING, "wrong type"); return *static_cast<const oGUI_EVENT_CREATE_DESC*>(this); }
const oGUI_EVENT_SHAPE_DESC& oGUI_EVENT_DESC::AsShape() const { oASSERT(oGUIISShapeEvent(Type), "wrong type"); return *static_cast<const oGUI_EVENT_SHAPE_DESC*>(this); }
const oGUI_EVENT_TIMER_DESC& oGUI_EVENT_DESC::AsTimer() const { oASSERT(Type == oGUI_TIMER, "wrong type"); return *static_cast<const oGUI_EVENT_TIMER_DESC*>(this); }
const oGUI_EVENT_DROP_DESC& oGUI_EVENT_DESC::AsDrop() const { oASSERT(Type == oGUI_DROP_FILES, "wrong type"); return *static_cast<const oGUI_EVENT_DROP_DESC*>(this); }
const oGUI_EVENT_INPUT_DEVICE_DESC& oGUI_EVENT_DESC::AsInputDevice() const { oASSERT(Type == oGUI_INPUT_DEVICE_CHANGED, "wrong type"); return *static_cast<const oGUI_EVENT_INPUT_DEVICE_DESC*>(this); }
const oGUI_EVENT_CUSTOM_DESC& oGUI_EVENT_DESC::AsCustom() const { oASSERT(Type == oGUI_CUSTOM_EVENT, "wrong type"); return *static_cast<const oGUI_EVENT_CUSTOM_DESC*>(this); }

typedef oFUNCTION<void(const oGUI_EVENT_DESC& _Event)> oGUI_EVENT_HOOK;

struct oGUI_ACTION_DESC
{
	// All actions use this desc. This can be filled out manually and submitted
	// to an action handler to spoof hardware events, for example from a network
	// stream thus enabling remote access.

	oGUI_ACTION_DESC()
		: hWindow(nullptr)
		, Action(oGUI_ACTION_UNKNOWN)
		, DeviceType(oGUI_INPUT_DEVICE_UNKNOWN)
		, DeviceID(oInvalid)
		, Position(0.0f)
		//, Key(oGUI_KEY_NONE)
		//, ActionCode(0)
	{ hSkeleton = nullptr; }

	oGUI_ACTION_DESC(
		oGUI_WINDOW _hWindow
		, unsigned int _TimestampMS
		, oGUI_ACTION _Action
		, oGUI_INPUT_DEVICE_TYPE _DeviceType
		, int _DeviceID)
			: hWindow(_hWindow)
			, TimestampMS(_TimestampMS)
			, Action(_Action)
			, DeviceType(_DeviceType)
			, DeviceID(_DeviceID)
			, Position(0.0f)
			//, Key(oGUI_KEY_NONE)
			//, ActionCode(0)
	{ hSkeleton = nullptr; }

	// _____________________________________________________________________________
	// Common across all actions

	// Control devices have their own handle and sometimes their own sub-action.
	oGUI_WINDOW hWindow;

	// Time at which the message was sent in milliseconds.
	unsigned int TimestampMS;

	oGUI_ACTION Action;
	oGUI_INPUT_DEVICE_TYPE DeviceType;

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
			oGUI_KEY Key;

			// For some specific types of controls, this is an additional action value.
			int ActionCode;
		};

		// if Action is an oGUI_ACTION_SKELETON, the hSkeleton is  a handle fit for 
		// use with a platform-specific accessor to the actual skeleton data.
		oGUI_SKELETON hSkeleton;
	};
};

typedef oFUNCTION<void(const oGUI_ACTION_DESC& _Action)> oGUI_ACTION_HOOK;

// So that hotkeys can be statically defined without a "non-aggregates cannot be 
// initialized" warning
struct oGUI_HOTKEY_DESC_NO_CTOR
{
	oGUI_KEY HotKey;
	unsigned short ID;
	bool AltDown;
	bool CtrlDown;
	bool ShiftDown;
};

struct oGUI_HOTKEY_DESC : oGUI_HOTKEY_DESC_NO_CTOR
{
	oGUI_HOTKEY_DESC()
	{
		HotKey = oGUI_KEY_NONE;
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
		, Type(oGUI_CONTROL_UNKNOWN)
		, Text("")
		, Position(oInvalid, oInvalid)
		, Size(oDEFAULT, oDEFAULT)
		, ID(oInvalid)
		, StartsNewGroup(false)
	{}

	// All controls must have a parent
	oGUI_WINDOW hParent;

	// If nullptr is specified, then the system default font will be used.
	oGUI_FONT hFont;

	// Type of control to create
	oGUI_CONTROL_TYPE Type;
	
	// Any item that contains a list of strings can set this to be
	// a '|'-terminated set of strings to immediately populate all
	// items. ("Item1|Item2|Item3"). This is valid for:
	// COMBOBOX, COMBOTEXTBOX, TAB
	const char* Text;
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
		, Alignment(oGUI_ALIGNMENT_TOP_LEFT)
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
	oGUI_ALIGNMENT Alignment;
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
void oGUIRecordInputState(const oGUI_ACTION_DESC& _Action, const oGUI_KEY* _pKeys, size_t _NumKeys, bool* _pKeyStates, size_t _NumKeyStates, float3* _pPointerPosition);
template<size_t NumKeys, size_t NumKeyStates> void oGUIRecordInputState(const oGUI_ACTION_DESC& _Action, const oGUI_KEY (&_pKeys)[NumKeys], bool (&_pKeyStates)[NumKeyStates], float3* _pPointerPosition) { oGUIRecordInputState(_Action, _pKeys, NumKeys, _pKeyStates, NumKeyStates, _pPointerPosition); }

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
oRECT oGUIResolveRect(const oRECT& _Parent, const oRECT& _UnadjustedChild, oGUI_ALIGNMENT _Alignment, bool _Clip);
inline oRECT oGUIResolveRect(const oRECT& _Parent, const int2& _UnadjustedChildPosition, const int2& _UnadjustedChildSize, oGUI_ALIGNMENT _Alignment, bool _Clip) { oRECT r; r.Min = oGUIResolveRectPosition(_UnadjustedChildPosition); r.Max = r.Min + _UnadjustedChildSize; return oGUIResolveRect(_Parent, r, _Alignment, _Clip); }

#endif
