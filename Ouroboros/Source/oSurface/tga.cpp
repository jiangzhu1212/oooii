// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.
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

scoped_allocation encode_tga(const image& img, const allocator& file_alloc, const allocator& temp_alloc, const compression& compression)
{
	oCHECK_ARG(compression == compression::none, "compression not supported");

	auto info = img.get_info();
	oCHECK_ARG(info.format == format::b8g8r8a8_unorm || info.format == format::b8g8r8_unorm, "source must be b8g8r8a8_unorm or b8g8r8_unorm");
	oCHECK_ARG(info.dimensions.x <= 0xffff && info.dimensions.y <= 0xffff, "dimensions must be <= 65535");

	tga_header h = {0};
	h.data_type_field = tga_data_type_field::rgb;
	h.bpp = (uchar)bits(info.format);
	h.width = (ushort)info.dimensions.x;
	h.height = (ushort)info.dimensions.y;

	const size_t size = sizeof(tga_header) + h.width * h.height * (h.bpp/8);

	scoped_allocation p(file_alloc.allocate(size, memory_alignment::align_default, "encoded tga"), size, file_alloc.deallocate);
	memcpy(p, &h, sizeof(tga_header));
	mapped_subresource dst;
	dst.data = byte_add((void*)p, sizeof(tga_header));
	dst.row_pitch = element_size(info.format) * h.width;
	dst.depth_pitch = dst.row_pitch * h.height;

	img.copy_to(0, dst, copy_option::flip_vertically);
	return p;
}

image decode_tga(const void* buffer, size_t size, const allocator& texel_alloc, const allocator& temp_alloc, const mip_layout& layout)
{
	info si = get_info_tga(buffer, size);
	oCHECK(si.format != format::unknown, "invalid tga");
	const tga_header* h = (const tga_header*)buffer;
	info dsi = si;
	dsi.mip_layout = layout;
	auto src = get_const_mapped_subresource(si, 0, 0, &h[1]);

	image img(dsi, texel_alloc);
	img.copy_from(0, src, copy_option::flip_vertically);
	return img;
}

}}
