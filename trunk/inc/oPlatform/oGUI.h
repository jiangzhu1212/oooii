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
// Common header for Graphical User Interface (GUI) concepts
#pragma once
#ifndef oGUI_h
#define oGUI_h

#include <oPlatform/oStddef.h> // oDEFAULT
#include <oPlatform/oX11KeyboardSymbols.h>

enum oGUI_WINDOW_STATE
{
	oGUI_WINDOW_NONEXISTANT, // Window does not exist
	oGUI_WINDOW_HIDDEN, // Window is invisible, but exists
	oGUI_WINDOW_MINIMIZED, // Window is reduces to iconic or taskbar size
	oGUI_WINDOW_RESTORED, // Window is in normal sub-screen-size mode
	oGUI_WINDOW_MAXIMIZED, // Window takes up the entire screen
	oGUI_WINDOW_FULLSCREEN, // Window takes exclusive access to screen, and will not have a title bar, border, etc regardless of its style, position or other parameters
};

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
	oGUI_CONTROL_LABEL_HYPERLINK, // supports multiple markups of <a href="<somelink>">MyLink</a> or <a id="someID">MyID</a>. There can be multiple in one string.
	oGUI_CONTROL_LABEL_SELECTABLE,
	oGUI_CONTROL_ICON, // oGUI_CONTROL_DESC::Text should be the native handle to the icon resource (HICON on Windows)
	oGUI_CONTROL_TEXTBOX,
	oGUI_CONTROL_FLOATBOX, // textbox that only allows floating point (single-precision) numbers to be entered. Specify oDEFAULT for Size values to use icon's values.
	oGUI_CONTROL_FLOATBOX_SPINNER,
	oGUI_CONTROL_COMBOBOX, // Supports specifying contents all at once, delimited by '|'. i.e. "Text1|Text2|Text3"
	oGUI_CONTROL_COMBOTEXTBOX, // Supports specifying contents all at once, delimited by '|'. i.e. "Text1|Text2|Text3"
	oGUI_CONTROL_PROGRESSBAR,
	oGUI_CONTROL_TAB, // Supports specifying contents all at once, delimited by '|'. i.e. "Text1|Text2|Text3"

	oGUI_NUM_CONTROLS,
};

enum oGUI_BORDER_STYLE
{
	oGUI_BORDER_SUNKEN,
	oGUI_BORDER_FLAT,
	oGUI_BORDER_RAISED,
};

oAPI const char* oAsString(const oGUI_WINDOW_STATE& _State);
oAPI const char* oAsString(const oGUI_WINDOW_STYLE& _Style);
oAPI const char* oAsString(const oGUI_CURSOR_STATE& _State);
oAPI const char* oAsString(const oGUI_ALIGNMENT& _Alignment);
oAPI const char* oAsString(const oGUI_CONTROL_TYPE& _Type);
oAPI const char* oAsString(const oGUI_BORDER_STYLE& _Style);

// Abstractions for the native platform handles
oDECLARE_HANDLE(oGUI_WINDOW);
oDECLARE_HANDLE(oGUI_CONTROL);
oDECLARE_HANDLE(oGUI_FONT);

struct oGUI_WINDOW_DESC
{
	oGUI_WINDOW_DESC()
		: hParent(nullptr)
		, ClientPosition(oDEFAULT, oDEFAULT)
		, ClientSize(oDEFAULT, oDEFAULT)
		, State(oGUI_WINDOW_RESTORED)
		, Style(oGUI_WINDOW_FIXED)
		, Debug(false)
		, Enabled(true)
		, HasFocus(true)
		, AlwaysOnTop(false)
		, DefaultEraseBackground(true)
		, AllowAltF4(true)
		, AllowAltEnter(true)
		, AllowTouch(true)
		, ShowMenu(false)
		, ShowStatusBar(false)
	{}

	oGUI_WINDOW hParent;
	int2 ClientPosition;
	int2 ClientSize;
	oGUI_WINDOW_STATE State;
	oGUI_WINDOW_STYLE Style;
	bool Debug;
	bool Enabled;
	bool HasFocus;
	bool AlwaysOnTop;
	bool DefaultEraseBackground;
	bool AllowAltF1; // toggles visibility of mouse cursor (nonstandard/Ouroboros-unique idea)
	bool AllowAltF4; // closes window
	bool AllowAltEnter; // toggles fullscreen
	bool AllowTouch;
	bool ShowMenu;
	bool ShowStatusBar;
};

struct oGUI_HOTKEY_DESC
{
//	oGUI_HOTKEY_DESC()
//	: HotKey(oKB_VoidSymbol)
//	, ID(0)
//	, AltDown(false)
//	, CtrlDown(false)
//	, ShiftDown(false)
//	{}

	oKEYBOARD_KEY HotKey;
	unsigned short ID;
	bool AltDown;
	bool CtrlDown;
	bool ShiftDown;
};

struct oGUI_CONTROL_DESC
{
	oGUI_CONTROL_DESC()
		: hParent(nullptr)
		, hFont(nullptr)
		, Type(oGUI_CONTROL_UNKNOWN)
		, Text("")
		, Position(oInvalid, oInvalid)
		, Size(oInvalid, oInvalid)
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
	{}

	oStringS FontName;
	float PointSize;
	bool Bold;
	bool Italic;
	bool Underline;
	bool StrikeOut;
};

#endif
