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
#include "pch.h"
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

template<> const char* oAsString(const oWindow::STATE& _State)
{
	switch (_State)
	{
		case oWindow::HIDDEN: return "hidden";
		case oWindow::RESTORED: return "restored";
		case oWindow::MINIMIZED: return "minimized";
		case oWindow::MAXIMIZED: return "maximized";
		default: break;
	}

	return "unknown";
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

const oGUID& oGetGUID( threadsafe const oWindow_Impl* threadsafe const * )
{
	// {85D2B690-422F-43af-BAB1-101118C1F546}
	static const oGUID oIIDWindow = { 0x85d2b690, 0x422f, 0x43af, { 0xba, 0xb1, 0x10, 0x11, 0x18, 0xc1, 0xf5, 0x46 } };
	return oIIDWindow;
}

struct oWindow_Impl : public oWindow
{
	enum DRAW_METHOD
	{
		DRAW_USING_GDI,
		DRAW_USING_D2D,
	};

	oDEFINE_REFCOUNT_INTERFACE(RefCount);
	oDEFINE_TRIVIAL_QUERYINTERFACE(oGetGUID<oWindow_Impl>());

	void GetDesc(DESC* _pDesc) const threadsafe override;
	void SetDesc(const DESC* _pDesc) threadsafe override;
	unsigned int GetDisplayIndex() const threadsafe override;
	bool Open(const DESC* _pDesc, const char* _Title = "") override;
	void Close() override;
	bool IsOpen() const threadsafe override;
	bool Begin() override;
	void End() override;
	const char* GetTitle() const override;
	void SetTitle(const char* _Title) override;
	bool HasFocus() const override;
	void SetFocus() override;
	void* GetNativeHandle() override;
	const void* GetProperty(const char* _Name) const override;
	bool SetProperty(const char* _Name, void* _Value) override;
	bool CreateResizer(ResizeHandlerFn _ResizeHandler, void* _pUserData, threadsafe Resizer** _ppResizer) threadsafe override;
	bool CreateRoundedBox(const RoundedBox::DESC* _pDesc, threadsafe RoundedBox** _ppRoundedBox) threadsafe override;
	bool CreateLine(const Line::DESC* _pDesc, threadsafe Line** _ppLine) threadsafe override;
	bool CreateFont(const Font::DESC* _pDesc, threadsafe Font** _ppFont) threadsafe override;
	bool CreateText(const Text::DESC* _pDesc, threadsafe Font* _pFont, threadsafe Text** _ppText) threadsafe override;
	bool CreatePicture(const Picture::DESC* _pDesc, threadsafe Picture** _ppPicture) threadsafe override;
	bool CreateSnapshot(oImage** _ppImage, bool _IncludeBorder = false) override;
	oWindow_Impl(const DESC* _pDesc, const char* _Title, unsigned int _FourCCDrawAPI);
	~oWindow_Impl();

	oLockedVector<Child*> Children;
	oLockedVector<Picture*>	Pictures;
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
	bool Closing;
	bool EnableCloseButton;
	mutable oMutex DescLock;
	oRefCount RefCount;
	DRAW_METHOD DrawMethod;
	char Title[_MAX_PATH];

#if oDXVER >= oDXVER_10
	oRef<ID2D1HwndRenderTarget> RenderTarget;
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
	threadsafe oRef<oWindow> w;
	GetWindow(&w);
	static_cast<threadsafe oWindow_Impl*>(w.c_ptr())->AddChild(this);
}

void oWindow::Child::Unregister()
{
	threadsafe oRef<oWindow> w;
	GetWindow(&w);
	static_cast<threadsafe oWindow_Impl*>(w.c_ptr())->RemoveChild(this);
}

namespace detail {

struct WINPAINT_DESC
{
	HDC hDC;
	unsigned int Scale;
};

static int GetShowCmd(oWindow::STATE _State)
{
	switch (_State)
	{
		case oWindow::HIDDEN: return SW_HIDE;
		case oWindow::MINIMIZED: return SW_SHOWMINIMIZED;
		case oWindow::MAXIMIZED: return SW_SHOWMAXIMIZED;
		default: break;
	}

	return SW_SHOWNORMAL;
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

static void GetDesc(HWND _hWnd, oWindow::DESC* _pDesc)
{
	RECT rClient;
	oGetClientScreenRect(_hWnd, &rClient);

	_pDesc->ClientX = rClient.left;
	_pDesc->ClientY = rClient.top;
	_pDesc->ClientWidth = rClient.right - rClient.left;
	_pDesc->ClientHeight = rClient.bottom - rClient.top;

	if (!IsWindowVisible(_hWnd)) _pDesc->State = oWindow::HIDDEN;
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

	//@oooii-Andrew
	oWindow_Impl* pThis = (oWindow_Impl*)oGetWindowContext(_hWnd, 0, 0, 0);
	if (pThis)
	{
		_pDesc->EnableCloseButton = pThis->EnableCloseButton;
	}
}

static bool SetDesc(HWND _hWnd, const oWindow::DESC* _pDesc)
{
	if (!_hWnd || !_pDesc) return false;
	oPumpMessages(_hWnd);

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
	UINT SWPFlags = SWP_NOMOVE|SWP_NOSIZE|SWP_NOZORDER;
	RECT rCurrent;
	GetWindowRect(_hWnd, &rCurrent);
	bool hasMove = false;
	bool hasResize = false;

	// maximized takes care of this in ShowWindow below, so don't step on that again in SetWindowPos
	if (state != oWindow::MAXIMIZED)
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

	if (_pDesc->HasFocus || _pDesc->AlwaysOnTop)
		SWPFlags &=~ SWP_NOZORDER;

	//@oooii-Andrew
	oWindow_Impl* pThis = (oWindow_Impl*)oGetWindowContext(_hWnd, 0, 0, 0);
	if (pThis)
	{
		pThis->EnableCloseButton = _pDesc->EnableCloseButton;
	}

	// Here's the next piece, test on cached state, not _pDesc->State
	// because we may've overridden it with a force hidden.
	if (currentDesc.State != state || styleChanged)
		ShowWindow(_hWnd, GetShowCmd(state));

	if (hasMove || hasResize || _pDesc->HasFocus)
		SetWindowPos(_hWnd, _pDesc->AlwaysOnTop ? HWND_TOPMOST : HWND_NOTOPMOST, rDesired.left, rDesired.top, rDesired.right-rDesired.left, rDesired.bottom-rDesired.top, SWPFlags);

	if (currentDesc.Enabled != _pDesc->Enabled)
		EnableWindow(_hWnd, _pDesc->Enabled);

	if (_pDesc->HasFocus)
		oSetFocus(_hWnd);

	// And here's the final piece of the hack from above where we force
	// hide and then show on style change.
	if (restate != state || styleChanged)
		ShowWindow(_hWnd, GetShowCmd(restate));

	return true;
}



struct oWindowResizer_Impl : public oWindow::Resizer
{
	oDEFINE_REFCOUNT_INTERFACE(RefCount);
	oDEFINE_TRIVIAL_QUERYINTERFACE(oGetGUID<oWindow>());
	//oDEFINE_TRIVIAL_QUERYINTERFACE(oGetGUID((volatile const oWindow_Impl *volatile const *) (NULL) ) );
	oDEFINE_GETINTERFACE_INTERFACE(threadsafe, GetWindow, threadsafe, oWindow, Window);

	oWindowResizer_Impl(threadsafe oWindow* _pParentWindow, oWindow::ResizeHandlerFn _ResizeHandlerFn, void* _pUserData)
		: Window(_pParentWindow)
		, ResizeHandler(_ResizeHandlerFn)
		, pUserData(_pUserData)
	{
		Register();
	}

	~oWindowResizer_Impl()
	{
		Unregister();
	}

	bool Open() override
	{
		RECT r;
		GetClientRect((HWND)Window->GetNativeHandle(), &r);
		oWindow::DESC desc;
		Window->GetDesc(&desc);
		ResizeHandler(oWindow::RESIZE_CHANGE, desc.State, r.right - r.left, r.bottom - r.top, pUserData);
		return true;
	}

	void HandlePaint(void* _pPlatformData) override {}
	bool HandleMessage(void* _pPlatformData) override
	{
		CWPSTRUCT* cw = static_cast<CWPSTRUCT*>(_pPlatformData);

		oWindow::DESC desc;
		Window->GetDesc(&desc);

		RECT r;
		GetClientRect((HWND)Window->GetNativeHandle(), &r);

		oWindow::RESIZE_EVENT e = oWindow::RESIZE_CHANGE;
		bool isSizeMsg = true;
		switch (cw->message)
		{
			case WM_ENTERSIZEMOVE: e = oWindow::RESIZE_BEGIN; break;
			case WM_SIZE: e = oWindow::RESIZE_CHANGE; break;
			case WM_EXITSIZEMOVE: e = oWindow::RESIZE_END; break;
			default: isSizeMsg = false;
		}

		if (isSizeMsg)
			ResizeHandler(e, desc.State, r.right - r.left, r.bottom - r.top, pUserData);

		return true;
	}

	void Close() override {}
	bool Begin() override { return true; }
	void End() override {}

	oRef<oWindow> Window;
	oWindow::ResizeHandlerFn ResizeHandler;
	void* pUserData;
	oRefCount RefCount;
};



struct RoundedBoxGDI : public oWindow::RoundedBox
{
	oDEFINE_REFCOUNT_INTERFACE(RefCount);
	oDEFINE_TRIVIAL_QUERYINTERFACE(oGetGUID<RoundedBoxGDI>());
	oDEFINE_GETINTERFACE_INTERFACE(threadsafe, GetWindow, threadsafe, oWindow, Window);

	inline HWND Hwnd() threadsafe { return (HWND)thread_cast<oWindow*>(Window.c_ptr())->GetNativeHandle(); }
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
			RECT rClient, rOld, rNew;
			GetClientRect(Hwnd(), &rClient);

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
				InvalidateRect(Hwnd(), 0/*&rNew*/, TRUE); // @oooii-tony: This only seems to update if the whole client is invalidated. Why?
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
		GetClientRect(Hwnd(), &rClient);

		char opts[5] = { 'f',0,0,0,0 };
		ConvertAlignment(opts+1, Desc.Anchor);

		RECT rDesc = oBuildRECTWH(Desc.X, Desc.Y, Desc.Width, Desc.Height);
		oAdjustRect(&rClient, &rDesc, &rAdjusted, Scale(), opts);

		int radius = static_cast<int>(p->Scale * Desc.Roundness * 2.5f);
		RoundRect(p->hDC, rAdjusted.left, rAdjusted.top, rAdjusted.right, rAdjusted.bottom, radius, radius);
	}

	threadsafe oRef<oWindow> Window;
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

	inline HWND Hwnd() threadsafe { return (HWND)thread_cast<oWindow*>(Window.c_ptr())->GetNativeHandle(); }
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

	threadsafe oRef<oWindow> Window;
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

	inline HWND Hwnd() threadsafe { return (HWND)thread_cast<oWindow*>(Window.c_ptr())->GetNativeHandle(); }

	FontGDI(threadsafe oWindow* _pWindow, const DESC* _pDesc)
		: Window(_pWindow)
		, Desc(*_pDesc)
		, hFont(0)
	{
		HDC hdc = GetDC(Hwnd());
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
		ReleaseDC(Hwnd(), hdc);
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

	threadsafe oRef<oWindow> Window;
	HFONT hFont;
	DESC Desc;
	oRefCount RefCount;
};



struct TextGDI : public oWindow::Text
{
	oDEFINE_REFCOUNT_INTERFACE(RefCount);
	oDEFINE_TRIVIAL_QUERYINTERFACE(oGetGUID<TextGDI>());
	oDEFINE_GETINTERFACE_INTERFACE(threadsafe, GetWindow, threadsafe, oWindow, Window);

	inline HWND Hwnd() threadsafe { return (HWND)thread_cast<oWindow*>(Window.c_ptr())->GetNativeHandle(); }

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
		InvalidateRect(Hwnd(), 0, TRUE);
	}

	void SetFont(threadsafe oWindow::Font* _pFont) threadsafe
	{
		oMutex::ScopedLock lock(Mutex);
		Font = _pFont;
		InvalidateRect(Hwnd(), 0, TRUE);
	}

	void HandlePaint(void* _pPlatformData) override
	{
		WINPAINT_DESC* p = static_cast<WINPAINT_DESC*>(_pPlatformData);
		oMutex::ScopedLock lock(Mutex);

		RECT rClient, rDesc = oBuildRECTWH(Desc.X, Desc.Y, Desc.Width, Desc.Height), rAdjusted;
		GetClientRect(Hwnd(), &rClient);
		char opts[5] = { 'f',0,0,0,0 };
		ConvertAlignment(opts+1, Desc.Anchor);
		oAdjustRect(&rClient, &rDesc, &rAdjusted, 1, opts);
		
		oGDIScopedSelect font(p->hDC, static_cast<FontGDI*>(Font.c_ptr())->hFont);
		
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

	threadsafe oRef<oWindow> Window;
	oRef<oWindow::Font> Font;
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

	inline HWND Hwnd() threadsafe { return (HWND)thread_cast<oWindow*>(Window.c_ptr())->GetNativeHandle(); }
	inline int Scale() threadsafe { return static_cast<threadsafe oWindow_Impl*>(Window.c_ptr())->GDIAntialiasingMultiplier; }

	PictureGDI(threadsafe oWindow* _pWindow, const Picture::DESC* _pDesc)
		: Window(_pWindow)
		, Desc(*_pDesc)
		, hBitmap(0)
		, hFlipBitmap(0)
	{
		HDC hdc = GetDC(Hwnd());
		hBitmap = CreateCompatibleBitmap(hdc, _pDesc->SurfaceDesc.Width, _pDesc->SurfaceDesc.Height);

		// @oooii-tony: just in case the user wants to flip horizontally and 
		// update frequently, such as from a video feed. Not the greatest, but 
		// a little memory is worth it to save the perf hit in a frequent-update
		// loop.
		hFlipBitmap = CreateCompatibleBitmap(hdc, _pDesc->SurfaceDesc.Width, _pDesc->SurfaceDesc.Height);

		ReleaseDC(Hwnd(), hdc);
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
			oMutex::ScopedLock lock(BitmapLock);
			// Always allocate enough memory for 8-bit formats and avoid a heap 
			// alloc below
			BITMAPINFO* pBMI = (BITMAPINFO*)_alloca(oSurface::GetBMISize(Desc.SurfaceDesc.Format));
			oSurface::GetBMI((void**)&pBMI, thread_cast<oSurface::DESC*>(&Desc.SurfaceDesc), 0, !_FlipVertically); // @oooii-tony: Windows is already flipping it, so if you want to flip it again, undo that
			HDC hdc = GetDC(Hwnd());
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
			ReleaseDC(Hwnd(), hdc);

			InvalidateRect(Hwnd(), 0, FALSE);
		}
	}

	void HandlePaint(void* _pPlatformData) override
	{
		WINPAINT_DESC* p = static_cast<WINPAINT_DESC*>(_pPlatformData);

		RECT rClient, rDesc = oBuildRECTWH(Desc.X, Desc.Y, Desc.Width, Desc.Height), rAdjusted;
		GetClientRect(Hwnd(), &rClient);
		oAdjustRect(&rClient, &rDesc, &rAdjusted, p->Scale, 0);

		oGDIStretchBitmap(p->hDC, rAdjusted.left, rAdjusted.top, rAdjusted.right-rAdjusted.left, rAdjusted.bottom-rAdjusted.top, hBitmap, SRCCOPY);
	}

	threadsafe oRef<oWindow> Window;
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

		inline HWND Hwnd() threadsafe { return (HWND)thread_cast<oWindow*>(Window.c_ptr())->GetNativeHandle(); }

		RoundedBoxD2D(threadsafe oWindow* _pWindow, const DESC* _pDesc)
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
			RenderTarget = static_cast<threadsafe oWindow_Impl*>(Window.c_ptr())->RenderTarget;
			return CreateBrush(Desc.Color, RenderTarget, &Brush) && CreateBrush(Desc.BorderColor, RenderTarget, &BorderBrush);
		}

		void Close() override { Brush = 0; BorderBrush = 0; RenderTarget = 0; }
		bool Begin() override { return true; }
		void End() override {}
		bool HandleMessage(void* _pPlatformData) override { return true; }
		void GetDesc(DESC* _pDesc) const threadsafe override { oMutex::ScopedLock lock(Mutex); *_pDesc = thread_cast<DESC&>(Desc); }
		void SetDesc(DESC* _pDesc) threadsafe override
		{
			oMutex::ScopedLock lock(Mutex);

			//@oooii-Andrew: using thread_cast because I'm locking above.
			if (Desc.BorderColor != _pDesc->BorderColor)
			{
				oRef<ID2D1SolidColorBrush> newBrush;
				CreateBrush(_pDesc->BorderColor, 
					thread_cast<ID2D1RenderTarget*>(RenderTarget.c_ptr()), 
					&newBrush);
				BorderBrush = newBrush;
			}
			if (Desc.Color != _pDesc->Color)
			{
				oRef<ID2D1SolidColorBrush> newBrush;
				CreateBrush(_pDesc->Color, 
					thread_cast<ID2D1RenderTarget*>(RenderTarget.c_ptr()),
					&newBrush);
				Brush = newBrush;
			}
			thread_cast<DESC&>(Desc) = *_pDesc;
			InvalidateRect(Hwnd(), 0, TRUE);
		}

		void HandlePaint(void* _pPlatformData) override
		{
			oMutex::ScopedLock lock(Mutex);

			D2D1_RECT_F rAdjusted;
			GetAdjustedRect(Hwnd(), Desc.X, Desc.Y, Desc.Width, Desc.Height, Desc.Anchor, &rAdjusted);
			oASSERT(RenderTarget, "No render target");
			if (oEqual(Desc.Roundness, 0.0f))
			{
				if (!oIsTransparentColor(Desc.Color))
					RenderTarget->FillRectangle(&rAdjusted, Brush);
				if (!oIsTransparentColor(Desc.BorderColor))
					RenderTarget->DrawRectangle(&rAdjusted, BorderBrush);
			}

			else
			{
				D2D1_ROUNDED_RECT r;
				r.rect = rAdjusted;
				r.radiusX = Desc.Roundness;
				r.radiusY = Desc.Roundness;
				if (!oIsTransparentColor(Desc.Color))
					RenderTarget->FillRoundedRectangle(&r, Brush);
				if (!oIsTransparentColor(Desc.BorderColor))
					RenderTarget->DrawRoundedRectangle(&r, BorderBrush);
			}
		}

		threadsafe oRef<oWindow> Window;
		oRef<ID2D1RenderTarget> RenderTarget;
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

		inline HWND Hwnd() threadsafe { return (HWND)thread_cast<oWindow*>(Window.c_ptr())->GetNativeHandle(); }

		LineD2D(threadsafe oWindow* _pWindow, const DESC* _pDesc)
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
			RenderTarget = static_cast<threadsafe oWindow_Impl*>(Window.c_ptr())->RenderTarget;
			return CreateBrush(Desc.Color, RenderTarget, &Brush) && CreateBrush(Desc.Color, RenderTarget, &Brush);
		}

		void Close() override { Brush = 0; RenderTarget = 0; }
		bool Begin() override { return true; }
		void End() override {}
		bool HandleMessage(void* _pPlatformData) override { return true; }
		void GetDesc(DESC* _pDesc) const threadsafe override { oMutex::ScopedLock lock(Mutex); *_pDesc = thread_cast<DESC&>(Desc); }
		void SetDesc(DESC* _pDesc) threadsafe override
		{
			oMutex::ScopedLock lock(Mutex);
			//@oooii-Andrew: using thread_cast because I'm locking above
			if (Desc.Color != _pDesc->Color)
			{
				oRef<ID2D1SolidColorBrush> newBrush;
				CreateBrush(_pDesc->Color, thread_cast<ID2D1RenderTarget*>(RenderTarget.c_ptr()), &newBrush);
				Brush = newBrush;
			}
			thread_cast<DESC&>(Desc) = *_pDesc;
			InvalidateRect(Hwnd(), 0, TRUE);
		}

		void HandlePaint(void* _pPlatformData) override
		{
			oMutex::ScopedLock lock(Mutex);
			oASSERT(RenderTarget, "No render target");
			if (!oIsTransparentColor(Desc.Color))
			{
				D2D1_POINT_2F p0, p1;
				p0.x = static_cast<float>(Desc.X1); p0.y = static_cast<float>(Desc.Y1);
				p1.x = static_cast<float>(Desc.X2); p1.y = static_cast<float>(Desc.Y2);
				RenderTarget->DrawLine(p0, p1, Brush, static_cast<float>(Desc.Thickness));
			}
		}

		threadsafe oRef<oWindow> Window;
		oRef<ID2D1RenderTarget> RenderTarget;
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

		threadsafe oRef<oWindow> Window;
		oRef<IDWriteTextFormat> Format;
		DESC Desc;
		oRefCount RefCount;
	};

	struct TextD2D : public oWindow::Text
	{
		oDEFINE_REFCOUNT_INTERFACE(RefCount);
		oDEFINE_TRIVIAL_QUERYINTERFACE(oGetGUID<TextD2D>());
		oDEFINE_GETINTERFACE_INTERFACE(threadsafe, GetWindow, threadsafe, oWindow, Window);

		inline HWND Hwnd() threadsafe { return (HWND)thread_cast<oWindow*>(Window.c_ptr())->GetNativeHandle(); }

		TextD2D(threadsafe oWindow* _pWindow, threadsafe oWindow::Font* _pFont, const DESC* _pDesc)
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
			RenderTarget = static_cast<threadsafe oWindow_Impl*>(Window.c_ptr())->RenderTarget;
			return CreateBrush(Desc.Color, RenderTarget, &Brush) && CreateBrush(Desc.ShadowColor, RenderTarget, &ShadowBrush);
		}

		void Close() override { Brush = 0; ShadowBrush = 0; RenderTarget = 0; }
		bool Begin() override { return true; }
		void End() override {}
		bool HandleMessage(void* _pPlatformData) override { return true; }

		void GetDesc(DESC* _pDesc) const threadsafe override { oMutex::ScopedLock lock(Mutex); *_pDesc = thread_cast<DESC&>(Desc); }
		void SetDesc(const DESC* _pDesc) threadsafe override
		{
			oMutex::ScopedLock lock(Mutex);
			TextD2D* pThis = thread_cast<TextD2D*>(this);
			pThis->Desc = *_pDesc;
			if (Desc.Color != _pDesc->Color)
				CreateBrush(_pDesc->Color, pThis->RenderTarget, &pThis->Brush);
			if (_pDesc->ShadowColor != Desc.ShadowColor)
				CreateBrush(_pDesc->ShadowColor, pThis->RenderTarget, &pThis->ShadowBrush);
			StringLength = static_cast<UINT>(oStrConvert(pThis->String, _pDesc->String));
			InvalidateRect(Hwnd(), 0, TRUE);
		}

		void SetFont(threadsafe oWindow::Font* _pFont) threadsafe
		{
			oMutex::ScopedLock lock(Mutex);
			Font = _pFont;
			InvalidateRect(Hwnd(), 0, TRUE);
		}

		void HandlePaint(void* _pPlatformData) override
		{
			D2D1_RECT_F rAdjusted;
			GetAdjustedRect(Hwnd(), Desc.X, Desc.Y, Desc.Width, Desc.Height, Desc.Anchor, &rAdjusted);

			DWRITE_PARAGRAPH_ALIGNMENT paragraphAlignment;
			DWRITE_TEXT_ALIGNMENT textAlignment;
			GetAlignment(Desc.Alignment, &paragraphAlignment, &textAlignment);

			oASSERT(RenderTarget, "No render target");

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
				RenderTarget->DrawText(String, StringLength, pFormat, rShadow, ShadowBrush);
			}

			oASSERT(Brush, "No brush created");
			RenderTarget->DrawText(String, StringLength, pFormat, rAdjusted, Brush);
		}

		threadsafe oRef<oWindow> Window;
		oRef<oWindow::Font> Font;
		oRef<ID2D1RenderTarget> RenderTarget;
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

		inline HWND Hwnd() threadsafe { return (HWND)thread_cast<oWindow*>(Window.c_ptr())->GetNativeHandle(); }

		PictureD2D(threadsafe oWindow* _pWindow, const Picture::DESC* _pDesc)
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
			RenderTarget = static_cast<threadsafe oWindow_Impl*>(Window.c_ptr())->RenderTarget;

			D2D1_SIZE_U size = { Desc.SurfaceDesc.Width, Desc.SurfaceDesc.Height };
			D2D1_BITMAP_PROPERTIES properties;
			properties.pixelFormat.format = DXGI_FORMAT_B8G8R8A8_UNORM;
			if (properties.pixelFormat.format == DXGI_FORMAT_UNKNOWN)
				return false;
			properties.pixelFormat.alphaMode = D2D1_ALPHA_MODE_IGNORE;
			oRef<ID2D1Factory> pFactory;
			oD2D1CreateFactory(&pFactory);
			pFactory->GetDesktopDpi(&properties.dpiX, &properties.dpiY);
			oV(RenderTarget->CreateBitmap(size, 0, 0, &properties, &Bitmap));
			return !!Bitmap;
		}

		void Close() override
		{
			Bitmap = 0;
			RenderTarget = 0;
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
				oMutex::ScopedLock lock(BitmapLock);
				thread_cast<ID2D1Bitmap*>(Bitmap.c_ptr())->CopyFromMemory(0, _pSourceData, (UINT32)_SourcePitch);
				FlippedHorizontally = _FlipHorizontally;
				FlippedVertically = _FlipVertically;
				InvalidateRect(Hwnd(), 0, FALSE); // @oooii-tony: This could be resized to just the area of the bitmap
			}
		}

		void HandlePaint(void* _pPlatformData) override
		{
			RECT cr;
			GetClientRect(Hwnd(), &cr);

			D2D1_RECT_F r;
			GetAdjustedRect(Hwnd(), Desc.X, Desc.Y, Desc.Width, Desc.Height, Desc.Anchor, &r);

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

			RenderTarget->SetTransform(flipMatrix);
			RenderTarget->DrawBitmap(Bitmap, r);
			RenderTarget->SetTransform(D2D1::Matrix3x2F::Identity());
		}

		threadsafe oRef<oWindow> Window;
		oRef<ID2D1RenderTarget> RenderTarget;
		oRef<ID2D1Bitmap> Bitmap;
		oMutex BitmapLock;
		DESC Desc;
		oRefCount RefCount;
		bool FlippedHorizontally;
		bool FlippedVertically;
	};

