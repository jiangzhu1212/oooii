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

// === BC SUMMARY ===
// Format  Precision    Bytes/Pixel  Recommended Use
// BC1     R5G6B5 + A1  0.5          Color w/ 1bit alpha, low-precision normal maps
// BC2     R5G6B5 + A4  1            ?
// BC3     R5G6B5 + A8  1            Color w/ full alpha or packed grayscale
// BC4     R8           0.5          Height/gloss maps, font atlases, any grayscale
// BC5     R8G8         1            Tangent-space normal maps or packed grayscales
// BC6     R16G16B16    1            HDR images 
// BC7     R8G8B8(A8)   1            High-quality color maps (w/ alpha)

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

enum class format : uchar
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
	r8g8b8_unorm_srgb,
	r8g8b8x8_unorm,
	r8g8b8x8_unorm_srgb,
	b8g8r8_unorm,
	b8g8r8_unorm_srgb,
	a8b8g8r8_unorm,
	a8b8g8r8_unorm_srgb,
	x8b8g8r8_unorm,
	x8b8g8r8_unorm_srgb,

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
	
	count,
};

enum class mip_layout : uchar
{
	// http://www.x.org/docs/intel/VOL_1_graphics_core.pdf
	// none : no mip chain, so row size == row pitch
	// tight: mips are right after each other: row size == row pitch and 
	//        the end of one mip is the start of the next.
	// below: Step down when moving from mip0 to mip1
	//        Step right when moving from mip1 to mip2
	//        Step down for all of the other mips
	//        row pitch is generally the next-scanline stride of mip0, 
	//        except for alignment issues e.g. with mip0 width of 1,
	//        or mip0 width <= 4 for block compressed formats.
	// right: Step right when moving from mip0 to mip1
	//        Step down for all of the other mips.
	//        row pitch is generally the next-scanline stride of mip0 
	//        plus the stride of mip1.
	// Note that for below and right the lowest mip can be below the bottom
	// line of mip1 and mip0 respectively for alignment reasons and/or block 
	// compressed formats.
	// None always has mip0, others have all mips down to 1x1.
	// +---------+ +---------+ +---------+ +---------+----+
	// |         | |         | |         | |         |    |
	// |  none   | |  Tight  | |  Below  | |  Right  |    |
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

	none,
	tight,
	below,
	right,
};

enum class semantic : uchar
{
	custom,
	color,
	tangent_normal,
	world_normal,
	specular,
	diffuse,
	height,
	noise,
	intensity,

	custom1d,
	color_correction1d,

	custom3d,
	color_correction3d,

	customcube,
	colorcube,

	count,

	first1d = custom1d,
	last1d = color_correction1d,
	first2d = custom,
	last2d = intensity,
	first3d = custom3d,
	last3d = color_correction3d,
	firstcube = customcube,
	lastcube = colorcube,
};

enum class cube_face : uchar
{
	posx,
	negx,
	posy,
	negy,
	posz,
	negz,

	count,
};

enum class copy_option : uchar
{
	none,
	flip_vertically,
};

struct bit_size
{
	uchar r;
	uchar g;
	uchar b;
	uchar a;
};

struct info
{
	info()
		: dimensions(0, 0, 0)
		, array_size(0)
		, format(format::unknown)
		, semantic(semantic::custom)
		, mip_layout(mip_layout::none)
	{}

	inline bool operator==(const info& that) const
	{
		return dimensions.x == that.dimensions.x
				&& dimensions.y == that.dimensions.y
				&& dimensions.z == that.dimensions.z
				&& array_size == that.array_size
				&& format == that.format
				&& semantic == that.semantic
				&& mip_layout == that.mip_layout;
	}

	inline bool is_1d() const { return semantic >= semantic::first1d && semantic <= semantic::last1d; }
	inline bool is_2d() const { return semantic >= semantic::first2d && semantic <= semantic::last2d; }
	inline bool is_3d() const { return semantic >= semantic::first3d && semantic <= semantic::last3d; }
	inline bool is_cube() const { return semantic >= semantic::firstcube && semantic <= semantic::lastcube; }
	inline bool is_array() const { return array_size != 0; }
	inline bool mips() const { return mip_layout != mip_layout::none; }

