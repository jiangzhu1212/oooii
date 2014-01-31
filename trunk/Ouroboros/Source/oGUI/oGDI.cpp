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
#include <oGUI/Windows/oGDI.h>
#include <oCore/windows/win_error.h>
#include <oBase/byte.h>
#include <oGUI/Windows/oWinRect.h>
#include <oGUI/Windows/oWinWindowing.h>

namespace ouro {
	namespace windows {
		namespace gdi {
		
BITMAPINFOHEADER make_header(const surface::info& _SurfaceInfo, bool _TopDown)
{
	switch (_SurfaceInfo.format)
	{
		case surface::format::b8g8r8a8_unorm:
		case surface::format::b8g8r8_unorm:
		case surface::format::r8_unorm:
			break;
		default:
			throw std::invalid_argument("unsupported format");
	}

	if (_SurfaceInfo.array_size != 0 || _SurfaceInfo.dimensions.z != 1)
		throw std::invalid_argument("no support for 3D or array surfaces");

	if (_SurfaceInfo.dimensions.x <= 0 || _SurfaceInfo.dimensions.y <= 0)
		throw std::invalid_argument("invalid dimensions");

	BITMAPINFOHEADER h;
	h.biSize = sizeof(BITMAPINFOHEADER);
	h.biWidth = _SurfaceInfo.dimensions.x;
	h.biHeight = _TopDown ? -_SurfaceInfo.dimensions.y : _SurfaceInfo.dimensions.y;
	h.biPlanes = 1; 
	h.biBitCount = static_cast<WORD>(surface::bits(_SurfaceInfo.format));
	h.biCompression = BI_RGB;
	h.biSizeImage = surface::element_size(_SurfaceInfo.format) * _SurfaceInfo.dimensions.y;
	h.biXPelsPerMeter = 0;
	h.biYPelsPerMeter = 0;
	h.biClrUsed = 0;
	h.biClrImportant = 0;
	return h;
}

BITMAPV4HEADER make_headerv4(const surface::info& _SurfaceInfo, bool _TopDown)
{
	BITMAPV4HEADER v4;
	BITMAPINFOHEADER v3 = make_header(_SurfaceInfo, _TopDown);
	memset(&v4, 0, sizeof(v4));
  memcpy(&v4, &v3, sizeof(v3));
	v4.bV4Size = sizeof(v4);
	v4.bV4RedMask = 0x00ff0000;
	v4.bV4GreenMask = 0x0000ff00;
	v4.bV4BlueMask = 0x000000ff;
	v4.bV4AlphaMask = 0xff000000;
	return v4;
}

void fill_monochrone_palette(RGBQUAD* _pColors, color _Color0, color _Color1)
{
	float4 c0;
	_Color0.decompose(&c0.x, &c0.x, &c0.z, &c0.w);

	float4 c1;
	_Color1.decompose(&c1.x, &c1.y, &c1.z, &c1.w);

	for (size_t i = 0; i < 256; i++)
	{
		float4 c = ::lerp(c0, c1, ubyte_to_unorm(i));
		RGBQUAD& q = _pColors[i];
		q.rgbRed = unorm_to_ubyte(c.x);
		q.rgbGreen = unorm_to_ubyte(c.y);
		q.rgbBlue = unorm_to_ubyte(c.z);
		q.rgbReserved = unorm_to_ubyte(c.w);
	}
}

surface::info get_info(const BITMAPINFOHEADER& _Header)
{
	surface::info si;
	si.dimensions.x = _Header.biWidth;
	si.dimensions.y = abs(_Header.biHeight); // ignore top v. bottom up here
	si.dimensions.z = 1;
	si.layout = surface::layout::image;
	si.array_size = 0;

	switch (_Header.biBitCount)
	{
		case 1: si.format = surface::r1_unorm; break;
		case 16: si.format = surface::b5g5r5a1_unorm; break;
		case 8: si.format = surface::format::r8_unorm; break;
		case 24: case 0: si.format = surface::format::b8g8r8_unorm; break;
		case 32: si.format = surface::format::b8g8r8a8_unorm; break;
		default: si.format = surface::format::unknown; break;
	}

	return si;
}

surface::info get_info(const BITMAPV4HEADER& _Header)
{
	return get_info((const BITMAPINFOHEADER&)_Header);
}

static size_t get_bitmapinfo_size(surface::format _Format)
{
	return surface::bits(_Format) == 8 ? (sizeof(BITMAPINFO) + sizeof(RGBQUAD) * 255) : sizeof(BITMAPINFO);
}

scoped_hbitmap make_bitmap(const surface::buffer* _pBuffer)
{
	ouro::surface::info si = _pBuffer->get_info();

	if (si.format != ouro::surface::b8g8r8a8_unorm)
		throw std::invalid_argument("only b8g8r8a8_unorm currently supported");

	surface::shared_lock lock(_pBuffer);
	return CreateBitmap(si.dimensions.x, si.dimensions.y, 1, 32, lock.mapped.data);
}

void memcpy2d(void* _pDestination, size_t _DestinationPitch, HBITMAP _hBmp, size_t _NumRows, bool _FlipVertically)
{
	scoped_getdc hDC(nullptr);

	struct BITMAPINFO_FULL
	{
		BITMAPINFOHEADER bmiHeader;
		RGBQUAD bmiColors[256];
	};

	BITMAPINFO_FULL bmif;
	memset(&bmif, 0, sizeof(bmif));
	bmif.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	GetDIBits(hDC, _hBmp, 0, 1, nullptr, (BITMAPINFO*)&bmif, DIB_RGB_COLORS);

	for (size_t y = 0; y < _NumRows; y++)
	{
		int nScanlinesRead = GetDIBits(hDC, _hBmp, static_cast<unsigned int>(_FlipVertically ? (_NumRows - 1 - y) : y), 1, byte_add(_pDestination, y * _DestinationPitch), (BITMAPINFO*)&bmif, DIB_RGB_COLORS);
		if (nScanlinesRead == ERROR_INVALID_PARAMETER)
			throw std::invalid_argument("invalid argument passed to GetDIBtis");
		else if (!nScanlinesRead)
			oTHROW(io_error, "GetDIBits failed");
	}
}

void draw_bitmap(HDC _hDC, int _X, int _Y, HBITMAP _hBitmap, DWORD _dwROP)
{
	BITMAP Bitmap;
	if (!_hDC || !_hBitmap)
		oTHROW_INVARG0();
	GetObject(_hBitmap, sizeof(BITMAP), (LPSTR)&Bitmap);
	scoped_compat hDCBitmap(_hDC);
	scoped_select sel(hDCBitmap, _hBitmap);
	oVB(BitBlt(_hDC, _X, _Y, Bitmap.bmWidth, Bitmap.bmHeight, hDCBitmap, 0, 0, _dwROP));
}

void stretch_bitmap(HDC _hDC, int _X, int _Y, int _Width, int _Height, HBITMAP _hBitmap, DWORD _dwROP)
{
	BITMAP Bitmap;
	if (!_hDC || !_hBitmap)
		oTHROW_INVARG0();
	GetObject(_hBitmap, sizeof(BITMAP), (LPSTR)&Bitmap);
	scoped_compat hDCBitmap(_hDC);
	scoped_select sel(hDCBitmap, _hBitmap);
	oVB(StretchBlt(_hDC, _X, _Y, _Width, _Height, hDCBitmap, 0, 0, Bitmap.bmWidth, Bitmap.bmHeight, _dwROP));
}

void stretch_blend_bitmap(HDC _hDC, int _X, int _Y, int _Width, int _Height, HBITMAP _hBitmap)
{
	static const BLENDFUNCTION kBlend = { AC_SRC_OVER, 0, 255, AC_SRC_ALPHA };
	BITMAP Bitmap;
	if (!_hDC || !_hBitmap)
		oTHROW_INVARG0();
	GetObject(_hBitmap, sizeof(BITMAP), (LPSTR)&Bitmap);
	scoped_compat hDCBitmap(_hDC);
	scoped_select sel(hDCBitmap, _hBitmap);
	oVB(AlphaBlend(_hDC, _X, _Y, _Width, _Height, hDCBitmap, 0, 0, Bitmap.bmWidth, Bitmap.bmHeight, kBlend));
}

void stretch_bits(HDC _hDC, const RECT& _DestRect, const int2& _SourceSize, surface::format _SourceFormat, const void* _pSourceBits, bool _FlipVertically)
{
	surface::info si;
	si.dimensions = int3(_SourceSize, 1);
	si.format = _SourceFormat;
	const size_t bitmapinfoSize = get_bitmapinfo_size(si.format);
	BITMAPINFO* pBMI = (BITMAPINFO*)_alloca(bitmapinfoSize);
	pBMI->bmiHeader = make_header(si, !_FlipVertically);
	if (bitmapinfoSize != sizeof(BITMAPINFO))
		fill_monochrone_palette(pBMI->bmiColors);
	scoped_blt_mode Mode(_hDC, HALFTONE);
	oVB(StretchDIBits(_hDC, _DestRect.left, _DestRect.top, oWinRectW(_DestRect), oWinRectH(_DestRect), 0, 0, _SourceSize.x, _SourceSize.y, _pSourceBits, pBMI, DIB_RGB_COLORS, SRCCOPY));
}

void stretch_bits(HWND _hWnd, const RECT& _DestRect, const int2& _SourceSize, surface::format _SourceFormat, const void* _pSourceBits, bool _FlipVertically)
{
	RECT destRect;
	if (_DestRect.bottom == invalid || _DestRect.top == invalid ||
		_DestRect.left == invalid || _DestRect.right == invalid)
	{
		RECT rClient;
		GetClientRect(_hWnd, &rClient);
		destRect = rClient;
	}
	else
		destRect = _DestRect;

	scoped_getdc hDC(_hWnd);
	stretch_bits(hDC, destRect, _SourceSize, _SourceFormat, _pSourceBits, _FlipVertically);
}

void stretch_bits(HWND _hWnd, const int2& _SourceSize, surface::format _SourceFormat, const void* _pSourceBits, bool _FlipVertically)
{
	RECT destRect;
	destRect.bottom = invalid;
	destRect.left = invalid;
	destRect.right = invalid;
	destRect.top = invalid;
	stretch_bits(_hWnd, destRect, _SourceSize, _SourceFormat, _pSourceBits, _FlipVertically);
}

int2 get_bitmap_dimensions(HDC _hDC)
{
	BITMAP BI;
	memset(&BI, 0, sizeof(BI));
	HGDIOBJ hBitmap = oGDIGetBitmap(_hDC);
	GetObject(hBitmap, sizeof(BITMAP), &BI);
	return int2(BI.bmWidth, BI.bmHeight);
}

		} // namespace gdi
	} // namespace windows
} // namespace ouro

