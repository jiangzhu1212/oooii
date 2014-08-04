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
#include <oBase/memory.h>
#include <oBase/throw.h>
#include "psd.h"

namespace ouro { namespace surface {

static format get_format(const psd_header* h)
{
#define SELFMT(ch, bpp) (((ch)<<16)|(bpp))
	switch (SELFMT(h->num_channels, h->bits_per_channel))
	{
		case SELFMT(3, psd_bits_per_channel::k8): return format::b8g8r8_unorm;
		case SELFMT(4, psd_bits_per_channel::k8): return format::b8g8r8a8_unorm;
		case SELFMT(4, psd_bits_per_channel::k16): return format::r16g16b16a16_unorm;
		case SELFMT(3, psd_bits_per_channel::k32): return format::r32g32b32_uint;
		case SELFMT(4, psd_bits_per_channel::k32): return format::r32g32b32a32_uint;
		default: break;
	}
#undef SELFMT
	return format::unknown;
}

info get_info_psd(const void* buffer, size_t size, psd_header* out_header)
{
	if (!psd_validate(buffer, size, out_header))
		return info();

	// only supports rgb or rgba at present
	if (out_header->num_channels < 3 || out_header->num_channels > 4)
		return info();

	if (out_header->bits_per_channel != psd_bits_per_channel::k8)
		return info();

	info i;
	i.dimensions = uint3(out_header->width, out_header->height, 1);
	i.format = get_format(out_header);
	return i;
}

info get_info_psd(const void* buffer, size_t size)
{
	psd_header h;
	return get_info_psd(buffer, size, &h);
}

scoped_allocation encode_psd(const texel_buffer& b, const alpha_option& option, const compression& compression)
{
	oTHROW(operation_not_supported, "psd encoding not supported");
}

// returns where _pSource left off
void* rle_decode2(void* oRESTRICT _pDestination, size_t _SizeofDestination, 
	size_t _ElementStride, size_t _RleElementSize, const void* oRESTRICT _pSource)
{
	int8_t* d = (int8_t*)_pDestination;
	const int8_t* oRESTRICT end = byte_add(d, _SizeofDestination);
	int8_t* oRESTRICT s = (int8_t*)_pSource;
	size_t dstep = _ElementStride - _RleElementSize;

	while (d < end)
	{
		int8_t count = *s++;
		if (count >= 0)
		{
			count = 1 + count;
			while (count--)
			{
				size_t bytes = _RleElementSize;
				while (bytes--)
					*d++ = *s++;
				d += dstep;
			}
		}

		else
		{
			count = 1 - count;
			while (count--)
			{
				for (size_t byte = 0; byte < _RleElementSize; byte)
					*d++ = *(s + byte);
				d += _ElementStride;
			}

			s += _RleElementSize;
		}
	}

	return s;
}

texel_buffer decode_psd(const void* buffer, size_t size, const alpha_option& option, const mip_layout& layout)
{
	psd_header h;
	info si = get_info_psd(buffer, size, &h);
	oCHECK(si.format != format::unknown, "invalid psd");

	texel_buffer b(si);

	auto bits_header = (const ushort*)get_image_data_section(buffer, size);
	auto compression = (const psd_compression)psd_swap(*bits_header);
	const void* bits = &bits_header[1];
	const size_t channel_bytes = h.bits_per_channel / 8;
	const size_t pixel_bytes = channel_bytes * h.num_channels;
		
	mapped_subresource mapped;
	uint2 byte_dimensions;
	b.map(0, &mapped, &byte_dimensions);
	finally UnmapBuffer([&] { b.unmap(0); });

	switch (compression)
	{
		//case psd_compression::raw:
		//{
		//	const auto sizes = channel_bits(si.format);
		//	const size_t plane_elemment_pitch = sizes.r / 8;
		//	const size_t plane_row_pitch = plane_elemment_pitch * si.dimensions.x;
		//	const size_t plane_depth_pitch = plane_row_pitch * si.dimensions.y;

		//	const bool dst_has_alpha = has_alpha(si.format);

		//	const void* red = byte_add(bits, plane_depth_pitch);
		//	const void* green = byte_add(red, plane_depth_pitch);
		//	const void* blue = byte_add(green, plane_depth_pitch);
		//	const void* alpha = (sizes.a ? byte_add(blue, plane_depth_pitch) : nullptr);

		//	switch (h.bits_per_channel)
		//	{
		//		// todo: swap around the formats as appropriate
		//		case psd_bits_per_channel::k8: interleave_channels<uchar>((uchar*)mapped.data, byte_dimensions.x, byte_dimensions.y, dst_has_alpha, red, green, blue, alpha); break;
		//		case psd_bits_per_channel::k16: interleave_channels<ushort>((ushort*)mapped.data, byte_dimensions.x, byte_dimensions.y, dst_has_alpha, red, green, blue, alpha); break;
		//		case psd_bits_per_channel::k32: interleave_channels<uint>((uint*)mapped.data, byte_dimensions.x, byte_dimensions.y, dst_has_alpha, red, green, blue, alpha); break;
		//		default: oTHROW(operation_not_supported, "unsupported bitdepth in psd decode");
		//	}

		//	break;
		//}

		case psd_compression::rle:
		{
			size_t size = byte_dimensions.x * byte_dimensions.y;
			void* d = mapped.data;
			
			for (uint16_t i = 0; i < h.num_channels; i++)
			{
				bits = rle_decode2(d, size, pixel_bytes, channel_bytes, bits);
				d = byte_add(d, channel_bytes);
			}

			break;
		}

		default: oTHROW(operation_not_supported, "unsupported compression type %d in psd decode", (int)compression);
	}

	return b;
}

}}