	uint3 dimensions;
	uint array_size;
	format format;
	semantic semantic;
	mip_layout mip_layout;
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
		, mip_level(0)
		, array_slice(0)
		, subsurface(0)
		, format(format::unknown)
	{}

	uint3 dimensions;
	uint mip_level;
	uint array_slice;
	uint subsurface;
	format format;
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
	const_mapped_subresource(const mapped_subresource& that) { operator=(that); }
	const_mapped_subresource(const const_mapped_subresource& that) { operator=(that); }

	const const_mapped_subresource& operator=(const mapped_subresource& that)
	{
		data = that.data;
		row_pitch = that.row_pitch;
		depth_pitch = that.depth_pitch;
		return *this;
	}

	const const_mapped_subresource& operator=(const const_mapped_subresource& that)
	{
		data = that.data;
		row_pitch = that.row_pitch;
		depth_pitch = that.depth_pitch;
		return *this;
	}

	const void* data;
	uint row_pitch;
	uint depth_pitch;
};

struct tile_info
{
	uint2 position;
	uint2 dimensions;
	uint mip_level;
	uint array_slice;
};

static const uint max_num_subsurfaces = 4;

// _____________________________________________________________________________
// format introspection

inline bool is_texture(const format& f) { return (int)f <= (int)format::b4g4r4a4_unorm; }

// true if a block-compressed format
bool is_block_compressed(const format& f);

// true if typically used to write Z-buffer/depth information
bool is_depth(const format& f);

// true if has alpha - rgbx types do not have alpha
bool has_alpha(const format& f);

// true if normalized between 0.0f and 1.0f
bool is_unorm(const format& f);

// true if srgb format
bool is_srgb(const format& f);

// true if channels of a pixel are not interleaved
bool is_planar(const format& f);

// true if in YUV space
bool is_yuv(const format& f);

// given a 3-component type return the equivalent 4-component type
// that adds alpha. If there is not an equivalent type this will 
// throw.
format add_alpha(const format& f);

// given a 4-component type return the equivalent 3-component type
// not all types have 3-component types defined at this time in which
// case this will throw. If there is not an equivalent 3-component
// type because it does not make sense, then this returns f.
format strip_alphaorx(const format& f);

// returns the number of separate channels used for a pixel
uint num_channels(const format& f);

// returns the number of separate plane's for planar formats
uint num_subformats(const format& f);

// returns the offset that should be applied to the main surface's mip level.
// Many planar formats store some planes at a different resolution than the
// first.
uint subsample_bias(const format& f, uint subsurface);

// returns byte size of the smallest element the format uses either a pixel, 
// a byte (for 1-bit formats) or a block size for block compressed types
uint element_size(const format& f, uint subsurface = 0);

// for most formats this is 1,1 but block formats and yuv may differ
uint2 min_dimensions(const format& f);

// returns number of bits for an element including X bits
uint bits(const format& f);

// returns number of bits per channel: either r,g,b,a or y,u,v,a
bit_size channel_bits(const format& f);

// returns the surface format of the plane of planar format or unknown for non-planar formats
format subformat(const format& f, uint subsurface);

// get the typical fourcc code associated with the format
fourcc to_fourcc(const format& f);

// converts a fourcc returned from to_fourcc to a format
format from_fourcc(const fourcc& fcc);

// given a surface format determine the NV12 format that comes closest to it
format closest_nv12(const format& f);

// _____________________________________________________________________________
// Mip Level (1 2D plane/slice, a simple image) introspection

