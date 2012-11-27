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
#include <oBasis/oSurface.h>
#include <oBasis/oAssert.h>
#include <oBasis/oMacros.h>
#include <oBasis/oMemory.h>
#include <oBasis/oMath.h>
#include <oBasis/oString.h>
#include <cstring>

#define oCHECK_SURFACE_DESC(_Desc) \
	oASSERT(greater_than_equal(_Desc.Dimensions, int3(1,1,1)), "invalid dimensions: [%d,%d,%d]", _Desc.Dimensions.x, _Desc.Dimensions.y, _Desc.Dimensions.z); \
	oASSERT(_Desc.NumSlices == 1 || _Desc.Dimensions.z == 1, "NumSlices or Depth has to be 1 [%d,%d]", _Desc.NumSlices, _Desc.Dimensions.z);

#define oCHECK_DIM(_Dim) oASSERT(_Dim >= 0, "invalid dimension: %d", _Dim);
#define oCHECK_DIM2(_Dim) oASSERT(_Dim.x >= 0 && _Dim.y >= 0, "invalid dimensions: [%d,%d]", _Dim.x, _Dim.y);
#define oCHECK_DIM3(_Dim) oASSERT(_Dim.x >= 0 && _Dim.y >= 0 && _Dim.z >= 0, "invalid dimensions: [%d,%d,%d]", _Dim.x, _Dim.y, _Dim.z);

struct BIT_SIZE
{
	unsigned char R;
	unsigned char G;
	unsigned char B;
	unsigned char A;
};

struct FORMAT_DESC
{
	const char* String;
	unsigned int FourCC;
	unsigned int Size;
	int2 MinDimensions;
	BIT_SIZE BitSize;
	unsigned short NumChannels;
	bool IsBlockCompressed;
	bool IsUNORM;
	bool HasAlpha;
};

