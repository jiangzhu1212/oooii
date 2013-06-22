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
// API to make working with Window's GDI easier.
#pragma once
#ifndef oGDI_h
#define oGDI_h

#include <oStd/oStdAtomic.h>
#include <oStd/color.h>
#include <oBasis/oGUI.h>
#include <oPlatform/Windows/oWindows.h>
#include <oPlatform/Windows/oWinWindowing.h>
#include <oPlatform/Windows/oWinRect.h>

// For functions that take logical height. Point size is like what appears in 
// most Windows API.
int oGDIPointToLogicalHeight(HDC _hDC, int _Point);
int oGDIPointToLogicalHeight(HDC _hDC, float _Point);

// Goes the opposite way of oGDIPointToLogicalHeight() and returns point size
int oGDILogicalHeightToPoint(HDC _hDC, int _Height);
float oGDILogicalHeightToPointF(HDC _hDC, int _Height);

#define oDEFINE_GDI_BOOL_CAST_OPERATORS(_Type, _Member) \
	operator bool() { return !!_Member; } \
	operator bool() const { return !!_Member; } \
	operator bool() volatile { return !!_Member; } \
	operator bool() const volatile { return !!_Member; } \
	operator _Type() { return _Member; } \
	operator _Type() const { return _Member; } \
	operator _Type() volatile { return _Member; } \
	operator _Type() const volatile { return _Member; }

#define oDEFINE_GDI_MOVE_PTR(_Name) do { _Name = _That._Name; _That._Name = nullptr; } while (false)

class oGDIScopedSelect
{
	HDC hDC;
	HGDIOBJ hOldObj;
public:
	oGDIScopedSelect() : hDC(nullptr), hOldObj(nullptr) {}
	oGDIScopedSelect(HDC _hDC, HGDIOBJ _hObj) : hDC(_hDC) { hOldObj = SelectObject(hDC, _hObj); }
	oGDIScopedSelect(oGDIScopedSelect&& _That) { operator=(std::move(_That)); }
	~oGDIScopedSelect() { SelectObject(hDC, hOldObj); }
	
	oGDIScopedSelect& operator=(oGDIScopedSelect&& _That)
	{
		if (this != &_That)
		{
			if (hDC && hOldObj) SelectObject(hDC, hOldObj);
			oDEFINE_GDI_MOVE_PTR(hDC);
			oDEFINE_GDI_MOVE_PTR(hOldObj);
		}
		return *this;
	}
};

class oGDIScopedIcon
{
	HICON hIcon;
	
	oGDIScopedIcon(const oGDIScopedIcon&);
	const oGDIScopedIcon& operator=(const oGDIScopedIcon&);

public:
	oGDIScopedIcon() : hIcon(nullptr) {}
	~oGDIScopedIcon() { if (hIcon) DestroyIcon(hIcon); }
	oGDIScopedIcon(HICON _hIcon) : hIcon(_hIcon) {}
	oGDIScopedIcon(oGDIScopedIcon&& _That) { operator=(_That); }
	
	const oGDIScopedIcon& operator=(HICON _hIcon)
	{
		if (hIcon) DestroyIcon(hIcon);
		hIcon = _hIcon;
		return *this;
	}
	
	const oGDIScopedIcon& operator=(oGDIScopedIcon&& _That)
	{
		if (this != &_That)
		{
			if (hIcon) DestroyIcon(hIcon);
			oDEFINE_GDI_MOVE_PTR(hIcon);
		}

		return *this;
	}

	oDEFINE_GDI_BOOL_CAST_OPERATORS(HICON, hIcon);
};

class oGDIScopedCursor
{
	HCURSOR hCursor;

	oGDIScopedCursor(const oGDIScopedCursor& _That);
	const oGDIScopedCursor& operator=(const oGDIScopedCursor& _That);

public:
	oGDIScopedCursor() : hCursor(nullptr) {}
	~oGDIScopedCursor() { if (hCursor) DestroyCursor(hCursor); }
	oGDIScopedCursor(HCURSOR _hCursor) : hCursor(_hCursor) {}
	oGDIScopedCursor(oGDIScopedCursor&& _That) { operator=(_That); }
	
	const oGDIScopedCursor& operator=(HCURSOR _hCursor)
	{
		if (hCursor) DestroyCursor(hCursor);
		hCursor = _hCursor;
		return *this;
	}
	
