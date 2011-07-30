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
#include <oooii/oWindows.h>
#include <oooii/oWindow.h>
#include <oooii/oAssert.h>
#include <oooii/oByte.h>
#include <oooii/oDisplay.h>
#include <oooii/oImage.h>
#include <oooii/oLockedVector.h>
#include <oooii/oMath.h>
#include <oooii/oRef.h>
#include <oooii/oRefCount.h>
#include <oooii/oStdio.h>
#include <oooii/oString.h>
#include <oooii/oSurface.h>
#include <oooii/oThreading.h>
#include <oooii/oD3D10.h>
#include <oooii/oSTL.h>
#include <oooii/oThread.h>
#include <oooii/oEvent.h>

static const unsigned int oWM_CLOSE = WM_USER;
static const unsigned int oWM_TRAY = WM_USER+1;

template<> const char* oAsString(const oWindow::STATE& _State)
{
	switch (_State)
	{
		case oWindow::HIDDEN: return "hidden";
		case oWindow::TRAYIZED: return "trayized";
		case oWindow::RESTORED: return "restored";
		case oWindow::MINIMIZED: return "minimized";
		case oWindow::MAXIMIZED: return "maximized";
		default: oASSUME(0);
	}
}

void ConvertAlignment(char* _Opts, oWindow::ALIGNMENT _Alignment)
{
	if (_Alignment > oWindow::MIDDLE_RIGHT)
		*_Opts++ = 'b';
	else if (_Alignment > oWindow::TOP_RIGHT)
		*_Opts++ = 'm';

	if ((_Alignment % 3) == 1)
		*_Opts++ = 'c';
	else if ((_Alignment % 3) == 2)
		*_Opts++ = 'r';
}

struct oWindow_Impl;

class oMsgPumpProc : public oThread::Proc, oNoncopyable
{
public:
	oDEFINE_REFCOUNT_INTERFACE(RefCount);
	oDEFINE_TRIVIAL_QUERYINTERFACE(oGetGUID<oMsgPumpProc>());

	void RunIteration() override;

	bool OnBegin() override;

	void OnEnd() override {}

	// This will not be valid until the window is created. call WaitUntilOpened first to be sure this is valid.
	HWND GetHwnd() const {return Hwnd;}
	// This should be called instead of DestroyWindow which wont do anything if called externally to this thread.
	void Destroy() const { PostMessage(Hwnd, oWM_CLOSE, 0, 0); } 
	// Window is guaranteed to be destroyed when this is returned. The thread itself will still be running,
	//	but will be doing nothing.
	void WaitUntilClosed() { CloseEvent.Wait(); }
	// Window is guaranteed to created. It may not be painted yet though. Although its promised that it will on its own shortly if not.
	void WaitUntilOpened() { OpenEvent.Wait(); }

	oMsgPumpProc(const oWindow::DESC* _pDesc, const char* _Title, oWindow_Impl *WImpl, WNDPROC _winProc);

private:

	oRefCount RefCount;
	HWND Hwnd;
	oWindow::DESC Desc;
	char Title[_MAX_PATH];
	WNDPROC WinProc;
	oWindow_Impl *WImpl;
	oEvent OpenEvent;
	oEvent CloseEvent;
};

const oGUID& oGetGUID( threadsafe const oMsgPumpProc* threadsafe const * )
{
	// {dc741266-a1b8-49a9-864d-42e9bfd3cd5c}
	static const oGUID oIIDoMsgPumpProc = { 0xdc741266, 0xa1b8, 0x49a9, { 0x86, 0x4d, 0x42, 0xe9, 0xbf, 0xd3, 0xcd, 0x5c } };
	return oIIDoMsgPumpProc; 
}

oMsgPumpProc::oMsgPumpProc(const oWindow::DESC* _pDesc, const char* _Title, oWindow_Impl *_wImpl, WNDPROC _winProc) : 
	Desc(*_pDesc), WinProc(_winProc), WImpl(_wImpl)
{
	// The window itself must be created from the thread, so save everything so it can be created in OnBegin.
	strcpy_s(Title, _Title);
	CloseEvent.Reset();
	OpenEvent.Reset();
}

void oMsgPumpProc::RunIteration()
{
	MSG msg;
	if(GetMessage(&msg, Hwnd, 0, 0) <= 0) //either an error or WM_QUIT
	{
		CloseEvent.Set();
	}
	else
	{
		TranslateMessage(&msg);
		if (msg.message == oWM_CLOSE)
			DestroyWindow(Hwnd);
		else
			DispatchMessage(&msg);
	}
}

bool oMsgPumpProc::OnBegin()
{
	if (FAILED(oCreateSimpleWindow(&Hwnd, WinProc, WImpl, Title, Desc.ClientX,  Desc.ClientY,  Desc.ClientWidth,  Desc.ClientHeight ) ) )
		return false;

	OpenEvent.Set();
	return true;
}

struct oWindow_Impl : public oWindow
{
	enum DRAW_METHOD
	{
		DRAW_USING_GDI,
		DRAW_USING_D2D,
		DRAW_USING_DX11,
	};

	oDEFINE_REFCOUNT_INTERFACE(RefCount);
	bool QueryInterface(const oGUID& _InterfaceID, threadsafe void** _ppInterface) threadsafe override;

	void GetDesc(DESC* _pDesc) const threadsafe override;
	void SetDesc(const DESC* _pDesc) threadsafe override;
	unsigned int GetDisplayIndex() const threadsafe override;
	bool Open(const DESC* _pDesc, const char* _Title = "") override;
	void Close() override;
	bool IsOpen() const threadsafe override;
	bool Begin() override;
	void End(bool _ForceRefresh, bool _blockUntilPainted) override;
	const char* GetTitle() const override;
	void SetTitle(const char* _Title) override;
	bool HasFocus() const override;
	void SetFocus() override;
	void* GetNativeHandle() threadsafe override;
	const void* GetProperty(const char* _Name) const override;
	bool SetProperty(const char* _Name, void* _Value) override;
	bool CreateResizer(RectHandlerFn _ResizeHandler, threadsafe Resizer** _ppResizer) threadsafe override;
	bool CreateRoundedBox(const RoundedBox::DESC* _pDesc, threadsafe RoundedBox** _ppRoundedBox) threadsafe override;
	bool CreateLine(const Line::DESC* _pDesc, threadsafe Line** _ppLine) threadsafe override;
	bool CreateFont(const Font::DESC* _pDesc, threadsafe Font** _ppFont) threadsafe override;
	bool CreateText(const Text::DESC* _pDesc, threadsafe Font* _pFont, threadsafe Text** _ppText) threadsafe override;
	bool CreatePicture(const Picture::DESC* _pDesc, threadsafe Picture** _ppPicture) threadsafe override;
	bool CreateVideo(const Video::DESC*_pDesc, oVideoContainer** _ppVideos, size_t _NumVideos, threadsafe Video** _ppVideo) threadsafe override;
	bool CreateSnapshot(oImage** _ppImage, bool _IncludeBorder = false) override;
	oWindow_Impl(const DESC& Desc, void* _pAssociatedNativeHandle, const char* _Title, unsigned int _FourCCDrawAPI, bool* _pSuccess);
	~oWindow_Impl();

	oRecursiveLockedVector<Child*> Children;  // Children have to be recursive because they can callback into the vector
	oLockedVector<Picture*>	Pictures;
	oLockedVector<Video*> Videos;
	oLockedVector<Line*> Lines;
	oLockedVector<RoundedBox*> RoundedBoxes;
	oLockedVector<Text*> Texts;

	HWND hWnd;
	HICON hIcon;
	unsigned int GDIAntialiasingMultiplier;
	HBRUSH hClearBrush;
	HDC hOffscreenAADC;
	HDC hOffscreenDC;
	HBITMAP hOffscreenAABmp; // AA shapes
	HBITMAP hOffscreenBmp; // let Windows to cleartyped-text
	oColor ClearColor;
	bool UseAntialiasing;
	bool Fullscreen;
	unsigned int FullscreenWidth;
	unsigned int FullscreenHeight;
	bool LockToVsync;
	bool Closing;
	bool EnableCloseButton;
	mutable oMutex DescLock;
	oRefCount RefCount;
	DRAW_METHOD DrawMethod;
	unsigned int MSSleepWhenNoFocus;
	unsigned int RefreshRateN; //hold onto refresh rate
	unsigned int RefreshRateD; 

	oRef<ID3D10Device1> D3DDevice;
	oRef<ID3D11Device> D3D11Device;
	oRef<ID3D11RenderTargetView> D3D11View;
	oRef<IDXGISwapChain> D3DSwapChain;
	static const DXGI_FORMAT D3DSwapChainFMT = DXGI_FORMAT_B8G8R8A8_UNORM;
	oMutex ResizeMutex;

	oRef<oMsgPumpProc> MsgPumpProc;
	oRef<threadsafe oThread> MsgPumpThread;
	
	char Title[_MAX_PATH];

	#if oDXVER >= oDXVER_10
		// The rendertarget and factory need to be cached on the window to ensure that 
		// all resources are created from the same objects.  The rendertarget can change
		// if a resize event occurs
		oRef<ID2D1Factory> Factory;
		oRef<ID2D1RenderTarget> RenderTarget;
		HRESULT CreateRendertarget();
	#endif

	static bool OpenChild(Child*& pChild) { return pChild->Open(); }
	static bool CloseChild(Child*& pChild) { pChild->Close(); return true; }
	static bool BeginChild(Child*& pChild) { return pChild->Begin(); }
	static bool EndChild(Child*& pChild) { pChild->End(); return true; }

	bool AddChild(Child* pChild) threadsafe;
	inline void RemoveChild(Child* _pChild) threadsafe { Children.erase(_pChild); }
	inline bool OpenChildren() threadsafe { return Children.foreach(&OpenChild); }
	inline void CloseChildren() threadsafe { Children.foreach(&CloseChild); }
	inline bool BeginChildren() threadsafe { return Children.foreach(&BeginChild); }
	inline void EndChildren() threadsafe { Children.foreach(&EndChild); }
	static bool MessageChild(oWindow::Child*& _pChild, void* _pUserData);
	bool HandleChildrenMessages(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) threadsafe;

	template<typename T> static bool PaintChild(T*& _pChild, void* _pUserData) { _pChild->HandlePaint(_pUserData); return true; }

	void OnClose();
	BOOL OnResize(UINT newWidth, UINT newHeight);
	BOOL OnPaint(HWND hwnd);

	HRESULT CreateDeviceResources();
	void DiscardDeviceResources();

	void GDIClear(COLORREF _Color);

	LRESULT WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	static LRESULT CALLBACK StaticWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
};

void oWindow::Child::Register()
{
	oRef<threadsafe oWindow> w;
	GetWindow(&w);
	static_cast<threadsafe oWindow_Impl*>(w.c_ptr())->AddChild(this);
}

void oWindow::Child::Unregister()
{
	oRef<threadsafe oWindow> w;
	GetWindow(&w);
	static_cast<threadsafe oWindow_Impl*>(w.c_ptr())->RemoveChild(this);
}

