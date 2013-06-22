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
// An abstraction for an operating system's window concept. This is fit for 
// containing child controls and doing other typical event-based Windowing. Do
// not use this for GPU-accelerated operations, see the oGPUWindow derivative of 
// this class for that type of functionality. Look to TESTWindowInWindow for an
// example of how to embed a GPU-accelerated window in an oWindow.
#pragma once
#ifndef oWindow_h
#define oWindow_h

#include <oConcurrency/oConcurrency.h>
#include <oBasis/oGUI.h>
#include <oBasis/oInterface.h>
#include <oBasis/oRef.h>
#include <oStd/oStdFuture.h>

interface oImage;

// {0AA1837F-CAFD-427B-BF50-307D100974EE}
oDEFINE_GUID_I(oWindow, 0xaa1837f, 0xcafd, 0x427b, 0xbf, 0x50, 0x30, 0x7d, 0x10, 0x9, 0x74, 0xee);
interface oWindow : oInterface
{
	// QueryInterface will return valid handles for the following types:
	// oGUI_WINDOW, oGUI_MENU, oGUI_STATUSBAR
	// Remember, these need not be freed in any way because they are 
	// non-refcounted.

	virtual oGUI_WINDOW GetNativeHandle() const threadsafe = 0;

	// Returns true while the windows thread is running, false
	// when the windows thread is exiting.
	virtual bool IsOpen() const threadsafe = 0;

	// Sleeps the calling thread until IsOpen returns false.
	virtual bool WaitUntilClosed(unsigned int _TimeoutMS = oInfiniteWait) const threadsafe = 0;

	// Blocks (might spin, not always sleep) until the window is fully opaque and
	// shown. This will wait forever if the window is hidden. The purpose of this
	// API is to be more authoritative on platforms where showing a window causes
	// a fade-in effect over a small amount of time. Doing a snapshot
	virtual bool WaitUntilOpaque(unsigned int _TimeoutMS = oInfiniteWait) const threadsafe = 0;

	// Close the window. The oGUI_CLOSING event will be triggered and if a hook 
	// denies the closing request, this function will return false. If the close
	// goes through, then IsOpen() will return false, but oGUI_CLOSED will not 
	// occur until the last reference to this window is gone and destruction 
	// begins.
	virtual bool Close() threadsafe = 0;

	// Returns true if called from the window message pump thread
	virtual bool IsWindowThread() const threadsafe = 0;

	// This could be done through the Map/Umap API, but often it is more 
	// convenient to immediately get/set these values.
	virtual bool HasFocus() const threadsafe = 0;
	virtual void SetFocus() threadsafe = 0;

	// For read-only querying of the window's desc, prefer this over Map/Unmap
	virtual void GetDesc(oGUI_WINDOW_DESC* _pDesc) const threadsafe = 0;

	// Maps a description of the window and prevents other threads from altering
	// the state while it is modified through external events such as user 
	// interaction or internal updates. Unlike GetDesc, this points directly to
	// internal memory used to update the window when Unmap is called so there is 
	// no extra copying of data.
	virtual bool Map(oGUI_WINDOW_DESC** _ppDesc) threadsafe = 0;
	virtual void Unmap() threadsafe = 0;

	// Gets/sets the cursor when it is over the current window.
	virtual void SetDesc(const oGUI_WINDOW_CURSOR_DESC& _CursorDesc) threadsafe = 0;
	virtual void GetDesc(oGUI_WINDOW_CURSOR_DESC* _pCursorDesc) const threadsafe = 0;

	// Get and set the title at the top of the OS decoration
	virtual char* GetTitle(char* _StrDestination, size_t _SizeofStrDestination) const threadsafe = 0;
	virtual void SetTitleV(const char* _Format, va_list _Args) threadsafe = 0;

	virtual char* GetStatusText(char* _StrDestination, size_t _SizeofStrDestination, int _StatusSectionIndex) const threadsafe = 0;
	virtual void SetStatusText(int _StatusSectionIndex, const char* _Format, va_list _Args) threadsafe = 0;

	// Convenient typing for string-based API
	inline void SetTitle(const char* _Format, ...) threadsafe { va_list args; va_start(args, _Format); SetTitleV(_Format, args); va_end(args); }
	template<size_t size> char* GetTitle(char (&_StrDestination)[size]) const threadsafe { return GetTitle(_StrDestination, size); }
	template<size_t capacity> char* GetTitle(oStd::fixed_string<char, capacity>& _StrDestination) const threadsafe { return GetTitle(_StrDestination, _StrDestination.capacity()); }