// returns number of mipmaps generated from the top dimension down to smallest or
// 0 if mips is false
uint num_mips(bool mips, const uint3& mip0dimensions);
inline uint num_mips(bool mips, const uint2& mip0dimensions) { return num_mips(mips, uint3(mip0dimensions, 1)); }
inline uint num_mips(const mip_layout& layout, const uint3& mip0dimensions) { return num_mips(layout != mip_layout::none, mip0dimensions); }
inline uint num_mips(const mip_layout& layout, const uint2& mip0dimensions) { return num_mips(layout != mip_layout::none, uint3(mip0dimensions, 1)); }
inline uint num_mips(const info& inf) { return num_mips(inf.mip_layout, inf.dimensions); }

// returns width, height, or depth dimension for a specific mip level given mip0's dimension
// This appropriately pads BC formats and conforms to hardware-expected sizes all the way 
// down to 1x1. Because surface is meant to be used with hardware-compatible surfaces, 
// all dimensions must be a power of 2.
uint dimension(const format& f, uint mip0dimensions, uint miplevel = 0, uint subsurface = 0);
uint2 dimensions(const format& f, const uint2& mip0dimensions, uint miplevel = 0, uint subsurface = 0);
uint3 dimensions(const format& f, const uint3& mip0dimensions, uint miplevel = 0, uint subsurface = 0);

// calculate mip dimensions for non-power-of-2 textures using the D3D/OGL2.0
// floor convention: http://www.opengl.org/registry/specs/ARB/texture_non_power_of_two.txt
uint dimension_npot(const format& f, uint mip0dimensions, uint miplevel = 0, uint subsurface = 0);
uint2 dimensions_npot(const format& f, const uint2& mip0dimensions, uint miplevel = 0, uint subsurface = 0);
uint3 dimensions_npot(const format& f, const uint3& mip0dimensions, uint miplevel = 0, uint subsurface = 0);

// returns the number of bytes for one row of valid data excluding padding and 
// considering non-single pixel sizes such as block-compressed sizes
uint row_size(const format& f, uint mipwidth, uint subsurface = 0);
inline uint row_size(const format& f, const uint2& mipdimensions, uint subsurface = 0) { return row_size(f, mipdimensions.x, subsurface); }
inline uint row_size(const format& f, const uint3& mipdimensions, uint subsurface = 0) { return row_size(f, mipdimensions.x, subsurface); }

// returns the number of bytes to increment to get to the next row of a 2d 
// surface including padding and considerion non-single pixel sizes such as 
// block-compressed sizes
uint row_pitch(const info& inf, uint miplevel = 0, uint subsurface = 0);

// returns the number of bytes to increment to get to the next slice of a 3d
// surface
uint depth_pitch(const info& inf, uint miplevel = 0, uint subsurface = 0);

// returns number of columns (number of elements in a row) in a mip level 
// with the specified width in pixels. Block compressed formats will return
// 1/4 the columns since their atomic element - the block - is 4x4 pixels.
uint num_columns(const format& f, uint mipwidth, uint subsurface = 0);
inline uint num_columns(const format& f, const uint2& mipdimensions, uint subsurface = 0) { return num_columns(f, mipdimensions.x, subsurface); }

// Returns the number of rows in a mip level with the specified height in 
// pixels. Block compressed formats have 1/4 the rows since their pitch includes 
// 4 rows at a time.
uint num_rows(const format& f, uint mipheight, uint subsurface = 0);
inline uint num_rows(const format& f, const uint2& mipdimensions, uint subsurface = 0) { return num_rows(f, mipdimensions.y, subsurface); }
inline uint num_rows(const format& f, const uint3& mipdimensions, uint subsurface = 0) { return num_rows(f, mipdimensions.y, subsurface) * max(1u, mipdimensions.z); }

// Returns the number of columns (x) and rows (y) in one call. This is different
// than dimensions, which is pixels. This is in elements, which can be 4x4 pixel 
// blocks for block compressed formats
inline uint2 num_columns_and_rows(const format& f, const uint2& mipdimensions, uint subsurface = 0) { return uint2(num_columns(f, mipdimensions, subsurface), num_rows(f, mipdimensions, subsurface)); }