namespace detail {

struct WINPAINT_DESC
{
	HDC hDC;
	unsigned int Scale;
};

static int GetShowCmd(oWindow::STATE _State, bool _TakeFocus)
{
	switch (_State)
	{
		case oWindow::HIDDEN: return SW_HIDE;
		case oWindow::TRAYIZED: return SW_HIDE;
		case oWindow::MINIMIZED: return _TakeFocus ? SW_SHOWMINIMIZED : SW_SHOWMINNOACTIVE;
		case oWindow::MAXIMIZED: return SW_SHOWMAXIMIZED;
		default: break;
	}

	return _TakeFocus ? SW_SHOWNORMAL : SW_SHOWNOACTIVATE;
}

static DWORD GetStyle(oWindow::STYLE _Style)
{
	switch (_Style)
	{
		case oWindow::BORDERLESS: return WS_POPUP;
		case oWindow::FIXED: return WS_CAPTION|WS_SYSMENU|WS_MINIMIZEBOX;
		default: break;
	}

	return WS_OVERLAPPEDWINDOW;
}

static void GetDefaultDesc(HWND _hWnd, const oWindow::DESC* _pInDesc, oWindow::DESC* _pOutDesc)
{
	*_pOutDesc = *_pInDesc;

	oDisplay::DESC displayDesc;
	oDisplay::GetDesc(oGetWindowDisplayIndex(_hWnd), &displayDesc);

	if (_pInDesc->ClientWidth == oWindow::DEFAULT)
		_pOutDesc->ClientWidth = 640;

	if (_pInDesc->ClientHeight == oWindow::DEFAULT)
		_pOutDesc->ClientHeight = 480;

	if (_pInDesc->ClientX == oWindow::DEFAULT)
		_pOutDesc->ClientX = (displayDesc.ModeDesc.Width - _pOutDesc->ClientWidth) / 2;

	if (_pInDesc->ClientY == oWindow::DEFAULT)
		_pOutDesc->ClientY = (displayDesc.ModeDesc.Height - _pOutDesc->ClientHeight) / 2;
}

static void GetFullscreenDimensions(const oWindow::DESC& _Desc, unsigned int* _pWidth, unsigned int* _pHeight)
{
	// This adjusts the description to match the actual dimensions
	// of the monitor that region would primarily reside on.
	RECT WindowRect;
	WindowRect.left = oWindow::DEFAULT == _Desc.ClientX ? 0 : _Desc.ClientX;
	WindowRect.top = oWindow::DEFAULT == _Desc.ClientY ? 0 : _Desc.ClientY;
	WindowRect.right = WindowRect.left + ( oWindow::DEFAULT == _Desc.ClientWidth ? 640 : _Desc.ClientWidth );
	WindowRect.bottom = WindowRect.top + ( oWindow::DEFAULT == _Desc.ClientHeight ? 480 : _Desc.ClientHeight );

	oRef<IDXGIAdapter1> Adapter;
	oRef<IDXGIOutput> Output;
	if( !oDXGIGetAdapterWithMonitor(WindowRect, &Adapter, &Output ) )
	{
		*_pWidth = 0;
		*_pHeight = 0;
		return;
	}

	DXGI_OUTPUT_DESC OutputDesc;
	Output->GetDesc(&OutputDesc);
	const RECT& Coord = OutputDesc.DesktopCoordinates;
	*_pWidth = Coord.right - Coord.left;
	*_pHeight = Coord.bottom - Coord.top;
}

static void GetDesc(HWND _hWnd, oWindow::DESC* _pDesc)
{
	RECT rClient;
	oGetClientScreenRect(_hWnd, &rClient);
	_pDesc->ClientX = rClient.left;
	_pDesc->ClientY = rClient.top;
	_pDesc->ClientWidth = rClient.right - rClient.left;
	_pDesc->ClientHeight = rClient.bottom - rClient.top;

	if (oTrayGetIconRect(_hWnd, 0, &rClient) && !IsWindowVisible(_hWnd)) _pDesc->State = oWindow::TRAYIZED;
	else if (!IsWindowVisible(_hWnd)) _pDesc->State = oWindow::HIDDEN;
	else if (IsIconic(_hWnd)) _pDesc->State = oWindow::MINIMIZED;
	else if (IsZoomed(_hWnd)) _pDesc->State = oWindow::MAXIMIZED;
	else _pDesc->State = oWindow::RESTORED;

	LONG_PTR style = GetWindowLongPtr(_hWnd, GWL_STYLE);
	if (style & WS_THICKFRAME) _pDesc->Style = oWindow::SIZEABLE;
	else if (style & WS_POPUP) _pDesc->Style = oWindow::BORDERLESS;
	else _pDesc->Style = oWindow::FIXED;

	_pDesc->Enabled = !!IsWindowEnabled(_hWnd);
	_pDesc->HasFocus = oHasFocus(_hWnd);
	_pDesc->AlwaysOnTop = !!(_hWnd == GetTopWindow(0));

	oWindow_Impl* pThis = (oWindow_Impl*)oGetWindowContext(_hWnd, 0, 0, 0);
	if (pThis)
		_pDesc->EnableCloseButton = pThis->EnableCloseButton;
}

static bool SetDesc(HWND _hWnd, const oWindow::DESC* _pDesc)
{
	if (!_hWnd || !_pDesc) return false;

	oWindow::DESC currentDesc;
	GetDesc(_hWnd, &currentDesc);

	// Strange bug, but I can't get a window whose style changes to 
	// get a WM_PAINT message after the change, but hiding and reshowing 
	// the window seems to work, so if there's a style change, force a
	// hide and reshow.
	oWindow::STATE state = _pDesc->State;
	oWindow::STATE restate = _pDesc->State;

	DWORD style = GetStyle(_pDesc->Style);

	bool styleChanged = false;
	if (currentDesc.Style != _pDesc->Style)
	{
		styleChanged = true;

		// http://msdn.microsoft.com/en-us/library/ms644898(VS.85).aspx
		// Strange, but the docs say to use last error this way...
		SetLastError(0);
		oVB(SetWindowLongPtr(_hWnd, GWL_STYLE, style));
	}

	RECT rDesired;
	oVB(GetClientRect(_hWnd, &rDesired));

	// Assign non-defaults
	if (_pDesc->ClientX != oWindow::DEFAULT) rDesired.left = _pDesc->ClientX;
	if (_pDesc->ClientY != oWindow::DEFAULT) rDesired.top = _pDesc->ClientY;
	if (_pDesc->ClientWidth != oWindow::DEFAULT) rDesired.right = rDesired.left + _pDesc->ClientWidth;
	if (_pDesc->ClientHeight != oWindow::DEFAULT) rDesired.bottom = rDesired.top + _pDesc->ClientHeight;
	AdjustWindowRect(&rDesired, style, FALSE);

	// Set up dimension settings
	// DO I NEED THIS WITH SetWindowLongPtr? SWP_FRAMECHANGED
	UINT SWPFlags = SWP_NOMOVE|SWP_NOSIZE|SWP_NOZORDER|SWP_NOACTIVATE;
	RECT rCurrent;
	GetWindowRect(_hWnd, &rCurrent);
	bool hasMove = false;
	bool hasResize = false;
	bool shouldTakeFocus = state != oWindow::TRAYIZED && (_pDesc->HasFocus || _pDesc->AlwaysOnTop);

	// maximized takes care of this in ShowWindow below, so don't step on that again in SetWindowPos
	if (state != oWindow::MAXIMIZED && state != oWindow::FULL_SCREEN)
	{
		// if moved, clear the no move flag
		if (rCurrent.left != rDesired.left || rCurrent.top != rDesired.top)
		{
			SWPFlags &=~ SWP_NOMOVE;
			hasMove = true;
		}

		// if resized, clear the no size flag
		if ((rCurrent.right-rCurrent.left) != (rDesired.right-rDesired.left) || (rCurrent.bottom-rCurrent.top) != (rDesired.bottom-rDesired.top))
		{
			SWPFlags &=~ SWP_NOSIZE;
			hasResize = true;
		}
	}

	if (shouldTakeFocus)
		SWPFlags &=~ SWP_NOZORDER|SWP_NOACTIVATE;

	oWindow_Impl* pThis = (oWindow_Impl*)oGetWindowContext(_hWnd, 0, 0, 0);
	if (pThis)
		pThis->EnableCloseButton = _pDesc->EnableCloseButton;

	// Here's the next piece, test on cached state, not _pDesc->State
	// because we may've overridden it with a force hidden.
	if (currentDesc.State != state || styleChanged)
	{
		SetWindowPos(_hWnd, HWND_BOTTOM, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE|SWP_NOACTIVATE);
		ShowWindow(_hWnd, GetShowCmd(state, shouldTakeFocus));
	}

	if (hasMove || hasResize || _pDesc->HasFocus)
		SetWindowPos(_hWnd, _pDesc->AlwaysOnTop ? HWND_TOPMOST : HWND_NOTOPMOST, rDesired.left, rDesired.top, rDesired.right-rDesired.left, rDesired.bottom-rDesired.top, SWPFlags);

	if (currentDesc.Enabled != _pDesc->Enabled)
		EnableWindow(_hWnd, _pDesc->Enabled);

	if (_pDesc->HasFocus && (restate != oWindow::HIDDEN && restate != oWindow::TRAYIZED))
		oSetFocus(_hWnd);

	// And here's the final piece of the hack from above where we force
	// hide and then show on style change.
	if (restate != state || styleChanged)
		ShowWindow(_hWnd, GetShowCmd(restate, shouldTakeFocus));

	// Handle the visibility of an associated tray icon
	if (state == oWindow::TRAYIZED && currentDesc.State != oWindow::TRAYIZED)
		oTrayMinimize(_hWnd, oWM_TRAY, 0);

	else if (state != oWindow::TRAYIZED && currentDesc.State == oWindow::TRAYIZED)
		oTrayRestore(_hWnd);

	return true;
}

struct oWindowResizer_Impl : public oWindow::Resizer
{
	oDEFINE_REFCOUNT_INTERFACE(RefCount);
	oDEFINE_TRIVIAL_QUERYINTERFACE(oGetGUID<oWindow>());
	//oDEFINE_TRIVIAL_QUERYINTERFACE(oGetGUID((volatile const oWindow_Impl *volatile const *) (NULL) ) );
	oDEFINE_GETINTERFACE_INTERFACE(threadsafe, GetWindow, threadsafe, oWindow, Window);

	oWindowResizer_Impl(threadsafe oWindow* _pParentWindow, oWindow::RectHandlerFn _ResizeHandlerFn)
		: Window(_pParentWindow)
		, RectHandler(_ResizeHandlerFn)
	{
		Register();
	}

	~oWindowResizer_Impl()
	{
		Unregister();
	}

	bool Open() override
	{
		oWindow::DESC desc;
		Window->GetDesc(&desc);
		oRECT rect;
		rect.SetMin( int2( desc.ClientX, desc.ClientY ) );
		rect.SetMax( rect.GetMin() + int2( desc.ClientWidth, desc.ClientHeight ) );
		RectHandler(oWindow::RECT_END, desc.State, rect);
		return true;
	}

	void HandlePaint(void* _pPlatformData) override {}
	bool HandleMessage(void* _pPlatformData) override
	{
		CWPSTRUCT* cw = static_cast<CWPSTRUCT*>(_pPlatformData);

		oWindow::RECT_EVENT e = oWindow::RECT_BEGIN;
		bool isSizeMsg = true;
		switch (cw->message)
		{
			case WM_ENTERSIZEMOVE: e = oWindow::RECT_BEGIN; break;
			case WM_SIZE: e = oWindow::RESIZE_OCCURING; break;
			case WM_MOVING: e = oWindow::MOVE_OCCURING; break;
			case WM_EXITSIZEMOVE: e = oWindow::RECT_END; break;
			default: isSizeMsg = false;
		}

		if (isSizeMsg)
		{
			// @oooii-eric: Have to be careful on calling anything that locks from the msg pump thread. It can cause deadlocks. So 
			//	don't use GetDesc here. Deadlocks are caused because any msg based windows api call will block until the msg pump
			//	thread processes it. the the msg pump thread is blocked on a mutex at the same time it will deadlock.
			RECT rClient;
			HWND hwnd = (HWND)Window->GetNativeHandle();
			oGetClientScreenRect(hwnd, &rClient);

			oWindow::STATE state;
			if (!IsWindowVisible(hwnd)) state = oWindow::HIDDEN;
			else if (IsIconic(hwnd)) state = oWindow::MINIMIZED;
			else if (IsZoomed(hwnd)) state = oWindow::MAXIMIZED;
			else state = oWindow::RESTORED;

			oRECT rect;
			rect.SetMin( int2( rClient.left, rClient.top ) );
			rect.SetMax( rect.GetMin() + int2( rClient.right - rClient.left, rClient.bottom - rClient.top ) );

			RectHandler(e, state, rect );
		}

		return true;
	}

	void Close() override {}
	bool Begin() override { return true; }
	void End() override {}

	oRef<threadsafe oWindow> Window;
	oWindow::RectHandlerFn RectHandler;
	oRefCount RefCount;
};

struct RoundedBoxGDI : public oWindow::RoundedBox
{
	oDEFINE_REFCOUNT_INTERFACE(RefCount);
	oDEFINE_TRIVIAL_QUERYINTERFACE(oGetGUID<RoundedBoxGDI>());
	oDEFINE_GETINTERFACE_INTERFACE(threadsafe, GetWindow, threadsafe, oWindow, Window);

	inline int Scale() threadsafe { return static_cast<threadsafe oWindow_Impl*>(Window.c_ptr())->GDIAntialiasingMultiplier; }

	RoundedBoxGDI(threadsafe oWindow* _pWindow, const DESC* _pDesc)
		: Window(_pWindow)
		, Desc(*_pDesc)
		, hPen(0)
		, hBrush(0)
	{
		UpdatePen(_pDesc->BorderColor);
		UpdateBrush(_pDesc->Color);
		Register();
		static_cast<threadsafe oWindow_Impl*>(Window.c_ptr())->RoundedBoxes.push_back(this);
	}

	~RoundedBoxGDI()
	{
		static_cast<threadsafe oWindow_Impl*>(Window.c_ptr())->RoundedBoxes.erase(this);
		Unregister();

		if (hPen) DeleteObject(hPen);
		if (hBrush) DeleteObject(hBrush);
	}

