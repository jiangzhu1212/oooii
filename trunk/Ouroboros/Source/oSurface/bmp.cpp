/**************************************************************************
 * The MIT License                                                        *
 * Copyright (c) 2014 Antony Arciuolo.                                    *
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

#include "bmp.h"

namespace ouro { namespace surface {

bool is_bmp(const void* buffer, size_t size)
{
	auto h = (const bmp_header*)buffer;
	return size >= sizeof(bmp_header) && h->bfType == bmp_signature;
}

info get_info_bmp(const void* buffer, size_t size)
{
	if (!is_bmp(buffer, size))
		return info();

	auto h = (const bmp_header*)buffer;
	auto bmi = (const bmp_info*)&h[1];

	if (bmi->bmiHeader.biBitCount != 32 && bmi->bmiHeader.biBitCount != 24)
		return info();

	info si;
	si.format = bmi->bmiHeader.biBitCount == 32 ? format::b8g8r8a8_unorm : format::b8g8r8_unorm;
	si.dimensions = int3(bmi->bmiHeader.biWidth, bmi->bmiHeader.biHeight, 1);
	return si;
}

format required_input_bmp(const format& stored)
{
	if (num_channels(stored) == 4)
		return format::b8g8r8a8_unorm;
	else if (num_channels(stored) == 3)
		return format::b8g8r8_unorm;
	return format::unknown;
}

scoped_allocation encode_bmp(const image& img, const allocator& file_alloc, const allocator& temp_alloc, const compression& compression)
{
	auto info = img.get_info();
	oCHECK(info.format == surface::format::b8g8r8a8_unorm || info.format == surface::format::b8g8r8_unorm, "source must be b8g8r8a8_unorm or b8g8r8_unorm");
	const uint src_byte_count = element_size(info.format);

	const uint ElementSize = surface::element_size(info.format);
	const uint UnalignedPitch = ElementSize * info.dimensions.x;
	const uint AlignedPitch = byte_align(UnalignedPitch, 4);
	const uint BufferSize = AlignedPitch * info.dimensions.y;

	const uint bfOffBits = sizeof(bmp_header) + sizeof(bmp_infoheader);
	const uint bfSize = bfOffBits + BufferSize;
	const bool kIs32Bit = info.format == surface::format::b8g8r8a8_unorm;

	scoped_allocation p(file_alloc.allocate(bfSize, 0), bfSize, file_alloc.deallocate);

	auto bfh = (bmp_header*)p;
	auto bmi = (bmp_info*)&bfh[1];
	void* bits = byte_add((void*)p, bfOffBits);

	bfh->bfType = bmp_signature;
	bfh->bfSize = bfSize;
	bfh->bfReserved1 = 0;
	bfh->bfReserved2 = 0;
	bfh->bfOffBits = bfOffBits;
	
	bmi->bmiHeader.biSize = sizeof(bmp_infoheader);
	bmi->bmiHeader.biWidth = info.dimensions.x;
	bmi->bmiHeader.biHeight = info.dimensions.y;
	bmi->bmiHeader.biPlanes = 1;
	bmi->bmiHeader.biBitCount = kIs32Bit ? 32 : 24;
	bmi->bmiHeader.biCompression = bmp_compression::rgb;
	bmi->bmiHeader.biSizeImage = BufferSize;
	bmi->bmiHeader.biXPelsPerMeter = 0x0ec4;
	bmi->bmiHeader.biYPelsPerMeter = 0x0ec4;
	bmi->bmiHeader.biClrUsed = 0;
	bmi->bmiHeader.biClrImportant = 0;

	shared_lock lock(img);
	
	uint Padding = AlignedPitch - UnalignedPitch;
	for (uint y = 0, y1 = info.dimensions.y-1; y < info.dimensions.y; y++, y1--)
	{
		uchar* scanline = (uchar*)byte_add(bits, y * AlignedPitch);
		const uchar* src = (const uchar*)byte_add(lock.mapped.data, y1 * lock.mapped.row_pitch);
		for (uint x = 0; x < UnalignedPitch; x += ElementSize)
		{
			*scanline++ = src[0];
			*scanline++ = src[1];
			*scanline++ = src[2];
			if (kIs32Bit)
				*scanline++ = src[3];
			src += src_byte_count;
		}

		for (uint x = UnalignedPitch; x < AlignedPitch; x++)
			*scanline++ = 0;
	}

	return p;
}

image decode_bmp(const void* buffer, size_t size, const allocator& texel_alloc, const allocator& temp_alloc, const mip_layout& layout)
{
	const bmp_header* bfh = (const bmp_header*)buffer;
	const bmp_info* bmi = (const bmp_info*)&bfh[1];
	const void* bits = byte_add(buffer, bfh->bfOffBits);

	info si = get_info_bmp(buffer, size);
	oCHECK(si.format != format::unknown, "invalid bmp");
	info dsi = si;
	dsi.mip_layout = layout;
	const_mapped_subresource src;
	src.data = bits;
	src.depth_pitch = bmi->bmiHeader.biSizeImage;
	src.row_pitch = src.depth_pitch / bmi->bmiHeader.biHeight;

	image img(dsi, texel_alloc);
	img.copy_from(0, src, copy_option::flip_vertically);
	return img;
}

}}