// Returns the row size and number of rows as one operation in an uint2
inline uint2 byte_dimensions(const format& f, const uint2& mipdimensions, uint subsurface = 0) { return uint2(row_size(f, mipdimensions, subsurface), num_rows(f, mipdimensions, subsurface)); }
inline uint2 byte_dimensions(const format& f, const uint3& mipdimensions, uint subsurface = 0) { return uint2(row_size(f, mipdimensions, subsurface), num_rows(f, mipdimensions, subsurface)); }

// Returns the size in bytes for the specified mip level. CAREFUL, this 
// does not always imply the size that should be passed to memcpy. Ensure 
// you have enforced there is no scanline padding or data using these 
// calculations aren't a subregion in a larger surface.
uint mip_size(const format& f, const uint2& mipdimensions, uint subsurface = 0);
inline uint mip_size(const format& f, const uint3& mipdimensions, uint subsurface = 0) { return mip_size(f, mipdimensions.xy(), subsurface) * mipdimensions.z; }

// Returns the number of bytes from the start of Mip0 where the upper left
// corner of the specified mip level's data begins. The dimensions must always
// be specified as the mip0 dimensions since this is a cumulative offset.
uint offset(const info& inf, uint miplevel = 0, uint subsurface = 0);

// _____________________________________________________________________________
// Slice/surface introspection

// Returns the size in bytes for one slice for the described mip chain.
// That is, in an array of textures, each texture is a full mip chain, and this
// pitch is the number of bytes for the total mip chain of one of those 
// textures. This is the same as calculating the size of a mip page as described 
// by layout. For 3d textures, make sure that array_size is set to 1 and use 
// depth to supply the size in the 3rd dimension.
uint slice_pitch(const info& inf, uint subsurface = 0);

// Calculates the size for a total buffer of 1d/2d/3d/cube textures by summing 
// the various mip chains, then multiplying it by the number of slices. 
// Optionally you can supply a subsurface index to limit the size calculation to 
// that subsurface only. For more clarity on the difference between array_size 
// and depth see: http://www.cs.umbc.edu/~olano/s2006c03/ch02.pdf where it is 
// clear that when a first mip level for a 3d texture is (4,3,5) that the next 
// mip level is (2,1,2) and the next (1,1,1) whereas array_size set to 5 would 
// mean: 5*(4,3), 5*(2,1), 5*(1,1).
uint total_size(const info& inf);
uint total_size(const info& inf, uint subsurface);

// Calculates the dimensions you would need for an image to fit this surface
// natively and in its entirety.
uint2 dimensions(const info& inf, uint subsurface = 0);

// Calculates the dimensions you would need for an image to fit a slice of this 
// surface natively and in its entirety.
uint2 slice_dimensions(const info& inf, uint subsurface = 0);

// _____________________________________________________________________________
// Subresource API

// To simplify the need for much of the above API, interfaces should be 
// developed that take a subresource id and internally use these API to
// translate that into the proper byte locations and sizes. 
inline uint calc_subresource(uint miplevel, uint arrayslice, uint subsurface, uint num_mips, uint num_slices) { uint nMips = max(1u, num_mips); return miplevel + (arrayslice * nMips) + (subsurface * nMips * max(1u, num_slices)); }

// Converts subresource back to its mip level and slice as long as the num mips
// in the mip chain is specified.
inline void unpack_subresource(uint subresource, uint num_mips, uint num_slices, 
	uint* out_miplevel, uint* out_arrayslice, uint* out_subsurface) { uint nMips = max(1u, num_mips); uint as = max(1u, num_slices); *out_miplevel = subresource % nMips; *out_arrayslice = (subresource / nMips) % as; *out_subsurface = subresource / (nMips * as); }

// Returns the number of all subresources described by the info.
inline uint num_subresources(const info& inf) { return max(1u, num_mips(inf.mip_layout, inf.dimensions)) * max(1u, inf.array_size); }

// Returns the info for a given subresource.
subresource_info subresource(const info& inf, uint subresource);
	