	bool Open() override { return true; }
	void Close() override {}
	bool Begin() override { return true; }
	void End() override {}
	bool HandleMessage(void* _pPlatformData) override { return true; }
	void GetDesc(DESC* _pDesc) const threadsafe override { oMutex::ScopedLock lock(Mutex); *_pDesc = thread_cast<DESC&>(Desc); }
	void SetDesc(DESC* _pDesc) threadsafe override
	{
		oMutex::ScopedLock lock(Mutex); 
		
		// Minimize the dirty rect
		{
			HWND hWnd = (HWND)Window->GetNativeHandle();
			RECT rClient, rOld, rNew;
			GetClientRect(hWnd, &rClient);

			char oldOpts[5] = { 'f',0,0,0,0 };
			ConvertAlignment(oldOpts+1, Desc.Anchor);

			char newOpts[5] = { 'f',0,0,0,0 };
			ConvertAlignment(newOpts+1, _pDesc->Anchor);

			RECT rDesc = oBuildRECTWH(Desc.X, Desc.Y, Desc.Width, Desc.Height), rpDesc = oBuildRECTWH(_pDesc->X, _pDesc->Y, _pDesc->Width, _pDesc->Height);
			oAdjustRect(&rClient, &rDesc, &rOld, Scale(), oldOpts);
			oAdjustRect(&rClient, &rpDesc, &rNew, Scale(), newOpts);

			if (memcmp(&rOld, &rNew, sizeof(RECT)))
			{
				rNew = oBuildRECT(__min(rOld.left, rNew.left), __min(rOld.top, rNew.top), __max(rOld.right, rNew.right), __max(rOld.bottom, rNew.bottom));
				InvalidateRect(hWnd, 0/*&rNew*/, TRUE); // @oooii-tony: This only seems to update if the whole client is invalidated. Why?
			}
		}
		
		thread_cast<RoundedBoxGDI*>(this)->UpdatePen(_pDesc->BorderColor);
		thread_cast<RoundedBoxGDI*>(this)->UpdateBrush(_pDesc->Color);
		
		thread_cast<DESC&>(Desc) = *_pDesc;
	}

	void HandlePaint(void* _pPlatformData) override
	{
		WINPAINT_DESC* p = static_cast<WINPAINT_DESC*>(_pPlatformData);
		oMutex::ScopedLock lock(Mutex);

		oGDIScopedSelect SelectPen(p->hDC, hPen);
		oGDIScopedSelect SelectBrush(p->hDC, hBrush);

		RECT rClient, rAdjusted;
		HWND hWnd = (HWND)Window->GetNativeHandle();
		GetClientRect(hWnd, &rClient);

		char opts[5] = { 'f',0,0,0,0 };
		ConvertAlignment(opts+1, Desc.Anchor);

		RECT rDesc = oBuildRECTWH(Desc.X, Desc.Y, Desc.Width, Desc.Height);
		oAdjustRect(&rClient, &rDesc, &rAdjusted, Scale(), opts);

		int radius = static_cast<int>(p->Scale * Desc.Roundness * 2.5f);
		RoundRect(p->hDC, rAdjusted.left, rAdjusted.top, rAdjusted.right, rAdjusted.bottom, radius, radius);
	}

	oRef<threadsafe oWindow> Window;
	HPEN hPen;
	HBRUSH hBrush;
	DESC Desc;
	mutable oMutex Mutex;
	oRefCount RefCount;

	void UpdatePen(oColor _NewBorderColor)
	{
		if (!hPen || Desc.BorderColor != _NewBorderColor)
		{
			if (hPen) DeleteObject(hPen);
			hPen = oGDICreatePen(_NewBorderColor, Scale());
			Desc.BorderColor = _NewBorderColor;
		}
	}

	void UpdateBrush(oColor _NewFillColor)
	{
		if (!hBrush || Desc.Color != _NewFillColor)
		{
			if (hBrush) DeleteObject(hBrush);
			hBrush = oGDICreateBrush(_NewFillColor);
			Desc.Color = _NewFillColor;
		}
	}
};

struct LineGDI : public oWindow::Line
{
	oDEFINE_REFCOUNT_INTERFACE(RefCount);
	oDEFINE_TRIVIAL_QUERYINTERFACE(oGetGUID<LineGDI>());
	oDEFINE_GETINTERFACE_INTERFACE(threadsafe, GetWindow, threadsafe, oWindow, Window);

	inline int Scale() threadsafe { return static_cast<threadsafe oWindow_Impl*>(Window.c_ptr())->GDIAntialiasingMultiplier; }

	LineGDI(threadsafe oWindow* _pWindow, const DESC* _pDesc)
		: Window(_pWindow)
		, Desc(*_pDesc)
		, hPen(0)
	{
		UpdatePen(_pDesc->Color, _pDesc->Thickness);
		Register();
		static_cast<threadsafe oWindow_Impl*>(Window.c_ptr())->Lines.push_back(this);
	}

	~LineGDI()
	{
		static_cast<threadsafe oWindow_Impl*>(Window.c_ptr())->Lines.erase(this);
		Unregister();

		if (hPen) DeleteObject(hPen);
	}

	bool Open() override { return true; }
	void Close() override {}
	bool Begin() override { return true; }
	void End() override {}
	bool HandleMessage(void* _pPlatformData) override { return true; }
	void GetDesc(DESC* _pDesc) const threadsafe override { oMutex::ScopedLock lock(Mutex); *_pDesc = thread_cast<DESC&>(Desc); }
	void SetDesc(DESC* _pDesc) threadsafe override
	{
		oMutex::ScopedLock lock(Mutex);
		thread_cast<LineGDI*>(this)->UpdatePen(_pDesc->Color, _pDesc->Thickness);
		thread_cast<DESC&>(Desc) = *_pDesc;
	}

	void HandlePaint(void* _pPlatformData) override
	{
		WINPAINT_DESC* p = static_cast<WINPAINT_DESC*>(_pPlatformData);
		oMutex::ScopedLock lock(Mutex);

		oGDIScopedSelect pen(p->hDC, hPen);
		POINT pt[2];
		pt[0].x = p->Scale * Desc.X1; pt[0].y = p->Scale * Desc.Y1;
		pt[1].x = p->Scale * Desc.X2; pt[1].y = p->Scale * Desc.Y2;
		Polyline(p->hDC, pt, 2);
	}

	oRef<threadsafe oWindow> Window;
	HPEN hPen;
	DESC Desc;
	mutable oMutex Mutex;
	oRefCount RefCount;

	void UpdatePen(oColor _NewColor, int _Thickness)
	{
		if (!hPen || Desc.Color != _NewColor || Desc.Thickness != _Thickness)
		{
			if (hPen) DeleteObject(hPen);
			hPen = oGDICreatePen(_NewColor, _Thickness * Scale());
			Desc.Color = _NewColor;
			Desc.Thickness = _Thickness;
		}
	}
};

struct FontGDI : public oWindow::Font
{
	oDEFINE_REFCOUNT_INTERFACE(RefCount);
	oDEFINE_TRIVIAL_QUERYINTERFACE(oGetGUID<FontGDI>());
	oDEFINE_GETINTERFACE_INTERFACE(threadsafe, GetWindow, threadsafe, oWindow, Window);

	FontGDI(threadsafe oWindow* _pWindow, const DESC* _pDesc)
		: Window(_pWindow)
		, Desc(*_pDesc)
		, hFont(0)
	{
		HWND hWnd = (HWND)Window->GetNativeHandle();
		HDC hdc = GetDC(hWnd);
		hFont = oGDICreateFont(
			_pDesc->FontName
			, static_cast<int>(_pDesc->PointSize)
			, _pDesc->Style == BOLD || _pDesc->Style == BOLDITALIC
			, _pDesc->Style == ITALIC || _pDesc->Style == BOLDITALIC
			, false);

		#ifdef _DEBUG
			// Use this to get the actual font used
			oGDIScopedSelect font(hdc, hFont);
			TEXTMETRIC tm;
			oVB(GetTextMetrics(hdc, &tm));
			// Here's what's requested of a device context
			LOGFONT lf;
			GetObject(hFont, sizeof(LOGFONT), &lf);
			oTRACE("GDIFont Created: %s %s%s %s %s", lf.lfFaceName, tm.tmWeight == FW_NORMAL ? "" : "bold", tm.tmItalic ? "italic" : "", oGDIGetCharSet(tm.tmCharSet), oGDIGetFontFamily(tm.tmPitchAndFamily));
		#endif
		ReleaseDC(hWnd, hdc);
		Register();
	}

	~FontGDI()
	{
		DeleteObject(hFont);
		Unregister();
	}

	bool Open() override { return true; }
	void Close() override {}
	bool Begin() override { return true; }
	void End() override {}
	void HandlePaint(void* _pPlatformData) override {}
	bool HandleMessage(void* _pPlatformData) override { return true; }
	void GetDesc(DESC* _pDesc) const threadsafe { *_pDesc = thread_cast<DESC&>(Desc); }

	oRef<threadsafe oWindow> Window;
	HFONT hFont;
	DESC Desc;
	oRefCount RefCount;
};

struct TextGDI : public oWindow::Text
{
	oDEFINE_REFCOUNT_INTERFACE(RefCount);
	oDEFINE_TRIVIAL_QUERYINTERFACE(oGetGUID<TextGDI>());
	oDEFINE_GETINTERFACE_INTERFACE(threadsafe, GetWindow, threadsafe, oWindow, Window);

	TextGDI(threadsafe oWindow* _pWindow, threadsafe oWindow::Font* _pFont, const DESC* _pDesc)
		: Window(_pWindow)
		, Font(_pFont)
		, Desc(*_pDesc)
		, StringLength(0)
	{
		*String = 0;
		Register();
		static_cast<threadsafe oWindow_Impl*>(Window.c_ptr())->Texts.push_back(this);
	}

	~TextGDI()
	{
		static_cast<threadsafe oWindow_Impl*>(Window.c_ptr())->Texts.erase(this);
		Unregister();
	}

	bool Open() override { return true; }
	void Close() override {}
	bool Begin() override { return true; }
	void End() override {}
	bool HandleMessage(void* _pPlatformData) override { return true; }

	void GetDesc(DESC* _pDesc) const threadsafe override { oMutex::ScopedLock lock(Mutex); *_pDesc = thread_cast<DESC&>(Desc); }
	void SetDesc(const DESC* _pDesc) threadsafe override
	{
		oMutex::ScopedLock lock(Mutex);
		TextGDI* pThis = thread_cast<TextGDI*>(this);
		pThis->Desc = *_pDesc;
		HWND hWnd = (HWND)Window->GetNativeHandle();
		InvalidateRect(hWnd, 0, TRUE);
	}

	void SetFont(threadsafe oWindow::Font* _pFont) threadsafe
	{
		oMutex::ScopedLock lock(Mutex);
		Font = _pFont;
		HWND hWnd = (HWND)Window->GetNativeHandle();
		InvalidateRect(hWnd, 0, TRUE);
	}

	void HandlePaint(void* _pPlatformData) override
	{
		WINPAINT_DESC* p = static_cast<WINPAINT_DESC*>(_pPlatformData);
		oMutex::ScopedLock lock(Mutex);

		HWND hWnd = (HWND)Window->GetNativeHandle();

		RECT rClient, rDesc = oBuildRECTWH(Desc.X, Desc.Y, Desc.Width, Desc.Height), rAdjusted;
		GetClientRect(hWnd, &rClient);
		char opts[5] = { 'f',0,0,0,0 };
		ConvertAlignment(opts+1, Desc.Anchor);
		oAdjustRect(&rClient, &rDesc, &rAdjusted, 1, opts);
		
		oGDIScopedSelect font(p->hDC, static_cast<threadsafe FontGDI*>(Font.c_ptr())->hFont);
		
		opts[0] = 's';
		ConvertAlignment(opts+1, Desc.Alignment);

		if (Desc.ShadowColor)
		{
			oWindow::Font::DESC fdesc;
			Font->GetDesc(&fdesc);
			int offset = static_cast<int>(round(fdesc.ShadowOffset));
			if (offset)
			{
				RECT rShadow = rAdjusted;
				rShadow.left += offset; rShadow.top += offset; rShadow.right += offset; rShadow.bottom += offset;
				oVB(oGDIDrawText(p->hDC, &rShadow, Desc.ShadowColor, 0, opts, Desc.String));
			}
		}

		oVB(oGDIDrawText(p->hDC, &rAdjusted, Desc.Color, 0, opts, Desc.String));
	}

	oRef<threadsafe oWindow> Window;
	oRef<threadsafe oWindow::Font> Font;
	DESC Desc;
	mutable oMutex Mutex;
	oRefCount RefCount;
	WCHAR String[1024];
	UINT StringLength;
};

struct PictureGDI : public oWindow::Picture
{
	oDEFINE_REFCOUNT_INTERFACE(RefCount);
	oDEFINE_TRIVIAL_QUERYINTERFACE(oGetGUID<PictureGDI>());
	oDEFINE_GETINTERFACE_INTERFACE(threadsafe, GetWindow, threadsafe, oWindow, Window);

	inline int Scale() threadsafe { return static_cast<threadsafe oWindow_Impl*>(Window.c_ptr())->GDIAntialiasingMultiplier; }

	PictureGDI(threadsafe oWindow* _pWindow, const Picture::DESC* _pDesc)
		: Window(_pWindow)
		, Desc(*_pDesc)
		, hBitmap(0)
		, hFlipBitmap(0)
	{
		HWND hWnd = (HWND)Window->GetNativeHandle();
		HDC hdc = GetDC(hWnd);
		hBitmap = CreateCompatibleBitmap(hdc, _pDesc->SurfaceDesc.Width, _pDesc->SurfaceDesc.Height);

		// @oooii-tony: just in case the user wants to flip horizontally and 
		// update frequently, such as from a video feed. Not the greatest, but 
		// a little memory is worth it to save the perf hit in a frequent-update
		// loop.
		hFlipBitmap = CreateCompatibleBitmap(hdc, _pDesc->SurfaceDesc.Width, _pDesc->SurfaceDesc.Height);

		ReleaseDC(hWnd, hdc);
		oASSERT(hBitmap, "");
		Register();
		static_cast<threadsafe oWindow_Impl*>(Window.c_ptr())->Pictures.push_back(this);
	}
	
