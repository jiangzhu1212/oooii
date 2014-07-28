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

namespace ouro { namespace surface {

#define TGA_RGB 2

#pragma pack(push,1)
struct TGAHeader
{
	unsigned char id_length;
	unsigned char paletted_type;
	unsigned char data_type_field;
	unsigned short paletted_origin;
	unsigned short paletted_length;
	unsigned char paletted_depth;
	unsigned short x_origin;
	unsigned short y_origin;
	unsigned short width;
	unsigned short height;
	unsigned char bpp;
	unsigned char image_descriptor;
};
#pragma pack(pop)

info get_info_tga(const void* buffer, size_t size)
{
	const TGAHeader* h = (const TGAHeader*)buffer;

	if (h->id_length || h->paletted_type || h->data_type_field != TGA_RGB 
		|| h->paletted_origin || h->paletted_length || h->paletted_depth || h->x_origin || h->y_origin
		|| h->width < 1 || h->height < 1 || h->image_descriptor
		|| !(h->bpp == 32 || h->bpp == 24))
		return info();

	info si;
	si.format = h->bpp == 32 ? format::b8g8r8a8_unorm : format::b8g8r8_unorm;
	si.dimensions = int3(h->width, h->height, 1);
	return si;
}

scoped_allocation encode_tga(const texel_buffer& b
	, const alpha_option& option
	, const compression& compression)
{
	oCHECK_ARG(compression == compression::none, "compression not supported");

	auto info = b.get_info();
	oCHECK_ARG(info.format == format::b8g8r8a8_unorm || info.format == format::b8g8r8_unorm, "source must be b8g8r8a8_unorm or b8g8r8_unorm");
	oCHECK_ARG(info.dimensions.x <= 0xffff && info.dimensions.y <= 0xffff, "dimensions must be <= 65535");

	auto dstinfo = info;
	dstinfo.format = alpha_option_format(info.format, option);

	TGAHeader h = {0};
	h.data_type_field = TGA_RGB;
	h.bpp = (uchar)bits(dstinfo.format);
	h.width = (ushort)info.dimensions.x;
	h.height = (ushort)info.dimensions.y;

	const size_t size = sizeof(TGAHeader) + h.width * h.height * (h.bpp/8);

	scoped_allocation p(malloc(size), size, free);
	memcpy(p, &h, sizeof(TGAHeader));
	mapped_subresource dst;
	dst.data = byte_add((void*)p, sizeof(TGAHeader));
	dst.row_pitch = element_size(dstinfo.format) * h.width;
	dst.depth_pitch = dst.row_pitch * h.height;

	b.convert_to(0, dst, dstinfo.format, copy_option::flip_vertically);
	return p;
}

texel_buffer decode_tga(const void* buffer, size_t size, const alpha_option& option, const layout& layout)
{
	info si = get_info_tga(buffer, size);
	oCHECK(si.format != format::unknown, "invalid bmp");
	const TGAHeader* h = (const TGAHeader*)buffer;
	info dsi = si;
	dsi.format = alpha_option_format(si.format, option);
	dsi.layout = layout;
	auto src = get_const_mapped_subresource(si, 0, 0, &h[1]);

	texel_buffer b(dsi);
	b.convert_from(0, src, si.format, copy_option::flip_vertically);
	return b;
}

}}
