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
// API to make working with Window's GDI bitmaps and icons easier.
#pragma once
#ifndef oGUI_win_gdi_bitmap_h
#define oGUI_win_gdi_bitmap_h

#include <oGUI/Windows/win_gdi.h>
#include <oSurface/buffer.h>

namespace ouro {
	namespace windows {
		namespace gdi {

// Supported formats: b8g8r8_unorm b8g8r8a8_unorm r8_unorm. Must be 2D non-array
BITMAPINFOHEADER make_header(const surface::info& _SurfaceInfo, bool _TopDown = true);
BITMAPV4HEADER make_headerv4(const surface::info& _SurfaceInfo, bool _TopDown = true);
void fill_monochrone_palette(RGBQUAD* _pColors, color _Color0 = black, color _Color1 = white);

surface::info get_info(const BITMAPINFOHEADER& _Header);
surface::info get_info(const BITMAPV4HEADER& _Header);
inline surface::info get_info(const BITMAPINFO& _Info) { return get_info(_Info.bmiHeader); }

// Creates a bitmap with the contents of the specified buffer.
scoped_bitmap make_bitmap(const surface::buffer* _pBuffer);

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
int2 bitmap_dimensions(HDC _hDC);

int2 icon_dimensions(HICON _hIcon);

// Use DeleteObject when finished with the returned handle
HBITMAP bitmap_from_icon(HICON _hIcon);

// Use DestroyIcon when finished with the return handle
HICON bitmap_to_icon(HBITMAP _hBmp);

// Use DestroyIcon when finished with the return handle
inline HICON load_icon(int _ResourceID) { return (HICON)LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(_ResourceID), IMAGE_ICON, 0, 0, 0); }

		} // namespace gdi
	} // namespace windows
} // namespace ouro

#endif