using namespace ouro;
using namespace ouro::windows::gdi;

void oGDIScreenCaptureWindow(HWND _hWnd, const RECT* _pRect, void* _pImageBuffer, size_t _SizeofImageBuffer, BITMAPINFO* _pBitmapInfo, bool _RedrawWindow, bool _FlipV)
{
	RECT r;
	if (_pRect)
	{
		// Find offset into client area
		POINT p = {0,0};
		ClientToScreen(_hWnd, &p);

		RECT wr;
		oVB(GetWindowRect(_hWnd, &wr));
		p.x -= wr.left;
		p.y -= wr.top;

		r.left = _pRect->left + p.x;
		r.right = _pRect->right + p.x;
		r.top = _pRect->top + p.y;
		r.bottom = _pRect->bottom + p.y;
	}

	else
	{
		RECT wr;
		oVB(GetWindowRect(_hWnd, &wr));
		r.left = 0;
		r.top = 0;
		r.right = oWinRectW(wr);
		r.bottom = oWinRectH(wr);
	}

	int2 size = oWinRectSize(r);

	if (size.x == 0 || size.y == 0)
		throw std::invalid_argument("invalid size");

	WORD BitDepth = 0;
	{
		ouro::display::info di = ouro::display::get_info(oWinGetDisplayId(_hWnd));
		BitDepth = static_cast<WORD>(di.mode.depth);
		if (BitDepth == 32) BitDepth = 24;
	}

	if (!_pBitmapInfo)
		oTHROW_INVARG0();
	const int BytesPerPixel = BitDepth / 8;
	const int AlignedWidth = byte_align(size.x, 4);
	const int RowPitch = AlignedWidth * BytesPerPixel;

	memset(_pBitmapInfo, 0, sizeof(BITMAPINFO));
	_pBitmapInfo->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	_pBitmapInfo->bmiHeader.biWidth = size.x;
	_pBitmapInfo->bmiHeader.biHeight = size.y;
	_pBitmapInfo->bmiHeader.biPlanes = 1;
	_pBitmapInfo->bmiHeader.biBitCount = BitDepth;
	_pBitmapInfo->bmiHeader.biCompression = BI_RGB;
	_pBitmapInfo->bmiHeader.biSizeImage = RowPitch * size.y;

	if (_pImageBuffer)
	{
		if (_SizeofImageBuffer < _pBitmapInfo->bmiHeader.biSizeImage)
			oTHROW0(no_buffer_space);

		HDC hDC = GetWindowDC(_hWnd);
		HDC hMemDC = CreateCompatibleDC(hDC);

		HBITMAP hBMP = CreateCompatibleBitmap(hDC, size.x, size.y);
		HBITMAP hOld = (HBITMAP)SelectObject(hMemDC, hBMP);
		if (_RedrawWindow)
			oVB(RedrawWindow(_hWnd, 0, 0, RDW_INVALIDATE|RDW_UPDATENOW));

		BitBlt(hMemDC, 0, 0, size.x, size.y, hDC, r.left, r.top, SRCCOPY);

		const int PadSize = AlignedWidth - size.x;
		for (int y = 0; y < size.y; y++)
		{
			GetDIBits(hMemDC, hBMP, _FlipV ? (size.y - 1 - y) : y, 1, byte_add(_pImageBuffer, y * RowPitch), _pBitmapInfo, DIB_RGB_COLORS);
			char* pad = (char*)byte_add(_pImageBuffer, (y*RowPitch) + (size.x*BytesPerPixel));
			switch (PadSize)
			{
				// Duff's device
				case 3: pad[2] = 0;
				case 2: pad[1] = 0;
				case 1: pad[0] = 0;
				default: break;
			}
		}

		SelectObject(hMemDC, hOld);
		DeleteDC(hMemDC);
		ReleaseDC(0, hDC);
	}
}

