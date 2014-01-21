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

static void convert_subresource_scanline(int _HorizontalElementCount
	, int _NthScanline
	, pixel_convert _Convert
	, int SourceElementSize
	, int DestinationElementSize
	, const const_mapped_subresource& _Source
	, mapped_subresource* _pDestination)
{
	const unsigned char* srow = (const unsigned char*)_Source.data + (_Source.row_pitch * _NthScanline);
	unsigned char* drow = (unsigned char*)_pDestination->data + (_pDestination->row_pitch * _NthScanline);

	for (int x = 0; x < _HorizontalElementCount; x++)
	{
		const unsigned char* spixel = srow + (SourceElementSize * x);
		unsigned char* dpixel = drow + (DestinationElementSize * x);
		_Convert(spixel, dpixel);
	}
}

static void convert_subresource(pixel_convert _Convert
	, const subresource_info& _SubresourceInfo
	, const const_mapped_subresource& _Source
	, format _DestinationFormat
	, mapped_subresource* _pDestination
	, bool _FlipVertically)
{
	const int selSize = element_size(_SubresourceInfo.format);
	const int delSize = element_size(_DestinationFormat);
	if (_FlipVertically)
		for (int y = _SubresourceInfo.dimensions.y-1; y >= 0; y--)
			convert_subresource_scanline(_SubresourceInfo.dimensions.x, y, _Convert, selSize, delSize, _Source, _pDestination);
	else
		for (int y = 0; y < _SubresourceInfo.dimensions.y; y++)
			convert_subresource_scanline(_SubresourceInfo.dimensions.x, y, _Convert, selSize, delSize, _Source, _pDestination);
}

void convert_subresource(const subresource_info& _SubresourceInfo
	, const const_mapped_subresource& _Source
	, format _DestinationFormat
	, mapped_subresource* _pDestination
	, bool _FlipVertically)
{
	if (_SubresourceInfo.format == _DestinationFormat)
		copy(_SubresourceInfo, _Source, _pDestination, false);
	else
	{
		pixel_convert cv = get_pixel_convert(_SubresourceInfo.format, _DestinationFormat);
		convert_subresource(cv, _SubresourceInfo, _Source, _DestinationFormat, _pDestination, _FlipVertically);
	}
}

void convert(const info& _SourceInfo
	, const const_mapped_subresource& _Source
	, const info& _DestinationInfo
	, mapped_subresource* _pDestination
	, bool _FlipVertically)
{
	if (any(_SourceInfo.dimensions != _DestinationInfo.dimensions))
		throw std::invalid_argument("dimensions must be the same");
	if (_SourceInfo.array_size != _SourceInfo.array_size)
		throw std::invalid_argument("array_size mismatch");

	pixel_convert cv = get_pixel_convert(_SourceInfo.format, _DestinationInfo.format);

	const int nSubresources = surface::num_subresources(_SourceInfo);
	for (int subresource = 0; subresource < nSubresources; subresource++)
	{
		auto srcSri = surface::subresource(_SourceInfo, subresource);
		auto dstSri = surface::subresource(_DestinationInfo, subresource);

		if (any(srcSri.dimensions != dstSri.dimensions))
			throw std::invalid_argument("dimensions mismatch");

		const_mapped_subresource Source = get_const_mapped_subresource(_SourceInfo, subresource, 0, _Source.data);
		mapped_subresource Destination = get_mapped_subresource(_DestinationInfo, subresource, 0, _pDestination->data);

		for (int slice = 0; slice < srcSri.dimensions.z; slice++)
		{
			convert_subresource(cv, srcSri, Source, _DestinationInfo.format, &Destination, _FlipVertically);

			Source.data = byte_add(Source.data, Source.depth_pitch);
			Destination.data = byte_add(Destination.data, Source.depth_pitch);
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
