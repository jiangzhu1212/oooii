/**************************************************************************
 * The MIT License                                                        *
 * Copyright (c) 2013 OOOii.                                              *
 * antony.arciuolo@oooii.com                                              *
 * kevin.myers@oooii.com                                                  *
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
// Utilities for working with HWND objects. It's amazing how inconsistent much 
// of the Windows API is, so prefer using the more consistent, centralized API 
// here over native calls, a seemingly simply wrappers might call several 
// carefully-ordered win32 API.

// Windows GUI handling is single-threaded. All multi-threaded calls can be 
// piped to the GUI thread for processing, however because of more complex usage
// build on top of this API, such Windows-native API are not used and most calls
// here are required to be on the window's messaging thread. Such functions will
// be marked with:
// WT: Window Thread: This should only be called from the same thread on which 
//     the window was created.
// IA: Instantaneous Accessor: This can be called from any thread, however it 
//     may not reflect any state queued up for processing, thus the state could
//     have changed by the time client code uses the result.
// FT: Free-Threaded: The API can be called from any thread at any time.

#pragma once
#ifndef oWinWindowing_h
#define oWinWindowing_h

#include <oStd/oStdThread.h>
#include <oBasis/oGUI.h>
#include <oPlatform/Windows/oWindows.h>

// _____________________________________________________________________________
// Custom messages

enum oWM
{
	// Custom Windows messages defined for Ouroboros' oWindow and controls.

	oWM_FIRST_MESSAGE = WM_APP,

	/**
		Called when all other messages have been handled and then only to windows
		that register themselves as "active" windows. If there are no active windows
		and the message pump is empty, the pump thread will block waiting for new
		messages.
		wParam
			This parameter is not used.
		lParam
			This parameter is not used.
		lResult
			Returns 0.
	*/
	oWM_MAINLOOP = oWM_FIRST_MESSAGE,

	/**
		Execute an arbitrary std::function on a window message pump thread.
		wParam
			This parameter is not used.
		lParam
			oTASK* pointer to a task that will have delete called on it once 
			execution is done.
		lResult
			Returns 0.
	*/
	oWM_DISPATCH,

	/**
		Calls SetTimer on the window thread with a null callback (the message pump
		should handle WM_TIMER and return status).
		wParam
			This is passed to the nIDEvent parameter of SetTimer.
		lParam
			This is passed to the uElapse parameter of SetTimer.
		lResult
			Returns 0.
	*/
	oWM_START_TIMER,

	/**
		Adds an action observer/hook function to an oWindow.
		wParam
			This parameter is not used.
		lParam
			An oGUI_ACTION_HOOK* that the oWindow will add to its observer list.
		lResult
			A hook ID (int) that can later be passed to oWM_ACTION_UNHOOK to remove 
			the observer.
	*/
	oWM_ACTION_HOOK,

	/**
		Removes an action observer/hook function from an oWindow.
		wParam
			This parameter is not used.
		lParam
			A hook ID (int) to be removed. The ID would have been returned from a
			oWM_ACTION_HOOK send.
		lResult
			Returns true if successful, or false if the ID was not found.
	*/
	oWM_ACTION_UNHOOK,

	/**
		Sent when oWindow::Trigger is called to trigger an action from code.
		wParam
			This parameter is not used.
		lParam
			An oGUI_ACTION_DESC* describing the action.
		lResult
			Returns 0.
	*/
	oWM_ACTION_TRIGGER,

	/**
		Adds an event observer/hook function to an oWindow.
		wParam
			This parameter is not used.
		lParam
			An oGUI_EVENT_HOOK* that the oWindow will add to its observer list.
		lResult
			A hook ID (int) that can later be passed to oWM_EVENT_UNHOOK to remove the 
			observer.
	*/
	oWM_EVENT_HOOK,

	/**
		Removes an event observer/hook function from an oWindow.
		wParam
			This parameter is not used.
		lParam
			A hook ID (int) to be removed. The ID would have been returned from a
			oWM_EVENT_HOOK send.
		lResult
			Returns true if successful, or false if the ID was not found.
	*/
	oWM_EVENT_UNHOOK,

	/**
		Defines an HACCEL for an oWindow through the use of oGUI_HOTKEY_DESC_NO_CTOR
		interfaces.
		wParam
			The number of hotkeys specified in the lParam array.
		lParam
			oGUI_HOTKEY_DESC_NO_CTOR* array of hotkeys to set.
		lResult
			Returns true if successful or false if there was a failure.
	*/
	oWM_SETHOTKEYS,

	/**
		Copy an oWindow's current HACCEL into oGUI_HOTKEY_DESC_NO_CTOR values.
		wParam
			The maximum number of oGUI_HOTKEY_DESC_NO_CTORs to be copied.
		lParam
			A pointer to the buffer that is to receive the oGUI_HOTKEY_DESC_NO_CTORs.
		lResult
			If lParam is nullptr, the return value is the number of hotkeys currently
			set as an HACCEL in the specified oWindow. If lParam is a valid pointer, 
			then the return value is the actual number of hotkeys copied.
	*/
	oWM_GETHOTKEYS,

	/**
		Set the text on an oWindow's status bar section.
		wParam
			The number of (int) widths for each section of an oWindow's status bar.
		lParam
			int* array of widths to set
		lResult
			Returns true if successful or false if there was a failure.
	*/
	oWM_STATUS_SETPARTS,

	/**
		Set the text on an oWindow's status bar section.
		wParam
			The index of the status bar's section on which the text will be set.
		lParam
			A pointer to a null-terminated string that is the status bar section text.
		lResult
			The return value is true if the text was set or false if the index is not
			valid.
	*/
	oWM_STATUS_SETTEXT,

	/**
		Get the text from an oWindow's status bar section.
		wParam
			The result of MAKEWPARAM(x,y) where x is the status bar's section from 
			which text will be retrieved and y is the maximum number of characters to
			be copied, including the terminating null character.
		lParam
			A pointer to the buffer that is to receive the text.
		lResult
			The return value is the number of characters copied, not including the 
			terminating null character.
	*/
	oWM_STATUS_GETTEXT,

	/**
		Determines the length, in characters, of the text associated with an 
		oWindow's status bar section.
		wParam
			The index of the status bar's section on which the text length will be
			determined.
		lParam
			This parameter is not used.
		lResult
			The return value is the length of the text in characters, not including 
			the terminating null character.
	*/
	oWM_STATUS_GETTEXTLENGTH,

	/**
		Received when an input device has updated skeleton data. Use of this message 
		is an attempt to model Kinect/Skeleton data (i.e. low-level gesture) on
		WM_TOUCH, with a GetHandle/ReleaseHandle to iterate through the number of 
		bones in the skeleton data.
		wParam
			The low-order word of wParam specifies the ID of the skeleton.
			The high-order word of wParam is zero.
		lParam
			Contains a skeleton input handle that can be used in a call to 
			GetSkeletonInputInfo to retrieve detailed information about the bones 
			associated with this message.
		lResult
			Returns 0.
	*/
	oWM_SKELETON,

	/**
		Received when a user of a tracking input device is tracked and recognized.
		wParam
			The user ID.
		lParam
			This parameter is not used.
		lResult
			Returns 0.
	*/
	oWM_USER_CAPTURED,

	/**
		Received when a user of a tracking input device is no longer tracked.
		wParam
			The user ID.
		lParam
			This parameter is not used.
		lResult
			Returns 0.
	*/
	oWM_USER_LOST,

	// Registered messages are for inter-process communication. The first added 
	// ones are to keep Kinect completely abstracted from oWindow.
	oWM_REGISTERED_FIRST,

	/**
		Received when a skeleton device is plugged in or otherwise reinitialized.
		wParam
			The low-order word of wParam specifies an oGUI_INPUT_DEVICE_TYPE.
			The high-order word of wParam specifies an oGUI_INPUT_DEVICE_STATUS.
			oGUI_INPUT_DEVICE_TYPE. The type of the device.
		lParam
			A pointer to a null-terminated string that is the device's instance name.
			No memory management is done to the string memory.
		lResult
			Returns 0.
	*/
	oWM_INPUT_DEVICE_CHANGE = oWM_REGISTERED_FIRST,

	oWM_REGISTERED_LAST = oWM_INPUT_DEVICE_CHANGE,

	// The number of oWM messages, this is not a message
	oWM_COUNT,

	oWM_REGISTERED_COUNT = oWM_REGISTERED_LAST - oWM_REGISTERED_FIRST + 1,
};