void oGDIScreenCaptureWindow(HWND _hWnd, bool _IncludeBorder, std::function<void*(size_t _Size)> _Allocate, void** _ppBuffer, size_t* _pBufferSize, bool _RedrawWindow, bool _FlipV)
{
	if (!_Allocate || !_ppBuffer || !_pBufferSize)
		oTHROW_INVARG0();

	*_ppBuffer = nullptr;
	*_pBufferSize = 0;

	// Create a bmp in memory
	RECT r;
	GetClientRect(_hWnd, &r);
	BITMAPINFO bmi;

	RECT* pRect = &r;
	if (_IncludeBorder)
		pRect = nullptr;

	oGDIScreenCaptureWindow(_hWnd, pRect, nullptr, 0, &bmi, _RedrawWindow, _FlipV);
	BITMAPFILEHEADER bmfh;
	memset(&bmfh, 0, sizeof(bmfh));
	bmfh.bfType = 'MB';
	bmfh.bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFO) + bmi.bmiHeader.biSizeImage;
	bmfh.bfOffBits = static_cast<DWORD>(sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFO));

	*_pBufferSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFO) + bmi.bmiHeader.biSizeImage;
	*_ppBuffer = _Allocate(*_pBufferSize);
	memcpy(*_ppBuffer, &bmfh, sizeof(bmfh));
	memcpy(byte_add(*_ppBuffer, sizeof(bmfh)), &bmi, sizeof(bmi));
	oGDIScreenCaptureWindow(_hWnd, pRect, byte_add(*_ppBuffer, sizeof(bmfh) + sizeof(bmi)), bmi.bmiHeader.biSizeImage, &bmi, _RedrawWindow, _FlipV);
}