	inline void SetStatusText(int _StatusSectionIndex, const char* _Format, ...) threadsafe { va_list args; va_start(args, _Format); SetStatusText(_StatusSectionIndex, _Format, args); va_end(args); }
	template<size_t size> char* GetStatusText(char (&_StrDestination)[size], int _StatusSectionIndex) const threadsafe { return GetStatusText(_StrDestination, size, _StatusSectionIndex); }
	template<size_t capacity> char* GetStatusText(oStd::fixed_string<char, capacity>& _StrDestination, int _StatusSectionIndex) const threadsafe { return GetStatusText(_StrDestination, _StrDestination.capacity(), _StatusSectionIndex); }

	// Sets all hotkeys (accelerators) that will be recognized. Set nullptr,0 to
	// remove all hotkeys.
	virtual void SetHotKeys(const oGUI_HOTKEY_DESC_NO_CTOR* _pHotKeys, size_t _NumHotKeys) threadsafe = 0;
	template<size_t size> void SetHotKeys(const oGUI_HOTKEY_DESC_NO_CTOR (&_pHotKeys)[size]) threadsafe { SetHotKeys(_pHotKeys, size); }
	template<size_t size> void SetHotKeys(const oGUI_HOTKEY_DESC (&_pHotKeys)[size]) threadsafe { SetHotKeys(_pHotKeys, size); }

	virtual int GetHotKeys(oGUI_HOTKEY_DESC_NO_CTOR* _pHotKeys, size_t _MaxNumHotKeys) const threadsafe = 0;
	template<size_t size> int GetHotKeys(oGUI_HOTKEY_DESC_NO_CTOR (&_pHotKeys)[size]) threadsafe { return GetHotKeys(_pHotKeys, size); }
	template<size_t size> int GetHotKeys(oGUI_HOTKEY_DESC (&_pHotKeys)[size]) threadsafe { return GetHotKeys(_pHotKeys, size); }

	// Create a new image with either the client or whole-window area captured
	// Remember this queues into the windows thread queue, so ensure the app is
	// in an expected state for the image to be captured. _Frame doesn't mean much
	// for the base implementation of oWindow, but is important in most more 
	// complex derivatives. It's intent is that the snapshot occurs on an explicit
	// frame. If oInvalid is specified then the snapshot occurs as soon as 
	// possible.
	virtual oStd::future<oRef<oImage>> CreateSnapshot(int _Frame = oInvalid, bool _IncludeBorder = false) const threadsafe = 0;

	// Appends a broadcast of an event as if the event came from user input. This
	// will block for events that return a value such as 
	virtual void Trigger(const oGUI_ACTION_DESC& _Action) threadsafe = 0;

	// Append a task to the window's message queue. It will be executed on the 
	// windows thread in order.
	virtual void Dispatch(const oTASK& _Task) threadsafe = 0;
	oDEFINE_CALLABLE_WRAPPERS(Dispatch, threadsafe, Dispatch);

	// Trigger an oGUI_TIMER event with the context after the specified number of 
	// milliseconds from when the function was called.
	virtual void SetTimer(uintptr_t _Context, unsigned int _RelativeTimeMS) threadsafe = 0;

	virtual int HookActions(const oGUI_ACTION_HOOK& _Hook) threadsafe = 0;
	virtual void UnhookActions(int _ActionHookID) threadsafe = 0;

	virtual int HookEvents(const oGUI_EVENT_HOOK& _Hook) threadsafe = 0;
	virtual void UnhookEvents(int _EventHookID) threadsafe = 0;
};

struct oWINDOW_INIT
{
	oWINDOW_INIT()
		: WindowTitle("")
		, InitialAlignment(oGUI_ALIGNMENT_MIDDLE_CENTER)
	{}

	const char* WindowTitle;
	oGUI_ALIGNMENT InitialAlignment;

	// NOTE: The oGUI_CREATING event gets fired during construction, so if that 
	// event is to be hooked it needs to be passed and hooked up during 
	// construction.
	oGUI_EVENT_HOOK EventHook;
	oGUI_ACTION_HOOK ActionHook;
	oGUI_WINDOW_DESC WinDesc;
};

bool oWindowCreate(const oWINDOW_INIT& _Init, threadsafe oWindow** _ppWindow);

#endif
