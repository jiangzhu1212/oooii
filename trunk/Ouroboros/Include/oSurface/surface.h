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
// 2D interleaved visual buffers (images, textures, pictures, bitmaps, whatever 
// you want to call them) are used in many different places in many different 
// ways, so encapsulate the commodity code which they all typically need such 
// as a consistent description, string conversion, size inspection, etc. 

// === DEFINITIONS ===
// SURFACE: A surface is the single buffer that can be bound to a modern 
// hardware texture sampler, or a SW emulator of such hardware. You get one 
// pointer. If you need more than that (YUV formats) then the format is not a 
// surface as defined here. It contains all slices of all mip chains.

// SUBSURFACE: A subsurface is a bit of a hack to future-proof oSurface to 
// support heterogeneous formats, especially relating to YUV. The intent is that
// formatS map to a single memory buffer capable of being bound to 
// a hardware sampler. For specifications of future HW support, this will be 
// true for several formats, so support them uniformly in the interface, but
// expose the subsurface concept to enable portable emulation in the meantime.

// ELEMENT: The 'pixel', but when considering block-compressed formats an 
// element is the 4x4 block of pixels that is the minimal atom used in BC 
// strategies.

// CHANNEL: One scalar value in an element, i.e. the R or the G channel of an 
// RGB element.

// MIP LEVEL: A 2D subregion of a surface that most closely resembles an image
// i.e. it is the 2D subregion that contains color data for a particular usage.

// MIP CHAIN: All mip levels from the full-sized one at mip0 to a 1x1x1 mip 
// level. Each level in between is 1/2 the size of its predecessor.

// ROW PITCH: The number of bytes to get to the next scanline of a given mip 
// level

// ROW SIZE: The number of bytes to copy when copying a scanline. Size != Pitch,
// so be careful to use these concepts separately because there can be 
// significant padding between scanlines.

// SLICE: A mip level arranged in an accessible way. Basically inside the buffer
// of a mip level is an array of regions organized only by memory offset. This
// is different than an array because an array contains mip chains, so is on the
// "outside". Slices are on the "inside" of containment because there can be a 
// slice component at each mip level.

// ARRAY: A series of mip chains (See SLICE for contrast).

// SUBRESOURCE: an index into the repeating pattern of slices that contain mip
// chains. This encapsulates two indices into which is often more convenient
// when passing around the system.

// DEPTH PITCH: The number of bytes to get to the next depth "slice" within a given
// mip level of a slice. (so within a given subresource)

// POSITION: The pixel coordinates from the upper left corner into a mip level 
// where the upper left of a tile begins

// TILE: A 2D subregion of a mip. A tile is the atom on which surface streaming 
// might operate and the size of a tile is often choosen to balance the workload 
// of decompressing or synthesizing a tile vs. how fine that processing gets 
// divided amongst threads.

#pragma once
#ifndef oSurface_h
#define oSurface_h

#include <oBase/byte.h>
#include <oBase/color.h>
#include <oBase/fourcc.h>
#include <oBase/macros.h>
#include <oBase/types.h>
#include <oHLSL/oHLSLMath.h>
#include <functional>