HBITMAP oGDIIconToBitmap(HICON _hIcon)
{
	HDC hDC = CreateCompatibleDC(nullptr);
	int2 Size = oGDIGetIconSize(_hIcon);
	HBITMAP hBitmap = CreateCompatibleBitmap(hDC, Size.x, Size.y);
	HBITMAP hOldBitmap = (HBITMAP)SelectObject(hDC, hBitmap);
	oVB(DrawIconEx(hDC, 0, 0, _hIcon, 0, 0, 0, nullptr, DI_NORMAL));
	SelectObject(hDC, hOldBitmap);
	DeleteDC(hDC);
	return hBitmap;
}

HICON oGDIBitmapToIcon(HBITMAP _hBmp)
{
	HICON hIcon = 0;
	BITMAPINFO bi = {0};
	bi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	if (GetDIBits(GetDC(0), _hBmp, 0, 0, 0, &bi, DIB_RGB_COLORS))
	{
		HBITMAP hMask = CreateCompatibleBitmap(GetDC(0), bi.bmiHeader.biWidth, bi.bmiHeader.biHeight);
		ICONINFO ii = {0};
		ii.fIcon = TRUE;
		ii.hbmColor = _hBmp;
		ii.hbmMask = hMask;
		hIcon = CreateIconIndirect(&ii);
		DeleteObject(hMask);
	}

	return hIcon;
}

