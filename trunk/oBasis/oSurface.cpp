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
#include <oBasis/oByte.h>
#include <oBasis/oInvalid.h>
#include <oBasis/oMacros.h>
#include <oBasis/oMemory.h>
#include <oBasis/oMath.h>
#include <cstring>

#define oCHECK_SURFACE_DESC(_Desc) \
	oASSERT(greater_than_equal(_Desc.Dimensions, int2(0,0)), "invalid dimensions: [%d,%d]", _Desc.Dimensions.x, _Desc.Dimensions.y); \
	oASSERT(_Desc.NumSlices, "invalid num slices: %d", _Desc.NumSlices); \

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
	unsigned int Size;
	BIT_SIZE BitSize;
	unsigned short NumChannels;
	bool IsBlockCompressed;
	bool IsUNORM;
	bool HasAlpha;
};

static const FORMAT_DESC sFormatDescs[] = 
{
	{ "UNKNOWN", 0, {0,0,0,0}, 0, false, false, false },
	{ "R32G32B32A32_TYPELESS", 4 * sizeof(float), {32,32,32,32}, 4, false, false, true },
	{ "R32G32B32A32_FLOAT", 4 * sizeof(float), {32,32,32,32}, 4, false, false, true },
	{ "R32G32B32A32_UINT", 4 * sizeof(int), {32,32,32,32}, 4, false, false, true },
	{ "R32G32B32A32_SINT", 4 * sizeof(int), {32,32,32,32}, 4, false, false, true },
	{ "R32G32B32_TYPELESS", 3 * sizeof(int), {32,32,32,0}, 3, false, false, true },
	{ "R32G32B32_FLOAT", 3 * sizeof(float), {32,32,32,0}, 3, false, false, false },
	{ "R32G32B32_UINT", 4 * sizeof(int), {32,32,32,0}, 3, false, false, false },
	{ "R32G32B32_SINT", 4 * sizeof(int), {32,32,32,0}, 3, false, false, false },
	{ "R16G16B16A16_TYPELESS", 4 * sizeof(short), {16,16,16,16}, 4, false, false, true },
	{ "R16G16B16A16_FLOAT", 4 * sizeof(short), {16,16,16,16}, 4, false, false, true },
	{ "R16G16B16A16_UNORM", 4 * sizeof(short), {16,16,16,16}, 4, false, true, true },
	{ "R16G16B16A16_UINT", 4 * sizeof(short), {16,16,16,16}, 4, false, false, true },
	{ "R16G16B16A16_SNORM", 4 * sizeof(short), {16,16,16,16}, 4, false, false, true },
	{ "R16G16B16A16_SINT", 4 * sizeof(short), {16,16,16,16}, 4, false, false, true },
	{ "R32G32_TYPELESS", 2 * sizeof(float), {32,32,0,0}, 2, false, false, false },
	{ "R32G32_FLOAT", 2 * sizeof(float), {32,32,0,0}, 2, false, false, false },
	{ "R32G32_UINT", 2 * sizeof(int), {32,32,0,0}, 2, false, false, false },
	{ "R32G32_SINT", 2 * sizeof(int), {32,32,0,0}, 2, false, false, false },
	{ "R32G8X24_TYPELESS", 2 * sizeof(float), {32,8,0,24}, 3, false, false, false },
	{ "D32_FLOAT_S8X24_UINT", 2 * sizeof(int), {32,8,0,24}, 3, false, false, false },
	{ "R32_FLOAT_X8X24_TYPELESS", 2 * sizeof(float), {32,8,0,24}, 3, false, false, false },
	{ "X32_TYPELESS_G8X24_UINT", 2 * sizeof(int), {32,8,0,24}, 3, false, false, false },
	{ "R10G10B10A2_TYPELESS", sizeof(int), {10,10,10,2}, 4, false, false, true },
	{ "R10G10B10A2_UNORM", sizeof(int), {10,10,10,2}, 4, false, true, true },
	{ "R10G10B10A2_UINT", sizeof(int), {10,10,10,2}, 4, false, false, true },
	{ "R11G11B10_FLOAT", sizeof(float), {11,11,11,0}, 3, false, false, false },
	{ "R8G8B8A8_TYPELESS", sizeof(int), {8,8,8,8}, 4, false, false, true },
	{ "R8G8B8A8_UNORM", sizeof(int), {8,8,8,8}, 4, false, true, true },
	{ "R8G8B8A8_UNORM_SRGB", sizeof(int), {8,8,8,8}, 4, false, true, true },
	{ "R8G8B8A8_UINT", sizeof(int), {8,8,8,8}, 4, false, false, true },
	{ "R8G8B8A8_SNORM", sizeof(int), {8,8,8,8}, 4, false, false, true },
	{ "R8G8B8A8_SINT", sizeof(int), {8,8,8,8}, 4, false, false, true },
	{ "R16G16_TYPELESS", 2 * sizeof(short), {16,16,0,0}, 2, false, false, false },
	{ "R16G16_FLOAT", 2 * sizeof(short), {16,16,0,0}, 2, false, false, false },
	{ "R16G16_UNORM", 2 * sizeof(short), {16,16,0,0}, 2, false, true, false },
	{ "R16G16_UINT", 2 * sizeof(short), {16,16,0,0}, 2, false, false, false },
	{ "R16G16_SNORM", 2 * sizeof(short), {16,16,0,0}, 2, false, false, false },
	{ "R16G16_SINT", 2 * sizeof(short), {16,16,0,0}, 2, false, false, false },
	{ "R32_TYPELESS", sizeof(float), {32,0,0,0}, 1, false, false, false },
	{ "D32_FLOAT", sizeof(float), {32,0,0,0}, 1, false, false, false },
	{ "R32_FLOAT", sizeof(float), {32,0,0,0}, 1, false, false, false },
	{ "R32_UINT", sizeof(int), {32,0,0,0}, 1, false, false, false },
	{ "R32_SINT", sizeof(int), {32,0,0,0}, 1, false, false, false },
	{ "R24G8_TYPELESS", sizeof(float), {24,8,0,0}, 2, false, false, false },
	{ "D24_UNORM_S8_UINT", sizeof(float), {24,8,0,0}, 2, false, true, false },
	{ "R24_UNORM_X8_TYPELESS", sizeof(float), {24,8,0,0}, 2, false, true, false },
	{ "X24_TYPELESS_G8_UINT", sizeof(float), {24,8,0,0}, 2, false, false, false },
	{ "R8G8_TYPELESS", 2 * sizeof(char), {8,8,0,0}, 2, false, false, false },
	{ "R8G8_UNORM", 2 * sizeof(char), {8,8,0,0}, 2, false, true, false },
	{ "R8G8_UINT", 2 * sizeof(char), {8,8,0,0}, 2, false, false, false },
	{ "R8G8_SNORM", 2 * sizeof(char), {8,8,0,0}, 2, false, false, false },
	{ "R8G8_SINT", 2 * sizeof(char), {8,8,0,0}, 2, false, false, false },
	{ "R16_TYPELESS", sizeof(short), {16,0,0,0}, 1, false, false, false },
	{ "R16_FLOAT", sizeof(short), {16,0,0,0}, 1, false, false, false },
	{ "D16_UNORM", sizeof(short), {16,0,0,0}, 1, false, true, false },
	{ "R16_UNORM", sizeof(short), {16,0,0,0}, 1, false, true, false },
	{ "R16_UINT", sizeof(short), {16,0,0,0}, 1, false, false, false },
	{ "R16_SNORM", sizeof(short), {16,0,0,0}, 1, false, false, false },
	{ "R16_SINT", sizeof(short), {16,0,0,0}, 1, false, false, false },
	{ "R8_TYPELESS", sizeof(char), {8,0,0,0}, 1, false, false, false },
	{ "R8_UNORM", sizeof(char), {8,0,0,0}, 1, false, true, false },
	{ "R8_UINT", sizeof(char), {8,0,0,0}, 1, false, false, false },
	{ "R8_SNORM", sizeof(char), {8,0,0,0}, 1, false, false, false },
	{ "R8_SINT", sizeof(char), {8,0,0,0}, 1, false, false, false },
	{ "A8_UNORM", sizeof(char), {8,0,0,0}, 1, false, true, true },
	{ "R1_UNORM", 1, {1,0,0,0}, 1, false, true, false },
	{ "R9G9B9E5_SHAREDEXP", sizeof(int), {9,9,9,5}, 4, false, false, false },
	{ "R8G8_B8G8_UNORM", sizeof(int), {8,8,8,8}, 4, false, true, false },
	{ "G8R8_G8B8_UNORM", sizeof(int), {8,8,8,8}, 4, false, true, false },
	{ "BC1_TYPELESS", 8, {5,6,5,0}, 3, true, false, false },
	{ "BC1_UNORM", 8, {5,6,5,0}, 3, true, true, false },
	{ "BC1_UNORM_SRGB", 8, {5,6,5,0}, 3, true, true, false },
	{ "BC2_TYPELESS", 16, {5,6,5,4}, 4, true, false, true },
	{ "BC2_UNORM", 16, {5,6,5,4}, 4, true, true, true },
	{ "BC2_UNORM_SRGB", 16, {5,6,5,4}, 4, true, true, true },
	{ "BC3_TYPELESS", 16, {5,6,5,8}, 4, true, false, true },
	{ "BC3_UNORM", 16, {5,6,5,8}, 4, true, true, true },
	{ "BC3_UNORM_SRGB", 16, {5,6,5,8}, 4, true, true, true },
	{ "BC4_TYPELESS", 8, {8,0,0,0}, 1, true, false, false },
	{ "BC4_UNORM", 8, {8,0,0,0}, 1, true, true, false },
	{ "BC4_SNORM", 8, {8,0,0,0}, 1, true, false, false },
	{ "BC5_TYPELESS", 16, {8,8,0,0}, 2, true, false, false },
	{ "BC5_UNORM", 16, {8,8,0,0}, 2, true, true, false },
	{ "BC5_SNORM", 16, {8,8,0,0}, 2, true, false, false },
	{ "B5G6R5_UNORM", sizeof(short), {5,6,5,0}, 3, false, true, false },
	{ "B5G5R5A1_UNORM", sizeof(short), {5,6,5,1}, 4, false, true, true },
	{ "B8G8R8A8_UNORM", sizeof(int), {8,8,8,8}, 4, false, true, true },
	{ "B8G8R8X8_UNORM", sizeof(int), {8,8,8,8}, 4, false, true, false },
	{ "R10G10B10_XR_BIAS_A2_UNORM", sizeof(int), 4, false, true, false },
	{ "B8G8R8A8_TYPELESS", sizeof(int), {8,8,8,8}, 4, false, false },
	{ "B8G8R8A8_UNORM_SRGB", sizeof(int), {8,8,8,8}, 4, false, true },
	{ "B8G8R8X8_TYPELESS", sizeof(int), {8,8,8,8}, 4, false, false },
	{ "B8G8R8X8_UNORM_SRGB", sizeof(int), {8,8,8,8}, 4, false, true },
	{ "BC6H_TYPELESS", 16, {0,0,0,0}, 3, true, false, false },
	{ "BC6H_UF16", 16, {0,0,0,0}, 3, true, false, false },
	{ "BC6H_SF16", 16, {0,0,0,0}, 3, true, false, false },
	{ "BC7_TYPELESS", 16, {0,0,0,0}, 4, true, false, true },
	{ "BC7_UNORM", 16, {0,0,0,0}, 4, true, true, true },
	{ "BC7_UNORM_SRGB", 16, {0,0,0,0}, 4, true, true, true },
	{ "R8G8B8_UNORM", 3 * sizeof(char), {8,8,8,0}, 3, false, true, false },
	{ "B8G8R8_UNORM", 3 * sizeof(char), {8,8,8,0}, 3, false, true, false },
};
static_assert(oCOUNTOF(sFormatDescs) == oSURFACE_NUM_FORMATS, "");