static const FORMAT_DESC sFormatDescs[] = 
{
	{ "UNKNOWN", '????', 0, int2(1, 1), {0,0,0,0}, 0, false, false, false },
	{ "R32G32B32A32_TYPELESS", '?i4 ', 4 * sizeof(float), int2(1, 1), {32,32,32,32}, 4, false, false, true },
	{ "R32G32B32A32_FLOAT", 'f4  ', 4 * sizeof(float), int2(1, 1), {32,32,32,32}, 4, false, false, true },
	{ "R32G32B32A32_UINT", 'ui4 ', 4 * sizeof(int), int2(1, 1), {32,32,32,32}, 4, false, false, true },
	{ "R32G32B32A32_SINT", 'si4 ', 4 * sizeof(int), int2(1, 1), {32,32,32,32}, 4, false, false, true },
	{ "R32G32B32_TYPELESS", '?i3 ', 3 * sizeof(int), int2(1, 1), {32,32,32,0}, 3, false, false, true },
	{ "R32G32B32_FLOAT", 'f3  ', 3 * sizeof(float), int2(1, 1), {32,32,32,0}, 3, false, false, false },
	{ "R32G32B32_UINT", 'ui3 ', 4 * sizeof(int), int2(1, 1), {32,32,32,0}, 3, false, false, false },
	{ "R32G32B32_SINT", 'si3 ', 4 * sizeof(int), int2(1, 1), {32,32,32,0}, 3, false, false, false },
	{ "R16G16B16A16_TYPELESS", '?s4 ', 4 * sizeof(short), int2(1, 1), {16,16,16,16}, 4, false, false, true },
	{ "R16G16B16A16_FLOAT", 'h4  ', 4 * sizeof(short), int2(1, 1), {16,16,16,16}, 4, false, false, true },
	{ "R16G16B16A16_UNORM", 'h4u ', 4 * sizeof(short), int2(1, 1), {16,16,16,16}, 4, false, true, true },
	{ "R16G16B16A16_UINT", 'us4 ', 4 * sizeof(short), int2(1, 1), {16,16,16,16}, 4, false, false, true },
	{ "R16G16B16A16_SNORM", 'h4s ', 4 * sizeof(short), int2(1, 1), {16,16,16,16}, 4, false, false, true },
	{ "R16G16B16A16_SINT", 'ss4 ', 4 * sizeof(short), int2(1, 1), {16,16,16,16}, 4, false, false, true },
	{ "R32G32_TYPELESS", '?i2 ', 2 * sizeof(float), int2(1, 1), {32,32,0,0}, 2, false, false, false },
	{ "R32G32_FLOAT", 'f2  ', 2 * sizeof(float), int2(1, 1), {32,32,0,0}, 2, false, false, false },
	{ "R32G32_UINT", 'ui2 ', 2 * sizeof(int), int2(1, 1), {32,32,0,0}, 2, false, false, false },
	{ "R32G32_SINT", 'si2 ', 2 * sizeof(int), int2(1, 1), {32,32,0,0}, 2, false, false, false },
	{ "R32G8X24_TYPELESS", '????', 2 * sizeof(float), int2(1, 1), {32,8,0,24}, 3, false, false, false },
	{ "D32_FLOAT_S8X24_UINT", '????', 2 * sizeof(int), int2(1, 1), {32,8,0,24}, 3, false, false, false },
	{ "R32_FLOAT_X8X24_TYPELESS", '????', 2 * sizeof(float), int2(1, 1), {32,8,0,24}, 3, false, false, false },
	{ "X32_TYPELESS_G8X24_UINT", '????', 2 * sizeof(int), int2(1, 1), {32,8,0,24}, 3, false, false, false },
	{ "R10G10B10A2_TYPELESS", '????', sizeof(int), int2(1, 1), {10,10,10,2}, 4, false, false, true },
	{ "R10G10B10A2_UNORM", '????', sizeof(int), int2(1, 1), {10,10,10,2}, 4, false, true, true },
	{ "R10G10B10A2_UINT", '????', sizeof(int), int2(1, 1), {10,10,10,2}, 4, false, false, true },
	{ "R11G11B10_FLOAT", '????', sizeof(float), int2(1, 1), {11,11,11,0}, 3, false, false, false },
	{ "R8G8B8A8_TYPELESS", '?c4 ', sizeof(int), int2(1, 1), {8,8,8,8}, 4, false, false, true },
	{ "R8G8B8A8_UNORM", 'c4u ', sizeof(int), int2(1, 1), {8,8,8,8}, 4, false, true, true },
	{ "R8G8B8A8_UNORM_SRGB", 'c4us', sizeof(int), int2(1, 1), {8,8,8,8}, 4, false, true, true },
	{ "R8G8B8A8_UINT", 'uc4 ', sizeof(int), int2(1, 1), {8,8,8,8}, 4, false, false, true },
	{ "R8G8B8A8_SNORM", 'c4s ', sizeof(int), int2(1, 1), {8,8,8,8}, 4, false, false, true },
	{ "R8G8B8A8_SINT", 'sc4 ', sizeof(int), int2(1, 1), {8,8,8,8}, 4, false, false, true },
	{ "R16G16_TYPELESS", '?s2 ', 2 * sizeof(short), int2(1, 1), {16,16,0,0}, 2, false, false, false },
	{ "R16G16_FLOAT", 'h2 ', 2 * sizeof(short), int2(1, 1), {16,16,0,0}, 2, false, false, false },
	{ "R16G16_UNORM", 'h2u ', 2 * sizeof(short), int2(1, 1), {16,16,0,0}, 2, false, true, false },
	{ "R16G16_UINT", 'us2 ', 2 * sizeof(short), int2(1, 1), {16,16,0,0}, 2, false, false, false },
	{ "R16G16_SNORM", 'h2s ', 2 * sizeof(short), int2(1, 1), {16,16,0,0}, 2, false, false, false },
	{ "R16G16_SINT", 'ss2 ', 2 * sizeof(short), int2(1, 1), {16,16,0,0}, 2, false, false, false },
	{ "R32_TYPELESS", '?i1 ', sizeof(float), int2(1, 1), {32,0,0,0}, 1, false, false, false },
	{ "D32_FLOAT", 'f1d ', sizeof(float), int2(1, 1), {32,0,0,0}, 1, false, false, false },
	{ "R32_FLOAT", 'f1  ', sizeof(float), int2(1, 1), {32,0,0,0}, 1, false, false, false },
	{ "R32_UINT", 'ui1 ', sizeof(int), int2(1, 1), {32,0,0,0}, 1, false, false, false },
	{ "R32_SINT", 'si1 ', sizeof(int), int2(1, 1), {32,0,0,0}, 1, false, false, false },
	{ "R24G8_TYPELESS", '????', sizeof(float), int2(1, 1), {24,8,0,0}, 2, false, false, false },
	{ "D24_UNORM_S8_UINT", '????', sizeof(float), int2(1, 1), {24,8,0,0}, 2, false, true, false },
	{ "R24_UNORM_X8_TYPELESS", '????', sizeof(float), int2(1, 1), {24,8,0,0}, 2, false, true, false },
	{ "X24_TYPELESS_G8_UINT", '????', sizeof(float), int2(1, 1), {24,8,0,0}, 2, false, false, false },
	{ "R8G8_TYPELESS", '?c2 ', 2 * sizeof(char), int2(1, 1), {8,8,0,0}, 2, false, false, false },
	{ "R8G8_UNORM", 'uc2u', 2 * sizeof(char), int2(1, 1), {8,8,0,0}, 2, false, true, false },
	{ "R8G8_UINT", 'ui2 ', 2 * sizeof(char), int2(1, 1), {8,8,0,0}, 2, false, false, false },
	{ "R8G8_SNORM", 'uc2s', 2 * sizeof(char), int2(1, 1), {8,8,0,0}, 2, false, false, false },
	{ "R8G8_SINT", 'si2 ', 2 * sizeof(char), int2(1, 1), {8,8,0,0}, 2, false, false, false },
	{ "R16_TYPELESS", '?s1 ', sizeof(short), int2(1, 1), {16,0,0,0}, 1, false, false, false },
	{ "R16_FLOAT", 'h1  ', sizeof(short), int2(1, 1), {16,0,0,0}, 1, false, false, false },
	{ "D16_UNORM", 'h1ud', sizeof(short), int2(1, 1), {16,0,0,0}, 1, false, true, false },
	{ "R16_UNORM", 'h1u ', sizeof(short), int2(1, 1), {16,0,0,0}, 1, false, true, false },
	{ "R16_UINT", 'us1 ', sizeof(short), int2(1, 1), {16,0,0,0}, 1, false, false, false },
	{ "R16_SNORM", 'h1s ', sizeof(short), int2(1, 1), {16,0,0,0}, 1, false, false, false },
	{ "R16_SINT", 'ss1 ', sizeof(short), int2(1, 1), {16,0,0,0}, 1, false, false, false },
	{ "R8_TYPELESS", '?c1 ', sizeof(char), int2(1, 1), {8,0,0,0}, 1, false, false, false },
	{ "R8_UNORM", 'uc1u', sizeof(char), int2(1, 1), {8,0,0,0}, 1, false, true, false },
	{ "R8_UINT", 'uc1 ', sizeof(char), int2(1, 1), {8,0,0,0}, 1, false, false, false },
	{ "R8_SNORM", 'uc1s', sizeof(char), int2(1, 1), {8,0,0,0}, 1, false, false, false },
	{ "R8_SINT", 'sc1 ', sizeof(char), int2(1, 1), {8,0,0,0}, 1, false, false, false },
	{ "A8_UNORM", 'ac1u', sizeof(char), int2(1, 1), {8,0,0,0}, 1, false, true, true },
	{ "R1_UNORM", 'bitu', 1, int2(1, 1), {1,0,0,0}, 1, false, true, false },
	{ "R9G9B9E5_SHAREDEXP", '????', sizeof(int), int2(1, 1), {9,9,9,5}, 4, false, false, false },
	{ "R8G8_B8G8_UNORM", '????', sizeof(int), int2(1, 1), {8,8,8,8}, 4, false, true, false },
	{ "G8R8_G8B8_UNORM", '????', sizeof(int), int2(1, 1), {8,8,8,8}, 4, false, true, false },
	{ "BC1_TYPELESS", 'BC1?', 8, int2(4, 4), {5,6,5,0}, 3, true, false, false },
	{ "BC1_UNORM", 'BC1u', 8, int2(4, 4), {5,6,5,0}, 3, true, true, false },
	{ "BC1_UNORM_SRGB", 'BC1s', 8, int2(4, 4), {5,6,5,0}, 3, true, true, false },
	{ "BC2_TYPELESS", 'BC2?', 16, int2(4, 4), {5,6,5,4}, 4, true, false, true },
	{ "BC2_UNORM", 'BC2u', 16, int2(4, 4), {5,6,5,4}, 4, true, true, true },
	{ "BC2_UNORM_SRGB", 'BC2s', 16, int2(4, 4), {5,6,5,4}, 4, true, true, true },
	{ "BC3_TYPELESS", 'BC3?', 16, int2(4, 4), {5,6,5,8}, 4, true, false, true },
	{ "BC3_UNORM", 'BC3u', 16, int2(4, 4), {5,6,5,8}, 4, true, true, true },
	{ "BC3_UNORM_SRGB", 'BC3s', 16, int2(4, 4), {5,6,5,8}, 4, true, true, true },
	{ "BC4_TYPELESS", 'BC4?', 8, int2(4, 4), {8,0,0,0}, 1, true, false, false },
	{ "BC4_UNORM", 'BC4u', 8, int2(4, 4), {8,0,0,0}, 1, true, true, false },
	{ "BC4_SNORM", 'BC4s', 8, int2(4, 4), {8,0,0,0}, 1, true, false, false },
	{ "BC5_TYPELESS", 'BC5?', 16, int2(4, 4), {8,8,0,0}, 2, true, false, false },
	{ "BC5_UNORM", 'BC5u', 16, int2(4, 4), {8,8,0,0}, 2, true, true, false },
	{ "BC5_SNORM", 'BC5s', 16, int2(4, 4), {8,8,0,0}, 2, true, false, false },
	{ "B5G6R5_UNORM", '????', sizeof(short), int2(1, 1), {5,6,5,0}, 3, false, true, false },
	{ "B5G5R5A1_UNORM", '????', sizeof(short), int2(1, 1), {5,6,5,1}, 4, false, true, true },
	{ "B8G8R8A8_UNORM", 'c4u ', sizeof(int), int2(1, 1), {8,8,8,8}, 4, false, true, true },
	{ "B8G8R8X8_UNORM", 'c4u ', sizeof(int), int2(1, 1), {8,8,8,8}, 4, false, true, false },
	{ "R10G10B10_XR_BIAS_A2_UNORM", '????', sizeof(int), 4, false, true, false },
	{ "B8G8R8A8_TYPELESS", '????', sizeof(int), int2(1, 1), {8,8,8,8}, 4, false, false },
	{ "B8G8R8A8_UNORM_SRGB", '????', sizeof(int), int2(1, 1), {8,8,8,8}, 4, false, true },
	{ "B8G8R8X8_TYPELESS", '????', sizeof(int), int2(1, 1), {8,8,8,8}, 4, false, false },
	{ "B8G8R8X8_UNORM_SRGB", '????', sizeof(int), int2(1, 1), {8,8,8,8}, 4, false, true },
	{ "BC6H_TYPELESS", 'BC6?', 16, int2(4, 4), {0,0,0,0}, 3, true, false, false },
	{ "BC6H_UF16", 'BC6u', 16, int2(4, 4), {0,0,0,0}, 3, true, false, false },
	{ "BC6H_SF16", 'BC6s', 16, int2(4, 4), {0,0,0,0}, 3, true, false, false },
	{ "BC7_TYPELESS", 'BC7?', 16, int2(4, 4), {0,0,0,0}, 4, true, false, true },
	{ "BC7_UNORM", 'BC7u', 16, int2(4, 4), {0,0,0,0}, 4, true, true, true },
	{ "BC7_UNORM_SRGB", 'BC7s', 16, int2(4, 4), {0,0,0,0}, 4, true, true, true },
	{ "R8G8B8_UNORM", 'c3u ', 3 * sizeof(char), int2(1, 1), {8,8,8,0}, 3, false, true, false },
	{ "B8G8R8_UNORM", 'c3u ', 3 * sizeof(char), int2(1, 1), {8,8,8,0}, 3, false, true, false },
};
static_assert(oCOUNTOF(sFormatDescs) == oSURFACE_NUM_FORMATS, "");

