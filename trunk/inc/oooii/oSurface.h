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
#pragma once
#ifndef oSurface_h
#define oSurface_h

#if defined(_WIN32) || defined(_WIN64)
	#include <oooii/oColor.h>
#endif

namespace oSurface
{
	// 2D visual buffers (images, textures, pictures, bitmaps, whatever you want
	// to call them) are used in many different places in many different ways, 
	// so encapsulate the commodity code which they all typically need such as 
	// a consistent description, string conversion, size inspection, etc.

	enum FORMAT
	{
		UNKNOWN,
		R32G32B32A32_TYPELESS,
		R32G32B32A32_FLOAT,
		R32G32B32A32_UINT,
		R32G32B32A32_SINT,
		R32G32B32_TYPELESS,
		R32G32B32_FLOAT,
		R32G32B32_UINT,
		R32G32B32_SINT,
		R16G16B16A16_TYPELESS,
		R16G16B16A16_FLOAT,
		R16G16B16A16_UNORM,
		R16G16B16A16_UINT,
		R16G16B16A16_SNORM,
		R16G16B16A16_SINT,
		R32G32_TYPELESS,
		R32G32_FLOAT,
		R32G32_UINT,
		R32G32_SINT,
		R32G8X24_TYPELESS,
		D32_FLOAT_S8X24_UINT,
		R32_FLOAT_X8X24_TYPELESS,
		X32_TYPELESS_G8X24_UINT,
		R10G10B10A2_TYPELESS,
		R10G10B10A2_UNORM,
		R10G10B10A2_UINT,
		R11G11B10_FLOAT,
		R8G8B8A8_TYPELESS,
		R8G8B8A8_UNORM,
		R8G8B8A8_UNORM_SRGB,
		R8G8B8A8_UINT,
		R8G8B8A8_SNORM,
		R8G8B8A8_SINT,
		R16G16_TYPELESS,
		R16G16_FLOAT,
		R16G16_UNORM,
		R16G16_UINT,
		R16G16_SNORM,
		R16G16_SINT,
		R32_TYPELESS,
		D32_FLOAT,
		R32_FLOAT,
		R32_UINT,
		R32_SINT,
		R24G8_TYPELESS,
		D24_UNORM_S8_UINT,
		R24_UNORM_X8_TYPELESS,
		X24_TYPELESS_G8_UINT,
		R8G8_TYPELESS,
		R8G8_UNORM,
		R8G8_UINT,
		R8G8_SNORM,
		R8G8_SINT,
		R16_TYPELESS,
		R16_FLOAT,
		D16_UNORM,
		R16_UNORM,
		R16_UINT,
		R16_SNORM,
		R16_SINT,
		R8_TYPELESS,
		R8_UNORM,
		R8_UINT,
		R8_SNORM,
		R8_SINT,
		A8_UNORM,
		R1_UNORM,
		R9G9B9E5_SHAREDEXP,
		R8G8_B8G8_UNORM,
		G8R8_G8B8_UNORM,
		BC1_TYPELESS,
		BC1_UNORM,
		BC1_UNORM_SRGB,
		BC2_TYPELESS,
		BC2_UNORM,
		BC2_UNORM_SRGB,
		BC3_TYPELESS,
		BC3_UNORM,
		BC3_UNORM_SRGB,
		BC4_TYPELESS,
		BC4_UNORM,
		BC4_SNORM,
		BC5_TYPELESS,
		BC5_UNORM,
		BC5_SNORM,
		B5G6R5_UNORM,
		B5G5R5A1_UNORM,
		B8G8R8A8_UNORM,
		B8G8R8X8_UNORM,
		R10G10B10_XR_BIAS_A2_UNORM,
		B8G8R8A8_TYPELESS,
		B8G8R8A8_UNORM_SRGB,
		B8G8R8X8_TYPELESS,
		B8G8R8X8_UNORM_SRGB,
		BC6H_TYPELESS,
		BC6H_UF16,
		BC6H_SF16,
		BC7_TYPELESS,
		BC7_UNORM,
		BC7_UNORM_SRGB,
//formats below here are not currently directly loadable to directx.
		R8G8B8_UNORM,
// YUV420 is sequential instead of interleaved. U and V are stored at quarter resolution
		YUV420_UNORM, 
		NUM_FORMATS,
	};

	struct DESC
	{
		DESC()
			: Width(0)
			, Height(0)
			, Depth(0)
			, NumMips(0)
			, RowPitch(0)
			, DepthPitch(0)
			, Format(UNKNOWN)
		{}

		unsigned int Width;
		unsigned int Height;
		
		// Depth can be rephrased as "array of 2D slices"
		union
		{
			unsigned int Depth; // 1 is the count of a 2D image
			unsigned int NumSlices; // 1 is the count of a singe mip chain
		};

		unsigned int NumMips; // 1 is the count of an image with no mips
		unsigned int RowPitch;
		
		union
		{
			unsigned int DepthPitch;
			unsigned int SlicePitch;
		};

		FORMAT Format;
	};

	// Use oAsString for FORMAT to string conversion
	FORMAT GetFromString(const char* _EnumString);

	int GetPlatformFormat(FORMAT _Format);
	template<typename T> inline T GetPlatformFormat(FORMAT _Format) { return static_cast<T>(GetPlatformFormat(_Format)); }
	
	// Returns true if the specified format is a block-compressed format.
	bool IsBlockCompressedFormat(FORMAT _Format);

	// Returns true if the specified format is one typically used to write 
	// Z-buffer depth information.
	bool IsDepthFormat(FORMAT _Format);

