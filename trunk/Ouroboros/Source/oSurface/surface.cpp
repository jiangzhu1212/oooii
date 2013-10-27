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
#include <oSurface/surface.h>
#include <oBase/assert.h>
#include <oBase/byte.h>
#include <oBase/memory.h>
#include <oBase/throw.h>
#include <oStd/atomic.h>
#include <oHLSL/oHLSLMath.h>

using namespace std::placeholders;
using namespace oStd;

namespace ouro {
	namespace surface {

#define oCHECK_INFO(_Info) \
	if (any(_Info.dimensions < int3(1,1,1))) throw std::invalid_argument(formatf("invalid dimensions: [%d,%d,%d]", _Info.dimensions.x, _Info.dimensions.y, _Info.dimensions.z)); \
	if (max(1, _Info.array_size) != 1 && _Info.dimensions.z != 1) throw std::invalid_argument(formatf("array_size or depth has to be 1 [%d,%d]", _Info.array_size, _Info.dimensions.z));

#define oCHECK_DIM(_Format, _Dim) if (_Dim < min_dimensions(_Format).x) throw std::invalid_argument(formatf("invalid dimension: %d", _Dim));
#define oCHECK_DIM2(_Format, _Dim) if (any(_Dim < min_dimensions(_Format))) throw std::invalid_argument(formatf("invalid dimensions: [%d,%d]", _Dim.x, _Dim.y));
#define oCHECK_DIM3(_Format, _Dim) if (any(_Dim.xy < min_dimensions(_Format))) throw std::invalid_argument(formatf("invalid dimensions: [%d,%d,%d]", _Dim.x, _Dim.y, _Dim.z));
#define oCHECK_NOT_PLANAR(_Format) if (is_planar(_Format)) throw std::invalid_argument("Planar formats may not behave well with this API. Review usage in this code and remove this when verified.");

struct bit_size { unsigned char r,g,b,a; };
struct subformats { enum format format[4]; };

namespace traits
{	enum value {

	is_bc = 1 << 0,
	is_unorm = 1 << 1,
	has_alpha = 1 << 2,
	is_depth = 1 << 3,
	is_planar = 1 << 4 ,
	is_yuv = 1 << 5,
	paletted = 1 << 6,
	subsurface1_bias1 = 1 << 7,
	subsurface2_bias1 = 1 << 8,
	subsurface3_bias1 = 1 << 9,

};}

struct format_info
{
	const char* string;
	unsigned int fourcc;
	unsigned int size;
	int2 min_dimensions;
	struct bit_size bit_size;
	unsigned char num_channels;
	unsigned char num_subformats;
	
