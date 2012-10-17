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
// Common header for Graphical User Interface (GUI) concepts. This should be 
// used as the basis of a cross-platform interface and conversion functions from 
// these generic concepts to platform-specific concepts.
#pragma once
#ifndef oGUI_h
#define oGUI_h

#include <oBasis/oStddef.h>
#include <oBasis/oX11KeyboardSymbols.h>

enum oGUI_WINDOW_STATE
{
	oGUI_WINDOW_NONEXISTANT, // Window does not exist
	oGUI_WINDOW_HIDDEN, // Window is invisible, but exists
	oGUI_WINDOW_MINIMIZED, // Window is reduces to iconic or taskbar size
	oGUI_WINDOW_RESTORED, // Window is in normal sub-screen-size mode
	oGUI_WINDOW_MAXIMIZED, // Window takes up the entire screen
	oGUI_WINDOW_FULLSCREEN_COOPERATIVE, // Window decoration is removed and the client fills the whole screen, but does not do special direct-access or HW-syncing - it still behaves like a window
	oGUI_WINDOW_FULLSCREEN_EXCLUSIVE, // Window takes exclusive access to screen, and will not have a title bar, border, etc regardless of its style, position or other parameters
};

// Invalid, hidden and minimized windows are not visible
inline bool oGUIIsVisible(oGUI_WINDOW_STATE _State) { return _State >= oGUI_WINDOW_RESTORED; }

inline bool oGUIIsFullscreen(oGUI_WINDOW_STATE _State) { return _State == oGUI_WINDOW_FULLSCREEN_EXCLUSIVE || _State == oGUI_WINDOW_FULLSCREEN_COOPERATIVE; }

enum oGUI_WINDOW_STYLE
{
	oGUI_WINDOW_EMBEDDED, // There is no OS decoration of the client area and the window is readied to be a child
	oGUI_WINDOW_BORDERLESS, // There is no OS decoration of the client area
	oGUI_WINDOW_FIXED, // There is OS decoration but no user resize is allowed
	oGUI_WINDOW_DIALOG, // There is OS decoration, but closing the window from the decoration is not allowed
	oGUI_WINDOW_SIZEABLE, // OS decoration and user can resize window
};

enum oGUI_CURSOR_STATE
{
	oGUI_CURSOR_NONE, // No cursor appears (hidden)
	oGUI_CURSOR_ARROW, // Default OS arrow
	oGUI_CURSOR_HAND, // A hand for translating/scrolling
	oGUI_CURSOR_HELP, // A question-mark-like icon
	oGUI_CURSOR_NOTALLOWED, // Use this to indicate mouse interaction is not allowed
	oGUI_CURSOR_WAIT_FOREGROUND, // User input is blocked while the operation is occurring
	oGUI_CURSOR_WAIT_BACKGROUND, // A performance-degrading operation is under way, but it doesn't block user input
	oGUI_CURSOR_USER, // A user-provided cursor is displayed
};

enum oGUI_ALIGNMENT
{
	oGUI_ALIGNMENT_TOP_LEFT, // Position is relative to parent's top-left corner
	oGUI_ALIGNMENT_TOP_CENTER, // Position is centered horizontally and vertically relative to the parent's top
	oGUI_ALIGNMENT_TOP_RIGHT,  // Position is relative to parent's top-right corner minus the child's width
	oGUI_ALIGNMENT_MIDDLE_LEFT, // Position is centered vertically and horizontally relative to parent's left corner
	oGUI_ALIGNMENT_MIDDLE_CENTER, // Position is relative to that which centers the child in the parent's bounds
	oGUI_ALIGNMENT_MIDDLE_RIGHT, // Position is centered vertically and horizontally relative to parent's right corner minus the child's width
	oGUI_ALIGNMENT_BOTTOM_LEFT, // Position is relative to parent's bottom-left corner
	oGUI_ALIGNMENT_BOTTOM_CENTER, // Position is centered horizontally and vertically relative to the parent's bottom minus the child's height
	oGUI_ALIGNMENT_BOTTOM_RIGHT, // Position is relative to parent's bottom-right corner minus the child's width
	oGUI_ALIGNMENT_FIT_PARENT, // Child is sized and positioned to match parent
};

enum oGUI_CONTROL_TYPE
{
	oGUI_CONTROL_UNKNOWN,
	oGUI_CONTROL_GROUPBOX,
	oGUI_CONTROL_BUTTON,
	oGUI_CONTROL_CHECKBOX,
	oGUI_CONTROL_RADIOBUTTON,
	oGUI_CONTROL_LABEL,
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
	oGUI_NUM_CONTROLS,
};

