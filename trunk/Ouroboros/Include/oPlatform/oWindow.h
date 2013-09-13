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
#ifndef oWindow_h
#define oWindow_h

#include <oBasis/oGUI.h>
#include <oConcurrency/oConcurrency.h>
#include <oStd/oStdFuture.h>

interface oImage;

// {C590552A-EDCC-434B-AF17-7871FC00B18D}
oDEFINE_GUID_I(oWindow, 0xc590552a, 0xedcc, 0x434b, 0xaf, 0x17, 0x78, 0x71, 0xfc, 0x0, 0xb1, 0x8d);
interface oWindow : oInterface
{
	// Note: All mutators to oWindow queue their changes and must be flushed using 
	// FlushMessages() before accessors are valid. This is true even if mutators 
	// are called on the oWindow's thread. For this reason, all accessors are not 
	// labeled threadsafe.

	
	// Basic API

	virtual oGUI_WINDOW GetNativeHandle() const threadsafe = 0;
	
	// Returns an index fit for use with oDisplay API based on the center of the
	// window's client area.
	virtual int GetDisplayIndex() const = 0;
	virtual bool IsWindowThread() const threadsafe = 0;


	// Window Shape API (style, state, size, position)

	virtual void SetShape(const oGUI_WINDOW_SHAPE_DESC& _Shape) threadsafe = 0;
	virtual oGUI_WINDOW_SHAPE_DESC GetShape() const = 0;
	inline void SetState(oGUI_WINDOW_STATE _State) threadsafe { oGUI_WINDOW_SHAPE_DESC s; s.State = _State; SetShape(s); }
	inline oGUI_WINDOW_STATE GetState() const { oGUI_WINDOW_SHAPE_DESC s = GetShape(); return s.State; }
	inline void SetStyle(oGUI_WINDOW_STYLE _Style) threadsafe { oGUI_WINDOW_SHAPE_DESC s; s.Style = _Style; SetShape(s); }
	inline oGUI_WINDOW_STYLE GetStyle() const { oGUI_WINDOW_SHAPE_DESC s = GetShape(); return s.Style; }
	inline void Show(oGUI_WINDOW_STATE _State = oGUI_WINDOW_RESTORED) threadsafe { oGUI_WINDOW_SHAPE_DESC s; s.State = _State; SetShape(s); }
	inline void Hide() threadsafe { Show(oGUI_WINDOW_HIDDEN); }
	inline void Minimize() threadsafe { Show(oGUI_WINDOW_MINIMIZED); }
	inline void Maximize() threadsafe { Show(oGUI_WINDOW_MAXIMIZED); }
	inline void Restore() threadsafe { Show(oGUI_WINDOW_RESTORED); }
	inline void SetClientPosition(const int2& _ClientPosition) threadsafe { oGUI_WINDOW_SHAPE_DESC s; s.ClientPosition = _ClientPosition; SetShape(s); }
	inline int2 GetClientPosition() const { oGUI_WINDOW_SHAPE_DESC s = GetShape(); return s.ClientPosition; }
	inline void SetClientSize(const int2& _ClientSize) threadsafe { oGUI_WINDOW_SHAPE_DESC s; s.ClientSize = _ClientSize; SetShape(s); }
	inline int2 GetClientSize() const { oGUI_WINDOW_SHAPE_DESC s = GetShape(); return s.ClientSize; }


	// Border/Decoration API

	virtual void SetIcon(oGUI_ICON _hIcon) threadsafe = 0;
	virtual oGUI_ICON GetIcon() const = 0;

	virtual void SetUserCursor(oGUI_CURSOR _hCursor) threadsafe = 0;
	virtual oGUI_CURSOR GetUserCursor() const = 0;

	// If user-cursor is specified, then the cursor specified with SetCursor is 
	// used.
	virtual void SetClientCursorState(oGUI_CURSOR_STATE _State) threadsafe = 0;
	virtual oGUI_CURSOR_STATE GetClientCursorState() const = 0;

	virtual void SetTitleV(const char* _Format, va_list _Args) threadsafe = 0;
	virtual char* GetTitle(char* _StrDestination, size_t _SizeofStrDestination) const = 0;