// Returns a surface::info for the nth subsurfaces miplevel. Optionally also 
// calculate the byte dimensions for the specified subresource. i.e. the width 
// in bytes to copy (different than pitch) and number of rows of that width to 
// copy.
info subsurface(const info& inf, uint subsurface, uint miplevel, uint2* out_byte_dimensions = nullptr);

// Returns the number of bytes required to contain the subresource (mip level
// from a particular slice) when what you got is a subresource as returned from
// calc_subresource().
uint subresource_size(const subresource_info& subresource_info);
inline uint subresource_size(const info& inf, uint subresource) { return subresource_size(surface::subresource(inf, subresource)); }

// Returns the offset from a base pointer to the start of the specified 
// subresource.
uint subresource_offset(const info& inf, uint subresource, uint depth = 0);

// Returns the RowSize (width in bytes of one row) in x and the NumRows (number 
// of rows to copy) in y. Remember that this calculation is necessary 
// particularly with block compressed formats where the number of data rows is 
// different than the height of the surface in pixels/texels. The results of 
// this function are particularly useful when using oMemcpy2D for the 
// _SourceRowSize and _NumRows parameters.
inline uint2 byte_dimensions(const subresource_info& subresource_info) { return uint2(row_size(subresource_info.format, subresource_info.dimensions.xy(), subresource_info.subsurface), num_rows(subresource_info.format, subresource_info.dimensions.xy(), subresource_info.subsurface)); }
inline uint2 byte_dimensions(const info& inf, uint subresource) { return byte_dimensions(surface::subresource(inf, subresource)); }

// Given the surface info, a subresource into the surface, and the base pointer
// to the surface, this will return a populated mapped_subresource or an 
// const_mapped_subresource. _pByteDimensions is optional and if valid will 
// return the byte dimensions for the specified subresource.
const_mapped_subresource get_const_mapped_subresource(const info& inf, uint subresource, uint depth, const void* surface_bytes, uint2* out_byte_dimensions = nullptr);
mapped_subresource get_mapped_subresource(const info& inf, uint subresource, uint depth, void* surface_bytes, uint2* out_byte_dimensions = nullptr);

// Copies a source buffer into the specified subresource of the surface 
// described by a base pointer and an info
void update(const info& inf, uint subresource, void* dst_surface, const void* src_surface, uint src_row_pitch, const copy_option& option);

// Copies from a surface subresource to a specified destination buffer.
void copy(const info& inf, uint subresource, const void* src_surface, void* dst_surface, uint dst_row_pitch, const copy_option& option = copy_option::none);
void copy(const info& inf, const const_mapped_subresource& src, const mapped_subresource& dst, const copy_option& option = copy_option::none);
void copy(const subresource_info& subresource_info, const const_mapped_subresource& src, const mapped_subresource& dst, const copy_option& option = copy_option::none);

// For 3d textures a mapped subresource contains all depth slices at that mip level,
// this function will output the data pointer adjusted for the requested depth index.
inline void* depth_index_offset(const mapped_subresource& mapped, uint depth) { return byte_add(mapped.data, mapped.depth_pitch, depth); }

// Single-pixel get/put API. Only use this for debug/non-performant cases. This
// currently only supports common r rgb and rgba cases.
void put(const subresource_info& subresource_info, const mapped_subresource& dst, const uint2& _Coordinate, color _Color);
color get(const subresource_info& subresource_info, const const_mapped_subresource& src, const uint2& _Coordinate);

// _____________________________________________________________________________
// Tile API

// Returns the number of tiles required to hold the full resolution specified by 
// mipdimensions. If tile dimensions don't divide perfectly into the mip 
// dimensions the extra tile required to store the difference is included in 
// this calculation.
uint2 dimensions_in_tiles(const uint2& mipdimensions, const uint2& tiledimensions);
uint3 dimensions_in_tiles(const uint3& mipdimensions, const uint2& tiledimensions);

