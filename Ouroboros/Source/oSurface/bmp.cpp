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
		return info();

	const BITMAPFILEHEADER* bfh = (const BITMAPFILEHEADER*)_pBuffer;
	const BITMAPINFO* bmi = (const BITMAPINFO*)&bfh[1];

	info si;
	si.format = bmi->bmiHeader.biBitCount == 32 ? b8g8r8a8_unorm : b8g8r8_unorm;
	si.dimensions = int3(bmi->bmiHeader.biWidth, bmi->bmiHeader.biHeight, 1);
	return si;
}

std::shared_ptr<char> encode_bmp(const buffer* _pBuffer
	, size_t* _pSize
	, const alpha_option::value& _Option
	, const compression::value& _Compression)
{
	auto info = _pBuffer->get_info();

	oCHECK(info.format == surface::b8g8r8a8_unorm || info.format == surface::b8g8r8_unorm, "source must be b8g8r8a8_unorm or b8g8r8_unorm");

	const uint ElementSize = surface::element_size(info.format);
	const uint UnalignedPitch = ElementSize * info.dimensions.x;
	const uint AlignedPitch = byte_align(UnalignedPitch, 4);
	const uint BufferSize = AlignedPitch * info.dimensions.y;

	const uint bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFO);
	const uint bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFO) + BufferSize;

	void* p = malloc(bfSize);
	std::shared_ptr<char> buffer((char*)p, free);
	if (_pSize)
		*_pSize = bfSize;

	BITMAPFILEHEADER* bfh = (BITMAPFILEHEADER*)p;
	BITMAPINFO* bmi = (BITMAPINFO*)&bfh[1];
	void* bits = &bmi[1];

	bfh->bfType = 0x4d42; // 'BM'
	bfh->bfReserved1 = 0;
	bfh->bfReserved2 = 0;
	bfh->bfSize = bfSize;
	bfh->bfOffBits = bfOffBits;
	
	bmi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmi->bmiHeader.biWidth = info.dimensions.x;
	bmi->bmiHeader.biHeight = info.dimensions.y;
	bmi->bmiHeader.biPlanes = 1;
	bmi->bmiHeader.biBitCount = info.format == surface::b8g8r8a8_unorm ? 32 : 24;
	bmi->bmiHeader.biCompression = 0; // BI_RGB
	bmi->bmiHeader.biSizeImage = BufferSize;
	bmi->bmiHeader.biXPelsPerMeter = 0x0ec4;
	bmi->bmiHeader.biYPelsPerMeter = 0x0ec4;
	bmi->bmiHeader.biClrUsed = 0;
	bmi->bmiHeader.biClrImportant = 0;
	bmi->bmiColors[0].rgbBlue = 0;
	bmi->bmiColors[0].rgbGreen = 0;
	bmi->bmiColors[0].rgbRed = 0;
	bmi->bmiColors[0].rgbReserved = 0;

	shared_lock lock(*_pBuffer);
	
	uint Padding = AlignedPitch - UnalignedPitch;

	for (int y = 0, y1 = info.dimensions.y-1; y < info.dimensions.y; y++, y1--)
	{
		uchar* scanline = (uchar*)byte_add(bits, y * AlignedPitch);
		const uchar* src = (const uchar*)byte_add(lock.mapped.data, y1 * lock.mapped.row_pitch);
		for (uint x = 0; x < UnalignedPitch; x += ElementSize)
		{
			*scanline++ = src[0];
			*scanline++ = src[1];
			*scanline++ = src[2];
			if (bmi->bmiHeader.biBitCount == 32)
				*scanline++ = src[3];
			src += (bmi->bmiHeader.biBitCount >> 3);
		}

		for (uint x = UnalignedPitch; x < AlignedPitch; x++)
			*scanline = 0;
	}

	return buffer;
}

std::shared_ptr<buffer> decode_bmp(const void* _pBuffer, size_t _BufferSize, const alpha_option::value& _Option, const layout& _Layout)
{
	const BITMAPFILEHEADER* bfh = (const BITMAPFILEHEADER*)_pBuffer;
	const BITMAPINFO* bmi = (const BITMAPINFO*)&bfh[1];
	const void* bits = &bmi[1];

	info si = get_info_bmp(_pBuffer, _BufferSize);
	oCHECK(si.format != unknown, "invalid bmp");
	info dsi = si;
	dsi.layout = _Layout;
	const_mapped_subresource msr;
	msr.data = bits;
	msr.depth_pitch = bmi->bmiHeader.biSizeImage;
	msr.row_pitch = msr.depth_pitch / bmi->bmiHeader.biHeight;

	dsi.format = alpha_option_format(si.format, _Option);

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