	const oGDIScopedCursor& operator=(oGDIScopedCursor&& _That)
	{
		if (hCursor) DestroyCursor(hCursor);
		oDEFINE_GDI_MOVE_PTR(hCursor);
		return *this;
	}

	oDEFINE_GDI_BOOL_CAST_OPERATORS(HCURSOR, hCursor);
};

class oGDIScopedGetDC
{
	HWND hWnd;
	HDC hDC;

	oGDIScopedGetDC(const oGDIScopedGetDC&);
	const oGDIScopedGetDC& operator=(const oGDIScopedGetDC&);

public:
	oGDIScopedGetDC() : hWnd(nullptr), hDC(nullptr) {}
	oGDIScopedGetDC(HWND _hWnd) : hWnd(_hWnd), hDC(GetDC(_hWnd)) {}
	oGDIScopedGetDC(oGDIScopedGetDC&& _That) { operator=(_That); }
	~oGDIScopedGetDC() { if (hWnd && hDC) ReleaseDC(hWnd, hDC); }

	oGDIScopedGetDC& operator=(oGDIScopedGetDC&& _That)
	{
		if (this != &_That)
		{
			if (hWnd && hDC)
				ReleaseDC(hWnd, hDC);
			oDEFINE_GDI_MOVE_PTR(hWnd);
			oDEFINE_GDI_MOVE_PTR(hDC);
		}
		return *this;
	}

	oDEFINE_GDI_BOOL_CAST_OPERATORS(HDC, hDC);
};

class oGDIScopedDC
{
	HDC hDC;

	oGDIScopedDC(const oGDIScopedDC& _That);
	const oGDIScopedDC& operator=(const oGDIScopedDC& _That);

public:
	oGDIScopedDC() : hDC(nullptr) {}
	~oGDIScopedDC() { if (hDC) DeleteDC(hDC); }
	oGDIScopedDC(HDC _hDC) : hDC(_hDC) {}
	oGDIScopedDC(oGDIScopedDC&& _That) { operator=(_That); }

	const oGDIScopedDC& operator=(HDC _hDC)
	{
		if (hDC) DeleteDC(hDC);
		hDC = _hDC;
		return *this;
	}

	oGDIScopedDC& operator=(oGDIScopedDC&& _That)
	{
		if (this != &_That)
		{
			if (hDC) DeleteDC(hDC);
			oDEFINE_GDI_MOVE_PTR(hDC);
		}
		return *this;
	}

	oDEFINE_GDI_BOOL_CAST_OPERATORS(HDC, hDC);
};

class oGDIScopedBltMode
{
	HDC hDC;
	int PrevMode;

	oGDIScopedBltMode(const oGDIScopedBltMode& _That);
	const oGDIScopedBltMode& operator=(const oGDIScopedBltMode& _That);

public:
	oGDIScopedBltMode() : hDC(nullptr) {}
	~oGDIScopedBltMode() { if (hDC) SetStretchBltMode(hDC, PrevMode); }
	oGDIScopedBltMode(HDC _hDC, int _Mode) : hDC(_hDC), PrevMode(SetStretchBltMode(hDC, _Mode)) {}
	oGDIScopedBltMode(oGDIScopedBltMode&& _That) { operator=(_That); }

	oGDIScopedBltMode& operator=(oGDIScopedBltMode&& _That)
	{
		if (this != &_That)
		{
			oDEFINE_GDI_MOVE_PTR(hDC);
			PrevMode = _That.PrevMode;
		}
		return *this;
	}

	oDEFINE_GDI_BOOL_CAST_OPERATORS(HDC, hDC);
};

class oGDIScopedBkMode
{
	HDC hDC;
	int PrevMode;

	oGDIScopedBkMode(const oGDIScopedBkMode& _That);
	const oGDIScopedBkMode& operator=(const oGDIScopedBkMode& _That);

public:
	oGDIScopedBkMode() : hDC(nullptr) {}
	~oGDIScopedBkMode() { if (hDC) SetStretchBltMode(hDC, PrevMode); }
	oGDIScopedBkMode(HDC _hDC, int _Mode) : hDC(_hDC), PrevMode(SetBkMode(hDC, _Mode)) {}
	oGDIScopedBkMode(oGDIScopedBkMode&& _That) { operator=(_That); }

