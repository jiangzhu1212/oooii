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
// An OS Window tailored to the needs of simple image 
// display, such as from a video feed or 3D rendering.
#pragma once
#ifndef oWindow_h
#define oWindow_h

#include <oBasis/oColor.h>
#include <oBasis/oInterface.h>
#include <oBasis/oMathTypes.h>
#include <oBasis/oSurface.h>

#if defined(_WIN32) || defined (_WIN64)
	// To minimize flicker on Windows, offscreen HDCs are used in GDI (legacy)
	// rendering. To be able to draw widgets using oWindow hooks, access to these
	// HDCs must be exposed separately from GetDC(). Use QueryInterface with one
	// of these GUIDs to obtain an HDC. The oHDC types are specified only to 
	// differentiate the type for the GUID query, both are regular HDCs. Since 
	// HDCs aren't ref-counted, nothing is done to increment internally. Client
	// code should take no delete or release action with the pointer used here.
	// HDCs are reallocated when RESIZING and RESIZED events occur, so client code
	// should not retain a cached reference but rather resolve the HDC explicitly 
	// at draw time.
	oDECLARE_HANDLE(oHDC);
	oDECLARE_HANDLE(oHDCAA);
	const oGUID& oGetGUID(threadsafe const oHDC* threadsafe const* = 0);
	const oGUID& oGetGUID(threadsafe const oHDCAA* threadsafe const* = 0);
#endif

interface oWindow : oInterface
{
	enum STATE
	{
		HIDDEN, // Window is invisible, but exists
		RESTORED, // Window is in normal sub-screen-size mode
		MINIMIZED, // Window is reduces to iconic or taskbar size
		MAXIMIZED, // Window takes up the entire screen
		FULLSCREEN, // Window takes exclusive access to screen, and will not have a title bar, border, etc regardless of its style, position or other parameters
	};

	enum CURSOR_STATE
	{
		NONE,
		ARROW,
		HAND,
		HELP,
		NOTALLOWED,
		WAIT_FOREGROUND,
		WAIT_BACKGROUND,
		USER,
	};

	enum STYLE
	{
		BORDERLESS, // There is no OS decoration of the client area
		FIXED, // There is OS decoration but no user resize is allowed
		DIALOG, // There is OS decoration, but closing the window is not allowed
		SIZEABLE, // OS decoration and user can resize window
	};

	// Draw mode exposes the underlying platform API used for widget rendering.
	// Based on this, OnDraw()'s void* _pContext can be C-cast to different types
	// as specified below. Do not hold onto this object because it is transient as 
	// the window moves and resizes.
	enum DRAW_MODE
	{
		USE_DEFAULT, // This will make the decision to use the most performant API for the current system
		USE_GDI, // HDC hDC = (HDC)_pContext
		USE_D2D, // ID2D1RenderTarget* pD2DRT = (ID2D1RenderTarget*)_pContext
		USE_DX11, // ID3D11RenderTargetView* pD3D11RTV = (ID3D11RenderTargetView*)_pContext
	};

	enum EVENT
	{
		RESIZING,
		RESIZED,
		CLOSING,
		CLOSED,
		DRAW_BACKBUFFER,
		DRAW_UIAA,
		DRAW_UI,
		DRAW_FRONTBUFFER,
	};

	enum ALIGNMENT
	{
		TOP_LEFT,
		TOP_CENTER,
		TOP_RIGHT,
		MIDDLE_LEFT,
		MIDDLE_CENTER,
		MIDDLE_RIGHT,
		BOTTOM_LEFT,
		BOTTOM_CENTER,
		BOTTOM_RIGHT,
	};

	struct DESC
	{
		DESC()
			: ClientPosition(oDEFAULT, oDEFAULT)
			, ClientSize(oDEFAULT, oDEFAULT)
			, FullscreenSize(oDEFAULT, oDEFAULT)
			, FullscreenRefreshRate(oDEFAULT)
			, BackgroundSleepMS(200)
			, State(RESTORED)
			, Style(SIZEABLE)
			, CursorState(ARROW)
			, Enabled(true)
			, HasFocus(true)
			, AlwaysOnTop(false)
			, FullscreenVSync(true)
			, UseAntialiasing(false)
			, ClearColor(std::Black)
			, AllowUserShowMouse(false)
			, AllowUserFullscreenToggle(false)
			, AllowUserKeyboardClose(false)
		{}

		int2 ClientPosition; // Position of the user area (not OS decoration/frame) of the window
		int2 ClientSize; // Size of the user area (not OS decoration/frame) of the window
		int2 FullscreenSize; // Size of the display/window when in fullscreen state (ignored in any non-fullscreen state)
		int FullscreenRefreshRate; // Display refresh rate when in fullscreen state (ignored in any non-fullscreen state)
		int BackgroundSleepMS; // Sleep this long when not the foreground window

		STATE State;
		STYLE Style;
		CURSOR_STATE CursorState; // Applies when cursor is over the client area
		bool Enabled;
		bool HasFocus;
		bool AlwaysOnTop;
		bool FullscreenVSync;
		bool UseAntialiasing;
		bool AllowUserShowMouse; // Enable Alt-F1 to toggle between set mouse state and visible
		bool AllowUserFullscreenToggle; // Enable Alt-Enter to toggle to fullscreen in windows
		bool AllowUserKeyboardClose; // Enable Alt-F4 to close in windows