#endif

} // namespace detail

bool oWindow::Create(const DESC* _pDesc, const char* _Title, unsigned int _FourCCDrawAPI, oWindow** _ppWindow)
{
	if (!_pDesc || !_ppWindow) return false;
	*_ppWindow = new oWindow_Impl(_pDesc, _Title, _FourCCDrawAPI);
	return !!*_ppWindow;
}

LRESULT CALLBACK oWindow_Impl::StaticWndProc(HWND _hWnd, UINT _uMsg, WPARAM _wParam, LPARAM _lParam)
{
	//char buf[4096];
	//oTRACE("%s", oGetWMDesc(buf, _hWnd, _uMsg, _wParam, _lParam));
	oWindow_Impl* pThis = (oWindow_Impl*)oGetWindowContext(_hWnd, _uMsg, _wParam, _lParam);
	return pThis ? pThis->WndProc(_hWnd, _uMsg, _wParam, _lParam) : DefWindowProc(_hWnd, _uMsg, _wParam, _lParam);
}

oWindow_Impl::oWindow_Impl(const DESC* _pDesc, const char* _Title, unsigned int _FourCCDrawAPI)
	: hWnd(0)
	, hIcon(0)
	, hClearBrush(0)
	, hOffscreenAADC(0)
	, hOffscreenAABmp(0)
	, hOffscreenBmp(0)
	, hOffscreenDC(0)
	, ClearColor(std::Black)
	, GDIAntialiasingMultiplier(_pDesc->UseAntialiasing ? 4 : 1)
	, UseAntialiasing(_pDesc->UseAntialiasing)
	, Closing(false)
	, DrawMethod(DRAW_USING_GDI)
{
#if oDXVER >= oDXVER_10
	oRef<ID2D1Factory> pFactory;
	oD2D1CreateFactory(&pFactory);

	DrawMethod = (pFactory ? DRAW_USING_D2D : DRAW_USING_GDI);
#endif

	if (_FourCCDrawAPI == 'D2D1')
		DrawMethod = DRAW_USING_D2D;
	else if (_FourCCDrawAPI == 'GDI ' || _FourCCDrawAPI == 'GDI')
		DrawMethod = DRAW_USING_GDI;

	*Title = 0;
	if (_pDesc)
		oWindow_Impl::Open(_pDesc, _Title);

	oTRACE("Created window \"%s\" using %s for drawing.", _Title, DrawMethod == DRAW_USING_D2D ? "D2D" : "GDI");
}