// Returns nullptr if the specified messasge is not a registered message.
const char* oWinGetMessageRegisterString(oWM _RegisteredMessage);

// _____________________________________________________________________________
// Raw Input (WM_INPUT support)

enum oUS_USAGE_PAGE_VALUES
{
	oUS_USAGE_PAGE_GENERIC_DESKTOP = 1,
	oUS_USAGE_PAGE_SIMULATION = 2,
	oUS_USAGE_PAGE_VIRTUAL_REALITY = 3,
	oUS_USAGE_PAGE_SPORT = 4,
	oUS_USAGE_PAGE_GAME = 5,
	oUS_USAGE_PAGE_GENERIC_DEVICE = 6,
	oUS_USAGE_PAGE_KEYBOARD = 7,
	oUS_USAGE_PAGE_LEDS = 8,
	oUS_USAGE_PAGE_BUTTON = 9,
};

enum oUS_USAGE
{
	oUS_USAGE_UNDEFINED = 0,
	oUS_USAGE_POINTER = 1,
	oUS_USAGE_MOUSE = 2,
	oUS_USAGE_RESERVED = 3,
	oUS_USAGE_JOYSTICK = 4,
	oUS_USAGE_GAMEPAD = 5,
	oUS_USAGE_KEYBOARD = 6,
	oUS_USAGE_KEYPAD = 7,
	oUS_USAGE_MULTIAXIS_CONTROLLER = 8,
	oUS_USAGE_TABLET_CONTROLS = 9,
};

// _____________________________________________________________________________
// Skeleton/Kinect integration

oDECLARE_HANDLE(HSKELETON);

// Use these to register a skeleton source, i.e. Kinect.
void RegisterSkeletonSource(HSKELETON _hSkeleton, const oFUNCTION<void(oGUI_BONE_DESC* _pSkeleton)>& _GetSkeleton);
void UnregisterSkeletonSource(HSKELETON _hSkeleton);

// Call this from a oWM_SKELETON message. Returns false if _pSkeleton is not 
// valid.
oAPI bool GetSkeletonDesc(HSKELETON _hSkeleton, oGUI_BONE_DESC* _pSkeleton);