const char* oAsString(oSURFACE_FORMAT _Format)
{
	return (_Format < oSURFACE_NUM_FORMATS) ? sFormatDescs[_Format].String : "UNKNOWN";
}

char* oToString(char* _StrDestination, size_t _SizeofStrDestination, const oSURFACE_FORMAT& _Format)
{
	return oStrcpy(_StrDestination, _SizeofStrDestination, oAsString(_Format));
}

bool oFromString(oSURFACE_FORMAT* _pFormat, const char* _StrSource)
{
	*_pFormat = oSURFACE_UNKNOWN;
	for (size_t i = 0; i < oCOUNTOF(sFormatDescs); i++)
	{
		if (!oStricmp(_StrSource, sFormatDescs[i].String))
		{
			*_pFormat = (oSURFACE_FORMAT)i;
			return true;
		}
	}
	return false;
}

bool oSurfaceFormatIsBlockCompressed(oSURFACE_FORMAT _Format)
{
	return (_Format < oSURFACE_NUM_FORMATS) ? sFormatDescs[_Format].IsBlockCompressed : false;
}

bool oSurfaceFormatIsDepth(oSURFACE_FORMAT _Format)
{
	bool result = false;

	switch (_Format)
	{
		case oSURFACE_D32_FLOAT:
		case oSURFACE_R32_TYPELESS:
		case oSURFACE_R24G8_TYPELESS:
		case oSURFACE_D24_UNORM_S8_UINT:
		case oSURFACE_R24_UNORM_X8_TYPELESS:
		case oSURFACE_D32_FLOAT_S8X24_UINT:
		case oSURFACE_R32_FLOAT_X8X24_TYPELESS:
		case oSURFACE_D16_UNORM:
		case oSURFACE_R16_TYPELESS:
			result = true;
		default:
			break;
	}

	return result;
}

bool oSurfaceFormatIsAlpha(oSURFACE_FORMAT _Format)
{
	return (_Format < oSURFACE_NUM_FORMATS) ? sFormatDescs[_Format].HasAlpha : false;
}

bool oSurfaceFormatIsUNORM(oSURFACE_FORMAT _Format)
{
	return (_Format < oSURFACE_NUM_FORMATS) ? sFormatDescs[_Format].IsUNORM : false;
}

int oSurfaceFormatGetNumChannels(oSURFACE_FORMAT _Format)
{
	return (_Format < oSURFACE_NUM_FORMATS) ? sFormatDescs[_Format].NumChannels : 0;
}

int oSurfaceFormatGetSize(oSURFACE_FORMAT _Format)
{
	return (_Format < oSURFACE_NUM_FORMATS) ? sFormatDescs[_Format].Size : 0;
}

int2 oSurfaceFormatGetMinDimensions(oSURFACE_FORMAT _Format)
{
	return (_Format < oSURFACE_NUM_FORMATS) ? sFormatDescs[_Format].MinDimensions : int2(0, 0);
}

int oSurfaceFormatGetBitSize(oSURFACE_FORMAT _Format)
{
	if (_Format == oSURFACE_R1_UNORM) return 1;
	return 8 * oSurfaceFormatGetSize(_Format);
}

void oSurfaceGetChannelBitSize(oSURFACE_FORMAT _Format, int* _pNBitsR, int* _pNBitsG, int* _pNBitsB, int* _pNBitsA)
{
	if (_Format < oSURFACE_NUM_FORMATS)
	{
		const BIT_SIZE& b = sFormatDescs[_Format].BitSize;
		*_pNBitsR = b.G; *_pNBitsG = b.G; *_pNBitsB = b.B; *_pNBitsA = b.A;
	}

	else
		*_pNBitsR = *_pNBitsG = *_pNBitsB = *_pNBitsA = 0;
}

