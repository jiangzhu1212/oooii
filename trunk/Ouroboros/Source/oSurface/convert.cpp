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

namespace ouro {
	namespace surface {

#define SC_PARAMS int _Subresource, const info& _SourceInfo, const const_mapped_subresource& _Source, const info& _DestinationInfo, mapped_subresource* _pDestination

typedef void (*pixel_convert)(const void* _pSourcePixel, void* _pDestinationPixel);

static void r8g8b8a8_unorm_to_r8g8b8_unorm(const void* a, void* b)
{
	const unsigned char* aa = (const unsigned char*)a;
	unsigned char* bb = (unsigned char*)b;
	*bb++ = *aa++;
	*bb++ = *aa++;
	*bb++ = *aa++;
}

static void r8g8b8_unorm_to_r8g8b8a8_unorm(const void* a, void* b)
{
	const unsigned char* aa = (const unsigned char*)a;
	unsigned char* bb = (unsigned char*)b;
	*bb++ = *aa++;
	*bb++ = *aa++;
	*bb++ = *aa++;
	*bb++ = 0xff;
}

static void b8g8r8a8_unorm_to_b8g8r8_unorm(const void* a, void* b)
{
	const unsigned char* aa = (const unsigned char*)a;
	unsigned char* bb = (unsigned char*)b;
	*bb++ = *aa++;
	*bb++ = *aa++;
	*bb++ = *aa++;
}

static void b8g8r8_unorm_to_b8g8r8a8_unorm(const void* a, void* b)
{
	const unsigned char* aa = (const unsigned char*)a;
	unsigned char* bb = (unsigned char*)b;
	*bb++ = *aa++;
	*bb++ = *aa++;
	*bb++ = *aa++;
	*bb++ = 0xff;
}

static void swap_red_blue_r8g8b8_unorm(const void* a, void* b)
{
	const unsigned char* aa = (const unsigned char*)a;
	unsigned char* bb = (unsigned char*)b;
	bb[2] = aa[0];
	bb[1] = aa[1];
	bb[0] = aa[2];
}

static void swap_red_blue_r8g8b8a8_unorm(const void* a, void* b)
{
	const unsigned char* aa = (const unsigned char*)a;
	unsigned char* bb = (unsigned char*)b;
	bb[2] = aa[0];
	bb[1] = aa[1];
	bb[0] = aa[2];
	bb[3] = aa[3];
}

pixel_convert get_pixel_convert(format _SourceFormat, format _DestinationFormat)
{
	#define IO(s,d) (((s)<<16) | (d))
	int sel = IO(_SourceFormat, _DestinationFormat);
	switch (sel)
	{
		case IO(r8g8b8a8_unorm, r8g8b8_unorm): return r8g8b8a8_unorm_to_r8g8b8_unorm;
		case IO(r8g8b8_unorm, r8g8b8a8_unorm): return r8g8b8_unorm_to_r8g8b8a8_unorm;
		case IO(b8g8r8a8_unorm, b8g8r8_unorm): return b8g8r8a8_unorm_to_b8g8r8_unorm;
		case IO(b8g8r8a8_unorm, r8g8b8_unorm): return swap_red_blue_r8g8b8_unorm;
		case IO(b8g8r8_unorm, b8g8r8a8_unorm): return b8g8r8_unorm_to_b8g8r8a8_unorm;
		case IO(b8g8r8_unorm, r8g8b8_unorm): 
		case IO(r8g8b8_unorm, b8g8r8_unorm): return swap_red_blue_r8g8b8_unorm;
		case IO(b8g8r8a8_unorm, r8g8b8a8_unorm): 
		case IO(r8g8b8a8_unorm, b8g8r8a8_unorm): return swap_red_blue_r8g8b8a8_unorm;
		default: break;
	}
	throw std::invalid_argument(formatf("%s -> %s not supported", as_string(_SourceFormat), as_string(_DestinationFormat)));
	#undef IO
}

static void convert_subresource(pixel_convert _Convert, const subresource_info& _SubresourceInfo
	, format _SourceFormat, const const_mapped_subresource& _Source
	, format _DestinationFormat, mapped_subresource* _pDestination)
{
	const int selSize = element_size(_SourceFormat);
	const int delSize = element_size(_DestinationFormat);

	for (int y = 0; y < _SubresourceInfo.dimensions.y; y++)
	{
		const unsigned char* srow = (const unsigned char*)_Source.data + (_Source.row_pitch * y);
		unsigned char* drow = (unsigned char*)_pDestination->data + (_pDestination->row_pitch * y);

		for (int x = 0; x < _SubresourceInfo.dimensions.x; x++)
		{
			const unsigned char* spixel = srow + (selSize * x);
			unsigned char* dpixel = drow + (delSize * x);
			_Convert(spixel, dpixel);
		}
	}
}

void convert_subresource(const subresource_info& _SubresourceInfo
	, format _SourceFormat, const const_mapped_subresource& _Source
	, format _DestinationFormat, mapped_subresource* _pDestination)
{
	pixel_convert cv = get_pixel_convert(_SourceFormat, _DestinationFormat);
	convert_subresource(cv, _SubresourceInfo, _SourceFormat, _Source, _DestinationFormat, _pDestination);
}

void convert(const info& _SourceInfo, const const_mapped_subresource& _Source, const info& _DestinationInfo, mapped_subresource* _pDestination)
{
	if (any(_SourceInfo.dimensions != _DestinationInfo.dimensions))
		throw std::invalid_argument("dimensions must be the same");
	if (_SourceInfo.array_size != _SourceInfo.array_size)
		throw std::invalid_argument("array_size mismatch");

	pixel_convert cv = get_pixel_convert(_SourceInfo.format, _DestinationInfo.format);

	const int NumMips = num_mips(_SourceInfo);

	for (int mip = 0; mip < NumMips; mip++)
	{
		for (int slice = 0; slice < _SourceInfo.array_size; slice++)
		{
			const int Subresource = calc_subresource(mip, slice, 0, NumMips, _SourceInfo.array_size);

			subresource_info SourceSubresourceInfo = subresource(_SourceInfo, Subresource);
			subresource_info DestinationSubresourceInfo = subresource(_DestinationInfo, Subresource);

			oASSERT(all(SourceSubresourceInfo.dimensions == DestinationSubresourceInfo.dimensions), "dimensions mismatch");
		
			const_mapped_subresource Source = get_const_mapped_subresource(_SourceInfo, Subresource, 0, _Source.data);
			mapped_subresource Destination = get_mapped_subresource(_DestinationInfo, Subresource, 0, _pDestination->data);
		
			convert_subresource(cv, SourceSubresourceInfo, _SourceInfo.format, Source, _DestinationInfo.format, &Destination);
		}
	}
}

typedef void (*pixel_swizzle)(void* _pPixel);

static void sw_red_blue(void* _pPixel)
{
	unsigned char* r = (unsigned char*)_pPixel;
	unsigned char* b = r + 2;
	std::swap(*r, *b);
}

pixel_swizzle get_pixel_swizzle(surface::format _SourceFormat, surface::format _DestinationFormat)
{
	#define IO(s,d) (((s)<<16) | (d))
	int sel = IO(_SourceFormat, _DestinationFormat);
	switch (sel)
	{
		case IO(r8g8b8_unorm, b8g8r8_unorm):
		case IO(b8g8r8_unorm, r8g8b8_unorm):
		case IO(r8g8b8a8_unorm, b8g8r8a8_unorm):
		case IO(b8g8r8a8_unorm, r8g8b8a8_unorm): return sw_red_blue;
		default: break;
	}
	throw std::invalid_argument(formatf("%s -> %s conversion not supported", as_string(_SourceFormat), as_string(_DestinationFormat)));
	#undef IO
}

void convert_swizzle(const info& _SurfaceInfo, surface::format _NewFormat, mapped_subresource* _pSurface)
{
	pixel_swizzle sw = get_pixel_swizzle(_SurfaceInfo.format, _NewFormat);
	surface::enumerate_pixels(_SurfaceInfo, *_pSurface, sw);
}

	} // namespace surface
} // namespace ouro