bool oGDIDrawLine(HDC _hDC, const int2& _P0, const int2& _P1)
{
	oVB(MoveToEx(_hDC, _P0.x, _P0.y, nullptr));
	oVB(LineTo(_hDC, _P1.x, _P1.y));
	return true;
}

bool oGDIDrawBox(HDC _hDC, const RECT& _rBox, int _EdgeRoundness, float _Alpha)
{
	if (ouro::equal(_Alpha, 1.0f))
	{
		if (_EdgeRoundness)
			oVB(RoundRect(_hDC, _rBox.left, _rBox.top, _rBox.right, _rBox.bottom, _EdgeRoundness, _EdgeRoundness));
		else
			oVB(Rectangle(_hDC, _rBox.left, _rBox.top, _rBox.right, _rBox.bottom));
		return true;
	}

	// Copy the contents out (is there no way to access things directly?)
	BITMAPINFO bmi = {0};
	surface::info si;
	si.dimensions = int3(oWinRectW(_rBox), oWinRectH(_rBox), 1);
	si.format = surface::format::b8g8r8a8_unorm;

	bmi.bmiHeader = windows::gdi::make_header(si);
	if (!bmi.bmiHeader.biWidth || !bmi.bmiHeader.biHeight)
		return true;
	
	HDC hDCBitmap = CreateCompatibleDC(_hDC);
	scoped_hbitmap hBmp(CreateDIBSection(hDCBitmap, (BITMAPINFO*)&bmi, DIB_RGB_COLORS, nullptr, nullptr, 0));

	scoped_select ScopedSelectBmp(hDCBitmap, hBmp);
	scoped_select ScopedSelectPen(hDCBitmap, oGDIGetPen(_hDC));
	scoped_select ScopedSelectBrush(hDCBitmap, oGDIGetBrush(_hDC));

	// Resolve areas around the curved corners so we can to the blend directly
	// later on. But just do the minimum necessary, don't copy the interior that
	// we're going to re-draw with a RECT.
	if (_EdgeRoundness)
	{
		// Minimize rects...
		int dist = _EdgeRoundness;
		RECT TL = { _rBox.left, _rBox.top, _rBox.left + dist, _rBox.top + dist };
		RECT RT = { _rBox.right - dist, _rBox.top, _rBox.right, _rBox.top + dist };
		RECT BL = { _rBox.left, _rBox.bottom - dist, _rBox.left + dist, _rBox.bottom };
		RECT BR = { _rBox.right - dist, _rBox.bottom - dist, _rBox.right, _rBox.bottom };
		BitBlt(hDCBitmap, 0, 0, dist, dist, _hDC, _rBox.left, _rBox.top, SRCCOPY);
		BitBlt(hDCBitmap, bmi.bmiHeader.biWidth - dist, 0, dist, dist, _hDC, _rBox.right - dist, _rBox.top, SRCCOPY);
		BitBlt(hDCBitmap, 0, bmi.bmiHeader.biHeight - dist, dist, dist, _hDC, _rBox.left, _rBox.bottom - dist, SRCCOPY);
		BitBlt(hDCBitmap, bmi.bmiHeader.biWidth - dist, bmi.bmiHeader.biHeight - dist, dist, dist, _hDC, _rBox.right - dist, _rBox.bottom - dist, SRCCOPY);
		oVB(RoundRect(hDCBitmap, 0, 0, bmi.bmiHeader.biWidth, bmi.bmiHeader.biHeight, _EdgeRoundness, _EdgeRoundness));
	}

	else
		oVB(Rectangle(hDCBitmap, 0, 0, bmi.bmiHeader.biWidth, bmi.bmiHeader.biHeight));

	BLENDFUNCTION blend = { AC_SRC_OVER, 0, (BYTE)(static_cast<int>(_Alpha * 255.0f) & 0xff), 0 };
	oVB(AlphaBlend(_hDC, _rBox.left, _rBox.top, bmi.bmiHeader.biWidth, bmi.bmiHeader.biHeight, hDCBitmap, 0, 0, bmi.bmiHeader.biWidth, bmi.bmiHeader.biHeight, blend));
	return true;
}

bool oGDIDrawEllipse(HDC _hDC, const RECT& _rBox)
{
	oVB(Ellipse(_hDC, _rBox.left, _rBox.top, _rBox.right, _rBox.bottom));
	return true;
}