int oSurfaceCalcNumMips(oSURFACE_LAYOUT _Layout, const int3& _Mip0Dimensions)
{
	// Rules of mips are to go to 1x1... so a 1024x8 texture has many more than 4
	// mips.

	if (_Mip0Dimensions.x <= 0 || _Mip0Dimensions.y <= 0 || _Mip0Dimensions.z <= 0)
		return 0;

	int nMips = 1;
	int3 mip = _Mip0Dimensions;
	while (_Layout != oSURFACE_LAYOUT_IMAGE && mip != int3(1,1,1))
	{
		nMips++;
		mip = max(int3(1,1,1), mip / int3(2,2,2));
	}

	return nMips;
}

int oSurfaceCalcNumMips(oSURFACE_LAYOUT _Layout, const int2& _Mip0Dimensions)
{
	return oSurfaceCalcNumMips(_Layout, int3(_Mip0Dimensions, 1));
}

int oSurfaceMipCalcDimension(oSURFACE_FORMAT _Format, int _Mip0Dimension, int _MipLevel)
{
	oCHECK_DIM(_Mip0Dimension);
	oASSERT(_Format != oSURFACE_UNKNOWN, "Unknown surface format passed to CalcMipDimension");
	oASSERT(_MipLevel == 0 || oIsPow2(_Mip0Dimension), "Mipchain dimensions must be a power of 2");
	int d = max(1, _Mip0Dimension >> _MipLevel);
	return oSurfaceFormatIsBlockCompressed(_Format) ? static_cast<int>(oByteAlign(d, 4)) : d;
}

int2 oSurfaceMipCalcDimensions(oSURFACE_FORMAT _Format, const int2& _Mip0Dimensions, int _MipLevel)
{
	return int2(
		oSurfaceMipCalcDimension(_Format, _Mip0Dimensions.x, _MipLevel)
		, oSurfaceMipCalcDimension(_Format, _Mip0Dimensions.y, _MipLevel));
}

int3 oSurfaceMipCalcDimensions(oSURFACE_FORMAT _Format, const int3& _Mip0Dimensions, int _MipLevel)
{
	return int3(
		oSurfaceMipCalcDimension(_Format, _Mip0Dimensions.x, _MipLevel)
		, oSurfaceMipCalcDimension(_Format, _Mip0Dimensions.y, _MipLevel)
		, oSurfaceMipCalcDimension(oSURFACE_R32_UINT, _Mip0Dimensions.z, _MipLevel)); // No block-compression alignment for depth
}

int oSurfaceMipCalcDimensionNPOT(oSURFACE_FORMAT _Format, int _Mip0Dimension, int _MipLevel)
{
	oCHECK_DIM(_Mip0Dimension);
	int d = max(1, _Mip0Dimension >> _MipLevel);
	return oSurfaceFormatIsBlockCompressed(_Format) ? static_cast<int>(oByteAlign(d, 4)) : d;
}

int2 oSurfaceMipCalcDimensionsNPOT(oSURFACE_FORMAT _Format, const int2& _Mip0Dimensions, int _MipLevel)
{
	return int2(
		oSurfaceMipCalcDimensionNPOT(_Format, _Mip0Dimensions.x, _MipLevel)
		, oSurfaceMipCalcDimensionNPOT(_Format, _Mip0Dimensions.y, _MipLevel));
}

int3 oSurfaceMipCalcDimensionsNPOT(oSURFACE_FORMAT _Format, const int3& _Mip0Dimensions, int _MipLevel)
{
	return int3(
		oSurfaceMipCalcDimensionNPOT(_Format, _Mip0Dimensions.x, _MipLevel)
		, oSurfaceMipCalcDimensionNPOT(_Format, _Mip0Dimensions.y, _MipLevel)
		, oSurfaceMipCalcDimensionNPOT(oSURFACE_R32_UINT, _Mip0Dimensions.z, _MipLevel)); // No block-compression alignment for depth
}

int oSurfaceMipCalcRowSize(oSURFACE_FORMAT _Format, int _MipWidth)
{
	oCHECK_DIM(_MipWidth);
	oASSERT(_Format != oSURFACE_UNKNOWN, "Unknown surface format passed to GetRowPitch");
	int w = oSurfaceMipCalcDimension(_Format, _MipWidth);
	if (oSurfaceFormatIsBlockCompressed(_Format)) // because the atom is a 4x4 block
		w /= 4;
	int s = oSurfaceFormatGetSize(_Format);
	return oInt(oByteAlign(w * s, sFormatDescs[_Format].Size));
}

int oSurfaceMipCalcRowPitch(const oSURFACE_DESC& _SurfaceDesc, int _MipLevel)
{
	oCHECK_SURFACE_DESC(_SurfaceDesc);
	const int numMips = oSurfaceCalcNumMips(_SurfaceDesc.Layout, _SurfaceDesc.Dimensions);
	if (_MipLevel >= numMips)
		return oInvalid;

	switch (_SurfaceDesc.Layout)
	{
		case oSURFACE_LAYOUT_IMAGE: 
			return oSurfaceMipCalcRowSize(_SurfaceDesc.Format, _SurfaceDesc.Dimensions);
		case oSURFACE_LAYOUT_TIGHT: 
			return oSurfaceMipCalcRowSize(_SurfaceDesc.Format, oSurfaceMipCalcDimensionNPOT(_SurfaceDesc.Format, _SurfaceDesc.Dimensions.x, _MipLevel));
		case oSURFACE_LAYOUT_BELOW: 
			{
				const int mip0RowSize = oSurfaceMipCalcRowSize(_SurfaceDesc.Format, _SurfaceDesc.Dimensions.x);
				if (numMips > 2)
				{
					return max(mip0RowSize, 
						oSurfaceMipCalcRowSize(_SurfaceDesc.Format, oSurfaceMipCalcDimensionNPOT(_SurfaceDesc.Format, _SurfaceDesc.Dimensions.x, 1)) + 
						oSurfaceMipCalcRowSize(_SurfaceDesc.Format, oSurfaceMipCalcDimensionNPOT(_SurfaceDesc.Format, _SurfaceDesc.Dimensions.x, 2)) );
				}
				else
					return mip0RowSize;
			}
		case oSURFACE_LAYOUT_RIGHT: 
			{
				const int mip0RowSize = oSurfaceMipCalcRowSize(_SurfaceDesc.Format, _SurfaceDesc.Dimensions.x);
				if (numMips > 1)
					return mip0RowSize + oSurfaceMipCalcRowSize(_SurfaceDesc.Format, oSurfaceMipCalcDimensionNPOT(_SurfaceDesc.Format, _SurfaceDesc.Dimensions.x, 1));
				else
					return mip0RowSize;
			}
		oNODEFAULT;
	}
}

