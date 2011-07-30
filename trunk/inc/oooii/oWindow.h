// $(header)
#pragma once
#ifndef oWindow_h
#define oWindow_h

#include <oooii/oColor.h>
#include <oooii/oInterface.h>
#include <oooii/oSurface.h>
#include <oVideo/oVideoCodec.h>
#include <oooii/oMath.h>

interface oImage;

interface oWindow : oInterface
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
		TRAYIZED, // Window is hidden, but has its icon placed in the system tray
		RESTORED, // Window is in normal sub-screen-size mode
		MINIMIZED, // Window is reduces to iconic or taskbar size
		MAXIMIZED, // Window takes up the entire screen
		FULL_SCREEN, // Window takes exclusive access to screen, and will not have a title bar, border, ect regardless of its style
	};

	enum STYLE
	{
		BORDERLESS, // There is no OS decoration of the client area
		FIXED, // There is OS decoration but no user resize is allowed
		SIZEABLE, // OS decoration and user can resize window
	};

	enum RECT_EVENT
	{
		RECT_BEGIN,
		MOVE_OCCURING,
		RESIZE_OCCURING,
		RECT_END,
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
			, RefreshRateN(0)
			, RefreshRateD(0)
			, State(RESTORED)
			, Style(SIZEABLE)
			, UseAntialiasing(false)
			, Enabled(true)
			, HasFocus(true)
			, AlwaysOnTop(false)
			, EnableCloseButton(true)
			, MSSleepWhenNoFocus(200)
			, LockToVsync(false)
		{}

		int ClientX;
		int ClientY;
		int ClientWidth;
		int ClientHeight;
		unsigned int RefreshRateN; //refresh rate is specified as a rational. use 0/0 to let system decide. This is only a request, it may not be honored.
		unsigned int RefreshRateD; //refresh rate denominator
		STATE State;
		STYLE Style;
		bool UseAntialiasing;
		bool Enabled;
		bool HasFocus;
		bool AlwaysOnTop;
		bool EnableCloseButton;
		unsigned int MSSleepWhenNoFocus;
		bool LockToVsync;
	};

	typedef oFUNCTION< void(RECT_EVENT _Event, STATE _State, oRECT _Rect )> RectHandlerFn;

	struct Child : oInterface
	{
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

	struct Resizer : Child
	{
	};

	interface RoundedBox : Child
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

	interface Line : Child
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

	interface Font : Child
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

	interface Text : Child
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

	interface Picture : Child
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

	interface Video : Child
	{
		struct DESC
		{
			DESC()
				: X(DEFAULT)
				, Y(DEFAULT)
				, Width(DEFAULT)
				, Height(DEFAULT)
				, Anchor(MIDDLE_CENTER)
				, UseFrameTime(false)
				, StitchVertically(true)
				, AllowCatchUp(true)
			{}

			// These dimensions are as-drawn
			int X; // DEFAULT is 0
			int Y; // DEFAULT is 0
			int Width; // use DEFAULT for full size of client window
			int Height; // use DEFAULT for full size of client window
			bool UseFrameTime; //play video back at rate specified in stream, if false play back as fast as possible
			bool StitchVertically; 
			oRECT SourceRects[24]; // video outside this rect will get discarded. You still have to pay the performance cost for decoding that video though.
			oRECT DestRects[24]; // video outside this rect will get discarded. You still have to pay the performance cost for decoding that video though.
			bool AllowCatchUp; // If true allow any containers that have fallen behind, to catch up. If false an assert will be thrown if containers don't alll decode the same frame.

			// Starting position relative to the parent window's client area
			ALIGNMENT Anchor;
		};

		virtual void GetDesc(DESC* _pDesc) const threadsafe = 0;
	};


	// _____________________________________________________________________________
	// Factory API

	// If a non-null _pDesc is specified, Open is automatically called during 
	// Create. For the default draw API, pass 0 to _DrawAPIFourCC. On Windows you 
	// can force a draw API to either 'D2D1' or 'GDI '.  You can also supply a _pAssociatedNativeHandle
	// which on windows is a D3D10.1 device to use when in 'D2D1' mode
	static bool Create(const DESC* _pDesc, void* _pAssociatedNativeHandle, const char* _Title, unsigned int _DrawAPIFourCC, oWindow** _ppWindow);

	virtual bool Open(const DESC* _pDesc, const char* _Title = "") = 0;
	virtual void Close() = 0;
	virtual bool IsOpen() const threadsafe = 0;

	virtual bool CreateResizer(RectHandlerFn _ResizeHandler, threadsafe Resizer** _ppResizer) threadsafe = 0;
	virtual bool CreateLine(const Line::DESC* _pDesc, threadsafe Line** _ppLine) threadsafe = 0;
	virtual bool CreateRoundedBox(const RoundedBox::DESC* _pDesc, threadsafe RoundedBox** _ppRoundedBox) threadsafe = 0;
	virtual bool CreateFont(const Font::DESC* _pDesc, threadsafe Font** _ppFont) threadsafe = 0;
	virtual bool CreateText(const Text::DESC* _pDesc, threadsafe Font* _pFont, threadsafe Text** _ppText) threadsafe = 0;
	virtual bool CreatePicture(const Picture::DESC* _pDesc, threadsafe Picture** _ppPicture) threadsafe = 0;
	virtual bool CreateVideo(const Video::DESC*_pDesc, oVideoContainer** _ppVideos, size_t _NumVideos, threadsafe Video** _ppVideo) threadsafe = 0;

	// _____________________________________________________________________________
	// Runtime API

	// This should be called at the top of the application's main loop. It 
	// primarily handles message polling.
	virtual bool Begin() = 0;

	// This should be called at the bottom of the application's main loop only 
	// if Begin succeeds. It primarily handles operations that occur after all 
	// other operations are finished, such as overlay drawing.
	// If _blockUntilPainted is true, this function will block until all painting
	// has completed. Try to avoid blocking, its mostly only useful in unittest or 
	// the rare case you must be sure the painting is finished when this function returns.
	virtual void End(bool _ForceRefresh = false, bool _blockUntilPainted = false) = 0;

	// Runs an empty Begin/End loop while the window remains open, so this just 
	// pumps messages until the user interacts with the window to close it, or 
	// the time is reached.
	static bool Pump(oWindow* _pWindow, bool _CloseOnTimeout, unsigned int _Timeout = oINFINITE_WAIT);

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

	virtual void* GetNativeHandle() threadsafe = 0;

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
