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
#include <oGUI/Windows/win_gdi_bitmap.h>
#include <oCore/windows/win_error.h>

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

	for (unsigned char i = 0; i < 256; i++)
	{
		float4 c = ::lerp(c0, c1, n8tof32(i));
		RGBQUAD& q = _pColors[i];
		q.rgbRed = f32ton8(c.x);
		q.rgbGreen = f32ton8(c.y);
		q.rgbBlue = f32ton8(c.z);
		q.rgbReserved = f32ton8(c.w);
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

scoped_bitmap make_bitmap(const surface::buffer* _pBuffer)
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

int2 bitmap_dimensions(HDC _hDC)
{
	BITMAP BI;
	memset(&BI, 0, sizeof(BI));
	GetObject(current_bitmap(_hDC), sizeof(BITMAP), &BI);
	return int2(BI.bmWidth, BI.bmHeight);
}

int2 icon_dimensions(HICON _hIcon)
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

	return int2(invalid,invalid);
}

HBITMAP bitmap_from_icon(HICON _hIcon)
{
	scoped_compat hDC(nullptr);
	int2 Size = icon_dimensions(_hIcon);
	HBITMAP hBitmap = CreateCompatibleBitmap(hDC, Size.x, Size.y);
	scoped_select sel(hDC, hBitmap);
	oVB(DrawIconEx(hDC, 0, 0, _hIcon, 0, 0, 0, nullptr, DI_NORMAL));
	return hBitmap;
}

HICON bitmap_to_icon(HBITMAP _hBmp)
{
	HICON hIcon = nullptr;
	BITMAPINFO bi = {0};
	bi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	if (GetDIBits(GetDC(0), _hBmp, 0, 0, 0, &bi, DIB_RGB_COLORS))
	{
		scoped_bitmap hMask(CreateCompatibleBitmap(GetDC(0), bi.bmiHeader.biWidth, bi.bmiHeader.biHeight));
		ICONINFO ii = {0};
		ii.fIcon = TRUE;
		ii.hbmColor = _hBmp;
		ii.hbmMask = hMask;
		hIcon = CreateIconIndirect(&ii);
		DeleteObject(hMask);
	}

	return hIcon;
}

		} // namespace gdi
	} // namespace windows
} // namespace ouro