int2 oSurfaceCalcDimensions(const oSURFACE_DESC& _SurfaceDesc)
{
	int2 sliceDimensions = oSurfaceSliceCalcDimensions(_SurfaceDesc);
	return int2(sliceDimensions.x, sliceDimensions.y * _SurfaceDesc.NumSlices);
}

int2 oSurfaceSliceCalcDimensions(const oSURFACE_DESC& _SurfaceDesc)
{
	oCHECK_SURFACE_DESC(_SurfaceDesc);
	int3 mip0dimensions = oSurfaceMipCalcDimensionsNPOT(_SurfaceDesc.Format, _SurfaceDesc.Dimensions, 0);
	switch (_SurfaceDesc.Layout)
	{
	case oSURFACE_LAYOUT_IMAGE: 
		{
			return int2(mip0dimensions.x, (mip0dimensions.y * mip0dimensions.z));
		}
	case oSURFACE_LAYOUT_TIGHT: 
		{
			const int surfaceSlicePitch = oSurfaceSliceCalcPitch(_SurfaceDesc);
			const int mip0RowPitch = oSurfaceMipCalcRowPitch(_SurfaceDesc, 0);
			return int2(mip0dimensions.x, (surfaceSlicePitch / mip0RowPitch));
		}
	case oSURFACE_LAYOUT_BELOW: 
		{
			int numMips = oSurfaceCalcNumMips(_SurfaceDesc.Layout, mip0dimensions);
			int3 mip1dimensions = numMips > 1 ? oSurfaceMipCalcDimensionsNPOT(_SurfaceDesc.Format, mip0dimensions, 1) : int3(0);
			int3 mip2dimensions = numMips > 2 ? oSurfaceMipCalcDimensionsNPOT(_SurfaceDesc.Format, mip0dimensions, 2) : int3(0);

			int mip0height = mip0dimensions.y * mip0dimensions.z;
			int mip1height = mip1dimensions.y * mip1dimensions.z;
			int mip2andUpHeight = mip2dimensions.y * mip2dimensions.z;
			for (int mip=3; mip<numMips; ++mip)
			{
				int3 mipNdimensions = oSurfaceMipCalcDimensionsNPOT(_SurfaceDesc.Format, mip0dimensions, mip);
				mip2andUpHeight += mipNdimensions.y * mipNdimensions.z;
			}
			return int2(max(mip0dimensions.x, mip1dimensions.x + mip2dimensions.x), (mip0height + max(mip1height, mip2andUpHeight)));
		}
	case oSURFACE_LAYOUT_RIGHT: 
		{
			int numMips = oSurfaceCalcNumMips(_SurfaceDesc.Layout, mip0dimensions);
			int3 mip1dimensions = numMips > 1 ? oSurfaceMipCalcDimensionsNPOT(_SurfaceDesc.Format, mip0dimensions, 1) : int3(0);

			int mip0height = mip0dimensions.y * mip0dimensions.z;
			int mip1andUpHeight = mip1dimensions.y * mip1dimensions.z;
			for (int mip=2; mip<numMips; ++mip)
			{
				int3 mipNdimensions = oSurfaceMipCalcDimensionsNPOT(_SurfaceDesc.Format, mip0dimensions, mip);
				mip1andUpHeight += mipNdimensions.y * mipNdimensions.z;
			}
			return int2(mip0dimensions.x + mip1dimensions.x, max(mip0height, mip1andUpHeight));
		}
	oNODEFAULT;
	}
}


int oSurfaceMipCalcNumColumns(oSURFACE_FORMAT _Format, int _MipWidth)
{
	oCHECK_DIM(_MipWidth);
	int widthInPixels = oSurfaceMipCalcDimension(_Format, _MipWidth);
	return oSurfaceFormatIsBlockCompressed(_Format) ? __max(1, widthInPixels/4) : widthInPixels;
}

int oSurfaceMipCalcNumRows(oSURFACE_FORMAT _Format, int _MipHeight)
{
	oCHECK_DIM(_MipHeight);
	int heightInPixels = oSurfaceMipCalcDimension(_Format, _MipHeight);
	return oSurfaceFormatIsBlockCompressed(_Format) ? __max(1, heightInPixels/4) : heightInPixels;
}

int oSurfaceMipCalcSize(oSURFACE_FORMAT _Format, const int2& _MipDimensions)
{
	oCHECK_DIM2(_MipDimensions);
	return oInt(oSurfaceMipCalcRowSize(_Format, _MipDimensions) * oSurfaceMipCalcNumRows(_Format, _MipDimensions));
}

static int oSurfaceMipCalcOffset_Tight(const oSURFACE_DESC& _SurfaceDesc, int _MipLevel)
{
	oCHECK_SURFACE_DESC(_SurfaceDesc);

	int3 mip0dimensions = _SurfaceDesc.Dimensions;

	oInt offset = 0;
	int mip = 0;
	while (mip != _MipLevel)
	{
		int3 previousMipDimensions = oSurfaceMipCalcDimensionsNPOT(_SurfaceDesc.Format, mip0dimensions, mip);
		offset += oSurfaceMipCalcSize(_SurfaceDesc.Format, previousMipDimensions);
		mip++;
	}

	return offset;
}

static int oSurfaceMipCalcOffset_Below(const oSURFACE_DESC& _SurfaceDesc, int _MipLevel)
{
	oCHECK_SURFACE_DESC(_SurfaceDesc);
	if (0 == _MipLevel)
		return 0;

	int3 mip0dimensions = _SurfaceDesc.Dimensions;
	int3 mip1dimensions = oSurfaceMipCalcDimensionsNPOT(_SurfaceDesc.Format, _SurfaceDesc.Dimensions, 1);
	int surfaceRowPitch = oSurfaceMipCalcRowPitch(_SurfaceDesc, 0);

	// Step down when moving from Mip0 to Mip1
	oInt offset = surfaceRowPitch * oSurfaceMipCalcNumRows(_SurfaceDesc.Format, mip0dimensions);
	if (1 == _MipLevel)
		return offset;

	// Step right when moving from Mip1 to Mip2
	offset += oSurfaceMipCalcRowSize(_SurfaceDesc.Format, mip1dimensions);

	// Step down for all of the other MIPs
	int mip = 2;
	while (mip != _MipLevel)
	{
		int3 previousMipDimensions = oSurfaceMipCalcDimensionsNPOT(_SurfaceDesc.Format, mip0dimensions, mip);
		offset += surfaceRowPitch * oSurfaceMipCalcNumRows(_SurfaceDesc.Format, previousMipDimensions);
		mip++;
	}		

	return offset;
}