	~PictureGDI()
	{
		static_cast<threadsafe oWindow_Impl*>(Window.c_ptr())->Pictures.erase(this);
		Unregister();

		if (hBitmap) DeleteObject(hBitmap);
		if (hFlipBitmap) DeleteObject(hFlipBitmap);
	}

	bool Open() override
	{
		return true;
	}

	void Close() override
	{
	}

	bool Begin() override { return true; }
	void End() override {}

	bool HandleMessage(void* _pPlatformData) override { return true; }

	void GetDesc(DESC* _pDesc) const threadsafe
	{
		*_pDesc = thread_cast<DESC&>(Desc);
	}

	void Copy(const void* _pSourceData, size_t _SourcePitch, bool _FlipHorizontally, bool _FlipVertically) threadsafe
	{
		if (hBitmap)
		{
			HWND hWnd = (HWND)Window->GetNativeHandle();
			oMutex::ScopedLock lock(BitmapLock);
			// Always allocate enough memory for 8-bit formats and avoid a heap 
			// alloc below
			BITMAPINFO* pBMI = (BITMAPINFO*)_alloca(oGetBMISize(Desc.SurfaceDesc.Format));
			oAllocateBMI(&pBMI, thread_cast<oSurface::DESC*>(&Desc.SurfaceDesc), 0, !_FlipVertically); // Windows is already flipping it, so if you want to flip it again, undo that
			HDC hdc = GetDC(hWnd);
			HDC htemp = CreateCompatibleDC(hdc);

			int w = _FlipHorizontally ? -(int)Desc.SurfaceDesc.Width : Desc.SurfaceDesc.Width;
			int h = _FlipVertically ? Desc.SurfaceDesc.Height : -(int)Desc.SurfaceDesc.Height;

			if (!_FlipHorizontally && !_FlipVertically)
				SetDIBits(htemp, hBitmap, 0, Desc.SurfaceDesc.Height, _pSourceData, pBMI, DIB_RGB_COLORS);
			else
			{
				SetDIBits(htemp, hFlipBitmap, 0, Desc.SurfaceDesc.Height, _pSourceData, pBMI, DIB_RGB_COLORS);
				SelectObject(htemp, hBitmap);
				oGDIStretchBitmap(htemp, w < 0 ? Desc.SurfaceDesc.Width : 0, h < 0 ? Desc.SurfaceDesc.Height : 0, w, h, hFlipBitmap, SRCCOPY);
			}
			
			DeleteDC(htemp);
			ReleaseDC(hWnd, hdc);

			InvalidateRect(hWnd, 0, FALSE);
		}
	}

	void HandlePaint(void* _pPlatformData) override
	{
		WINPAINT_DESC* p = static_cast<WINPAINT_DESC*>(_pPlatformData);

		HWND hWnd = (HWND)Window->GetNativeHandle();
		RECT rClient, rDesc = oBuildRECTWH(Desc.X, Desc.Y, Desc.Width, Desc.Height), rAdjusted;
		GetClientRect(hWnd, &rClient);
		oAdjustRect(&rClient, &rDesc, &rAdjusted, p->Scale, 0);

		oGDIStretchBitmap(p->hDC, rAdjusted.left, rAdjusted.top, rAdjusted.right-rAdjusted.left, rAdjusted.bottom-rAdjusted.top, hBitmap, SRCCOPY);
	}

	oRef<threadsafe oWindow> Window;
	HBITMAP hBitmap;
	HBITMAP hFlipBitmap;
	oMutex BitmapLock;
	DESC Desc;
	oRefCount RefCount;
};

#if oDXVER >= oDXVER_10

	bool CreateBrush(oColor _Color, ID2D1RenderTarget* _pRenderTarget, ID2D1SolidColorBrush** _ppBrush)
	{
		if (!_pRenderTarget || !_ppBrush) return false;
		if (_Color)
		{
			float r,g,b,a;
			oDecomposeColor(_Color, &r, &g, &b, &a);
			ID2D1SolidColorBrush* pBrush = 0;
			oV(_pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(r, g, b, a), &pBrush));
			*_ppBrush = pBrush;
		}

		else
			*_ppBrush = 0;