		oColor ClearColor;
	};

	// The return value is ignored by most events. However CLOSING functions should 
	// return true in the general case, or false to disallow the closing of a window. 
	// Only one false is required to abort closing. CLOSING is only issued from user 
	// interaction. Programmatic closing of a window skips directly to CLOSED.
	// Use QueryInterface() and GetDrawMode() to obtain the correct system resources.
	// for responding to events.
	typedef oFUNCTION<bool(EVENT _Event, unsigned int _SuperSampleScale, const DESC& _Desc)> HookFn;

	// Redraws the client area, optionally blocking and/or specifying a limited area
	// for update.
	virtual void Refresh(bool _Blocking = true, const oRECT* _pDirtyRect = nullptr) threadsafe = 0;

	// Creates a snapshot of the window as it appears at the time of this call. In
	// Windowed mode this often requires the window to be fully visible, so a call
	// to this function will call SetFocus() on this window.
	virtual bool CreateSnapshot(interface oImage** _ppImage, bool _IncludeBorder = false) threadsafe = 0;

	virtual char* GetTitle(char* _StrDestination, size_t _SizeofStrDestination) const threadsafe = 0;
	template<size_t size> inline char* GetTitle(char (&_StrDestination)[size]) const threadsafe { return GetTitle(_StrDestination, size); }
	virtual void SetTitle(const char* _Title) threadsafe = 0;

	virtual bool HasFocus() const threadsafe = 0;
	virtual void SetFocus() threadsafe = 0;

	// Open is defined as the window in any state non-closing or closed state. 
	// Closing can be triggered by user interaction such as clicking on the close 
	// button or pressing a keyboard combiniation that closes the window. The closed 
	// state occurs either after closing or through programmatic release of the
	// window, meaning the window is not considered open once destruction starts.
	virtual bool IsOpen() const threadsafe = 0;

	// For read-only querying of the window's DESC, prefer this over Map/Unmap
	virtual void GetDesc(DESC* _pDesc) threadsafe = 0;

	// Maps a description of the window and prevents other threads from altering
	// the state while it is modified through external events such as user 
	// interaction or internal updates.
	virtual DESC* Map() threadsafe = 0;
	virtual void Unmap() threadsafe = 0;

	virtual DRAW_MODE GetDrawMode() const threadsafe = 0;
	virtual void* GetNativeHandle() threadsafe = 0;

	// Install a callback that is called with various EVENTs during the run of
	// this window. The callback will be executed in the window's execution
	// thread. An initial RESIZED will be called to ensure any device-dependent
	// resources are allocated before the Hook is fully registered. The hook 
	// should return true if successful. If false, Hook() will return oInvalid
	// and oErrorGetLast() can be used to get the error that occurred on the 
	// Window thread. Symmetrically Unhook() calls a final RESIZING to ensure 
	// any device-dependent resources are freed. Draw events are broken out 
	// into various phases/layers. First there is DRAW_BACKBUFFER for doing 
	// general drawing without UI-centric APIs. This is the perferred stage for 
	// 3D or video rendering. Then DRAW_UIAA draws anti-aliased items. In 
	// supersampled implementations, the oversized buffer is reduced before 
	// drawing DRAW_UI objects. This is preferred only for domain-specific 
	// antialiasing like ClearType for text. Finally there is a last phase that 
	// draws directly to the front buffer after all hardware-accelerated render 
	// targets have been composited and read back in DRAW_FRONTBUFFER.
	virtual unsigned int Hook(HookFn _Hook) threadsafe = 0;
	virtual void Unhook(unsigned int _HookID) threadsafe = 0;

	// For platform-specific things not covered by Desc. These return the 
	// address of where the data is stored, so they need to be cast and then 
	// dereferenced to get the value, so:
	// MYVALUE v = *(const MYVALUE*)w->GetProperty("PropertyName");
	virtual const void* GetProperty(const char* _Name) const = 0;
	virtual bool SetProperty(const char* _Name, void* _Value) = 0;
	template<typename T> const T& GetProperty(const char* _Name) const { return (const T&)*(T*)GetProperty(_Name); }
};

// NOTE: Mostly _pAssociatedNativeHandle should be NULL, but if you're careful
// to match what you pass with the DRAW_MODE specified, then you can integrate
// oWindow with another large visual management system such as a renderer or 
// video streamer.
bool oWindowCreate(const oWindow::DESC& _Desc, void* _pAssociatedNativeHandle, oWindow::DRAW_MODE _Mode, threadsafe oWindow** _ppWindow);
inline bool oWindowCreate(const oWindow::DESC& _Desc, void* _pAssociatedNativeHandle, oWindow::DRAW_MODE _Mode, oWindow** _ppWindow) { return oWindowCreate(_Desc, _pAssociatedNativeHandle, _Mode, const_cast<threadsafe oWindow**>(_ppWindow)); }

#endif