static int oSurfaceMipCalcOffset_Right(const oSURFACE_DESC& _SurfaceDesc, int _MipLevel)
{
	oCHECK_SURFACE_DESC(_SurfaceDesc);
	if (0 == _MipLevel)
		return 0;

	int3 mip0dimensions = _SurfaceDesc.Dimensions;
	int surfaceRowPitch = oSurfaceMipCalcRowPitch(_SurfaceDesc, 0);

	// Step right when moving from Mip0 to Mip1
	oInt offset = oSurfaceMipCalcRowSize(_SurfaceDesc.Format, mip0dimensions);

	// Step down for all of the other MIPs
	int mip = 1;
	while (mip != _MipLevel)
	{
		int3 previousMipDimensions = oSurfaceMipCalcDimensionsNPOT(_SurfaceDesc.Format, mip0dimensions, mip);
		offset += surfaceRowPitch * oSurfaceMipCalcNumRows(_SurfaceDesc.Format, previousMipDimensions);
		mip++;
	}		

	return offset;
}

int oSurfaceMipCalcOffset(const oSURFACE_DESC& _SurfaceDesc, int _MipLevel)
{
	oCHECK_SURFACE_DESC(_SurfaceDesc);
	const int numMips = oSurfaceCalcNumMips(_SurfaceDesc.Layout, _SurfaceDesc.Dimensions);
	if (_MipLevel >= numMips) 
		return oInvalid;

	switch (_SurfaceDesc.Layout)
	{
		case oSURFACE_LAYOUT_IMAGE: return 0;
		case oSURFACE_LAYOUT_TIGHT: return oSurfaceMipCalcOffset_Tight(_SurfaceDesc, _MipLevel);
		case oSURFACE_LAYOUT_BELOW: return oSurfaceMipCalcOffset_Below(_SurfaceDesc, _MipLevel);
		case oSURFACE_LAYOUT_RIGHT: return oSurfaceMipCalcOffset_Right(_SurfaceDesc, _MipLevel);
		oNODEFAULT;
	}
}

int oSurfaceSliceCalcPitch(const oSURFACE_DESC& _SurfaceDesc)
{
	oCHECK_SURFACE_DESC(_SurfaceDesc);
	oInt pitch = 0;

	switch (_SurfaceDesc.Layout)
	{
		case oSURFACE_LAYOUT_IMAGE:
		case oSURFACE_LAYOUT_RIGHT:
		case oSURFACE_LAYOUT_BELOW:
			return oSurfaceMipCalcSize(_SurfaceDesc.Format, oSurfaceSliceCalcDimensions(_SurfaceDesc));

		case oSURFACE_LAYOUT_TIGHT:
		{
			// Sum the size of all mip levels
			int3 dimensions = _SurfaceDesc.Dimensions;
			int nMips = oSurfaceCalcNumMips(_SurfaceDesc.Layout, dimensions);
			while (nMips > 0)
			{
				pitch += oSurfaceMipCalcSize(_SurfaceDesc.Format, dimensions.xy()) * dimensions.z;
				dimensions = max(int3(1,1,1), dimensions / int3(2,2,2));
				nMips--;
			}

			// Align slicePitch to mip0RowPitch
			const int mip0RowPitch = oSurfaceMipCalcRowPitch(_SurfaceDesc, 0);
			pitch = (((pitch + (mip0RowPitch - 1)) / mip0RowPitch) * mip0RowPitch);
			break;
		}
		oNODEFAULT;
	}

	return pitch;
}

int oSurfaceMipCalcDepthPitch(const oSURFACE_DESC& _SurfaceDesc, int _MipLevel)
{
	oCHECK_SURFACE_DESC(_SurfaceDesc);

	int3 mipDimensions = oSurfaceMipCalcDimensionsNPOT(_SurfaceDesc.Format, _SurfaceDesc.Dimensions, _MipLevel);
	return oSurfaceMipCalcRowPitch(_SurfaceDesc, _MipLevel) * oSurfaceMipCalcNumRows(_SurfaceDesc.Format, mipDimensions.xy());
}

int oSurfaceCalcSize(const oSURFACE_DESC& _SurfaceDesc)
{
	return oInt(oSurfaceSliceCalcPitch(_SurfaceDesc) * _SurfaceDesc.NumSlices);
}

void oSurfaceSubresourceGetDesc(const oSURFACE_DESC& _SurfaceDesc, int _Subresource, oSURFACE_SUBRESOURCE_DESC* _pSubresourceDesc)
{
	oCHECK_SURFACE_DESC(_SurfaceDesc);
	int numMips = oSurfaceCalcNumMips(_SurfaceDesc.Layout, _SurfaceDesc.Dimensions);
	oSurfaceSubresourceUnpack(_Subresource, numMips, &_pSubresourceDesc->MipLevel, &_pSubresourceDesc->Slice);
	oASSERT(_pSubresourceDesc->Slice < _SurfaceDesc.NumSlices, "Slice index is out of range for the specified surface");
	_pSubresourceDesc->Dimensions = oSurfaceMipCalcDimensionsNPOT(_SurfaceDesc.Format, _SurfaceDesc.Dimensions, _pSubresourceDesc->MipLevel);
}

int oSurfaceSubresourceCalcSize(const oSURFACE_DESC& _SurfaceDesc, const oSURFACE_SUBRESOURCE_DESC& _SubresourceDesc)
{
	return oSurfaceMipCalcSize(_SurfaceDesc.Format, _SubresourceDesc.Dimensions);
}

int oSurfaceSubresourceCalcOffset(const oSURFACE_DESC& _SurfaceDesc, int _Subresource, int _DepthIndex)
{
	oASSERT(_DepthIndex < _SurfaceDesc.Dimensions.z, "Depth index is out of range");
	oSURFACE_SUBRESOURCE_DESC ssrd;
	oSurfaceSubresourceGetDesc(_SurfaceDesc, _Subresource, &ssrd);

	int offset = oSurfaceMipCalcOffset(_SurfaceDesc, ssrd.MipLevel);
	if (_DepthIndex)
		offset += oSurfaceMipCalcDepthPitch(_SurfaceDesc, ssrd.MipLevel) * _DepthIndex;
	else if (ssrd.Slice)
		offset += oSurfaceSliceCalcPitch(_SurfaceDesc) * ssrd.Slice;

	return offset;
}