namespace ouro {
	namespace surface {

oDECLARE_SMALL_ENUM(format, uchar)
{
	unknown,
	r32g32b32a32_typeless,
	r32g32b32a32_float,
	r32g32b32a32_uint,
	r32g32b32a32_sint,
	r32g32b32_typeless,
	r32g32b32_float,
	r32g32b32_uint,
	r32g32b32_sint,
	r16g16b16a16_typeless,
	r16g16b16a16_float,
	r16g16b16a16_unorm,
	r16g16b16a16_uint,
	r16g16b16a16_snorm,
	r16g16b16a16_sint,
	r32g32_typeless,
	r32g32_float,
	r32g32_uint,
	r32g32_sint,
	r32g8x24_typeless,
	d32_float_s8x24_uint,
	r32_float_x8x24_typeless,
	x32_typeless_g8x24_uint,
	r10g10b10a2_typeless,
	r10g10b10a2_unorm,
	r10g10b10a2_uint,
	r11g11b10_float,
	r8g8b8a8_typeless,
	r8g8b8a8_unorm,
	r8g8b8a8_unorm_srgb,
	r8g8b8a8_uint,
	r8g8b8a8_snorm,
	r8g8b8a8_sint,
	r16g16_typeless,
	r16g16_float,
	r16g16_unorm,
	r16g16_uint,
	r16g16_snorm,
	r16g16_sint,
	r32_typeless,
	d32_float,
	r32_float,
	r32_uint,
	r32_sint,
	r24g8_typeless,
	d24_unorm_s8_uint,
	r24_unorm_x8_typeless,
	x24_typeless_g8_uint,
	r8g8_typeless,
	r8g8_unorm,
	r8g8_uint,
	r8g8_snorm,
	r8g8_sint,
	r16_typeless,
	r16_float,
	d16_unorm,
	r16_unorm,
	r16_uint,
	r16_snorm,
	r16_sint,
	r8_typeless,
	r8_unorm,
	r8_uint,
	r8_snorm,
	r8_sint,
	a8_unorm,
	r1_unorm,
	r9g9b9e5_sharedexp,
	r8g8_b8g8_unorm,
	g8r8_g8b8_unorm,
	bc1_typeless,
	bc1_unorm,
	bc1_unorm_srgb,
	bc2_typeless,
	bc2_unorm,
	bc2_unorm_srgb,
	bc3_typeless,
	bc3_unorm,
	bc3_unorm_srgb,
	bc4_typeless,
	bc4_unorm,
	bc4_snorm,
	bc5_typeless,
	bc5_unorm,
	bc5_snorm,
	b5g6r5_unorm,
	b5g5r5a1_unorm,
	b8g8r8a8_unorm,
	b8g8r8x8_unorm,
	r10g10b10_xr_bias_a2_unorm,
	b8g8r8a8_typeless,
	b8g8r8a8_unorm_srgb,
	b8g8r8x8_typeless,
	b8g8r8x8_unorm_srgb,
	bc6h_typeless,
	bc6h_uf16,
	bc6h_sf16,
	bc7_typeless,
	bc7_unorm,
	bc7_unorm_srgb,
	ayuv,
	y410,
	y416,
	nv12,
	p010,
	p016,
	opaque_420,
	yuy2,
	y210,
	y216,
	nv11,
	ai44,
	ia44,
	p8,
	a8p8,
	b4g4r4a4_unorm,

	// formats below here are not currently directly loadable to directx.
	r8g8b8_unorm,
	b8g8r8_unorm,

	// multi-surface yuv formats (for emulation ahead of hw and for more robust 
	// compression); also not currently directly loadable to directx.
	y8_u8_v8_unorm, // one r8_unorm per channel
	y8_a8_u8_v8_unorm, // one r8_unorm per channel
	ybc4_ubc4_vbc4_unorm, // one bc4_unorm per channel
	ybc4_abc4_ubc4_vbc4_unorm, // one bc4_unorm per channel
	y8_u8v8_unorm, // y: r8_unorm uv: r8g8_unorm (half-res)
	y8a8_u8v8_unorm, // ay: r8g8_unorm uv: r8g8_unorm (half-res)
	ybc4_uvbc5_unorm, // y: bc4_unorm uv: bc5_unorm (half-res)
	yabc5_uvbc5_unorm, // ay: bc5_unorm uv: bc5_unorm (half-res)
	
	format_count,
};

oDECLARE_SMALL_ENUM(layout, uchar)
{
	// image: no mip chain, so RowSize == RowPitch
	// tight: mips are right after each other, the naive/initial thing a person
	//        might do where next-scanline strides are the same as valid data so
	//        the next-scanline stride also changes with each mip level.
	// below: Step down when moving from Mip0 to Mip1
	//        Step right when moving from Mip1 to Mip2
	//        Step down for all of the other MIPs
	//        RowPitch is generally the next-scanline stride of Mip0, except
	//        when there are alignment issues e.g. with Mip0 width of 1,
	//        or Mip0 width of 4 or less for block compressed formats.:
	// right: Step right when moving from Mip0 to Mip1
	//        Step down for all of the other MIPs.
	//        RowPitch is generZy the next-scanline stride of Mip0 plus the 
	//        stride of mip1.
	// Note that for below and right the lowest Mip can be below the bottom
	// line of Mip1 and Mip0 respectively for alignment reasons and/or block 
	// compressed formats, see http://intellinuxgraphics.org/VOL_1_graphics_core.pdf
	// +---------+ +---------+ +---------+ +---------+----+
	// |         | |         | |         | |         |    |
	// |  Image  | |  Tight  | |  Below  | |  Right  |    |
	// |         | |         | |         | |         +--+-+
	// |         | |         | |         | |         |  |  
	// |         | |         | |         | |         +-++  
	// +---------+ +----+----+ +----+--+-+ +---------+++  
	//             |    |      |    |  |             ++
	//             |    |      |    +-++                  
	//             +--+-+      +----+++                    
	//             |  |             ++                    
	//             +-++                                 
	//             +++                                  
	//             ++

	image,
	tight,
	below,
	right,

};

struct info
{
	info()
		: layout(image)
		, format(unknown)
		, dimensions(0, 0, 0)
		, array_size(0)
	{}

