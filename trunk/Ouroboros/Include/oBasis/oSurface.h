/**************************************************************************
 * The MIT License                                                        *
 * Copyright (c) 2013 OOOii.                                              *
 * antony.arciuolo@oooii.com                                              *
 * kevin.myers@oooii.com                                                  *
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
// oSURFACE_FORMATS map to a single memory buffer capable of being bound to 
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

// SLICE: A mip chain arranged in an accessible way (see oSURFACE_LAYOUT). A 
// slice can represent one mip chain in a texture array or a face of a cube map.

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

#include <oBasis/oFourCC.h>
#include <oBasis/oByte.h>
#include <oBasis/oInt.h>
#include <oBasis/oInvalid.h>
#include <oBasis/oMathTypes.h>

enum oSURFACE_FORMAT
{
	oSURFACE_UNKNOWN,
	oSURFACE_R32G32B32A32_TYPELESS,
	oSURFACE_R32G32B32A32_FLOAT,
	oSURFACE_R32G32B32A32_UINT,
	oSURFACE_R32G32B32A32_SINT,
	oSURFACE_R32G32B32_TYPELESS,
	oSURFACE_R32G32B32_FLOAT,
	oSURFACE_R32G32B32_UINT,
	oSURFACE_R32G32B32_SINT,
	oSURFACE_R16G16B16A16_TYPELESS,
	oSURFACE_R16G16B16A16_FLOAT,
	oSURFACE_R16G16B16A16_UNORM,
	oSURFACE_R16G16B16A16_UINT,
	oSURFACE_R16G16B16A16_SNORM,
	oSURFACE_R16G16B16A16_SINT,
	oSURFACE_R32G32_TYPELESS,
	oSURFACE_R32G32_FLOAT,
	oSURFACE_R32G32_UINT,
	oSURFACE_R32G32_SINT,
	oSURFACE_R32G8X24_TYPELESS,
	oSURFACE_D32_FLOAT_S8X24_UINT,
	oSURFACE_R32_FLOAT_X8X24_TYPELESS,
	oSURFACE_X32_TYPELESS_G8X24_UINT,
	oSURFACE_R10G10B10A2_TYPELESS,
	oSURFACE_R10G10B10A2_UNORM,
	oSURFACE_R10G10B10A2_UINT,
	oSURFACE_R11G11B10_FLOAT,
	oSURFACE_R8G8B8A8_TYPELESS,
	oSURFACE_R8G8B8A8_UNORM,
	oSURFACE_R8G8B8A8_UNORM_SRGB,
	oSURFACE_R8G8B8A8_UINT,
	oSURFACE_R8G8B8A8_SNORM,
	oSURFACE_R8G8B8A8_SINT,
	oSURFACE_R16G16_TYPELESS,
	oSURFACE_R16G16_FLOAT,
	oSURFACE_R16G16_UNORM,
	oSURFACE_R16G16_UINT,
	oSURFACE_R16G16_SNORM,
	oSURFACE_R16G16_SINT,
	oSURFACE_R32_TYPELESS,
	oSURFACE_D32_FLOAT,
	oSURFACE_R32_FLOAT,
	oSURFACE_R32_UINT,
	oSURFACE_R32_SINT,
	oSURFACE_R24G8_TYPELESS,
	oSURFACE_D24_UNORM_S8_UINT,
	oSURFACE_R24_UNORM_X8_TYPELESS,
	oSURFACE_X24_TYPELESS_G8_UINT,
	oSURFACE_R8G8_TYPELESS,
	oSURFACE_R8G8_UNORM,
	oSURFACE_R8G8_UINT,
	oSURFACE_R8G8_SNORM,
	oSURFACE_R8G8_SINT,
	oSURFACE_R16_TYPELESS,
	oSURFACE_R16_FLOAT,
	oSURFACE_D16_UNORM,
	oSURFACE_R16_UNORM,
	oSURFACE_R16_UINT,
	oSURFACE_R16_SNORM,
	oSURFACE_R16_SINT,
	oSURFACE_R8_TYPELESS,
	oSURFACE_R8_UNORM,
	oSURFACE_R8_UINT,
	oSURFACE_R8_SNORM,
	oSURFACE_R8_SINT,
	oSURFACE_A8_UNORM,
	oSURFACE_R1_UNORM,
	oSURFACE_R9G9B9E5_SHAREDEXP,
	oSURFACE_R8G8_B8G8_UNORM,
	oSURFACE_G8R8_G8B8_UNORM,
	oSURFACE_BC1_TYPELESS,
	oSURFACE_BC1_UNORM,
	oSURFACE_BC1_UNORM_SRGB,
	oSURFACE_BC2_TYPELESS,
	oSURFACE_BC2_UNORM,
	oSURFACE_BC2_UNORM_SRGB,
	oSURFACE_BC3_TYPELESS,
	oSURFACE_BC3_UNORM,
	oSURFACE_BC3_UNORM_SRGB,
	oSURFACE_BC4_TYPELESS,
	oSURFACE_BC4_UNORM,
	oSURFACE_BC4_SNORM,
	oSURFACE_BC5_TYPELESS,
	oSURFACE_BC5_UNORM,
	oSURFACE_BC5_SNORM,
	oSURFACE_B5G6R5_UNORM,
	oSURFACE_B5G5R5A1_UNORM,
	oSURFACE_B8G8R8A8_UNORM,
	oSURFACE_B8G8R8X8_UNORM,
	oSURFACE_R10G10B10_XR_BIAS_A2_UNORM,
	oSURFACE_B8G8R8A8_TYPELESS,
	oSURFACE_B8G8R8A8_UNORM_SRGB,
	oSURFACE_B8G8R8X8_TYPELESS,
	oSURFACE_B8G8R8X8_UNORM_SRGB,
	oSURFACE_BC6H_TYPELESS,
	oSURFACE_BC6H_UF16,
	oSURFACE_BC6H_SF16,
	oSURFACE_BC7_TYPELESS,
	oSURFACE_BC7_UNORM,
	oSURFACE_BC7_UNORM_SRGB,
	oSURFACE_AYUV,
	oSURFACE_Y410,
	oSURFACE_Y416,
	oSURFACE_NV12,
	oSURFACE_P010,
	oSURFACE_P016,
	oSURFACE_420_OPAQUE,
	oSURFACE_YUY2,
	oSURFACE_Y210,
	oSURFACE_Y216,
	oSURFACE_NV11,
	oSURFACE_AI44,
	oSURFACE_IA44,
	oSURFACE_P8,
	oSURFACE_A8P8,
	oSURFACE_B4G4R4A4_UNORM,
	//formats below here are not currently directly loadable to DirectX.
	oSURFACE_R8G8B8_UNORM,
	oSURFACE_B8G8R8_UNORM,
	// Multi-surface YUV formats (for emulation ahead of HW and for more robust 
	// compression); also not currently directly loadable to DirectX.
	oSURFACE_Y8_U8_V8_UNORM, // One R8_UNORM per channel
	oSURFACE_Y8_A8_U8_V8_UNORM, // One R8_UNORM per channel
	oSURFACE_YBC4_UBC4_VBC4_UNORM, // One BC4_UNORM per channel
	oSURFACE_YBC4_ABC4_UBC4_VBC4_UNORM, // One BC4_UNORM per channel
	oSURFACE_Y8_U8V8_UNORM, // Y: R8_UNORM UV: R8G8_UNORM (half-res)
	oSURFACE_Y8A8_U8V8_UNORM, // AY: R8G8_UNORM UV: R8G8_UNORM (half-res)
	oSURFACE_YBC4_UVBC5_UNORM, // Y: BC4_UNORM UV: BC5_UNORM (half-res)
	oSURFACE_YABC5_UVBC5_UNORM, // AY: BC5_UNORM UV: BC5_UNORM (half-res)
	oSURFACE_NUM_FORMATS,
};

enum oSURFACE_LAYOUT
{
	// IMAGE: no mip chain, so RowSize == RowPitch
	// TIGHT: mips are right after each other, the naive/initial thing a person
	//        might do where next-scanline strides are the same as valid data so
	//        the next-scanline stride also changes with each mip level.
	// BELOW: Step down when moving from Mip0 to Mip1
	//        Step right when moving from Mip1 to Mip2
	//        Step down for all of the other MIPs
	//        RowPitch is generally the next-scanline stride of Mip0, except
	//        when there are alignment issues e.g. with Mip0 width of 1,
	//        or Mip0 width of 4 or less for block compressed formats.
	// RIGHT: Step right when moving from Mip0 to Mip1
	//        Step down for all of the other MIPs.
	//        RowPitch is generally the next-scanline stride of Mip0 plus the 
	//        stride of mip1.
	// Note that for BELOW and RIGHT the lowest Mip can be below the bottom
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

	oSURFACE_LAYOUT_IMAGE,
	oSURFACE_LAYOUT_TIGHT,
	oSURFACE_LAYOUT_BELOW,
	oSURFACE_LAYOUT_RIGHT,
};

struct oSURFACE_DESC
{
	oSURFACE_DESC()
		: Dimensions(oInvalid, oInvalid, oInvalid)
		, NumSlices(1)
		, Format(oSURFACE_UNKNOWN)
		, Layout(oSURFACE_LAYOUT_IMAGE)
	{}

	int3 Dimensions;
	int NumSlices;
	oSURFACE_FORMAT Format;
	oSURFACE_LAYOUT Layout;
};

struct oSURFACE_SUBRESOURCE_DESC
{
	int3 Dimensions;
	int MipLevel;
	int Slice;
	int Subsurface;
};

struct oSURFACE_TILE_DESC
{
	int2 Position;
	int MipLevel;
	int Slice;
};

struct oSURFACE_MAPPED_SUBRESOURCE
{
	void* pData;
	unsigned int RowPitch;
	unsigned int DepthPitch;
};

struct oSURFACE_CONST_MAPPED_SUBRESOURCE
{
	oSURFACE_CONST_MAPPED_SUBRESOURCE() {}
	oSURFACE_CONST_MAPPED_SUBRESOURCE(const oSURFACE_MAPPED_SUBRESOURCE& _other)
	{
		*this = _other;
	}
	oSURFACE_CONST_MAPPED_SUBRESOURCE& operator=(const oSURFACE_MAPPED_SUBRESOURCE& _other)
	{
		pData = _other.pData;
		RowPitch = _other.RowPitch;
		DepthPitch = _other.DepthPitch;

		return *this;
	}

	const void* pData;
	unsigned int RowPitch;
	unsigned int DepthPitch;
};

// _____________________________________________________________________________
// oSURFACE_FORMAT introspection

// oAsString returns string form of enum. oToString does the same to a buffer,
// and oFromString matches an enum string to its value.

// Returns true if the specified format is a block-compressed format.
bool oSurfaceFormatIsBlockCompressed(oSURFACE_FORMAT _Format);

// Returns true if the specified format is one typically used to write 
// Z-buffer depth information.
bool oSurfaceFormatIsDepth(oSURFACE_FORMAT _Format);

// Returns true of the specified format includes RGB and A or S. SRGB formats
// will result in true from this function.
bool oSurfaceFormatIsAlpha(oSURFACE_FORMAT _Format);

// Returns true if the specified format is normalized between 0.0f and 1.0f
bool oSurfaceFormatIsUNORM(oSURFACE_FORMAT _Format);

// Returns true if the specified format is organized such that its elements are
// not interleaved in memory.
bool oSurfaceFormatIsPlanar(oSURFACE_FORMAT _Format);

// Returns true if the specified format is in YUV space. YUV can apply to both
// single-surface and multi-surface YUV formats.
bool oSurfaceFormatIsYUV(oSURFACE_FORMAT _Format);

// Returns the number of separate channels used for a pixel. For example RGB 
// has 3 channels, XRGB has 4, RGBA has 4.
int oSurfaceFormatGetNumChannels(oSURFACE_FORMAT _Format);

// Emulated YUV formats can have more than one surface format associated with 
// them, such as oSURFACE_R8_UNORM for surface index 0 and oSURFACE_R8G8_UNORM
// for surface index 0. For most formats this will return 0 meaning the format
// is for one and only one surface.
int oSurfaceFormatGetNumSubformats(oSURFACE_FORMAT _Format);

// Returns the offset to MipLevel that the specified sub-surface uses. For some 
// YUV formats, the second plane (UV) is half the size of the first plane (AY). 
// For most formats this will return 0 and for those down-sampled planes, this 
// will return a value to add to the mip level to evaluate/base size off of.
int oSurfaceFormatGetSubsampleBias(oSURFACE_FORMAT _Format, int _SubsurfaceIndex);

// Returns the number of bytes required to store the smallest atom of a 
// surface. For single-bit image formats, this will return 1. For tiled 
// formats this will return the byte size of 1 tile.
int oSurfaceFormatGetSize(oSURFACE_FORMAT _Format, int _SubsurfaceIndex = 0);

// Returns the minimum dimensions the format supports.  For most formats
// this is 1,1 but for block formats it can be something else.
int2 oSurfaceFormatGetMinDimensions(oSURFACE_FORMAT _Format);

// Get number of bits per format. This includes any X bits as described in
// the format enum.
int oSurfaceFormatGetBitSize(oSURFACE_FORMAT _Format);

// Returns the number of bits per channel, either R,G,B,A or Y,U,V,A.
void oSurfaceGetChannelBitSize(oSURFACE_FORMAT _Format, int* _pNBitsR, int* _pNBitsG, int* _pNBitsB, int* _pNBitsA);

// Returns the surface format of the specified format's nth subformat. For RGBA-
// based formats, this will return the same value as _Format for index 0 and 
// oSURFACE_UNKNOWN for any other plane. For YUV formats this will return the
// format needed for each oSURFACE used to emulate the YUV format.
oSURFACE_FORMAT oSurfaceGetSubformat(oSURFACE_FORMAT _Format, int _SubsurfaceIndex);

// Most formats for data types have a fourcc that is unique at least amongst 
// other oSURFACE_FORMATs. This is useful for serialization where you want 
// something a bit more version-stable than an enum value.
oFourCC oSurfaceFormatToFourCC(oSURFACE_FORMAT _Format);

// Convert an oFourCC as returned from oSurfaceFormatToFourCC to its associated 
// oSURFACE_FORMAT.
oSURFACE_FORMAT oSurfaceFormatFromFourCC(oFourCC _FourCC);

// _____________________________________________________________________________
// Mip Level (1 2D plane/slice, a simple image) introspection

// Returns the number of mipmaps generated from the specified dimensions down to 
// a 1x1 mip level.
int oSurfaceCalcNumMips(bool _HasMips, const int3& _Mip0Dimensions);
inline int oSurfaceCalcNumMips(bool _HasMips, const int2& _Mip0Dimensions) { return oSurfaceCalcNumMips(_HasMips, int3(_Mip0Dimensions, 1)); }
inline int oSurfaceCalcNumMips(oSURFACE_LAYOUT _Layout, const int3& _Mip0Dimensions) { return oSurfaceCalcNumMips(_Layout != oSURFACE_LAYOUT_IMAGE, _Mip0Dimensions); }
inline int oSurfaceCalcNumMips(oSURFACE_LAYOUT _Layout, const int2& _Mip0Dimensions) { return oSurfaceCalcNumMips(_Layout != oSURFACE_LAYOUT_IMAGE, int3(_Mip0Dimensions, 1)); }

// Returns the width, height, or depth dimension of the specified mip level
// given mip0's dimension. This appropriately pads BC formats and conforms to 
// hardware-expected sizes all the way down to 1x1. Because oSurface is meant to
// be used with hardware-compatible surfaces, all dimensions must be a power 
// of 2.

int oSurfaceMipCalcDimension(oSURFACE_FORMAT _Format, int _Mip0Dimension, int _MipLevel = 0, int _SubsurfaceIndex = 0);
int2 oSurfaceMipCalcDimensions(oSURFACE_FORMAT _Format, const int2& _Mip0Dimensions, int _MipLevel = 0, int _SubsurfaceIndex = 0);
int3 oSurfaceMipCalcDimensions(oSURFACE_FORMAT _Format, const int3& _Mip0Dimensions, int _MipLevel = 0, int _SubsurfaceIndex = 0);

// Calculate mip dimensions for non-power-of-2 textures using the D3D/OGL2.0
// floor convention: http://www.opengl.org/registry/specs/ARB/texture_non_power_of_two.txt
int oSurfaceMipCalcDimensionNPOT(oSURFACE_FORMAT _Format, int _Mip0Dimension, int _MipLevel = 0, int _SubsurfaceIndex = 0);
int2 oSurfaceMipCalcDimensionsNPOT(oSURFACE_FORMAT _Format, const int2& _Mip0Dimensions, int _MipLevel = 0, int _SubsurfaceIndex = 0);
int3 oSurfaceMipCalcDimensionsNPOT(oSURFACE_FORMAT _Format, const int3& _Mip0Dimensions, int _MipLevel = 0, int _SubsurfaceIndex = 0);

// Returns the size in bytes for one row of valid data for the specified mip 
// level. This does a bit more than a simple multiply because it takes into 
// consideration block compressed formats. This IS NOT the calculation to use to
// get to the next scanline of data unless it can be guaranteed that there is
// no padding nor is the valid data merely a subregion of a larger 2D plane, but 
// this IS the calculate for the number of bytes in the current scanline.
int oSurfaceMipCalcRowSize(oSURFACE_FORMAT _Format, int _MipWidth, int _SubsurfaceIndex = 0);
inline int oSurfaceMipCalcRowSize(oSURFACE_FORMAT _Format, const int2& _MipDimensions, int _SubsurfaceIndex = 0) { return oSurfaceMipCalcRowSize(_Format, _MipDimensions.x, _SubsurfaceIndex); }
inline int oSurfaceMipCalcRowSize(oSURFACE_FORMAT _Format, const int3& _MipDimensions, int _SubsurfaceIndex = 0) { return oSurfaceMipCalcRowSize(_Format, _MipDimensions.x, _SubsurfaceIndex); }

// Returns the number of bytes to increment to get to the next row of a 2D 
// surface. Do not read or write using this value as padding may be used for 
// other reasons such as internal data or to store other elements/mip levels.
int oSurfaceMipCalcRowPitch(const oSURFACE_DESC& _SurfaceDesc, int _MipLevel = 0, int _SubsurfaceIndex = 0);

// Returns the number of bytes to increment to get to the next slice of a 3D
// surface. Its calculated as RowPitch * number of rows at the requested
// mip level.
int oSurfaceMipCalcDepthPitch(const oSURFACE_DESC& _SurfaceDesc, int _MipLevel = 0, int _SubsurfaceIndex = 0);

// Returns the number of columns (number of elements in a row) in a mip level 
// with the specified width in pixels. Block compressed formats will return 1/4
// the columns since their atomic element - the block - is 4x4 pixels.
int oSurfaceMipCalcNumColumns(oSURFACE_FORMAT _Format, int _MipWidth, int _SubsurfaceIndex = 0);
inline int oSurfaceMipCalcNumColumns(oSURFACE_FORMAT _Format, const int2& _MipDimensions, int _SubsurfaceIndex = 0) { return oSurfaceMipCalcNumColumns(_Format, _MipDimensions.x, _SubsurfaceIndex); }

// Returns the number of rows in a mip level with the specified height in 
// pixels. Block compressed formats have 1/4 the rows since their pitch includes 
// 4 rows at a time.
int oSurfaceMipCalcNumRows(oSURFACE_FORMAT _Format, int _MipHeight, int _SubsurfaceIndex = 0);
inline int oSurfaceMipCalcNumRows(oSURFACE_FORMAT _Format, const int2& _MipDimensions, int _SubsurfaceIndex = 0) { return oSurfaceMipCalcNumRows(_Format, _MipDimensions.y, _SubsurfaceIndex); }
inline int oSurfaceMipCalcNumRows(oSURFACE_FORMAT _Format, const int3& _MipDimensions, int _SubsurfaceIndex = 0) { return oSurfaceMipCalcNumRows(_Format, _MipDimensions.y, _SubsurfaceIndex) * _MipDimensions.z; }

// Returns the number of columns (x) and rows (y) in one call. This is different
// than dimensions, which is pixels. This is in elements, which can be 4x4 pixel 
// blocks for block compressed formats
inline int2 oSurfaceMipCalcNumColumnsAndRows(oSURFACE_FORMAT _Format, const int2& _MipDimensions, int _SubsurfaceIndex = 0) { return int2(oSurfaceMipCalcNumColumns(_Format, _MipDimensions, _SubsurfaceIndex), oSurfaceMipCalcNumRows(_Format, _MipDimensions, _SubsurfaceIndex)); }

// Returns the RowSize and NumRows as one operation in an int2
inline int2 oSurfaceMipCalcByteDimensions(oSURFACE_FORMAT _Format, const int2& _MipDimensions, int _SubsurfaceIndex = 0) { return int2(oSurfaceMipCalcRowSize(_Format, _MipDimensions, _SubsurfaceIndex), oSurfaceMipCalcNumRows(_Format, _MipDimensions, _SubsurfaceIndex)); }
inline int2 oSurfaceMipCalcByteDimensions(oSURFACE_FORMAT _Format, const int3& _MipDimensions, int _SubsurfaceIndex = 0) { return int2(oSurfaceMipCalcRowSize(_Format, _MipDimensions, _SubsurfaceIndex), oSurfaceMipCalcNumRows(_Format, _MipDimensions, _SubsurfaceIndex)); }

// Returns the size in bytes for the specified mip level. CAREFUL, this does not 
// always imply the size that should be passed to memcpy. Ensure you have 
// enforced that there is no scanline padding or that data using these 
// calculations aren't a subregion in a larger surface.
int oSurfaceMipCalcSize(oSURFACE_FORMAT _Format, const int2& _MipDimensions, int _SubsurfaceIndex = 0);
inline int oSurfaceMipCalcSize(oSURFACE_FORMAT _Format, const int3& _MipDimensions, int _SubsurfaceIndex = 0) { return oSurfaceMipCalcSize(_Format, _MipDimensions.xy(), _SubsurfaceIndex) * _MipDimensions.z; }

// Returns the number of bytes from the start of Mip0 where the upper left
// corner of the specified mip level's data begins. The dimensions must always
// be specified as the mip0 dimensions since this is a cumulative offset.
int oSurfaceMipCalcOffset(const oSURFACE_DESC& _SurfaceDesc, int _MipLevel = 0, int _SubsurfaceIndex = 0);

// Returns the number of tiles required to hold the full resolution specified by 
// _MipDimensions. If tile dimensions don't divide perfectly into the mip 
// dimensions the extra tile required to store the difference is included in 
// this calculation.
int2 oSurfaceMipCalcDimensionsInTiles(const int2& _MipDimensions, const int2& _TileDimensions);
int3 oSurfaceMipCalcDimensionsInTiles(const int3& _MipDimensions, const int2& _TileDimensions);

// Returns the number of tiles required to store all data for a mip of the 
// specified dimensions.
int oSurfaceMipCalcNumTiles(const int2& _MipDimensions, const int2& _TileDimensions);
int oSurfaceMipCalcNumTiles(const int3& _MipDimensions, const int2& _TileDimensions);

// _____________________________________________________________________________
// Slice/Surface introspection

// Returns the size in bytes for one slice for the described mip chain.
// That is, in an array of textures, each texture is a full mip chain, and this
// pitch is the number of bytes for the total mip chain of one of those textures.
// This is the same as calculating the size of a mip page as described by 
// oSURFACE_LAYOUT.
// For 3d textures, make sure that NumSlices is set to 1 and use Depth to 
// supply the size in the 3rd dimension.
int oSurfaceSliceCalcPitch(const oSURFACE_DESC& _SurfaceDesc, int _SubsurfaceIndex = 0);

// Returns the number of tiles required to store all data for all mips in a 
// slice.
int oSurfaceSliceCalcNumTiles(const oSURFACE_DESC& _SurfaceDesc, const int2& _TileDimensions);

// Calculates the size for a total buffer of 1d/2d/3d/cube textures by summing the 
// various mip chains, then multiplying it by the number of slices. 
// 3d textures need to have NumSlices set to 1.
// All other texture types need to have Dimensions.z set to 1.
// For more clarity on the difference between NumSlices and Depth see:
// http://www.cs.umbc.edu/~olano/s2006c03/ch02.pdf
// where it's clear that when a first mip level for a 3d texture is (4,3,5) that 
// the next mip level is (2,1,2) and the next (1,1,1)
// Whereas NumSlices set to 5 would mean: 5*(4,3), 5*(2,1), 5*(1,1)
int oSurfaceCalcSize(const oSURFACE_DESC& _SurfaceDesc, int _SubsurfaceIndex = 0);

// Calculates the dimensions you would need for an oImage to fit this surface
// natively and in its entirety.
int2 oSurfaceCalcDimensions(const oSURFACE_DESC& _SurfaceDesc, int _SubsurfaceIndex = 0);

// Calculates the dimensions you would need for an oImage to fit a slice of this surface
// natively and in its entirety.
int2 oSurfaceSliceCalcDimensions(const oSURFACE_DESC& _SurfaceDesc, int _SubsurfaceIndex = 0);

// _____________________________________________________________________________
// Subresource API

// To simplify the need for much of the above API, interfaces should be 
// developed that take a subresource id and internally use these API to
// translate that into the proper byte locations and sizes. 
inline int oSurfaceCalcSubresource(int _MipLevel, int _SliceIndex, int _SubsurfaceIndex, int _NumMips, int _NumSlices) { return _MipLevel + (_SliceIndex * _NumMips) + (_SubsurfaceIndex * _NumMips * _NumSlices); }

// Converts _Subresource back to its mip level and slice as long as the num mips
// in the mip chain is specified.
inline void oSurfaceSubresourceUnpack(int _Subresource, int _NumMips, int _NumSlices, int* _pMipLevel, int* _pSliceIndex, int* _pSubsurfaceIndex) { *_pMipLevel = _Subresource % _NumMips; *_pSliceIndex = (_Subresource / _NumMips) % _NumSlices; *_pSubsurfaceIndex = _Subresource / (_NumMips * _NumSlices); }

// Fills the subresource desc with the dimensions and mip information for a 
// given subresource.
void oSurfaceSubresourceGetDesc(const oSURFACE_DESC& _SurfaceDesc, int _Subresource, oSURFACE_SUBRESOURCE_DESC* _pSubresourceDesc);

// Returns the number of bytes required to contain the subresource (mip level
// from a particular slice) when what you got is a subresource as returned from
// oSurfaceCalcSubresource().
int oSurfaceSubresourceCalcSize(const oSURFACE_DESC& _SurfaceDesc, const oSURFACE_SUBRESOURCE_DESC& _SubresourceDesc);
inline int oSurfaceSubresourceCalcSize(const oSURFACE_DESC& _SurfaceDesc, int _Subresource) { oSURFACE_SUBRESOURCE_DESC ssrd; oSurfaceSubresourceGetDesc(_SurfaceDesc, _Subresource, &ssrd); return oSurfaceSubresourceCalcSize(_SurfaceDesc, ssrd); }

// Returns the offset from a base pointer to the start of the specified 
// subresource.
int oSurfaceSubresourceCalcOffset(const oSURFACE_DESC& _SurfaceDesc, int _Subresource, int _DepthIndex = 0);

// Returns the RowSize (width in bytes of one row) in x and the NumRows (number 
// of rows to copy) in y. Remember that this calculation is necessary 
// particularly with block compressed formats where the number of data rows is 
// different than the height of the surface in pixels/texels. The results of 
// this function are particularly useful when using oMemcpy2D for the 
// _SourceRowSize and _NumRows parameters.
inline int2 oSurfaceSubresourceCalcByteDimensions(oSURFACE_FORMAT _Format, const oSURFACE_SUBRESOURCE_DESC& _SubresourceDesc) { return int2(oSurfaceMipCalcRowSize(_Format, _SubresourceDesc.Dimensions.xy(), _SubresourceDesc.Subsurface), oSurfaceMipCalcNumRows(_Format, _SubresourceDesc.Dimensions.xy(), _SubresourceDesc.Subsurface)); }
inline int2 oSurfaceSubresourceCalcByteDimensions(const oSURFACE_DESC& _SurfaceDesc, int _Subresource) { oSURFACE_SUBRESOURCE_DESC ssrd; oSurfaceSubresourceGetDesc(_SurfaceDesc, _Subresource, &ssrd); return oSurfaceSubresourceCalcByteDimensions(_SurfaceDesc.Format, ssrd); }

// _____________________________________________________________________________
// Tile API

// Returns the nth mip where the mip level best-fits into a tile. i.e. several
// tiles are no longer required to store all the mip data, and all mip levels
// after the one returned by this function can together fit into one mip level.
int oSurfaceTileCalcBestFitMipLevel(const oSURFACE_DESC& _SurfaceDesc, const int2& _TileDimensions);

// Given the specified position, return the tile ID. This also will update 
// _TileDesc.Position start position that is aligned to the tile, which might be 
// different if a less rigorous value is chosen for _Position. Tile IDs are 
// calculated from the top-left and count up left-to-right, then top-to-bottom,
// then to the next smaller mip as described by oSURFACE_LAYOUT, then continues
// counting into the top level mip of the next slice and so on.
int oSurfaceCalcTile(const oSURFACE_DESC& _SurfaceDesc, const int2& _TileDimensions, oSURFACE_TILE_DESC& _InOutTileDesc);

// Fill in values for mip level, slice, and position in mip (pixel coordinates 
// from upper left) for the tile specified by _TileID.
void oSurfaceTileGetDesc(const oSURFACE_DESC& _SurfaceDesc, const int2& _TileDimensions, int _TileID, oSURFACE_TILE_DESC* _pTileDesc);

// A suggestion on whether or not you should use large memory pages. A little 
// bit arbitrary, but initially always returns false if your image is smaller 
// than half a large page, to ensure your not wasting too much memory. After 
// that, will return true if each tile won't copy at least half a small page 
// worth before encountering a tlb miss.
// TODO: currently have to pass in the page sizes which is platform code, maybe 
// thats fine, or should this go somewhere else?
bool oShouldUseLargePages(const int3& _SurfaceDimensions, oSURFACE_FORMAT _Format, int _TileWidth, int _SmallPageSize, int _LargePageSize);

// _____________________________________________________________________________
// Misc.

// given the surface desc, a subresource into the surface, and the base pointer
// to the surface, this will return a populated oSURFACE_MAPPED_SUBRESOURCE or 
// an oSURFACE_CONST_MAPPED_SUBRESOURCE. _pByteDimensions is optional and if 
// valid will return the byte dimensions for the specified subresource.
void oSurfaceCalcMappedSubresource(const oSURFACE_DESC& _SurfaceDesc, int _Subresource, int _DepthIndex, void* _pSurface, oSURFACE_MAPPED_SUBRESOURCE* _pMappedSubresource, int2* _pByteDimensions = nullptr);
void oSurfaceCalcMappedSubresource(const oSURFACE_DESC& _SurfaceDesc, int _Subresource, int _DepthIndex, const void* _pSurface, oSURFACE_CONST_MAPPED_SUBRESOURCE* _pMappedSubresource, int2* _pByteDimensions = nullptr);

// Copies a source buffer into the specified subresource of the surface 
// described by a base pointer and an oSURFACE_DESC
void oSurfaceUpdateSubresource(const oSURFACE_DESC& _SurfaceDesc, int _Subresource, void* _pDestinationSurface, const void* _pSource, size_t _SourceRowPitch, bool _FlipVertical);

// Copies from a surface subresource to a specified destination buffer.
void oSurfaceCopySubresource(const oSURFACE_DESC& _SurfaceDesc, int _Subresource, const void* _pSourceSurface, void* _pDestination, size_t _DestinationRowPitch, bool _FlipVertical);
void oSurfaceCopySubresource(const oSURFACE_DESC& _SurfaceDesc, const oSURFACE_CONST_MAPPED_SUBRESOURCE& _SrcMap, oSURFACE_MAPPED_SUBRESOURCE* _DstMap, bool _FlipVertical);

// For 3d textures a mapped subresource contains all depth slices at that mip level,
// this function will output the data pointer adjusted for the requested depth index.
inline void oSurfaceMappedSubresourceOffsetDepthIndex(const oSURFACE_MAPPED_SUBRESOURCE& _MappedSubresource, int _DepthIndex, void** _pMappedSubResourceData) { *_pMappedSubResourceData = oByteAdd(_MappedSubresource.pData, _MappedSubresource.DepthPitch, _DepthIndex); }

#endif