// _____________________________________________________________________________
// Top-Level Window Creation, Lifetime and Message Processing

// Creates a simple window class and window. This should be the start of most
// windows to be used with other API from this system. Styles are controlled 
// separately with other API. _pThis should be the class that will be managing/
// wrapping the window. In this way a static WNDPROC can quickly become a method
// of a class. See oWinGetWrapper for more details.
// Threading:
// Care should be taken to call this function in the thread intended for window 
// message processing because Windows automatically sets up the message pump to 
// be this-thread unique, and communicating with this window from other threads 
// can cause race conditions.
oAPI bool oWinCreate(HWND* _pHwnd, const int2& _ClientPosition, const int2& _ClientSize, WNDPROC _Wndproc, void* _pThis = nullptr, bool _AsMessagingWindow = false);

// This function simplifies the association of a 'this' pointer with an HWND. 
// In the WndProc passed to oWinCreate, call this early to get the pointer 
// passed as _pThis. Then user code can convert quickly from the static 
// function requirement of Windows to a more encapsulated C++ class style
// handling of the WNDPROC. This transfers the CREATESTRUCT values to USERDATA 
// during WM_CREATE or for WM_INITDIALOG directly sets the pointer to USERDATA.
// If _pThis is nullptr, then DefWindowProc is called. See oDECLARE_WNDPROC for
// a boilerplate implementation of the static wrapper and class method.
// Threading: WT
oAPI void* oWinGetThis(HWND _hWnd, UINT _uMsg, WPARAM _wParam, LPARAM _lParam);
template<typename T> inline T* oWinGetThis(HWND _hWnd, UINT _uMsg, WPARAM _wParam, LPARAM _lParam) { return (T*)oWinGetThis(_hWnd, _uMsg, _wParam, _lParam); }

// Translates the message if it is a registered into its associated oWM. If it
// it not a registered message, this passes the message through.
UINT oWinTranslateMessage(UINT _uMsg);

// Declares the method WndProc and a static function WndProc within a class to
// create a Windows-compliant C-Style function to pass to oWinCreate above that
// with oWinCreate's _pThis pointer can be redirected to the member method. 
// Client code must copy-paste from the WndProc here (or MS docs) and implement
// the method like a regular WndProc. Remember that oWinGetThis is employed, so 
// client code should take care not to stomp on direct HWND USERDATA and instead 
// define member variables in the 'this' class.
#define oDECLARE_WNDPROC(_ClassName) \
	LRESULT CALLBACK WndProc(HWND _hWnd, UINT _uMsg, WPARAM _wParam, LPARAM _lParam); \
	static LRESULT CALLBACK StaticWndProc(HWND _hWnd, UINT _uMsg, WPARAM _wParam, LPARAM _lParam) \
		{ _ClassName* pThis = oWinGetThis<_ClassName>(_hWnd, _uMsg, _wParam, _lParam); \
			_uMsg = oWinTranslateMessage(_uMsg); \
			return pThis ? pThis->WndProc(_hWnd, _uMsg, _wParam, _lParam) : DefWindowProc(_hWnd, _uMsg, _wParam, _lParam); }

// Returns the thread id for the specified window, or oStd::thread::id() if the
// _hWnd is invalid.
// Threading: IA
oAPI oStd::thread::id oWinGetWindowThread(HWND _hWnd);

// Returns true of the current thread is the one dispatching messages for the 
// window. Many of the API declared below assert this condition to be true.
// Threading: FT
inline bool oWinIsWindowThread(HWND _hWnd) { return oStd::this_thread::get_id() == oWinGetWindowThread(_hWnd); }

// Converts the specified oGUI_HOTKEY_DESCs into an ACCEL array. _pAccels must
// be least the same number of items as _HotKeys.
oAPI void oWinAccelFromHotKeys(ACCEL* _pAccels, const oGUI_HOTKEY_DESC_NO_CTOR* _pHotKeys, size_t _NumHotKeys);

// Converts the specified ACCELs into an oGUI_HOTKEY_DESC array. _pHotKeys must
// be least the same number of items as _pAccels.
oAPI void oWinAccelToHotKeys(oGUI_HOTKEY_DESC_NO_CTOR* _pHotKeys, const ACCEL* _pAccels, size_t _NumHotKeys);

// Dispatches a single message from the Windows message queue. If _WaitForNext 
// is true, the calling thread will sleep (GetMessage). If _WaitForNext is 
// false, then PeekMessage is used. If this returns false, it means a new 
// message was not dispatched. If the message queue is valid and empty, 
// oErrorGetLast() will return std::errc::no_message_available, which might mean
// to client code "do something else", like render a 3D scene or play a video. 
// If a  WM_QUIT message is received, this will return 
// std::errc::operation_canceled, implying the thread pump loop should exit.
// Threading: WT
oAPI bool oWinDispatchMessage(HWND _hWnd, HACCEL _hAccel, bool _WaitForNext = true);

// Send this to wake the specified window's message pump thread up from a 
// blocking GetMessage() call. This is useful if the thread does work of which 
// the Windows message pump (GetMessage) isn't aware, such as a wrapper function
// scheduled on a thread of one of the other Set/Get functions declared below.
// Threading: FT
oAPI bool oWinWake(HWND _hWnd);

