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
#define oCHECK_DIM(_Format, _Dim) oASSERT(_Dim >= oSurfaceFormatGetMinDimensions(_Format).x, "invalid dimension: %d", _Dim);
#define oCHECK_DIM2(_Format, _Dim) oASSERT(greater_than_equal(_Dim, oSurfaceFormatGetMinDimensions(_Format)), "invalid dimensions: [%d,%d]", _Dim.x, _Dim.y);
#define oCHECK_DIM3(_Format, _Dim) oASSERT(greater_than_equal(_Dim.xy, oSurfaceFormatGetMinDimensions(_Format)), "invalid dimensions: [%d,%d,%d]", _Dim.x, _Dim.y, _Dim.z);

#define oASSERT_PLANAR_SUPPORT(_Format) oASSERT(!oSurfaceFormatIsPlanar(_Format), "Planar formats may not behave well with this API. Review usage in this code and remove this when verified.");

struct BIT_SIZE
{
	unsigned char R;
	unsigned char G;
	unsigned char B;
	unsigned char A;
};

struct oSURFACE_SUBFORMATS_DESC
{
	oSURFACE_FORMAT Format[4];
};

enum oSURFACE_FORMAT_TRAIT
{
	oSURFACE_IS_BC = 1,
	oSURFACE_IS_UNORM = oSURFACE_IS_BC<<1,
	oSURFACE_HAS_ALPHA = oSURFACE_IS_UNORM<<1,
	oSURFACE_IS_DEPTH = oSURFACE_HAS_ALPHA<<1,
	oSURFACE_IS_PLANAR = oSURFACE_IS_DEPTH<<1,
	oSURFACE_IS_YUV = oSURFACE_IS_PLANAR<<1,
	oSURFACE_PALETTED = oSURFACE_IS_YUV<<1,
	oSURFACE_SUBSURFACE1_BIAS1 = oSURFACE_PALETTED<<1,
	oSURFACE_SUBSURFACE1_BIAS2 = oSURFACE_SUBSURFACE1_BIAS1<<1,
};

struct FORMAT_DESC
{
	const char* String;
	unsigned int FourCC;
	unsigned int Size;
	int2 MinDimensions;
	BIT_SIZE BitSize;
	unsigned char NumChannels;
	unsigned char NumSubformats;
	
	// @oooii-tony: Consider calling this a ProxyFormat and then using this same
	// mechanism for dealing with depth v. color types.
	oSURFACE_SUBFORMATS_DESC Subformats;
	int Traits;
};

#define k0_0 (0)
#define k4_32 (4*4)
#define k3_32 (3*4)
#define k2_32 (2*4)
#define k1_32 (1*4)
#define k4_16 (4*2)
#define k3_16 (3*2)
#define k2_16 (2*2)
#define k1_16 (1*2)
#define k4_8 (4)
#define k3_8 (3)
#define k2_8 (2)
#define k1_8 (1)
#define k4_4 (1)
#define k3_4 (1)
#define k2_4 (1)
#define k1_4 (1)
#define kYAUV (2)
#define kYUV (1)
#define kBC_8 (8)
#define kBC_16 (16)
#define kYAUV_BC (16)
#define kYUV_BC (8)

static const int2 kSmallestMip(1,1);
static const int2 kSmallestMipBC(4,4);
static const int2 kSmallestMipYUV(2,2);

static const BIT_SIZE kBS_UNK = {0,0,0,0};
static const BIT_SIZE kBS_4_32 = {32,32,32,32};
static const BIT_SIZE kBS_3_32 = {32,32,32,0};
static const BIT_SIZE kBS_2_32 = {32,32,0,0};
static const BIT_SIZE kBS_1_32 = {32,0,0,0};
static const BIT_SIZE kBS_4_16 = {16,16,16,16};
static const BIT_SIZE kBS_3_16 = {16,16,16,0};
static const BIT_SIZE kBS_2_16 = {16,16,0,0};
static const BIT_SIZE kBS_1_16 = {16,0,0,0};
static const BIT_SIZE kBS_4_8 = {8,8,8,8};
static const BIT_SIZE kBS_3_8 = {8,8,8,0};
static const BIT_SIZE kBS_2_8 = {8,8,0,0};
static const BIT_SIZE kBS_1_8 = {8,0,0,0};
static const BIT_SIZE kBS_565 = {5,6,5,0};
static const BIT_SIZE kBS_DEC3N = {10,10,10,2};
static const BIT_SIZE kBS_DS = {24,8,0,0};
static const BIT_SIZE kBS_4_4 = {4,4,4,4};
static const BIT_SIZE kBS_3_4 = {4,4,4,0};
static const BIT_SIZE kBS_2_4 = {4,4,0,0};
static const BIT_SIZE kBS_1_4 = {4,0,0,0};

static const oFourCC kFCC_UNK = oFCC('????');

static const oSURFACE_SUBFORMATS_DESC kSFD_UNK = {oSURFACE_UNKNOWN, oSURFACE_UNKNOWN, oSURFACE_UNKNOWN, oSURFACE_UNKNOWN};
static const oSURFACE_SUBFORMATS_DESC kSFD_R8_R8 = {oSURFACE_R8_UNORM, oSURFACE_R8_UNORM, oSURFACE_UNKNOWN, oSURFACE_UNKNOWN};
static const oSURFACE_SUBFORMATS_DESC kSFD_R8_RG8 = {oSURFACE_R8_UNORM, oSURFACE_R8G8_UNORM, oSURFACE_UNKNOWN, oSURFACE_UNKNOWN};
static const oSURFACE_SUBFORMATS_DESC kSFD_RG8_RG8 = {oSURFACE_R8G8_UNORM, oSURFACE_R8G8_UNORM, oSURFACE_UNKNOWN, oSURFACE_UNKNOWN};
static const oSURFACE_SUBFORMATS_DESC kSFD_R16_RG16 = {oSURFACE_R16_UNORM, oSURFACE_R16G16_UNORM, oSURFACE_UNKNOWN, oSURFACE_UNKNOWN};
static const oSURFACE_SUBFORMATS_DESC kSFD_BC4_BC4 = {oSURFACE_BC4_UNORM, oSURFACE_BC4_UNORM, oSURFACE_UNKNOWN, oSURFACE_UNKNOWN};
static const oSURFACE_SUBFORMATS_DESC kSFD_BC4_BC5 = {oSURFACE_BC4_UNORM, oSURFACE_BC5_UNORM, oSURFACE_UNKNOWN, oSURFACE_UNKNOWN};
static const oSURFACE_SUBFORMATS_DESC kSFD_BC5_BC5 = {oSURFACE_BC5_UNORM, oSURFACE_BC5_UNORM, oSURFACE_UNKNOWN, oSURFACE_UNKNOWN};
static const oSURFACE_SUBFORMATS_DESC kSFD_R8_4 = {oSURFACE_R8_UNORM, oSURFACE_R8_UNORM, oSURFACE_R8_UNORM, oSURFACE_R8_UNORM};
static const oSURFACE_SUBFORMATS_DESC kSFD_R8_3 = {oSURFACE_R8_UNORM, oSURFACE_R8_UNORM, oSURFACE_R8_UNORM, oSURFACE_UNKNOWN};
static const oSURFACE_SUBFORMATS_DESC kSFD_BC4_4 = {oSURFACE_BC4_UNORM, oSURFACE_BC4_UNORM, oSURFACE_BC4_UNORM, oSURFACE_BC4_UNORM};
static const oSURFACE_SUBFORMATS_DESC kSFD_BC4_3 = {oSURFACE_BC4_UNORM, oSURFACE_BC4_UNORM, oSURFACE_BC4_UNORM, oSURFACE_UNKNOWN};