	inline void SetTitle(const char* _Format, ...) threadsafe { va_list args; va_start(args, _Format); SetTitleV(_Format, args); va_end(args); }
	template<size_t size> char* GetTitle(char (&_StrDestination)[size]) const { return GetTitle(_StrDestination, size); }
	template<size_t capacity> char* GetTitle(oStd::fixed_string<char, capacity>& _StrDestination) const { return GetTitle(_StrDestination, _StrDestination.capacity()); }

	// For widths, if the last width is -1, it will be interpreted as "take up 
	// rest of width of window".
	virtual void SetNumStatusSections(const int* _pStatusSectionWidths, size_t _NumStatusSections) threadsafe = 0;
	virtual int GetNumStatusSections(int* _pStatusSectionWidths = nullptr, size_t _MaxNumStatusSectionWidths = 0) const = 0;
	template<size_t size> void SetNumStatusSections(const int (&_pStatusSectionWidths)[size]) threadsafe { SetNumStatusSections(_pStatusSectionWidths, size); }

	virtual void SetStatusTextV(int _StatusSectionIndex, const char* _Format, va_list _Args) threadsafe = 0;
	virtual char* GetStatusText(char* _StrDestination, size_t _SizeofStrDestination, int _StatusSectionIndex) const = 0;

	inline void SetStatusText(int _StatusSectionIndex, const char* _Format, ...) threadsafe { va_list args; va_start(args, _Format); SetStatusTextV(_StatusSectionIndex, _Format, args); va_end(args); }
	template<size_t size> char* GetStatusText(char (&_StrDestination)[size], int _StatusSectionIndex) const threadsafe { return GetStatusText(_StrDestination, size, _StatusSectionIndex); }
	template<size_t capacity> char* GetStatusText(oStd::fixed_string<char, capacity>& _StrDestination, int _StatusSectionIndex) const { return GetStatusText(_StrDestination, _StrDestination.capacity(), _StatusSectionIndex); }

	virtual void SetStatusIcon(int _StatusSectionIndex, oGUI_ICON _hIcon) threadsafe = 0;
	virtual oGUI_ICON GetStatusIcon(int _StatusSectionIndex) const = 0;


	// Draw Order/Dependency API

	// Overrides Shape and STYLE to be RESTORED and NONE. If this exclusive 
	// fullscreen is true, then this throws an exception. A parent is 
	// automatically an owner of this window. If there is already an owner, this 
	// will throw an exception.
	virtual void SetParent(oWindow* _pParent) threadsafe = 0;
	virtual oWindow* GetParent() const = 0;

	// An owner is more like a sibling who does all the work. Use this for the 
	// association between dialog boxes and the windows on top of which they are
	// applied. If there is a parent, this will throw an exception.
	virtual void SetOwner(oWindow* _pOwner) threadsafe = 0;
	virtual oWindow* GetOwner() const = 0;

	virtual void SetSortOrder(oGUI_WINDOW_SORT_ORDER _SortOrder) threadsafe = 0;
	virtual oGUI_WINDOW_SORT_ORDER GetSortOrder() const = 0;

	virtual void SetFocus(bool _Focus = true) threadsafe = 0;
	virtual bool HasFocus() const = 0;


	// Extended Input API

	// If true, platform oTRACEs of every event and action will be enabled. This
	// is false by default.
	virtual void SetDebug(bool _Debug = true) threadsafe = 0;
	virtual bool GetDebug() const = 0;

	// Many touch screen drivers emulated mouse events in a way that is not 
	// consistent with regular mouse behavior, so if that affects application 
	// logic, here's a switch. This is false by default.
	virtual void SetAllowTouchActions(bool _Allow = true) threadsafe = 0;
	virtual bool GetAllowTouchActions() const = 0;

	// If set to true, any mouse action in the client area will move the window.
	// This is useful for borderless windows like splash screens. This is false by 
	// default.
	virtual void SetClientDragToMove(bool _DragMoves = true) threadsafe = 0;
	virtual bool GetClientDragToMove() const = 0;

	// If set to true, Alt-F4 will trigger an oGUI_CLOSING event. This is true by 
	// default.
	virtual void SetAltF4Closes(bool _AltF4Closes = true) threadsafe = 0;
	virtual bool GetAltF4Closes() const = 0;

	virtual void SetEnabled(bool _Enabled) threadsafe = 0;
	virtual bool GetEnabled() const = 0;

	virtual void SetCapture(bool _Capture) threadsafe = 0;
	virtual bool HasCapture() const = 0;