// Similar to ::SetParent, but for ownership to mask that the API is very 
// inconsistent and mislabeled.
oAPI bool oWinSetOwner(HWND _hWnd, HWND _hOwner);

// Retrieves the current owner.
oAPI HWND oWinGetOwner(HWND _hWnd);

// _____________________________________________________________________________
// Top-Level Window State

// Returns true if the specified window is a valid ready-to-use window. False
// applies to either nullptr or a window that has been destroyed.
// Threading: IA
oAPI bool oWinExists(HWND _hWnd);

// Returns true if the specified _hWnd is valid and has focus
// Threading: IA
oAPI bool oWinHasFocus(HWND _hWnd);

// Threading: WT
oAPI bool oWinSetFocus(HWND _hWnd, bool _Focus = true);

// Returns true if the specified _hWnd is valid and isEnabled
// Threading: IA
oAPI bool oWinIsEnabled(HWND _hWnd);

// Threading: WT
oAPI bool oWinEnable(HWND _hWnd, bool _Enabled = true);

// Threading: IA
oAPI bool oWinIsAlwaysOnTop(HWND _hWnd);

// Threading: WT
oAPI bool oWinSetAlwaysOnTop(HWND _hWnd, bool _AlwaysOnTop = true);

// There's a known issue that a simple ShowWindow doesn't always work on some 
// minimized apps. The WAR seems to be to set focus to anything else, then try 
// to restore the app. This is exposed to be used on any HWND because it is a 
// real bug/poor behavior in the OS, and is used in oWinSetState to properly
// handle focus.
oAPI bool oWinRestore(HWND _hWnd);

// Returns one of the states enumerated in oGUI_WINDOW_STATE. This uses basic
// Windows API to calculate the state (IsZoomed, IsIconic, etc.).
// Threading: IA
oAPI oGUI_WINDOW_STATE oWinGetState(HWND _hWnd);

// Sets the state of the window. Because it's convenient and atomic, focus can 
// be set at the time of calling this function.
// Threading: WT
oAPI bool oWinSetState(HWND _hWnd, oGUI_WINDOW_STATE _State, bool _TakeFocus = true);

// Returns one of the styles for a top-level window enumerated in 
// oGUI_WINDOW_STYLE. This can return an inappropriate style if the style for
// the specified window was not set using oWinSetStyle.
// Threading: IA
oAPI oGUI_WINDOW_STYLE oWinGetStyle(HWND _hWnd);

// SetStyle can also move/resize the _hWnd if _prClient is specified. _prClient
// is the position and size of the client area of the window. The actual window
// position and size are adjusted to accommodate the specified client size.
// NOTE: This takes into account any associated HMENU, and likewise this API 
// treats a StatusBar control in the same manner. This assumes only one 
// StatusBar is associated with the specified _hWnd.
// Threading: WT
oAPI bool oWinSetStyle(HWND _hWnd, oGUI_WINDOW_STYLE _Style, bool _HasStatusBar, const RECT* _prClient = nullptr);

// _____________________________________________________________________________
// Top-Level Window Geometry

// Returns screen coordinates of the client;'s top left (0,0 in client space)
// Threading: IA
oAPI int2 oWinGetPosition(HWND _hWnd);

// NOTE: It's more efficient if style is changing to roll changes to geometry
// into the _prClient param of oWinSetStyle. If style is not changing, then it
// is more efficient to call one of this oWinSetPosition* functions.

// Sets window position such that the client coordinate 0,0 is at the specified 
// screen position
// Threading: WT
oAPI bool oWinSetPosition(HWND _hWnd, const int2& _ScreenPosition);

// Sets window position and size such that the client coordinate 0,0 is at the
// specified screen position.
// Threading: WT
oAPI bool oWinSetPositionAndSize(HWND _hWnd, const int2& _Position, const int2& _Size);

// Play a windows animation from and to the specified RECTs. This respects user 
// settings as to whether window animated transitions are on. This does not 
// actually move the window, just plays the tween frames from some other 
// instantaneous move.
// Threading: WT
oAPI bool oWinAnimate(HWND _hWnd, const RECT& _From, const RECT& _To);

// Returns the rectangle in which the window is most closely contained. If the
// specified _hWnd is a child window, then the client rect of the parent is 
// returned. If the specified _hWnd is a top-level window, then the rectangle
// of the workarea (non-taskbar area) of the monitor the window is mostly on is 
// returned (NOTE: This is NOT GetDesktopWindow() since that always results in 
// the primary monitor. Prefer this API to GetClientRect(GetDesktopWindow...)
// because this is more multi-monitor-aware. By default, hWnd is queried for its
// parent, but that can be ignored if _hExplicitParent is specified. That is 
// because sometimes client code might need to size a window inside a container
// before it is actually parented, and this allows for that.
oAPI RECT oWinGetParentRect(HWND _hWnd, HWND _hExplicitParent = nullptr);

// Returns the rect of the specified window relative to its parent's client
// area. If _hExplicitParent is nullptr, GetParent() is used on the specified 
// Window. If the specified _hWnd is a top-level window, then the desktop HWND
// is returned.
oAPI RECT oWinGetRelativeRect(HWND _hWnd, HWND _hExplicitParent = nullptr);

