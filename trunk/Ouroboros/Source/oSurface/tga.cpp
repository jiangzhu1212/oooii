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
	si.format = h->bpp == 32 ? b8g8r8a8_unorm : b8g8r8_unorm;
	si.dimensions = int3(h->width, h->height, 1);
	return si;
}

std::shared_ptr<char> encode_tga(const buffer& buffer
	, size_t* out_size
	, const alpha_option::value& option
	, const compression::value& compression)
{
	oCHECK_ARG(compression == compression::none, "compression not supported");

	auto info = buffer.get_info();
	oCHECK_ARG(info.format == b8g8r8a8_unorm || info.format == b8g8r8_unorm, "source must be b8g8r8a8_unorm or b8g8r8_unorm");

	auto dstinfo = info;
	dstinfo.format = alpha_option_format(info.format, option);

	TGAHeader h = {0};
	h.data_type_field = TGA_RGB;
	h.bpp = (uchar)bits(dstinfo.format);
	h.width = (ushort)info.dimensions.x;
	h.height = (ushort)info.dimensions.y;

	const size_t size = sizeof(TGAHeader) + h.width * h.height * (h.bpp/8);

	void* p = malloc(size);
	std::shared_ptr<char> b((char*)p, free);
	if (out_size)
		*out_size = size;

	memcpy(p, &h, sizeof(TGAHeader));
	mapped_subresource dst;
	dst.data = byte_add(p, sizeof(TGAHeader));
	dst.row_pitch = element_size(dstinfo.format) * h.width;
	dst.depth_pitch = dst.row_pitch * h.height;

	buffer.convert_to(0, dst, dstinfo.format, copy_option::flip_vertically);
	return b;
}

buffer decode_tga(const void* _buffer, size_t size, const alpha_option::value& option, const layout& layout)
{
	info si = get_info_tga(_buffer, size);
	oCHECK(si.format != unknown, "invalid bmp");
	const TGAHeader* h = (const TGAHeader*)_buffer;
	info dsi = si;
	dsi.format = alpha_option_format(si.format, option);
	dsi.layout = layout;
	auto src = get_const_mapped_subresource(si, 0, 0, &h[1]);

	buffer b(dsi);
	b.convert_from(0, src, si.format, copy_option::flip_vertically);
	return b;
}

}}