	inline bool operator==(const info& _That) const
	{
		return layout == _That.layout
			&& format == _That.format
			&& dimensions.x == _That.dimensions.x
			&& dimensions.y == _That.dimensions.y
			&& dimensions.z == _That.dimensions.z
			&& array_size == _That.array_size;
	}

	enum layout layout;
	enum format format;
	int3 dimensions;
	int array_size;
};

struct box
{
	box()
		: left(0), right(0)
		, top(0), bottom(1)
		, front(0), back(1)
	{}

	box(uint _Left, uint _Right, uint _Top, uint _Bottom, uint _Front = 0, uint _Back = 1)
		: left(_Left), right(_Right)
		, top(_Top), bottom(_Bottom)
		, front(_Front), back(_Back)
	{}

	// Convenience when a box is required, but the data is 1-dimensional.
	box(uint _Width)
		: left(0), right(_Width)
		, top(0), bottom(1)
		, front(0), back(1)
	{}

	inline bool empty() const { return left == right || top == bottom || front == back; }
	inline uint width() const { return right - left; }
	inline uint height() const { return bottom - top; }

	uint left;
	uint right;
	uint top;
	uint bottom;
	uint front;
	uint back;
};

struct subresource_info
{
	subresource_info()
		: dimensions(0, 0, 0)
		, format(unknown)
		, mip_level(0)
		, array_slice(0)
		, subsurface(0)
	{}

	int3 dimensions;
	enum format format;
	int mip_level;
	int array_slice;
	int subsurface;
};

struct mapped_subresource
{
	mapped_subresource() : data(nullptr), row_pitch(0), depth_pitch(0) {}

	void* data;
	uint row_pitch;
	uint depth_pitch;
};

struct const_mapped_subresource
{
	const_mapped_subresource() : data(nullptr), row_pitch(0), depth_pitch(0) {}
	const_mapped_subresource(const mapped_subresource& _That) { operator=(_That); }
	const_mapped_subresource(const const_mapped_subresource& _That) { operator=(_That); }

	const const_mapped_subresource& operator=(const mapped_subresource& _That)
	{
		data = _That.data;
		row_pitch = _That.row_pitch;
		depth_pitch = _That.depth_pitch;
		return *this;
	}

	const const_mapped_subresource& operator=(const const_mapped_subresource& _That)
	{
		data = _That.data;
		row_pitch = _That.row_pitch;
		depth_pitch = _That.depth_pitch;
		return *this;
	}

