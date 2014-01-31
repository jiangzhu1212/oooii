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
// API to make working with Window's GDI easier.
#pragma once
#ifndef oGDI_h
#define oGDI_h

#include <oBase/color.h>
#include <oGUI/oGUI.h>
#include <oGUI/Windows/win_gdi.h>
#include <oSurface/buffer.h>
#include <atomic>

// _____________________________________________________________________________
// Bitmap APIs

namespace ouro {
	namespace windows {
		namespace gdi {

// Supported formats: b8g8r8_unorm b8g8r8a8_unorm r8_unorm. Must be 2D non-array
BITMAPINFOHEADER make_header(const surface::info& _SurfaceInfo, bool _TopDown = true);
BITMAPV4HEADER make_headerv4(const surface::info& _SurfaceInfo, bool _TopDown = true);
void fill_monochrone_palette(RGBQUAD* _pColors, color _Color0 = Black, color _Color1 = White);

surface::info get_info(const BITMAPINFOHEADER& _Header);
surface::info get_info(const BITMAPV4HEADER& _Header);
inline surface::info get_info(const BITMAPINFO& _Info) { return get_info(_Info.bmiHeader); }

// Creates a bitmap with the contents of the specified buffer.
scoped_hbitmap make_bitmap(const surface::buffer* _pBuffer);

// Copy the contents of a bitmap to the specified buffer.
void memcpy2d(void* _pDestination, size_t _DestinationPitch, HBITMAP _hBmp, size_t _NumRows, bool _FlipVertically = false);

// Draws the specified bitmap to the device context using the ROP operation
// as described in BitBlt documentation.
void draw_bitmap(HDC _hDC, int _X, int _Y, HBITMAP _hBitmap, DWORD _dwROP);
void stretch_bitmap(HDC _hDC, int _X, int _Y, int _Width, int _Height, HBITMAP _hBitmap, DWORD _dwROP);
void stretch_blend_bitmap(HDC _hDC, int _X, int _Y, int _Width, int _Height, HBITMAP _hBitmap);

// This stretches the source bits directly to fill the specified rectangle in
// the specified device context.
void stretch_bits(HDC _hDC, const RECT& _DestRect, const int2& _SourceSize, surface::format _SourceFormat, const void* _pSourceBits, bool _FlipVertically = true);

// This stretches the source bits directly to fill the specified HWND's client
// area. This is a nice shortcut when working with cameras that will fill some
// UI element, or a top-level window.
void stretch_bits(HWND _hWnd, const int2& _SourceSize, surface::format _SourceFormat, const void* _pSourceBits, bool _FlipVertically = true);
void stretch_bits(HWND _hWnd, const RECT& _DestRect, const int2& _SourceSize, surface::format _SourceFormat, const void* _pSourceBits, bool _FlipVertically = true);

// Returns the width and height of the bitmap in the _hDC
int2 get_bitmap_dimensions(HDC _hDC);

		} // namespace gdi
	} // namespace windows
} // namespace ouro

namespace ouro {
	namespace windows {
		namespace gdi {

class selection_box
{
	// Like when you drag to select several files in Windows Explorer. This uses
	// SetCapture() and ReleaseCapture() on hParent to retain mouse control while
	// drawing the selection box.

	selection_box(const selection_box&);
	const selection_box& operator=(const selection_box&);

public:
	struct DESC
	{
		DESC()
			: hParent(nullptr)
			, Fill(color(DodgerBlue, 0.33f))
			, Border(DodgerBlue)
			, EdgeRoundness(0)
			, UseOffscreenRender(false)
		{}

		HWND hParent;
		color Fill; // can have alpha value
		color Border;
		int EdgeRoundness;

		// DXGI back-buffers don't like rounded edge rendering, so copy contents
		// to an offscreen HBITMAP, draw the rect and resolve it back to the main
		// target. Even if this is true, offscreen render only occurs if there is
		// edge roundness.
		bool UseOffscreenRender;
	};

	selection_box();
	selection_box(selection_box&& _That) { operator=(_That); }
	const selection_box& operator=(selection_box&& _That);

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
	scoped_hpen hPen;
	scoped_hbrush hBrush;
	scoped_hbrush hNullBrush;
	scoped_hbitmap hOffscreenBMP;