int2 oSurfaceSubresourceCalcByteDimensions(const oSURFACE_DESC& _SurfaceDesc, int _Subresource)
{
	oSURFACE_SUBRESOURCE_DESC ssrd;
	oSurfaceSubresourceGetDesc(_SurfaceDesc, _Subresource, &ssrd);
	return int2(oSurfaceMipCalcRowSize(_SurfaceDesc.Format, ssrd.Dimensions.xy()), oSurfaceMipCalcNumRows(_SurfaceDesc.Format, ssrd.Dimensions.xy()));
}

int oSurfaceTileCalcBestFitMipLevel(const oSURFACE_DESC& _SurfaceDesc, const int2& _TileDimensions)
{
	oCHECK_SURFACE_DESC(_SurfaceDesc);
	if (oSURFACE_LAYOUT_IMAGE == _SurfaceDesc.Layout)
		return 0;

	int nthMip = 0;
	int3 mip = _SurfaceDesc.Dimensions;
	while (mip != int3(1,1,1))
	{
		if (less_than_equal(_SurfaceDesc.Dimensions.xy(), _TileDimensions))
			break;

		nthMip++;
		mip = max(int3(1,1,1), mip / int3(2,2,2));
	}

	return nthMip;
}

int oSurfaceMipCalcDimensionInTiles(int _MipDimension, int _TileDimension)
{
	int div = _MipDimension / _TileDimension;
	if (0 != (_MipDimension % _TileDimension))
		div++;
	return div;
}

int2 oSurfaceMipCalcDimensionsInTiles(const int2& _MipDimensions, const int2& _TileDimensions)
{
	return int2(
		oSurfaceMipCalcDimensionInTiles(_MipDimensions.x, _TileDimensions.x)
		, oSurfaceMipCalcDimensionInTiles(_MipDimensions.y, _TileDimensions.y));
}

int3 oSurfaceMipCalcDimensionsInTiles(const int3& _MipDimensions, const int2& _TileDimensions)
{
	return int3(
		oSurfaceMipCalcDimensionInTiles(_MipDimensions.x, _TileDimensions.x)
		, oSurfaceMipCalcDimensionInTiles(_MipDimensions.y, _TileDimensions.y)
		, _MipDimensions.z);
}

int oSurfaceMipCalcNumTiles(const int2& _MipDimensions, const int2& _TileDimensions)
{
	int2 mipDimInTiles = oSurfaceMipCalcDimensionsInTiles(_MipDimensions, _TileDimensions);
	return mipDimInTiles.x * mipDimInTiles.y;
}

int oSurfaceMipCalcNumTiles(const int3& _MipDimensions, const int2& _TileDimensions)
{
	int3 mipDimInTiles = oSurfaceMipCalcDimensionsInTiles(_MipDimensions, _TileDimensions);
	return mipDimInTiles.x * mipDimInTiles.y;
}

int oSurfaceSliceCalcNumTiles(const oSURFACE_DESC& _SurfaceDesc, const int2& _TileDimensions)
{
	if (oSURFACE_LAYOUT_IMAGE == _SurfaceDesc.Layout)
		return oSurfaceMipCalcNumTiles(_SurfaceDesc.Dimensions, _TileDimensions);

	int numTiles = 0;
	int lastMip = 1 + oSurfaceTileCalcBestFitMipLevel(_SurfaceDesc, _TileDimensions);
	for (int i = 0; i <= lastMip; i++)
	{
		int3 mipDim = oSurfaceMipCalcDimensions(_SurfaceDesc.Format, _SurfaceDesc.Dimensions, i);
		numTiles += oSurfaceMipCalcNumTiles(mipDim, _TileDimensions);
	}

	return numTiles;
}

static int oSurfaceSliceCalcStartTileID(const oSURFACE_DESC& _SurfaceDesc, const int2& _TileDimensions, int _Slice)
{
	int numTilesPerSlice = oSurfaceSliceCalcNumTiles(_SurfaceDesc, _TileDimensions);
	return _Slice * numTilesPerSlice;
}

// how many tiles from the startID to start the specified mip
static int oSurfaceSliceCalcMipTileIDOffset(const oSURFACE_DESC& _SurfaceDesc, const int2& _TileDimensions, int _MipLevel)
{
	if (oSURFACE_LAYOUT_IMAGE == _SurfaceDesc.Layout)
		return 0;

	int numTiles = 0;
	int numMips = __min(_MipLevel, 1 + oSurfaceTileCalcBestFitMipLevel(_SurfaceDesc, _TileDimensions));
	for (int i = 0; i < numMips; i++)
	{
		int3 mipDim = oSurfaceMipCalcDimensions(_SurfaceDesc.Format, _SurfaceDesc.Dimensions, i);
		numTiles += oSurfaceMipCalcNumTiles(mipDim, _TileDimensions);
	}

	return numTiles;
}

static int oSurfaceMipCalcStartTileID(const oSURFACE_DESC& _SurfaceDesc, const int2& _TileDimensions, int _MipLevel, int _Slice)
{
	int sliceStartID = oSurfaceSliceCalcStartTileID(_SurfaceDesc, _TileDimensions, _Slice);
	int mipIDOffset = oSurfaceSliceCalcMipTileIDOffset(_SurfaceDesc, _TileDimensions, _MipLevel);
	return sliceStartID + mipIDOffset;
}

int oSurfaceCalcTile(const oSURFACE_DESC& _SurfaceDesc, const int2& _TileDimensions, oSURFACE_TILE_DESC& _InOutTileDesc)
{
	int mipStartTileID = oSurfaceMipCalcStartTileID(_SurfaceDesc, _TileDimensions, _InOutTileDesc.MipLevel, _InOutTileDesc.Slice);
	int2 PositionInTiles = _InOutTileDesc.Position / _TileDimensions;
	int2 mipDim = oSurfaceMipCalcDimensions(_SurfaceDesc.Format, _SurfaceDesc.Dimensions.xy(), _InOutTileDesc.MipLevel);
	int2 mipDimInTiles = oSurfaceMipCalcDimensionsInTiles(mipDim, _TileDimensions);
	int tileID = mipStartTileID + (mipDimInTiles.x * PositionInTiles.y) + PositionInTiles.x;
	_InOutTileDesc.Position = PositionInTiles * _TileDimensions;
	return tileID;
}