const char* oAsString(const oSURFACE_FORMAT& _Format)
{
	return (_Format < oSURFACE_NUM_FORMATS) ? sFormatDescs[_Format].String : "UNKNOWN";
}

char* oToString(char* _StrDestination, size_t _SizeofStrDestination, const oSURFACE_FORMAT& _Format)
{
	return 0 == strcpy_s(_StrDestination, _SizeofStrDestination, oAsString(_Format)) ? _StrDestination : nullptr;
}

bool oFromString(oSURFACE_FORMAT* _pFormat, const char* _StrSource)
{
	*_pFormat = oSURFACE_UNKNOWN;
	for (size_t i = 0; i < oCOUNTOF(sFormatDescs); i++)
	{
		if (!_stricmp(_StrSource, sFormatDescs[i].String))
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
		mip = oMax(int3(1,1,1), mip / int3(2,2,2));
	}

	return nMips;
}

int oSurfaceCalcNumMips(oSURFACE_LAYOUT _Layout, const int2& _Mip0Dimensions)
{
	return oSurfaceCalcNumMips(_Layout, int3(_Mip0Dimensions, 1));
}

static int oSurfaceMipCalcDimension(oSURFACE_FORMAT _Format, int _Mip0Dimension, int _MipLevel = 0)
{
	oCHECK_DIM(_Mip0Dimension);
	oASSERT(_Format != oSURFACE_UNKNOWN, "Unknown surface format passed to CalcMipDimension");
	oASSERT(_MipLevel == 0 || oIsPow2(_Mip0Dimension), "Mipchain dimensions must be a power of 2");
	int d = __max(1, _Mip0Dimension >> _MipLevel);
	return oSurfaceFormatIsBlockCompressed(_Format) ? static_cast<int>(oByteAlign(d, 4)) : d;
}

