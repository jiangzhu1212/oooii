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
#include <oSurface/convert.h>

#include <dxgi.h>

namespace ouro { namespace surface {

static const uint DDS_MAGIC = 0x20534444; // "DDS "

#define DDS_FOURCC 0x00000004
#define DDS_RGB 0x00000040
#define DDS_RGBA 0x00000041
#define DDS_LUMINANCE 0x00020000
#define DDS_LUMINANCEA 0x00020001
#define DDS_ALPHA 0x00000002
#define DDS_PAL8 0x00000020

#define DDS_HEADER_FLAGS_TEXTURE        0x00001007  // DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT 
#define DDS_HEADER_FLAGS_MIPMAP         0x00020000  // DDSD_MIPMAPCOUNT
#define DDS_HEADER_FLAGS_VOLUME         0x00800000  // DDSD_DEPTH
#define DDS_HEADER_FLAGS_PITCH          0x00000008  // DDSD_PITCH
#define DDS_HEADER_FLAGS_LINEARSIZE     0x00080000  // DDSD_LINEARSIZE

#define DDS_HEIGHT 0x00000002 // DDSD_HEIGHT
#define DDS_WIDTH  0x00000004 // DDSD_WIDTH

#define DDS_SURFACE_FLAGS_TEXTURE 0x00001000 // DDSCAPS_TEXTURE
#define DDS_SURFACE_FLAGS_MIPMAP  0x00400008 // DDSCAPS_COMPLEX | DDSCAPS_MIPMAP
#define DDS_SURFACE_FLAGS_CUBEMAP 0x00000008 // DDSCAPS_COMPLEX

#define DDS_CUBEMAP_POSITIVEX 0x00000600 // DDSCAPS2_CUBEMAP | DDSCAPS2_CUBEMAP_POSITIVEX
#define DDS_CUBEMAP_NEGATIVEX 0x00000a00 // DDSCAPS2_CUBEMAP | DDSCAPS2_CUBEMAP_NEGATIVEX
#define DDS_CUBEMAP_POSITIVEY 0x00001200 // DDSCAPS2_CUBEMAP | DDSCAPS2_CUBEMAP_POSITIVEY
#define DDS_CUBEMAP_NEGATIVEY 0x00002200 // DDSCAPS2_CUBEMAP | DDSCAPS2_CUBEMAP_NEGATIVEY
#define DDS_CUBEMAP_POSITIVEZ 0x00004200 // DDSCAPS2_CUBEMAP | DDSCAPS2_CUBEMAP_POSITIVEZ
#define DDS_CUBEMAP_NEGATIVEZ 0x00008200 // DDSCAPS2_CUBEMAP | DDSCAPS2_CUBEMAP_NEGATIVEZ

#define DDS_CUBEMAP_ALLFACES ( DDS_CUBEMAP_POSITIVEX | DDS_CUBEMAP_NEGATIVEX |\
                               DDS_CUBEMAP_POSITIVEY | DDS_CUBEMAP_NEGATIVEY |\
                               DDS_CUBEMAP_POSITIVEZ | DDS_CUBEMAP_NEGATIVEZ )

#define DDS_CUBEMAP 0x00000200 // DDSCAPS2_CUBEMAP

#define DDS_FLAGS_VOLUME 0x00200000 // DDSCAPS2_VOLUME

enum DDS_RESOURCE_DIMENSION
{
	DDS_RESOURCE_DIMENSION_TEXTURE1D	= 2,
	DDS_RESOURCE_DIMENSION_TEXTURE2D	= 3,
	DDS_RESOURCE_DIMENSION_TEXTURE3D	= 4,
};

enum DDS_RESOURCE_MISC_FLAGS
{
	DDS_RESOURCE_MISC_TEXTURECUBE = 4,
};

typedef DDS_RESOURCE_DIMENSION D3D10_RESOURCE_DIMENSION;

#pragma pack(push,1)
struct DDS_PIXELFORMAT
{
	uint dwSize;
	uint dwFlags;
	uint dwFourCC;
	uint dwRGBBitCount;
	uint dwRBitMask;
	uint dwGBitMask;
	uint dwBBitMask;
	uint dwABitMask;
};

struct DDS_HEADER
{
	uint dwSize;
	uint dwFlags;
	uint dwHeight;
	uint dwWidth;
	uint dwPitchOrLinearSize;
	uint dwDepth;
	uint dwMipMapCount;
	uint dwReserved1[11];
	DDS_PIXELFORMAT ddspf;
	uint dwCaps;
	uint dwCaps2;
	uint dwCaps3;
	uint dwCaps4;
	uint dwReserved2;
};

struct DDS_HEADER_DXT10
{
	DXGI_FORMAT dxgiFormat;
	D3D10_RESOURCE_DIMENSION resourceDimension;
	uint miscFlag;
	uint arraySize;
	uint miscFlags2;
};

#define ISBITMASK( r,g,b,a) ( ddpf.dwRBitMask == r && ddpf.dwGBitMask == g && ddpf.dwBBitMask == b && ddpf.dwABitMask == a)

const DDS_PIXELFORMAT DDSPF_DXT1 = { sizeof(DDS_PIXELFORMAT), DDS_FOURCC, MAKEFOURCC('D','X','T','1'), 0, 0, 0, 0, 0 };
const DDS_PIXELFORMAT DDSPF_DXT2 = { sizeof(DDS_PIXELFORMAT), DDS_FOURCC, MAKEFOURCC('D','X','T','2'), 0, 0, 0, 0, 0 };
const DDS_PIXELFORMAT DDSPF_DXT3 = { sizeof(DDS_PIXELFORMAT), DDS_FOURCC, MAKEFOURCC('D','X','T','3'), 0, 0, 0, 0, 0 };
const DDS_PIXELFORMAT DDSPF_DXT4 = { sizeof(DDS_PIXELFORMAT), DDS_FOURCC, MAKEFOURCC('D','X','T','4'), 0, 0, 0, 0, 0 };
const DDS_PIXELFORMAT DDSPF_DXT5 = { sizeof(DDS_PIXELFORMAT), DDS_FOURCC, MAKEFOURCC('D','X','T','5'), 0, 0, 0, 0, 0 };
const DDS_PIXELFORMAT DDSPF_BC4_UNORM = { sizeof(DDS_PIXELFORMAT), DDS_FOURCC, MAKEFOURCC('B','C','4','U'), 0, 0, 0, 0, 0 };
const DDS_PIXELFORMAT DDSPF_BC4_SNORM = { sizeof(DDS_PIXELFORMAT), DDS_FOURCC, MAKEFOURCC('B','C','4','S'), 0, 0, 0, 0, 0 };
const DDS_PIXELFORMAT DDSPF_BC5_UNORM = { sizeof(DDS_PIXELFORMAT), DDS_FOURCC, MAKEFOURCC('B','C','5','U'), 0, 0, 0, 0, 0 };
const DDS_PIXELFORMAT DDSPF_BC5_SNORM = { sizeof(DDS_PIXELFORMAT), DDS_FOURCC, MAKEFOURCC('B','C','5','S'), 0, 0, 0, 0, 0 };
const DDS_PIXELFORMAT DDSPF_R8G8_B8G8 = { sizeof(DDS_PIXELFORMAT), DDS_FOURCC, MAKEFOURCC('R','G','B','G'), 0, 0, 0, 0, 0 };
const DDS_PIXELFORMAT DDSPF_G8R8_G8B8 = { sizeof(DDS_PIXELFORMAT), DDS_FOURCC, MAKEFOURCC('G','R','G','B'), 0, 0, 0, 0, 0 };
const DDS_PIXELFORMAT DDSPF_YUY2 = { sizeof(DDS_PIXELFORMAT), DDS_FOURCC, MAKEFOURCC('Y','U','Y','2'), 0, 0, 0, 0, 0 };
const DDS_PIXELFORMAT DDSPF_A8R8G8B8 = { sizeof(DDS_PIXELFORMAT), DDS_RGBA, 0, 32, 0x00ff0000, 0x0000ff00, 0x000000ff, 0xff000000 };
const DDS_PIXELFORMAT DDSPF_X8R8G8B8 = { sizeof(DDS_PIXELFORMAT), DDS_RGB,  0, 32, 0x00ff0000, 0x0000ff00, 0x000000ff, 0x00000000 };
const DDS_PIXELFORMAT DDSPF_A8B8G8R8 = { sizeof(DDS_PIXELFORMAT), DDS_RGBA, 0, 32, 0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000 };
const DDS_PIXELFORMAT DDSPF_X8B8G8R8 = { sizeof(DDS_PIXELFORMAT), DDS_RGB,  0, 32, 0x000000ff, 0x0000ff00, 0x00ff0000, 0x00000000 };
const DDS_PIXELFORMAT DDSPF_G16R16 = { sizeof(DDS_PIXELFORMAT), DDS_RGB,  0, 32, 0x0000ffff, 0xffff0000, 0x00000000, 0x00000000 };
const DDS_PIXELFORMAT DDSPF_R5G6B5 = { sizeof(DDS_PIXELFORMAT), DDS_RGB, 0, 16, 0x0000f800, 0x000007e0, 0x0000001f, 0x00000000 };
const DDS_PIXELFORMAT DDSPF_A1R5G5B5 = { sizeof(DDS_PIXELFORMAT), DDS_RGBA, 0, 16, 0x00007c00, 0x000003e0, 0x0000001f, 0x00008000 };
const DDS_PIXELFORMAT DDSPF_A4R4G4B4 = { sizeof(DDS_PIXELFORMAT), DDS_RGBA, 0, 16, 0x00000f00, 0x000000f0, 0x0000000f, 0x0000f000 };
const DDS_PIXELFORMAT DDSPF_R8G8B8 = { sizeof(DDS_PIXELFORMAT), DDS_RGB, 0, 24, 0x00ff0000, 0x0000ff00, 0x000000ff, 0x00000000 };
const DDS_PIXELFORMAT DDSPF_L8 = { sizeof(DDS_PIXELFORMAT), DDS_LUMINANCE, 0,  8, 0xff, 0x00, 0x00, 0x00 };
const DDS_PIXELFORMAT DDSPF_L16 = { sizeof(DDS_PIXELFORMAT), DDS_LUMINANCE, 0, 16, 0xffff, 0x0000, 0x0000, 0x0000 };
const DDS_PIXELFORMAT DDSPF_A8L8 = { sizeof(DDS_PIXELFORMAT), DDS_LUMINANCEA, 0, 16, 0x00ff, 0x0000, 0x0000, 0xff00 };
const DDS_PIXELFORMAT DDSPF_A8 = { sizeof(DDS_PIXELFORMAT), DDS_ALPHA, 0, 8, 0x00, 0x00, 0x00, 0xff };

// D3DFMT_A2R10G10B10/D3DFMT_A2B10G10R10 should be written using DX10 extension to avoid D3DX 10:10:10:2 reversal issue
// This indicates the DDS_HEADER_DXT10 extension is present (the format is in dxgiFormat)
const DDS_PIXELFORMAT DDSPF_DX10 = { sizeof(DDS_PIXELFORMAT), DDS_FOURCC, MAKEFOURCC('D','X','1','0'), 0, 0, 0, 0, 0 };

#pragma pack(pop)

static_assert(sizeof(DDS_HEADER) == 124, "DDS Header size mismatch");
static_assert(sizeof(DDS_HEADER_DXT10) == 20, "DDS DX10 Extended Header size mismatch");

static DXGI_FORMAT GetDXGIFormat(const DDS_PIXELFORMAT& ddpf)
{
	if (ddpf.dwFlags & DDS_RGB)
	{
		// sRGB formats are written using the "DX10" extended header
		switch (ddpf.dwRGBBitCount)
		{
			case 32:
				if (ISBITMASK(0x000000ff,0x0000ff00,0x00ff0000,0xff000000)) return DXGI_FORMAT_R8G8B8A8_UNORM;
				if (ISBITMASK(0x00ff0000,0x0000ff00,0x000000ff,0xff000000)) return DXGI_FORMAT_B8G8R8A8_UNORM;
				if (ISBITMASK(0x00ff0000,0x0000ff00,0x000000ff,0x00000000)) return DXGI_FORMAT_B8G8R8X8_UNORM;
				// No DXGI format maps to ISBITMASK(0x000000ff,0x0000ff00,0x00ff0000,0x00000000) aka D3DFMT_X8B8G8R8
				// Many common DDS reader/writers (including D3DX) swap the
				// the RED/BLUE masks for 10:10:10:2 formats. We assumme
				// below that the 'backwards' header mask is being used since it is most
				// likely written by D3DX. The more robust solution is to use the 'DX10'
				// header extension and specify the DXGI_FORMAT_R10G10B10A2_UNORM format directly
				// For 'correct' writers, this should be 0x000003ff,0x000ffc00,0x3ff00000 for RGB data
				if (ISBITMASK(0x3ff00000,0x000ffc00,0x000003ff,0xc0000000)) return DXGI_FORMAT_R10G10B10A2_UNORM;
				// No DXGI format maps to ISBITMASK(0x000003ff,0x000ffc00,0x3ff00000,0xc0000000) aka D3DFMT_A2R10G10B10
				if (ISBITMASK(0x0000ffff,0xffff0000,0x00000000,0x00000000))	return DXGI_FORMAT_R16G16_UNORM;
				if (ISBITMASK(0xffffffff,0x00000000,0x00000000,0x00000000)) return DXGI_FORMAT_R32_FLOAT; // D3DX writes this out as a FourCC of 114 (Only 32-bit color channel format in D3D9 was R32F)
				break;

			case 24:
			// No 24bpp DXGI formats aka D3DFMT_R8G8B8
			break;

		case 16:
			if (ISBITMASK(0x7c00,0x03e0,0x001f,0x8000)) return DXGI_FORMAT_B5G5R5A1_UNORM;
			if (ISBITMASK(0xf800,0x07e0,0x001f,0x0000)) return DXGI_FORMAT_B5G6R5_UNORM;
			// No DXGI format maps to ISBITMASK(0x7c00,0x03e0,0x001f,0x0000) aka D3DFMT_X1R5G5B5
			if (ISBITMASK(0x0f00,0x00f0,0x000f,0xf000)) return DXGI_FORMAT_B4G4R4A4_UNORM;
			// No DXGI format maps to ISBITMASK(0x0f00,0x00f0,0x000f,0x0000) aka D3DFMT_X4R4G4B4
			// No 3:3:2, 3:3:2:8, or paletted DXGI formats aka D3DFMT_A8R3G3B2, D3DFMT_R3G3B2, D3DFMT_P8, D3DFMT_A8P8, etc.
			break;
		}
	}
	
	else if (ddpf.dwFlags & DDS_LUMINANCE)
	{
		if (8 == ddpf.dwRGBBitCount)
		{
			if (ISBITMASK(0x000000ff,0x00000000,0x00000000,0x00000000)) return DXGI_FORMAT_R8_UNORM; // D3DX10/11 writes this out as DX10 extension
			// No DXGI format maps to ISBITMASK(0x0f,0x00,0x00,0xf0) aka D3DFMT_A4L4
		}

		if (16 == ddpf.dwRGBBitCount)
		{
			if (ISBITMASK(0x0000ffff,0x00000000,0x00000000,0x00000000)) return DXGI_FORMAT_R16_UNORM; // D3DX10/11 writes this out as DX10 extension
			if (ISBITMASK(0x000000ff,0x00000000,0x00000000,0x0000ff00)) return DXGI_FORMAT_R8G8_UNORM; // D3DX10/11 writes this out as DX10 extension
		}
	}

	else if (ddpf.dwFlags & DDS_ALPHA)
	{
		if (8 == ddpf.dwRGBBitCount) return DXGI_FORMAT_A8_UNORM;
	}

	else if (ddpf.dwFlags & DDS_FOURCC)
	{
		if (MAKEFOURCC('D','X','T','1') == ddpf.dwFourCC) return DXGI_FORMAT_BC1_UNORM;
		if (MAKEFOURCC('D','X','T','3') == ddpf.dwFourCC) return DXGI_FORMAT_BC2_UNORM;
		if (MAKEFOURCC('D','X','T','5') == ddpf.dwFourCC) return DXGI_FORMAT_BC3_UNORM;
		// While pre-mulitplied alpha isn't directly supported by the DXGI formats,
		// they are basically the same as these BC formats so they can be mapped
		if (MAKEFOURCC('D','X','T','2') == ddpf.dwFourCC) return DXGI_FORMAT_BC2_UNORM;
		if (MAKEFOURCC('D','X','T','4') == ddpf.dwFourCC) return DXGI_FORMAT_BC3_UNORM;
		if (MAKEFOURCC('A','T','I','1') == ddpf.dwFourCC) return DXGI_FORMAT_BC4_UNORM;
		if (MAKEFOURCC('B','C','4','U') == ddpf.dwFourCC) return DXGI_FORMAT_BC4_UNORM;
		if (MAKEFOURCC('B','C','4','S') == ddpf.dwFourCC) return DXGI_FORMAT_BC4_SNORM;
		if (MAKEFOURCC('A','T','I','2') == ddpf.dwFourCC) return DXGI_FORMAT_BC5_UNORM;
		if (MAKEFOURCC('B','C','5','U') == ddpf.dwFourCC) return DXGI_FORMAT_BC5_UNORM;
		if (MAKEFOURCC('B','C','5','S') == ddpf.dwFourCC) return DXGI_FORMAT_BC5_SNORM;
		// BC6H and BC7 are written using the "DX10" extended header
		if (MAKEFOURCC('R','G','B','G') == ddpf.dwFourCC) return DXGI_FORMAT_R8G8_B8G8_UNORM;
		if (MAKEFOURCC('G','R','G','B') == ddpf.dwFourCC) return DXGI_FORMAT_G8R8_G8B8_UNORM;
		if (MAKEFOURCC('Y','U','Y','2') == ddpf.dwFourCC) return DXGI_FORMAT_YUY2;

	// Check for D3DFORMAT enums being set here
		switch (ddpf.dwFourCC)
		{
			case 36: return DXGI_FORMAT_R16G16B16A16_UNORM; // D3DFMT_A16B16G16R16
			case 110: return DXGI_FORMAT_R16G16B16A16_SNORM; // D3DFMT_Q16W16V16U16
			case 111: return DXGI_FORMAT_R16_FLOAT; // D3DFMT_R16F
			case 112: return DXGI_FORMAT_R16G16_FLOAT; // D3DFMT_G16R16F
			case 113: return DXGI_FORMAT_R16G16B16A16_FLOAT; // D3DFMT_A16B16G16R16F
			case 114: return DXGI_FORMAT_R32_FLOAT; // D3DFMT_R32F
			case 115: return DXGI_FORMAT_R32G32_FLOAT; // D3DFMT_G32R32F
			case 116: return DXGI_FORMAT_R32G32B32A32_FLOAT; // D3DFMT_A32B32G32R32F
		}
	}

	return DXGI_FORMAT_UNKNOWN;
}

static surface::format GetSurfaceFormat(DXGI_FORMAT format)
{
	return (surface::format)format;
}

bool HasD3D10Header(const DDS_HEADER& header)
{
	return (header.ddspf.dwFlags & DDS_FOURCC) && (MAKEFOURCC('D','X','1','0') == header.ddspf.dwFourCC);
}

info get_info_dds(const void* buffer, size_t size)
{
	if (size < (sizeof(uint32_t) + sizeof(DDS_HEADER)) || *(const uint*)buffer != DDS_MAGIC)
		return info();

	auto* h = (const DDS_HEADER*)byte_add(buffer, sizeof(uint));
  if (h->dwSize != sizeof(DDS_HEADER) || h->ddspf.dwSize != sizeof(DDS_PIXELFORMAT))
		return info();

	const bool kIsD3D10 = HasD3D10Header(*h);

	if ((kIsD3D10 && size < (sizeof(DDS_HEADER) + sizeof(uint32_t) + sizeof(DDS_HEADER_DXT10))))
		return info();

	bool cubemap = false;
	DDS_RESOURCE_DIMENSION resourceType;

	info si;
	si.dimensions = int3(h->dwWidth, h->dwHeight, h->dwDepth);
	si.layout = h->dwMipMapCount == 1 ? image : tight;

	if (kIsD3D10)
	{
		auto d3d10ext = (const DDS_HEADER_DXT10*)&h[1];
		si.array_size = d3d10ext->arraySize;
		if (!si.array_size) // invalid according to DDS
			return info();

		switch (d3d10ext->dxgiFormat)
		{
			case DXGI_FORMAT_AI44:
			case DXGI_FORMAT_IA44:
			case DXGI_FORMAT_P8:
			case DXGI_FORMAT_A8P8: return info(); // not supported
			default:
				if (bits(GetSurfaceFormat(d3d10ext->dxgiFormat)) == 0)
					return info();
		}

		si.format = GetSurfaceFormat(d3d10ext->dxgiFormat);

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
		si.format = GetSurfaceFormat(GetDXGIFormat(h->ddspf));
		if (si.format == unknown)
			return info();

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

	return si;
}

static void map_bits(const info& inf, const void* src_dds_buffer, size_t src_dds_size, const_mapped_subresource* subresources, size_t num_subresources)
{
	const void* end = byte_add(src_dds_buffer, src_dds_size);
	const int nMips = num_mips(inf.layout != image, inf.dimensions);
	const int nSlices = inf.array_size;

	size_t index = 0;
	for (int j = 0; j < nSlices; j++)
	{
		for (int i = 0; i < nMips; i++ )
		{
			int subresource = calc_subresource(i, j, 0, nMips, nSlices);
			subresources[subresource] = get_const_mapped_subresource(inf, subresource, 0, src_dds_buffer);
			oCHECK(byte_add(subresources[subresource].data, subresources[subresource].depth_pitch) < end, "end of file");
		}
	}
}

scoped_allocation encode_dds(const texel_buffer& b, const alpha_option& option, const compression& compression)
{
	oTHROW(operation_not_supported, "dds not yet implemented");
}

texel_buffer decode_dds(const void* buffer, size_t size, const alpha_option& option, const layout& layout)
{
	oCHECK(option == alpha_option::preserve, "changing alpha option not supported for dds");
	info si = get_info_dds(buffer, size);
	oCHECK(si.format != unknown, "invalid dds");

	texel_buffer b(si);

	const int nSubresources = num_subresources(si);
	const_mapped_subresource* subresources = (const_mapped_subresource*)default_allocate(sizeof(const_mapped_subresource) * nSubresources, 0);
	finally DeleteInitData([&] { if (subresources) default_deallocate(subresources); });

	auto h = (const DDS_HEADER*)byte_add(buffer, sizeof(DDS_HEADER));
	const void* bits = byte_add(buffer, sizeof(DDS_MAGIC) + sizeof(DDS_HEADER) + (HasD3D10Header(*h) ? sizeof(DDS_HEADER_DXT10) : 0));
	map_bits(si, bits, byte_diff(bits, buffer), subresources, nSubresources);

	for (int i = 0; i < nSubresources; i++)
		b.update_subresource(i, subresources[i]);

	return b;
}

}}