// Fills specified rect with the specified window's client rect. This differs 
// from Window's GetClientRect to support status bar as a native window concept.
// So basically if the specified HWND has a status bar, the returned client size
// is shrunk by the size of the status bar (not shrunk if the status bar is 
// hidden).
oAPI bool oWinGetClientRect(HWND _hWnd, RECT* _pRect);

// Fills specified rect with the specified window's client rect, but in screen 
// coordinates. If _HasStatusBar is true, its height is added to the client size 
// to make the status bar seem as if it's not part of the client area.
oAPI bool oWinGetClientScreenRect(HWND _hWnd, bool _HasStatusBar, RECT* _pRect);

// _____________________________________________________________________________
// Top-Level Window Accessors/Mutators

// Gets the index of the display that is most closely associated with the 
// specified window. NOTE: This is an oDisplay-style index compatible with 
// oDisplayEnum(), not a DirectX index.
// Threading: IA
oAPI int oWinGetDisplayIndex(HWND _hWnd);

// Returns the window class's hbrBackground value. This is a non-counted 
// reference, so no lifetime management should be performed on the HBRUSH.
// Threading: IA
oAPI HBRUSH oWinGetBackgroundBrush(HWND _hWnd);

// Returns the currently set font.
// Valid for: All
// Threading: IA
oAPI HFONT oWinGetFont(HWND _hWnd);

// Returns the currently set (themed) default font
oAPI HFONT oWinGetDefaultFont();

// If nullptr the DEFAULT_GUI_FONT is set. See oGDICreateFont to create a custom 
// font.
// Valid for: All
// Threading: WT
oAPI bool oWinSetFont(HWND _hWnd, HFONT _hFont = nullptr);

// Sets the text of a window. If _SubItemIndex is oInvalid, this will set the
// main text. If the window is a control that contains subitems, the specified
// subitem's text will be set. The specified _SubItemIndex must exist or be
// oInvalid for this to return success/true.
// Threading: WT
// Also note that this is a pointer specified, so the buffer must remain valid
// until this function returns.
oAPI bool oWinSetText(HWND _hWnd, const char* _Text, int _SubItemIndex = oInvalid);

// Returns the number of characters used if the string were truncated. This 
// includes the ellipse.
oAPI size_t oWinGetTruncatedLength(HWND _hWnd, const char* _StrSource);

// Adds ellipse to the left of the string and shortens the string to fit in the
// specified _hWnd's client rect. This will preserve as much of the right  of 
// the string as possible.
oAPI char* oWinTruncateLeft(char* _StrDestination, size_t _SizeofStrDestination, HWND _hWnd, const char* _StrSource);
template<size_t size> char* oWinTruncateLeft(char (&_StrDestination)[size], HWND _hWnd, const char* _StrSource) { return oWinTruncateLeft(_StrDestination, size, _hWnd, _StrSource); }
template<size_t capacity> char* oWinTruncateLeft(oStd::fixed_string<char, capacity>& _StrDestination, HWND _hWnd, const char* _StrSource) { return oWinTruncateLeft(_StrDestination, _StrDestination.capacity(), _hWnd, _StrSource); }

// Fills _StrDestination with a copy of the path that has ellipse placed in the 
// middle of the path according to windows rules sized to the rectangle and 
// context used by the specified HWND for drawing.
oAPI char* oWinTruncatePath(char* _StrDestination, size_t _SizeofStrDestination, HWND _hWnd, const char* _Path);
template<size_t size> char* oWinTruncatePath(char (&_StrDestination)[size], HWND _hWnd, const char* _Path) { return oWinTruncatePath(_StrDestination, size, _hWnd, _Path); }
template<size_t capacity> char* oWinTruncatePath(oStd::fixed_string<char, capacity>& _StrDestination, HWND _hWnd, const char* _Path) { return oWinTruncatePath(_StrDestination, _StrDestination.capacity(), _hWnd, _Path); }

// Returns _StrDestination on success, otherwise nullptr.
// Threading: IA
oAPI char* oWinGetText(char* _StrDestination, size_t _SizeofStrDestination, HWND _hWnd, int _SubItemIndex = oInvalid);
template<size_t size> char* oWinGetText(char (&_StrDestination)[size], HWND _hWnd, int _SubItemIndex = oInvalid) { return oWinGetText(_StrDestination, size, _hWnd, _SubItemIndex); }
template<size_t capacity> char* oWinGetText(oStd::fixed_string<char, capacity>& _StrDestination, HWND _hWnd, int _SubItemIndex = oInvalid) { return oWinGetText(_StrDestination, _StrDestination.capacity(), _hWnd, _SubItemIndex); }

// Gets the icon (large or small) associated with the specified _hWnd. No 
// lifetime needs to be performed on the returned value.
// Threading: IA
oAPI HICON oWinGetIcon(HWND _hWnd, bool _BigIcon = false);

// Asynchronously set the specified window's icon. This will enqueue the set,
// but may not be atomic with other calls either from client code or from 
// Windows internal code. If this is called from a thread different than the
// _hWnd's, this will return false and oErrorGetLast will return 
// std::errc::operation_not_permitted.
oAPI bool oWinSetIconAsync(HWND _hWnd, HICON _hIcon, bool _BigIcon = false);

// @oooii-tony: I am still unclear about lifetime management... I don't see leak
// reports in current usage, so I've not have something to fully trace through...
// Let me know if you see leaks relating to HICONs.
// Threading: WT
oAPI bool oWinSetIcon(HWND _hWnd, HICON _hIcon, bool _BigIcon = false);