	struct subformats subformats;
	int traits;
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

static const bit_size kBS_UNK = {0,0,0,0};
static const bit_size kBS_4_32 = {32,32,32,32};
static const bit_size kBS_3_32 = {32,32,32,0};
static const bit_size kBS_2_32 = {32,32,0,0};
static const bit_size kBS_1_32 = {32,0,0,0};
static const bit_size kBS_4_16 = {16,16,16,16};
static const bit_size kBS_3_16 = {16,16,16,0};
static const bit_size kBS_2_16 = {16,16,0,0};
static const bit_size kBS_1_16 = {16,0,0,0};
static const bit_size kBS_4_8 = {8,8,8,8};
static const bit_size kBS_3_8 = {8,8,8,0};
static const bit_size kBS_2_8 = {8,8,0,0};
static const bit_size kBS_1_8 = {8,0,0,0};
static const bit_size kBS_565 = {5,6,5,0};
static const bit_size kBS_DEC3N = {10,10,10,2};
static const bit_size kBS_DS = {24,8,0,0};
static const bit_size kBS_4_4 = {4,4,4,4};
static const bit_size kBS_3_4 = {4,4,4,0};
static const bit_size kBS_2_4 = {4,4,0,0};
static const bit_size kBS_1_4 = {4,0,0,0};

static const fourcc kFCC_UNK = oFCC('????');

static const subformats kSFD_UNK = {unknown, unknown, unknown, unknown};
static const subformats kSFD_R8_R8 = {r8_unorm, r8_unorm, unknown, unknown};
static const subformats kSFD_R8_RG8 = {r8_unorm, r8g8_unorm, unknown, unknown};
static const subformats kSFD_RG8_RG8 = {r8g8_unorm, r8g8_unorm, unknown, unknown};
static const subformats kSFD_R16_RG16 = {r16_unorm, r16g16_unorm, unknown, unknown};
static const subformats kSFD_BC4_BC4 = {bc4_unorm, bc4_unorm, unknown, unknown};
static const subformats kSFD_BC4_BC5 = {bc4_unorm, bc5_unorm, unknown, unknown};
static const subformats kSFD_BC5_BC5 = {bc5_unorm, bc5_unorm, unknown, unknown};
static const subformats kSFD_R8_4 = {r8_unorm, r8_unorm, r8_unorm, r8_unorm};
static const subformats kSFD_R8_3 = {r8_unorm, r8_unorm, r8_unorm, unknown};
static const subformats kSFD_BC4_4 = {bc4_unorm, bc4_unorm, bc4_unorm, bc4_unorm};
static const subformats kSFD_BC4_3 = {bc4_unorm, bc4_unorm, bc4_unorm, unknown};

static const format_info sFormatInfo[] = 
{
	{ "unknown", kFCC_UNK, k0_0, kSmallestMip, kBS_UNK, 0, 0, kSFD_UNK, 0 },
	{ "r32g32b32a32_typeless", oFCC('?i4 '), k4_32, kSmallestMip, kBS_4_32, 4, 1, kSFD_UNK, traits::has_alpha },
	{ "r32g32b32a32_float", oFCC('f4  '), k4_32, kSmallestMip, kBS_4_32, 4, 1, kSFD_UNK, traits::has_alpha },
	{ "r32g32b32a32_uint", oFCC('ui4 '), k4_32, kSmallestMip, kBS_4_32, 4, 1, kSFD_UNK, traits::has_alpha },
	{ "r32g32b32a32_sint", oFCC('si4 '), k4_32, kSmallestMip, kBS_4_32, 4, 1, kSFD_UNK, traits::has_alpha },
	{ "r32g32b32_typeless", oFCC('?i3 '), k3_32, kSmallestMip, kBS_3_32, 3, 1, kSFD_UNK, traits::has_alpha },
	{ "r32g32b32_float", oFCC('f3  '), k3_32, kSmallestMip, kBS_3_32, 3, 1, kSFD_UNK, 0 },
	{ "r32g32b32_uint", oFCC('ui3 '), k4_32, kSmallestMip, kBS_3_32, 3, 1, kSFD_UNK, 0 },
	{ "r32g32b32_sint", oFCC('si3 '), k4_32, kSmallestMip, kBS_3_32, 3, 1, kSFD_UNK, 0 },
	{ "r16g16b16a16_typeless", oFCC('?s4 '), k4_16, kSmallestMip, kBS_4_16, 4, 1, kSFD_UNK, traits::has_alpha },
	{ "r16g16b16a16_float", oFCC('h4  '), k4_16, kSmallestMip, kBS_4_16, 4, 1, kSFD_UNK, traits::has_alpha },
	{ "r16g16b16a16_unorm", oFCC('h4u '), k4_16, kSmallestMip, kBS_4_16, 4, 1, kSFD_UNK, traits::is_unorm|traits::has_alpha },
	{ "r16g16b16a16_uint", oFCC('us4 '), k4_16, kSmallestMip, kBS_4_16, 4, 1, kSFD_UNK, traits::has_alpha },
	{ "r16g16b16a16_snorm", oFCC('h4s '), k4_16, kSmallestMip, kBS_4_16, 4, 1, kSFD_UNK, traits::has_alpha },
	{ "r16g16b16a16_sint", oFCC('ss4 '), k4_16, kSmallestMip, kBS_4_16, 4, 1, kSFD_UNK, traits::has_alpha },
	{ "r32g32_typeless", oFCC('?i2 '), k2_32, kSmallestMip, kBS_2_32, 2, 1, kSFD_UNK, 0 },
	{ "r32g32_float", oFCC('f2  '), k2_32, kSmallestMip, kBS_2_32, 2, 1, kSFD_UNK, 0 },
	{ "r32g32_uint", oFCC('ui2 '), k2_32, kSmallestMip, kBS_2_32, 2, 1, kSFD_UNK, 0 },
	{ "R32G32_sINT", oFCC('si2 '), k2_32, kSmallestMip, kBS_2_32, 2, 1, kSFD_UNK, 0 },
	{ "r32g8x24_typeless", kFCC_UNK, k2_32, kSmallestMip, {32,8,0,24}, 3, 1, kSFD_UNK, 0 },
	{ "d32_float_s8x24_uint", kFCC_UNK, k2_32, kSmallestMip, {32,8,0,24}, 3, 1, kSFD_UNK, traits::is_depth },
	{ "r32_float_x8x24_typeless", kFCC_UNK, k2_32, kSmallestMip, {32,8,0,24}, 3, 1, kSFD_UNK, traits::is_depth },
	{ "x32_typeless_g8x24_uint", kFCC_UNK, k2_32, kSmallestMip, {32,8,0,24}, 3, 1, kSFD_UNK, 0 },
	{ "r10g10b10a2_typeless", kFCC_UNK, k1_32, kSmallestMip, kBS_DEC3N, 4, 1, kSFD_UNK, traits::has_alpha },
	{ "r10g10b10a2_unorm", kFCC_UNK, k1_32, kSmallestMip, kBS_DEC3N, 4, 1, kSFD_UNK, traits::is_unorm|traits::has_alpha },
	{ "r10g10b10a2_uint", kFCC_UNK, k1_32, kSmallestMip, kBS_DEC3N, 4, 1, kSFD_UNK, traits::has_alpha },
	{ "r11g11b10_float", kFCC_UNK, k1_32, kSmallestMip, {11,11,10,0}, 3, 1, kSFD_UNK, 0 },
	{ "r8g8b8a8_typeless", oFCC('?c4 '), k1_32, kSmallestMip, kBS_4_8, 4, 1, kSFD_UNK, traits::has_alpha },
	{ "r8g8b8a8_unorm", oFCC('c4u '), k1_32, kSmallestMip, kBS_4_8, 4, 1, kSFD_UNK, traits::is_unorm|traits::has_alpha },
	{ "r8g8b8a8_unorm_srgb", oFCC('c4us'), k1_32, kSmallestMip, kBS_4_8, 4, 1, kSFD_UNK, traits::is_unorm|traits::has_alpha },
	{ "r8g8b8a8_uint", oFCC('uc4 '), k1_32, kSmallestMip, kBS_4_8, 4, 1, kSFD_UNK, traits::has_alpha },
	{ "r8g8b8a8_snorm", oFCC('c4s '), k1_32, kSmallestMip, kBS_4_8, 4, 1, kSFD_UNK, traits::has_alpha },
	{ "r8g8b8a8_sint", oFCC('sc4 '), k1_32, kSmallestMip, kBS_4_8, 4, 1, kSFD_UNK, traits::has_alpha },
	{ "r16g16_typeless", oFCC('?s2 '), k2_16, kSmallestMip, kBS_2_16, 2, 1, kSFD_UNK, 0 },
	{ "r16g16_float", oFCC('h2 '), k2_16, kSmallestMip, kBS_2_16, 2, 1, kSFD_UNK, 0 },
	{ "r16g16_unorm", oFCC('h2u '), k2_16, kSmallestMip, kBS_2_16, 2, 1, kSFD_UNK, traits::is_unorm },
	{ "r16g16_uint", oFCC('us2 '), k2_16, kSmallestMip, kBS_2_16, 2, 1, kSFD_UNK, 0 },
	{ "r16g16_snorm", oFCC('h2s '), k2_16, kSmallestMip, kBS_2_16, 2, 1, kSFD_UNK, 0 },
	{ "r16g16_sint", oFCC('ss2 '), k2_16, kSmallestMip, kBS_2_16, 2, 1, kSFD_UNK, 0 },
	{ "r32_typeless", oFCC('?i1 '), k1_32, kSmallestMip, kBS_1_32, 1, 1, kSFD_UNK, traits::is_depth },
	{ "d32_float", oFCC('f1d '), k1_32, kSmallestMip, kBS_1_32, 1, 1, kSFD_UNK, traits::is_depth },
	{ "r32_float", oFCC('f1  '), k1_32, kSmallestMip, kBS_1_32, 1, 1, kSFD_UNK, 0 },
	{ "r32_uint", oFCC('ui1 '), k1_32, kSmallestMip, kBS_1_32, 1, 1, kSFD_UNK, 0 },
	{ "r32_sint", oFCC('si1 '), k1_32, kSmallestMip, kBS_1_32, 1, 1, kSFD_UNK, 0 },
	{ "r24g8_typeless", kFCC_UNK, k1_32, kSmallestMip, kBS_DS, 2, 1, kSFD_UNK, traits::is_depth },
	{ "d24_unorm_s8_uint", kFCC_UNK, k1_32, kSmallestMip, kBS_DS, 2, 1, kSFD_UNK, traits::is_unorm|traits::is_depth },
	{ "r24_unorm_x8_typeless", kFCC_UNK, k1_32, kSmallestMip, kBS_DS, 2, 1, kSFD_UNK, traits::is_unorm|traits::is_depth },
	{ "x24_typeless_g8_uint", kFCC_UNK, k1_32, kSmallestMip, kBS_DS, 2, 1, kSFD_UNK, traits::is_depth },
	{ "r8g8_typeless", oFCC('?c2 '), k2_8, kSmallestMip, kBS_2_8, 2, 1, kSFD_UNK, 0 },
	{ "r8g8_unorm", oFCC('uc2u'), k2_8, kSmallestMip, kBS_2_8, 2, 1, kSFD_UNK, traits::is_unorm },
	{ "r8g8_uint", oFCC('ui2 '), k2_8, kSmallestMip, kBS_2_8, 2, 1, kSFD_UNK, 0 },
	{ "r8g8_snorm", oFCC('uc2s'), k2_8, kSmallestMip, kBS_2_8, 2, 1, kSFD_UNK, 0 },
	{ "r8g8_sint", oFCC('si2 '), k2_8, kSmallestMip, kBS_2_8, 2, 1, kSFD_UNK, 0 },
	{ "r16_typeless", oFCC('?s1 '), k1_16, kSmallestMip, kBS_1_16, 1, 1, kSFD_UNK, traits::is_depth },
	{ "r16_float", oFCC('h1  '), k1_16, kSmallestMip, kBS_1_16, 1, 1, kSFD_UNK, 0 },
	{ "d16_unorm", oFCC('h1ud'), k1_16, kSmallestMip, kBS_1_16, 1, 1, kSFD_UNK, traits::is_unorm|traits::is_depth },
	{ "r16_unorm", oFCC('h1u '), k1_16, kSmallestMip, kBS_1_16, 1, 1, kSFD_UNK, traits::is_unorm },
	{ "r16_uint", oFCC('us1 '), k1_16, kSmallestMip, kBS_1_16, 1, 1, kSFD_UNK, 0 },
	{ "r16_snorm", oFCC('h1s '), k1_16, kSmallestMip, kBS_1_16, 1, 1, kSFD_UNK, 0 },
	{ "r16_sint", oFCC('ss1 '), k1_16, kSmallestMip, kBS_1_16, 1, 1, kSFD_UNK, 0 },
	{ "r8_typeless", oFCC('?c1 '), k1_8, kSmallestMip, kBS_1_8, 1, 1, kSFD_UNK, 0 },
	{ "r8_unorm", oFCC('uc1u'), k1_8, kSmallestMip, kBS_1_8, 1, 1, kSFD_UNK, traits::is_unorm },
	{ "r8_uint", oFCC('uc1 '), k1_8, kSmallestMip, kBS_1_8, 1, 1, kSFD_UNK, 0 },
	{ "r8_snorm", oFCC('uc1s'), k1_8, kSmallestMip, kBS_1_8, 1, 1, kSFD_UNK, 0 },
	{ "r8_sint", oFCC('sc1 '), k1_8, kSmallestMip, kBS_1_8, 1, 1, kSFD_UNK, 0 },
	{ "a8_unorm", oFCC('ac1u'), k1_8, kSmallestMip, kBS_1_8, 1, 1, kSFD_UNK, traits::is_unorm|traits::has_alpha },
	{ "r1_unorm", oFCC('bitu'), 1, kSmallestMip, {1,0,0,0}, 1, 1, kSFD_UNK, traits::is_unorm },
	{ "r9g9b9e5_sharedexp", kFCC_UNK, k1_32, kSmallestMip, {9,9,9,5}, 4, 1, kSFD_UNK, 0 },
	{ "r8g8_b8g8_unorm", kFCC_UNK, k1_32, kSmallestMip, kBS_4_8, 4, 1, kSFD_UNK, traits::is_unorm },
	{ "g8r8_g8b8_unorm", kFCC_UNK, k1_32, kSmallestMip, kBS_4_8, 4, 1, kSFD_UNK, traits::is_unorm },
	{ "bc1_typeless", oFCC('BC1?'), kBC_8, kSmallestMipBC, kBS_565, 3, 1, kSFD_UNK, traits::is_bc },
	{ "bc1_unorm", oFCC('BC1u'), kBC_8, kSmallestMipBC, kBS_565, 3, 1, kSFD_UNK, traits::is_bc|traits::is_unorm },
	{ "bc1_unorm_srgb", oFCC('BC1s'), kBC_8, kSmallestMipBC, kBS_565, 3, 1, kSFD_UNK, traits::is_bc|traits::is_unorm },
	{ "bc2_typeless", oFCC('BC2?'), kBC_16, kSmallestMipBC, {5,6,5,4}, 4, 1, kSFD_UNK, traits::is_bc|traits::has_alpha },
	{ "bc2_unorm", oFCC('BC2u'), kBC_16, kSmallestMipBC, {5,6,5,4}, 4, 1, kSFD_UNK, traits::is_bc|traits::is_unorm|traits::has_alpha },
	{ "bc2_unorm_srgb", oFCC('BC2s'), kBC_16, kSmallestMipBC, {5,6,5,4}, 4, 1, kSFD_UNK, traits::is_bc|traits::is_unorm|traits::has_alpha },
	{ "bc3_typeless", oFCC('BC3?'), kBC_16, kSmallestMipBC, {5,6,5,8}, 4, 1, kSFD_UNK, traits::is_bc|traits::has_alpha },
	{ "bc3_unorm", oFCC('BC3u'), kBC_16, kSmallestMipBC, {5,6,5,8}, 4, 1, kSFD_UNK, traits::is_bc|traits::is_unorm|traits::has_alpha },
	{ "bc3_unorm_srgb", oFCC('BC3s'), kBC_16, kSmallestMipBC, {5,6,5,8}, 4, 1, kSFD_UNK, traits::is_bc|traits::is_unorm|traits::has_alpha },
	{ "bc4_typeless", oFCC('BC4?'), kBC_8, kSmallestMipBC, kBS_1_8, 1, 1, kSFD_UNK, traits::is_bc },
	{ "bc4_unorm", oFCC('BC4u'), kBC_8, kSmallestMipBC, kBS_1_8, 1, 1, kSFD_UNK, traits::is_bc|traits::is_unorm },
	{ "bc4_snorm", oFCC('BC4s'), kBC_8, kSmallestMipBC, kBS_1_8, 1, 1, kSFD_UNK, traits::is_bc },
	{ "bc5_typeless", oFCC('BC5?'), kBC_16, kSmallestMipBC, kBS_2_8, 2, 1, kSFD_UNK, traits::is_bc },
	{ "bc5_unorm", oFCC('BC5u'), kBC_16, kSmallestMipBC, kBS_2_8, 2, 1, kSFD_UNK, traits::is_bc|traits::is_unorm },
	{ "bc5_snorm", oFCC('BC5s'), kBC_16, kSmallestMipBC, kBS_2_8, 2, 1, kSFD_UNK, traits::is_bc },
	{ "b5g6r5_unorm", kFCC_UNK, k1_16, kSmallestMip, kBS_565, 3, 1, kSFD_UNK, traits::is_unorm },
	{ "b5g5r5a1_unorm", kFCC_UNK, k1_16, kSmallestMip, {5,6,5,1}, 4, 1, kSFD_UNK, traits::is_unorm|traits::has_alpha },
	{ "b8g8r8a8_unorm", oFCC('c4u '), k1_32, kSmallestMip, kBS_4_8, 4, 1, kSFD_UNK, traits::is_unorm|traits::has_alpha },
	{ "b8g8r8x8_unorm", oFCC('c4u '), k1_32, kSmallestMip, kBS_4_8, 4, 1, kSFD_UNK, traits::is_unorm },
	{ "r10g10b10_xr_bias_a2_unorm", kFCC_UNK, k1_32, kSmallestMip, kBS_DEC3N, 4, 1, kSFD_UNK, traits::is_unorm },
	{ "b8g8r8a8_typeless", kFCC_UNK, k1_32, kSmallestMip, kBS_4_8, 4, 1, kSFD_UNK, 0 },
	{ "b8g8r8a8_unorm_srgb", kFCC_UNK, k1_32, kSmallestMip, kBS_4_8, 4, 1, kSFD_UNK, traits::is_unorm },
	{ "b8g8r8x8_typeless", kFCC_UNK, k1_32, kSmallestMip, kBS_4_8, 4, 1, kSFD_UNK, 0 },
	{ "b8g8r8x8_unorm_srgb", kFCC_UNK, k1_32, kSmallestMip, kBS_4_8, 4, 1, kSFD_UNK, traits::is_unorm },
	{ "bc6h_typeless", oFCC('BC6?'), kBC_16, kSmallestMipBC, kBS_UNK, 3, 1, kSFD_UNK, traits::is_bc },
	{ "bc6h_uf16", oFCC('BC6u'), kBC_16, kSmallestMipBC, kBS_UNK, 3, 1, kSFD_UNK, traits::is_bc },
	{ "bc6h_sf16", oFCC('BC6s'), kBC_16, kSmallestMipBC, kBS_UNK, 3, 1, kSFD_UNK, traits::is_bc },
	{ "bc7_typeless", oFCC('BC7?'), kBC_16, kSmallestMipBC, kBS_UNK, 4, 1, kSFD_UNK, traits::is_bc|traits::has_alpha },
	{ "bc7_unorm", oFCC('BC7u'), kBC_16, kSmallestMipBC, kBS_UNK, 4, 1, kSFD_UNK, traits::is_bc|traits::is_unorm|traits::has_alpha },
	{ "bc7_unorm_srgb", oFCC('BC7s'), kBC_16, kSmallestMipBC, kBS_UNK, 4, 1, kSFD_UNK, traits::is_bc|traits::is_unorm|traits::has_alpha },
	{ "ayuv", oFCC('AYUV'), k4_32, kSmallestMip, kBS_4_4, 4, 1, kSFD_UNK, traits::is_unorm|traits::has_alpha|traits::is_yuv },
	{ "y410", oFCC('Y410'), k1_32, kSmallestMip, kBS_DEC3N, 4, 1, {r10g10b10a2_unorm,unknown}, traits::is_unorm|traits::has_alpha|traits::is_yuv },
	{ "y416", oFCC('Y416'), k4_8 , kSmallestMip, kBS_4_16, 4, 1,{b8g8r8a8_unorm,unknown}, traits::is_unorm|traits::has_alpha|traits::is_yuv },
	{ "nv12", oFCC('NV12'), kYUV, kSmallestMipYUV, kBS_3_8, 3, 2, kSFD_R8_RG8, traits::is_unorm|traits::is_yuv },
	{ "yuv2", oFCC('YUV2'), k4_8, kSmallestMip, kBS_4_8, 4, 1, kSFD_R8_RG8, traits::is_unorm|traits::has_alpha|traits::is_yuv },
	{ "p010", oFCC('P010'), k1_16, kSmallestMip, {10,10,10,0}, 3, 2, kSFD_R16_RG16, traits::is_unorm|traits::is_planar|traits::is_yuv },
	{ "p016", oFCC('P016'), k1_16, kSmallestMip, kBS_3_16, 3, 2, kSFD_R16_RG16, traits::is_unorm|traits::is_planar|traits::is_yuv },
	{ "420_opaque", oFCC('420O'), k1_8, kSmallestMipYUV, kBS_3_8, 3, 2, kSFD_R8_RG8, traits::is_unorm|traits::is_planar|traits::is_yuv },
	{ "y210", oFCC('Y210'), k4_16, kSmallestMipYUV, kBS_4_16, 3, 1, kSFD_UNK, traits::is_unorm|traits::is_yuv },
	{ "y216", oFCC('Y216'), k4_16, kSmallestMipYUV, kBS_4_16, 3, 1, kSFD_UNK, traits::is_unorm|	traits::is_yuv },
	{ "nv11", kFCC_UNK, kYUV, kSmallestMipYUV, kBS_3_8, 3, 2, kSFD_R8_RG8, traits::is_unorm|traits::is_planar|traits::is_yuv },
	{ "ia44", oFCC('IA44'), k2_4, kSmallestMip, {4,0,0,4}, 2, 1, kSFD_UNK, traits::has_alpha|traits::paletted }, // index-alpha
	{ "ai44", oFCC('AI44'), k2_4, kSmallestMip, {4,0,0,4}, 2, 1, kSFD_UNK, traits::has_alpha|traits::paletted }, // alpha-index
	{ "p8", oFCC('P8  '), k1_8, kSmallestMip, kBS_UNK, 1, 1, kSFD_UNK, traits::paletted },
	{ "a8p8", oFCC('A8P8'), k2_8, kSmallestMip, kBS_UNK, 2, 1, kSFD_UNK, traits::has_alpha|traits::paletted },
	{ "b4g4r4a4_unorm", oFCC('n4u '), k1_16, kSmallestMip, kBS_4_4, 4, 1, kSFD_UNK, traits::is_unorm|traits::has_alpha },
	{ "r8g8b8_unorm", oFCC('c3u '), k3_8, kSmallestMip, kBS_3_8, 3, 1, kSFD_UNK, traits::is_unorm },
	{ "b8g8r8_unorm", oFCC('c3u '), k3_8, kSmallestMip, kBS_3_8, 3, 1, kSFD_UNK, traits::is_unorm },
	{ "y8_u8_v8_unorm", oFCC('yuv8'), k1_8, kSmallestMipYUV, kBS_3_8, 3, 3, kSFD_R8_3, traits::is_unorm|traits::is_planar|traits::is_yuv|traits::subsurface1_bias1|traits::subsurface2_bias1 },
	{ "y8_a8_u8_v8_unorm", oFCC('auv8'), k1_8, kSmallestMipYUV, kBS_3_8, 3, 4, kSFD_R8_4, traits::is_unorm|traits::has_alpha|traits::is_planar|traits::is_yuv|traits::subsurface2_bias1|traits::subsurface3_bias1 },
	{ "ybc4_ubc4_vbc4_unorm", oFCC('yuvb'), kBC_8, kSmallestMipYUV, kBS_4_8, 3, 3, kSFD_BC4_3, traits::is_bc|traits::is_unorm|traits::is_planar|traits::is_yuv|traits::subsurface1_bias1|traits::subsurface2_bias1 },
	{ "ybc4_abc4_ubc4_vbc4_unorm", oFCC('auvb'), kBC_8, kSmallestMipYUV, kBS_4_8, 3, 4, kSFD_BC4_4, traits::is_bc|traits::is_unorm|traits::has_alpha|traits::is_planar|traits::is_yuv|traits::subsurface2_bias1|traits::subsurface3_bias1 },
	{ "y8_u8v8_unorm", oFCC('yv8u'), kYUV , kSmallestMipYUV, kBS_3_8, 3, 2, kSFD_R8_RG8, traits::is_unorm|traits::is_planar|traits::is_yuv|traits::subsurface1_bias1 },
	{ "y8a8_u8v8_unorm", oFCC('av8u'), kYAUV, kSmallestMipYUV, kBS_4_8, 4, 2, kSFD_RG8_RG8, traits::is_unorm|traits::has_alpha|traits::is_planar|traits::is_yuv|traits::subsurface1_bias1 },
	{ "ybc4_uvbc5_unorm", oFCC('yvbu'), kYUV_BC, kSmallestMipYUV, kBS_3_8, 3, 2, kSFD_BC4_BC5, traits::is_bc|traits::is_unorm|traits::is_planar|traits::is_yuv|traits::subsurface1_bias1 },
	{ "yabc5_uvbc5_unorM", oFCC('avbu'), kYAUV_BC, kSmallestMipYUV, kBS_4_8, 4, 2, kSFD_BC5_BC5, traits::is_bc|traits::is_unorm|traits::has_alpha|traits::is_planar|traits::is_yuv|traits::subsurface1_bias1 },
};
static_assert(oCOUNTOF(sFormatInfo) == format_count, "");

	} // namespace surface

const char* as_string(const surface::format& _Format)
{
	return (_Format < surface::format_count) ? surface::sFormatInfo[_Format].string : "unknown";
}

char* to_string(char* _StrDestination, size_t _SizeofStrDestination, const surface::format& _Format)
{
	return strlcpy(_StrDestination, as_string(_Format), _SizeofStrDestination) < _SizeofStrDestination ? _StrDestination : nullptr;
}

bool from_string(surface::format* _pFormat, const char* _StrSource)
{
	*_pFormat = surface::unknown;
	oFORI(i, surface::sFormatInfo)
	{
		if (!_stricmp(_StrSource, surface::sFormatInfo[i].string))
		{
			*_pFormat = surface::format(i);
			return true;
		}
	}
	return false;
}

