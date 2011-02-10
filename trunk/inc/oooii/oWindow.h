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
#pragma once
#ifndef oWindow_h
#define oWindow_h

#include <oooii/oColor.h>
#include <oooii/oInterface.h>
#include <oooii/oSurface.h>

interface oImage;

interface oWindow : public oInterface
{
	// An OS Window tailored to the needs of simple image display, such as from 
	// a video feed or 3D rendering.

	// DEFAULT can be set to ClientX, ClientY, ClientWidth, ClientHeight to 
	// indicate that use of the DESC should leave the specified parameter as it
	// was.
	static const int DEFAULT = 0x80000000;

	enum STATE
	{
		HIDDEN, // Window is invisible, but exists
		RESTORED, // Window is in normal sub-screen-size mode
		MINIMIZED, // Window is reduces to iconic or taskbar size
		MAXIMIZED, // Window takes up the entire screen
	};

	enum STYLE
	{
		BORDERLESS, // There is no OS decoration of the client area
		FIXED, // There is OS decoration but no user resize is allowed
		SIZEABLE, // OS decoration and user can resize window
	};

	enum RESIZE_EVENT
	{
		RESIZE_BEGIN,
		RESIZE_CHANGE,
		RESIZE_END,
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
			: ClientX(DEFAULT)
			, ClientY(DEFAULT)
			, ClientWidth(DEFAULT)
			, ClientHeight(DEFAULT)
			, State(RESTORED)
			, Style(SIZEABLE)
			, UseAntialiasing(false)
			, Enabled(true)
			, HasFocus(true)
			, AlwaysOnTop(false)
			, EnableCloseButton(true)
		{}

		int ClientX;
		int ClientY;
		int ClientWidth;
		int ClientHeight;
		STATE State;
		STYLE Style;
		bool UseAntialiasing;
		bool Enabled;
		bool HasFocus;
		bool AlwaysOnTop;
		bool EnableCloseButton;
	};

	typedef void (*ResizeHandlerFn)(RESIZE_EVENT _Event, STATE _State, unsigned int _Width, unsigned int _Height, void* _pUserData);

	class Child : public oInterface
	{
	public:
		virtual void GetWindow(threadsafe oWindow** _ppWindow) threadsafe = 0;

		// This has an implementation that registers a new child with its
		// parent window. The user should call this from a child's ctor when
		// the object is otherwise initialized. Unregister should be called 
		// from the dtor.
		void Register();
		void Unregister();

	private:
		friend struct oWindow_Impl;

		// This class can be implemented by the user to extend
		// the default functionality of the window.

		// Will be called by Open() in the order the child was appended.
		virtual bool Open() = 0;

		// Will be called by Close() in the order the child appended.
		virtual void Close() = 0;

		// Will be called by Begin() Any child not returning true will 
		// cause Begin to return false as well.
		virtual bool Begin() = 0;

		// Will be called by End()
		virtual void End() = 0;

		// Will be called by the Window's repaint process in the order
		// the child was appended.
		virtual void HandlePaint(void* _pPlatformData) = 0;

		// Will be called by the Window's message pump in the order 
		// the child was appended. This should return true if the next 
		// child in the chain should handle its messages, or false to 
		// short-circuit further handling, or any other error to indicate 
		// failure. The parameter is a pointer to a parameter list struct 
		// specific to the platform. On Windows, this is a CWPSTRUCT.
		virtual bool HandleMessage(void* _pPlatformData) = 0;
	};

	class Resizer : public Child
	{
	};

	interface RoundedBox : public Child
	{
		struct DESC
		{
			DESC()
				: X(DEFAULT)
				, Y(DEFAULT)
				, Width(DEFAULT)
				, Height(DEFAULT)
				, Anchor(MIDDLE_CENTER)
				, Color(std::White)
				, BorderColor(std::Black)
				, Roundness(10.0f)
			{}

			int X;
			int Y;
			int Width;
			int Height;
			ALIGNMENT Anchor;
			oColor Color;
			oColor BorderColor;
			float Roundness;
		};

		virtual void GetDesc(DESC* _pDesc) const threadsafe = 0;
		virtual void SetDesc(DESC* _pDesc) threadsafe = 0;
	};

	interface Line : public Child
	{
		struct DESC
		{
			DESC()
				: X1(0)
				, Y1(0)
				, X2(0)
				, Y2(0)
				, Thickness(1)
				, Color(std::White)
			{}

			int X1;
			int Y1;
			int X2;
			int Y2;
			int Thickness;
			oColor Color;
		};

		virtual void GetDesc(DESC* _pDesc) const threadsafe = 0;
		virtual void SetDesc(DESC* _pDesc) threadsafe = 0;
	};

	interface Font : public Child
	{
		enum STYLE
		{
			NORMAL,
			BOLD,
			ITALIC,
			BOLDITALIC,
		};

		struct DESC
		{
			DESC()
				: Style(NORMAL)
				, PointSize(10.0f)
				, ShadowOffset(2.0f)
			{
				*FontName = 0;
			}

			char FontName[64];
			STYLE Style;
			float PointSize; // Like in MS Word and such.
			float ShadowOffset; // In points. Bigger fonts will require bigger offsets.
		};

		virtual void GetDesc(DESC* _pDesc) const threadsafe = 0;
	};

	interface Text : public Child
	{
		struct DESC
		{
			DESC()
				: X(DEFAULT)
				, Y(DEFAULT)
				, Width(DEFAULT)
				, Height(DEFAULT)
				, Anchor(MIDDLE_CENTER)
				, Alignment(MIDDLE_CENTER)
				, Color(std::White)
				, ShadowColor(std::Black)
				, MultiLine(false)
			{
				*String = 0;
			}