// _____________________________________________________________________________
// UI Control Creation, Lifetime and Message Processing

// NOTE: Tabs send WM_NOTIFY messages. The passed NMHDR::code includes:
// TCN_KEYDOWN, TCN_SELCHANGE, TCN_SELCHANGING, TCN_GETOBJECT, TCN_FOCUSCHANGE

// When finished with this HWND, use DestroyWindow() to free it, though since
// all controls must have a parent and on Windows the parent deletes all its
// children, in many cases the lifetime of the HWND need not be managed, or even
// retained.
// Threading: WT
oAPI HWND oWinControlCreate(const oGUI_CONTROL_DESC& _Desc);

// Several controls have very typical behavior that occurs on a WM_NOTIFY 
// message, so encapsulate that functionality here. Returns true if the message
// was consumed by this handler, or false if it did nothing or subsequent 
// handlers should not be masked. If this returns true, then the WM_NOTIFY 
// handler should return the value filled into _plResult. Type will be figured
// out internally, but if client code needs to figure out the type before this
// function, passing that through can avoid the recalculation.
// FloatboxSpinner: handles incrementing/decrementing values
// Hyperlabel: handles chasing web links
// Threading: WT
oAPI bool oWinControlDefaultOnNotify(HWND _hWnd, const NMHDR& _NotifyMessageHeader, LRESULT* _plResult = nullptr, oGUI_CONTROL_TYPE _Type = oGUI_CONTROL_UNKNOWN);

// Converts a Windows control message to an action. This calls 
// oWinControlDefaultOnNotify if there is no other appropriate handling. This
// returns true if the output action and lresults are valid and should be 
// respected or false if this was not a control message.
oAPI bool oWinControlToAction(HWND _hWnd, UINT _uMsg, WPARAM _wParam, LPARAM _lParam, oGUI_ACTION_DESC* _pAction, LRESULT* _pLResult);

// _____________________________________________________________________________
// UI Control State

// Returns true if the specified _hControl is a valid ready-to-use window. False
// applies to either nullptr or a window that has been destroyed.
// Valid for: All
// Threading: IA
inline bool oWinControlExists(HWND _hControl) { return oWinExists(_hControl); }

// Returns true if the specified control can receive events
// Valid for: All
// Threading: IA
inline bool oWinControlIsEnabled(HWND _hControl) { return oWinIsEnabled(_hControl); }

// Set whether or not the specified control can receive events
// Valid for: All
// Threading: WT
inline void oWinControlEnable(HWND _hControl, bool _Enabled = true) { oWinEnable(_hControl, _Enabled); }

// Sets whether the control can be seen or not. If not visible, the control 
// becomes non-interactive (tabstops, etc.)
// Threading: WT
inline bool oWinControlSetVisible(HWND _hControl, bool _Visible = true) { return oWinSetState(_hControl, _Visible ? oGUI_WINDOW_RESTORED : oGUI_WINDOW_HIDDEN, false); }

// _____________________________________________________________________________
// UI Control SubItems

// Returns the number of subitems associated with the specified _hWnd. If the 
// type of HWND doesn't have subitems, then the return value is zero with no
// error reported.
// Valid for: All
// Threading: IA
oAPI int oWinControlGetNumSubItems(HWND _hControl);

// Removes all subitems from the specified _hWnd. If the type doesn't support
// subitems, no error is reported.
// Valid for: All
// Threading: WT
oAPI bool oWinControlClearSubItems(HWND _hControl);

// Inserts before the specified subitem index. If _SubItemIndex is oInvalid (-1) 
// then the item is appended. This returns the index of the item set, or 
// oInvalid on failure.
// Valid for: ComboBox, ComboTextbox, Tab
// Threading: WT
oAPI int oWinControlInsertSubItem(HWND _hControl, const char* _SubItemText, int _SubItemIndex);

// Deletes the nth subitem
// Valid for: ComboBox, ComboTextbox, Tab
// Threading: WT
oAPI bool oWinControlDeleteSubItem(HWND _hControl, const char* _SubItemText, int _SubItemIndex);

// Adds all items in the specified delimited list in order.
// Valid for: ComboBox, ComboTextbox, Tab
// Threading: WT
oAPI bool oWinControlAddSubItems(HWND _hControl, const char* _DelimitedString, char _Delimiter = '|');

// Returns the index of the first subitem to match the specified text, or 
// oInvalid if not found (or other error).
// Valid For: ComboBox, ComboTextbox, Tab
// Threading: IA
oAPI int oWinControlFindSubItem(HWND _hControl, const char* _SubItemText);

// Sets the specified index as the selected item/text for the combobox
// Valid for: ComboBox, ComboTextbox, Tab
// Threading: WT
oAPI bool oWinControlSelectSubItem(HWND _hControl, int _Index);

// Sets the specified text as the selected item/text for the combobox (if it 
// exists)
// Valid for: ComboBox, ComboTextbox, Tab
// Threading: WT
inline bool oWinControlSelectSubItem(HWND _hControl, const char* _SubItemText) { return oWinControlSelectSubItem(_hControl, oWinControlFindSubItem(_hControl, _SubItemText)); }