	int2 MouseDownAt;
	int2 MouseAt;
	float Opacity;
	bool Selecting;

	DESC Desc;
};

		} // namespace gdi
	} // namespace windows
} // namespace ouro

// Captures image data of the specified window and fills _pImageBuffer.
// pImageBuffer can be NULL, in which case only _pBitmapInfo is filled out.
// For RECT, either specify a client-space rectangle, or NULL to capture the 
// window including the frame.
void oGDIScreenCaptureWindow(HWND _hWnd, const RECT* _pRect, void* _pImageBuffer, size_t _SizeofImageBuffer, BITMAPINFO* _pBitmapInfo, bool _RedrawWindow, bool _FlipV);

// Given an allocator, this will allocate the properly-sized buffer and fill it
// with the captured window image using the above oGDIScreenCaptureWindow() API.
// If the buffer is fwritten to a file, it would be a .bmp. This function 
// internally set *_ppBuffer to nullptr. Later when the allocation happens, 
// there is still a chance of failure so if this throws an exception check 
// *_ppBuffer and if not nullptr then ensure it is freed to prevent a memory 
// leak.
void oGDIScreenCaptureWindow(HWND _hWnd, bool _IncludeBorder, std::function<void*(size_t _Size)> _Allocate, void** _ppBuffer, size_t* _pBufferSize, bool _RedrawWindow, bool _FlipV);

// Use DeleteObject when finished with the returned handle
HBITMAP oGDIIconToBitmap(HICON _hIcon);

// Use DestroyIcon when finished with the return handle
HICON oGDIBitmapToIcon(HBITMAP _hBmp);

// Use DestroyIcon when finished with the return handle
inline HICON oGDILoadIcon(int _ResourceID) { return (HICON)LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(_ResourceID), IMAGE_ICON, 0, 0, 0); }

// _____________________________________________________________________________
// Other APIs

bool oGDIDrawLine(HDC _hDC, const int2& _P0, const int2& _P1);

// Uses the currently bound pen and brush to draw a box (also can be used to 
// draw a circle if a high roundness is used). If Alpha is [0,1), then a uniform 
// alpha blend is done for the box.
bool oGDIDrawBox(HDC _hDC, const RECT& _rBox, int _EdgeRoundness = 0, float _Alpha = 1.0f);

// Uses the currently bound pen and brush to draw an ellipse
bool oGDIDrawEllipse(HDC _hDC, const RECT& _rBox);

// Returns the rect required for a single line of text using the specified HDC's 
// font and other settings.
RECT oGDICalcTextRect(HDC _hDC, const char* _Text);

// Draws text using GDI.
bool oGDIDrawText(HDC _hDC, const ouro::text_info& _Desc, const char* _Text);

// If color alpha is true 0, then a null/empty objects is returned. Use 
// DeleteObject on the value returned from these functions when finish with the
// object. (width == 0 means "default")
HPEN oGDICreatePen(ouro::color _Color, int _Width = 0);
HBRUSH oGDICreateBrush(ouro::color _Color);

inline HBITMAP oGDIGetBitmap(HDC _hDC) { return (HBITMAP)GetCurrentObject(_hDC, OBJ_BITMAP); }
inline HBRUSH oGDIGetBrush(HDC _hDC) { return (HBRUSH)GetCurrentObject(_hDC, OBJ_BRUSH); }
inline HFONT oGDIGetFont(HDC _hDC) { return (HFONT)GetCurrentObject(_hDC, OBJ_FONT); }
inline HPEN oGDIGetPen(HDC _hDC) { return (HPEN)GetCurrentObject(_hDC, OBJ_PEN); }

COLORREF oGDIGetPenColor(HPEN _hPen, int* _pWidth = nullptr);

// Returns the COLORREF of the specified brush
COLORREF oGDIGetBrushColor(HBRUSH _hBrush);

int2 oGDIGetIconSize(HICON _hIcon);

HFONT oGDICreateFont(const ouro::font_info& _Desc);
void oGDIGetFontDesc(HFONT _hFont, ouro::font_info* _pDesc);

const char* oGDIGetFontFamily(BYTE _tmPitchAndFamily);
const char* oGDIGetCharSet(BYTE _tmCharSet);

// _____________________________________________________________________________
// More complex utilities

#endif
