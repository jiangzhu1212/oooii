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
 * OF CONTRACT, TORT OR OTHERWrISE, ARISING FROM, OUT OF OR IN CONNECTION  *
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.        *
 **************************************************************************/
#include <oSurface/codec.h>
#include <oSurface/convert.h>
#include <dxgi.h>
#include "dds.h"

namespace ouro { namespace surface {

static format from_dds_format(DDS_FORMAT f)
{
	switch (f)
	{
		case DDS_FORMAT_R8G8B8_UNORM: return format::r8g8b8_unorm;
		case DDS_FORMAT_B8G8R8_UNORM: return format::b8g8r8_unorm;
		default: break;
	}

	return (format)f;
}

static DDS_FORMAT to_dds_format(format f)
{
	return (DDS_FORMAT)f;
}

dds_pixel_format GetPixelFormat(const format& f)
{
	static const dds_pixel_format bgr = DDSPF_B8G8R8_UNORM;
	static const dds_pixel_format rgb = DDSPF_R8G8B8_UNORM;

	switch (f)
	{
		case format::b8g8r8_unorm: return bgr;
		case format::r8g8b8_unorm: return rgb;
		default: break;
	}
	return to_ddspf(to_dds_format(f));
}

bool is_dds(const void* buffer, size_t size)
{
	return size >= (sizeof(uint32_t) + sizeof(dds_header)) && *(const uint32_t*)buffer == dds_signature;
}

format required_input_dds(const format& stored)
{
	return (is_texture(stored) || stored == format::r8g8b8_unorm || stored == format::b8g8r8_unorm) 
		? stored
		: format::unknown;
}

info get_info_dds(const void* buffer, size_t size)
{
	if (!is_dds(buffer, size))
		return info();

	auto* h = (const dds_header*)byte_add(buffer, sizeof(uint32_t));
  if (h->dwSize != sizeof(dds_header) || h->ddspf.dwSize != sizeof(dds_pixel_format))
		return info();

	const bool kIsD3D10 = has_dx10_header(*h);

	if ((kIsD3D10 && size < (sizeof(dds_header) + sizeof(uint32_t) + sizeof(dds_header_dx10))))
		return info();

	bool cubemap = false;
	DDS_RESOURCE_DIMENSION resourceType;

	info si;
	si.dimensions = int3(h->dwWidth, h->dwHeight, h->dwDepth);
	si.mip_layout = (h->dwFlags & DDS_HEADER_FLAGS_MIPMAP) && h->dwMipMapCount != 1 ? mip_layout::tight : mip_layout::none;

	if (kIsD3D10)
	{
		auto d3d10ext = (const dds_header_dx10*)&h[1];
		si.array_size = d3d10ext->arraySize;
		if (!si.array_size) // invalid according to DDS
			return info();

		switch (d3d10ext->dxgiFormat)
		{
			case DDS_FORMAT_AI44:
			case DDS_FORMAT_IA44:
			case DDS_FORMAT_P8:
			case DDS_FORMAT_A8P8: return info(); // not supported
			default:
				if (bits(from_dds_format(d3d10ext->dxgiFormat)) == 0)
					return info();
		}

		si.format = from_dds_format(d3d10ext->dxgiFormat);

		switch (d3d10ext->resourceDimension)
		{
			case DDS_RESOURCE_DIMENSION_TEXTURE1D:
				// D3DX writes 1D textures with a fixed Height of 1
				if ((h->dwFlags & DDS_HEIGHT) && h->dwHeight != 1)
					return info();
				si.dimensions.y = si.dimensions.z = 1;
				break;

			case DDS_RESOURCE_DIMENSION_TEXTURE2D:
				if (d3d10ext->miscFlag & DDS_RESOURCE_MISC_TEXTURECUBE)
				{
					si.array_size *= 6;
					cubemap = true;
				}
				si.dimensions.z = 1;
				break;

			case DDS_RESOURCE_DIMENSION_TEXTURE3D:
				if (!(h->dwFlags & DDS_HEADER_FLAGS_VOLUME))
					return info(); // ERROR_INVALID_DATA
				if (si.array_size > 1)
					return info(); // ERROR_NOT_SUPPORTED
				break;

			default:
				return info();
		}

		resourceType = d3d10ext->resourceDimension;
	}

	else
	{
		si.format = from_dds_format(from_ddspf(h->ddspf));
		if (si.format == format::unknown)
		{
			if (h->ddspf.dwRGBBitCount == 24)
			{
				auto ddpf = h->ddspf;
				if (ISBITMASK(0x000000ff,0x0000ff00,0x00ff0000,0x00000000)) si.format = format::r8g8b8_unorm;
				else if (ISBITMASK(0x00ff0000,0x0000ff00,0x000000ff,0x00000000)) si.format = format::b8g8r8_unorm;
				else return info();
			}
			else
				return info();
		}

		if (h->dwFlags & DDS_HEADER_FLAGS_VOLUME)
			resourceType = DDS_RESOURCE_DIMENSION_TEXTURE3D;
		else 
		{
			if (h->dwCaps2 & DDS_CUBEMAP)
			{
				// We require all six faces to be defined
				if ((h->dwCaps2 & DDS_CUBEMAP_ALLFACES ) != DDS_CUBEMAP_ALLFACES)
					return info();
				si.array_size = 6;
				cubemap = true;
			}

			si.dimensions.z = 1;
			resourceType = DDS_RESOURCE_DIMENSION_TEXTURE2D;
			// No way for a legacy Direct3D 9 DDS to express a 1D texture
		}

		oCHECK0(bits(si.format));
	}

	// There's no way to distinguish between a not-array and an array of size 1.
	// To be consistent with other formats rather than considering every image 
	// an array of size 1, consider any array of size 1 not an array.
	if (si.array_size == 1)
		si.array_size = 0;

	return si;
}

static void map_bits(const info& inf, const void* oRESTRICT src_dds_buffer, size_t src_dds_size, const_mapped_subresource* oRESTRICT subresources, size_t num_subresources)
{
	const void* end = byte_add(src_dds_buffer, src_dds_size);
	const uint nMips = max(1u, num_mips(inf.mip_layout != mip_layout::none, inf.dimensions));
	const uint nSlices = max(1u, inf.array_size);

	size_t index = 0;
	for (uint j = 0; j < nSlices; j++)
	{
		for (uint i = 0; i < nMips; i++ )
		{
			uint subresource = calc_subresource(i, j, 0, nMips, nSlices);
			subresources[subresource] = get_const_mapped_subresource(inf, subresource, 0, src_dds_buffer);
			oCHECK(byte_add(subresources[subresource].data, subresources[subresource].depth_pitch) <= end, "end of file");
		}
	}
}

static void map_bits(const info& inf, const void* oRESTRICT src_dds_buffer, size_t src_dds_size, mapped_subresource* oRESTRICT subresources, size_t num_subresources)
{
	map_bits(inf, src_dds_buffer, src_dds_size, (const_mapped_subresource*)subresources, num_subresources);
}

scoped_allocation encode_dds(const image& img, const allocator& file_alloc, const allocator& temp_alloc, const compression& compression)
{
	//if (1) oTHROW(operation_not_supported, "dds not yet implemented");

	const auto inf = img.get_info();
	oCHECK(inf.mip_layout == mip_layout::none || inf.mip_layout == mip_layout::tight, "right and below mip layouts not supported");

	const bool is3d = false; // how to specify? probably .z != 0...
	const bool isbc = dds_is_block_compressed(to_dds_format(inf.format));
	const bool mips = inf.mip_layout != mip_layout::none;
	const bool iscube = false; // how to specify?

	const dds_pixel_format ddspf = GetPixelFormat(inf.format);
	const bool kHasDX10 = iscube || inf.array_size > 1 || ddspf.dwFourCC == MAKEFOURCC('D','X','1','0');

	const size_t bits_size = img.size();
	const size_t size = bits_size + sizeof(dds_signature) + sizeof(dds_header) 
		+ (kHasDX10 ? sizeof(dds_header_dx10) : 0);

	scoped_allocation alloc(file_alloc.allocate(size, 0), size, file_alloc.deallocate);

	*(uint*)alloc = dds_signature;
	auto h = (dds_header*)byte_add((void*)alloc, sizeof(dds_signature));
	auto d3d10ext = (dds_header_dx10*)&h[1];
	void* bits = kHasDX10 ? (void*)&d3d10ext[1] : (void*)&h[1];

	uint pitch = dds_calc_pitch(to_dds_format(inf.format), inf.dimensions.x);
	h->dwSize = sizeof(dds_header);
	h->dwFlags = DDS_HEADER_FLAGS_TEXTURE | (mips ? DDS_HEADER_FLAGS_MIPMAP : 0) | (isbc ? DDS_HEADER_FLAGS_LINEARSIZE : 0) | (is3d ? DDS_HEADER_FLAGS_VOLUME : 0);
	h->dwHeight = inf.dimensions.y;
	h->dwWidth = inf.dimensions.x;
	h->dwPitchOrLinearSize = isbc ? pitch : 0;
	h->dwDepth = is3d ? inf.dimensions.z : 0;
	h->dwMipMapCount = mips ? num_mips(inf) : 0;
	memset(h->dwReserved1, 0, sizeof(h->dwReserved1));
	h->ddspf = ddspf;
	h->dwCaps = DDS_SURFACE_FLAGS_TEXTURE | ((is3d || mips || inf.array_size) ? DDS_SURFACE_FLAGS_COMPLEX : 0) | (mips ? DDS_SURFACE_FLAGS_MIPMAP : 0);
	h->dwCaps2 = (iscube ? DDS_CUBEMAP_ALLFACES : 0) | (is3d ? DDS_FLAGS_VOLUME : 0);
	h->dwCaps3 = 0;
	h->dwCaps4 = 0;
	h->dwReserved2 = 0;
	if (kHasDX10)
	{
		d3d10ext->dxgiFormat = to_dds_format(inf.format);
		d3d10ext->resourceDimension = is3d ? DDS_RESOURCE_DIMENSION_TEXTURE3D : DDS_RESOURCE_DIMENSION_TEXTURE2D; // how to tell 1D? y==0 && z==0?
		d3d10ext->miscFlag = iscube ? DDS_RESOURCE_MISC_TEXTURECUBE : 0;
		d3d10ext->arraySize = is3d ? 1u : max(1u, inf.array_size); // DDS requires this to be non-zero
		d3d10ext->miscFlags2 = 0;
	}

	const int nSubresources = num_subresources(inf);
	mapped_subresource* subresources = (mapped_subresource*)temp_alloc.allocate(sizeof(mapped_subresource) * nSubresources, 0);
	finally DeleteInitData([&] { if (subresources) temp_alloc.deallocate(subresources); });
	map_bits(inf, bits, bits_size, subresources, nSubresources);

	for (int i = 0; i < nSubresources; i++)
		img.copy_to(i, subresources[i]);

	return alloc;
}

image decode_dds(const void* buffer, size_t size, const allocator& texel_alloc, const allocator& temp_alloc, const mip_layout& layout)
{
	info inf = get_info_dds(buffer, size);
	oCHECK(inf.format != format::unknown, "invalid dds");
	image img(inf, texel_alloc);

	const uint nSubresources = num_subresources(inf);
	const_mapped_subresource* subresources = (const_mapped_subresource*)temp_alloc.allocate(sizeof(const_mapped_subresource) * nSubresources, 0);
	finally DeleteInitData([&] { if (subresources) temp_alloc.deallocate(subresources); });

	auto h = (const dds_header*)byte_add(buffer, sizeof(dds_signature));
	const void* bits = byte_add(buffer, sizeof(dds_signature) + sizeof(dds_header) + (has_dx10_header(*h) ? sizeof(dds_header_dx10) : 0));
	map_bits(inf, bits, size - byte_diff(bits, buffer), subresources, nSubresources);

	for (uint i = 0; i < nSubresources; i++)
		img.update_subresource(i, subresources[i]);

	return img;
}

}}