		return !!*_ppBrush;
	}

	void GetAlignment(oWindow::ALIGNMENT _Alignment, DWRITE_PARAGRAPH_ALIGNMENT* _pParagraphAlignment, DWRITE_TEXT_ALIGNMENT* _pTextAlignment)
	{
		static DWRITE_PARAGRAPH_ALIGNMENT sParagraphAlignments[] = 
		{
			DWRITE_PARAGRAPH_ALIGNMENT_NEAR,
			DWRITE_PARAGRAPH_ALIGNMENT_CENTER,
			DWRITE_PARAGRAPH_ALIGNMENT_FAR,
		};

		static DWRITE_TEXT_ALIGNMENT sTextAlignments[] = 
		{
			DWRITE_TEXT_ALIGNMENT_LEADING,
			DWRITE_TEXT_ALIGNMENT_CENTER,
			DWRITE_TEXT_ALIGNMENT_TRAILING,
		};

		*_pParagraphAlignment = sParagraphAlignments[_Alignment / 3];
		*_pTextAlignment = sTextAlignments[_Alignment % 3];
	}

	void GetAdjustedRect(HWND _hWnd, int _X, int _Y, unsigned int _Width, unsigned _Height, oWindow::ALIGNMENT _Anchor, D2D1_RECT_F* pAdjustedRect)
	{
		RECT rClient;
		GetClientRect(_hWnd, &rClient);

		int x = 0, y = 0;

		RECT rClipped;
		rClipped.left = __max(_X, rClient.left);
		rClipped.top = __max(_Y, rClient.top);
		rClipped.right = _Width == oWindow::DEFAULT ? rClient.right : __min(rClipped.left + (LONG)_Width, rClient.right);
		rClipped.bottom = _Height == oWindow::DEFAULT ? rClient.bottom : __min(rClipped.top + (LONG)_Height, rClient.bottom);

		DWRITE_PARAGRAPH_ALIGNMENT paragraphAlignment;
		DWRITE_TEXT_ALIGNMENT textAlignment;
		GetAlignment(_Anchor, &paragraphAlignment, &textAlignment);

		switch (paragraphAlignment)
		{
			case DWRITE_PARAGRAPH_ALIGNMENT_CENTER: y = ((rClient.bottom - rClient.top) - (rClipped.bottom - rClipped.top)) / 2; break;
			case DWRITE_PARAGRAPH_ALIGNMENT_FAR: y = rClient.bottom - (rClipped.bottom - rClipped.top); break;
			default: break;
		}

		switch (textAlignment)
		{
			case DWRITE_TEXT_ALIGNMENT_CENTER: x = ((rClient.right - rClient.left) - (rClipped.right - rClipped.left)) / 2; break;
			case DWRITE_TEXT_ALIGNMENT_TRAILING: x = rClient.right - (rClipped.right - rClipped.left); break;
			default: break;
		}

		x += (_X == oWindow::DEFAULT) ? 0 : _X;
		y += (_Y == oWindow::DEFAULT) ? 0 : _Y;

		float scaleX, scaleY;
		oGetScreenDPIScale(&scaleX, &scaleY);

		*pAdjustedRect = D2D1::RectF(
			static_cast<FLOAT>(x) / scaleX
			, static_cast<FLOAT>(y) / scaleY
			, static_cast<FLOAT>(x + (rClipped.right - rClipped.left)) / scaleX
			, static_cast<FLOAT>(y + (rClipped.bottom - rClipped.top)) / scaleY
			);
	}



	struct RoundedBoxD2D : public oWindow::RoundedBox
	{
		oDEFINE_REFCOUNT_INTERFACE(RefCount);
		oDEFINE_TRIVIAL_QUERYINTERFACE(oGetGUID<RoundedBoxD2D>());
		oDEFINE_GETINTERFACE_INTERFACE(threadsafe, GetWindow, threadsafe, oWindow, Window);

		RoundedBoxD2D(threadsafe oWindow_Impl* _pWindow, const DESC* _pDesc)
			: Window(_pWindow)
			, Desc(*_pDesc)
		{
			Register();
			static_cast<threadsafe oWindow_Impl*>(Window.c_ptr())->RoundedBoxes.push_back(this);
		}

		~RoundedBoxD2D()
		{
			static_cast<threadsafe oWindow_Impl*>(Window.c_ptr())->RoundedBoxes.erase(this);
			Unregister();
		}

		bool Open() override
		{
			return CreateBrush(Desc.Color, Window->RenderTarget, &Brush) && CreateBrush(Desc.BorderColor, Window->RenderTarget, &BorderBrush);
		}

		void Close() override { Brush = 0; BorderBrush = 0; }
		bool Begin() override { return true; }
		void End() override {}
		bool HandleMessage(void* _pPlatformData) override { return true; }
		void GetDesc(DESC* _pDesc) const threadsafe override { oMutex::ScopedLock lock(Mutex); *_pDesc = thread_cast<DESC&>(Desc); }
		void SetDesc(DESC* _pDesc) threadsafe override
		{
			oMutex::ScopedLock lock(Mutex);
			HWND hWnd = (HWND)Window->GetNativeHandle();

			if (Desc.BorderColor != _pDesc->BorderColor)
			{
				oRef<ID2D1SolidColorBrush> newBrush;
				CreateBrush(_pDesc->BorderColor, 
					Window->RenderTarget, 
					&newBrush);
				BorderBrush = newBrush;
			}
			if (Desc.Color != _pDesc->Color)
			{
				oRef<ID2D1SolidColorBrush> newBrush;
				CreateBrush(_pDesc->Color, 
					Window->RenderTarget,
					&newBrush);
				Brush = newBrush;
			}
			thread_cast<DESC&>(Desc) = *_pDesc; // safe because of the lock above
			InvalidateRect(hWnd, 0, TRUE);
		}

		void HandlePaint(void* _pPlatformData) override
		{
			oMutex::ScopedLock lock(Mutex);
			HWND hWnd = (HWND)Window->GetNativeHandle();

			D2D1_RECT_F rAdjusted;
			GetAdjustedRect(hWnd, Desc.X, Desc.Y, Desc.Width, Desc.Height, Desc.Anchor, &rAdjusted);
			oASSERT(Window->RenderTarget, "No render target");
			if (oEqual(Desc.Roundness, 0.0f))
			{
				if (!oIsTransparentColor(Desc.Color))
					Window->RenderTarget->FillRectangle(&rAdjusted, Brush);
				if (!oIsTransparentColor(Desc.BorderColor))
					Window->RenderTarget->DrawRectangle(&rAdjusted, BorderBrush);
			}

			else
			{
				D2D1_ROUNDED_RECT r;
				r.rect = rAdjusted;
				r.radiusX = Desc.Roundness;
				r.radiusY = Desc.Roundness;
				if (!oIsTransparentColor(Desc.Color))
					Window->RenderTarget->FillRoundedRectangle(&r, Brush);
				if (!oIsTransparentColor(Desc.BorderColor))
					Window->RenderTarget->DrawRoundedRectangle(&r, BorderBrush);
			}
		}

		oRef<threadsafe oWindow_Impl> Window;
		oRef<ID2D1SolidColorBrush> Brush;
		oRef<ID2D1SolidColorBrush> BorderBrush;
		DESC Desc;
		mutable oMutex Mutex;
		oRefCount RefCount;
	};

	struct LineD2D : public oWindow::Line
	{
		oDEFINE_REFCOUNT_INTERFACE(RefCount);
		oDEFINE_TRIVIAL_QUERYINTERFACE(oGetGUID<LineD2D>());
		oDEFINE_GETINTERFACE_INTERFACE(threadsafe, GetWindow, threadsafe, oWindow, Window);

		LineD2D(threadsafe oWindow_Impl* _pWindow, const DESC* _pDesc)
			: Window(_pWindow)
			, Desc(*_pDesc)
		{
			Register();
			static_cast<threadsafe oWindow_Impl*>(Window.c_ptr())->Lines.push_back(this);
		}

		~LineD2D()
		{
			static_cast<threadsafe oWindow_Impl*>(Window.c_ptr())->Lines.erase(this);
			Unregister();
		}

		bool Open() override
		{
			return CreateBrush(Desc.Color, Window->RenderTarget, &Brush) && CreateBrush(Desc.Color, Window->RenderTarget, &Brush);
		}

		void Close() override { Brush = 0;}
		bool Begin() override { return true; }
		void End() override {}
		bool HandleMessage(void* _pPlatformData) override { return true; }
		void GetDesc(DESC* _pDesc) const threadsafe override { oMutex::ScopedLock lock(Mutex); *_pDesc = thread_cast<DESC&>(Desc); }
		void SetDesc(DESC* _pDesc) threadsafe override
		{
			oMutex::ScopedLock lock(Mutex);
			HWND hWnd = (HWND)Window->GetNativeHandle();

			//@oooii-Andrew: using thread_cast because I'm locking above
			if (Desc.Color != _pDesc->Color)
			{
				oRef<ID2D1SolidColorBrush> newBrush;
				CreateBrush(_pDesc->Color, Window->RenderTarget, &newBrush);
				Brush = newBrush;
			}
			thread_cast<DESC&>(Desc) = *_pDesc;
			InvalidateRect(hWnd, 0, TRUE);
		}

		void HandlePaint(void* _pPlatformData) override
		{
			oMutex::ScopedLock lock(Mutex);
			oASSERT(Window->RenderTarget, "No render target");
			if (!oIsTransparentColor(Desc.Color))
			{
				D2D1_POINT_2F p0, p1;
				p0.x = static_cast<float>(Desc.X1); p0.y = static_cast<float>(Desc.Y1);
				p1.x = static_cast<float>(Desc.X2); p1.y = static_cast<float>(Desc.Y2);
				Window->RenderTarget->DrawLine(p0, p1, Brush, static_cast<float>(Desc.Thickness));
			}
		}

		oRef<threadsafe oWindow_Impl> Window;
		oRef<ID2D1SolidColorBrush> Brush;
		DESC Desc;
		mutable oMutex Mutex;
		oRefCount RefCount;
	};


	struct FontD2D : public oWindow::Font
	{
		oDEFINE_REFCOUNT_INTERFACE(RefCount);
		oDEFINE_TRIVIAL_QUERYINTERFACE(oGetGUID<FontD2D>());
		oDEFINE_GETINTERFACE_INTERFACE(threadsafe, GetWindow, threadsafe, oWindow, Window);

		FontD2D(threadsafe oWindow* _pWindow, const DESC* _pDesc)
			: Window(_pWindow)
			, Desc(*_pDesc)
		{
			WCHAR NAME[64];
			oStrConvert(NAME, _pDesc->FontName);

			oV(oGetDWriteFactorySingleton()->CreateTextFormat(
				NAME
				, 0
				, (_pDesc->Style == BOLD || _pDesc->Style == BOLDITALIC) ? DWRITE_FONT_WEIGHT_BOLD : DWRITE_FONT_WEIGHT_NORMAL
				, (_pDesc->Style == ITALIC || _pDesc->Style == BOLDITALIC) ? DWRITE_FONT_STYLE_ITALIC : DWRITE_FONT_STYLE_NORMAL
				, DWRITE_FONT_STRETCH_NORMAL
				, oPointToDIP(_pDesc->PointSize)
				, L"en-us"
				, &Format
				));

			oV(Format->GetFontFamilyName(NAME, oCOUNTOF(NAME)));
			char fontFamilyName[64];
			oStrConvert(fontFamilyName, NAME);
			wchar_t LOCALE[64];
			oV(Format->GetLocaleName(LOCALE, oCOUNTOF(LOCALE)));
			char locale[64];
			oStrConvert(locale, LOCALE);
			oTRACE("DWriteFont Created: %s %s%s %s", fontFamilyName, Format->GetFontWeight() == DWRITE_FONT_WEIGHT_REGULAR ? "" : "bold", Format->GetFontStyle() == DWRITE_FONT_STYLE_ITALIC ? "italic" : "", locale);

			Register();
		}

		~FontD2D()
		{
			Unregister();
		}

		bool Open() override { return true; }
		void Close() override {}
		bool Begin() override { return true; }
		void End() override {}
		void HandlePaint(void* _pPlatformData) override {}
		bool HandleMessage(void* _pPlatformData) override { return true; }
		void GetDesc(DESC* _pDesc) const threadsafe { *_pDesc = thread_cast<DESC&>(Desc); }
		IDWriteTextFormat* GetFormat() threadsafe { return thread_cast<IDWriteTextFormat*>(Format.c_ptr()); }

		oRef<threadsafe oWindow> Window;
		oRef<IDWriteTextFormat> Format;
		DESC Desc;
		oRefCount RefCount;
	};

	struct TextD2D : public oWindow::Text
	{
		oDEFINE_REFCOUNT_INTERFACE(RefCount);
		oDEFINE_TRIVIAL_QUERYINTERFACE(oGetGUID<TextD2D>());
		oDEFINE_GETINTERFACE_INTERFACE(threadsafe, GetWindow, threadsafe, oWindow, Window);

		TextD2D(threadsafe oWindow_Impl* _pWindow, threadsafe oWindow::Font* _pFont, const DESC* _pDesc)
			: Window(_pWindow)
			, Font(_pFont)
			, Desc(*_pDesc)
			, StringLength(0)
		{
			*String = 0;
			TextD2D::SetDesc(_pDesc);
			Register();
			static_cast<threadsafe oWindow_Impl*>(Window.c_ptr())->Texts.push_back(this);
		}

		~TextD2D()
		{
			static_cast<threadsafe oWindow_Impl*>(Window.c_ptr())->Texts.erase(this);
			Unregister();
		}

		bool Open() override
		{
			return CreateBrush(Desc.Color, Window->RenderTarget, &Brush) && CreateBrush(Desc.ShadowColor, Window->RenderTarget, &ShadowBrush);
		}

		void Close() override { Brush = 0; ShadowBrush = 0; }
		bool Begin() override { return true; }
		void End() override {}
		bool HandleMessage(void* _pPlatformData) override { return true; }

		void GetDesc(DESC* _pDesc) const threadsafe override { oMutex::ScopedLock lock(Mutex); *_pDesc = thread_cast<DESC&>(Desc); }
		void SetDesc(const DESC* _pDesc) threadsafe override
		{
			oMutex::ScopedLock lock(Mutex);
			HWND hWnd = (HWND)Window->GetNativeHandle();

			TextD2D* pThis = thread_cast<TextD2D*>(this);
			pThis->Desc = *_pDesc;
			if (Desc.Color != _pDesc->Color)
				CreateBrush(_pDesc->Color, Window->RenderTarget, &pThis->Brush);
			if (_pDesc->ShadowColor != Desc.ShadowColor)
				CreateBrush(_pDesc->ShadowColor, Window->RenderTarget, &pThis->ShadowBrush);
			StringLength = static_cast<UINT>(oStrConvert(pThis->String, _pDesc->String));
			InvalidateRect(hWnd, 0, TRUE);
		}

		void SetFont(threadsafe oWindow::Font* _pFont) threadsafe
		{
			oMutex::ScopedLock lock(Mutex);
			HWND hWnd = (HWND)Window->GetNativeHandle();
			Font = _pFont;
			InvalidateRect(hWnd, 0, TRUE);
		}

		void HandlePaint(void* _pPlatformData) override
		{
			D2D1_RECT_F rAdjusted;
			HWND hWnd = (HWND)Window->GetNativeHandle();
			GetAdjustedRect(hWnd, Desc.X, Desc.Y, Desc.Width, Desc.Height, Desc.Anchor, &rAdjusted);

			DWRITE_PARAGRAPH_ALIGNMENT paragraphAlignment;
			DWRITE_TEXT_ALIGNMENT textAlignment;
			GetAlignment(Desc.Alignment, &paragraphAlignment, &textAlignment);

			oASSERT(Window->RenderTarget, "No render target");

			oMutex::ScopedLock lock(Mutex);

			IDWriteTextFormat* pFormat = static_cast<threadsafe FontD2D*>(Font.c_ptr())->GetFormat();
			pFormat->SetParagraphAlignment(paragraphAlignment);
			pFormat->SetTextAlignment(textAlignment);
			pFormat->SetWordWrapping(Desc.MultiLine ? DWRITE_WORD_WRAPPING_WRAP : DWRITE_WORD_WRAPPING_NO_WRAP);

			if (ShadowBrush)
			{
				oWindow::Font::DESC fdesc;
				Font->GetDesc(&fdesc);
				D2D1_RECT_F rShadow = rAdjusted;
				rShadow.left += fdesc.ShadowOffset;
				rShadow.top += fdesc.ShadowOffset;
				rShadow.right += fdesc.ShadowOffset;
				rShadow.bottom += fdesc.ShadowOffset;
				Window->RenderTarget->DrawText(String, StringLength, pFormat, rShadow, ShadowBrush);
			}

			oASSERT(Brush, "No brush created");
			Window->RenderTarget->DrawText(String, StringLength, pFormat, rAdjusted, Brush);
		}

		oRef<threadsafe oWindow_Impl> Window;
		oRef<threadsafe oWindow::Font> Font;
		oRef<ID2D1SolidColorBrush> Brush;
		oRef<ID2D1SolidColorBrush> ShadowBrush;
		DESC Desc;
		mutable oMutex Mutex;
		oRefCount RefCount;
		WCHAR String[1024];
		UINT StringLength;
	};

	struct PictureD2D : public oWindow::Picture
	{
		oDEFINE_REFCOUNT_INTERFACE(RefCount);
		oDEFINE_TRIVIAL_QUERYINTERFACE(oGetGUID<PictureD2D>());
		oDEFINE_GETINTERFACE_INTERFACE(threadsafe, GetWindow, threadsafe, oWindow, Window);

		PictureD2D(threadsafe oWindow_Impl* _pWindow, const Picture::DESC* _pDesc)
			: Window(_pWindow)
			, Desc(*_pDesc)
		{
			Register();
			oASSERT(!_pWindow->IsOpen() || Bitmap, "Something failed in PictureD2D construction");
			static_cast<threadsafe oWindow_Impl*>(Window.c_ptr())->Pictures.push_back(this);
		}
		~PictureD2D()
		{
			static_cast<threadsafe oWindow_Impl*>(Window.c_ptr())->Pictures.erase(this);
			Unregister();
		}

		bool Open() override
		{
			D2D1_SIZE_U size = { Desc.SurfaceDesc.Width, Desc.SurfaceDesc.Height };
			D2D1_BITMAP_PROPERTIES properties;
			properties.pixelFormat.format = DXGI_FORMAT_B8G8R8A8_UNORM;
			if (properties.pixelFormat.format == DXGI_FORMAT_UNKNOWN)
				return false;
			properties.pixelFormat.alphaMode = D2D1_ALPHA_MODE_IGNORE;


			Window->Factory->GetDesktopDpi(&properties.dpiX, &properties.dpiY);
			oV(Window->RenderTarget->CreateBitmap(size, 0, 0, &properties, &Bitmap));
			return !!Bitmap;
		}

		void Close() override
		{
			Bitmap = 0;
		}

		bool Begin() override { return true; }
		void End() override {}

		bool HandleMessage(void* _pPlatformData) override { return true; }

		void GetDesc(DESC* _pDesc) const threadsafe
		{
			*_pDesc = thread_cast<DESC&>(Desc);
		}

		void Copy(const void* _pSourceData, size_t _SourcePitch, bool _FlipHorizontally, bool _FlipVertically) threadsafe
		{
			if (Bitmap)
			{
				HWND hWnd = (HWND)Window->GetNativeHandle();
				oMutex::ScopedLock lock(BitmapLock);
				thread_cast<ID2D1Bitmap*>(Bitmap.c_ptr())->CopyFromMemory(0, _pSourceData, (UINT32)_SourcePitch);
				FlippedHorizontally = _FlipHorizontally;
				FlippedVertically = _FlipVertically;
				InvalidateRect(hWnd, 0, FALSE); // @oooii-tony: This could be resized to just the area of the bitmap
			}
		}

		void HandlePaint(void* _pPlatformData) override
		{
			HWND hWnd = (HWND)Window->GetNativeHandle();
			RECT cr;
			GetClientRect(hWnd, &cr);

			D2D1_RECT_F r;
			GetAdjustedRect(hWnd, Desc.X, Desc.Y, Desc.Width, Desc.Height, Desc.Anchor, &r);

			float w = r.right - r.left;
			float h = r.bottom - r.top;

			float centerX = r.left + w / 2.0f;
			float centerY = r.top + h / 2.0f;

			D2D1::Matrix3x2F flipMatrix = D2D1::Matrix3x2F::Identity();

			if (FlippedHorizontally)
			{
				flipMatrix._11 = -1.0f;
				flipMatrix._31 = centerX * 2.0f;
			}

			if (FlippedVertically)
			{
				flipMatrix._22 = -flipMatrix._22;
				flipMatrix._32 = centerY * 2.0f;
			}

			Window->RenderTarget->SetTransform(flipMatrix);
			Window->RenderTarget->DrawBitmap(Bitmap, r);
			Window->RenderTarget->SetTransform(D2D1::Matrix3x2F::Identity());
		}

		oRef<threadsafe oWindow_Impl> Window;
		oRef<ID2D1Bitmap> Bitmap;
		oMutex BitmapLock;
		DESC Desc;
		oRefCount RefCount;
		bool FlippedHorizontally;
		bool FlippedVertically;
	};

	struct VideoD2D : public oWindow::Video
	{
		oDEFINE_REFCOUNT_INTERFACE(RefCount);
		oDEFINE_CONST_GETDESC_INTERFACE(Desc, threadsafe );
		oDEFINE_NOOP_QUERYINTERFACE();
		oDEFINE_GETINTERFACE_INTERFACE(threadsafe, GetWindow, threadsafe, oWindow, Window);

		VideoD2D( threadsafe oWindow_Impl* _pWindow, const DESC* _pDesc, oVideoContainer** _ppVideos, size_t _NumVideos, ID3D10Texture2D* _pBackBuffer, bool* _pSuccess)
			: Window(_pWindow)
			, Desc(*_pDesc)
		{
			*_pSuccess = false;

			bool (__stdcall *CreateDecode)(oVideoDecodeD3D10::DESC _desc, oVideoContainer** _ppContainers, size_t _NumVideos, oVideoDecodeD3D10** _ppDecoder) = NULL;
			// Softlink oVideo
			{
				static const char* DLLName =
#if _DEBUG
					"oVideoD.dll";
#else
					"oVideo.dll";
#endif
				// @oooii-kevin: This is obtained via dumpbin /EXPORTS
				const char* CreateDecodeName = "?Create@oVideoDecodeD3D10@@SA_NUDESC@1@PEAPEAUoVideoContainer@@_KPEAPEAU1@@Z";
				VideoDLL = oModule::Link(DLLName, &CreateDecodeName, (void**)&CreateDecode, 1 );
				if( !VideoDLL )
				{
					oSetLastError(EINVAL, "Can't load %s", DLLName );
					return;
				}
			}


			oVideoDecodeD3D10::DESC desc;
			desc.UseFrameTime = Desc.UseFrameTime;
			desc.StitchVertically = Desc.StitchVertically;
			memcpy(desc.SourceRects, Desc.SourceRects, sizeof(Desc.SourceRects));
			memcpy(desc.DestRects, Desc.DestRects, sizeof(Desc.DestRects));
			desc.AllowCatchUp = Desc.AllowCatchUp;

			if( !CreateDecode( desc, _ppVideos, _NumVideos, &Decoder ) )
				return;

			if( !Decoder->Register( _pBackBuffer ) )
 				return;

			Register();
			Window->Videos.push_back(this);

			*_pSuccess = true;
		}
		~VideoD2D()
		{
			Window->Videos.erase(this);
			Unregister();
			oModule::Unlink(VideoDLL);
		}

		bool Open() override
		{
			return true;
		}

		void Close() override
		{
		}

		bool Begin() override { return true; }
		void End() override {}

		bool HandleMessage(void* _pPlatformData) override { return true; }

		void HandlePaint(void* _pPlatformData) override
		{
			Decoder->Decode( );
		}

		void RegisterDecoder(ID3D10Texture2D* _pBackBuffer)
		{
			Decoder->Register( _pBackBuffer );
		}
		void UnregisterDecoder()
		{
			Decoder->Unregister();
		}

		oRef<threadsafe oWindow_Impl> Window;
		oRef<oVideoDecodeD3D10> Decoder;
		DESC Desc;
		oRefCount RefCount;
		oHMODULE VideoDLL;
	};