			// Position relative to the anchor
			int X;
			int Y;
			int Width;
			int Height;

			// Starting position relative to the parent window's client area
			ALIGNMENT Anchor;

			// Alignment within the logical rect defined above. For text that
			// is centered no matter the window size, specify MIDDLE_CENTER for
			// both Anchor and Alignment.
			ALIGNMENT Alignment;

			oColor Color;
			
			// Specify 0 to have unshadowed text
			oColor ShadowColor; 
			
			char String[1024];
			bool MultiLine;
		};

		virtual void GetDesc(DESC* _pDesc) const threadsafe = 0;
		virtual void SetDesc(const DESC* _pDesc) threadsafe = 0;
		virtual void SetFont(threadsafe Font* _pFont) threadsafe = 0;
	};

	interface Picture : public Child
	{
		struct DESC
		{
			DESC()
				: X(DEFAULT)
				, Y(DEFAULT)
				, Width(DEFAULT)
				, Height(DEFAULT)
				, Anchor(MIDDLE_CENTER)
			{}

			// These dimensions are as-drawn
			int X; // DEFAULT is 0
			int Y; // DEFAULT is 0
			int Width; // use DEFAULT for full size of client window
			int Height; // use DEFAULT for full size of client window

			// Starting position relative to the parent window's client area
			ALIGNMENT Anchor;

			// This is the dimensions of the underlying bitmap data. Copy 
			// respects these dimensions.
			oSurface::DESC SurfaceDesc;
		};

		virtual void GetDesc(DESC* _pDesc) const threadsafe = 0;
		virtual void Copy(const void* _pSourceData, size_t _SourcePitch, bool _FlipHorizontal = false, bool _FlipVertical = false) threadsafe = 0;
	};

	// _____________________________________________________________________________
	// Factory API

	// If a non-null _pDesc is specified, Open is automatically called during 
	// Create. For the default draw API, pass 0 to _DrawAPIFourCC. On Windows you 
	// can force a draw API to either 'D2D1' or 'GDI '.
	static bool Create(const DESC* _pDesc, const char* _Title, unsigned int _DrawAPIFourCC, oWindow** _ppWindow);

	virtual bool Open(const DESC* _pDesc, const char* _Title = "") = 0;
	virtual void Close() = 0;
	virtual bool IsOpen() const threadsafe = 0;

	virtual bool CreateResizer(ResizeHandlerFn _ResizeHandler, void* _pUserData, threadsafe Resizer** _ppResizer) threadsafe = 0;
	virtual bool CreateLine(const Line::DESC* _pDesc, threadsafe Line** _ppLine) threadsafe = 0;
	virtual bool CreateRoundedBox(const RoundedBox::DESC* _pDesc, threadsafe RoundedBox** _ppRoundedBox) threadsafe = 0;
	virtual bool CreateFont(const Font::DESC* _pDesc, threadsafe Font** _ppFont) threadsafe = 0;
	virtual bool CreateText(const Text::DESC* _pDesc, threadsafe Font* _pFont, threadsafe Text** _ppText) threadsafe = 0;
	virtual bool CreatePicture(const Picture::DESC* _pDesc, threadsafe Picture** _ppPicture) threadsafe = 0;

	// _____________________________________________________________________________
	// Runtime API

	// This should be called at the top of the application's main loop. It 
	// primarily handles message polling.
	virtual bool Begin() = 0;

	// This should be called at the bottom of the application's main loop only 
	// if Begin succeeds. It primarily handles operations that occur after all 
	// other operations are finished, such as overlay drawing.
	virtual void End() = 0;

	// Runs an empty Begin/End loop while the window remains open, so this just 
	// pumps messages until the user interacts with the window to close it, or 
	// the time is reached.
	static bool Pump(oWindow* _pWindow, bool _CloseOnTimeout, unsigned int _Timeout = ~0u);

	// _____________________________________________________________________________
	// Accessors/Mutators

	// This changes the desc of the window and does all subsequent updates, so 
	// this can be a heavyweight operation. GetDesc and SetDesc are immediate 
	// mode, meaning that if SetDesc is called to set the window to BORDERLESS, 
	// then the next time GetDesc is called, Style will be BORDERLESS and X,Y
	// will be 0,0. So to restore back to the prior state, the user should store 
	// the desc before setting the new state.
	virtual void SetDesc(const DESC* _pDesc) threadsafe = 0;
	virtual void GetDesc(DESC* _pDesc) const threadsafe = 0;

	// Returns the display on which the majority of the window is displayed
	virtual unsigned int GetDisplayIndex() const threadsafe = 0;

	virtual const char* GetTitle() const = 0;
	virtual void SetTitle(const char* _Title) = 0;

	virtual bool HasFocus() const = 0;
	virtual void SetFocus() = 0;

	virtual void* GetNativeHandle() = 0; // returns an HWND on windows

	// Creates a snapshot of the window as it appears at the time of this call.
	virtual bool CreateSnapshot(oImage** _ppImage, bool _IncludeBorder = false) = 0;

	// For platform-specific things not covered by Desc. These return the 
	// address of where the data is stored, so they need to be cast and then 
	// dereferenced to get the value, so:
	// MYVALUE v = *(const MYVALUE*)w->GetProperty("PropertyName");
	virtual const void* GetProperty(const char* _Name) const = 0;
	virtual bool SetProperty(const char* _Name, void* _Value) = 0;
	template<typename T> const T& GetProperty(const char* _Name) const { return (const T&)*(T*)GetProperty(_Name); }
};

#endif