	namespace surface {

static bool has_trait(format _Format, unsigned int _Trait)
{
	return ((_Format) < format_count) ? !!(sFormatInfo[_Format].traits & (_Trait)) : false;
}

bool is_block_compressed(format _Format)
{
	return has_trait(_Format, traits::is_bc);
}

bool is_depth(format _Format)
{
	return has_trait(_Format, traits::is_depth);
}

bool has_alpha(format _Format)
{
	return has_trait(_Format, traits::has_alpha);
}

bool is_unorm(format _Format)
{
	return has_trait(_Format, traits::is_unorm);
}

bool is_planar(format _Format)
{
	return has_trait(_Format, traits::is_planar);
}

bool is_yuv(format _Format)
{
	return has_trait(_Format, traits::is_yuv);
}

int num_channels(format _Format)
{
	return (_Format < format_count) ? sFormatInfo[_Format].num_channels : 0;
}

int num_subformats(format _Format)
{
	return (_Format < format_count) ? sFormatInfo[_Format].num_subformats: 0;
}

int subsample_bias(format _Format, int _SubsurfaceIndex)
{
	if (_Format < format_count)
	{
		if (_SubsurfaceIndex == 1 && sFormatInfo[_Format].traits & traits::subsurface1_bias1) return 1;
		if (_SubsurfaceIndex == 2 && sFormatInfo[_Format].traits & traits::subsurface2_bias1) return 1;
		if (_SubsurfaceIndex == 3 && sFormatInfo[_Format].traits & traits::subsurface3_bias1) return 1;
	}

	return 0;
}

int element_size(format _Format, int _SubsurfaceIndex)
{
	return (_Format < format_count) ? (_SubsurfaceIndex ? element_size(sFormatInfo[_Format].subformats.format[_SubsurfaceIndex]) : sFormatInfo[_Format].size) : 0;
}

int2 min_dimensions(format _Format)
{
	return (_Format < format_count) ? sFormatInfo[_Format].min_dimensions : int2(0, 0);
}

int bits(format _Format)
{
	if (_Format == r1_unorm) return 1;
	return 8 * element_size(_Format);
}

void channel_bits(format _Format, int* _pNBitsR, int* _pNBitsG, int* _pNBitsB, int* _pNBitsA)
{
	if (_Format < format_count)
	{
		const struct bit_size& b = sFormatInfo[_Format].bit_size;
		*_pNBitsR = b.r; *_pNBitsG = b.g; *_pNBitsB = b.b; *_pNBitsA = b.a;
	}

	else
		*_pNBitsR = *_pNBitsG = *_pNBitsB = *_pNBitsA = 0;
}

format subformat(format _Format, int _SubsurfaceIndex)
{
	if (_SubsurfaceIndex < 0)
		throw std::invalid_argument("SubsurfaceIndex can't be negative");

	if (sFormatInfo[_Format].num_subformats < _SubsurfaceIndex)
		return unknown;

	if (!!(sFormatInfo[_Format].traits & traits::is_yuv))
		return sFormatInfo[_Format].subformats.format[_SubsurfaceIndex];

	return _Format;
}

fourcc to_fourcc(format _Format)
{
	return (_Format < format_count) ? sFormatInfo[_Format].fourcc : fourcc(0);
}

format from_fourcc(fourcc _FourCC)
{
	oFORI(i, sFormatInfo)
	{
		if (_FourCC == sFormatInfo[i].fourcc)
			return format(i);
	}
	return unknown;
}

format closest_nv12(format _Format)
{
	if (num_subformats(_Format) == 2) // already nv12
		return _Format;
	if (num_subformats(_Format) == 3) // no alpha
		return (is_block_compressed(_Format)) ? ybc4_uvbc5_unorm : y8_u8v8_unorm;
	if (is_block_compressed(_Format)) // alpha
		return yabc5_uvbc5_unorm;
	return y8a8_u8v8_unorm;
}

int num_mips(bool _HasMips, const int3& _Mip0Dimensions)
{
	// Rules of mips are to go to 1x1... so a 1024x8 texture has many more than 4
	// mips.

	if (_Mip0Dimensions.x <= 0 || _Mip0Dimensions.y <= 0 || _Mip0Dimensions.z <= 0)
		return 0;

	int nMips = 1;
	int3 mip = _Mip0Dimensions;

	while (_HasMips && any(mip != int3(1,1,1)))
	{
		nMips++;
		mip = max(int3(1,1,1), mip / int3(2,2,2));
	}

	return nMips;
}

int dimension(format _Format, int _Mip0Dimension, int _MipLevel, int _SubsurfaceIndex)
{
	oCHECK_DIM(_Format, _Mip0Dimension);
	if (_Format == unknown)
		throw std::invalid_argument("Unknown surface format passed to surface::dimension");
	const int subsampleBias = subsample_bias(_Format, _SubsurfaceIndex);
	int d = max(1, _Mip0Dimension >> (_MipLevel + subsampleBias));
	return is_block_compressed(_Format) ? static_cast<int>(byte_align(d, 4)) : d;
}

int2 dimensions(format _Format, const int2& _Mip0Dimensions, int _MipLevel, int _SubsurfaceIndex)
{
	return int2(dimension(_Format, _Mip0Dimensions.x, _MipLevel, _SubsurfaceIndex)
		, dimension(_Format, _Mip0Dimensions.y, _MipLevel, _SubsurfaceIndex));
}

int3 dimensions(format _Format, const int3& _Mip0Dimensions, int _MipLevel, int _SubsurfaceIndex)
{
	return int3(dimension(_Format, _Mip0Dimensions.x, _MipLevel, _SubsurfaceIndex)
		, dimension(_Format, _Mip0Dimensions.y, _MipLevel, _SubsurfaceIndex)
		, dimension(r32_uint, _Mip0Dimensions.z, _MipLevel, _SubsurfaceIndex)); // No block-compression alignment for depth
}

int dimension_npot(format _Format, int _Mip0Dimension, int _MipLevel, int _SubsurfaceIndex)
{
	oCHECK_DIM(_Format, _Mip0Dimension);

	format NthSurfaceFormat = subformat(_Format, _SubsurfaceIndex);
	const int MipLevelBias = subsample_bias(_Format, _SubsurfaceIndex);
	
	// @tony: This was added while merging oYUVSurface functionality into 
	// oSurface, so we recognize that the int2 values may not be the same and 
	// won't be for formats like YUV9, but first get the lion's share of the code
	// across and revisit this once the main algo is vetted.
	const int2 MinDimension = min_dimensions(_Format);

	int d = max(1, _Mip0Dimension >> (_MipLevel + MipLevelBias));
	int NPOTDim = is_block_compressed(NthSurfaceFormat) ? static_cast<int>(byte_align(d, 4)) : d;

	if (_SubsurfaceIndex == 0 && subsample_bias(_Format, 1) != 0)
		NPOTDim = max(MinDimension.x, NPOTDim & ~(MinDimension.x-1)); // always even down to 2x2

	return NPOTDim;
}

int2 dimensions_npot(format _Format, const int2& _Mip0Dimensions, int _MipLevel, int _SubsurfaceIndex)
{
	#ifdef _DEBUG
		const int2 MinDimension = min_dimensions(_Format);
		oASSERT(MinDimension.x == MinDimension.y, "There is currently no support for aniso min dimensions");
	#endif

	return int2(
		dimension_npot(_Format, _Mip0Dimensions.x, _MipLevel, _SubsurfaceIndex)
		, dimension_npot(_Format, _Mip0Dimensions.y, _MipLevel, _SubsurfaceIndex));
}

int3 dimensions_npot(format _Format, const int3& _Mip0Dimensions, int _MipLevel, int _SubsurfaceIndex)
{
	#ifdef _DEBUG
		const int2 MinDimension = min_dimensions(_Format);
		oASSERT(MinDimension.x == MinDimension.y, "There is currently no support for aniso min dimensions");
	#endif

	return int3(
		dimension_npot(_Format, _Mip0Dimensions.x, _MipLevel, _SubsurfaceIndex)
		, dimension_npot(_Format, _Mip0Dimensions.y, _MipLevel, _SubsurfaceIndex)
		, dimension_npot(r32_uint, _Mip0Dimensions.z, _MipLevel, _SubsurfaceIndex)); // No block-compression alignment for depth
}

int row_size(format _Format, int _MipWidth, int _SubsurfaceIndex)
{
	oCHECK_DIM(_Format, _MipWidth);
	oASSERT(_Format != unknown, "Unknown surface format passed to GetRowPitch");
	int w = dimension(_Format, _MipWidth, 0, _SubsurfaceIndex);
	if (is_block_compressed(_Format)) // because the atom is a 4x4 block
		w /= 4;
	const int s = element_size(_Format, _SubsurfaceIndex);
	return byte_align(w * s, s);
}

int row_pitch(const info& _SurfaceInfo, int _MipLevel, int _SubsurfaceIndex)
{
	oCHECK_INFO(_SurfaceInfo)
	const int numMips = num_mips(_SurfaceInfo.layout, _SurfaceInfo.dimensions);
	if (_MipLevel >= numMips)
		throw std::invalid_argument("invalid _MipLevel");

	switch (_SurfaceInfo.layout)
	{
		case image: 
			return row_size(_SurfaceInfo.format, _SurfaceInfo.dimensions, _SubsurfaceIndex);
		case tight: 
			return row_size(_SurfaceInfo.format, dimension_npot(_SurfaceInfo.format, _SurfaceInfo.dimensions.x, _MipLevel, _SubsurfaceIndex), _SubsurfaceIndex);
		case below: 
		{
			const int mip0RowSize = row_size(_SurfaceInfo.format, _SurfaceInfo.dimensions.x, _SubsurfaceIndex);
			if (numMips > 2)
			{
					return max(mip0RowSize, 
					row_size(_SurfaceInfo.format, dimension_npot(_SurfaceInfo.format, _SurfaceInfo.dimensions.x, 1, _SubsurfaceIndex)) + 
					row_size(_SurfaceInfo.format, dimension_npot(_SurfaceInfo.format, _SurfaceInfo.dimensions.x, 2, _SubsurfaceIndex)) );
			}
			else
				return mip0RowSize;
		}

		case right: 
		{
			const int mip0RowSize = row_size(_SurfaceInfo.format, _SurfaceInfo.dimensions.x, _SubsurfaceIndex);
			if (numMips > 1)
				return mip0RowSize + row_size(_SurfaceInfo.format, dimension_npot(_SurfaceInfo.format, _SurfaceInfo.dimensions.x, 1, _SubsurfaceIndex), _SubsurfaceIndex);
			else
				return mip0RowSize;
		}

		oNODEFAULT;
	}
}

int depth_pitch(const info& _SurfaceInfo, int _MipLevel, int _SubsurfaceIndex)
{
	oCHECK_INFO(_SurfaceInfo)
	int3 mipDimensions = dimensions_npot(_SurfaceInfo.format, _SurfaceInfo.dimensions, _MipLevel, 0);
	return row_pitch(_SurfaceInfo, _MipLevel, _SubsurfaceIndex) * num_rows(_SurfaceInfo.format, mipDimensions.xy(), _SubsurfaceIndex);
}

int num_columns(format _Format, int _MipWidth, int _SubsurfaceIndex)
{
	oCHECK_DIM(_Format, _MipWidth);
	int widthInPixels = dimension(_Format, _MipWidth, _SubsurfaceIndex);
	return is_block_compressed(_Format) ? __max(1, widthInPixels/4) : widthInPixels;
}

int num_rows(format _Format, int _MipHeight, int _SubsurfaceIndex)
{
	oCHECK_DIM(_Format, _MipHeight);
	int heightInPixels = dimension_npot(_Format, _MipHeight, 0, _SubsurfaceIndex);
	return is_block_compressed(_Format) ? __max(1, heightInPixels/4) : heightInPixels;
}

int mip_size(format _Format, const int2& _MipDimensions, int _SubsurfaceIndex)
{
	oCHECK_DIM2(_Format, _MipDimensions);
	return row_size(_Format, _MipDimensions, _SubsurfaceIndex) * num_rows(_Format, _MipDimensions, _SubsurfaceIndex);
}

static int offset_image(const info& _SurfaceInfo, int _MipLevel, int _SubsurfaceIndex)
{
	oASSERT(_MipLevel == 0, "layout::image doesn't have mip levels");
	int offset = 0;
	for (int i = 0; i < _SubsurfaceIndex; i++)
		offset += byte_align(total_size(_SurfaceInfo, i), oDEFAULT_MEMORY_ALIGNMENT);
	return offset;
}

static int offset_tight(const info& _SurfaceInfo, int _MipLevel, int _SubsurfaceIndex)
{
	oCHECK_INFO(_SurfaceInfo)
	int3 mip0dimensions = _SurfaceInfo.dimensions;
	int offset = 0;
	int mip = 0;
	while (mip != _MipLevel)
	{
		int3 previousMipDimensions = dimensions_npot(_SurfaceInfo.format, mip0dimensions, mip, _SubsurfaceIndex);
		offset += mip_size(_SurfaceInfo.format, previousMipDimensions, _SubsurfaceIndex);
		mip++;
	}
	return offset;
}

static int offset_below(const info& _SurfaceInfo, int _MipLevel, int _SubsurfaceIndex)
{
	oCHECK_INFO(_SurfaceInfo)
	if (0 == _MipLevel)
		return 0;

	int3 mip0dimensions = _SurfaceInfo.dimensions;
	int3 mip1dimensions = dimensions_npot(_SurfaceInfo.format, _SurfaceInfo.dimensions, 1, _SubsurfaceIndex);
	int surfaceRowPitch = row_pitch(_SurfaceInfo, 0, _SubsurfaceIndex);

	// Step down when moving from Mip0 to Mip1
	int offset = surfaceRowPitch * num_rows(_SurfaceInfo.format, mip0dimensions, _SubsurfaceIndex);
	if (1 == _MipLevel)
		return offset;

	// Step right when moving from Mip1 to Mip2
	offset += row_size(_SurfaceInfo.format, mip1dimensions, _SubsurfaceIndex);

	// Step down for all of the other MIPs
	int mip = 2;
	while (mip != _MipLevel)
	{
		int3 previousMipDimensions = dimensions_npot(_SurfaceInfo.format, mip0dimensions, mip, _SubsurfaceIndex);
		offset += surfaceRowPitch * num_rows(_SurfaceInfo.format, previousMipDimensions, _SubsurfaceIndex);
		mip++;
	}		

	return offset;
}

static int offset_right(const info& _SurfaceInfo, int _MipLevel, int _SubsurfaceIndex)
{
	oCHECK_INFO(_SurfaceInfo)
	if (0 == _MipLevel)
		return 0;

	int3 mip0dimensions = _SurfaceInfo.dimensions;
	int surfaceRowPitch = row_pitch(_SurfaceInfo, 0, _SubsurfaceIndex);

	// Step right when moving from Mip0 to Mip1
	int offset = row_size(_SurfaceInfo.format, mip0dimensions, _SubsurfaceIndex);

	// Step down for all of the other MIPs
	int mip = 1;
	while (mip != _MipLevel)
	{
		int3 previousMipDimensions = dimensions_npot(_SurfaceInfo.format, mip0dimensions, mip, _SubsurfaceIndex);
		offset += surfaceRowPitch * num_rows(_SurfaceInfo.format, previousMipDimensions, _SubsurfaceIndex);
		mip++;
	}		

	return offset;
}

int offset(const info& _SurfaceInfo, int _MipLevel, int _SubsurfaceIndex)
{
	oCHECK_INFO(_SurfaceInfo)
	const int numMips = num_mips(_SurfaceInfo.layout, _SurfaceInfo.dimensions);
	if (_MipLevel >= numMips) 
		throw std::invalid_argument("invalid _MipLevel");

	switch (_SurfaceInfo.layout)
	{
		case image: return offset_image(_SurfaceInfo, _MipLevel, _SubsurfaceIndex);
		case tight: return offset_tight(_SurfaceInfo, _MipLevel, _SubsurfaceIndex);
		case below: return offset_below(_SurfaceInfo, _MipLevel, _SubsurfaceIndex);
		case right: return offset_right(_SurfaceInfo, _MipLevel, _SubsurfaceIndex);
		oNODEFAULT;
	}
}

int slice_pitch(const info& _SurfaceInfo, int _SubsurfaceIndex)
{
	oCHECK_INFO(_SurfaceInfo)
	int pitch = 0;

	switch (_SurfaceInfo.layout)
	{
		case image: case right: case below:
			return mip_size(_SurfaceInfo.format, slice_dimensions(_SurfaceInfo, 0), _SubsurfaceIndex);

		case tight:
		{
			// Sum the size of all mip levels
			int3 dimensions = _SurfaceInfo.dimensions;
			int nMips = num_mips(_SurfaceInfo.layout, dimensions);
			while (nMips > 0)
			{
				pitch += mip_size(_SurfaceInfo.format, dimensions.xy(), _SubsurfaceIndex) * dimensions.z;
				dimensions = max(int3(1,1,1), dimensions / int3(2,2,2));
				nMips--;
			}

			// Align slicePitch to mip0RowPitch
			const int mip0RowPitch = row_pitch(_SurfaceInfo, 0, _SubsurfaceIndex);
			pitch = (((pitch + (mip0RowPitch - 1)) / mip0RowPitch) * mip0RowPitch);
			break;
		}
		oNODEFAULT;
	}

	return pitch;
}

int total_size(const info& _SurfaceInfo, int _SubsurfaceIndex)
{
	if (_SubsurfaceIndex < 0)
	{
		int size = 0;
		const int nSurfaces = num_subformats(_SurfaceInfo.format);
		for (int i = 0; i < nSurfaces; i++)
		{
			// byte_align is needed here to avoid a memory corruption crash. I'm not 
			// sure why it is needed, but I think that size is a memory structure 
			// containing all surface sizes, so they are all expected to be aligned.
			size += byte_align(total_size(_SurfaceInfo, i), oDEFAULT_MEMORY_ALIGNMENT);
		}
		return size;
	}
	
	return slice_pitch(_SurfaceInfo, _SubsurfaceIndex) * max(_SurfaceInfo.array_size, 1);
}

int2 dimensions(const info& _SurfaceInfo, int _SubsurfaceIndex)
{
	int2 sliceDimensions = slice_dimensions(_SurfaceInfo, _SubsurfaceIndex);
	return int2(sliceDimensions.x, sliceDimensions.y * max(1, _SurfaceInfo.array_size));
}

int2 slice_dimensions(const info& _SurfaceInfo, int _SubsurfaceIndex)
{
	oCHECK_INFO(_SurfaceInfo)
	int3 mip0dimensions = dimensions_npot(_SurfaceInfo.format, _SurfaceInfo.dimensions, 0, _SubsurfaceIndex);
	switch (_SurfaceInfo.layout)
	{
		case image: 
			return int2(mip0dimensions.x, (mip0dimensions.y * mip0dimensions.z));
		
		case tight: 
		{
			const int surfaceSlicePitch = slice_pitch(_SurfaceInfo, _SubsurfaceIndex);
			const int mip0RowPitch = row_pitch(_SurfaceInfo, 0, _SubsurfaceIndex);
			return int2(mip0dimensions.x, (surfaceSlicePitch / mip0RowPitch));
		}
		case below: 
		{
			int numMips = num_mips(_SurfaceInfo.layout, mip0dimensions);
			int3 mip1dimensions = numMips > 1 ? dimensions_npot(_SurfaceInfo.format, mip0dimensions, 1, _SubsurfaceIndex) : int3(0);
			int3 mip2dimensions = numMips > 2 ? dimensions_npot(_SurfaceInfo.format, mip0dimensions, 2, _SubsurfaceIndex) : int3(0);

			int mip0height = mip0dimensions.y * mip0dimensions.z;
			int mip1height = mip1dimensions.y * mip1dimensions.z;
			int mip2andUpHeight = mip2dimensions.y * mip2dimensions.z;
			for (int mip=3; mip<numMips; ++mip)
			{
				int3 mipNdimensions = dimensions_npot(_SurfaceInfo.format, mip0dimensions, mip, _SubsurfaceIndex);
				mip2andUpHeight += mipNdimensions.y * mipNdimensions.z;
			}
			return int2(max(mip0dimensions.x, mip1dimensions.x + mip2dimensions.x), (mip0height + max(mip1height, mip2andUpHeight)));
		}
		case right: 
		{
			int numMips = num_mips(_SurfaceInfo.layout, mip0dimensions);
			int3 mip1dimensions = numMips > 1 ? dimensions_npot(_SurfaceInfo.format, mip0dimensions, 1, _SubsurfaceIndex) : int3(0);

			int mip0height = mip0dimensions.y * mip0dimensions.z;
			int mip1andUpHeight = mip1dimensions.y * mip1dimensions.z;
			for (int mip=2; mip<numMips; ++mip)
			{
				int3 mipNdimensions = dimensions_npot(_SurfaceInfo.format, mip0dimensions, mip, _SubsurfaceIndex);
				mip1andUpHeight += mipNdimensions.y * mipNdimensions.z;
			}
			return int2(mip0dimensions.x + mip1dimensions.x, max(mip0height, mip1andUpHeight));
		}
		oNODEFAULT;
	}
}

subresource_info subresource(const info& _SurfaceInfo, int _Subresource)
{
	oCHECK_INFO(_SurfaceInfo)
	subresource_info inf;
	int numMips = num_mips(_SurfaceInfo.layout, _SurfaceInfo.dimensions);
	unpack_subresource(_Subresource, numMips, _SurfaceInfo.array_size, &inf.mip_level, &inf.array_slice, &inf.subsurface);
	if (_SurfaceInfo.array_size && max(1, inf.array_slice) >= max(1, _SurfaceInfo.array_size)) throw std::invalid_argument("array slice index is out of range");
	if (inf.subsurface >= num_subformats(_SurfaceInfo.format)) throw std::invalid_argument("subsurface index is out of range for the specified surface");
	inf.dimensions = dimensions_npot(_SurfaceInfo.format, _SurfaceInfo.dimensions, inf.mip_level, inf.subsurface);
	inf.format = _SurfaceInfo.format;
	return inf;
}

info subsurface(const info& _SurfaceInfo, int _SubsurfaceIndex, int _MipLevel, int2* _pByteDimensions)
{
	info inf;
	inf.dimensions = dimensions_npot(_SurfaceInfo.format, _SurfaceInfo.dimensions, _MipLevel, _SubsurfaceIndex);
	inf.array_size = _SurfaceInfo.array_size;
	inf.format = subformat(_SurfaceInfo.format, _SubsurfaceIndex);
	inf.layout = _SurfaceInfo.layout;
	if (_pByteDimensions)
	{
		_pByteDimensions->x = row_size(inf.format, inf.dimensions);
		_pByteDimensions->y = num_rows(inf.format, inf.dimensions);
	}
	return inf;
}

int subresource_size(const subresource_info& _SubresourceInfo)
{
	return mip_size(_SubresourceInfo.format, _SubresourceInfo.dimensions, _SubresourceInfo.subsurface);
}

int subresource_offset(const info& _SurfaceInfo, int _Subresource, int _DepthIndex)
{
	oASSERT(_DepthIndex < _SurfaceInfo.dimensions.z, "Depth index is out of range");
	subresource_info inf = subresource(_SurfaceInfo, _Subresource);
	int off = offset(_SurfaceInfo, inf.mip_level, inf.subsurface);
	if (_DepthIndex)
		off += depth_pitch(_SurfaceInfo, inf.mip_level, inf.subsurface) * _DepthIndex;
	else if (inf.array_slice > 0)
		off += slice_pitch(_SurfaceInfo, inf.subsurface) * inf.array_slice;
	return off;
}

const_mapped_subresource get_const_mapped_subresource(const info& _SurfaceInfo, int _Subresource, int _DepthIndex, const void* _pSurface, int2* _pByteDimensions)
{
	subresource_info inf = subresource(_SurfaceInfo, _Subresource);

	const_mapped_subresource msr;
	msr.row_pitch = row_pitch(_SurfaceInfo, inf.mip_level, inf.subsurface);
	msr.depth_pitch = depth_pitch(_SurfaceInfo, inf.mip_level, inf.subsurface);
	msr.data = byte_add(_pSurface, subresource_offset(_SurfaceInfo, _Subresource, _DepthIndex));

	if (_pByteDimensions)
		*_pByteDimensions = byte_dimensions(_SurfaceInfo.format, inf.dimensions.xy(), inf.subsurface);
	return msr;
}

mapped_subresource get_mapped_subresource(const info& _SurfaceInfo, int _Subresource, int _DepthIndex, void* _pSurface, int2* _pByteDimensions)
{
	const_mapped_subresource msr = get_const_mapped_subresource(_SurfaceInfo, _Subresource, _DepthIndex, _pSurface, _pByteDimensions);
	return (mapped_subresource&)msr;
}

void update(const info& _SurfaceInfo, int _Subresource, int _DepthIndex, void* _pDestinationSurface, const void* _pSource, size_t _SourceRowPitch, bool _FlipVertical)
{
	int2 ByteDimensions;
	mapped_subresource msr = get_mapped_subresource(_SurfaceInfo, _Subresource, _DepthIndex, _pDestinationSurface, &ByteDimensions);
	memcpy2d(msr.data, msr.row_pitch, _pSource, _SourceRowPitch, ByteDimensions.x, ByteDimensions.y, _FlipVertical);
}

void copy(const info& _SurfaceInfo, int _Subresource, int _DepthIndex, const void* _pSourceSurface, void* _pDestination, size_t _DestinationRowPitch, bool _FlipVertical)
{
	int2 ByteDimensions;
	const_mapped_subresource msr = get_const_mapped_subresource(_SurfaceInfo, _Subresource, _DepthIndex, _pSourceSurface, &ByteDimensions);
	memcpy2d(_pDestination, _DestinationRowPitch, msr.data, msr.row_pitch, ByteDimensions.x, ByteDimensions.y, _FlipVertical);
}

void copy(const info& _SurfaceInfo, const const_mapped_subresource& _Source, mapped_subresource* _Destination, bool _FlipVertical)
{
	memcpy2d(_Destination->data, _Destination->row_pitch, _Source.data, _Source.row_pitch, _SurfaceInfo.dimensions.x*element_size(_SurfaceInfo.format), _SurfaceInfo.dimensions.y, _FlipVertical);
}

void copy(const subresource_info& _SubresourceInfo, const const_mapped_subresource& _Source, mapped_subresource* _Destination, bool _FlipVertical)
{
	memcpy2d(_Destination->data, _Destination->row_pitch, _Source.data, _Source.row_pitch, _SubresourceInfo.dimensions.x*element_size(_SubresourceInfo.format), _SubresourceInfo.dimensions.y, _FlipVertical);
}

void put(const subresource_info& _SubresourceInfo, mapped_subresource* _Destination, const int2& _Coordinate, color _Color)
{
	const int elSize = element_size(format(_SubresourceInfo.format));
	unsigned char* p = (unsigned char*)_Destination->data + (_Coordinate.y * _Destination->row_pitch) + (_Coordinate.x * elSize);
	int rr, gg, bb, aa;
	_Color.decompose(&rr, &gg, &bb, &aa);
	unsigned char r = (unsigned char)rr, g = (unsigned char)gg, b = (unsigned char)bb, a = (unsigned char)aa;
	switch (_SubresourceInfo.format)
	{
		case r8g8b8a8_unorm: *p++ = r; *p++ = g; *p++ = b; *p++ = a; break;
		case r8g8b8_unorm: *p++ = r; *p++ = g; *p++ = b; break;
		case b8g8r8a8_unorm: *p++ = b; *p++ = g; *p++ = r; *p++ = a; break;
		case b8g8r8_unorm: *p++ = b; *p++ = g; *p++ = r; break;
		case r8_unorm: *p++ = r; break;
		default: throw std::invalid_argument("unsupported format");
	}
}

color get(const subresource_info& _SubresourceInfo, const const_mapped_subresource& _Source, const int2& _Coordinate)
{
	const int elSize = element_size(_SubresourceInfo.format);
	const unsigned char* p = (const unsigned char*)_Source.data + (_Coordinate.y * _Source.row_pitch) + (_Coordinate.x * elSize);
	int r=0, g=0, b=0, a=255;
	switch (_SubresourceInfo.format)
	{
		case r8g8b8a8_unorm: r = *p++; g = *p++; b = *p++; a = *p++; break;
		case r8g8b8_unorm: r = *p++; g = *p++; b = *p++; break;
		case b8g8r8a8_unorm: b = *p++; g = *p++; r = *p++; a = *p++; break;
		case b8g8r8_unorm: b = *p++; g = *p++; r = *p++; break;
		case r8_unorm: r = *p++; g=r; b=r; break;
		default: break;
	}
	
	return color(r,g,b,a);
}

// Returns the nth mip where the mip level best-fits into a tile. i.e. several
// tiles are no longer required to store all the mip data, and all mip levels
// after the one returned by this function can together fit into one mip level.
static int best_fit_mip_level(const info& _SurfaceInfo, const int2& _TileDimensions)
{
	oCHECK_INFO(_SurfaceInfo)
	if (image == _SurfaceInfo.layout)
		return 0;

	int nthMip = 0;
	int3 mip = _SurfaceInfo.dimensions;
	while (any(mip != int3(1,1,1)))
	{
		if (all(_SurfaceInfo.dimensions.xy() <= _TileDimensions))
			break;

		nthMip++;
		mip = max(int3(1,1,1), mip / int3(2,2,2));
	}

	return nthMip;
}

int num_slice_tiles(const info& _SurfaceInfo, const int2& _TileDimensions)
{
	oCHECK_NOT_PLANAR(_SurfaceInfo.format);

	if (image == _SurfaceInfo.layout)
		return num_tiles(_SurfaceInfo.dimensions, _TileDimensions);

	int numTiles = 0;
	int lastMip = 1 + best_fit_mip_level(_SurfaceInfo, _TileDimensions);
	for (int i = 0; i <= lastMip; i++)
	{
		int3 mipDim = dimensions(_SurfaceInfo.format, _SurfaceInfo.dimensions, i);
		numTiles += num_tiles(mipDim, _TileDimensions);
	}

	return numTiles;
}

int dimension_in_tiles(int _MipDimension, int _TileDimension)
{
	int div = _MipDimension / _TileDimension;
	if (0 != (_MipDimension % _TileDimension))
		div++;
	return div;
}

int2 dimensions_in_tiles(const int2& _MipDimensions, const int2& _TileDimensions)
{
	return int2(dimension_in_tiles(_MipDimensions.x, _TileDimensions.x)
		, dimension_in_tiles(_MipDimensions.y, _TileDimensions.y));
}

int3 dimensions_in_tiles(const int3& _MipDimensions, const int2& _TileDimensions)
{
	return int3(dimension_in_tiles(_MipDimensions.x, _TileDimensions.x)
		, dimension_in_tiles(_MipDimensions.y, _TileDimensions.y)
		, _MipDimensions.z);
}

int num_tiles(const int2& _MipDimensions, const int2& _TileDimensions)
{
	int2 mipDimInTiles = dimensions_in_tiles(_MipDimensions, _TileDimensions);
	return mipDimInTiles.x * mipDimInTiles.y;
}

int num_tiles(const int3& _MipDimensions, const int2& _TileDimensions)
{
	int3 mipDimInTiles = dimensions_in_tiles(_MipDimensions, _TileDimensions);
	return mipDimInTiles.x * mipDimInTiles.y;
}

// calculate the tile id of the first tile in the specified slice
static int slice_first_tile_id(const info& _SurfaceInfo, const int2& _TileDimensions, int _Slice)
{
	oCHECK_NOT_PLANAR(_SurfaceInfo.format);
	int numTilesPerSlice = num_slice_tiles(_SurfaceInfo, _TileDimensions);
	return _Slice * numTilesPerSlice;
}

// how many tiles from the startID to start the specified mip
static int tile_id_offset(const info& _SurfaceInfo, const int2& _TileDimensions, int _MipLevel)
{
	oCHECK_NOT_PLANAR(_SurfaceInfo.format);
	if (image == _SurfaceInfo.layout)
		return 0;
	int numTiles = 0;
	int numMips = __min(_MipLevel, 1 + best_fit_mip_level(_SurfaceInfo, _TileDimensions));
	for (int i = 0; i < numMips; i++)
	{
		int3 mipDim = dimensions(_SurfaceInfo.format, _SurfaceInfo.dimensions, i);
		numTiles += num_tiles(mipDim, _TileDimensions);
	}

	return numTiles;
}

static int mip_first_tile_id(const info& _SurfaceInfo, const int2& _TileDimensions, int _MipLevel, int _Slice)
{
	oCHECK_NOT_PLANAR(_SurfaceInfo.format);

	int sliceStartID = slice_first_tile_id(_SurfaceInfo, _TileDimensions, _Slice);
	int mipIDOffset = tile_id_offset(_SurfaceInfo, _TileDimensions, _MipLevel);
	return sliceStartID + mipIDOffset;
}

bool use_large_pages(const info& _SurfaceInfo, const int2& _TileDimensions, int _SmallPageSize, int _LargePageSize)
{
	oCHECK_NOT_PLANAR(_SurfaceInfo.format);
	int surfaceSize = mip_size(_SurfaceInfo.format, _SurfaceInfo.dimensions);
	if (surfaceSize < (_LargePageSize / 4))
		return false;

	int surfacePitch = row_pitch(_SurfaceInfo);

	// number of rows before we get a page miss
	float numRowsPerPage = _SmallPageSize / static_cast<float>(surfacePitch);

	int tileByteWidth = row_size(_SurfaceInfo.format, _TileDimensions);
	
	// estimate how many bytes we would work on in a tile before encountering a 
	// tlb cache miss... not precise, but should be close enough for our purpose 
	// here. 
	float numBytesPerTLBMiss = tileByteWidth * numRowsPerPage - std::numeric_limits<float>::epsilon(); 
	
	// If we are not going to get at least half a small page size of work done per tlb miss, better to use large pages instead.
	return numBytesPerTLBMiss <= (_SmallPageSize / 2);
}

int calc_tile_id(const info& _SurfaceInfo, const tile_info& _TileInfo, int2* _pPosition)
{
	oCHECK_NOT_PLANAR(_SurfaceInfo.format);
	int mipStartTileID = mip_first_tile_id(_SurfaceInfo, _TileInfo.dimensions, _TileInfo.mip_level, _TileInfo.array_slice);
	int2 PositionInTiles = _TileInfo.position / _TileInfo.dimensions;
	int2 mipDim = dimensions(_SurfaceInfo.format, _SurfaceInfo.dimensions.xy(), _TileInfo.mip_level);
	int2 mipDimInTiles = dimensions_in_tiles(mipDim, _TileInfo.dimensions);
	int tileID = mipStartTileID + (mipDimInTiles.x * PositionInTiles.y) + PositionInTiles.x;
	if (_pPosition)
		*_pPosition = PositionInTiles * _TileInfo.dimensions;
	return tileID;
}

tile_info get_tile(const info& _SurfaceInfo, const int2& _TileDimensions, int _TileID)
{
	tile_info inf;

	oCHECK_NOT_PLANAR(_SurfaceInfo.format);
	int numTilesPerSlice = num_slice_tiles(_SurfaceInfo, _TileDimensions);
	inf.dimensions = _TileDimensions;
	inf.array_slice = _TileID / numTilesPerSlice;
	if (max(1, inf.array_slice) >= max(1, _SurfaceInfo.array_size)) throw std::invalid_argument("TileID is out of range for the specified mip dimensions");

	int firstTileInMip = 0;
	int3 mipDim = _SurfaceInfo.dimensions;
	inf.mip_level = 0;
	int nthTileIntoSlice = _TileID % numTilesPerSlice; 

	if (nthTileIntoSlice > 0)
	{
		do 
		{
			mipDim = dimensions(_SurfaceInfo.format, _SurfaceInfo.dimensions, ++inf.mip_level);
			firstTileInMip += num_tiles(mipDim, _TileDimensions);

		} while (nthTileIntoSlice < firstTileInMip);
	}
	
	int tileOffsetFromMipStart = nthTileIntoSlice - firstTileInMip;
	int3 mipDimInTiles = dimensions_in_tiles(mipDim, _TileDimensions);
	int2 positionInTiles = int2(tileOffsetFromMipStart % mipDimInTiles.x, tileOffsetFromMipStart / mipDimInTiles.y);
	inf.position = positionInTiles * _TileDimensions;
	return inf;
}

void enumerate_pixels(const info& _SurfaceInfo
	, const const_mapped_subresource& _MappedSubresource
	, const std::function<void(const void* _pPixel)>& _Enumerator)
{
	const void* pRow = _MappedSubresource.data;
	const void* pEnd = byte_add(pRow, _SurfaceInfo.dimensions.y * _MappedSubresource.row_pitch); // should this be depth/slice pitch?
	const int FormatSize = element_size(_SurfaceInfo.format);
	for (; pRow < pEnd; pRow = byte_add(pRow, _MappedSubresource.row_pitch))
	{
		const void* pPixel = pRow;
		const void* pRowEnd = byte_add(pPixel, _SurfaceInfo.dimensions.x * FormatSize);
		for (; pPixel < pRowEnd; pPixel = byte_add(pPixel, FormatSize))
			_Enumerator(pPixel);
	}
}

void enumerate_pixels(const info& _SurfaceInfo
	, mapped_subresource& _MappedSubresource
	, const std::function<void(void* _pPixel)>& _Enumerator)
{
	void* pRow = _MappedSubresource.data;
	void* pEnd = byte_add(pRow, _SurfaceInfo.dimensions.y * _MappedSubresource.row_pitch); // should this be depth/slice pitch?
	const int FormatSize = element_size(_SurfaceInfo.format);
	for (; pRow < pEnd; pRow = byte_add(pRow, _MappedSubresource.row_pitch))
	{
		void* pPixel = pRow;
		void* pRowEnd = byte_add(pPixel, _SurfaceInfo.dimensions.x * FormatSize);
		for (; pPixel < pRowEnd; pPixel = byte_add(pPixel, FormatSize))
			_Enumerator(pPixel);
	}
}

// Calls the specified function on each pixel of two same-formatted images.
void enumerate_pixels(const info& _SurfaceInfo
	, const const_mapped_subresource& _MappedSubresource1
	, const const_mapped_subresource& _MappedSubresource2
	, const std::function<void(const void* oRESTRICT _pPixel1, const void* oRESTRICT _pPixel2)>& _Enumerator)
{
	const void* pRow1 = _MappedSubresource1.data;
	const void* pRow2 = _MappedSubresource2.data;
	const void* pEnd1 = byte_add(pRow1, _SurfaceInfo.dimensions.y * _MappedSubresource1.row_pitch);
	const int FormatSize = element_size(_SurfaceInfo.format);
	while (pRow1 < pEnd1)
	{
		const void* pPixel1 = pRow1;
		const void* pPixel2 = pRow2;
		const void* pRowEnd1 = byte_add(pPixel1, _SurfaceInfo.dimensions.x * FormatSize);
		while (pPixel1 < pRowEnd1)
		{
			_Enumerator(pPixel1, pPixel2);
			pPixel1 = byte_add(pPixel1, FormatSize);
			pPixel2 = byte_add(pPixel2, FormatSize);
		}

		pRow1 = byte_add(pRow1, _MappedSubresource1.row_pitch);
		pRow2 = byte_add(pRow2, _MappedSubresource2.row_pitch);
	}
}

void enumerate_pixels(const info& _SurfaceInfoInput
	, const const_mapped_subresource& _MappedSubresourceInput1
	, const const_mapped_subresource& _MappedSubresourceInput2
	, const info& _SurfaceInfoOutput
	, mapped_subresource& _MappedSubresourceOutput
	, const std::function<void(const void* oRESTRICT _pPixel1, const void* oRESTRICT _pPixel2, void* oRESTRICT _pPixelOut)>& _Enumerator)
{
	if (any(_SurfaceInfoInput.dimensions != _SurfaceInfoOutput.dimensions))
		throw std::invalid_argument(formatf("Dimensions mismatch In(%dx%d) != Out(%dx%d)"
			, _SurfaceInfoInput.dimensions.x
			, _SurfaceInfoInput.dimensions.y
			, _SurfaceInfoOutput.dimensions.x
			, _SurfaceInfoOutput.dimensions.y));
	
	const void* pRow1 = _MappedSubresourceInput1.data;
	const void* pRow2 = _MappedSubresourceInput2.data;
	const void* pEnd1 = byte_add(pRow1, _SurfaceInfoInput.dimensions.y * _MappedSubresourceInput1.row_pitch);
	void* pRowOut = _MappedSubresourceOutput.data;
	const int InputFormatSize = element_size(_SurfaceInfoInput.format);
	const int OutputFormatSize = element_size(_SurfaceInfoOutput.format);
	while (pRow1 < pEnd1)
	{
		const void* pPixel1 = pRow1;
		const void* pPixel2 = pRow2;
		const void* pRowEnd1 = byte_add(pPixel1, _SurfaceInfoInput.dimensions.x * InputFormatSize);
		void* pOutPixel = pRowOut;
		while (pPixel1 < pRowEnd1)
		{
			_Enumerator(pPixel1, pPixel2, pOutPixel);
			pPixel1 = byte_add(pPixel1, InputFormatSize);
			pPixel2 = byte_add(pPixel2, InputFormatSize);
			pOutPixel = byte_add(pOutPixel, OutputFormatSize);
		}

		pRow1 = byte_add(pRow1, _MappedSubresourceInput1.row_pitch);
		pRow2 = byte_add(pRow2, _MappedSubresourceInput2.row_pitch);
		pRowOut = byte_add(pRowOut, _MappedSubresourceOutput.row_pitch);
	}
}

typedef void (*rms_enumerator)(const void* oRESTRICT _pPixel1, const void* oRESTRICT _pPixel2, void* oRESTRICT _pPixelOut, unsigned int* _pAccum);

static void sum_squared_diff_r8_to_r8(const void* oRESTRICT _pPixel1, const void* oRESTRICT _pPixel2, void* oRESTRICT _pPixelOut, unsigned int* _pAccum)
{
	const unsigned char* p1 = (const unsigned char*)_pPixel1;
	const unsigned char* p2 = (const unsigned char*)_pPixel2;
	unsigned char absDiff = unsigned char (abs(*p1 - *p2));
	*(unsigned char*)_pPixelOut = absDiff;
	oStd::atomic_fetch_add(_pAccum, unsigned int(absDiff * absDiff));
}

static void sum_squared_diff_b8g8r8_to_r8(const void* oRESTRICT _pPixel1, const void* oRESTRICT _pPixel2, void* oRESTRICT _pPixelOut, unsigned int* _pAccum)
{
	const unsigned char* p = (const unsigned char*)_pPixel1;
	unsigned char b = *p++; unsigned char g = *p++; unsigned char r = *p++;
	float L1 = color(r, g, b, 255).luminance();
	p = (const unsigned char*)_pPixel2;
	b = *p++; g = *p++; r = *p++;
	float L2 = color(r, g, b, 255).luminance();
	unsigned char absDiff = unorm_to_ubyte(abs(L1 - L2));
	*(unsigned char*)_pPixelOut = absDiff;
	oStd::atomic_fetch_add(_pAccum, unsigned int(absDiff * absDiff));
}

static void sum_squared_diff_b8g8r8a8_to_r8(const void* oRESTRICT _pPixel1, const void* oRESTRICT _pPixel2, void* oRESTRICT _pPixelOut, unsigned int* _pAccum)
{
	const unsigned char* p = (const unsigned char*)_pPixel1;
	unsigned char b = *p++; unsigned char g = *p++; unsigned char r = *p++; unsigned char a = *p++;
	float L1 = color(r, g, b, a).luminance();
	p = (const unsigned char*)_pPixel2;
	b = *p++; g = *p++; r = *p++; a = *p++; 
	float L2 = color(r, g, b, a).luminance();
	unsigned char absDiff = unorm_to_ubyte(abs(L1 - L2));
	*(unsigned char*)_pPixelOut = absDiff;
	oStd::atomic_fetch_add(_pAccum, unsigned int(absDiff * absDiff));
}

static rms_enumerator get_rms_enumerator(format _InFormat, format _OutFormat)
{
	#define IO(i,o) (((i)<<16) | (o))
	int req = IO(_InFormat, _OutFormat);

	switch (req)
	{
		case IO(r8_unorm, r8_unorm): return sum_squared_diff_r8_to_r8;
		case IO(b8g8r8_unorm, r8_unorm): return sum_squared_diff_b8g8r8_to_r8;
		case IO(b8g8r8a8_unorm, r8_unorm): return sum_squared_diff_b8g8r8a8_to_r8;
		default: break;
	}

	throw std::invalid_argument(formatf("%s -> %s not supported", as_string(_InFormat), as_string(_OutFormat)));

	#undef IO
}

float calc_rms(const info& _SurfaceInfo
	, const const_mapped_subresource& _MappedSubresource1
	, const const_mapped_subresource& _MappedSubresource2)
{
	rms_enumerator en = get_rms_enumerator(_SurfaceInfo.format, r8_unorm);
	unsigned int SumOfSquares = 0;
	unsigned int DummyPixelOut[4]; // largest a pixel can ever be currently

	enumerate_pixels(_SurfaceInfo
		, _MappedSubresource1
		, _MappedSubresource2
		, std::bind(en, _1, _2, &DummyPixelOut, &SumOfSquares));

	return sqrt(SumOfSquares / float(_SurfaceInfo.dimensions.x * _SurfaceInfo.dimensions.y));
}

float calc_rms(const info& _SurfaceInfoInput
	, const const_mapped_subresource& _MappedSubresourceInput1
	, const const_mapped_subresource& _MappedSubresourceInput2
	, const info& _SurfaceInfoOutput
	, mapped_subresource& _MappedSubresourceOutput)
{
	std::function<void(const void* _pPixel1, const void* _pPixel2, void* _pPixelOut)> Fn;

	rms_enumerator en = get_rms_enumerator(_SurfaceInfoInput.format, _SurfaceInfoOutput.format);
	unsigned int SumOfSquares = 0;

	enumerate_pixels(_SurfaceInfoInput
		, _MappedSubresourceInput1
		, _MappedSubresourceInput2
		, _SurfaceInfoOutput
		, _MappedSubresourceOutput
		, std::bind(en, _1, _2, _3, &SumOfSquares));

	return sqrt(SumOfSquares / float(_SurfaceInfoInput.dimensions.x * _SurfaceInfoInput.dimensions.y));
}

typedef void (*histogram_enumerator)(const void* _pPixel, unsigned int* _Histogram);

static void histogram_r8_unorm_8bit(const void* _pPixel, unsigned int* _Histogram)
{
	unsigned char c = *(const unsigned char*)_pPixel;
	oStd::atomic_increment(&_Histogram[c]);
}

static void histogram_b8g8r8a8_unorm_8bit(const void* _pPixel, unsigned int* _Histogram)
{
	const unsigned char* p = (const unsigned char*)_pPixel;
	unsigned char b = *p++; unsigned char g = *p++; unsigned char r = *p++;
	unsigned char Index = unorm_to_ubyte(color(r, g, b, 255).luminance());
	oStd::atomic_increment(&_Histogram[Index]);
}

static void histogram_r16_unorm_16bit(const void* _pPixel, unsigned int* _Histogram)
{
	unsigned short c = *(const unsigned short*)_pPixel;
	oStd::atomic_increment(&_Histogram[c]);
}

static void histogram_r16_float_16bit(const void* _pPixel, unsigned int* _Histogram)
{
	half h = saturate(*(const half*)_pPixel);
	unsigned short c = static_cast<unsigned short>(round(65535.0f * h));
	oStd::atomic_increment(&_Histogram[c]);
}

histogram_enumerator get_histogram_enumerator(format _Format, int _Bitdepth)
{
	#define IO(f,b) (((f) << 16) | (b))
	int sel = IO(_Format, _Bitdepth);
	switch (sel)
	{
		case IO(r8_unorm, 8): return histogram_r8_unorm_8bit;
		case IO(b8g8r8a8_unorm, 8): return histogram_b8g8r8a8_unorm_8bit;
		case IO(r16_unorm, 16): return histogram_r16_unorm_16bit;
		case IO(r16_float, 16): return histogram_r16_float_16bit;
		default: break;
	}

	throw std::invalid_argument(formatf("%dbit histogram on %s not supported", _Bitdepth, as_string(_Format)));

	#undef IO
}

void histogram8(const info& _SurfaceInfo, const const_mapped_subresource& _MappedSubresource, unsigned int _Histogram[256])
{
	memset(_Histogram, 0, sizeof(unsigned int) * 256);
	histogram_enumerator en = get_histogram_enumerator(_SurfaceInfo.format, 8);
	enumerate_pixels(_SurfaceInfo, _MappedSubresource, std::bind(en, _1, _Histogram));
}

void histogram16(const info& _SurfaceInfo, const const_mapped_subresource& _MappedSubresource, unsigned int _Histogram[65536])
{
	memset(_Histogram, 0, sizeof(unsigned int) * 65536);
	histogram_enumerator en = get_histogram_enumerator(_SurfaceInfo.format, 16);
	enumerate_pixels(_SurfaceInfo, _MappedSubresource, std::bind(en, _1, _Histogram));
}

	} // namespace surface
} // namespace ouro