// Selects n from the currently selected, properly wrapping around either end
// Valid for: ComboBox, ComboTextBox, Tab
// Threading: WT
oAPI bool oWinControlSelectSubItemRelative(HWND _hControl, int _Offset);

// Returns the index of the currently selected item, or -1 if none selected
// Valid for: ComboBox, ComboTextBox, Tab
// Threading: IA
oAPI int oWinControlGetSelectedSubItem(HWND _hControl);

// _____________________________________________________________________________
// Control Accessors/Mutators

// This will probably return oWINDOW_CONTROL_UNKNOWN for any control not created
// with oWinControlCreate().
// Valid for: All
// Threading: IA
oAPI oGUI_CONTROL_TYPE oWinControlGetType(HWND _hControl);

// Returns the size returned if int2(oDEFAULT, oDEFAULT) were passed as the size
// value to oWinControlCreate().
// Valid for: All
// Threading: no threading issues. (call from anywhere)
oAPI int2 oWinControlGetInitialSize(oGUI_CONTROL_TYPE _Type, const int2& _Size);

// Returns true if the specified _hWnd has the WS_TABSTOP style and is also 
// enabled and visible, so all criteria from tabbing and using the control must 
// be met for this to return true and thus the answer can change if the state of 
// the control changes. This should only be called from the same thread on which 
// the window was created.
// Valid for: All
// Threading: IA
oAPI bool oWinControlIsTabStop(HWND _hControl);

// Returns the ID for a control given its HWND
// Valid for: All
// Threading: IA
oAPI int oWinControlGetID(HWND _hControl);

// Returns the HWND for a control given its parent and ID
// Valid for: All
// Threading: IA
oAPI HWND oWinControlGetFromID(HWND _hParent, unsigned short _ID);

// Returns the bounding rectangle of the specified _hControl in terms of its
// parent's client area.
// Valid for: All
// Threading: IA
oAPI RECT oWinControlGetRect(HWND _hControl);

// Returns the currently set font.
// Valid for: All
// Threading: IA
inline HFONT oWinControlGetFont(HWND _hControl) { return oWinGetFont(_hControl); }

// If nullptr the DEFAULT_GUI_FONT is set
// Valid for: All
// Threading: WT
inline bool oWinControlSetFont(HWND _hControl, HFONT _hFont = nullptr) { return oWinSetFont(_hControl, _hFont); }

// Either sets the text for simple controls, or the nth text item in list-like
// controls such as tabs, combobox, and listboxes. If _SubItemIndex is not found
// this will fail. If _SubItemIndex is oInvalid, then the main text of the 
// control is set. If the control doesn't have the concept of main text, then 
// this will fail.
// Valid for: All
// Threading:: WT
inline bool oWinControlSetText(HWND _hControl, const char* _Text, int _SubItemIndex = oInvalid) { return oWinSetText(_hControl, _Text, _SubItemIndex); }

// Returns _StrDestination on success, otherwise nullptr.
// Threading: IA
inline char* oWinControlGetText(char* _StrDestination, size_t _SizeofStrDestination, HWND _hControl, int _SubItemIndex = oInvalid) { return oWinGetText(_StrDestination, _SizeofStrDestination, _hControl, _SubItemIndex); }
template<size_t size> char* oWinControlGetText(char (&_StrDestination)[size], HWND _hControl, int _SubItemIndex = oInvalid) { return oWinGetText(_StrDestination, _hControl, _SubItemIndex); }
template<size_t capacity> char* oWinControlGetText(oStd::fixed_string<char, capacity>& _StrDestination, HWND _hControl, int _SubItemIndex = oInvalid) { return oWinGetText(_StrDestination, _hControl, _SubItemIndex); }

// Returns _StrDestination on success, otherwise nullptr. _StrDestination is 
// filled with the portion of the specified _hControl that is considered 
// selected by the UI.
// Valid for: ComboTextbox, TextBox, FloatBox, FloatBoxSpinner
oAPI char* oWinControlGetSelectedText(char* _StrDestination, size_t _SizeofStrDestination, HWND _hControl);
template<size_t size> char* oWinControlGetSelectedText(char (&_StrDestination)[size], HWND _hControl) { return oWinControlGetSelectedText(_StrDestination, size, _hWnd); }
template<size_t capacity> char* oWinControlGetSelectedText(oStd::fixed_string<char, capacity>& _StrDestination, HWND _hControl) { return oWinControlGetSelectedText(_StrDestination, _StrDestination.capacity(), _hWnd); }

// Applies a selection highlight to the specified _hWnd according to the 
// specified range.
// Valid for: ComboTextBox, TextBox, FloatBox, FloatBoxSpinner, LabelSelectable,
//            SliderSelectable
// Threading: WT
oAPI bool oWinControlSelect(HWND _hControl, int _Start, int _Length);

// Sets/Gets a textbox-like object by value. For verified input boxes like 
// FloatBox or FloatBoxSpinner, this bypasses first-pass error checking and 
// ensures that well-formatted data passes the control's second-pass error 
// checking. Using oWinSetText on these without the first-pass error checking
// would quietly noop.
// Valid for: All the same as oWinSetText
// Threading: WT
oAPI bool oWinControlSetValue(HWND _hControl, float _Value);

// This interprets whatever text is specified as a float (non-scientific form).
// If the string could not be interpreted as a float then this returns a NaN.
// Use isnan() from oMath.h to test.
// Valid for: All
// Threading: IA
oAPI float oWinControlGetFloat(HWND _hControl);