RECT oGDICalcTextRect(HDC _hDC, const char* _Text)
{
	RECT r = {0,0,0,0};
	oVB(DrawText(_hDC, _Text, -1, &r, DT_CALCRECT|DT_LEFT));
	return r;
}

static bool oGDIDrawText(HDC _hDC, const ouro::text_info& _Desc, const char* _Text, RECT* _pActual)
{
	int r,g,b,a;
	_Desc.foreground.decompose(&r, &g, &b, &a);

	int br,bg,bb,ba;
	_Desc.background.decompose(&br, &bg, &bb, &ba);

	int sr,sg,sb,sa;
	_Desc.shadow.decompose(&sr, &sg, &sb, &sa);

	if (!a)
	{
		if (!ba) return false;
		r = br;
		g = bg;
		b = bb;
	}

	UINT uFormat = DT_WORDBREAK;
	switch (_Desc.alignment % 3)
	{
		case 0: uFormat |= DT_LEFT; break;
		case 1: uFormat |= DT_CENTER; break;
		case 2: uFormat |= DT_RIGHT; break;
		oNODEFAULT;
	}

	switch (_Desc.alignment / 3)
	{
		case 0: uFormat |= DT_TOP; break;
		case 1: uFormat |= DT_VCENTER; break;
		case 2: uFormat |= DT_BOTTOM; break;
		oNODEFAULT;
	}

	bool forcedSingleLine = !_Desc.single_line && ((uFormat & DT_BOTTOM) || (uFormat & DT_VCENTER));
	if (forcedSingleLine || _Desc.single_line)
	{
		if (forcedSingleLine)
			oTRACE_ONCE("GDI doesn't support multi-line, vertically aligned text. See DrawText docs for more details. http://msdn.microsoft.com/en-us/library/ms901121.aspx");
		uFormat &=~ DT_WORDBREAK;
		uFormat |= DT_SINGLELINE;
	}

	if (_pActual)
		uFormat |= DT_CALCRECT;

	RECT rect = oWinRectWH(_Desc.position, _Desc.size);
	if (!_pActual)
		_pActual = &rect;
	else
	{
		_pActual->top = 0;
		_pActual->left = 0;
		_pActual->right = 1;
		_pActual->bottom = 1;
	}

	if (sa && any(_Desc.shadow_offset != int2(0,0)))
	{
		// If the background is opaque, cast an opaque shadow
		scoped_text_color ShadowState(_hDC, RGB(sr,sg,sb), RGB(sr,sg,sb), ba);
		RECT rShadow = oWinRectTranslate(*_pActual, _Desc.shadow_offset);
		DrawText(_hDC, _Text, -1, &rShadow, uFormat);
	}
	
	scoped_text_color TextState(_hDC, RGB(r,g,b), RGB(br,bg,bb), ba);
	DrawText(_hDC, _Text, -1, _pActual, uFormat);
	return true;
}

bool oGDICalcTextBox(HDC _hDC, const ouro::text_info& _Desc, const char* _Text)
{
	RECT r = oWinRectWH(_Desc.position, _Desc.size);
	return oGDIDrawText(_hDC, _Desc, _Text, &r);
}

bool oGDIDrawText(HDC _hDC, const ouro::text_info& _Desc, const char* _Text)
{
	return oGDIDrawText(_hDC, _Desc, _Text, nullptr);
}

HPEN oGDICreatePen(color _Color, int _Width)
{
	int r,g,b,a;
	_Color.decompose(&r, &g, &b, &a);
	return CreatePen(a ? PS_SOLID : PS_NULL, _Width, RGB(r,g,b));
}

HBRUSH oGDICreateBrush(color _Color)
{
	int r,g,b,a;
	_Color.decompose(&r, &g, &b, &a);
	return a ? CreateSolidBrush(RGB(r,g,b)) : (HBRUSH)GetStockObject(HOLLOW_BRUSH);
}

COLORREF oGDIGetPenColor(HPEN _hPen, int* _pWidth)
{
	LOGPEN lp;
	if (!GetObject(_hPen, sizeof(LOGPEN), &lp))
	{
		EXTLOGPEN elp;
		GetObject(_hPen, sizeof(EXTLOGPEN), &elp);
		if (_pWidth)
			*_pWidth = static_cast<int>(elp.elpWidth);
		return elp.elpColor;
	}

	if (_pWidth)
		*_pWidth = static_cast<int>(lp.lopnWidth.x);
	return lp.lopnColor;
}