static int oSurfaceMipCalcDimensionNPOT(oSURFACE_FORMAT _Format, int _Mip0Dimension, int _MipLevel = 0)
{
	oCHECK_DIM(_Mip0Dimension);
	int d = __max(1, _Mip0Dimension / (1 << _MipLevel));
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
		, oSurfaceMipCalcDimension(_Format, _Mip0Dimensions.z, _MipLevel));
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
		, oSurfaceMipCalcDimensionNPOT(_Format, _Mip0Dimensions.z, _MipLevel));
}

oSize64 oSurfaceMipCalcRowSize(oSURFACE_FORMAT _Format, int _MipWidth)
{
	oCHECK_DIM(_MipWidth);
	oASSERT(_Format != oSURFACE_UNKNOWN, "Unknown surface format passed to GetRowPitch");
	int w = oSurfaceMipCalcDimension(_Format, _MipWidth);
	if (oSurfaceFormatIsBlockCompressed(_Format)) // because the atom is a 4x4 block
		w /= 4;
	int s = oSurfaceFormatGetSize(_Format);
	return static_cast<int>(oByteAlign(w * s, sFormatDescs[_Format].Size));
}

oSize64 oSurfaceMipCalcRowPitch(const oSURFACE_DESC& _SurfaceDesc, int _MipLevel)
{
	oCHECK_SURFACE_DESC(_SurfaceDesc);
	switch (_SurfaceDesc.Layout)
	{
		case oSURFACE_LAYOUT_IMAGE: return oSurfaceMipCalcRowSize(_SurfaceDesc.Format, _SurfaceDesc.Dimensions);
		case oSURFACE_LAYOUT_TIGHT: return oSurfaceMipCalcRowSize(_SurfaceDesc.Format, oSurfaceMipCalcDimension(_SurfaceDesc.Format, _SurfaceDesc.Dimensions.x, _MipLevel));
		case oSURFACE_LAYOUT_HORIZONTAL: return oSurfaceMipCalcRowSize(_SurfaceDesc.Format, _SurfaceDesc.Dimensions.x) + oSurfaceMipCalcRowSize(_SurfaceDesc.Format, oSurfaceMipCalcDimension(_SurfaceDesc.Format, _SurfaceDesc.Dimensions.x, 1));
		case oSURFACE_LAYOUT_VERTICAL: return oSurfaceMipCalcRowSize(_SurfaceDesc.Format, _SurfaceDesc.Dimensions.x);
		oNODEFAULT;
	}
}