#endif

} // namespace detail

bool oWindow::Create(const DESC* _pDesc, void* _pAssociatedNativeHandle, const char* _Title, unsigned int _DrawAPIFourCC, oWindow** _ppWindow)
{
	if (!_pDesc || !_ppWindow)
	{
		oSetLastError(EINVAL);
		return false;
	}

	bool success = false;
	oCONSTRUCT(_ppWindow, oWindow_Impl(*_pDesc, _pAssociatedNativeHandle, _Title, _DrawAPIFourCC, &success));

	if (success)
	{
		// Load/test OOOii lib icon:
		extern void GetDescoooii_ico(const char** ppBufferName, const void** ppBuffer, size_t* pSize);
		const char* BufferName = 0;
		const void* pBuffer = 0;
		size_t bufferSize = 0;
		GetDescoooii_ico(&BufferName, &pBuffer, &bufferSize);

		oRef<oImage> ico;
		oVERIFY(oImage::Create(pBuffer, bufferSize, oSurface::UNKNOWN, &ico));

		HBITMAP hBmp = ico->AsBmp();
		HICON hIcon = oIconFromBitmap(hBmp);
		oVERIFY((*_ppWindow)->SetProperty("Icon", &hIcon));
		DeleteObject(hIcon);
		DeleteObject(hBmp);
	}

	return success;
}

LRESULT CALLBACK oWindow_Impl::StaticWndProc(HWND _hWnd, UINT _uMsg, WPARAM _wParam, LPARAM _lParam)
{
	//char buf[4096];
	//oTRACE("%s", oGetWMDesc(buf, _hWnd, _uMsg, _wParam, _lParam));
	oWindow_Impl* pThis = (oWindow_Impl*)oGetWindowContext(_hWnd, _uMsg, _wParam, _lParam);
	return pThis ? pThis->WndProc(_hWnd, _uMsg, _wParam, _lParam) : DefWindowProc(_hWnd, _uMsg, _wParam, _lParam);
}

oWindow_Impl::oWindow_Impl(const DESC& _Desc, void* _pAssociatedNativeHandle, const char* _Title, unsigned int _FourCCDrawAPI, bool* _pSuccess)
	: hWnd(0)
	, hIcon(0)
	, hClearBrush(0)
	, hOffscreenAADC(0)
	, hOffscreenAABmp(0)
	, hOffscreenBmp(0)
	, hOffscreenDC(0)
	, ClearColor(std::Black)
	, GDIAntialiasingMultiplier(_Desc.UseAntialiasing ? 4 : 1)
	, UseAntialiasing(_Desc.UseAntialiasing)
	, LockToVsync(_Desc.LockToVsync)
	, Closing(false)
	, DrawMethod(DRAW_USING_GDI)
{
	*_pSuccess = false;
	detail::GetFullscreenDimensions(_Desc, &FullscreenWidth, &FullscreenHeight);

	#if oDXVER >= oDXVER_10
	if(_FourCCDrawAPI == 'DX11' && _pAssociatedNativeHandle)
	{
		IUnknown* pInterface = (IUnknown*)( _pAssociatedNativeHandle );
		pInterface->QueryInterface(&D3D11Device);
		DrawMethod = DRAW_USING_DX11;
	}
	else if(_FourCCDrawAPI == 'GDI ' || _FourCCDrawAPI == 'GDI')
	{
		DrawMethod = DRAW_USING_GDI;
	}
	else
	{
		oD2D1CreateFactory(&Factory);

		DrawMethod = (Factory ? DRAW_USING_D2D : DRAW_USING_GDI);

		if (DRAW_USING_D2D == DrawMethod && _pAssociatedNativeHandle)
		{
			// If we're drawing with D2D and a native handle is supplied it means the caller is trying to supply a D3D10Device1
			IUnknown* pInterface = (IUnknown*)( _pAssociatedNativeHandle );
			pInterface->QueryInterface(&D3DDevice);
		}
	}
	#endif
	
	*Title = 0;

	MSSleepWhenNoFocus = _Desc.MSSleepWhenNoFocus;
	*_pSuccess = oWindow_Impl::Open(&_Desc, _Title);

	if (*_pSuccess)
		oTRACE("Created window \"%s\" using %s for drawing.", _Title, DrawMethod == DRAW_USING_D2D ? "D2D" : "GDI");
}

oWindow_Impl::~oWindow_Impl()
{
	Close();
	MsgPumpThread = nullptr;
	MsgPumpProc = nullptr;
	DiscardDeviceResources();
}

void oWindow_Impl::GetDesc(oWindow::DESC* _pDesc) const threadsafe
{
	oASSERT(hWnd, "");
	oMutex::ScopedLock lock(DescLock);
	detail::GetDesc(hWnd, _pDesc);
	_pDesc->UseAntialiasing = UseAntialiasing;
	_pDesc->LockToVsync = LockToVsync;
	_pDesc->RefreshRateN = RefreshRateN;
	_pDesc->RefreshRateD = RefreshRateD;
	if(D3DSwapChain)
	{
		BOOL isFullScreen;
		oWindow_Impl *nonConstThis = const_cast<oWindow_Impl*>(this); //GetFullscreenState isn't const correct.
		nonConstThis->D3DSwapChain->GetFullscreenState(&isFullScreen, nullptr);
		if(isFullScreen)
			_pDesc->State = FULL_SCREEN;
		DXGI_SWAP_CHAIN_DESC swapChainDesc;
		nonConstThis->D3DSwapChain->GetDesc(&swapChainDesc);
		_pDesc->RefreshRateN = swapChainDesc.BufferDesc.RefreshRate.Numerator;
		_pDesc->RefreshRateD = swapChainDesc.BufferDesc.RefreshRate.Denominator;
	}
}

void oWindow_Impl::SetDesc(const oWindow::DESC* _pDesc) threadsafe
{
	oASSERT(hWnd, "");
	oMutex::ScopedLock lock(DescLock);

	Fullscreen = (_pDesc->State == FULL_SCREEN);
	detail::SetDesc(hWnd, _pDesc);
	LockToVsync = _pDesc->LockToVsync;
	RefreshRateN = _pDesc->RefreshRateN;
	RefreshRateD = _pDesc->RefreshRateD;
}

unsigned int oWindow_Impl::GetDisplayIndex() const threadsafe
{
	return oGetWindowDisplayIndex(hWnd);
}

bool oWindow_Impl::Open(const oWindow::DESC* _pDesc, const char* _Title)
{
	oMutex::ScopedLock lock(DescLock);
	Close();
	Closing = false;
	
	MsgPumpProc /= new oMsgPumpProc(_pDesc, _Title, this, StaticWndProc);
	oThread::Create("oWindow Message pump Thread", oKB(64), false, MsgPumpProc.c_ptr(), &MsgPumpThread);
	MsgPumpProc->WaitUntilOpened();

	hWnd = MsgPumpProc->GetHwnd();

	strcpy_s(Title, _Title);

	RefreshRateN = _pDesc->RefreshRateN;
	RefreshRateD = _pDesc->RefreshRateD;

	HRESULT hr = CreateDeviceResources();
	if (FAILED(hr))
	{
		Close();
		oWinSetLastError(hr, "Failed to create Device resources");
		return false;
	}

	if (!OpenChildren())
	{
		Close();
		return false;
	}

	oWindow::DESC wDesc;
	detail::GetDefaultDesc(hWnd, _pDesc, &wDesc);
	SetDesc(&wDesc);
	return true;
}

void oWindow_Impl::Close()
{
	oMutex::ScopedLock lock(DescLock);
	if (hWnd)
	{
		if(D3DSwapChain) //can't delete a swap chain while in full screen mode
			D3DSwapChain->SetFullscreenState(FALSE, nullptr);
		Closing = true;
		CloseChildren();
		MsgPumpProc->Destroy();
		MsgPumpProc->WaitUntilClosed();
		DiscardDeviceResources();
		hWnd = 0;
	}
}

bool oWindow_Impl::IsOpen() const threadsafe
{
	return !!hWnd && !Closing;
}

bool oWindow_Impl::Begin()
{
	if (!hWnd) return false;
	BeginChildren();
	if ( MSSleepWhenNoFocus > 0 && !HasFocus())
		Sleep(MSSleepWhenNoFocus);
	if(DrawMethod == DRAW_USING_DX11)
	{
		oMutex::ScopedLock lock(ResizeMutex);
		oASSERT(D3DSwapChain, "Missing swap chain");

		ID3D11DeviceContext *pImmediateContext;
		D3D11Device->GetImmediateContext(&pImmediateContext);

		DXGI_SWAP_CHAIN_DESC scdesc;
		D3DSwapChain->GetDesc(&scdesc);

		oWindow::DESC wDesc;
		GetDesc(&wDesc);
		D3D11_VIEWPORT v;
		v.TopLeftX = 0.0f;
		v.TopLeftY = 0.0f;
		v.Width = static_cast<float>(scdesc.BufferDesc.Width);
		v.Height = static_cast<float>(scdesc.BufferDesc.Height);
		v.MinDepth = 0.0f;
		v.MaxDepth = 1.0f;
		pImmediateContext->RSSetViewports(1, &v);

		pImmediateContext->OMSetRenderTargets( 1, D3D11View.address(), NULL );
	}

	return true;
}

void oWindow_Impl::End(bool _ForceRefresh, bool _blockUntilPainted)
{
	if(DrawMethod == DRAW_USING_DX11)
	{
		oMutex::ScopedLock lock(ResizeMutex);
		oASSERT(D3DSwapChain, "Missing swap chain");
		D3DSwapChain->Present(LockToVsync ? 1 : 0,0);
	}
	else
	{
		if( _ForceRefresh && !_blockUntilPainted )
		{
			InvalidateRect(hWnd, 0, FALSE);
		}
		else if( !_ForceRefresh && _blockUntilPainted) //this combo is rarely useful
		{
			//this will block until onpaint has finished. If there is no update region though, it will be a no-op
			UpdateWindow(hWnd);
		}
		else if( _ForceRefresh && _blockUntilPainted )
		{
			//This is the same as calling InvalidateRect followed by UpdateWindow except it is atomic.
			// i.e. its guaranteed to only generate 1 paint msg, where calling the 2 functions may generate 1 or 2 depending on luck.
			RedrawWindow(hWnd, nullptr, nullptr, RDW_INVALIDATE | RDW_UPDATENOW); 
		}
	}
	EndChildren();
}

const char* oWindow_Impl::GetTitle() const
{
	return Title;
}

void oWindow_Impl::SetTitle(const char* _Title)
{
	strcpy_s(Title, _Title);
	oSetTitle(hWnd, _Title);
}

bool oWindow_Impl::HasFocus() const
{
	return oHasFocus(hWnd);
}