void oSurfaceTileGetDesc(const oSURFACE_DESC& _SurfaceDesc, const int2& _TileDimensions, int _TileID, oSURFACE_TILE_DESC* _pTileDesc)
{
	int numTilesPerSlice = oSurfaceSliceCalcNumTiles(_SurfaceDesc, _TileDimensions);
	_pTileDesc->Slice = _TileID / numTilesPerSlice;
	oASSERT(_pTileDesc->Slice < _SurfaceDesc.NumSlices, "TileID is out of range for the specified mip dimensions");

	int firstTileInMip = 0;
	int3 mipDim = _SurfaceDesc.Dimensions;
	_pTileDesc->MipLevel = 0;
	int nthTileIntoSlice = _TileID % numTilesPerSlice; 

	if (nthTileIntoSlice > 0)
	{
		do 
		{
			mipDim = oSurfaceMipCalcDimensions(_SurfaceDesc.Format, _SurfaceDesc.Dimensions, ++_pTileDesc->MipLevel);
			firstTileInMip += oSurfaceMipCalcNumTiles(mipDim, _TileDimensions);

		} while (nthTileIntoSlice < firstTileInMip);
	}
	
	int tileOffsetFromMipStart = nthTileIntoSlice - firstTileInMip;
	int3 mipDimInTiles = oSurfaceMipCalcDimensionsInTiles(mipDim, _TileDimensions);
	int2 positionInTiles = int2(tileOffsetFromMipStart % mipDimInTiles.x, tileOffsetFromMipStart / mipDimInTiles.y);
	_pTileDesc->Position = positionInTiles * _TileDimensions;
}

//currently fairly aggressive at suggesting large pages
bool oShouldUseLargePages(const int3& _SurfaceDimensions, oSURFACE_FORMAT _Format, int _TileWidth, int _SmallPageSize, int _LargePageSize)
{
	int surfaceSize = oSurfaceMipCalcSize(_Format, _SurfaceDimensions);
	if(surfaceSize < (_LargePageSize/4))
		return false;

	oSURFACE_DESC desc;
	desc.Format = _Format;
	desc.Dimensions = _SurfaceDimensions;
	int surfacePitch = oSurfaceMipCalcRowPitch(desc);

	float numRowsPerPage = _SmallPageSize/static_cast<float>(surfacePitch); //number of rows before we get a page miss

	int tileByteWidth = oSurfaceMipCalcRowSize(_Format, _TileWidth);
	//estimate how many bytes we would work on in a tile before encountering a tlb cache miss
	float numBytesPerTLBMiss = tileByteWidth * numRowsPerPage - std::numeric_limits<float>::epsilon(); //not precise, but should be close enough for our purpose here. 
	//If we are not going to get at least half a small page size of work done per tlb miss, better to use large pages instead.
	if(numBytesPerTLBMiss <= (_SmallPageSize/2))
		return true;
	else
		return false;
}

void oSurfaceCalcMappedSubresource(const oSURFACE_DESC& _SurfaceDesc, int _Subresource, int _DepthIndex, const void* _pSurface, oSURFACE_CONST_MAPPED_SUBRESOURCE* _pMappedSubresource, int2* _pByteDimensions)
{
	oSURFACE_SUBRESOURCE_DESC ssrd;
	oSurfaceSubresourceGetDesc(_SurfaceDesc, _Subresource, &ssrd);

	_pMappedSubresource->RowPitch = oSurfaceMipCalcRowPitch(_SurfaceDesc, ssrd.MipLevel);
	_pMappedSubresource->DepthPitch = oSurfaceMipCalcDepthPitch(_SurfaceDesc, ssrd.MipLevel);
	_pMappedSubresource->pData = oByteAdd(_pSurface, oSurfaceSubresourceCalcOffset(_SurfaceDesc, _Subresource, _DepthIndex));

	if (_pByteDimensions)
		*_pByteDimensions = oSurfaceMipCalcByteDimensions(_SurfaceDesc.Format, ssrd.Dimensions.xy());
}

void oSurfaceCalcMappedSubresource(const oSURFACE_DESC& _SurfaceDesc, int _Subresource, int _DepthIndex, void* _pSurface, oSURFACE_MAPPED_SUBRESOURCE* _pMappedSubresource, int2* _pByteDimensions)
{
	oSurfaceCalcMappedSubresource(_SurfaceDesc, _Subresource, _DepthIndex, _pSurface, (oSURFACE_CONST_MAPPED_SUBRESOURCE*)_pMappedSubresource, _pByteDimensions);
}

void oSurfaceUpdateSubresource(const oSURFACE_DESC& _SurfaceDesc, int _Subresource, int _DepthIndex, void* _pDestinationSurface, const void* _pSource, size_t _SourceRowPitch, bool _FlipVertical)
{
	oSURFACE_MAPPED_SUBRESOURCE mapped;
	int2 ByteDimensions;
	oSurfaceCalcMappedSubresource(_SurfaceDesc, _Subresource, _DepthIndex, _pDestinationSurface, &mapped, &ByteDimensions);
	if (_FlipVertical)
		oMemcpy2dVFlip(mapped.pData, mapped.RowPitch, _pSource, _SourceRowPitch, ByteDimensions.x, ByteDimensions.y);
	else
		oMemcpy2d(mapped.pData, mapped.RowPitch, _pSource, _SourceRowPitch, ByteDimensions.x, ByteDimensions.y);
}

void oSurfaceCopySubresource(const oSURFACE_DESC& _SurfaceDesc, int _Subresource, int _DepthIndex, const void* _pSourceSurface, void* _pDestination, size_t _DestinationRowPitch, bool _FlipVertical)
{
	oSURFACE_CONST_MAPPED_SUBRESOURCE mapped;
	int2 ByteDimensions;
	oSurfaceCalcMappedSubresource(_SurfaceDesc, _Subresource, _DepthIndex, _pSourceSurface, &mapped, &ByteDimensions);
	if (_FlipVertical)
		oMemcpy2dVFlip(_pDestination, _DestinationRowPitch, mapped.pData, mapped.RowPitch, ByteDimensions.x, ByteDimensions.y);
	else
		oMemcpy2d(_pDestination, _DestinationRowPitch, mapped.pData, mapped.RowPitch, ByteDimensions.x, ByteDimensions.y);
}

void oSurfaceCopySubresource(const oSURFACE_DESC& _SurfaceDesc, oSURFACE_CONST_MAPPED_SUBRESOURCE& _SrcMap, oSURFACE_MAPPED_SUBRESOURCE* _DstMap, bool _FlipVertical)
{
	if (_FlipVertical)
		oMemcpy2dVFlip(_DstMap->pData, _DstMap->RowPitch, _SrcMap.pData, _SrcMap.RowPitch, _SurfaceDesc.Dimensions.x*oSurfaceFormatGetSize(_SurfaceDesc.Format), _SurfaceDesc.Dimensions.y);
	else
		oMemcpy2d(_DstMap->pData, _DstMap->RowPitch, _SrcMap.pData, _SrcMap.RowPitch, _SurfaceDesc.Dimensions.x*oSurfaceFormatGetSize(_SurfaceDesc.Format), _SurfaceDesc.Dimensions.y);
}