	const void* data;
	uint row_pitch;
	uint depth_pitch;
};

struct tile_info
{
	int2 position;
	int2 dimensions;
	int mip_level;
	int array_slice;
};

static const int max_num_subsurfaces = 4;

// _____________________________________________________________________________
// format introspection

// Returns true if the specified format is a block-compressed format.
bool is_block_compressed(format _Format);

// Returns true if the specified format is one typically used to write Z-buffer 
// depth information.
bool is_depth(format _Format);

// Returns true of the specified format includes RGB and A or S. SRGB formats
// will result in true from this function.
bool has_alpha(format _Format);

// Returns true if the specified format is normalized between 0.0f and 1.0f
bool is_unorm(format _Format);

// Returns true if the specified format is organized such that its elements are
// not interleaved in memory.
bool is_planar(format _Format);

// Returns true if the specified format is in YUV space. YUV can apply to both
// single-surface and multi-surface YUV formats.
bool is_yuv(format _Format);

// Returns the number of separate channels used for a pixel. For example RGB has  
// 3 channels, XRGB has 4, RGBA has 4.
int num_channels(format _Format);

// Emulated YUV formats can have more than one surface format associated with 
// them, such as r8_unorm for surface index 0 and r8g8_unorm for surface index 
// 0. For most formats this will return 0 meaning the format is for one and only 
// one surface.
int num_subformats(format _Format);

// Returns the offset to MipLevel that the specified sub-surface uses. For some 
// YUV formats, the second plane (UV) is half the size of the first plane (AY). 
// For most formats this will return 0 and for those down-sampled planes, this 
// will return a value to add to the mip level to evaluate/base size off of.
int subsample_bias(format _Format, int _SubsurfaceIndex);

// Returns the number of bytes required to store the smallest atom of a surface. 
// For single-bit image formats, this will return 1. For tiled formats this will 
// return the byte size of 1 tile.
uint element_size(format _Format, int _SubsurfaceIndex = 0);

// Returns the minimum dimensions the format supports.  For most formats this is 
// 1,1 but for block formats it can be something else.
int2 min_dimensions(format _Format);

// Get number of bits per format. This includes any X bits as described in the 
// format enum.
int bits(format _Format);

// Returns the number of bits per channel, either R,G,B,A or Y,U,V,A.
void channel_bits(format _Format, int* _pNBitsR, int* _pNBitsG, int* _pNBitsB, int* _pNBitsA);

// Returns the surface format of the specified format's nth subformat. For RGBA-
// based formats, this will return the same value as _Format for index 0 and 
// surface::unknown for any other plane. For YUV formats this will return the
// format needed for each oSURFACE used to emulate the YUV format.
format subformat(format _Format, int _SubsurfaceIndex);

// Most formats for data types have a fourcc that is unique at least amongst 
// other formats. This is useful for serialization where you want 
// something a bit more version-stable than an enum value.
fourcc to_fourcc(format _Format);

// Convert an fourcc as returned from oSurfaceFormatToFourCC to its associated 
// format.
format from_fourcc(fourcc _FourCC);

// Given a surface format, determine the NV12 format that comes closest to it.
format closest_nv12(format _Format);

// _____________________________________________________________________________
// Mip Level (1 2D plane/slice, a simple image) introspection

// Returns the number of mipmaps generated from the specified dimensions down to 
// a 1x1 mip level.
int num_mips(bool _HasMips, const int3& _Mip0Dimensions);
inline int num_mips(bool _HasMips, const int2& _Mip0Dimensions) { return num_mips(_HasMips, int3(_Mip0Dimensions, 1)); }
inline int num_mips(layout _Layout, const int3& _Mip0Dimensions) { return num_mips(_Layout != image, _Mip0Dimensions); }
inline int num_mips(layout _Layout, const int2& _Mip0Dimensions) { return num_mips(_Layout != image, int3(_Mip0Dimensions, 1)); }
inline int num_mips(const info& _SurfaceInfo) { return num_mips(_SurfaceInfo.layout, _SurfaceInfo.dimensions); }

// Returns the width, height, or depth dimension of the specified mip level
// given mip0's dimension. This appropriately pads BC formats and conforms to 
// hardware-expected sizes all the way down to 1x1. Because oSurface is meant to
// be used with hardware-compatible surfaces, all dimensions must be a power 
// of 2.
int dimension(format _Format, int _Mip0Dimension, int _MipLevel = 0, int _SubsurfaceIndex = 0);
int2 dimensions(format _Format, const int2& _Mip0Dimensions, int _MipLevel = 0, int _SubsurfaceIndex = 0);
int3 dimensions(format _Format, const int3& _Mip0Dimensions, int _MipLevel = 0, int _SubsurfaceIndex = 0);

// Calculate mip dimensions for non-power-of-2 textures using the D3D/OGL2.0
// floor convention: http://www.opengl.org/registry/specs/ARB/texture_non_power_of_two.txt
int dimension_npot(format _Format, int _Mip0Dimension, int _MipLevel = 0, int _SubsurfaceIndex = 0);
int2 dimensions_npot(format _Format, const int2& _Mip0Dimensions, int _MipLevel = 0, int _SubsurfaceIndex = 0);
int3 dimensions_npot(format _Format, const int3& _Mip0Dimensions, int _MipLevel = 0, int _SubsurfaceIndex = 0);

// Returns the size in bytes for one row of valid data for the specified mip 
// level. This does a bit more than a simple multiply because it takes into 
// consideration block compressed formats. This IS NOT the calculation to use to
// get to the next scanline of data unless it can be guaranteed that there is
// no padding nor is the valid data merely a subregion of a larger 2D plane, but 
// this IS the calculate for the number of bytes in the current scanline.
uint row_size(format _Format, int _MipWidth, int _SubsurfaceIndex = 0);
inline uint row_size(format _Format, const int2& _MipDimensions, int _SubsurfaceIndex = 0) { return row_size(_Format, _MipDimensions.x, _SubsurfaceIndex); }
inline uint row_size(format _Format, const int3& _MipDimensions, int _SubsurfaceIndex = 0) { return row_size(_Format, _MipDimensions.x, _SubsurfaceIndex); }

// Returns the number of bytes to increment to get to the next row of a 2D 
// surface. Do not read or write using this value as padding may be used for 
// other reasons such as internal data or to store other elements/mip levels.
uint row_pitch(const info& _SurfaceInfo, int _MipLevel = 0, int _SubsurfaceIndex = 0);

// Returns the number of bytes to increment to get to the next slice of a 3D
// surface. Its calculated as RowPitch * number of rows at the requested
// mip level.
uint depth_pitch(const info& _SurfaceInfo, int _MipLevel = 0, int _SubsurfaceIndex = 0);

// Returns the number of columns (number of elements in a row) in a mip level 
// with the specified width in pixels. Block compressed formats will return 1/4
// the columns since their atomic element - the block - is 4x4 pixels.
int num_columns(format _Format, int _MipWidth, int _SubsurfaceIndex = 0);
inline int num_columns(format _Format, const int2& _MipDimensions, int _SubsurfaceIndex = 0) { return num_columns(_Format, _MipDimensions.x, _SubsurfaceIndex); }

// Returns the number of rows in a mip level with the specified height in 
// pixels. Block compressed formats have 1/4 the rows since their pitch includes 
// 4 rows at a time.
int num_rows(format _Format, int _MipHeight, int _SubsurfaceIndex = 0);
inline int num_rows(format _Format, const int2& _MipDimensions, int _SubsurfaceIndex = 0) { return num_rows(_Format, _MipDimensions.y, _SubsurfaceIndex); }
inline int num_rows(format _Format, const int3& _MipDimensions, int _SubsurfaceIndex = 0) { return num_rows(_Format, _MipDimensions.y, _SubsurfaceIndex) * _MipDimensions.z; }

// Returns the number of columns (x) and rows (y) in one call. This is different
// than dimensions, which is pixels. This is in elements, which can be 4x4 pixel 
// blocks for block compressed formats
inline int2 num_columns_and_rows(format _Format, const int2& _MipDimensions, int _SubsurfaceIndex = 0) { return int2(num_columns(_Format, _MipDimensions, _SubsurfaceIndex), num_rows(_Format, _MipDimensions, _SubsurfaceIndex)); }

// Returns the RowSize and NumRows as one operation in an int2
inline int2 byte_dimensions(format _Format, const int2& _MipDimensions, int _SubsurfaceIndex = 0) { return int2(row_size(_Format, _MipDimensions, _SubsurfaceIndex), num_rows(_Format, _MipDimensions, _SubsurfaceIndex)); }
inline int2 byte_dimensions(format _Format, const int3& _MipDimensions, int _SubsurfaceIndex = 0) { return int2(row_size(_Format, _MipDimensions, _SubsurfaceIndex), num_rows(_Format, _MipDimensions, _SubsurfaceIndex)); }

// Returns the size in bytes for the specified mip level. CAREFUL, this does not 
// always imply the size that should be passed to memcpy. Ensure you have 
// enforced that there is no scanline padding or that data using these 
// calculations aren't a subregion in a larger surface.
int mip_size(format _Format, const int2& _MipDimensions, int _SubsurfaceIndex = 0);
inline int mip_size(format _Format, const int3& _MipDimensions, int _SubsurfaceIndex = 0) { return mip_size(_Format, _MipDimensions.xy(), _SubsurfaceIndex) * _MipDimensions.z; }

// Returns the number of bytes from the start of Mip0 where the upper left
// corner of the specified mip level's data begins. The dimensions must always
// be specified as the mip0 dimensions since this is a cumulative offset.
int offset(const info& _SurfaceInfo, int _MipLevel = 0, int _SubsurfaceIndex = 0);

// _____________________________________________________________________________
// Slice/surface introspection

// Returns the size in bytes for one slice for the described mip chain.
// That is, in an array of textures, each texture is a full mip chain, and this
// pitch is the number of bytes for the total mip chain of one of those 
// textures. This is the same as calculating the size of a mip page as described 
// by layout. For 3d textures, make sure that array_size is set to 1 and use 
// depth to supply the size in the 3rd dimension.
uint slice_pitch(const info& _SurfaceInfo, int _SubsurfaceIndex = 0);

// Calculates the size for a total buffer of 1d/2d/3d/cube textures by summing 
// the various mip chains, then multiplying it by the number of slices. 
// Optionally you can supply a subsurface index to limit the size calculation to 
// that subsurface only. 3D textures need to have array_size set to 1. All other 
// texture types need to have dimensions.z set to 1. For more clarity on the 
// difference between array_size and depth see:
// http://www.cs.umbc.edu/~olano/s2006c03/ch02.pdf where it is clear that when a 
// first mip level for a 3d texture is (4,3,5) that the next mip level is 
// (2,1,2) and the next (1,1,1) whereas array_size set to 5 would mean: 5*(4,3), 
// 5*(2,1), 5*(1,1).
uint total_size(const info& _SurfaceInfo, int _SubsurfaceIndex = -1);

// Calculates the dimensions you would need for an image to fit this surface
// natively and in its entirety.
int2 dimensions(const info& _SurfaceInfo, int _SubsurfaceIndex = 0);

// Calculates the dimensions you would need for an image to fit a slice of this 
// surface natively and in its entirety.
int2 slice_dimensions(const info& _SurfaceInfo, int _SubsurfaceIndex = 0);

// _____________________________________________________________________________
// Subresource API

// To simplify the need for much of the above API, interfaces should be 
// developed that take a subresource id and internally use these API to
// translate that into the proper byte locations and sizes. 
inline int calc_subresource(int _MipLevel, int _ArraySliceIndex, int _SubsurfaceIndex, int _NumMips, int _NumArraySlices) { int nMips = max(1, _NumMips); return _MipLevel + (_ArraySliceIndex * nMips) + (_SubsurfaceIndex * nMips * max(1, _NumArraySlices)); }

// Converts _Subresource back to its mip level and slice as long as the num mips
// in the mip chain is specified.
inline void unpack_subresource(int _Subresource, int _NumMips, int _NumArraySlices, int* _pMipLevel, int* _pArraySliceIndex, int* _pSubsurfaceIndex) { int nMips = max(1, _NumMips); int as = max(1, _NumArraySlices); *_pMipLevel = _Subresource % nMips; *_pArraySliceIndex = (_Subresource / nMips) % as; *_pSubsurfaceIndex = _Subresource / (nMips * as); }

// Returns the number of all subresources described by the info.
inline int num_subresources(const info& _SurfaceInfo) { return num_mips(_SurfaceInfo.layout, _SurfaceInfo.dimensions) * max(1, _SurfaceInfo.array_size); }

// Returns the info for a given subresource.
subresource_info subresource(const info& _SurfaceInfo, int _Subresource);

// Returns a surface::info for the nth subsurfaces miplevel. Optionally also 
// calculate the byte dimensions for the specified subresource. i.e. the width 
// in bytes to copy (different than pitch) and number of rows of that width to 
// copy.
info subsurface(const info& _SurfaceInfo, int _SubsurfaceIndex, int _MipLevel, int2* _pByteDimensions = nullptr);

// Returns the number of bytes required to contain the subresource (mip level
// from a particular slice) when what you got is a subresource as returned from
// calc_subresource().
uint subresource_size(const subresource_info& _SubresourceInfo);
inline uint subresource_size(const info& _SurfaceInfo, int _Subresource) { return subresource_size(subresource(_SurfaceInfo, _Subresource)); }

// Returns the offset from a base pointer to the start of the specified 
// subresource.
uint subresource_offset(const info& _SurfaceInfo, int _Subresource, int _DepthIndex = 0);

// Returns the RowSize (width in bytes of one row) in x and the NumRows (number 
// of rows to copy) in y. Remember that this calculation is necessary 
// particularly with block compressed formats where the number of data rows is 
// different than the height of the surface in pixels/texels. The results of 
// this function are particularly useful when using oMemcpy2D for the 
// _SourceRowSize and _NumRows parameters.
inline uint2 byte_dimensions(const subresource_info& _SubresourceInfo) { return uint2(row_size(_SubresourceInfo.format, _SubresourceInfo.dimensions.xy(), _SubresourceInfo.subsurface), num_rows(_SubresourceInfo.format, _SubresourceInfo.dimensions.xy(), _SubresourceInfo.subsurface)); }
inline uint2 byte_dimensions(const info& _SurfaceInfo, int _Subresource) { return byte_dimensions(subresource(_SurfaceInfo, _Subresource)); }

// Given the surface info, a subresource into the surface, and the base pointer
// to the surface, this will return a populated mapped_subresource or an 
// const_mapped_subresource. _pByteDimensions is optional and if valid will 
// return the byte dimensions for the specified subresource.
const_mapped_subresource get_const_mapped_subresource(const info& _SurfaceInfo, int _Subresource, int _DepthIndex, const void* _pSurface, int2* _pByteDimensions = nullptr);
mapped_subresource get_mapped_subresource(const info& _SurfaceInfo, int _Subresource, int _DepthIndex, void* _pSurface, int2* _pByteDimensions = nullptr);

// Copies a source buffer into the specified subresource of the surface 
// described by a base pointer and an info
void update(const info& _SurfaceInfo, int _Subresource, void* _pDestinationSurface, const void* _pSource, size_t _SourceRowPitch, bool _FlipVertical);

// Copies from a surface subresource to a specified destination buffer.
void copy(const info& _SurfaceInfo, int _Subresource, const void* _pSourceSurface, void* _pDestination, size_t _DestinationRowPitch, bool _FlipVertical);
void copy(const info& _SurfaceInfo, const const_mapped_subresource& _Source, mapped_subresource* _Destination, bool _FlipVertical);
void copy(const subresource_info& _SubresourceInfo, const const_mapped_subresource& _Source, mapped_subresource* _Destination, bool _FlipVertical);

// For 3d textures a mapped subresource contains all depth slices at that mip level,
// this function will output the data pointer adjusted for the requested depth index.
inline void* depth_index_offset(const mapped_subresource& _MappedSubresource, int _DepthIndex) { return byte_add(_MappedSubresource.data, _MappedSubresource.depth_pitch, _DepthIndex); }

// Single-pixel get/put API. Only use this for debug/non-performant cases. This
// currently only supports common r rgb and rgba cases.
void put(const subresource_info& _SubresourceInfo, mapped_subresource* _Destination, const int2& _Coordinate, color _Color);
color get(const subresource_info& _SubresourceInfo, const const_mapped_subresource& _Source, const int2& _Coordinate);

// _____________________________________________________________________________
// Tile API

// Returns the number of tiles required to hold the full resolution specified by 
// _MipDimensions. If tile dimensions don't divide perfectly into the mip 
// dimensions the extra tile required to store the difference is included in 
// this calculation.
int2 dimensions_in_tiles(const int2& _MipDimensions, const int2& _TileDimensions);
int3 dimensions_in_tiles(const int3& _MipDimensions, const int2& _TileDimensions);

// Returns the number of tiles required to store all data for a mip of the 
// specified dimensions.
int num_tiles(const int2& _MipDimensions, const int2& _TileDimensions);
int num_tiles(const int3& _MipDimensions, const int2& _TileDimensions);

// Returns the number of tiles required to store all data for all mips in a 
// slice.
int num_slice_tiles(const info& _SurfaceInfo, const int2& _TileDimensions);

// Given the specified position, return the tile ID. This also will update 
// _TileDesc.Position start position that is aligned to the tile, which might be 
// different if a less rigorous value is chosen for _Position. Tile IDs are 
// calculated from the top-left and count up left-to-right, then top-to-bottom,
// then to the next smaller mip as described by surface::layout, then continues
// counting into the top level mip of the next slice and so on.
int calc_tile_id(const info& _SurfaceInfo, const tile_info& _TileInfo, int2* _pPosition);

// Returns more detailed info from a tile ID
// from upper left) for the tile specified by _TileID.
tile_info get_tile(const info& _SurfaceInfo, const int2& _TileDimensions, int _TileID);

// A suggestion on whether or not you should use large memory pages. A little 
// bit arbitrary, but initially always returns false if your image is smaller 
// than half a large page, to ensure your not wasting too much memory. After 
// that, will return true if each tile won't copy at least half a small page 
// worth before encountering a tlb miss.
// TODO: currently have to pass in the page sizes which is platform code, maybe 
// thats fine, or should this go somewhere else?
bool use_large_pages(const info& _SurfaceInfo, const int2& _TileDimensions, int _SmallPageSize, int _LargePageSize);

// _____________________________________________________________________________
// Heavier-weight util functions that might need to be broken out into their own
// headers, but there's not enough to do that yet...

// Calls the specified function on each pixel.
void enumerate_pixels(const info& _SurfaceInfo
	, const const_mapped_subresource& _MappedSubresource
	, const std::function<void(const void* _pPixel)>& _Enumerator);

void enumerate_pixels(const info& _SurfaceInfo
	, mapped_subresource& _MappedSubresource
	, const std::function<void(void* _pPixel)>& _Enumerator);

// Calls the specified function on each pixel of two same-formatted surfaces.
void enumerate_pixels(const info& _SurfaceInfo
	, const const_mapped_subresource& _MappedSubresource1
	, const const_mapped_subresource& _MappedSubresource2
	, const std::function<void(const void* oRESTRICT _pPixel1, const void* oRESTRICT _pPixel2)>& _Enumerator);

// Calls the specified function on each pixel of two same-formatted surfaces.
// and writes to a 3rd other-format surface.
void enumerate_pixels(const info& _SurfaceInfoInput
	, const const_mapped_subresource& _MappedSubresourceInput1
	, const const_mapped_subresource& _MappedSubresourceInput2
	, const info& _SurfaceInfoOutput
	, mapped_subresource& _MappedSubresourceOutput
	, const std::function<void(const void* oRESTRICT _pPixel1, const void* oRESTRICT _pPixel2, void* oRESTRICT _pPixelOut)>& _Enumerator);

// Returns the root mean square of the difference between the two surfaces.
float calc_rms(const info& _SurfaceInfo
	, const const_mapped_subresource& _MappedSubresource1
	, const const_mapped_subresource& _MappedSubresource2);

// Fills the output surface with abs(Input1 - Input2) for each pixel and returns 
// the root mean square. Currently the only supportedoutput image is r8_unorm.
// Inputs can be r8_unorm, b8g8r8_unorm, b8g8r8a8_unorm
float calc_rms(const info& _SurfaceInfoInput
	, const const_mapped_subresource& _MappedSubresourceInput1
	, const const_mapped_subresource& _MappedSubresourceInput2
	, const info& _SurfaceInfoOutput
	, mapped_subresource& _MappedSubresourceOutput);

// Fills the specified array with the count of pixels at each luminance value.
// (lum [0,1] mapped to [0,255] or [0,256]).
void histogram8(const info& _SurfaceInfo, const const_mapped_subresource& _MappedSubresource, unsigned int _Histogram[256]);
void histogram16(const info& _SurfaceInfo, const const_mapped_subresource& _MappedSubresource, unsigned int _Histogram[65536]);

	} // namespace surface
} // namespace ouro

#endif