static const FORMAT_DESC sFormatDescs[] = 
{
	{ "UNKNOWN", kFCC_UNK, k0_0, kSmallestMip, kBS_UNK, 0, 0, kSFD_UNK, 0 },
	{ "R32G32B32A32_TYPELESS", oFCC('?i4 '), k4_32, kSmallestMip, kBS_4_32, 4, 1, kSFD_UNK, oSURFACE_HAS_ALPHA },
	{ "R32G32B32A32_FLOAT", oFCC('f4  '), k4_32, kSmallestMip, kBS_4_32, 4, 1, kSFD_UNK, oSURFACE_HAS_ALPHA },
	{ "R32G32B32A32_UINT", oFCC('ui4 '), k4_32, kSmallestMip, kBS_4_32, 4, 1, kSFD_UNK, oSURFACE_HAS_ALPHA },
	{ "R32G32B32A32_SINT", oFCC('si4 '), k4_32, kSmallestMip, kBS_4_32, 4, 1, kSFD_UNK, oSURFACE_HAS_ALPHA },
	{ "R32G32B32_TYPELESS", oFCC('?i3 '), k3_32, kSmallestMip, kBS_3_32, 3, 1, kSFD_UNK, oSURFACE_HAS_ALPHA },
	{ "R32G32B32_FLOAT", oFCC('f3  '), k3_32, kSmallestMip, kBS_3_32, 3, 1, kSFD_UNK, 0 },
	{ "R32G32B32_UINT", oFCC('ui3 '), k4_32, kSmallestMip, kBS_3_32, 3, 1, kSFD_UNK, 0 },
	{ "R32G32B32_SINT", oFCC('si3 '), k4_32, kSmallestMip, kBS_3_32, 3, 1, kSFD_UNK, 0 },
	{ "R16G16B16A16_TYPELESS", oFCC('?s4 '), k4_16, kSmallestMip, kBS_4_16, 4, 1, kSFD_UNK, oSURFACE_HAS_ALPHA },
	{ "R16G16B16A16_FLOAT", oFCC('h4  '), k4_16, kSmallestMip, kBS_4_16, 4, 1, kSFD_UNK, oSURFACE_HAS_ALPHA },
	{ "R16G16B16A16_UNORM", oFCC('h4u '), k4_16, kSmallestMip, kBS_4_16, 4, 1, kSFD_UNK, oSURFACE_IS_UNORM|oSURFACE_HAS_ALPHA },
	{ "R16G16B16A16_UINT", oFCC('us4 '), k4_16, kSmallestMip, kBS_4_16, 4, 1, kSFD_UNK, oSURFACE_HAS_ALPHA },
	{ "R16G16B16A16_SNORM", oFCC('h4s '), k4_16, kSmallestMip, kBS_4_16, 4, 1, kSFD_UNK, oSURFACE_HAS_ALPHA },
	{ "R16G16B16A16_SINT", oFCC('ss4 '), k4_16, kSmallestMip, kBS_4_16, 4, 1, kSFD_UNK, oSURFACE_HAS_ALPHA },
	{ "R32G32_TYPELESS", oFCC('?i2 '), k2_32, kSmallestMip, kBS_2_32, 2, 1, kSFD_UNK, 0 },
	{ "R32G32_FLOAT", oFCC('f2  '), k2_32, kSmallestMip, kBS_2_32, 2, 1, kSFD_UNK, 0 },
	{ "R32G32_UINT", oFCC('ui2 '), k2_32, kSmallestMip, kBS_2_32, 2, 1, kSFD_UNK, 0 },
	{ "R32G32_SINT", oFCC('si2 '), k2_32, kSmallestMip, kBS_2_32, 2, 1, kSFD_UNK, 0 },
	{ "R32G8X24_TYPELESS", kFCC_UNK, k2_32, kSmallestMip, {32,8,0,24}, 3, 1, kSFD_UNK, 0 },
	{ "D32_FLOAT_S8X24_UINT", kFCC_UNK, k2_32, kSmallestMip, {32,8,0,24}, 3, 1, kSFD_UNK, oSURFACE_IS_DEPTH },
	{ "R32_FLOAT_X8X24_TYPELESS", kFCC_UNK, k2_32, kSmallestMip, {32,8,0,24}, 3, 1, kSFD_UNK, oSURFACE_IS_DEPTH },
	{ "X32_TYPELESS_G8X24_UINT", kFCC_UNK, k2_32, kSmallestMip, {32,8,0,24}, 3, 1, kSFD_UNK, 0 },
	{ "R10G10B10A2_TYPELESS", kFCC_UNK, k1_32, kSmallestMip, kBS_DEC3N, 4, 1, kSFD_UNK, oSURFACE_HAS_ALPHA },
	{ "R10G10B10A2_UNORM", kFCC_UNK, k1_32, kSmallestMip, kBS_DEC3N, 4, 1, kSFD_UNK, oSURFACE_IS_UNORM|oSURFACE_HAS_ALPHA },
	{ "R10G10B10A2_UINT", kFCC_UNK, k1_32, kSmallestMip, kBS_DEC3N, 4, 1, kSFD_UNK, oSURFACE_HAS_ALPHA },
	{ "R11G11B10_FLOAT", kFCC_UNK, k1_32, kSmallestMip, {11,11,10,0}, 3, 1, kSFD_UNK, 0 },
	{ "R8G8B8A8_TYPELESS", oFCC('?c4 '), k1_32, kSmallestMip, kBS_4_8, 4, 1, kSFD_UNK, oSURFACE_HAS_ALPHA },
	{ "R8G8B8A8_UNORM", oFCC('c4u '), k1_32, kSmallestMip, kBS_4_8, 4, 1, kSFD_UNK, oSURFACE_IS_UNORM|oSURFACE_HAS_ALPHA },
	{ "R8G8B8A8_UNORM_SRGB", oFCC('c4us'), k1_32, kSmallestMip, kBS_4_8, 4, 1, kSFD_UNK, oSURFACE_IS_UNORM|oSURFACE_HAS_ALPHA },
	{ "R8G8B8A8_UINT", oFCC('uc4 '), k1_32, kSmallestMip, kBS_4_8, 4, 1, kSFD_UNK, oSURFACE_HAS_ALPHA },
	{ "R8G8B8A8_SNORM", oFCC('c4s '), k1_32, kSmallestMip, kBS_4_8, 4, 1, kSFD_UNK, oSURFACE_HAS_ALPHA },
	{ "R8G8B8A8_SINT", oFCC('sc4 '), k1_32, kSmallestMip, kBS_4_8, 4, 1, kSFD_UNK, oSURFACE_HAS_ALPHA },
	{ "R16G16_TYPELESS", oFCC('?s2 '), k2_16, kSmallestMip, kBS_2_16, 2, 1, kSFD_UNK, 0 },
	{ "R16G16_FLOAT", oFCC('h2 '), k2_16, kSmallestMip, kBS_2_16, 2, 1, kSFD_UNK, 0 },
	{ "R16G16_UNORM", oFCC('h2u '), k2_16, kSmallestMip, kBS_2_16, 2, 1, kSFD_UNK, oSURFACE_IS_UNORM },
	{ "R16G16_UINT", oFCC('us2 '), k2_16, kSmallestMip, kBS_2_16, 2, 1, kSFD_UNK, 0 },
	{ "R16G16_SNORM", oFCC('h2s '), k2_16, kSmallestMip, kBS_2_16, 2, 1, kSFD_UNK, 0 },
	{ "R16G16_SINT", oFCC('ss2 '), k2_16, kSmallestMip, kBS_2_16, 2, 1, kSFD_UNK, 0 },
	{ "R32_TYPELESS", oFCC('?i1 '), k1_32, kSmallestMip, kBS_1_32, 1, 1, kSFD_UNK, oSURFACE_IS_DEPTH },
	{ "D32_FLOAT", oFCC('f1d '), k1_32, kSmallestMip, kBS_1_32, 1, 1, kSFD_UNK, oSURFACE_IS_DEPTH },
	{ "R32_FLOAT", oFCC('f1  '), k1_32, kSmallestMip, kBS_1_32, 1, 1, kSFD_UNK, 0 },
	{ "R32_UINT", oFCC('ui1 '), k1_32, kSmallestMip, kBS_1_32, 1, 1, kSFD_UNK, 0 },
	{ "R32_SINT", oFCC('si1 '), k1_32, kSmallestMip, kBS_1_32, 1, 1, kSFD_UNK, 0 },
	{ "R24G8_TYPELESS", kFCC_UNK, k1_32, kSmallestMip, kBS_DS, 2, 1, kSFD_UNK, oSURFACE_IS_DEPTH },
	{ "D24_UNORM_S8_UINT", kFCC_UNK, k1_32, kSmallestMip, kBS_DS, 2, 1, kSFD_UNK, oSURFACE_IS_UNORM|oSURFACE_IS_DEPTH },
	{ "R24_UNORM_X8_TYPELESS", kFCC_UNK, k1_32, kSmallestMip, kBS_DS, 2, 1, kSFD_UNK, oSURFACE_IS_UNORM|oSURFACE_IS_DEPTH },
	{ "X24_TYPELESS_G8_UINT", kFCC_UNK, k1_32, kSmallestMip, kBS_DS, 2, 1, kSFD_UNK, oSURFACE_IS_DEPTH },
	{ "R8G8_TYPELESS", oFCC('?c2 '), k2_8, kSmallestMip, kBS_2_8, 2, 1, kSFD_UNK, 0 },
	{ "R8G8_UNORM", oFCC('uc2u'), k2_8, kSmallestMip, kBS_2_8, 2, 1, kSFD_UNK, oSURFACE_IS_UNORM },
	{ "R8G8_UINT", oFCC('ui2 '), k2_8, kSmallestMip, kBS_2_8, 2, 1, kSFD_UNK, 0 },
	{ "R8G8_SNORM", oFCC('uc2s'), k2_8, kSmallestMip, kBS_2_8, 2, 1, kSFD_UNK, 0 },
	{ "R8G8_SINT", oFCC('si2 '), k2_8, kSmallestMip, kBS_2_8, 2, 1, kSFD_UNK, 0 },
	{ "R16_TYPELESS", oFCC('?s1 '), k1_16, kSmallestMip, kBS_1_16, 1, 1, kSFD_UNK, oSURFACE_IS_DEPTH },
	{ "R16_FLOAT", oFCC('h1  '), k1_16, kSmallestMip, kBS_1_16, 1, 1, kSFD_UNK, 0 },
	{ "D16_UNORM", oFCC('h1ud'), k1_16, kSmallestMip, kBS_1_16, 1, 1, kSFD_UNK, oSURFACE_IS_UNORM|oSURFACE_IS_DEPTH },
	{ "R16_UNORM", oFCC('h1u '), k1_16, kSmallestMip, kBS_1_16, 1, 1, kSFD_UNK, oSURFACE_IS_UNORM },
	{ "R16_UINT", oFCC('us1 '), k1_16, kSmallestMip, kBS_1_16, 1, 1, kSFD_UNK, 0 },
	{ "R16_SNORM", oFCC('h1s '), k1_16, kSmallestMip, kBS_1_16, 1, 1, kSFD_UNK, 0 },
	{ "R16_SINT", oFCC('ss1 '), k1_16, kSmallestMip, kBS_1_16, 1, 1, kSFD_UNK, 0 },
	{ "R8_TYPELESS", oFCC('?c1 '), k1_8, kSmallestMip, kBS_1_8, 1, 1, kSFD_UNK, 0 },
	{ "R8_UNORM", oFCC('uc1u'), k1_8, kSmallestMip, kBS_1_8, 1, 1, kSFD_UNK, oSURFACE_IS_UNORM },
	{ "R8_UINT", oFCC('uc1 '), k1_8, kSmallestMip, kBS_1_8, 1, 1, kSFD_UNK, 0 },
	{ "R8_SNORM", oFCC('uc1s'), k1_8, kSmallestMip, kBS_1_8, 1, 1, kSFD_UNK, 0 },
	{ "R8_SINT", oFCC('sc1 '), k1_8, kSmallestMip, kBS_1_8, 1, 1, kSFD_UNK, 0 },
	{ "A8_UNORM", oFCC('ac1u'), k1_8, kSmallestMip, kBS_1_8, 1, 1, kSFD_UNK, oSURFACE_IS_UNORM|oSURFACE_HAS_ALPHA },
	{ "R1_UNORM", oFCC('bitu'), 1, kSmallestMip, {1,0,0,0}, 1, 1, kSFD_UNK, oSURFACE_IS_UNORM },
	{ "R9G9B9E5_SHAREDEXP", kFCC_UNK, k1_32, kSmallestMip, {9,9,9,5}, 4, 1, kSFD_UNK, 0 },
	{ "R8G8_B8G8_UNORM", kFCC_UNK, k1_32, kSmallestMip, kBS_4_8, 4, 1, kSFD_UNK, oSURFACE_IS_UNORM },
	{ "G8R8_G8B8_UNORM", kFCC_UNK, k1_32, kSmallestMip, kBS_4_8, 4, 1, kSFD_UNK, oSURFACE_IS_UNORM },
	{ "BC1_TYPELESS", oFCC('BC1?'), kBC_8, kSmallestMipBC, kBS_565, 3, 1, kSFD_UNK, oSURFACE_IS_BC },
	{ "BC1_UNORM", oFCC('BC1u'), kBC_8, kSmallestMipBC, kBS_565, 3, 1, kSFD_UNK, oSURFACE_IS_BC|oSURFACE_IS_UNORM },
	{ "BC1_UNORM_SRGB", oFCC('BC1s'), kBC_8, kSmallestMipBC, kBS_565, 3, 1, kSFD_UNK, oSURFACE_IS_BC|oSURFACE_IS_UNORM },
	{ "BC2_TYPELESS", oFCC('BC2?'), kBC_16, kSmallestMipBC, {5,6,5,4}, 4, 1, kSFD_UNK, oSURFACE_IS_BC|oSURFACE_HAS_ALPHA },
	{ "BC2_UNORM", oFCC('BC2u'), kBC_16, kSmallestMipBC, {5,6,5,4}, 4, 1, kSFD_UNK, oSURFACE_IS_BC|oSURFACE_IS_UNORM|oSURFACE_HAS_ALPHA },
	{ "BC2_UNORM_SRGB", oFCC('BC2s'), kBC_16, kSmallestMipBC, {5,6,5,4}, 4, 1, kSFD_UNK, oSURFACE_IS_BC|oSURFACE_IS_UNORM|oSURFACE_HAS_ALPHA },
	{ "BC3_TYPELESS", oFCC('BC3?'), kBC_16, kSmallestMipBC, {5,6,5,8}, 4, 1, kSFD_UNK, oSURFACE_IS_BC|oSURFACE_HAS_ALPHA },
	{ "BC3_UNORM", oFCC('BC3u'), kBC_16, kSmallestMipBC, {5,6,5,8}, 4, 1, kSFD_UNK, oSURFACE_IS_BC|oSURFACE_IS_UNORM|oSURFACE_HAS_ALPHA },
	{ "BC3_UNORM_SRGB", oFCC('BC3s'), kBC_16, kSmallestMipBC, {5,6,5,8}, 4, 1, kSFD_UNK, oSURFACE_IS_BC|oSURFACE_IS_UNORM|oSURFACE_HAS_ALPHA },
	{ "BC4_TYPELESS", oFCC('BC4?'), kBC_8, kSmallestMipBC, kBS_1_8, 1, 1, kSFD_UNK, oSURFACE_IS_BC },
	{ "BC4_UNORM", oFCC('BC4u'), kBC_8, kSmallestMipBC, kBS_1_8, 1, 1, kSFD_UNK, oSURFACE_IS_BC|oSURFACE_IS_UNORM },
	{ "BC4_SNORM", oFCC('BC4s'), kBC_8, kSmallestMipBC, kBS_1_8, 1, 1, kSFD_UNK, oSURFACE_IS_BC },
	{ "BC5_TYPELESS", oFCC('BC5?'), kBC_16, kSmallestMipBC, kBS_2_8, 2, 1, kSFD_UNK, oSURFACE_IS_BC },
	{ "BC5_UNORM", oFCC('BC5u'), kBC_16, kSmallestMipBC, kBS_2_8, 2, 1, kSFD_UNK, oSURFACE_IS_BC|oSURFACE_IS_UNORM },
	{ "BC5_SNORM", oFCC('BC5s'), kBC_16, kSmallestMipBC, kBS_2_8, 2, 1, kSFD_UNK, oSURFACE_IS_BC },
	{ "B5G6R5_UNORM", kFCC_UNK, k1_16, kSmallestMip, kBS_565, 3, 1, kSFD_UNK, oSURFACE_IS_UNORM },
	{ "B5G5R5A1_UNORM", kFCC_UNK, k1_16, kSmallestMip, {5,6,5,1}, 4, 1, kSFD_UNK, oSURFACE_IS_UNORM|oSURFACE_HAS_ALPHA },
	{ "B8G8R8A8_UNORM", oFCC('c4u '), k1_32, kSmallestMip, kBS_4_8, 4, 1, kSFD_UNK, oSURFACE_IS_UNORM|oSURFACE_HAS_ALPHA },
	{ "B8G8R8X8_UNORM", oFCC('c4u '), k1_32, kSmallestMip, kBS_4_8, 4, 1, kSFD_UNK, oSURFACE_IS_UNORM },
	{ "R10G10B10_XR_BIAS_A2_UNORM", kFCC_UNK, k1_32, kSmallestMip, kBS_DEC3N, 4, 1, kSFD_UNK, oSURFACE_IS_UNORM },
	{ "B8G8R8A8_TYPELESS", kFCC_UNK, k1_32, kSmallestMip, kBS_4_8, 4, 1, kSFD_UNK, 0 },
	{ "B8G8R8A8_UNORM_SRGB", kFCC_UNK, k1_32, kSmallestMip, kBS_4_8, 4, 1, kSFD_UNK, oSURFACE_IS_UNORM },
	{ "B8G8R8X8_TYPELESS", kFCC_UNK, k1_32, kSmallestMip, kBS_4_8, 4, 1, kSFD_UNK, 0 },
	{ "B8G8R8X8_UNORM_SRGB", kFCC_UNK, k1_32, kSmallestMip, kBS_4_8, 4, 1, kSFD_UNK, oSURFACE_IS_UNORM },
	{ "BC6H_TYPELESS", oFCC('BC6?'), kBC_16, kSmallestMipBC, kBS_UNK, 3, 1, kSFD_UNK, oSURFACE_IS_BC },
	{ "BC6H_UF16", oFCC('BC6u'), kBC_16, kSmallestMipBC, kBS_UNK, 3, 1, kSFD_UNK, oSURFACE_IS_BC },
	{ "BC6H_SF16", oFCC('BC6s'), kBC_16, kSmallestMipBC, kBS_UNK, 3, 1, kSFD_UNK, oSURFACE_IS_BC },
	{ "BC7_TYPELESS", oFCC('BC7?'), kBC_16, kSmallestMipBC, kBS_UNK, 4, 1, kSFD_UNK, oSURFACE_IS_BC|oSURFACE_HAS_ALPHA },
	{ "BC7_UNORM", oFCC('BC7u'), kBC_16, kSmallestMipBC, kBS_UNK, 4, 1, kSFD_UNK, oSURFACE_IS_BC|oSURFACE_IS_UNORM|oSURFACE_HAS_ALPHA },
	{ "BC7_UNORM_SRGB", oFCC('BC7s'), kBC_16, kSmallestMipBC, kBS_UNK, 4, 1, kSFD_UNK, oSURFACE_IS_BC|oSURFACE_IS_UNORM|oSURFACE_HAS_ALPHA },
	{ "AYUV", oFCC('AYUV'), k4_32, kSmallestMip, kBS_4_4, 4, 1, kSFD_UNK, oSURFACE_IS_UNORM|oSURFACE_HAS_ALPHA|oSURFACE_IS_YUV },
	{ "Y410", oFCC('Y410'), k1_32, kSmallestMip, kBS_DEC3N, 4, 1, {oSURFACE_R10G10B10A2_UNORM,oSURFACE_UNKNOWN}, oSURFACE_IS_UNORM|oSURFACE_HAS_ALPHA|oSURFACE_IS_YUV },
	{ "Y416", oFCC('Y416'), k4_8 , kSmallestMip, kBS_4_16, 4, 1,{oSURFACE_B8G8R8A8_UNORM,oSURFACE_UNKNOWN}, oSURFACE_IS_UNORM|oSURFACE_HAS_ALPHA|oSURFACE_IS_YUV },
	{ "NV12", oFCC('NV12'), kYUV, kSmallestMipYUV, kBS_3_8, 3, 2, kSFD_R8_RG8, oSURFACE_IS_UNORM|oSURFACE_IS_YUV },
	{ "YUV2", oFCC('YUV2'), k4_8, kSmallestMip, kBS_4_8, 4, 1, kSFD_R8_RG8, oSURFACE_IS_UNORM|oSURFACE_HAS_ALPHA|oSURFACE_IS_YUV },
	{ "P010", oFCC('P010'), k1_16, kSmallestMip, {10,10,10,0}, 3, 2, kSFD_R16_RG16, oSURFACE_IS_UNORM|oSURFACE_IS_PLANAR|oSURFACE_IS_YUV },
	{ "P016", oFCC('P016'), k1_16, kSmallestMip, kBS_3_16, 3, 2, kSFD_R16_RG16, oSURFACE_IS_UNORM|oSURFACE_IS_PLANAR|oSURFACE_IS_YUV },
	{ "420_OPAQUE", oFCC('420O'), k1_8, kSmallestMipYUV, kBS_3_8, 3, 2, kSFD_R8_RG8, oSURFACE_IS_UNORM|oSURFACE_IS_PLANAR|oSURFACE_IS_YUV },
	{ "Y210", oFCC('Y210'), k4_16, kSmallestMipYUV, kBS_4_16, 3, 1, kSFD_UNK, oSURFACE_IS_UNORM|oSURFACE_IS_YUV },
	{ "Y216", oFCC('Y216'), k4_16, kSmallestMipYUV, kBS_4_16, 3, 1, kSFD_UNK, oSURFACE_IS_UNORM|	oSURFACE_IS_YUV },
	{ "NV11", kFCC_UNK, kYUV, kSmallestMipYUV, kBS_3_8, 3, 2, kSFD_R8_RG8, oSURFACE_IS_UNORM|oSURFACE_IS_PLANAR|oSURFACE_IS_YUV },
	{ "IA44", oFCC('IA44'), k2_4, kSmallestMip, {4,0,0,4}, 2, 1, kSFD_UNK, oSURFACE_HAS_ALPHA|oSURFACE_PALETTED }, // index-alpha
	{ "AI44", oFCC('AI44'), k2_4, kSmallestMip, {4,0,0,4}, 2, 1, kSFD_UNK, oSURFACE_HAS_ALPHA|oSURFACE_PALETTED }, // alpha-index
	{ "P8", oFCC('P8  '), k1_8, kSmallestMip, kBS_UNK, 1, 1, kSFD_UNK, oSURFACE_PALETTED }, // paletted
	{ "A8P8", oFCC('A8P8'), k2_8, kSmallestMip, kBS_UNK, 2, 1, kSFD_UNK, oSURFACE_HAS_ALPHA|oSURFACE_PALETTED },
	{ "B4G4R4A4_UNORM", oFCC('n4u '), k1_16, kSmallestMip, kBS_4_4, 4, 1, kSFD_UNK, oSURFACE_IS_UNORM|oSURFACE_HAS_ALPHA },
	{ "R8G8B8_UNORM", oFCC('c3u '), k3_8, kSmallestMip, kBS_3_8, 3, 1, kSFD_UNK, oSURFACE_IS_UNORM },
	{ "B8G8R8_UNORM", oFCC('c3u '), k3_8, kSmallestMip, kBS_3_8, 3, 1, kSFD_UNK, oSURFACE_IS_UNORM },
	{ "Y8_U8_V8_UNORM", oFCC('yuv8'), k1_8, kSmallestMipYUV, kBS_3_8, 3, 3, kSFD_R8_3, oSURFACE_IS_UNORM|oSURFACE_IS_PLANAR|oSURFACE_IS_YUV|oSURFACE_SUBSURFACE1_BIAS1 },
	{ "Y8_A8_U8_V8_UNORM", oFCC('auv8'), k1_8, kSmallestMipYUV, kBS_3_8, 3, 4, kSFD_R8_4, oSURFACE_IS_UNORM|oSURFACE_HAS_ALPHA|oSURFACE_IS_PLANAR|oSURFACE_IS_YUV|oSURFACE_SUBSURFACE1_BIAS1 },
	{ "YBC4_UBC4_VBC4_UNORM", oFCC('yuvb'), kBC_8, kSmallestMipYUV, kBS_4_8, 3, 3, kSFD_BC4_3, oSURFACE_IS_UNORM|oSURFACE_IS_PLANAR|oSURFACE_IS_YUV|oSURFACE_SUBSURFACE1_BIAS1 },
	{ "YBC4_ABC4_UBC4_VBC4_UNORM", oFCC('auvb'), kBC_8, kSmallestMipYUV, kBS_4_8, 3, 4, kSFD_BC4_4, oSURFACE_IS_UNORM|oSURFACE_HAS_ALPHA|oSURFACE_IS_PLANAR|oSURFACE_IS_YUV|oSURFACE_SUBSURFACE1_BIAS1 },
	{ "Y8_U8V8_UNORM", oFCC('yv8u'), kYUV , kSmallestMipYUV, kBS_3_8, 3, 2, kSFD_R8_RG8, oSURFACE_IS_UNORM|oSURFACE_IS_PLANAR|oSURFACE_IS_YUV|oSURFACE_SUBSURFACE1_BIAS1 },
	{ "Y8A8_U8V8_UNORM", oFCC('av8u'), kYAUV, kSmallestMipYUV, kBS_4_8, 4, 2, kSFD_RG8_RG8, oSURFACE_IS_UNORM|oSURFACE_HAS_ALPHA|oSURFACE_IS_PLANAR|oSURFACE_IS_YUV|oSURFACE_SUBSURFACE1_BIAS1 },
	{ "YBC4_UVBC5_UNORM", oFCC('yvbu'), kYUV_BC, kSmallestMipYUV, kBS_3_8, 3, 2, kSFD_BC4_BC5, oSURFACE_IS_UNORM|oSURFACE_IS_PLANAR|oSURFACE_IS_YUV|oSURFACE_SUBSURFACE1_BIAS1 },
	{ "YABC5_UVBC5_UNORM", oFCC('avbu'), kYAUV_BC, kSmallestMipYUV, kBS_4_8, 4, 2, kSFD_BC5_BC5, oSURFACE_IS_UNORM|oSURFACE_HAS_ALPHA|oSURFACE_IS_PLANAR|oSURFACE_IS_YUV|oSURFACE_SUBSURFACE1_BIAS1 },
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

inline bool oSurfaceFormatHasTrait(oSURFACE_FORMAT _Format, oSURFACE_FORMAT_TRAIT _Trait)
{
	return ((_Format) < oSURFACE_NUM_FORMATS) ? !!(sFormatDescs[_Format].Traits & (_Trait)) : false;
}

bool oSurfaceFormatIsBlockCompressed(oSURFACE_FORMAT _Format)
{
	return oSurfaceFormatHasTrait(_Format, oSURFACE_IS_BC);
}

bool oSurfaceFormatIsDepth(oSURFACE_FORMAT _Format)
{
	return oSurfaceFormatHasTrait(_Format, oSURFACE_IS_DEPTH);
}

bool oSurfaceFormatIsAlpha(oSURFACE_FORMAT _Format)
{
	return oSurfaceFormatHasTrait(_Format, oSURFACE_HAS_ALPHA);
}

bool oSurfaceFormatIsUNORM(oSURFACE_FORMAT _Format)
{
	return oSurfaceFormatHasTrait(_Format, oSURFACE_IS_UNORM);
}

bool oSurfaceFormatIsPlanar(oSURFACE_FORMAT _Format)
{
	return oSurfaceFormatHasTrait(_Format, oSURFACE_IS_PLANAR);
}

bool oSurfaceFormatIsYUV(oSURFACE_FORMAT _Format)
{
	return oSurfaceFormatHasTrait(_Format, oSURFACE_IS_YUV);
}

int oSurfaceFormatGetNumChannels(oSURFACE_FORMAT _Format)
{
	return (_Format < oSURFACE_NUM_FORMATS) ? sFormatDescs[_Format].NumChannels : 0;
}

int oSurfaceFormatGetNumSubformats(oSURFACE_FORMAT _Format)
{
	return (_Format < oSURFACE_NUM_FORMATS) ? sFormatDescs[_Format].NumSubformats: 0;
}

int oSurfaceFormatGetSubsampleBias(oSURFACE_FORMAT _Format, int _SubsurfaceIndex)
{
	if (_SubsurfaceIndex == 1 && _Format < oSURFACE_NUM_FORMATS)
	{
		if (sFormatDescs[_Format].Traits & oSURFACE_SUBSURFACE1_BIAS1) return 1;
		if (sFormatDescs[_Format].Traits & oSURFACE_SUBSURFACE1_BIAS2) return 2;
	}

	return 0;
}

int oSurfaceFormatGetSize(oSURFACE_FORMAT _Format, int _SubsurfaceIndex)
{
	return (_Format < oSURFACE_NUM_FORMATS) ? (_SubsurfaceIndex ? oSurfaceFormatGetSize(sFormatDescs[_Format].Subformats.Format[_SubsurfaceIndex]) : sFormatDescs[_Format].Size) : 0;
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

oSURFACE_FORMAT oSurfaceGetSubformat(oSURFACE_FORMAT _Format, int _SubsurfaceIndex)
{
	oASSERT(_SubsurfaceIndex >= 0, "SubsurfaceIndex can't be negative");
	if (sFormatDescs[_Format].NumSubformats < _SubsurfaceIndex)
		return oSURFACE_UNKNOWN;

	if (!!(sFormatDescs[_Format].Traits & oSURFACE_IS_YUV))
		return sFormatDescs[_Format].Subformats.Format[max(_SubsurfaceIndex, 1)];

	return _Format;
}

int oSurfaceCalcNumMips(bool _HasMips, const int3& _Mip0Dimensions)
{
	// Rules of mips are to go to 1x1... so a 1024x8 texture has many more than 4
	// mips.

	if (_Mip0Dimensions.x <= 0 || _Mip0Dimensions.y <= 0 || _Mip0Dimensions.z <= 0)
		return 0;

	int nMips = 1;
	int3 mip = _Mip0Dimensions;
	while (_HasMips && mip != int3(1,1,1))
	{
		nMips++;
		mip = max(int3(1,1,1), mip / int3(2,2,2));
	}

	return nMips;
}

int oSurfaceMipCalcDimension(oSURFACE_FORMAT _Format, int _Mip0Dimension, int _MipLevel, int _SubsurfaceIndex)
{
	oCHECK_DIM(_Format, _Mip0Dimension);
	oASSERT(_Format != oSURFACE_UNKNOWN, "Unknown surface format passed to CalcMipDimension");
	const int subsampleBias = oSurfaceFormatGetSubsampleBias(_Format, _SubsurfaceIndex);
	int d = max(1, _Mip0Dimension >> (_MipLevel + subsampleBias));
	return oSurfaceFormatIsBlockCompressed(_Format) ? static_cast<int>(oByteAlign(d, 4)) : d;
}

int2 oSurfaceMipCalcDimensions(oSURFACE_FORMAT _Format, const int2& _Mip0Dimensions, int _MipLevel, int _SubsurfaceIndex)
{
	return int2(
		oSurfaceMipCalcDimension(_Format, _Mip0Dimensions.x, _MipLevel, _SubsurfaceIndex)
		, oSurfaceMipCalcDimension(_Format, _Mip0Dimensions.y, _MipLevel, _SubsurfaceIndex));
}

int3 oSurfaceMipCalcDimensions(oSURFACE_FORMAT _Format, const int3& _Mip0Dimensions, int _MipLevel, int _SubsurfaceIndex)
{
	return int3(
		oSurfaceMipCalcDimension(_Format, _Mip0Dimensions.x, _MipLevel, _SubsurfaceIndex)
		, oSurfaceMipCalcDimension(_Format, _Mip0Dimensions.y, _MipLevel, _SubsurfaceIndex)
		, oSurfaceMipCalcDimension(oSURFACE_R32_UINT, _Mip0Dimensions.z, _MipLevel, _SubsurfaceIndex)); // No block-compression alignment for depth
}

int oSurfaceMipCalcDimensionNPOT(oSURFACE_FORMAT _Format, int _Mip0Dimension, int _MipLevel, int _SubsurfaceIndex)
{
	oCHECK_DIM(_Format, _Mip0Dimension);

	oSURFACE_FORMAT NthSurfaceFormat = oSurfaceGetSubformat(_Format, _SubsurfaceIndex);
	const int MipLevelBias = oSurfaceFormatGetSubsampleBias(_Format, _SubsurfaceIndex);
	
	// @oooii-tony: This was added while merging oYUVSurface functionality into 
	// oSurface, so we recognize that the int2 values may not be the same and 
	// won't be for formats like YUV9, but first get the lion's share of the code
	// across and revisit this once the main algo is vetted.
	const int2 MinDimension = oSurfaceFormatGetMinDimensions(_Format);

	int d = max(1, _Mip0Dimension >> (_MipLevel + MipLevelBias));
	int NPOTDim = oSurfaceFormatIsBlockCompressed(NthSurfaceFormat) ? static_cast<int>(oByteAlign(d, 4)) : d;

	if (_SubsurfaceIndex == 0 && oSurfaceFormatGetSubsampleBias(_Format, 1) != 0)
		NPOTDim = max(MinDimension.x, NPOTDim & ~(MinDimension.x-1)); // always even down to 2x2

	return NPOTDim;
}

int2 oSurfaceMipCalcDimensionsNPOT(oSURFACE_FORMAT _Format, const int2& _Mip0Dimensions, int _MipLevel, int _SubsurfaceIndex)
{
	#ifdef _DEBUG
		const int2 MinDimension = oSurfaceFormatGetMinDimensions(_Format);
		oASSERT(MinDimension.x == MinDimension.y, "There is currently no support for aniso min dimensions");
	#endif

	return int2(
		oSurfaceMipCalcDimensionNPOT(_Format, _Mip0Dimensions.x, _MipLevel, _SubsurfaceIndex)
		, oSurfaceMipCalcDimensionNPOT(_Format, _Mip0Dimensions.y, _MipLevel, _SubsurfaceIndex));
}

int3 oSurfaceMipCalcDimensionsNPOT(oSURFACE_FORMAT _Format, const int3& _Mip0Dimensions, int _MipLevel, int _SubsurfaceIndex)
{
	#ifdef _DEBUG
		const int2 MinDimension = oSurfaceFormatGetMinDimensions(_Format);
		oASSERT(MinDimension.x == MinDimension.y, "There is currently no support for aniso min dimensions");
	#endif

	return int3(
		oSurfaceMipCalcDimensionNPOT(_Format, _Mip0Dimensions.x, _MipLevel, _SubsurfaceIndex)
		, oSurfaceMipCalcDimensionNPOT(_Format, _Mip0Dimensions.y, _MipLevel, _SubsurfaceIndex)
		, oSurfaceMipCalcDimensionNPOT(oSURFACE_R32_UINT, _Mip0Dimensions.z, _MipLevel, _SubsurfaceIndex)); // No block-compression alignment for depth
}

int oSurfaceMipCalcRowSize(oSURFACE_FORMAT _Format, int _MipWidth, int _SubsurfaceIndex)
{
	oCHECK_DIM(_Format, _MipWidth);
	oASSERT(_Format != oSURFACE_UNKNOWN, "Unknown surface format passed to GetRowPitch");
	int w = oSurfaceMipCalcDimension(_Format, _MipWidth);
	if (oSurfaceFormatIsBlockCompressed(_Format)) // because the atom is a 4x4 block
		w /= 4;
	const int s = oSurfaceFormatGetSize(_Format, _SubsurfaceIndex);
	return oInt(oByteAlign(w * s, s));
}

int oSurfaceMipCalcRowPitch(const oSURFACE_DESC& _SurfaceDesc, int _MipLevel, int _SubsurfaceIndex)
{
	oCHECK_SURFACE_DESC(_SurfaceDesc);
	const int numMips = oSurfaceCalcNumMips(_SurfaceDesc.Layout, _SurfaceDesc.Dimensions);
	if (_MipLevel >= numMips)
		return oInvalid;

	switch (_SurfaceDesc.Layout)
	{
		case oSURFACE_LAYOUT_IMAGE: 
			return oSurfaceMipCalcRowSize(_SurfaceDesc.Format, _SurfaceDesc.Dimensions, _SubsurfaceIndex);
		case oSURFACE_LAYOUT_TIGHT: 
			return oSurfaceMipCalcRowSize(_SurfaceDesc.Format, oSurfaceMipCalcDimensionNPOT(_SurfaceDesc.Format, _SurfaceDesc.Dimensions.x, _MipLevel, _SubsurfaceIndex), _SubsurfaceIndex);
		case oSURFACE_LAYOUT_BELOW: 
		{
			const int mip0RowSize = oSurfaceMipCalcRowSize(_SurfaceDesc.Format, _SurfaceDesc.Dimensions.x, _SubsurfaceIndex);
			if (numMips > 2)
			{
					return max(mip0RowSize, 
					oSurfaceMipCalcRowSize(_SurfaceDesc.Format, oSurfaceMipCalcDimensionNPOT(_SurfaceDesc.Format, _SurfaceDesc.Dimensions.x, 1, _SubsurfaceIndex)) + 
					oSurfaceMipCalcRowSize(_SurfaceDesc.Format, oSurfaceMipCalcDimensionNPOT(_SurfaceDesc.Format, _SurfaceDesc.Dimensions.x, 2, _SubsurfaceIndex)) );
			}
			else
				return mip0RowSize;
		}

		case oSURFACE_LAYOUT_RIGHT: 
		{
			const int mip0RowSize = oSurfaceMipCalcRowSize(_SurfaceDesc.Format, _SurfaceDesc.Dimensions.x, _SubsurfaceIndex);
			if (numMips > 1)
				return mip0RowSize + oSurfaceMipCalcRowSize(_SurfaceDesc.Format, oSurfaceMipCalcDimensionNPOT(_SurfaceDesc.Format, _SurfaceDesc.Dimensions.x, 1, _SubsurfaceIndex), _SubsurfaceIndex);
			else
				return mip0RowSize;
		}

		oNODEFAULT;
	}
}

int2 oSurfaceCalcDimensions(const oSURFACE_DESC& _SurfaceDesc, int _SubsurfaceIndex)
{
	int2 sliceDimensions = oSurfaceSliceCalcDimensions(_SurfaceDesc, _SubsurfaceIndex);
	return int2(sliceDimensions.x, sliceDimensions.y * _SurfaceDesc.NumSlices);
}

int2 oSurfaceSliceCalcDimensions(const oSURFACE_DESC& _SurfaceDesc, int _SubsurfaceIndex)
{
	oCHECK_SURFACE_DESC(_SurfaceDesc);
	int3 mip0dimensions = oSurfaceMipCalcDimensionsNPOT(_SurfaceDesc.Format, _SurfaceDesc.Dimensions, 0, _SubsurfaceIndex);
	switch (_SurfaceDesc.Layout)
	{
		case oSURFACE_LAYOUT_IMAGE: 
			return int2(mip0dimensions.x, (mip0dimensions.y * mip0dimensions.z));
		
		case oSURFACE_LAYOUT_TIGHT: 
		{
			const int surfaceSlicePitch = oSurfaceSliceCalcPitch(_SurfaceDesc, _SubsurfaceIndex);
			const int mip0RowPitch = oSurfaceMipCalcRowPitch(_SurfaceDesc, 0, _SubsurfaceIndex);
			return int2(mip0dimensions.x, (surfaceSlicePitch / mip0RowPitch));
		}
		case oSURFACE_LAYOUT_BELOW: 
		{
			int numMips = oSurfaceCalcNumMips(_SurfaceDesc.Layout, mip0dimensions);
			int3 mip1dimensions = numMips > 1 ? oSurfaceMipCalcDimensionsNPOT(_SurfaceDesc.Format, mip0dimensions, 1, _SubsurfaceIndex) : int3(0);
			int3 mip2dimensions = numMips > 2 ? oSurfaceMipCalcDimensionsNPOT(_SurfaceDesc.Format, mip0dimensions, 2, _SubsurfaceIndex) : int3(0);

			int mip0height = mip0dimensions.y * mip0dimensions.z;
			int mip1height = mip1dimensions.y * mip1dimensions.z;
			int mip2andUpHeight = mip2dimensions.y * mip2dimensions.z;
			for (int mip=3; mip<numMips; ++mip)
			{
				int3 mipNdimensions = oSurfaceMipCalcDimensionsNPOT(_SurfaceDesc.Format, mip0dimensions, mip, _SubsurfaceIndex);
				mip2andUpHeight += mipNdimensions.y * mipNdimensions.z;
			}
			return int2(max(mip0dimensions.x, mip1dimensions.x + mip2dimensions.x), (mip0height + max(mip1height, mip2andUpHeight)));
		}
		case oSURFACE_LAYOUT_RIGHT: 
		{

			int numMips = oSurfaceCalcNumMips(_SurfaceDesc.Layout, mip0dimensions);
			int3 mip1dimensions = numMips > 1 ? oSurfaceMipCalcDimensionsNPOT(_SurfaceDesc.Format, mip0dimensions, 1, _SubsurfaceIndex) : int3(0);

			int mip0height = mip0dimensions.y * mip0dimensions.z;
			int mip1andUpHeight = mip1dimensions.y * mip1dimensions.z;
			for (int mip=2; mip<numMips; ++mip)
			{
				int3 mipNdimensions = oSurfaceMipCalcDimensionsNPOT(_SurfaceDesc.Format, mip0dimensions, mip, _SubsurfaceIndex);
				mip1andUpHeight += mipNdimensions.y * mipNdimensions.z;
			}
			return int2(mip0dimensions.x + mip1dimensions.x, max(mip0height, mip1andUpHeight));
		}
		oNODEFAULT;
	}
}

int oSurfaceMipCalcNumColumns(oSURFACE_FORMAT _Format, int _MipWidth, int _SubsurfaceIndex)
{
	oCHECK_DIM(_Format, _MipWidth);
	int widthInPixels = oSurfaceMipCalcDimension(_Format, _MipWidth, _SubsurfaceIndex);
	return oSurfaceFormatIsBlockCompressed(_Format) ? __max(1, widthInPixels/4) : widthInPixels;
}

int oSurfaceMipCalcNumRows(oSURFACE_FORMAT _Format, int _MipHeight, int _SubsurfaceIndex)
{
	oCHECK_DIM(_Format, _MipHeight);
	int heightInPixels = oSurfaceMipCalcDimensionNPOT(_Format, _MipHeight, _SubsurfaceIndex);
	return oSurfaceFormatIsBlockCompressed(_Format) ? __max(1, heightInPixels/4) : heightInPixels;
}

int oSurfaceMipCalcSize(oSURFACE_FORMAT _Format, const int2& _MipDimensions, int _SubsurfaceIndex)
{
	oCHECK_DIM2(_Format, _MipDimensions);
	return oInt(oSurfaceMipCalcRowSize(_Format, _MipDimensions, _SubsurfaceIndex) * oSurfaceMipCalcNumRows(_Format, _MipDimensions, _SubsurfaceIndex));
}

static int oSurfaceMipCalcOffset_Tight(const oSURFACE_DESC& _SurfaceDesc, int _MipLevel, int _SubsurfaceIndex)
{
	oCHECK_SURFACE_DESC(_SurfaceDesc);

	int3 mip0dimensions = _SurfaceDesc.Dimensions;

	oInt offset = 0;
	int mip = 0;
	while (mip != _MipLevel)
	{
		int3 previousMipDimensions = oSurfaceMipCalcDimensionsNPOT(_SurfaceDesc.Format, mip0dimensions, mip, _SubsurfaceIndex);
		offset += oSurfaceMipCalcSize(_SurfaceDesc.Format, previousMipDimensions, _SubsurfaceIndex);
		mip++;
	}

	return offset;
}

static int oSurfaceMipCalcOffset_Below(const oSURFACE_DESC& _SurfaceDesc, int _MipLevel, int _SubsurfaceIndex)
{
	oCHECK_SURFACE_DESC(_SurfaceDesc);
	if (0 == _MipLevel)
		return 0;

	int3 mip0dimensions = _SurfaceDesc.Dimensions;
	int3 mip1dimensions = oSurfaceMipCalcDimensionsNPOT(_SurfaceDesc.Format, _SurfaceDesc.Dimensions, 1, _SubsurfaceIndex);
	int surfaceRowPitch = oSurfaceMipCalcRowPitch(_SurfaceDesc, 0, _SubsurfaceIndex);

	// Step down when moving from Mip0 to Mip1
	oInt offset = surfaceRowPitch * oSurfaceMipCalcNumRows(_SurfaceDesc.Format, mip0dimensions, _SubsurfaceIndex);
	if (1 == _MipLevel)
		return offset;

	// Step right when moving from Mip1 to Mip2
	offset += oSurfaceMipCalcRowSize(_SurfaceDesc.Format, mip1dimensions, _SubsurfaceIndex);

	// Step down for all of the other MIPs
	int mip = 2;
	while (mip != _MipLevel)
	{
		int3 previousMipDimensions = oSurfaceMipCalcDimensionsNPOT(_SurfaceDesc.Format, mip0dimensions, mip, _SubsurfaceIndex);
		offset += surfaceRowPitch * oSurfaceMipCalcNumRows(_SurfaceDesc.Format, previousMipDimensions, _SubsurfaceIndex);
		mip++;
	}		

	return offset;
}

static int oSurfaceMipCalcOffset_Right(const oSURFACE_DESC& _SurfaceDesc, int _MipLevel, int _SubsurfaceIndex)
{
	oCHECK_SURFACE_DESC(_SurfaceDesc);
	if (0 == _MipLevel)
		return 0;

	int3 mip0dimensions = _SurfaceDesc.Dimensions;
	int surfaceRowPitch = oSurfaceMipCalcRowPitch(_SurfaceDesc, 0, _SubsurfaceIndex);

	// Step right when moving from Mip0 to Mip1
	oInt offset = oSurfaceMipCalcRowSize(_SurfaceDesc.Format, mip0dimensions, _SubsurfaceIndex);

	// Step down for all of the other MIPs
	int mip = 1;
	while (mip != _MipLevel)
	{
		int3 previousMipDimensions = oSurfaceMipCalcDimensionsNPOT(_SurfaceDesc.Format, mip0dimensions, mip, _SubsurfaceIndex);
		offset += surfaceRowPitch * oSurfaceMipCalcNumRows(_SurfaceDesc.Format, previousMipDimensions, _SubsurfaceIndex);
		mip++;
	}		

	return offset;
}

int oSurfaceMipCalcOffset(const oSURFACE_DESC& _SurfaceDesc, int _MipLevel, int _SubsurfaceIndex)
{
	oCHECK_SURFACE_DESC(_SurfaceDesc);
	const int numMips = oSurfaceCalcNumMips(_SurfaceDesc.Layout, _SurfaceDesc.Dimensions);
	if (_MipLevel >= numMips) 
		return oInvalid;

	switch (_SurfaceDesc.Layout)
	{
		case oSURFACE_LAYOUT_IMAGE: return 0;
		case oSURFACE_LAYOUT_TIGHT: return oSurfaceMipCalcOffset_Tight(_SurfaceDesc, _MipLevel, _SubsurfaceIndex);
		case oSURFACE_LAYOUT_BELOW: return oSurfaceMipCalcOffset_Below(_SurfaceDesc, _MipLevel, _SubsurfaceIndex);
		case oSURFACE_LAYOUT_RIGHT: return oSurfaceMipCalcOffset_Right(_SurfaceDesc, _MipLevel, _SubsurfaceIndex);
		oNODEFAULT;
	}
}

int oSurfaceSliceCalcPitch(const oSURFACE_DESC& _SurfaceDesc, int _SubsurfaceIndex)
{
	oCHECK_SURFACE_DESC(_SurfaceDesc);
	oInt pitch = 0;

	switch (_SurfaceDesc.Layout)
	{
		case oSURFACE_LAYOUT_IMAGE:
		case oSURFACE_LAYOUT_RIGHT:
		case oSURFACE_LAYOUT_BELOW:
			return oSurfaceMipCalcSize(_SurfaceDesc.Format, oSurfaceSliceCalcDimensions(_SurfaceDesc, _SubsurfaceIndex), _SubsurfaceIndex);

		case oSURFACE_LAYOUT_TIGHT:
		{
			// Sum the size of all mip levels
			int3 dimensions = _SurfaceDesc.Dimensions;
			int nMips = oSurfaceCalcNumMips(_SurfaceDesc.Layout, dimensions);
			while (nMips > 0)
			{
				pitch += oSurfaceMipCalcSize(_SurfaceDesc.Format, dimensions.xy(), _SubsurfaceIndex) * dimensions.z;
				dimensions = max(int3(1,1,1), dimensions / int3(2,2,2));
				nMips--;
			}

			// Align slicePitch to mip0RowPitch
			const int mip0RowPitch = oSurfaceMipCalcRowPitch(_SurfaceDesc, 0, _SubsurfaceIndex);
			pitch = (((pitch + (mip0RowPitch - 1)) / mip0RowPitch) * mip0RowPitch);
			break;
		}
		oNODEFAULT;
	}

	return pitch;
}

int oSurfaceMipCalcDepthPitch(const oSURFACE_DESC& _SurfaceDesc, int _MipLevel, int _SubsurfaceIndex)
{
	oCHECK_SURFACE_DESC(_SurfaceDesc);

	int3 mipDimensions = oSurfaceMipCalcDimensionsNPOT(_SurfaceDesc.Format, _SurfaceDesc.Dimensions, _MipLevel, _SubsurfaceIndex);
	return oSurfaceMipCalcRowPitch(_SurfaceDesc, _MipLevel, _SubsurfaceIndex) * oSurfaceMipCalcNumRows(_SurfaceDesc.Format, mipDimensions.xy(), _SubsurfaceIndex);
}

int oSurfaceCalcSize(const oSURFACE_DESC& _SurfaceDesc, int _SubsurfaceIndex)
{
	return oInt(oSurfaceSliceCalcPitch(_SurfaceDesc, _SubsurfaceIndex) * _SurfaceDesc.NumSlices);
}

void oSurfaceSubresourceGetDesc(const oSURFACE_DESC& _SurfaceDesc, int _Subresource, oSURFACE_SUBRESOURCE_DESC* _pSubresourceDesc)
{
	oCHECK_SURFACE_DESC(_SurfaceDesc);
	int numMips = oSurfaceCalcNumMips(_SurfaceDesc.Layout, _SurfaceDesc.Dimensions);
	oSurfaceSubresourceUnpack(_Subresource, numMips, _SurfaceDesc.NumSlices, &_pSubresourceDesc->MipLevel, &_pSubresourceDesc->Slice, &_pSubresourceDesc->Subsurface);
	oASSERT(_pSubresourceDesc->Slice < _SurfaceDesc.NumSlices, "Slice index is out of range for the specified surface");
	oASSERT(_pSubresourceDesc->Subsurface < oSurfaceFormatGetNumSubformats(_SurfaceDesc.Format), "Subsurface index is out of range for the specified surface");
	_pSubresourceDesc->Dimensions = oSurfaceMipCalcDimensionsNPOT(_SurfaceDesc.Format, _SurfaceDesc.Dimensions, _pSubresourceDesc->MipLevel, _pSubresourceDesc->Subsurface);
}

int oSurfaceSubresourceCalcSize(const oSURFACE_DESC& _SurfaceDesc, const oSURFACE_SUBRESOURCE_DESC& _SubresourceDesc)
{
	return oSurfaceMipCalcSize(_SurfaceDesc.Format, _SubresourceDesc.Dimensions, _SubresourceDesc.Subsurface);
}

int oSurfaceSubresourceCalcOffset(const oSURFACE_DESC& _SurfaceDesc, int _Subresource, int _DepthIndex)
{
	oASSERT(_DepthIndex < _SurfaceDesc.Dimensions.z, "Depth index is out of range");
	oSURFACE_SUBRESOURCE_DESC ssrd;
	oSurfaceSubresourceGetDesc(_SurfaceDesc, _Subresource, &ssrd);

	int offset = oSurfaceMipCalcOffset(_SurfaceDesc, ssrd.MipLevel, ssrd.Subsurface);
	if (_DepthIndex)
		offset += oSurfaceMipCalcDepthPitch(_SurfaceDesc, ssrd.MipLevel, ssrd.Subsurface) * _DepthIndex;
	else if (ssrd.Slice)
		offset += oSurfaceSliceCalcPitch(_SurfaceDesc, ssrd.Subsurface) * ssrd.Slice;

	return offset;
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
	oASSERT_PLANAR_SUPPORT(_SurfaceDesc.Format);

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
	oASSERT_PLANAR_SUPPORT(_SurfaceDesc.Format);

	int numTilesPerSlice = oSurfaceSliceCalcNumTiles(_SurfaceDesc, _TileDimensions);
	return _Slice * numTilesPerSlice;
}

// how many tiles from the startID to start the specified mip
static int oSurfaceSliceCalcMipTileIDOffset(const oSURFACE_DESC& _SurfaceDesc, const int2& _TileDimensions, int _MipLevel)
{
	oASSERT_PLANAR_SUPPORT(_SurfaceDesc.Format);

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
	oASSERT_PLANAR_SUPPORT(_SurfaceDesc.Format);

	int sliceStartID = oSurfaceSliceCalcStartTileID(_SurfaceDesc, _TileDimensions, _Slice);
	int mipIDOffset = oSurfaceSliceCalcMipTileIDOffset(_SurfaceDesc, _TileDimensions, _MipLevel);
	return sliceStartID + mipIDOffset;
}

int oSurfaceCalcTile(const oSURFACE_DESC& _SurfaceDesc, const int2& _TileDimensions, oSURFACE_TILE_DESC& _InOutTileDesc)
{
	oASSERT_PLANAR_SUPPORT(_SurfaceDesc.Format);

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
	oASSERT_PLANAR_SUPPORT(_SurfaceDesc.Format);

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
	oASSERT_PLANAR_SUPPORT(_Format);

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
		*_pByteDimensions = oSurfaceMipCalcByteDimensions(_SurfaceDesc.Format, ssrd.Dimensions.xy(), ssrd.Subsurface);
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

void oSurfaceCopySubresource(const oSURFACE_DESC& _SurfaceDesc, const oSURFACE_CONST_MAPPED_SUBRESOURCE& _SrcMap, oSURFACE_MAPPED_SUBRESOURCE* _DstMap, bool _FlipVertical)
{
	if (_FlipVertical)
		oMemcpy2dVFlip(_DstMap->pData, _DstMap->RowPitch, _SrcMap.pData, _SrcMap.RowPitch, _SurfaceDesc.Dimensions.x*oSurfaceFormatGetSize(_SurfaceDesc.Format), _SurfaceDesc.Dimensions.y);
	else
		oMemcpy2d(_DstMap->pData, _DstMap->RowPitch, _SrcMap.pData, _SrcMap.RowPitch, _SurfaceDesc.Dimensions.x*oSurfaceFormatGetSize(_SurfaceDesc.Format), _SurfaceDesc.Dimensions.y);
}