void oWindow_Impl::SetFocus()
{
	oSetFocus(hWnd);
}

void* oWindow_Impl::GetNativeHandle() threadsafe
{
	return hWnd;
}

const void* oWindow_Impl::GetProperty(const char* _Name) const
{
	if (_Name)
	{
		if (!_stricmp("Icon", _Name)) return &hIcon;
	}

	return 0;
}
HRESULT oWindow_Impl::CreateDeviceResources()
{
	#if oDXVER >= oDXVER_10

		if(DrawMethod == DRAW_USING_DX11 && !RenderTarget)
		{
			RECT rWindow;
			GetWindowRect(hWnd, &rWindow);

			oMutex::ScopedLock lock(ResizeMutex);
			oV_RETURN( oDXGICreateSwapchain( D3D11Device, rWindow.right - rWindow.left, rWindow.bottom - rWindow.top, RefreshRateN, RefreshRateD, D3DSwapChainFMT, hWnd, &D3DSwapChain ) );
			oV_RETURN( CreateRendertarget() );
		}
		else if (DrawMethod == DRAW_USING_D2D && !RenderTarget)
		{
			RECT rWindow;
			GetWindowRect(hWnd, &rWindow);
			
			if (!D3DDevice)
			{
				oRef<IDXGIAdapter1> Adapter;
				oRef<IDXGIOutput> Output;

				if (!oDXGIGetAdapterWithMonitor(rWindow, &Adapter, &Output))
				{
					return E_FAIL;
				}

				HRESULT hr = oD3D10::Singleton()->D3D10CreateDevice1(Adapter, D3D10_DRIVER_TYPE_HARDWARE, NULL, D3D10_CREATE_DEVICE_BGRA_SUPPORT, D3D10_FEATURE_LEVEL_10_1, D3D10_1_SDK_VERSION, &D3DDevice);

				if (E_NOINTERFACE == hr)
				{
					oTRACE("oWindow: Failed to create D3D 10.1 device, falling back to D3D 10.0.");
					hr = oD3D10::Singleton()->D3D10CreateDevice1(Adapter, D3D10_DRIVER_TYPE_HARDWARE, NULL, D3D10_CREATE_DEVICE_BGRA_SUPPORT, D3D10_FEATURE_LEVEL_10_0, D3D10_1_SDK_VERSION, &D3DDevice);
				}

				oV_RETURN(hr);
			}
			
			oV_RETURN( oDXGICreateSwapchain( D3DDevice, rWindow.right - rWindow.left, rWindow.bottom - rWindow.top, RefreshRateN, RefreshRateD, D3DSwapChainFMT, hWnd, &D3DSwapChain ) );
			oV_RETURN( CreateRendertarget() );
		}
		else
	#endif
	if (DrawMethod == DRAW_USING_GDI && !hOffscreenDC)
	{
		hClearBrush = oGDICreateBrush(ClearColor);
		HDC hdc = GetDC(hWnd);
		RECT rc;
		GetClientRect(hWnd, &rc);
		hOffscreenAADC = CreateCompatibleDC(hdc);
		hOffscreenDC = CreateCompatibleDC(hOffscreenAADC);

		hOffscreenAABmp = CreateCompatibleBitmap(hdc, GDIAntialiasingMultiplier * (rc.right-rc.left), GDIAntialiasingMultiplier * (rc.bottom-rc.top));
		hOffscreenBmp = CreateCompatibleBitmap(hdc, rc.right-rc.left, rc.bottom-rc.top);

		SelectObject(hOffscreenAADC, hOffscreenAABmp);
		SelectObject(hOffscreenDC, hOffscreenBmp);

		ReleaseDC(hWnd, hdc);
	}

	return S_OK;
}

void oWindow_Impl::DiscardDeviceResources()
{
	#if oDXVER >= oDXVER_10
		RenderTarget = 0;
	#endif

	if (hClearBrush) { DeleteObject(hClearBrush); hClearBrush = 0; }
	if (hOffscreenAABmp) { DeleteObject(hOffscreenAABmp); hOffscreenAABmp = 0; }
	if (hOffscreenBmp) { DeleteObject(hOffscreenBmp); hOffscreenBmp = 0; }
	if (hOffscreenAADC) { DeleteDC(hOffscreenAADC); hOffscreenAADC = 0; }
	if (hOffscreenDC) { DeleteDC(hOffscreenDC); hOffscreenDC = 0; }
}

bool oWindow_Impl::SetProperty(const char* _Name, void* _pValue)
{
	bool result = false;
	if (hWnd && _Name)
	{
		if (!_stricmp("Icon", _Name))
		{
			if (hIcon) DestroyIcon(hIcon);
			hIcon = _pValue ? *(HICON*)_pValue : 0;
			oSetIcon(hWnd, true, hIcon);
			result = true;
		}
	}

	return result;
}

bool oWindow_Impl::CreateResizer(RectHandlerFn _RectHandler, threadsafe Resizer** _ppResizer) threadsafe
{
	if (!_RectHandler || !_ppResizer) return false;
	*_ppResizer = new detail::oWindowResizer_Impl(this, _RectHandler);
		return !!*_ppResizer;
}

#if oDXVER >= oDXVER_10
	#define CREATE_DRAWABLE(classBaseName) \
		if (!_pDesc || !_ppDrawable) return false; \
		if (DrawMethod == DRAW_USING_D2D) *_ppDrawable = new detail::classBaseName##D2D(this, _pDesc); \
		else *_ppDrawable = new detail::classBaseName##GDI(this, _pDesc); \
		return !!*_ppDrawable;
#else
	#define CREATE_DRAWABLE(classBaseName) \
		if (!_pDesc || !_ppDrawable) return false; \
		*_ppDrawable = new detail::classBaseName##GDI(this, _pDesc); \
		return !!*_ppDrawable;
#endif

bool oWindow_Impl::CreateRoundedBox(const RoundedBox::DESC* _pDesc, threadsafe RoundedBox** _ppDrawable) threadsafe
{
	CREATE_DRAWABLE(RoundedBox);
}

bool oWindow_Impl::CreateLine(const Line::DESC* _pDesc, threadsafe Line** _ppDrawable) threadsafe
{
	CREATE_DRAWABLE(Line);
}

bool oWindow_Impl::CreateFont(const Font::DESC* _pDesc, threadsafe Font** _ppDrawable) threadsafe
{
	CREATE_DRAWABLE(Font);
}

bool oWindow_Impl::CreateText(const Text::DESC* _pDesc, threadsafe Font* _pFont, threadsafe Text** _ppDrawable) threadsafe
{
	if (!_pDesc || !_pFont || !_ppDrawable) return false;
	#if oDXVER >= oDXVER_10
		if (DrawMethod == DRAW_USING_D2D)
			*_ppDrawable = new detail::TextD2D(this, _pFont, _pDesc);
		else
	#endif
			*_ppDrawable = new detail::TextGDI(this, _pFont, _pDesc);
	return !!*_ppDrawable;
}

bool oWindow_Impl::CreatePicture(const Picture::DESC* _pDesc, threadsafe Picture** _ppDrawable) threadsafe
{
	if (_pDesc && _pDesc->SurfaceDesc.Format == oSurface::UNKNOWN)
	{
		oTRACE("ERROR: CreatePicture() Surface format is UNKNOWN.");
		return false;
	}

	CREATE_DRAWABLE(Picture);
}

bool oWindow_Impl::CreateVideo(const Video::DESC*_pDesc, oVideoContainer** _ppVideos, size_t _NumVideos, threadsafe Video** _ppVideo) threadsafe
{
	if (!_pDesc || !_ppVideo) return false;

	if (DrawMethod != DRAW_USING_D2D)
	{
		oSetLastError(EINVAL, "GDI currently doesn't support video playback.");
		return false;
	}

	bool success = false;
#if oDXVER >= oDXVER_10
	oRef<ID3D10Texture2D> D3DRenderTarget;
	oV( D3DSwapChain->GetBuffer(0, __uuidof(ID3D10Texture2D), (void**)&D3DRenderTarget ) );
	oCONSTRUCT(_ppVideo, detail::VideoD2D(this, _pDesc, _ppVideos, _NumVideos, D3DRenderTarget.c_ptr(), &success ) );
#endif
	return success;
}

bool oWindow_Impl::CreateSnapshot(oImage** _ppImage, bool _IncludeBorder)
{
	// Create a bmp in memory
	RECT r;
	GetClientRect(hWnd, &r);
	BITMAPINFO bmi;

	RECT* pRect = &r;
	if (_IncludeBorder)
		pRect = 0;

	oGDIScreenCaptureWindow(hWnd, pRect, 0, 0, &bmi);

	BITMAPFILEHEADER bmfh;
	memset(&bmfh, 0, sizeof(bmfh));
	bmfh.bfType = 'MB';
	bmfh.bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFO) + bmi.bmiHeader.biSizeImage;
	bmfh.bfOffBits = static_cast<DWORD>(sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFO));

	size_t bufSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFO) + bmi.bmiHeader.biSizeImage;
	char* buf = new char[bufSize];
	memcpy(buf, &bmfh, sizeof(bmfh));
	memcpy(oByteAdd(buf, sizeof(bmfh)), &bmi, sizeof(bmi));
	oGDIScreenCaptureWindow(hWnd, pRect, oByteAdd(buf, sizeof(bmfh) + sizeof(bmi)), bmi.bmiHeader.biSizeImage, &bmi);

	bool result = oImage::Create(buf, bufSize, oSurface::UNKNOWN, _ppImage);
	delete [] buf;

	return result;
}

bool oWindow_Impl::AddChild(Child* _pChild) threadsafe
{
	if (!IsOpen() || _pChild->Open())
	{
		Children.push_back(_pChild);
		return true;
	}
	
	return false;
}

bool oWindow_Impl::MessageChild(oWindow::Child*& _pChild, void* _pUserData)
{
	return _pChild->HandleMessage(_pUserData);
}

bool oWindow_Impl::HandleChildrenMessages(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) threadsafe
{
	CWPSTRUCT cw;
	cw.hwnd = hwnd;
	cw.message = uMsg;
	cw.wParam = wParam;
	cw.lParam = lParam;
	return Children.foreach(oBIND( &oWindow_Impl::MessageChild, oBIND1, (void*)&cw ));
}

BOOL oWindow_Impl::OnResize(UINT _NewWidth, UINT _NewHeight)
{
	#if oDXVER >= oDXVER_10

	if(D3DSwapChain)
	{
		if( Fullscreen )
		{
			_NewWidth = FullscreenWidth;
			_NewHeight = FullscreenHeight;
		}
		D3DSwapChain->SetFullscreenState(Fullscreen, nullptr);
	}

	if (DrawMethod == DRAW_USING_DX11 && RenderTarget)
	{
		oMutex::ScopedLock lock(ResizeMutex);
		D3DSwapChain->ResizeBuffers( 3, _NewWidth, _NewHeight, D3DSwapChainFMT, 0 );
		CreateRendertarget();
	}
	else if (DrawMethod == DRAW_USING_D2D && RenderTarget)
	{
		oMutex::ScopedLock lock(ResizeMutex);
		oLockedVector<Video*>::LockedSTLVector vec = Videos.lock();
		// First tell any videos to unregister so they no longer hold any references to the swapchain
		oFOREACH( oWindow::Video* pVid, *vec )
		{
			static_cast<detail::VideoD2D*>(pVid)->UnregisterDecoder();
		}

		// Now we can safely resize
		RenderTarget = NULL;
		D3DSwapChain->ResizeBuffers( 3, _NewWidth, _NewHeight, D3DSwapChainFMT, 0 );
		CreateRendertarget();

		// Get the new backbuffer for the swap chain
		oRef<ID3D10Texture2D> D3DRenderTarget;
		oV( D3DSwapChain->GetBuffer(0, __uuidof(ID3D10Texture2D), (void**)&D3DRenderTarget ) );

		// Re-register the videos so they can use the new swapchain
		oFOREACH( oWindow::Video* pVid, *vec )
		{
			static_cast<detail::VideoD2D*>(pVid)->RegisterDecoder(D3DRenderTarget.c_ptr());
		}
		return TRUE;
	}
	else
	#endif
	if (DrawMethod == DRAW_USING_GDI && hOffscreenDC)
	{
		DiscardDeviceResources();
		CreateDeviceResources();
	}

	return FALSE;
}