int oSurfaceMipCalcNumRows(oSURFACE_FORMAT _Format, int _MipHeight)
{
	oCHECK_DIM(_MipHeight);
	int heightInPixels = oSurfaceMipCalcDimension(_Format, _MipHeight);
	return oSurfaceFormatIsBlockCompressed(_Format) ? __max(1, heightInPixels/4) : heightInPixels;
}

oSize64 oSurfaceMipCalcSize(oSURFACE_FORMAT _Format, const int2& _MipDimensions)
{
	oCHECK_DIM2(_MipDimensions);
	return oSurfaceMipCalcRowSize(_Format, _MipDimensions) * oSurfaceMipCalcNumRows(_Format, _MipDimensions);
}

static oSize64 oSurfaceMipCalcOffset_Tight(const oSURFACE_DESC& _SurfaceDesc, int _MipLevel)
{
	oCHECK_SURFACE_DESC(_SurfaceDesc);
	oSize64 offset = 0;
	int mip = _MipLevel;
	int2 dimensions = _SurfaceDesc.Dimensions;

	while (mip != _MipLevel)
	{
		offset += oSurfaceMipCalcSize(_SurfaceDesc.Format, dimensions);
		dimensions = oSurfaceMipCalcDimensions(_SurfaceDesc.Format, _SurfaceDesc.Dimensions, mip);
		mip++;
	}

	return offset;
}