oWindow_Impl::~oWindow_Impl()
{
	Close();
	DiscardDeviceResources();
}

void oWindow_Impl::GetDesc(oWindow::DESC* _pDesc) const threadsafe
{
	oASSERT(hWnd, "");
	oMutex::ScopedLock lock(DescLock);
	detail::GetDesc(hWnd, _pDesc);
	_pDesc->UseAntialiasing = UseAntialiasing;
}

void oWindow_Impl::SetDesc(const oWindow::DESC* _pDesc) threadsafe
{
	oASSERT(hWnd, "");
	oMutex::ScopedLock lock(DescLock);
	detail::SetDesc(hWnd, _pDesc);
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
	
	if (FAILED(oCreateSimpleWindow(&hWnd, StaticWndProc, this, _Title)))
		return false;

	strcpy_s(Title, _Title);

	oV(CreateDeviceResources());
	if (!OpenChildren())
		return false;

	oWindow::DESC wDesc;
	detail::GetDefaultDesc(hWnd, _pDesc, &wDesc);
	return detail::SetDesc(hWnd, &wDesc);
}

void oWindow_Impl::Close()
{
	oMutex::ScopedLock lock(DescLock);
	if (hWnd)
	{
		DESC desc;
		detail::GetDesc(hWnd, &desc);
		desc.State = HIDDEN;
		detail::SetDesc(hWnd, &desc);
		Closing = true;
		CloseChildren();
		DiscardDeviceResources();
		DestroyWindow(hWnd);
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
	oPumpMessages(hWnd);
	if (!HasFocus())
		Sleep(200);
	return true;
}

void oWindow_Impl::End()
{
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

void* oWindow_Impl::GetNativeHandle()
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
	HRESULT hr = S_OK;

	#if oDXVER >= oDXVER_10

		if (DrawMethod == DRAW_USING_D2D && !RenderTarget)
		{
			RECT rClient;
			GetClientRect(hWnd, &rClient);
			D2D1_SIZE_U size = D2D1::SizeU(rClient.right - rClient.left, rClient.bottom - rClient.top);
			oRef<ID2D1Factory> pFactory;
			oD2D1CreateFactory(&pFactory);
			if(pFactory)
				hr = pFactory->CreateHwndRenderTarget(D2D1::RenderTargetProperties(), D2D1::HwndRenderTargetProperties(hWnd, size), &RenderTarget);

			if (SUCCEEDED(hr))
				RenderTarget->SetAntialiasMode(UseAntialiasing ? D2D1_ANTIALIAS_MODE_PER_PRIMITIVE : D2D1_ANTIALIAS_MODE_ALIASED);
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

	return hr;
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

bool oWindow_Impl::CreateResizer(ResizeHandlerFn _ResizeHandler, void* _pUserData, threadsafe Resizer** _ppResizer) threadsafe
{
	if (!_ResizeHandler || !_ppResizer) return false;
	*_ppResizer = new detail::oWindowResizer_Impl(this, _ResizeHandler, _pUserData);
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

	bool result = oImage::Create(buf, bufSize, _ppImage);
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
		if (DrawMethod == DRAW_USING_D2D && RenderTarget)
		{
			RenderTarget->Resize(D2D1::SizeU(_NewWidth, _NewHeight));
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

				Pictures.foreach(oBIND( &oWindow_Impl::PaintChild<oWindow::Picture>, oBIND1, (void*)0 ));
				Lines.foreach(oBIND( &oWindow_Impl::PaintChild<oWindow::Line>, oBIND1, (void*)0 ));
				RoundedBoxes.foreach(oBIND( &oWindow_Impl::PaintChild<oWindow::RoundedBox>, oBIND1, (void*)0 ));
				Texts.foreach(oBIND( &oWindow_Impl::PaintChild<oWindow::Text>, oBIND1, (void*)0 ));

				hr = RenderTarget->EndDraw();
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
		if (!EnableCloseButton)	//@oooii-Andrew
			return 0;
	case WM_DESTROY:
		OnClose();
		return 0;

	case WM_SYSKEYDOWN:
	case WM_SYSKEYUP:
		//if (_wParam == VK_RETURN)
		//	oTRACE("TOGGLE FULLSCREEN");
		break;

	default:
		break;
	}

	return DefWindowProc(_hWnd, _uMsg, _wParam, _lParam);
}

bool oWindow::Pump(oWindow* _pWindow, bool _CloseOnTimeout, unsigned int _Timeout)
{
	unsigned int time = oTimerMS();
	while (_pWindow->IsOpen())
	{
		if (_pWindow->Begin())
			_pWindow->End();

		if (_Timeout != ~0u && (oTimerMS() - time) > _Timeout)
		{
			if (_CloseOnTimeout)
				_pWindow->Close();

			return false;
		}
	}

	return true;
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