	oGDIScopedBkMode& operator=(oGDIScopedBkMode&& _That)
	{
		if (this != &_That)
		{
			oDEFINE_GDI_MOVE_PTR(hDC);
			PrevMode = _That.PrevMode;
		}
		return *this;
	}

	oDEFINE_GDI_BOOL_CAST_OPERATORS(HDC, hDC);
};

template<typename HGDIOBJType> class oGDIScopedObject
{
	oStd::atomic<HGDIOBJType> hObject;

	oGDIScopedObject(const oGDIScopedObject& _That);
	const oGDIScopedObject& operator=(const oGDIScopedObject& _That);

public:
	oGDIScopedObject() : hObject(nullptr) {}
	oGDIScopedObject(oGDIScopedObject&& _That) { operator=(std::move(_That)); }
	oGDIScopedObject(HGDIOBJType _hObject) : hObject(_hObject) {}
	~oGDIScopedObject() { if (hObject) DeleteObject(hObject); }
	
	// threadsafe in the sense that this swaps the internal value and then 
	// after that's done cleans up the object. This way one thread can assign
	// null and another thread can test for null and recreate the object as
	// appropriate
	const oGDIScopedObject& operator=(HGDIOBJType _hObject)
	{
		HGDIOBJType hTemp = hObject;
		hObject.exchange(_hObject);
		if (hTemp) DeleteObject(hTemp); 
		return *this;
	}

	oGDIScopedObject& operator=(oGDIScopedObject&& _That)
	{
		if (this != &_That)
		{
			HGDIOBJType hTemp = _That.hObject;
			_That.hObject.exchange(nullptr);
			HGDIOBJType hThisTemp = hObject;
			hObject.exchange(hTemp);
			if (hThisTemp) DeleteObject(hThisTemp);
		}
		return *this;
	}

	const threadsafe oGDIScopedObject& operator=(HGDIOBJType _hObject) threadsafe
	{
		HGDIOBJType hTemp = hObject;
		hObject.exchange(_hObject);
		if (hTemp) DeleteObject(hTemp); 
		return *this;
	}

	oDEFINE_GDI_BOOL_CAST_OPERATORS(HGDIOBJType, hObject);
};

class oGDIScopedOffscreen
{
	oGDIScopedGetDC hDCWin;
	oGDIScopedDC hDCOffscreen;
	oGDIScopedObject<HBITMAP> hOffscreen;
	oGDIScopedSelect SelectOffscreen;
	RECT rClient;

public:
	oGDIScopedOffscreen() {}
	oGDIScopedOffscreen(HWND _hWnd)
		: hDCWin(_hWnd)
		, hDCOffscreen(CreateCompatibleDC(hDCWin))
	{
		oWinGetClientRect(_hWnd, &rClient);
		hOffscreen = CreateCompatibleBitmap(hDCWin, oWinRectW(rClient), oWinRectH(rClient));
		SelectOffscreen = std::move(oGDIScopedSelect(hDCOffscreen, hOffscreen));
	}
	oGDIScopedOffscreen(oGDIScopedOffscreen&& _That) { operator=(std::move(_That)); }
	~oGDIScopedOffscreen() { BitBlt(hDCWin, rClient.left, rClient.top, oWinRectW(rClient), oWinRectH(rClient), hDCOffscreen, rClient.left, rClient.top, SRCCOPY); }

	const oGDIScopedOffscreen& operator=(HWND _hWnd)
	{
		oGDIScopedOffscreen off(_hWnd);
		*this = std::move(off);
	}

	oGDIScopedOffscreen& operator=(oGDIScopedOffscreen&& _That)
	{
		if (this != &_That)
		{
			hDCWin = std::move(_That.hDCWin);
			hDCOffscreen = std::move(_That.hDCOffscreen);
			hOffscreen = std::move(_That.hOffscreen);
			rClient = std::move(_That.rClient);
		}
		return *this;
	}

	oDEFINE_GDI_BOOL_CAST_OPERATORS(HDC, hDCOffscreen);
};

// _____________________________________________________________________________
// Bitmap APIs

struct oBMI_DESC
{
	oBMI_DESC()
		: Dimensions(oInvalid, oInvalid)
		, Format(oSURFACE_UNKNOWN)
		, RowPitch(oInvalid)
		, FlipVertically(true)
		, ARGBMonochrome8Zero(oStd::Black)
		, ARGBMonochrome8One(oStd::White)
	{}

