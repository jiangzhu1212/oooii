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
#include <oSurface/codec.h>
#include <oSurface/convert.h>

namespace ouro {
	namespace surface {

struct RGBQUAD
{
	unsigned char rgbBlue;
	unsigned char rgbGreen;
	unsigned char rgbRed;
	unsigned char rgbReserved;
};

struct BITMAPINFOHEADER
{
	unsigned int biSize;
	long biWidth;
	long biHeight;
	unsigned short biPlanes;
	unsigned short biBitCount;
	unsigned int biCompression;
	unsigned int biSizeImage;
	long biXPelsPerMeter;
	long biYPelsPerMeter;
	unsigned int biClrUsed;
	unsigned int biClrImportant;
};

#pragma pack(push,2)
struct BITMAPFILEHEADER
{
	unsigned short bfType;
	unsigned int bfSize;
	unsigned short bfReserved1;
	unsigned short bfReserved2;
	unsigned int bfOffBits;
};
#pragma pack(pop)

struct BITMAPINFO
{
	BITMAPINFOHEADER bmiHeader;
	RGBQUAD bmiColors[1];
};

info get_info_bmp(const void* _pBuffer, size_t _BufferSize)
{
	if (_BufferSize < 2 || memcmp(_pBuffer, "BM", 2))
		throw std::invalid_argument("the buffer is not a bmp");

	const BITMAPFILEHEADER* bfh = (const BITMAPFILEHEADER*)_pBuffer;
	const BITMAPINFO* bmi = (const BITMAPINFO*)&bfh[1];

	info si;
	si.format = bmi->bmiHeader.biBitCount == 32 ? b8g8r8a8_unorm : b8g8r8_unorm;
	si.dimensions = int3(bmi->bmiHeader.biWidth, bmi->bmiHeader.biHeight, 1);
	return si;
}

std::shared_ptr<char> encode_bmp(const buffer* _pBuffer
	, size_t* _pSize
	, alpha_option::value _Option
	, compression::value _Compression)
{
	throw std::exception("bmp encode unsupported");
}

std::shared_ptr<buffer> decode_bmp(const void* _pBuffer, size_t _BufferSize, alpha_option::value _Option)
{
	if (_BufferSize < 2 || memcmp(_pBuffer, "BM", 2))
		throw std::invalid_argument("the buffer is not a bmp");

	const BITMAPFILEHEADER* bfh = (const BITMAPFILEHEADER*)_pBuffer;
	const BITMAPINFO* bmi = (const BITMAPINFO*)&bfh[1];
	const void* bits = &bmi[1];

	info si = get_info_bmp(_pBuffer, _BufferSize);
	info dsi = si;
	const_mapped_subresource msr;
	msr.data = bits;
	msr.depth_pitch = bmi->bmiHeader.biSizeImage;
	msr.row_pitch = msr.depth_pitch / bmi->bmiHeader.biHeight;

	switch (_Option)
	{
		case alpha_option::force_alpha:
			if (si.format == b8g8r8_unorm)
				dsi.format = b8g8r8a8_unorm;
			break;
		case alpha_option::force_no_alpha:
			if (si.format == b8g8r8a8_unorm)
				dsi.format = b8g8r8_unorm;
			break;
		default:
			break;
	}

	std::shared_ptr<buffer> s = buffer::make(dsi);
	subresource_info sri = subresource(si, 0);
	{
		lock_guard lock(s);
		convert_subresource(sri, msr, dsi.format, &lock.mapped, true);
	}

	return s;
}

	} // namespace surface
} // namespace ouro
