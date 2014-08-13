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
#include <oBase/throw.h>

#include "tga.h"

namespace ouro { namespace surface {

bool is_tga(const void* buffer, size_t size)
{
	auto h = (const tga_header*)buffer;
	return !h->id_length && !h->paletted_type && tga_is_valid_dtf(h->data_type_field)
		&& !h->paletted_origin && !h->paletted_length && !h->paletted_depth
		&& h->width >= 1 && h->height >= 1 && !h->image_descriptor
		&& (h->bpp == 32 || h->bpp == 24);
}

format required_input_tga(const format& stored)
{
	if (num_channels(stored) == 4)
		return format::b8g8r8a8_unorm;
	else if (num_channels(stored) == 3)
		return format::b8g8r8_unorm;
	return format::unknown;
}

info get_info_tga(const void* buffer, size_t size)
{
	if (!is_tga(buffer, size))
		return info();

	auto h = (const tga_header*)buffer;
	info si;
	si.format = h->bpp == 32 ? format::b8g8r8a8_unorm : format::b8g8r8_unorm;
	si.dimensions = int3(h->width, h->height, 1);
	return si;
}

scoped_allocation encode_tga(const texel_buffer& b, const compression& compression)
{
	oCHECK_ARG(compression == compression::none, "compression not supported");

	auto info = b.get_info();
	oCHECK_ARG(info.format == format::b8g8r8a8_unorm || info.format == format::b8g8r8_unorm, "source must be b8g8r8a8_unorm or b8g8r8_unorm");
	oCHECK_ARG(info.dimensions.x <= 0xffff && info.dimensions.y <= 0xffff, "dimensions must be <= 65535");

	tga_header h = {0};
	h.data_type_field = tga_data_type_field::rgb;
	h.bpp = (uchar)bits(info.format);
	h.width = (ushort)info.dimensions.x;
	h.height = (ushort)info.dimensions.y;

	const size_t size = sizeof(tga_header) + h.width * h.height * (h.bpp/8);

	scoped_allocation p(malloc(size), size, free);
	memcpy(p, &h, sizeof(tga_header));
	mapped_subresource dst;
	dst.data = byte_add((void*)p, sizeof(tga_header));
	dst.row_pitch = element_size(info.format) * h.width;
	dst.depth_pitch = dst.row_pitch * h.height;

	b.copy_to(0, dst, copy_option::flip_vertically);
	return p;
}

texel_buffer decode_tga(const void* buffer, size_t size, const mip_layout& layout)
{
	info si = get_info_tga(buffer, size);
	oCHECK(si.format != format::unknown, "invalid tga");
	const tga_header* h = (const tga_header*)buffer;
	info dsi = si;
	dsi.mip_layout = layout;
	auto src = get_const_mapped_subresource(si, 0, 0, &h[1]);

	texel_buffer b(dsi);
	b.copy_from(0, src, copy_option::flip_vertically);
	return b;
}

}}