	int2 Dimensions;
	oSURFACE_FORMAT Format;
	int RowPitch;
	bool FlipVertically;
	oStd::color ARGBMonochrome8Zero;
	oStd::color ARGBMonochrome8One;
};

// Initialize a BITMAPINFO with information from the specified oBMI_DESC. The 
// specified _pBMI should be allocated to the number of bytes returned by 
// oGDIGetBMISize. If an 8-bit format is specified, palette data will be 
// initialized with a gradient going from zero to one as specified in the desc.
void oGDIInitializeBMI(const oBMI_DESC& _Desc, BITMAPINFO* _pBMI);

// 8 bit formats won't render correctly because BITMAPINFO infers palette data 
// from 8-bit, so allocate enough room for the palette.
size_t oGDIGetBMISize(oSURFACE_FORMAT _Format);
inline size_t oGDIGetBMISize(oImage::FORMAT _Format) { return oGDIGetBMISize(oImageFormatToSurfaceFormat(_Format)); }

oSURFACE_FORMAT oGDIGetFormat(const BITMAPINFOHEADER& _BitmapInfoHeader);

// Captures image data of the specified window and fills _pImageBuffer.
// pImageBuffer can be NULL, in which case only _pBitmapInfo is filled out.
// For RECT, either specify a client-space rectangle, or NULL to capture the 
// window including the frame.
bool oGDIScreenCaptureWindow(HWND _hWnd, const RECT* _pRect, void* _pImageBuffer, size_t _SizeofImageBuffer, BITMAPINFO* _pBitmapInfo, bool _RedrawWindow);

// Given an allocator, this will allocate the properly-sized buffer and fill it
// with the captured window image using the above oGDIScreenCaptureWindow() API.
// If the buffer is fwritten to a file, it would be a .bmp. This function 
// internally set *_ppBuffer to nullptr. Later when the allocation happens, 
// there is still a chance of failure, so whether this succeeds or fails, check
// *_ppBuffer and if not nullptr, then ensure it is freed to prevent a memory 
// leak.
bool oGDIScreenCaptureWindow(HWND _hWnd, bool _IncludeBorder, oFUNCTION<void*(size_t _Size)> _Allocate, void** _ppBuffer, size_t* _pBufferSize, bool _RedrawWindow);

// _dwROP is one of the raster operations from BitBlt()
BOOL oGDIDrawBitmap(HDC _hDC, INT _X, INT _Y, HBITMAP _hBitmap, DWORD _dwROP);
BOOL oGDIStretchBitmap(HDC _hDC, INT _X, INT _Y, INT _Width, INT _Height, HBITMAP _hBitmap, DWORD _dwROP);
BOOL oGDIStretchBlendBitmap(HDC _hDC, INT _X, INT _Y, INT _Width, INT _Height, HBITMAP _hBitmap);

// This stretches the source bits directly to fill the specified rectangle in
// the specified device context.
bool oGDIStretchBits(HDC _hDC, const RECT& _DestRect, const int2& _SourceSize, oSURFACE_FORMAT _SourceFormat, const void* _pSourceBits, int _SourceRowPitch, bool _FlipVertically = true);

// This stretches the source bits directly to fill the specified HWND's client
// area. This is a nice shortcut when working with cameras that will fill some
// UI element, or a top-level window.
bool oGDIStretchBits(HWND _hWnd, const int2& _SourceSize, oSURFACE_FORMAT _SourceFormat, const void* _pSourceBits, int _SourceRowPitch, bool _FlipVertically = true);
bool oGDIStretchBits(HWND _hWnd, const RECT& _DestRect, const int2& _SourceSize, oSURFACE_FORMAT _SourceFormat, const void* _pSourceBits, int _SourceRowPitch, bool _FlipVertically = true);
// _____________________________________________________________________________
// Other APIs

int2 oGDIGetDimensions(HDC _hDC);

bool oGDIDrawLine(HDC _hDC, const int2& _P0, const int2& _P1);

// Uses the currently bound pen and brush to draw a box (also can be used to 
// draw a circle if a high roundness is used). If Alpha is [0,1), then a uniform 
// alpha blend is done for the box.
bool oGDIDrawBox(HDC _hDC, const RECT& _rBox, int _EdgeRoundness = 0, float _Alpha = 1.0f);