BOOL oWindow_Impl::OnPaint(HWND _hWnd)
{
	RECT rect;
	if(GetUpdateRect(_hWnd, &rect, false) == 0) //shouldn't happen but windows docs say it can in rare cases, don't bother painting if nothing to paint.
		return TRUE;

	PAINTSTRUCT ps;
	oVB(BeginPaint(_hWnd, &ps));
	HRESULT hr = CreateDeviceResources();
	if (SUCCEEDED(hr))
	{
		#if oDXVER >= oDXVER_10
			if (DrawMethod == DRAW_USING_D2D)
			{
				RenderTarget->BeginDraw();
				RenderTarget->SetTransform(D2D1::IdentityMatrix());

				float r,g,b,a;
				oDecomposeColor(ClearColor, &r, &g, &b, &a);
				RenderTarget->Clear(D2D1::ColorF(r,g,b,a));

				{ // Since videos are drawn with D3D we need to insert our draw right before D2D calls but after the clear
					RenderTarget->EndDraw();
					Videos.foreach(oBIND( &oWindow_Impl::PaintChild<oWindow::Video>, oBIND1, (void*)0 ));
					RenderTarget->BeginDraw();
				}
	
				Pictures.foreach(oBIND( &oWindow_Impl::PaintChild<oWindow::Picture>, oBIND1, (void*)0 ));
				Lines.foreach(oBIND( &oWindow_Impl::PaintChild<oWindow::Line>, oBIND1, (void*)0 ));
				RoundedBoxes.foreach(oBIND( &oWindow_Impl::PaintChild<oWindow::RoundedBox>, oBIND1, (void*)0 ));
				Texts.foreach(oBIND( &oWindow_Impl::PaintChild<oWindow::Text>, oBIND1, (void*)0 ));

				hr = RenderTarget->EndDraw();

				D3DSwapChain->Present(LockToVsync ? 1 : 0,0);
			}
			else
		#endif
		if (DrawMethod == DRAW_USING_GDI)
		{
			RECT r;
			GetClientRect(hWnd, &r);

			FillRect(hOffscreenAADC, &r, hClearBrush);

			detail::WINPAINT_DESC wp;
			wp.hDC = hOffscreenAADC;
			wp.Scale = GDIAntialiasingMultiplier;

			Pictures.foreach(oBIND( &oWindow_Impl::PaintChild<oWindow::Picture>, oBIND1,  &wp ) );
			Lines.foreach(oBIND( &oWindow_Impl::PaintChild<oWindow::Line>, oBIND1,  &wp ) );
			RoundedBoxes.foreach(oBIND( &oWindow_Impl::PaintChild<oWindow::RoundedBox>, oBIND1,  &wp ) );

			int oldMode = GetStretchBltMode(hOffscreenDC);
			SetStretchBltMode(hOffscreenDC, HALFTONE);

			int width = GDIAntialiasingMultiplier * (r.right-r.left);
			int height = GDIAntialiasingMultiplier * (r.bottom-r.top);
			StretchBlt(hOffscreenDC, r.left, r.top, r.right-r.left, r.bottom-r.top, hOffscreenAADC, 0, 0, width, height, SRCCOPY);
			SetStretchBltMode(hOffscreenDC, oldMode);

			// But let windows do cleartyping on text
			wp.hDC = hOffscreenDC;
			Texts.foreach(oBIND( &oWindow_Impl::PaintChild<oWindow::Text>, oBIND1,  &wp ));

			// Do final present
			BitBlt(ps.hdc, r.left, r.top, r.right-r.left, r.bottom-r.top, hOffscreenDC, 0, 0, SRCCOPY);
		}
	}

	if (FAILED(hr))
		DiscardDeviceResources();

	EndPaint(_hWnd, &ps);
	return TRUE;
}

void oWindow_Impl::OnClose()
{
	if (!Closing)
	{
		// Hide window so teardown isn't visible
		oWindow::DESC desc;
		detail::GetDesc(hWnd, &desc);
		desc.State = oWindow::HIDDEN;
		SetDesc(&desc);
		if (IsOpen())
			Closing = true;
	}
}

LRESULT oWindow_Impl::WndProc(HWND _hWnd, UINT _uMsg, WPARAM _wParam, LPARAM _lParam)
{
	if (!HandleChildrenMessages(_hWnd, _uMsg, _wParam, _lParam))
		return TRUE;

	switch (_uMsg)
	{
		case WM_SIZE:
			if (OnResize(LOWORD(_lParam), HIWORD(_lParam)))
				return 0;
			break;

		// handle WM_ERASEBKGND for flicker-free client area update
		// http://msdn.microsoft.com/en-us/library/ms969905.aspx
		case WM_ERASEBKGND:
			return 1;

		case WM_PRINT:
		case WM_PRINTCLIENT:
		case WM_PAINT:
		case WM_DISPLAYCHANGE:
			if (OnPaint(_hWnd))
				return 0;
			break;

		case WM_CLOSE:
			if (!EnableCloseButton)
				return 0;

		case WM_DESTROY:
			OnClose();
			return 0;

		case WM_SYSKEYDOWN:
		case WM_SYSKEYUP:
			//if (_wParam == VK_RETURN)
			//	oTRACE("TOGGLE FULLSCREEN");
			break;

		case oWM_TRAY:
		{
			UINT Notification = 0, ID = 0;
			int X = 0, Y = 0;
			oTrayDecodeCallbackMessageParams(_wParam, _lParam, &Notification, &ID, &X, &Y);

			// @oooii-tony: This is very basic, because really a tray icon
			// either needs a context menu (no api for that yet) or it needs
			// to put up tooltips (no api yet). So basically TRAYIZED is all 
			// just a min-to-tray feature at the moment, and here's what
			// gets out of it.

			if (Notification == WM_LBUTTONUP || Notification == WM_RBUTTONUP)
			{
				oWindow::DESC d;
				GetDesc(&d);
				d.State = oWindow::RESTORED;
				SetDesc(&d);
			}

			// @oooii-tony: TODO: Add the unidentified messages to oGetWMDesc
			if (Notification != WM_MOUSEMOVE)
			{
				#ifdef _DEBUG
					char str[1024];
					oTRACE("oWM_TRAY: %s", oGetWMDesc(str, _hWnd, Notification, 0, 0));
				#endif
			}

			break;
		}

		default:
			break;
	}

	return DefWindowProc(_hWnd, _uMsg, _wParam, _lParam);
}

#if oDXVER >= oDXVER_10
HRESULT oWindow_Impl::CreateRendertarget()
{
	if(DrawMethod == DRAW_USING_DX11)
	{
		oRef<ID3D11Texture2D> D3DRenderTarget;
		oV_RETURN( D3DSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&D3DRenderTarget ) );
		D3D11Device->CreateRenderTargetView( D3DRenderTarget, NULL,	&D3D11View );
	}
	else
	{
		oRef<ID3D10Texture2D> D3DRenderTarget;
		oV_RETURN( D3DSwapChain->GetBuffer(0, __uuidof(ID3D10Texture2D), (void**)&D3DRenderTarget ) );

		oRef<IDXGISurface> RTSurface;
		oV_RETURN(D3DRenderTarget->QueryInterface(&RTSurface));

		D2D1_RENDER_TARGET_PROPERTIES rtProps;
		rtProps.pixelFormat.format = D3DSwapChainFMT;
		rtProps.pixelFormat.alphaMode = D2D1_ALPHA_MODE_IGNORE;
		rtProps.dpiX = 0.0f;
		rtProps.dpiY = 0.0f;
		rtProps.minLevel = D2D1_FEATURE_LEVEL_DEFAULT;
		rtProps.type = D2D1_RENDER_TARGET_TYPE_DEFAULT;
		rtProps.usage = D2D1_RENDER_TARGET_USAGE_NONE;

		oV_RETURN(Factory->CreateDxgiSurfaceRenderTarget(RTSurface, &rtProps, &RenderTarget));

		RenderTarget->SetAntialiasMode(UseAntialiasing ? D2D1_ANTIALIAS_MODE_PER_PRIMITIVE : D2D1_ANTIALIAS_MODE_ALIASED);
	}
	return S_OK;
}
#endif

bool oWindow_Impl::QueryInterface( const oGUID& _InterfaceID, threadsafe void** _ppInterface ) threadsafe
{
	if (oGetGUID<oWindow>() == _InterfaceID)
	{
		Reference();
		*_ppInterface = this;
		return true;
	}

	return false;
}

bool oWindow::Pump(oWindow* _pWindow, bool _CloseOnTimeout, unsigned int _Timeout)
{
	unsigned int time = oTimerMS();
	while (_pWindow->IsOpen())
	{
		if (_pWindow->Begin())
			_pWindow->End();

		if (_Timeout != oINFINITE_WAIT && (oTimerMS() - time) > _Timeout)
		{
			if (_CloseOnTimeout)
				_pWindow->Close();

			return false;
		}
	}

	return true;
}

template<> errno_t oFromString(oWindow::STYLE *_style, const char* _StrSource)
{
	if(_stricmp(_StrSource,"borderless") == 0)
		*_style = oWindow::BORDERLESS;
	else if(_stricmp(_StrSource,"fixed") == 0)
		*_style = oWindow::FIXED;
	else
		*_style = oWindow::SIZEABLE;
	return 0;
}

const oGUID& oGetGUID( threadsafe const oWindow* threadsafe const * )
{
	// {963AD3B4-0F53-4e6f-8D6D-C0FBB3CB58A1}
	static const oGUID oIIDMWindowResizer = { 0x963ad3b4, 0xf53, 0x4e6f, { 0x8d, 0x6d, 0xc0, 0xfb, 0xb3, 0xcb, 0x58, 0xa1 } };
	return oIIDMWindowResizer;
}

const oGUID& oGetGUID( threadsafe const detail::RoundedBoxGDI* threadsafe const * )
{
	// {571C1B76-ABB6-4f08-BD32-078919BA2C76}
	static const oGUID oIIDRoundedBoxGDT = { 0x571c1b76, 0xabb6, 0x4f08, { 0xbd, 0x32, 0x7, 0x89, 0x19, 0xba, 0x2c, 0x76 } };
	return oIIDRoundedBoxGDT;
}

const oGUID& oGetGUID( threadsafe const detail::LineGDI* threadsafe const * )
{
	// {01855CE1-563C-4213-976C-974C8381A252}
	static const oGUID oIIDLineGDI = { 0x1855ce1, 0x563c, 0x4213, { 0x97, 0x6c, 0x97, 0x4c, 0x83, 0x81, 0xa2, 0x52 } };
	return oIIDLineGDI;
}

const oGUID& oGetGUID( threadsafe const detail::TextGDI* threadsafe const * )
{
	// {DA612088-F9B9-48a3-80D5-4974697F06D1}
	static const oGUID oIIDTextGDI = { 0xda612088, 0xf9b9, 0x48a3, { 0x80, 0xd5, 0x49, 0x74, 0x69, 0x7f, 0x6, 0xd1 } };
	return oIIDTextGDI;
}

const oGUID& oGetGUID( threadsafe const detail::PictureGDI* threadsafe const * )
{
	// {B151EB44-AB77-4049-89B1-DD7E0B8EBB4D}
	static const oGUID oIIDPictureGDI = { 0xb151eb44, 0xab77, 0x4049, { 0x89, 0xb1, 0xdd, 0x7e, 0xb, 0x8e, 0xbb, 0x4d } };
	return oIIDPictureGDI;
}

const oGUID& oGetGUID( threadsafe const detail::FontGDI* threadsafe const * )
{
	// {8B0A96BB-31DD-464b-93EF-DCBE3AC9260E}
	static const oGUID oIIDFontGDI = { 0x8b0a96bb, 0x31dd, 0x464b, { 0x93, 0xef, 0xdc, 0xbe, 0x3a, 0xc9, 0x26, 0xe } };
	return oIIDFontGDI;
}

const oGUID& oGetGUID( threadsafe const detail::RoundedBoxD2D* threadsafe const * )
{
	// {3D461425-9AF9-4c32-BE4C-F3DE41A4190F}
	static const oGUID oIIDRoundedBoxD2D = { 0x3d461425, 0x9af9, 0x4c32, { 0xbe, 0x4c, 0xf3, 0xde, 0x41, 0xa4, 0x19, 0xf } };
	return oIIDRoundedBoxD2D;
}
const oGUID& oGetGUID( threadsafe const detail::LineD2D* threadsafe const * )
{
	// {2BFDB884-A3FA-4dc8-82DF-EF172C11D785}
	static const oGUID oIIDLineD2D = { 0x2bfdb884, 0xa3fa, 0x4dc8, { 0x82, 0xdf, 0xef, 0x17, 0x2c, 0x11, 0xd7, 0x85 } };
	return oIIDLineD2D;
}

const oGUID& oGetGUID( threadsafe const detail::FontD2D* threadsafe const * )
{
	// {6E55C103-4F0F-4da1-8B48-7FBC7885ADE5}
	static const oGUID oIIDFontD2D = { 0x6e55c103, 0x4f0f, 0x4da1, { 0x8b, 0x48, 0x7f, 0xbc, 0x78, 0x85, 0xad, 0xe5 } };
	return oIIDFontD2D;
}

const oGUID& oGetGUID( threadsafe const detail::TextD2D* threadsafe const * )
{
	// {0D9885AB-2FBE-4104-ACD4-AC0BDDB942A7}
	static const oGUID oIIDTextD2D = { 0xd9885ab, 0x2fbe, 0x4104, { 0xac, 0xd4, 0xac, 0xb, 0xdd, 0xb9, 0x42, 0xa7 } };
	return oIIDTextD2D;
}

const oGUID& oGetGUID( threadsafe const detail::PictureD2D* threadsafe const * )
{
	// {FA8EF1D6-8A42-420b-8DE9-AC3F083E3644}
	static const oGUID oIIDPictureD2D = { 0xfa8ef1d6, 0x8a42, 0x420b, { 0x8d, 0xe9, 0xac, 0x3f, 0x8, 0x3e, 0x36, 0x44 } };
	return oIIDPictureD2D;
}