	// Returns true if the specified format is normalized between 0.0f and 1.0f
	bool IsUNORM(FORMAT _Format);

	// Returns the number of separate channels used for a pixel. For example RGB 
	// has 3 channels, XRGB has 4, RGBA has 4.
	unsigned int GetNumChannels(FORMAT _Format);

	// Returns the number of bytes required to store the smallest atom of a 
	// surface. For single-bit image formats, this will return 1. For tiled 
	// formats this will return the byte size of 1 tile.
	unsigned int GetSize(FORMAT _Format);

	// Get number of bits per format. This includes any X bits as described in
	// the format enum.
	unsigned int GetBitSize(FORMAT _Format);

	// Returns the size in bytes for one row for the described mip chain. 
	// This does a bit more than a simple multiply because it takes into 
	// consideration block compressed formats.
	unsigned int CalcRowPitch(FORMAT _Format, unsigned int _Mip0Width, unsigned int _MipLevel = 0);

	// Returns the size in bytes for one mip level for the described mip chain
	unsigned int CalcLevelPitch(FORMAT _Format, unsigned int _Mip0Width, unsigned int _Mip0Height, unsigned int _MipLevel = 0);

	// Returns the size in bytes for one 2D array slice for the described mip chain.
	// That is, in an array of textures, each texture is a full mip chain, and this
	// pitch is the number of bytes for the total mip chain of one of those textures.
	unsigned int CalcSlicePitch(FORMAT _Format, unsigned int _Mip0Width, unsigned int _Mip0Height, unsigned int _NumMips = 0);

	// Returns the width, height, or depth dimension of the specified mip level
	// given mip0's dimension. BCAlign set to true will align the dimension to be 
	// compatible with 4x4 block compression. All dimensions must be a power of 2.
	unsigned int CalcMipDimension(FORMAT _Format, unsigned int _Mip0Dimension, unsigned int _MipLevel = 0);

	// Returns the number of rows in a mip with the specified height in pixels.
	// Block compressed formats have 1/4 the rows size their pitch includes 4 rows
	// at a time.
	unsigned int CalcNumRows(FORMAT _Format, unsigned int _Mip0Height, unsigned int _MipLevel = 0);

	// Returns the number of mip levels for the specified dimensions
	unsigned int CalcMipCount(unsigned int _Mip0Width, unsigned int _Mip0Height);

	// Returns the size necessary for all mip levels for the specified dimensions.
	unsigned int CalcBufferSize(FORMAT _Format, unsigned int _Mip0Width, unsigned int _Mip0Height, unsigned int _NumSlices);

	// Returns the number of bytes required to contain the subresource (mip level
	// from a particular slice)
	unsigned int CalcSubresourceSize(FORMAT _Format, unsigned int _Mip0Width, unsigned int _Mip0Height, unsigned int _NumSlices, unsigned int _Subresource);

	// Returns the number of bytes into a mip chain where the specified level begins.
	unsigned int CalcLevelOffset(FORMAT _Format, unsigned int _Mip0Width, unsigned int _Mip0Height, unsigned int _MipLevel = 0);

	// Returns the number of bytes into a buffer where the specified subresource 
	// (mip level from a particular slice) begins.
	unsigned int CalcBufferOffset(FORMAT _Format, unsigned int _Mip0Width, unsigned int _Mip0Height, unsigned int _NumSlices, unsigned int _Subresource);

	// To simplify the need for much of the above API, interfaces should be 
	// developed that take a subresource id and internally use these API to
	// translate that into the proper byte locations and sizes. 
	inline unsigned int CalcSubresource(unsigned int _MipLevel, unsigned int _SliceIndex, unsigned int _NumMips) { return _MipLevel + (_SliceIndex * _NumMips); }

	// Given a known number of mips, convert a subresource back into slice and 
	// mip level 
	inline void UnpackSubresource(unsigned int _Subresource, unsigned int _NumMips, unsigned int* _pMipLevel, unsigned int* _pSliceIndex)
	{
		*_pSliceIndex = _Subresource / _NumMips;
		*_pMipLevel = _Subresource % _NumMips;
	}

	unsigned int CalculateSize(const DESC* _pDesc);

	struct YUV420
	{
		YUV420() : pY(nullptr), pU(nullptr), pV(nullptr), YPitch(0), UVPitch(0) {}
		unsigned char* pY; // Luminance at full resolution
		unsigned char* pU; // Chrominance U at quarter resolution
		unsigned char* pV; // Chrominance V at quarter resolution

		size_t YPitch; // Luminance (Y) pitch
		size_t UVPitch; // Chrominance (UV) pitch
	};
	
	void convert_B8G8R8_UNORM_to_YUV420(const unsigned int _Width, const unsigned int _Height, const unsigned char* _pSrcRGBSrc, const size_t _pRGBPitch, YUV420* _pYUVDst, bool _yflip = false);
	void convert_B8G8R8A8_UNORM_to_YUV420(const unsigned int _Width, const unsigned int _Height, const unsigned char* _pSrcRGBASrc, const size_t _pRGBAPitch, YUV420* _pYUVDst, bool _yflip = false);
	void convert_YUV420_to_B8G8R8A8_UNORM( const unsigned int _Width, const unsigned int _Height, const YUV420& _YUVSrc, unsigned char* _pSrcRGBADst, const size_t _pRGBAPitch );


	bool ConvertSurface(const DESC& _srcDesc, const unsigned char* _pSrcSurface, const DESC& _dstDesc, unsigned char* _pDstSurface);
} // namespace oSurface

#endif