// Uses the currently bound pen and brush to draw an ellipse
bool oGDIDrawEllipse(HDC _hDC, const RECT& _rBox);

// Returns the rect required for a single line of text using the specified HDC's 
// font and other settings.
oAPI RECT oGDICalcTextRect(HDC _hDC, const char* _Text);

// Draws text using GDI.
bool oGDIDrawText(HDC _hDC, const oGUI_TEXT_DESC& _Desc, const char* _Text);

// If color alpha is true 0, then a null/empty objects is returned. Use 
// DeleteObject on the value returned from these functions when finish with the
// object. (width == 0 means "default")
HPEN oGDICreatePen(oStd::color _Color, int _Width = 0);
HBRUSH oGDICreateBrush(oStd::color _Color);

inline HBITMAP oGDIGetBitmap(HDC _hDC) { return (HBITMAP)GetCurrentObject(_hDC, OBJ_BITMAP); }
inline HBRUSH oGDIGetBrush(HDC _hDC) { return (HBRUSH)GetCurrentObject(_hDC, OBJ_BRUSH); }
inline HFONT oGDIGetFont(HDC _hDC) { return (HFONT)GetCurrentObject(_hDC, OBJ_FONT); }
inline HPEN oGDIGetPen(HDC _hDC) { return (HPEN)GetCurrentObject(_hDC, OBJ_PEN); }

COLORREF oGDIGetPenColor(HPEN _hPen, int* _pWidth = nullptr);

// Returns the COLORREF of the specified brush
COLORREF oGDIGetBrushColor(HBRUSH _hBrush);

int2 oGDIGetIconSize(HICON _hIcon);

HFONT oGDICreateFont(const oGUI_FONT_DESC& _Desc);
void oGDIGetFontDesc(HFONT _hFont, oGUI_FONT_DESC* _pDesc);

const char* oGDIGetFontFamily(BYTE _tmPitchAndFamily);
const char* oGDIGetCharSet(BYTE _tmCharSet);

// _____________________________________________________________________________
// More complex utilities

class oGDISelectionBox
{
	// Like when you drag to select several files in Windows Explorer. This uses
	// SetCapture() and ReleaseCapture() on hParent to retain mouse control while
	// drawing the selection box.

	oGDISelectionBox(const oGDISelectionBox&);
	const oGDISelectionBox& operator=(const oGDISelectionBox&);

public:
	struct DESC
	{
		DESC()
			: hParent(nullptr)
			, Fill(oStd::color(oStd::DodgerBlue, 0.33f))
			, Border(oStd::DodgerBlue)
			, EdgeRoundness(0)
			, UseOffscreenRender(false)
		{}

		HWND hParent;
		oStd::color Fill; // can have alpha value
		oStd::color Border;
		int EdgeRoundness;

		// DXGI back-buffers don't like rounded edge rendering, so copy contents
		// to an offscreen HBITMAP, draw the rect and resolve it back to the main
		// target. Even if this is true, offscreen render only occurs if there is
		// edge roundness.
		bool UseOffscreenRender;
	};

	oGDISelectionBox();
	oGDISelectionBox(oGDISelectionBox&& _That) { operator=(_That); }
	const oGDISelectionBox& operator=(oGDISelectionBox&& _That);

	void SetDesc(const DESC& _Desc);
	void GetDesc(DESC* _pDesc);

	// returns true if drawing is required by the state (between mouse down and 
	// mouse up events). This is useful if there's non-trivial effort in 
	// extracting the HDC that draw will use.
	bool IsSelecting() const;

	void Draw(HDC _hDC);
	void OnResize(const int2& _NewParentSize);
	void OnMouseDown(const int2& _MousePosition);
	void OnMouseMove(const int2& _MousePosition);
	void OnMouseUp();

private:
	oGDIScopedObject<HPEN> hPen;
	oGDIScopedObject<HBRUSH> hBrush;
	oGDIScopedObject<HBRUSH> hNullBrush;
	oGDIScopedObject<HBITMAP> hOffscreenBMP;

	int2 MouseDownAt;
	int2 MouseAt;
	float Opacity;
	bool Selecting;

	DESC Desc;
};


#endif
