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
// Utilities for working with HWND objects. It's amazing how inconsistent much 
// of the Windows API is, so prefer using the more consistent, centralized API 
// here over native calls, a seemingly simple wrappers might call several 
// carefully-ordered win32 API.

// Windows GUI handling is single-threaded. All multi-threaded calls can be 
// piped to the GUI thread for processing, but that is not done at this level,
// this module is solely intended to simplify and make consistent some of 
// Window's disparate and evolutionary API. All functions are expected to be 
// called from the same thread as oWinCreate was, or they will return failure 
// unless another description exists in the function's comments.

#pragma once
#ifndef oWinWindowing_h
#define oWinWindowing_h

#include <oStd/thread.h>
#include <oCore/display.h>
#include <oBasis/oGUI.h>
#include "../Source/oStd/win.h"

#if (defined(NTDDI_WIN7) && (NTDDI_VERSION >= NTDDI_WIN7))
	#define oWINDOWS_HAS_REGISTERTOUCHWINDOW
#endif

// _____________________________________________________________________________
// Custom messages

enum oWM
{
	// Custom Windows messages defined for windows created with oWinCreate.

	oWM_FIRST_MESSAGE = WM_APP,

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
		Use this to call DestroyWindow() with a PostMessage().
		wParam
			This parameter is not used.
		lParam
			This parameter is not used.
		lResult
			Returns 0.
	*/
	oWM_DESTROY,