static oSize64 oSurfaceMipCalcOffset_Vertical(const oSURFACE_DESC& _SurfaceDesc, int _MipLevel)
{
	oCHECK_SURFACE_DESC(_SurfaceDesc);
	if (0 == _MipLevel)
		return 0;

	// start at mip1
	oSize64 offset = oSurfaceMipCalcSize(_SurfaceDesc.Format, _SurfaceDesc.Dimensions);

	int mip = 1;
	int2 dimensions = _SurfaceDesc.Dimensions;

	while (mip != _MipLevel)
	{
		offset += oSurfaceMipCalcRowSize(_SurfaceDesc.Format, dimensions);
		dimensions = oSurfaceMipCalcDimensions(_SurfaceDesc.Format, _SurfaceDesc.Dimensions, mip);
		mip++;
	}		

	return offset;
}

static oSize64 oSurfaceMipCalcOffset_Horizontal(const oSURFACE_DESC& _SurfaceDesc, int _MipLevel)
{
	oCHECK_SURFACE_DESC(_SurfaceDesc);
	if (0 == _MipLevel)
		return 0;

	int mip0RowSize = oSurfaceMipCalcRowSize(_SurfaceDesc.Format, _SurfaceDesc.Dimensions);
	oSize64 offset = mip0RowSize;

	int mip = 1;
	int2 dimensions = _SurfaceDesc.Dimensions;

	while (mip != _MipLevel)
	{
		offset += mip0RowSize * oSurfaceMipCalcNumRows(_SurfaceDesc.Format, dimensions);
		dimensions = oSurfaceMipCalcDimensions(_SurfaceDesc.Format, _SurfaceDesc.Dimensions, mip);
		mip++;
	}		

	return offset;
}

oSize64 oSurfaceMipCalcOffset(const oSURFACE_DESC& _SurfaceDesc, int _MipLevel)
{
	oCHECK_SURFACE_DESC(_SurfaceDesc);
	switch (_SurfaceDesc.Layout)
	{
		case oSURFACE_LAYOUT_IMAGE: return 0;
		case oSURFACE_LAYOUT_TIGHT: return oSurfaceMipCalcOffset_Tight(_SurfaceDesc, _MipLevel);
		case oSURFACE_LAYOUT_HORIZONTAL: return oSurfaceMipCalcOffset_Horizontal(_SurfaceDesc, _MipLevel);
		case oSURFACE_LAYOUT_VERTICAL: return oSurfaceMipCalcOffset_Vertical(_SurfaceDesc, _MipLevel);
		oNODEFAULT;
	}
}

oSize64 oSurfaceSliceCalcPitch(const oSURFACE_DESC& _SurfaceDesc)
{
	oCHECK_SURFACE_DESC(_SurfaceDesc);
	switch (_SurfaceDesc.Layout)
	{
		case oSURFACE_LAYOUT_IMAGE:
			return oSurfaceMipCalcSize(_SurfaceDesc.Format, _SurfaceDesc.Dimensions);

		case oSURFACE_LAYOUT_TIGHT:
		{
			// Sum the size of all mip levels
			int nMips = oSurfaceCalcNumMips(_SurfaceDesc.Layout, _SurfaceDesc.Dimensions);
			oSize64 pitch = 0;
			int2 dimensions = _SurfaceDesc.Dimensions;
			while (nMips > 0)
			{
				pitch += oSurfaceMipCalcSize(_SurfaceDesc.Format, dimensions);
				nMips--;
			}
			return pitch;
		}

		case oSURFACE_LAYOUT_HORIZONTAL:
		{
			oSize64 pitch = oSurfaceMipCalcRowPitch(_SurfaceDesc);
			oSize64 mip0NumRows = oSurfaceMipCalcNumRows(_SurfaceDesc.Format, _SurfaceDesc.Dimensions);
			return pitch * mip0NumRows;
		}

		case oSURFACE_LAYOUT_VERTICAL:
		{
			oSize64 pitch = oSurfaceMipCalcRowPitch(_SurfaceDesc);
			oSize64 mip0NumRows = oSurfaceMipCalcNumRows(_SurfaceDesc.Format, _SurfaceDesc.Dimensions);
			oSize64 mip1NumRows = oSurfaceMipCalcNumRows(_SurfaceDesc.Format, oSurfaceMipCalcDimensions(_SurfaceDesc.Format, _SurfaceDesc.Dimensions, 1));
			return pitch * (mip0NumRows + mip1NumRows);
		}

		oNODEFAULT;
	}
}