enum oGUI_BORDER_STYLE
{
	oGUI_BORDER_SUNKEN,
	oGUI_BORDER_FLAT,
	oGUI_BORDER_RAISED,
};

enum oGUI_EVENT
{
	oGUI_IDLE, // called in message pump loop when window-related messages aren't being processed.
	oGUI_ACTIVATED, // called just after a window comes to be in focus
	oGUI_DEACTIVATED, // called just after a window is no longer in focus
	oGUI_CREATING, // called before during window or control creation. This should return true if successful, or false if there's a failure
	oGUI_PAINT, // called when the window wants to redraw itself. NOTE: On Windows the user must call BeginPaint/EndPaint on the event's hSource as documented in WM_PAINT.
	oGUI_DISPLAY_CHANGED, // called when the desktop/screens change
	oGUI_SETCURSOR, // called when the cursor should be set differently
	oGUI_MOVING, // called just before a window moves
	oGUI_MOVED, // called just after a window moves
	oGUI_SIZING, // called just before a window's client area changes size
	oGUI_SIZED, // called just after a window's client area changes size
	oGUI_CLOSING, // called if an abortable attempt was made to close the window. Hook can return false if close is not allowed
	oGUI_CLOSED, // called after a commitment to closing the window has been made.
	oGUI_TO_FULLSCREEN, // called when a request to become fullscreen is made. Hook can return false on failure
	oGUI_FROM_FULLSCREEN, // called when a request to go from fullscreen to windowed is made
	oGUI_LOST_CAPTURE, // Sent if a window had input capture, but something else in the system caused it to lose capture
};

enum oGUI_ACTION
{
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
};

oAPI const char* oAsString(oGUI_WINDOW_STATE _State);
oAPI const char* oAsString(oGUI_WINDOW_STYLE _Style);
oAPI const char* oAsString(oGUI_CURSOR_STATE _State);
oAPI const char* oAsString(oGUI_ALIGNMENT _Alignment);
oAPI const char* oAsString(oGUI_CONTROL_TYPE _Type);
oAPI const char* oAsString(oGUI_BORDER_STYLE _Style);
oAPI const char* oAsString(oGUI_EVENT _Event);
oAPI const char* oAsString(oGUI_ACTION _Action);

// Abstractions for the native platform handles
oDECLARE_HANDLE(oGUI_WINDOW);
oDECLARE_HANDLE(oGUI_MENU);
oDECLARE_DERIVED_HANDLE(oGUI_WINDOW, oGUI_STATUSBAR);
oDECLARE_DERIVED_HANDLE(oGUI_WINDOW, oGUI_CONTROL);
oDECLARE_HANDLE(oGUI_FONT);

struct oGUI_WINDOW_DESC
{
	oGUI_WINDOW_DESC()
		: hParent(nullptr)
		, hOwner(nullptr)
		, ClientPosition(oDEFAULT, oDEFAULT)
		, ClientSize(oDEFAULT, oDEFAULT)
		, State(oGUI_WINDOW_RESTORED)
		, Style(oGUI_WINDOW_FIXED)
		, CursorState(oGUI_CURSOR_ARROW)
		, Debug(false)
		, Enabled(true)
		, HasFocus(true)
		, AlwaysOnTop(false)
		, DefaultEraseBackground(true)
		, AllowAltF4(true)
		, AllowAltEnter(true)
		, AllowTouch(true)
		, EnableIdleEvent(true)
		, ShowMenu(false)
		, ShowStatusBar(false)
		, HasCapture(false)
		, ShowCursorOutsideClientArea(true)
	{
		oINIT_ARRAY(StatusWidths, oInvalid);
	}

	// Specifying a parent makes this window a child window, meaning it becomes a 
	// control (see oGUI_CONTROL code below). This means top-level decoration such
	// as a border or min/max/restore controls will not be shown.
	oGUI_WINDOW hParent;

	// Owner is the relationship of this window as a (modal) dialog box to that 
	// which it has disabled. By specifying an owner the relationship will be 
	// preserved.
	oGUI_WINDOW hOwner;

	int2 ClientPosition;
	int2 ClientSize;
	oGUI_WINDOW_STATE State;
	oGUI_WINDOW_STYLE Style;
	oGUI_CURSOR_STATE CursorState;
	short StatusWidths[8];
	bool Debug;
	bool Enabled;
	bool HasFocus;
	bool AlwaysOnTop;
	bool DefaultEraseBackground;
	bool AllowAltF1; // toggles visibility of mouse cursor (nonstandard/Ouroboros-unique idea)
	bool AllowAltF4; // closes window
	bool AllowAltEnter; // toggles fullscreen
	bool AllowTouch;
	bool EnableIdleEvent; // if false the message thread will sleep if no more messages are available. If true, oGUI_IDLE will fire each time there are not messages
	bool ShowMenu;
	bool ShowStatusBar;
	bool HasCapture; // forces messages to the window even if the focus/cursor is outside the window. Setting this to true will also bring the window to the foreground.
	bool ShowCursorOutsideClientArea; // if true, a cursor state that is not visible will be made visible if it leaves the client area
};