COLORREF oGDIGetBrushColor(HBRUSH _hBrush)
{
	LOGBRUSH lb;
	GetObject(_hBrush, sizeof(LOGBRUSH), &lb);
	return lb.lbColor;
}

int2 oGDIGetIconSize(HICON _hIcon)
{
	ICONINFO ii;
	BITMAP b;
	if (GetIconInfo(_hIcon, &ii))
	{
		if (ii.hbmColor)
		{
			if (GetObject(ii.hbmColor, sizeof(b), &b))
				return int2(b.bmWidth, b.bmHeight);
		}

		else
		{
			if (GetObject(ii.hbmMask, sizeof(b), &b))
				return int2(b.bmWidth, b.bmHeight);
		}
	}

	return int2(-1,-1);
}

HFONT oGDICreateFont(const ouro::font_info& _Desc)
{
	scoped_getdc hDC(GetDesktopWindow());
	HFONT hFont = CreateFont(
		point_to_logical_heightf(hDC, _Desc.point_size)
		, 0
		, 0
		, 0
		, _Desc.bold ? FW_BOLD : FW_NORMAL
		, _Desc.italic
		, _Desc.underline
		, _Desc.strikeout
		, DEFAULT_CHARSET
		, OUT_DEFAULT_PRECIS
		, CLIP_DEFAULT_PRECIS
		, _Desc.antialiased ? CLEARTYPE_QUALITY : NONANTIALIASED_QUALITY
		, DEFAULT_PITCH
		, _Desc.name);
	return hFont;
}

void oGDIGetFontDesc(HFONT _hFont, ouro::font_info* _pDesc)
{
	LOGFONT lf = {0};
	::GetObject(_hFont, sizeof(lf), &lf);
	_pDesc->name = lf.lfFaceName;
	_pDesc->bold = lf.lfWeight > FW_NORMAL;
	_pDesc->italic = !!lf.lfItalic;
	_pDesc->underline = !!lf.lfUnderline;
	_pDesc->strikeout = !!lf.lfStrikeOut;
	_pDesc->antialiased = lf.lfQuality != NONANTIALIASED_QUALITY;
	scoped_getdc hDC(GetDesktopWindow());
	_pDesc->point_size = logical_height_to_pointf(hDC, lf.lfHeight);
}

const char* oGDIGetFontFamily(BYTE _tmPitchAndFamily)
{
	switch (_tmPitchAndFamily & 0xf0)
	{
		case FF_DECORATIVE: return "Decorative";
		case FF_DONTCARE: return "Don't care";
		case FF_MODERN: return "Modern";
		case FF_ROMAN: return "Roman";
		case FF_SCRIPT: return "Script";
		case FF_SWISS: return "Swiss";
		default: break;
	}

	return "Unknown";
}

const char* oGDIGetCharSet(BYTE _tmCharSet)
{
	switch (_tmCharSet)
	{
		case ANSI_CHARSET: return "ANSI";
		case BALTIC_CHARSET: return "Baltic";
		case CHINESEBIG5_CHARSET: return "Chinese Big5";
		case DEFAULT_CHARSET: return "Default";
		case EASTEUROPE_CHARSET: return "East Europe";
		case GB2312_CHARSET: return "GB2312";
		case GREEK_CHARSET: return "Greek";
		case HANGUL_CHARSET: return "Hangul";
		case MAC_CHARSET: return "Mac";
		case OEM_CHARSET: return "OEM";
		case RUSSIAN_CHARSET: return "Russian";
		case SHIFTJIS_CHARSET: return "Shift JIS";
		case SYMBOL_CHARSET: return "Symbol";
		case TURKISH_CHARSET: return "Turkish";
		case VIETNAMESE_CHARSET: return "Vietnamese";
		case JOHAB_CHARSET: return "Johab";
		case ARABIC_CHARSET: return "Arabic";
		case HEBREW_CHARSET: return "Hebrew";
		case THAI_CHARSET: return "Thai";
		default: break;
	}

	return "Unknown";
}

int oGDIEstimatePointSize(int _PixelHeight)
{
	// http://reeddesign.co.uk/test/points-pixels.html
	switch (_PixelHeight)
	{
		case 8: return 6;
		case 9: return 7;
		case 11: return 8;
		case 12: return 9;
		case 13: return 10;
		case 15: return 11;
		case 16: return 12;
		case 17: return 13;
		case 19: return 14;
		case 21: return 15;
		case 22: return 16;
		case 23: return 17;
		case 24: return 18;
		case 26: return 20;
		case 29: return 22;
		case 32: return 24;
		case 35: return 26;
		case 36: return 27;
		case 37: return 28;
		case 38: return 29;
		case 40: return 30;
		case 42: return 32;
		case 45: return 34;
		case 48: return 36;
		default: break;
	}

	if (_PixelHeight < 8)
		return 4;
	
	return static_cast<int>((_PixelHeight * 36.0f / 48.0f) + 0.5f);
}