oSize64 oSurfaceCalcSize(const oSURFACE_DESC& _SurfaceDesc)
{
	return oSurfaceSliceCalcPitch(_SurfaceDesc) * _SurfaceDesc.NumSlices;
}

static void oSurfaceSubresourceUnpack(int _Subresource, int _NumMips, int* _pMipLevel, int* _pSliceIndex)
{
	*_pSliceIndex = _Subresource / _NumMips;
	*_pMipLevel = _Subresource % _NumMips;
}

void oSurfaceSubresourceGetDesc(const oSURFACE_DESC& _SurfaceDesc, int _Subresource, oSURFACE_SUBRESOURCE_DESC* _pSubresourceDesc)
{
	oCHECK_SURFACE_DESC(_SurfaceDesc);
	int numMips = oSurfaceCalcNumMips(_SurfaceDesc.Layout, _SurfaceDesc.Dimensions);
	oSurfaceSubresourceUnpack(_Subresource, numMips, &_pSubresourceDesc->MipLevel, &_pSubresourceDesc->Slice);
	oASSERT(_pSubresourceDesc->Slice < _SurfaceDesc.NumSlices, "Slice index is out of range for the specified surface");
	_pSubresourceDesc->Dimensions = oSurfaceMipCalcDimensions(_SurfaceDesc.Format, _SurfaceDesc.Dimensions, _pSubresourceDesc->MipLevel);
}

oSize64 oSurfaceSubresourceCalcSize(const oSURFACE_DESC& _SurfaceDesc, const oSURFACE_SUBRESOURCE_DESC& _SubresourceDesc)
{
	return oSurfaceMipCalcSize(_SurfaceDesc.Format, _SubresourceDesc.Dimensions);
}

oSize64 oSurfaceSubresourceCalcOffset(const oSURFACE_DESC& _SurfaceDesc, int _Subresource)
{
	oCHECK_SURFACE_DESC(_SurfaceDesc);

	if (oSURFACE_LAYOUT_IMAGE == _SurfaceDesc.Layout)
		return _Subresource * oSurfaceMipCalcSize(_SurfaceDesc.Format, _SurfaceDesc.Dimensions);

	oSURFACE_SUBRESOURCE_DESC ssrd;
	oSurfaceSubresourceGetDesc(_SurfaceDesc, _Subresource, &ssrd);
	oSize64 slicePitch = oSurfaceSliceCalcPitch(_SurfaceDesc);
	return (slicePitch * ssrd.Slice) + oSurfaceMipCalcOffset(_SurfaceDesc, ssrd.MipLevel);
}

int oSurfaceTileCalcBestFitMipLevel(const oSURFACE_DESC& _SurfaceDesc, const int2& _TileDimensions)
{
	oCHECK_SURFACE_DESC(_SurfaceDesc);
	if (oSURFACE_LAYOUT_IMAGE == _SurfaceDesc.Layout)
		return 0;

	int nthMip = 0;
	int2 mip = _SurfaceDesc.Dimensions;
	while (mip != int2(1,1))
	{
		if (less_than_equal(_SurfaceDesc.Dimensions, _TileDimensions))
			break;

		nthMip++;
		mip = oMax(int2(1,1), mip / int2(2,2));
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

int oSurfaceSliceCalcNumTiles(const oSURFACE_DESC& _SurfaceDesc, const int2& _TileDimensions)
{
	if (oSURFACE_LAYOUT_IMAGE == _SurfaceDesc.Layout)
		return oSurfaceMipCalcNumTiles(_SurfaceDesc.Dimensions, _TileDimensions);

	int numTiles = 0;
	int lastMip = 1 + oSurfaceTileCalcBestFitMipLevel(_SurfaceDesc, _TileDimensions);
	for (int i = 0; i <= lastMip; i++)
	{
		int2 mipDim = oSurfaceMipCalcDimensions(_SurfaceDesc.Format, _SurfaceDesc.Dimensions, i);
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
		int2 mipDim = oSurfaceMipCalcDimensions(_SurfaceDesc.Format, _SurfaceDesc.Dimensions, i);
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
	int2 mipDim = _SurfaceDesc.Dimensions;
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
	int2 mipDimInTiles = oSurfaceMipCalcDimensionsInTiles(mipDim, _TileDimensions);
	int2 positionInTiles = int2(tileOffsetFromMipStart % mipDimInTiles.x, tileOffsetFromMipStart / mipDimInTiles.y);
	_pTileDesc->Position = positionInTiles * _TileDimensions;
}