struct oGUI_EVENT_DESC
{
	oGUI_EVENT_DESC()
		: hSource(nullptr)
		, hStatusBar(nullptr)
		, hMenu(nullptr)
		, pWindow(nullptr)
		, Event(oGUI_CREATING)
		, ClientPosition(oDEFAULT, oDEFAULT)
		, ClientSize(oDEFAULT, oDEFAULT)
		, ScreenSize(oDEFAULT, oDEFAULT)
	{}

	oGUI_WINDOW hSource;
	oGUI_STATUSBAR hStatusBar;
	oGUI_MENU hMenu;

	// This is nullptr during oWM_CREATING because the creation isn't finished 
	// yet, however at that time hSource, hStatusBar and hMenu are valid as far as 
	// the native creating event allows (WM_CREATE on Windows).
	threadsafe interface oWindow* pWindow;

	oGUI_EVENT Event;
	int2 ClientPosition; // Position of the hSource's client area
	int2 ClientSize; // Size of the hSource's client area
	int2 ScreenSize; // for oGUI_TO_FULLSCREEN or oGUI_DISPLAY_CHANGE
	oGUI_WINDOW_STATE State;
};

typedef oFUNCTION<bool(const oGUI_EVENT_DESC& _Event)> oGUI_EVENT_HOOK;

struct oGUI_ACTION_DESC
{
	oGUI_ACTION_DESC()
		: hSource(nullptr)
		, Action(oGUI_ACTION_UNKNOWN)
		, SourceID(oInvalid)
		, ActionCode(0)
		, PointerPosition(0.0f)
	{}

	oGUI_WINDOW hSource;
	oGUI_ACTION Action;
	union
	{
		int SourceID;
		oKEYBOARD_KEY Key;
	};

	int ActionCode; // subtype of action
	float3 PointerPosition;
};

typedef oFUNCTION<void(const oGUI_ACTION_DESC& _Action)> oGUI_ACTION_HOOK;

// So that hotkeys can be statically defined without a non-aggregates cannot be initialized warning
struct oGUI_HOTKEY_DESC_NO_CTOR
{
	oKEYBOARD_KEY HotKey;
	unsigned short ID;
	bool AltDown;
	bool CtrlDown;
	bool ShiftDown;
};

struct oGUI_HOTKEY_DESC : oGUI_HOTKEY_DESC_NO_CTOR
{
	oGUI_HOTKEY_DESC()
	{
		HotKey = oKB_VoidSymbol;
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

// @oooii-tony: Design choice: Should "shadow" be part of the font, or part of 
// the per-draw text desc?

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

	oStringS FontName;
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
		, Foreground(std::White)
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
	oColor Foreground;
	oColor Background;
	oColor Shadow;
	int2 ShadowOffset;
	bool SingleLine;
};

// A utility function to analyze the specified action and compare it for keydown
// and keyup events from keys specified in the keys array. If there's a match,
// the corresponding keystate is marked as true if down, and false if up. NOTE:
// _NumKeys can be a multiple of _NumKeyStates to support multiple key bindings
// for the same KeyState. i.e. if you have two states LEFT and RIGHT and you 
// want to bind A and D, but also left-arrow and right-arrow, that can be done
// with an array of [A,D,left,right] and the Keystates will be written correctly
// for LEFT and RIGHT. If _ForceLowerCase is true, oKB_A and oKB_a will be 
// treated as oKB_a. This is true for A-Z,a-z. If the action is a pointer move,
// then the position is recorded to _pPointerPosition.
void oGUIRecordInputState(const oGUI_ACTION_DESC& _Action, const oKEYBOARD_KEY* _pKeys, size_t _NumKeys, bool* _pKeyStates, size_t _NumKeyStates, float3* _pPointerPosition, bool _ForceLowerCase = true);
template<size_t NumKeys, size_t NumKeyStates> void oGUIRecordInputState(const oGUI_ACTION_DESC& _Action, const oKEYBOARD_KEY (&_pKeys)[NumKeys], bool (&_pKeyStates)[NumKeyStates], float3* _pPointerPosition, bool _ForceLowerCase = true) { oGUIRecordInputState(_Action, _pKeys, NumKeys, _pKeyStates, NumKeyStates, _pPointerPosition, _ForceLowerCase); }
#endif
