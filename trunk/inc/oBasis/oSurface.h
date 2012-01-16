/**************************************************************************
 * The MIT License                                                        *
 * Copyright (c) 2011 Antony Arciuolo & Kevin Myers                       *
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
#pragma once
#ifndef oSurface_h
#define oSurface_h

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
	//formats below here are not currently directly loadable to DirectX.
	oSURFACE_R8G8B8_UNORM,
	oSURFACE_B8G8R8_UNORM,
	oSURFACE_NUM_FORMATS,
};

struct oSURFACE_DESC
{
	oSURFACE_DESC()
		: Dimensions(0,0,0)
		, NumMips(0)
		, RowPitch(0)
		, DepthPitch(0)
		, Format(oSURFACE_UNKNOWN)
	{}

	int3 Dimensions;

	unsigned int NumMips; // 1 is the count of an image with no mips
	unsigned int RowPitch;
		
	union
	{
		unsigned int DepthPitch;
		unsigned int SlicePitch;
	};

	oSURFACE_FORMAT Format;
};

// oAsString returns string form of enum. oToString does the same to a buffer,
// and oFromString matches an enum string to its value.

	// Returns true if the specified format is a block-compressed format.
bool oSurfaceIsBlockCompressedFormat(oSURFACE_FORMAT _Format);

// Returns true if the specified format is one typically used to write 
// Z-buffer depth information.
bool oSurfaceIsDepthFormat(oSURFACE_FORMAT _Format);

// Retursn true of the specified format includes RGB and A or S. SRGB formats
// will result in true from this function.
bool oSurfaceIsAlphaFormat(oSURFACE_FORMAT _Format);

// Returns true if the specified format is normalized between 0.0f and 1.0f
bool oSurfaceIsUNORM(oSURFACE_FORMAT _Format);

// Returns the number of separate channels used for a pixel. For example RGB 
// has 3 channels, XRGB has 4, RGBA has 4.
unsigned int oSurfaceGetNumChannels(oSURFACE_FORMAT _Format);

// Returns the number of bytes required to store the smallest atom of a 
// surface. For single-bit image formats, this will return 1. For tiled 
// formats this will return the byte size of 1 tile.
unsigned int oSurfaceGetSize(oSURFACE_FORMAT _Format);

// Get number of bits per format. This includes any X bits as described in
// the format enum.
unsigned int oSurfaceGetBitSize(oSURFACE_FORMAT _Format);

// Returns the number of bits per channel
void oSurfaceGetChannelBitSize(oSURFACE_FORMAT _Format, int* _pNBitsR, int* _pNBitsG, int* _pNBitsB, int* _pNBitsA);

// Returns the size in bytes for one row for the described mip chain. 
// This does a bit more than a simple multiply because it takes into 
// consideration block compressed formats.
unsigned int oSurfaceCalcRowPitch(oSURFACE_FORMAT _Format, unsigned int _Mip0Width, unsigned int _MipLevel = 0);

// Returns the size in bytes for one mip level for the described mip chain
unsigned int oSurfaceCalcLevelPitch(oSURFACE_FORMAT _Format, int2 _MipDimensions, unsigned int _MipLevel = 0);

// Returns the size in bytes for one 2D array slice for the described mip chain.
// That is, in an array of textures, each texture is a full mip chain, and this
// pitch is the number of bytes for the total mip chain of one of those textures.
unsigned int oSurfaceCalcSlicePitch(oSURFACE_FORMAT _Format, int2 _MipDimensions, unsigned int _NumMips = 0);

// Returns the width, height, or depth dimension of the specified mip level
// given mip0's dimension. BCAlign set to true will align the dimension to be 
// compatible with 4x4 block compression. All dimensions must be a power of 2.
unsigned int oSurfaceCalcMipDimension(oSURFACE_FORMAT _Format, unsigned int _Mip0Dimension, unsigned int _MipLevel = 0);

// Returns the number of rows in a mip with the specified height in pixels.
// Block compressed formats have 1/4 the rows size their pitch includes 4 rows
// at a time.
unsigned int oSurfaceCalcNumRows(oSURFACE_FORMAT _Format, unsigned int _Mip0Height, unsigned int _MipLevel = 0);

// Returns the number of mip levels for the specified dimensions
unsigned int oSurfaceCalcMipCount(int2 _MipDimensions);

// Returns the size necessary for all mip levels for the specified dimensions.
unsigned int oSurfaceCalcBufferSize(oSURFACE_FORMAT _Format, int2 _MipDimensions, unsigned int _NumSlices);

// Returns the number of bytes required to contain the subresource (mip level
// from a particular slice)
unsigned int oSurfaceCalcSubresourceSize(oSURFACE_FORMAT _Format, int2 _MipDimensions, unsigned int _NumSlices, unsigned int _Subresource);

// Returns the number of bytes into a mip chain where the specified level begins.
unsigned int oSurfaceCalcLevelOffset(oSURFACE_FORMAT _Format, int2 _MipDimensions, unsigned int _MipLevel = 0);

// Returns the number of bytes into a buffer where the specified subresource 
// (mip level from a particular slice) begins.
unsigned int oSurfaceCalcBufferOffset(oSURFACE_FORMAT _Format, int2 _MipDimensions, unsigned int _NumSlices, unsigned int _Subresource);

// To simplify the need for much of the above API, interfaces should be 
// developed that take a subresource id and internally use these API to
// translate that into the proper byte locations and sizes. 
inline unsigned int oSurfaceCalcSubresource(unsigned int _MipLevel, unsigned int _SliceIndex, unsigned int _NumMips) { return _MipLevel + (_SliceIndex * _NumMips); }

// Given a known number of mips, convert a subresource back into slice and 
// mip level 
inline void oSurfaceUnpackSubresource(unsigned int _Subresource, unsigned int _NumMips, unsigned int* _pMipLevel, unsigned int* _pSliceIndex)
{
	*_pSliceIndex = _Subresource / _NumMips;
	*_pMipLevel = _Subresource % _NumMips;
}

unsigned int oSurfaceCalcSize(const oSURFACE_DESC& _Desc);
unsigned int oSurfaceCalcSize(oSURFACE_FORMAT _Format, unsigned int _Width, unsigned int _Height);

inline unsigned int oSurfaceCalcSize(oSURFACE_FORMAT _Format, const int2& _Dimensions)
{
	// @oooii-tony: Code base is migrating towards "if it's a simple type, just use int",
	// so cast away unsigned-ness for now.
	return oSurfaceCalcSize(_Format, static_cast<unsigned int>(_Dimensions.x), static_cast<unsigned int>(_Dimensions.y));
}

#endif