// Associates the specified _hIcon with the specified _hControl
// Valid for: Icon
// Threading: WT
oAPI bool oWinControlSetIcon(HWND _hControl, HICON _hIcon, int _SubItemIndex = oInvalid);

// Get's the icon associated with the specified _hControl
// Valid for: Icon
// Threading: IA
oAPI HICON oWinControlGetIcon(HWND _hControl, int _SubItemIndex = oInvalid);

// Returns true if the specified _hControl's checked state is true. This returns 
// false for any other tri-state state or if the specified _hControl is not a 
// type that can be checked. If legitimately not checked, oErrorGetLast will 
// return 0.
// Valid for: CheckBox, RadioButton
// Threading: IA
oAPI bool oWinControlIsChecked(HWND _hControl);

// Sets the specified _hControl as checked or not. If not a valid type, this 
// will return false.
// Valid for: CheckBox, RadioButton
// Threading: WT
oAPI bool oWinControlSetChecked(HWND _hControl, bool _Checked = true);

// Sets the specified _hControl's min and max range. If not a valid type, this 
// will return false. If _Min == _Max, then the control is put in the "unknown
// range" state. For ProgressBar, this means it shows the "marquee", a scrolling 
// progress bar that indicates something is happening, but it is not known how 
// much of that something is complete.
// Valid for: Slider, SliderSelectable, ProgressBar
// Threading: WT
oAPI bool oWinControlSetRange(HWND _hControl, int _Min, int _Max);

// Gets the specified _hControl's min and max range. If not a valid type, this 
// will return false.
// Valid for: Slider, SliderSelectable, ProgressBar
// Threading: WT
oAPI bool oWinControlGetRange(HWND _hControl, int* _pMin, int* _pMax);

// Sets the specified _hControl's position within its min and max range. If 
// is outside the range, then it is clamped. If the _hControl is not a valid 
// type, this will return false.
// Valid for: Slider, SliderSelectable, ProgressBar
// Threading: WT
oAPI bool oWinControlSetRangePosition(HWND _hControl, int _Position, bool _bNotify = true);

// Gets the specified _hControl's position within its min and max range. If 
// is outside the range, then it is clamped. If the _hControl is not a valid 
// type, this will return false.
// Valid for: Slider, SliderSelectable, ProgressBar
// Threading: WT
oAPI int oWinControlGetRangePosition(HWND _hControl);

// Sets a tick at the specified point on a trackbar
// Valid for: SliderWithTicks
oAPI bool oWinControlSetTick(HWND _hControl, int _Position);

// Removes all ticks on a trackbar
// Valid for: SliderWithTicks
oAPI bool oWinControlClearTicks(HWND _hControl);

// Sets the control to be in a state indicative to the user as in error.
// Valid for: ProgressBar
// Threading: WT
oAPI bool oWinControlSetErrorState(HWND _hControl, bool _InErrorState = true);

// Returns true if the control is currently indicating an error state to the 
// user. If the control is one valid for this function, the last error is set
// to 0 to differentiate between a valid false and a false because an improper 
// type was specified.
// Valid for: ProgressBar
// Threading: WT
oAPI bool oWinControlGetErrorState(HWND _hControl);

// Utility code that will readjust a range position that is outside the 
// control's selected region.
// Valid for: SliderSelectable
// Threading: WT
oAPI bool oWinControlClampPositionToSelected(HWND _hControl);

// _____________________________________________________________________________
// @oooii-tony: Dialog-related stuff. I still don't quite understand the 
// difference between a window and a dialog - it seems like dialogs were once a 
// simple idea that benefited from some simplification, but it seems that time
// has gone away. Experiment more with creating various "simple" dialogs and 
// see if we can't just dissolve the truly scary stuff it takes to make dialogs
// work. (See the implementation for oDlgNewTemplate)

#define oDECLARE_DLGPROC(_Static, _Name) \
	_Static INT_PTR CALLBACK _Name(HWND _hWnd, UINT _uMsg, WPARAM _wParam, LPARAM _lParam)

#define oDEFINE_DLGPROC(_ClassName, _Name) \
	INT_PTR CALLBACK _ClassName::_Name(HWND _hWnd, UINT _uMsg, WPARAM _wParam, LPARAM _lParam) \
{	_ClassName* pThis = oWinGetThis<_ClassName>(_hWnd, _uMsg, _wParam, _lParam); \
	return pThis ? pThis->WndProc(_hWnd, _uMsg, _wParam, _lParam) : DefWindowProc(_hWnd, _uMsg, _wParam, _lParam); \
}

#define oDEFINE_DLGPROC_DEBUG(_ClassName, _Name) \
	INT_PTR CALLBACK _ClassName::_Name(HWND _hWnd, UINT _uMsg, WPARAM _wParam, LPARAM _lParam) \
{	char WM[1024]; \
	oTRACE("%s", oWinParseWMMessage(WM, _hWnd, _uMsg, _wParam, _lParam)); \
	_ClassName* pThis = oWinGetThis<_ClassName>(_hWnd, _uMsg, _wParam, _lParam); \
	return pThis ? pThis->WndProc(_hWnd, _uMsg, _wParam, _lParam) : DefWindowProc(_hWnd, _uMsg, _wParam, _lParam); \
}

#endif