	virtual void SetHotKeys(const oGUI_HOTKEY_DESC_NO_CTOR* _pHotKeys, size_t _NumHotKeys) threadsafe = 0;
	template<size_t size> void SetHotKeys(const oGUI_HOTKEY_DESC_NO_CTOR (&_pHotKeys)[size]) threadsafe { SetHotKeys(_pHotKeys, size); }
	template<size_t size> void SetHotKeys(const oGUI_HOTKEY_DESC (&_pHotKeys)[size]) threadsafe { SetHotKeys(_pHotKeys, size); }

	virtual int GetHotKeys(oGUI_HOTKEY_DESC_NO_CTOR* _pHotKeys, size_t _MaxNumHotKeys) const = 0;
	template<size_t size> int GetHotKeys(oGUI_HOTKEY_DESC_NO_CTOR (&_pHotKeys)[size]) threadsafe { return GetHotKeys(_pHotKeys, size); }
	template<size_t size> int GetHotKeys(oGUI_HOTKEY_DESC (&_pHotKeys)[size]) threadsafe { return GetHotKeys(_pHotKeys, size); }


	// Observer API

	virtual int HookActions(const oGUI_ACTION_HOOK& _Hook) threadsafe = 0;
	virtual void UnhookActions(int _ActionHookID) threadsafe = 0;

	virtual int HookEvents(const oGUI_EVENT_HOOK& _Hook) threadsafe = 0;
	virtual void UnhookEvents(int _EventHookID) threadsafe = 0;

	
	// Execution API

	// Appends a broadcast of an action as if it came from user input.
	virtual void Trigger(const oGUI_ACTION_DESC& _Action) threadsafe = 0;

	// Post an event that is specified by the user here.
	virtual void Post(int _CustomEventCode, uintptr_t _Context) threadsafe = 0;

	// Posts the specified task in the window's message queue and executes it in 
	// order with other events. This is useful for wrapping platform-specific 
	// window/control calls.
	virtual void Dispatch(const oTASK& _Task) threadsafe = 0;
	oDEFINE_CALLABLE_WRAPPERS(Dispatch, threadsafe, Dispatch);

	// Schedules an oImage to be generated from the oWindow. In the simple case,
	// _Frame is not used and the front buffer is captured. Due to platform rules
	// this may involve bringing the specified window to focus.
	virtual oStd::future<oStd::ref<oImage>> CreateSnapshot(int _Frame = oInvalid, bool _IncludeBorder = false) threadsafe const = 0;

	// Causes an oGUI_TIMER event to occur with the specified context after the 
	// specified time. This will be called every specified millisections until 
	// StopTimer is called with the same context.
	virtual void SetTimer(uintptr_t _Context, unsigned int _RelativeTimeMS) threadsafe = 0;
	virtual void StopTimer(uintptr_t _Context) threadsafe = 0;

	// Flushes the window's message queue. This should be called in a loop on the
	// same thread where the window was created. If _WaitForNext is false, this 
	// will loop until the messasge queue is empty and exit. If _WaitForNext is 
	// true this will block and process messages until Quit() is called.
	virtual void FlushMessages(bool _WaitForNext = false) = 0;

	// Breaks out of a blocking FlushMessages routine.
	virtual void Quit() threadsafe = 0;
};

struct oWINDOW_INIT
{
	oWINDOW_INIT()
		: Title("")
		, hIcon(nullptr)
		, hCursor(nullptr)
		, ClientCursorState(oGUI_CURSOR_ARROW)
		, SortOrder(oGUI_WINDOW_SORTED)
		, Debug(false)
		, AllowTouch(false)
		, ClientDragToMove(false)
		, AltF4Closes(false)
	{}

	const char* Title;
	oGUI_ICON hIcon;
	oGUI_CURSOR hCursor;
	oGUI_CURSOR_STATE ClientCursorState;
	oGUI_WINDOW_SORT_ORDER SortOrder;
	bool Debug;
	bool AllowTouch;
	bool ClientDragToMove;
	bool AltF4Closes;

	// NOTE: The oGUI_CREATING event gets fired during construction, so if that 
	// event is to be hooked it needs to be passed and hooked up during 
	// construction.
	oGUI_EVENT_HOOK EventHook;
	oGUI_ACTION_HOOK ActionHook;
	oGUI_WINDOW_SHAPE_DESC Shape;
};

bool oWindowCreate(const oWINDOW_INIT& _Init, oWindow** _ppWindow);

#endif
