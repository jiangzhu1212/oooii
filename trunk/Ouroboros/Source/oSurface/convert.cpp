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
#include <oSurface/convert.h>
#include <oBase/assert.h>
#include <oBase/stringize.h>
#include <oBase/throw.h>
#include <oHLSL/oHLSLMath.h>

#include <ispc_texcomp.h>

namespace ouro { namespace surface {

typedef void (*pixel_convert)(const void* src_pixel, void* dst_pixel);

static void r8g8b8a8_unorm_to_r8g8b8_unorm(const void* a, void* b)
{
	const uchar* aa = (const uchar*)a;
	uchar* bb = (uchar*)b;
	*bb++ = *aa++;
	*bb++ = *aa++;
	*bb++ = *aa++;
}

static void r8g8b8_unorm_to_r8g8b8a8_unorm(const void* a, void* b)
{
	const uchar* aa = (const uchar*)a;
	uchar* bb = (uchar*)b;
	*bb++ = *aa++;
	*bb++ = *aa++;
	*bb++ = *aa++;
	*bb++ = 0xff;
}

static void b8g8r8a8_unorm_to_b8g8r8_unorm(const void* a, void* b)
{
	const uchar* aa = (const uchar*)a;
	uchar* bb = (uchar*)b;
	*bb++ = *aa++;
	*bb++ = *aa++;
	*bb++ = *aa++;
}

static void b8g8r8_unorm_to_b8g8r8a8_unorm(const void* a, void* b)
{
	const uchar* aa = (const uchar*)a;
	uchar* bb = (uchar*)b;
	*bb++ = *aa++;
	*bb++ = *aa++;
	*bb++ = *aa++;
	*bb++ = 0xff;
}

static void b8g8r8_unorm_to_a8b8g8r8_unorm(const void* a, void* b)
{
	const uchar* aa = (const uchar*)a;
	uchar* bb = (uchar*)b;
	*bb++ = 0xff;
	*bb++ = *aa++;
	*bb++ = *aa++;
	*bb++ = *aa++;
}

static void b8g8r8_unorm_to_x8b8g8r8_unorm(const void* a, void* b)
{
	const uchar* aa = (const uchar*)a;
	uchar* bb = (uchar*)b;
	*bb++ = 0xff;
	*bb++ = *aa++;
	*bb++ = *aa++;
	*bb++ = *aa++;
}

static void a8b8g8r8_unorm_to_b8g8r8_unorm(const void* a, void* b)
{
	const uchar* aa = (const uchar*)a;
	uchar* bb = (uchar*)b;
	aa++; // alpha
	*bb++ = *aa++;
	*bb++ = *aa++;
	*bb++ = *aa++;
}

static void x8b8g8r8_unorm_to_b8g8r8_unorm(const void* a, void* b)
{
	const uchar* aa = (const uchar*)a;
	uchar* bb = (uchar*)b;
	aa++; // x
	*bb++ = *aa++;
	*bb++ = *aa++;
	*bb++ = *aa++;
}

static void a8b8g8r8_unorm_to_b8g8r8a8_unorm(const void* a, void* b)
{
	const uchar* aa = (const uchar*)a;
	uchar* bb = (uchar*)b;
	uchar al = *aa++;
	*bb++ = *aa++;
	*bb++ = *aa++;
	*bb++ = *aa++;
	*bb   = al;
}

static void x8b8g8r8_unorm_to_b8g8r8a8_unorm(const void* a, void* b)
{
	const uchar* aa = (const uchar*)a;
	uchar* bb = (uchar*)b;
	uchar x = *aa++;
	*bb++ = *aa++;
	*bb++ = *aa++;
	*bb++ = *aa++;
	*bb   = x;
}

static void swap_red_blue_r8g8b8_unorm(const void* a, void* b)
{
	const uchar* aa = (const uchar*)a;
	uchar* bb = (uchar*)b;
	bb[2] = aa[0];
	bb[1] = aa[1];
	bb[0] = aa[2];
}

static void swap_red_blue_r8g8b8a8_unorm(const void* a, void* b)
{
	const uchar* aa = (const uchar*)a;
	uchar* bb = (uchar*)b;
	bb[2] = aa[0];
	bb[1] = aa[1];
	bb[0] = aa[2];
	bb[3] = aa[3];
}

static void x8b8g8r8_unorm_to_bc1_unorm(const void* a, void* b)
{
}

static void x8b8g8r8_unorm_to_bc7_unorm(const void* a, void* b)
{
}

pixel_convert get_pixel_convert(format srcfmt, format dstfmt)
{
	#define IO_(s,d) ((uint(s)<<16) | uint(d))
	#define IO(s,d) IO_(format::s, format::d)
	uint sel = IO_(srcfmt, dstfmt);
	switch (sel)
	{
		case IO(r8g8b8a8_unorm, r8g8b8_unorm): return r8g8b8a8_unorm_to_r8g8b8_unorm;
		case IO(r8g8b8_unorm,		r8g8b8a8_unorm): return r8g8b8_unorm_to_r8g8b8a8_unorm;
		case IO(b8g8r8a8_unorm, b8g8r8_unorm): return b8g8r8a8_unorm_to_b8g8r8_unorm;
		case IO(b8g8r8a8_unorm, r8g8b8_unorm): return swap_red_blue_r8g8b8_unorm;
		case IO(b8g8r8_unorm,		b8g8r8a8_unorm): return b8g8r8_unorm_to_b8g8r8a8_unorm;
		case IO(b8g8r8_unorm,		a8b8g8r8_unorm): return b8g8r8_unorm_to_a8b8g8r8_unorm;
		case IO(b8g8r8_unorm,		x8b8g8r8_unorm): return b8g8r8_unorm_to_x8b8g8r8_unorm;
		case IO(a8b8g8r8_unorm,	b8g8r8_unorm): return a8b8g8r8_unorm_to_b8g8r8_unorm;
		case IO(x8b8g8r8_unorm,	b8g8r8_unorm): return x8b8g8r8_unorm_to_b8g8r8_unorm;
		case IO(a8b8g8r8_unorm,	b8g8r8a8_unorm): return a8b8g8r8_unorm_to_b8g8r8a8_unorm;
		case IO(x8b8g8r8_unorm,	b8g8r8a8_unorm): return x8b8g8r8_unorm_to_b8g8r8a8_unorm;
		case IO(b8g8r8_unorm,		r8g8b8_unorm): 
		case IO(r8g8b8_unorm,		b8g8r8_unorm): return swap_red_blue_r8g8b8_unorm;
		case IO(b8g8r8a8_unorm, r8g8b8a8_unorm): 
		case IO(r8g8b8a8_unorm, b8g8r8a8_unorm): return swap_red_blue_r8g8b8a8_unorm;

		// tag-like noop functions that trigger special conversion routines
		case IO(x8b8g8r8_unorm, bc1_unorm): return x8b8g8r8_unorm_to_bc1_unorm;
		case IO(x8b8g8r8_unorm, bc7_unorm): return x8b8g8r8_unorm_to_bc7_unorm;

		default: break;
	}
	throw std::invalid_argument(formatf("%s -> %s not supported", as_string(srcfmt), as_string(dstfmt)));
	#undef IO
}

static void check_bc_inputs(const subresource_info& i
	, const const_mapped_subresource& src
	, const format& dst_format
	, const mapped_subresource& dst
	, const copy_option& option, const format& expected_source_format)
{
	oCHECK_ARG(is_block_compressed(dst_format), "non-bc format specified to BC compression");
	oCHECK_ARG(has_alpha(i.format) == has_alpha(dst_format) || dst_format == format::bc7_typeless || dst_format == format::bc7_unorm || dst_format == format::bc7_unorm_srgb, "BC compression alpha support mismatch");
	oCHECK_ARG(is_unorm(i.format) == is_unorm(dst_format), "BC compression unorm mismatch");
	oCHECK_ARG(is_srgb(i.format) == is_srgb(dst_format), "BC compression srgb mismatch");
	oCHECK_ARG((i.dimensions.x & 0x3) == 0, "width must be a multiple of 4 for BC compression");
	oCHECK_ARG((i.dimensions.y & 0x3) == 0, "height must be a multiple of 4 for BC compression");
	oCHECK(option == copy_option::none, "cannot flip vertically during BC compression");
	
	const uint SrcRowPitch = row_size(expected_source_format, i.dimensions.x);
	const uint BCRowPitch = row_size(dst_format, i.dimensions.x);
	oCHECK(src.row_pitch == SrcRowPitch, "mip_layout must be 'none' for a BC compression destination buffer");
	oCHECK(dst.row_pitch == BCRowPitch, "mip_layout must be 'none' for a BC compression destination buffer");
}

static void convert_subresource_scanline(int _HorizontalElementCount
	, uint _SrcNthScanline
	, uint _DstNthScanline
	, pixel_convert _Convert
	, int SourceElementSize
	, int DestinationElementSize
	, const const_mapped_subresource& src
	, const mapped_subresource& dst)
{
	const uchar* srow = (const uchar*)src.data + (src.row_pitch * _SrcNthScanline);
	uchar* drow = (uchar*)dst.data + (dst.row_pitch * _DstNthScanline);

	for (int x = 0; x < _HorizontalElementCount; x++)
	{
		const uchar* spixel = srow + (SourceElementSize * x);
		uchar* dpixel = drow + (DestinationElementSize * x);
		_Convert(spixel, dpixel);
	}
}

static void convert_subresource(pixel_convert convert
	, const subresource_info& i
	, const const_mapped_subresource& src
	, format dst_format
	, const mapped_subresource& dst
	, const copy_option& option)
{
	if (convert == x8b8g8r8_unorm_to_bc1_unorm)
	{
		check_bc_inputs(i, src, dst_format, dst, option, format::x8b8g8r8_unorm);
		CompressBlocksBC1((const rgba_surface*)src.data, (uchar*)dst.data);
	}

	if (convert == x8b8g8r8_unorm_to_bc7_unorm)
	{
		check_bc_inputs(i, src, dst_format, dst, option, format::x8b8g8r8_unorm);
		bc7_enc_settings settings;
		GetProfile_fast(&settings);
		CompressBlocksBC7((const rgba_surface*)src.data, (uchar*)dst.data, &settings);
	}

	else
	{
		const auto selSize = element_size(i.format);
		const auto delSize = element_size(dst_format);
		if (option == copy_option::flip_vertically)
		{
			const uint bottom = i.dimensions.y - 1;
			for (uint y = 0; y < i.dimensions.y; y++)
				convert_subresource_scanline(i.dimensions.x, y, bottom - y, convert, selSize, delSize, src, dst);
		}
		else
			for (uint y = 0; y < i.dimensions.y; y++)
				convert_subresource_scanline(i.dimensions.x, y, y, convert, selSize, delSize, src, dst);
	}
}

/*static*/ void convert_subresource_bc6h(const subresource_info& i
	, const const_mapped_subresource& src
	, format dst_format
	, const mapped_subresource& dst
	, const copy_option& option)
{
	oCHECK_ARG((i.dimensions.x & 0x3) == 0, "width must be a multiple of 4 for BC6h compression");
	oCHECK_ARG((i.dimensions.y & 0x3) == 0, "height must be a multiple of 4 for BC6h compression");

	const uint BCRowPitch = (i.dimensions.x/4) * 8;

	oCHECK(option == copy_option::none, "cannot flip vertically during BC6h compression");
	oCHECK(src.row_pitch == BCRowPitch, "layout must be 'image' for a BC6h compression destination buffer");
	oCHECK(dst.row_pitch == BCRowPitch, "layout must be 'image' for a BC6h compression destination buffer");

	bc6h_enc_settings settings;
	GetProfile_bc6h_fast(&settings);
	CompressBlocksBC6H((const rgba_surface*)src.data, (uint8_t*)dst.data, &settings);
}

void convert_subresource(const subresource_info& i
	, const const_mapped_subresource& src
	, format dst_format
	, const mapped_subresource& dst
	, const copy_option& option)
{
	switch (dst_format)
	{
		case surface::format::bc7_unorm_srgb:
		case surface::format::bc7_unorm:
		{
			check_bc_inputs(i, src, dst_format, dst, option, format::x8b8g8r8_unorm);
			bc7_enc_settings settings;
			GetProfile_fast(&settings);
			CompressBlocksBC7((const rgba_surface*)src.data, (uchar*)dst.data, &settings);
			break;
		}

		case surface::format::bc6h_typeless:
		case surface::format::bc6h_uf16:
		case surface::format::bc6h_sf16:
		{
			// IIRC input is x16b16g16r16
			oTHROW(operation_not_supported, "bc6h compression not yet integrated");
			break;
		}

		default:
		{
			if (i.format == dst_format)
				copy(i, src, dst, option);
			else
			{
				pixel_convert cv = get_pixel_convert(i.format, dst_format);
				convert_subresource(cv, i, src, dst_format, dst, option);
			}
		}
	}
}

void convert(const info& src_info
	, const const_mapped_subresource& src
	, const info& dst_info
	, const mapped_subresource& dst
	, const copy_option& option)
{
	if (any(src_info.dimensions != dst_info.dimensions))
		throw std::invalid_argument("dimensions must be the same");
	if (src_info.array_size != src_info.array_size)
		throw std::invalid_argument("array_size mismatch");

	pixel_convert cv = get_pixel_convert(src_info.format, dst_info.format);

	const int nSubresources = surface::num_subresources(src_info);
	for (int subresource = 0; subresource < nSubresources; subresource++)
	{
		auto srcSri = surface::subresource(src_info, subresource);
		auto dstSri = surface::subresource(dst_info, subresource);

		if (any(srcSri.dimensions != dstSri.dimensions))
			throw std::invalid_argument("dimensions mismatch");

		const_mapped_subresource Source = get_const_mapped_subresource(src_info, subresource, 0, src.data);
		mapped_subresource Destination = get_mapped_subresource(dst_info, subresource, 0, dst.data);

		for (uint slice = 0; slice < srcSri.dimensions.z; slice++)
		{
			convert_subresource(cv, srcSri, Source, dst_info.format, Destination, option);

			Source.data = byte_add(Source.data, Source.depth_pitch);
			Destination.data = byte_add(Destination.data, Source.depth_pitch);
		}
	}
}

typedef void (*pixel_swizzle)(void* _pPixel);

static void sw_red_blue(void* _pPixel)
{
	uchar* r = (uchar*)_pPixel;
	uchar* b = r + 2;
	std::swap(*r, *b);
}

pixel_swizzle get_pixel_swizzle(surface::format src_format, surface::format dst_format)
{
	#define IO(s,d) ((uint(s)<<16) | uint(d))
	uint sel = IO(src_format, dst_format);
	switch (sel)
	{
		case IO(format::r8g8b8_unorm, format::b8g8r8_unorm):
		case IO(format::b8g8r8_unorm, format::r8g8b8_unorm):
		case IO(format::r8g8b8a8_unorm, format::b8g8r8a8_unorm):
		case IO(format::b8g8r8a8_unorm, format::r8g8b8a8_unorm): return sw_red_blue;
		default: break;
	}
	throw std::invalid_argument(formatf("%s -> %s conversion not supported", as_string(src_format), as_string(dst_format)));
	#undef IO
}

void convert_swizzle(const info& i, const surface::format& new_format, const mapped_subresource& mapped)
{
	pixel_swizzle sw = get_pixel_swizzle(i.format, new_format);
	surface::enumerate_pixels(i, mapped, sw);
}

}}