	/**
		Received when an input device has updated skeleton data. Use of this message 
		is an attempt to model Kinect/Skeleton data (i.e. low-level gesture) on
		WM_TOUCH.
		wParam
			The low-order word of wParam specifies the ID of the skeleton.
			The high-order word of wParam is zero.
		lParam
			Contains a skeleton input handle that can be used in a call to 
			GetSkeletonDesc to retrieve detailed information about the bones 
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

// Returns the native value for the specified oWM message. The value specified
// must be >= oWM_REGISTERED_FIRST and <= oWM_REGISTERED_LAST, else this will
// return 0. This is intended primarily for producers of the message since the 
// window is set up to consume and convert native registered messages.
UINT oWinGetNativeRegisteredMessage(oWM _RegisteredMessage);

// Call the specified function for each of the top level windows on the system. 
// The function should return true to keep searching or false to exit early.
void oWinEnumWindows(const std::function<bool(HWND _hWnd)>& _Enumerator);

// Retrieves the HWND of the top level window owned by the specified process and
// the ID of its message pump thread. Since a process can have more than one top 
// level window an optional name can also be specified to make certain the 
// correct window is returned.
bool oWinGetProcessTopWindowAndThread(ouro::process::id _ProcessID
	, HWND* _pHWND
	, oStd::thread::id* _pThreadID
	, const char* _pWindowName = nullptr);

// _____________________________________________________________________________
// Basic Top-Level Window Creation, Lifetime and Message Processing

// Fills _StrDestination with the class name of a window created with 
// oWinCreate. This sythesizes the name, it does not use ::GetClassName to 
// retrieve a name.
oAPI char* oWinMakeClassName(char* _StrDestination, size_t _SizeofStrDestination, WNDPROC _Wndproc);
template<size_t size> char* oWinMakeClassName(char (&_StrDestination)[size], WNDPROC _Wndproc) { return oWinMakeClassName(_StrDestination, size, _Wndproc); }
template<typename charT, size_t capacity> char* oWinMakeClassName(ouro::fixed_string<charT, capacity>& _StrDestination, WNDPROC _Wndproc) { return oWinMakeClassName(_StrDestination, _StrDestination.capacity(), _Wndproc); }

// Returns true if the specified window uses the specified _Wndproc and was
// created with oWinCreate.
oAPI bool oWinIsClass(HWND _hWnd, WNDPROC _Wndproc);

// Passed as the platform-provided init parameter.
struct oWIN_CREATESTRUCT
{
	oGUI_WINDOW_SHAPE_DESC Shape;
	void* pThis;
	void* pInit;
};

// Creates a top-level window class and window. This should be the start of most 
// windows to be used with other API from this system. an instance of
// oWIN_CREATESTRUCT will be passed to WM_CREATE's CREATESTRUCT's lpCreateParams 
// and the _pInit specified to oWinCreate will be passed in the member of pInit
// of oWIN_CREATESTRUCT. Be careful if using pThis or native platform calls 
// since the window is not yet fully constructed. _pThis should be the class 
// that will be managing/wrapping the window. The pointer will be stored and 
// accessible through helper API below that easily converts a static WNDPROC 
// into a class method. See oWinGetThis for more details.
// Threading:
// Care should be taken to call this function in the thread intended for window 
// message processing.
oAPI HWND oWinCreate(HWND _hParent
	, const char* _Title
	, oGUI_WINDOW_STYLE _Style
	, const int2& _ClientPosition
	, const int2& _ClientSize
	, WNDPROC _Wndproc
	, void* _pInit
	, void* _pThis);

// Calls DestroyWindow if called from the window thread, or posts an 
// oWM_DESTROY, which will call DestroyWindow from the window thread if this 
// function is called from any other thread. Thus it is safe to call this from 
// any thread. Typical use is to call this in a wrapper class's destructor, 
// which can be called from any thread that releases its last refcount.
oAPI void oWinDestroy(HWND _hWnd);

// Returns the pointer passed as _pThis to oWinCreate. This gets set only at 
// the very end of WM_CREATE since the this object would be in the middle of 
// bootstrapping, so this may return nullptr, which when it does it is 
// recommended that the system call DefWindowProc().
void* oWinGetThis(HWND _hWnd);
template<typename T> inline T* oWinGetThis(HWND _hWnd) { return (T*)::oWinGetThis(_hWnd); }

// Translates the message if it is a registered into its associated oWM. If it
// it not a registered message, this passes the message through. Call this on
// whatever the first change at getting the message is so that this-class-
// specific messages are properly recognized (most translations will become an 
// oWM between oWM_REGISTERED_FIRST and oWM_REGISTERED_LAST).
UINT oWinTranslateMessage(HWND _hWnd, UINT _uMsg);

// The "DefWindowProc" for Windows created with oWinCreate. This is generally
// called before any user-specified callback (oDECLARE_WNDPROC) to handle the 
// very basics. If this returns a value greater than 0, then this handling 
// should override any additional handling and return 0. Mostly this supports
// more elegant handling of fullscreen, status bar-like-menu integration into
// the window and simplified device change events.
LRESULT CALLBACK oWinWindowProc(HWND _hWnd, UINT _uMsg, WPARAM _wParam, LPARAM _lParam);

// Declares the method WndProc and a static function StaticWndProc within a class 
// to create a Windows-compliant C-Style function to pass to oWinCreate above 
// that with oWinCreate's _pThis pointer can be redirected to the member method. 
#define oDECLARE_WNDPROC(_ClassName) \
	LRESULT CALLBACK WndProc(HWND _hWnd, UINT _uMsg, WPARAM _wParam, LPARAM _lParam); \
	static LRESULT CALLBACK StaticWndProc(HWND _hWnd, UINT _uMsg, WPARAM _wParam, LPARAM _lParam) \
	{ _uMsg = oWinTranslateMessage(_hWnd, _uMsg); \
		if (oWinWindowProc(_hWnd, _uMsg, _wParam, _lParam) >= 0) return 0; \
		_ClassName* pThis = oWinGetThis<_ClassName>(_hWnd); \
		if (!pThis) return DefWindowProc(_hWnd, _uMsg, _wParam, _lParam); \
		return pThis->WndProc(_hWnd, _uMsg, _wParam, _lParam); \
	}

// Returns the thread id for the specified window, or oStd::thread::id() if the
// _hWnd is invalid.
oAPI oStd::thread::id oWinGetWindowThread(HWND _hWnd);

// Returns true of the current thread is the one dispatching messages for the 
// window. Many of the API declared below assert this condition to be true. This 
// can be called from any thread.
inline bool oWinIsWindowThread(HWND _hWnd) { return oStd::this_thread::get_id() == oWinGetWindowThread(_hWnd); }

// Returns true if the specified message is a class-specifically registered 
// custom message.
inline bool oWinIsRegisteredMessage(UINT _uMsg) { return _uMsg >= 0xC000 && _uMsg <= 0xFFFF; }

// Dispatches a single message from the Windows message queue. If _WaitForNext 
// is true, the calling thread will sleep (GetMessage). If _WaitForNext is 
// false, then PeekMessage is used. If this returns false, it means a new 
// message was not dispatched. If the message queue is valid and empty, 
// oErrorGetLast() will return std::errc::no_message_available, which might mean
// to client code "do something else", like render a 3D scene or play a video. 
// If a  WM_QUIT message is received, this will return false with a last error
// of std::errc::operation_canceled, implying the thread pump loop should exit.
oAPI bool oWinDispatchMessage(HWND _hWnd, HACCEL _hAccel, bool _WaitForNext = true);

// _____________________________________________________________________________
// Extended Top-Level Window Message Processing

// Sometimes a windows API will call SendMessage internally. When several APIs 
// are called together, a lot of noise can be generated for that event, and 
// sometimes can result in invalid states because of what oWinWindowing APIs
// emulate. This function is used internally to wrap certain calls so events can 
// be queried to see if a response is truly needed, or if there is more coming.
// This is reference-counted. It starts in not-temp mode at 0, and each setting
// to true increments the count so this can be called recursively. Values are 
// clamped at zero.
oAPI bool oWinSetTempChange(HWND _hWnd, bool _IsTemp);

// This is used to query the state of "temp change" for a window.
oAPI bool oWinIsTempChange(HWND _hWnd);

// On Windows, getting a WM_TOUCH message is not the default. Also receiving 
// this message can affect how mouse messages behave since a lot of drivers 
// emulate the mouse through touch. This API provides a toggle for the specified
// window as to whether such events are enabled. This will return failure on 
// versions of Windows prior to Windows 7 since the underlying API is not 
// supported prior to Windows 7.
oAPI bool oWinRegisterTouchEvents(HWND _hWnd, bool _Registered);

// Converts the specified oGUI_HOTKEY_DESCs into an ACCEL array. _pAccels must
// be least the same number of items as _HotKeys.
oAPI void oWinAccelFromHotKeys(ACCEL* _pAccels, const oGUI_HOTKEY_DESC_NO_CTOR* _pHotKeys, size_t _NumHotKeys);

// Converts the specified ACCELs into an oGUI_HOTKEY_DESC array. _pHotKeys must
// be least the same number of items as _pAccels.
oAPI void oWinAccelToHotKeys(oGUI_HOTKEY_DESC_NO_CTOR* _pHotKeys, const ACCEL* _pAccels, size_t _NumHotKeys);

// _____________________________________________________________________________
// Basic Window Components.

// Gets the icon (large or small) associated with the specified _hWnd. No 
// lifetime needs to be performed on the returned value.
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
oAPI bool oWinSetIcon(HWND _hWnd, HICON _hIcon, bool _BigIcon = false);

// Sets the text of a window. If _SubItemIndex is oInvalid, this will set the
// main text. If the window is a control that contains subitems, the specified
// subitem's text will be set. The specified _SubItemIndex must exist or be
// oInvalid for this to return success/true.
// Also note that this is a pointer specified, so the buffer must remain valid
// until this function returns.
oAPI bool oWinSetText(HWND _hWnd, const char* _Text, int _SubItemIndex = oInvalid);

// Returns _StrDestination on success, otherwise nullptr.
oAPI char* oWinGetText(char* _StrDestination, size_t _SizeofStrDestination, HWND _hWnd, int _SubItemIndex = oInvalid);
template<size_t size> char* oWinGetText(char (&_StrDestination)[size], HWND _hWnd, int _SubItemIndex = oInvalid) { return oWinGetText(_StrDestination, size, _hWnd, _SubItemIndex); }
template<size_t capacity> char* oWinGetText(ouro::fixed_string<char, capacity>& _StrDestination, HWND _hWnd, int _SubItemIndex = oInvalid) { return oWinGetText(_StrDestination, _StrDestination.capacity(), _hWnd, _SubItemIndex); }

// Returns the window class's hbrBackground value. This is a non-counted 
// reference, so no lifetime management should be performed on the HBRUSH.
oAPI HBRUSH oWinGetBackgroundBrush(HWND _hWnd);

// If nullptr the DEFAULT_GUI_FONT is set. See oGDICreateFont to create a custom 
// font.
oAPI bool oWinSetFont(HWND _hWnd, HFONT _hFont = nullptr);

// Returns the currently set font.
oAPI HFONT oWinGetFont(HWND _hWnd);

// Returns the currently set (themed) default font
oAPI HFONT oWinGetDefaultFont();

// _____________________________________________________________________________
// Extended Window Components. Window's handling of menus is "special". Status 
// bars always seem like a part of the frame since they have a "grip" for resize 
// so make the behavior of menu and status bar as uniformly not-a-child-object 
// as possible.

// Windows created with oWinCreate retain a top-level window so that it's always
// around to consistently calculate size, since menus affect non-client area 
// calculations. This returns the userdata-stored value regardless of what 
// ::GetMenu() returns.
oAPI HMENU oWinGetMenu(HWND _hWnd);

// Calls SetMenu() to either the value of oWinGetMenu or null, depending on the 
// _Show value.
oAPI bool oWinShowMenu(HWND _hWnd, bool _Show = true);

// Returns true if the specified window has a menu
oAPI bool oWinMenuShown(HWND _hWnd);

// All windows created with oWinCreate have a status bar. This retrieves it.
oAPI HWND oWinGetStatusBar(HWND _hWnd);

// Sets the status bar of the associated window to be visible or hidden.
oAPI bool oWinShowStatusBar(HWND _hWnd, bool _Show = true);

// Returns the state set by oWinShowStatusBar. This can return visible even if 
// the parent window is hidden - it only describes the local state of the 
// status bar.
oAPI bool oWinStatusBarShown(HWND _hWnd);

// _____________________________________________________________________________
// Top-Level Window State

// Returns true if the specified window is a valid ready-to-use window. False
// applies to either nullptr or a window that has been destroyed.
oAPI bool oWinExists(HWND _hWnd);

// Due to Aero-style compositing, there may be a period while a window is being
// shown from hidden where the window is not opaque, thus generating incorrect 
// snapshots. Use this to determine if a window is finished with any transition 
// and is in a steady visual state.
oAPI bool oWinIsOpaque(HWND _hWnd);

// Gets the index of the display that is most closely associated with the 
// specified window. NOTE: This is an oDisplay-style index compatible with 
// oDisplayEnum(), not a DirectX index.
oAPI ouro::display::id oWinGetDisplayId(HWND _hWnd);

// Returns true if the specified _hWnd is valid and has focus
oAPI bool oWinHasFocus(HWND _hWnd);
oAPI bool oWinSetFocus(HWND _hWnd, bool _Focus = true);

// Returns true if the specified _hWnd is valid and isEnabled
oAPI bool oWinIsEnabled(HWND _hWnd);
oAPI bool oWinEnable(HWND _hWnd, bool _Enabled = true);

oAPI bool oWinIsAlwaysOnTop(HWND _hWnd);
oAPI bool oWinSetAlwaysOnTop(HWND _hWnd, bool _AlwaysOnTop = true);

// Similar to ::SetParent, but for ownership to mask that the API is very 
// inconsistent and mislabeled.
oAPI bool oWinSetOwner(HWND _hWnd, HWND _hOwner);
oAPI HWND oWinGetOwner(HWND _hWnd);
oAPI bool oWinIsOwner(HWND _hWnd);

// This calls ::SetParent, but also sets WS_CHILD v. WS_POPUP correctly.
oAPI bool oWinSetParent(HWND _hWnd, HWND _hParent);
oAPI HWND oWinGetParent(HWND _hWnd);
oAPI bool oWinIsParent(HWND _hWnd);

// Returns true if this window has been flagged as a render target. Mainly a 
// render target is not allowed to have a status bar.
oAPI bool oWinIsRenderTarget(HWND _hWnd);

// Returns true if this window has been flagged as the owner of the thread it
// was created on. Owners call PostQuitMessage when they are destroyed. Non-
// owners (such as the assert dialog box) do not.
oAPI bool oWinIsThreadOwner(HWND _hWnd);

// Sets a flag to mark the specified window as a render target. This does 
// nothing more than to cause oWinIsRenderTarget to return true or false and 
// does nothing to actually make the specified window a render target since that
// is up to the rendering system.
oAPI bool oWinSetIsRenderTarget(HWND _hWnd, bool _IsRenderTarget = true);

// Returns true if this window has been flagged as fullscreen exclusive. Mainly 
// this is checked if there's a call to oWinSetShape while the window is being
// managed by another API (DirectX).
oAPI bool oWinIsFullscreenExclusive(HWND _hWnd);

// Sets a flag to mark the specified window as exclusively controlled by a 
// fullscreen device. This does nothing more than to cause 
// oWinIsFullscreenExclusive to return true or false and does nothing to 
// actually make the specified window exclusive since that is up to the 
// rendering system.
oAPI bool oWinSetIsFullscreenExclusive(HWND _hWnd, bool _IsFullscreenExclusive = true);

// Returns true if alt-F4 is enabled to close the window.
oAPI bool oWinAltF4IsEnabled(HWND _hWnd);

// Sets whether or not Alt-F4 closes the window.
oAPI bool oWinAltF4Enable(HWND _hWnd, bool _Enabled = true);

// _____________________________________________________________________________
// Top-Level Window Shape

// Fills the specified rect with the specified window's client rect. If the 
// window is a child window, the coordinates will be in the parent's client 
// space. If the specified window is a top-level window then the rect is in 
// virtual desktop coordinate space. This differs from Window's GetClientRect to 
// support status bar as a native window concept. So basically if the specified 
// HWND has a status bar, the returned client size is shrunk by the size of the 
// status bar (not shrunk if the status bar is hidden). Also this always has
// a meaninful top-left coord rather than the 0,0 of GetClientRect.
oAPI bool oWinGetClientRect(HWND _hWnd, RECT* _pRect);

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

// Returns the shape of the specified window, i.e. all information that affects
// the size and position of the window.
oAPI oGUI_WINDOW_SHAPE_DESC oWinGetShape(HWND _hWnd);

// Sets the shape of the specified window.
oAPI bool oWinSetShape(HWND _hWnd, const oGUI_WINDOW_SHAPE_DESC& _Shape);

// There's a known issue that a simple ShowWindow doesn't always work on some 
// minimized apps. The WAR seems to be to set focus to anything else, then try 
// to restore the app. This is exposed to be used on any HWND because it is a 
// real bug/poor behavior in the OS, and is used in oWinSetState to properly
// handle focus.
oAPI bool oWinRestore(HWND _hWnd);

// Play a windows animation from and to the specified RECTs. This respects user 
// settings as to whether window animated transitions are on. This does not 
// actually move the window, just plays the tween frames from some other 
// instantaneous move.
oAPI bool oWinAnimate(HWND _hWnd, const RECT& _From, const RECT& _To);

// _____________________________________________________________________________
// String Modifiers. Alter a string based on the shape of a window

// Returns the number of characters used if the string were truncated. This 
// includes the ellipse.
oAPI size_t oWinGetTruncatedLength(HWND _hWnd, const char* _StrSource);

// Adds ellipse to the left of the string and shortens the string to fit in the
// specified _hWnd's client rect. This will preserve as much of the right  of 
// the string as possible.
oAPI char* oWinTruncateLeft(char* _StrDestination, size_t _SizeofStrDestination, HWND _hWnd, const char* _StrSource);
template<size_t size> char* oWinTruncateLeft(char (&_StrDestination)[size], HWND _hWnd, const char* _StrSource) { return oWinTruncateLeft(_StrDestination, size, _hWnd, _StrSource); }
template<size_t capacity> char* oWinTruncateLeft(ouro::fixed_string<char, capacity>& _StrDestination, HWND _hWnd, const char* _StrSource) { return oWinTruncateLeft(_StrDestination, _StrDestination.capacity(), _hWnd, _StrSource); }

// Fills _StrDestination with a copy of the path that has ellipse placed in the 
// middle of the path according to windows rules sized to the rectangle and 
// context used by the specified HWND for drawing.
oAPI char* oWinTruncatePath(char* _StrDestination, size_t _SizeofStrDestination, HWND _hWnd, const char* _Path);
template<size_t size> char* oWinTruncatePath(char (&_StrDestination)[size], HWND _hWnd, const char* _Path) { return oWinTruncatePath(_StrDestination, size, _hWnd, _Path); }
template<size_t capacity> char* oWinTruncatePath(ouro::fixed_string<char, capacity>& _StrDestination, HWND _hWnd, const char* _Path) { return oWinTruncatePath(_StrDestination, _StrDestination.capacity(), _hWnd, _Path); }

// _____________________________________________________________________________
// UI Control Creation, Lifetime and Message Processing

// NOTE: Tabs send WM_NOTIFY messages. The passed NMHDR::code includes:
// TCN_KEYDOWN, TCN_SELCHANGE, TCN_SELCHANGING, TCN_GETOBJECT, TCN_FOCUSCHANGE

// When finished with this HWND, use DestroyWindow() to free it, though since
// all controls must have a parent and on Windows the parent deletes all its
// children, in many cases the lifetime of the HWND need not be managed, or even
// retained.
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
inline bool oWinControlExists(HWND _hControl) { return oWinExists(_hControl); }

// Returns true if the specified control can receive events
// Valid for: All
inline bool oWinControlIsEnabled(HWND _hControl) { return oWinIsEnabled(_hControl); }

// Set whether or not the specified control can receive events
// Valid for: All
inline void oWinControlEnable(HWND _hControl, bool _Enabled = true) { oWinEnable(_hControl, _Enabled); }

// Returns true if the specified control is marked as visible
oAPI bool oWinControlIsVisible(HWND _hControl);

// Sets whether the control can be seen or not. If not visible, the control 
// becomes non-interactive (tabstops, etc.)
oAPI bool oWinControlSetVisible(HWND _hControl, bool _Visible = true);

// _____________________________________________________________________________
// UI Control SubItems

// Returns the number of subitems associated with the specified _hWnd. If the 
// type of HWND doesn't have subitems, then the return value is zero with no
// error reported.
// Valid for: All
oAPI int oWinControlGetNumSubItems(HWND _hControl);

// Removes all subitems from the specified _hWnd. If the type doesn't support
// subitems, no error is reported.
// Valid for: All
oAPI bool oWinControlClearSubItems(HWND _hControl);

// Inserts before the specified subitem index. If _SubItemIndex is oInvalid (-1) 
// then the item is appended. This returns the index of the item set, or 
// oInvalid on failure.
// Valid for: ComboBox, ComboTextbox, Tab
oAPI int oWinControlInsertSubItem(HWND _hControl, const char* _SubItemText, int _SubItemIndex);

// Deletes the nth subitem
// Valid for: ComboBox, ComboTextbox, Tab
oAPI bool oWinControlDeleteSubItem(HWND _hControl, const char* _SubItemText, int _SubItemIndex);

// Adds all items in the specified delimited list in order.
// Valid for: ComboBox, ComboTextbox, Tab
oAPI bool oWinControlAddSubItems(HWND _hControl, const char* _DelimitedString, char _Delimiter = '|');

// Returns the index of the first subitem to match the specified text, or 
// oInvalid if not found (or other error).
// Valid For: ComboBox, ComboTextbox, Tab
oAPI int oWinControlFindSubItem(HWND _hControl, const char* _SubItemText);

// Sets the specified index as the selected item/text for the combobox
// Valid for: ComboBox, ComboTextbox, Tab
oAPI bool oWinControlSelectSubItem(HWND _hControl, int _Index);

// Sets the specified text as the selected item/text for the combobox (if it 
// exists)
// Valid for: ComboBox, ComboTextbox, Tab
inline bool oWinControlSelectSubItem(HWND _hControl, const char* _SubItemText) { return oWinControlSelectSubItem(_hControl, oWinControlFindSubItem(_hControl, _SubItemText)); }

// Selects n from the currently selected, properly wrapping around either end
// Valid for: ComboBox, ComboTextBox, Tab
oAPI bool oWinControlSelectSubItemRelative(HWND _hControl, int _Offset);

// Returns the index of the currently selected item, or -1 if none selected
// Valid for: ComboBox, ComboTextBox, Tab
oAPI int oWinControlGetSelectedSubItem(HWND _hControl);

// _____________________________________________________________________________
// Control Accessors/Mutators

// This will probably return oWINDOW_CONTROL_UNKNOWN for any control not created
// with oWinControlCreate().
// Valid for: All
oAPI oGUI_CONTROL_TYPE oWinControlGetType(HWND _hControl);

// Returns the size returned if int2(oDEFAULT, oDEFAULT) were passed as the size
// value to oWinControlCreate().
// Valid for: All
oAPI int2 oWinControlGetInitialSize(oGUI_CONTROL_TYPE _Type, const int2& _Size);

// Returns true if the specified _hWnd has the WS_TABSTOP style and is also 
// enabled and visible, so all criteria from tabbing and using the control must 
// be met for this to return true and thus the answer can change if the state of 
// the control changes. This should only be called from the same thread on which 
// the window was created.
// Valid for: All
oAPI bool oWinControlIsTabStop(HWND _hControl);

// Returns the ID for a control given its HWND
// Valid for: All
oAPI int oWinControlGetID(HWND _hControl);

// Returns the HWND for a control given its parent and ID
// Valid for: All
oAPI HWND oWinControlGetFromID(HWND _hParent, unsigned short _ID);

// Returns the bounding rectangle of the specified _hControl in terms of its
// parent's client area.
// Valid for: All
oAPI RECT oWinControlGetRect(HWND _hControl);

// Returns the currently set font.
// Valid for: All
inline HFONT oWinControlGetFont(HWND _hControl) { return oWinGetFont(_hControl); }

// If nullptr the DEFAULT_GUI_FONT is set
// Valid for: All
inline bool oWinControlSetFont(HWND _hControl, HFONT _hFont = nullptr) { return oWinSetFont(_hControl, _hFont); }

// Either sets the text for simple controls, or the nth text item in list-like
// controls such as tabs, combobox, and listboxes. If _SubItemIndex is not found
// this will fail. If _SubItemIndex is oInvalid, then the main text of the 
// control is set. If the control doesn't have the concept of main text, then 
// this will fail.
// Valid for: All
inline bool oWinControlSetText(HWND _hControl, const char* _Text, int _SubItemIndex = oInvalid) { return oWinSetText(_hControl, _Text, _SubItemIndex); }

// Returns _StrDestination on success, otherwise nullptr.
inline char* oWinControlGetText(char* _StrDestination, size_t _SizeofStrDestination, HWND _hControl, int _SubItemIndex = oInvalid) { return oWinGetText(_StrDestination, _SizeofStrDestination, _hControl, _SubItemIndex); }
template<size_t size> char* oWinControlGetText(char (&_StrDestination)[size], HWND _hControl, int _SubItemIndex = oInvalid) { return oWinGetText(_StrDestination, _hControl, _SubItemIndex); }
template<size_t capacity> char* oWinControlGetText(ouro::fixed_string<char, capacity>& _StrDestination, HWND _hControl, int _SubItemIndex = oInvalid) { return oWinGetText(_StrDestination, _hControl, _SubItemIndex); }

// Returns _StrDestination on success, otherwise nullptr. _StrDestination is 
// filled with the portion of the specified _hControl that is considered 
// selected by the UI.
// Valid for: ComboTextbox, TextBox, FloatBox, FloatBoxSpinner
oAPI char* oWinControlGetSelectedText(char* _StrDestination, size_t _SizeofStrDestination, HWND _hControl);
template<size_t size> char* oWinControlGetSelectedText(char (&_StrDestination)[size], HWND _hControl) { return oWinControlGetSelectedText(_StrDestination, size, _hWnd); }
template<size_t capacity> char* oWinControlGetSelectedText(ouro::fixed_string<char, capacity>& _StrDestination, HWND _hControl) { return oWinControlGetSelectedText(_StrDestination, _StrDestination.capacity(), _hWnd); }

// Applies a selection highlight to the specified _hWnd according to the 
// specified range.
// Valid for: ComboTextBox, TextBox, FloatBox, FloatBoxSpinner, LabelSelectable,
//            SliderSelectable
oAPI bool oWinControlSelect(HWND _hControl, int _Start, int _Length);

// Sets/Gets a textbox-like object by value. For verified input boxes like 
// FloatBox or FloatBoxSpinner, this bypasses first-pass error checking and 
// ensures that well-formatted data passes the control's second-pass error 
// checking. Using oWinSetText on these without the first-pass error checking
// would quietly noop.
// Valid for: All the same as oWinSetText
oAPI bool oWinControlSetValue(HWND _hControl, float _Value);

// This interprets whatever text is specified as a float (non-scientific form).
// If the string could not be interpreted as a float then this returns a NaN.
// Use isnan() from oMath.h to test.
// Valid for: All
oAPI float oWinControlGetFloat(HWND _hControl);

// Associates the specified _hIcon with the specified _hControl
// Valid for: Icon
oAPI bool oWinControlSetIcon(HWND _hControl, HICON _hIcon, int _SubItemIndex = oInvalid);

// Get's the icon associated with the specified _hControl
// Valid for: Icon
oAPI HICON oWinControlGetIcon(HWND _hControl, int _SubItemIndex = oInvalid);

// Returns true if the specified _hControl's checked state is true. This returns 
// false for any other tri-state state or if the specified _hControl is not a 
// type that can be checked. If legitimately not checked, oErrorGetLast will 
// return 0.
// Valid for: CheckBox, RadioButton
oAPI bool oWinControlIsChecked(HWND _hControl);

// Sets the specified _hControl as checked or not. If not a valid type, this 
// will return false.
// Valid for: CheckBox, RadioButton
oAPI bool oWinControlSetChecked(HWND _hControl, bool _Checked = true);

// Sets the specified _hControl's min and max range. If not a valid type, this 
// will return false. If _Min == _Max, then the control is put in the "unknown
// range" state. For ProgressBar, this means it shows the "marquee", a scrolling 
// progress bar that indicates something is happening, but it is not known how 
// much of that something is complete.
// Valid for: Slider, SliderSelectable, ProgressBar
oAPI bool oWinControlSetRange(HWND _hControl, int _Min, int _Max);

// Gets the specified _hControl's min and max range. If not a valid type, this 
// will return false.
// Valid for: Slider, SliderSelectable, ProgressBar
oAPI bool oWinControlGetRange(HWND _hControl, int* _pMin, int* _pMax);

// Sets the specified _hControl's position within its min and max range. If 
// is outside the range, then it is clamped. If the _hControl is not a valid 
// type, this will return false.
// Valid for: Slider, SliderSelectable, ProgressBar
oAPI bool oWinControlSetRangePosition(HWND _hControl, int _Position, bool _bNotify = true);

// Gets the specified _hControl's position within its min and max range. If 
// is outside the range, then it is clamped. If the _hControl is not a valid 
// type, this will return false.
// Valid for: Slider, SliderSelectable, ProgressBar
oAPI int oWinControlGetRangePosition(HWND _hControl);

// Sets a tick at the specified point on a trackbar
// Valid for: SliderWithTicks
oAPI bool oWinControlSetTick(HWND _hControl, int _Position);

// Removes all ticks on a trackbar
// Valid for: SliderWithTicks
oAPI bool oWinControlClearTicks(HWND _hControl);

// Sets the control to be in a state indicative to the user as in error.
// Valid for: ProgressBar, ProgressBarUnknown
oAPI bool oWinControlSetErrorState(HWND _hControl, bool _InErrorState = true);

// Returns true if the control is currently indicating an error state to the 
// user. If the control is one valid for this function, the last error is set
// to 0 to differentiate between a valid false and a false because an improper 
// type was specified.
// Valid for: ProgressBar
oAPI bool oWinControlGetErrorState(HWND _hControl);

// Utility code that will readjust a range position that is outside the 
// control's selected region.
// Valid for: SliderSelectable
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
	{ _uMsg = oWinTranslateMessage(_hWnd, _uMsg); \
		if (oWinWindowProc(_hWnd, _uMsg, _wParam, _lParam) >= 0) return 0; \
		_ClassName* pThis = oWinGetThis<_ClassName>(_hWnd); \
		if (!pThis) return DefWindowProc(_hWnd, _uMsg, _wParam, _lParam); \
		return pThis->WndProc(_hWnd, _uMsg, _wParam, _lParam); \
	}

#endif