// Returns the number of tiles required to store all data for a mip of the 
// specified dimensions.
uint num_tiles(const uint2& mipdimensions, const uint2& tiledimensions);
uint num_tiles(const uint3& mipdimensions, const uint2& tiledimensions);

// Returns the number of tiles required to store all data for all mips in a 
// slice.
uint num_slice_tiles(const info& inf, const uint2& tiledimensions);

// Given the specified position, return the tile ID. This also will update 
// _TileDesc.Position start position that is aligned to the tile, which might be 
// different if a less rigorous value is chosen for _Position. Tile IDs are 
// calculated from the top-left and count up left-to-right, then top-to-bottom,
// then to the next smaller mip as described by surface::layout, then continues
// counting into the top level mip of the next slice and so on.
uint calc_tile_id(const info& inf, const tile_info& _TileInfo, uint2* out_position);

// Returns more detailed info from a tile ID
// from upper left) for the tile specified by tileid.
tile_info get_tile(const info& inf, const uint2& tiledimensions, uint tileid);

// A suggestion on whether or not you should use large memory pages. A little 
// bit arbitrary, but initially always returns false if your image is smaller 
// than half a large page, to ensure your not wasting too much memory. After 
// that, will return true if each tile won't copy at least half a small page 
// worth before encountering a tlb miss.
// TODO: currently have to pass in the page sizes which is platform code, maybe 
// thats fine, or should this go somewhere else?
// NOTE: the criteria used by this function is not for consoles, PC-only
bool use_large_pages(const info& inf, const uint2& tiledimensions, uint small_page_size_bytes, uint large_page_size_bytes);

// _____________________________________________________________________________
// Heavier-weight util functions that might need to be broken out into their own
// headers, but there's not enough to do that yet...

// Calls the specified function on each pixel.
void enumerate_pixels(const info& inf
	, const const_mapped_subresource& cmapped
	, const std::function<void(const void* _pPixel)>& enumerator);

void enumerate_pixels(const info& inf
	, const mapped_subresource& cmapped
	, const std::function<void(void* _pPixel)>& enumerator);

// Calls the specified function on each pixel of two same-formatted surfaces.
void enumerate_pixels(const info& inf
	, const const_mapped_subresource& mapped1
	, const const_mapped_subresource& mapped2
	, const std::function<void(const void* oRESTRICT _pPixel1, const void* oRESTRICT _pPixel2)>& enumerator);

// Calls the specified function on each pixel of two same-formatted surfaces.
// and writes to a 3rd other-format surface.
void enumerate_pixels(const info& _SurfaceInfoInput
	, const const_mapped_subresource& mappedInput1
	, const const_mapped_subresource& mappedInput2
	, const info& _SurfaceInfoOutput
	, mapped_subresource& mappedOutput
	, const std::function<void(const void* oRESTRICT _pPixel1, const void* oRESTRICT _pPixel2, void* oRESTRICT _pPixelOut)>& enumerator);

// Returns the root mean square of the difference between the two surfaces.
float calc_rms(const info& inf
	, const const_mapped_subresource& mapped1
	, const const_mapped_subresource& mapped2);

// Fills the output surface with abs(Input1 - Input2) for each pixel and returns 
// the root mean square. Currently the only supportedoutput image is r8_unorm.
// Inputs can be r8_unorm, b8g8r8_unorm, b8g8r8a8_unorm
float calc_rms(const info& _SurfaceInfoInput
	, const const_mapped_subresource& mappedInput1
	, const const_mapped_subresource& mappedInput2
	, const info& _SurfaceInfoOutput
	, mapped_subresource& mappedOutput);

// Fills the specified array with the count of pixels at each luminance value.
// (lum [0,1] mapped to [0,255] or [0,65536]).
void histogram8(const info& inf, const const_mapped_subresource& mapped, uint _Histogram[256]);
void histogram16(const info& inf, const const_mapped_subresource& mapped, uint _Histogram[65536]);

	} // namespace surface
} // namespace ouro

#endif