selection_box::selection_box()
	: MouseDownAt(0,0)
	, MouseAt(0,0)
	, Opacity(0.0f)
	, Selecting(false)
	, hNullBrush(oGDICreateBrush(ouro::color(0)))
{
}

const selection_box& selection_box::operator=(selection_box&& _That)
{
	if (this != &_That)
	{
		MouseDownAt = std::move(_That.MouseDownAt);
		MouseAt = std::move(_That.MouseAt);
		Opacity = std::move(_That.Opacity);
		Selecting = std::move(_That.Selecting);
		hPen = std::move(_That.hPen);
		hBrush = std::move(_That.hBrush);
		hNullBrush = std::move(_That.hNullBrush);
		hOffscreenBMP = std::move(_That.hOffscreenBMP);
		Desc = std::move(_That.Desc);
	}
	return *this;
}

void selection_box::SetDesc(const DESC& _Desc)
{
	oASSERT(_Desc.hParent, "A valid HWND for a parent window must be specified");

	if (Desc.hParent != _Desc.hParent)
	{
		Selecting = false;
		hOffscreenBMP = nullptr;
	}

	hPen = oGDICreatePen(_Desc.Border);
	hBrush = oGDICreateBrush(_Desc.Fill);
	float r,g,b;
	_Desc.Fill.decompose(&r, &g, &b, &Opacity);

	Desc = _Desc;
}

void selection_box::GetDesc(DESC* _pDesc)
{
	*_pDesc = Desc;
}

bool selection_box::IsSelecting() const
{
	return Selecting;	
}

void selection_box::Draw(HDC _hDC)
{
	if (!Selecting)
		return;

	const RECT SelRect = oWinRect(MouseDownAt, MouseAt);
	const int2 SelSize = oWinRectSize(SelRect);

	// by default, don't use an offscreen buffer. Rounded edge rect drawing can
	// crash Windows if rendering to an HDC from a DXGI back-buffer, so that's why
	// we go off-screen here.
	HDC hResolveDC = _hDC, hTargetDC = _hDC;
	
	if (Desc.EdgeRoundness && Desc.UseOffscreenRender && !hOffscreenBMP)
	{
		RECT rParent;
		GetClientRect(Desc.hParent, &rParent);
		hTargetDC = CreateCompatibleDC(_hDC);
		hOffscreenBMP = CreateCompatibleBitmap(hTargetDC, oWinRectW(rParent), oWinRectH(rParent));
		SelectObject(hTargetDC, hOffscreenBMP);
	}

	if (Desc.EdgeRoundness && Desc.UseOffscreenRender)
		BitBlt(hTargetDC, SelRect.left, SelRect.top, SelSize.x, SelSize.y, hResolveDC, SelRect.left, SelRect.top, SRCCOPY);

	{
		scoped_select B(hTargetDC, hBrush);
		oGDIDrawBox(hTargetDC, SelRect, Desc.EdgeRoundness, Opacity);
	}
	
	{
		scoped_select P(hTargetDC, hPen);
		scoped_select B(hTargetDC, hNullBrush);
		oGDIDrawBox(hTargetDC, SelRect, Desc.EdgeRoundness, 1.0f);
	}

	if (Desc.EdgeRoundness && Desc.UseOffscreenRender)
		BitBlt(hResolveDC, SelRect.left, SelRect.top, SelSize.x, SelSize.y, hTargetDC, SelRect.left, SelRect.top, SRCCOPY);
}

void selection_box::OnResize(const int2& _NewParentSize)
{
	// pick up realloc in Draw()
	hOffscreenBMP = nullptr;
}

void selection_box::OnMouseDown(const int2& _MousePosition)
{
	MouseDownAt = MouseAt = _MousePosition;
	Selecting = true;
	SetCapture(Desc.hParent);
}

void selection_box::OnMouseMove(const int2& _MousePosition)
{
	MouseAt = _MousePosition;
}

void selection_box::OnMouseUp()
{
	Selecting = false;
	ReleaseCapture();
}

