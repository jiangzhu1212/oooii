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
#include <oHLSL/oHLSLMath.h>
#include <atomic>

using namespace std::placeholders;
using namespace std;

namespace ouro {
	namespace surface {

static inline uint safe_array_size(const info& _Info) { return ::max(1u, _Info.array_size); }

#define oCHECK_DIM(f, _Dim) if (_Dim < min_dimensions(f).x) throw invalid_argument(formatf("invalid dimension: %u", _Dim));
#define oCHECK_DIM2(f, _Dim) if (any(_Dim < min_dimensions(f))) throw invalid_argument(formatf("invalid dimensions: [%u,%u]", _Dim.x, _Dim.y));
#define oCHECK_DIM3(f, _Dim) if (any(_Dim.xy < min_dimensions(f))) throw invalid_argument(formatf("invalid dimensions: [%u,%u,%u]", _Dim.x, _Dim.y, _Dim.z));
#define oCHECK_NOT_PLANAR(f) if (is_planar(f)) throw invalid_argument("Planar formats may not behave well with this API. Review usage in this code and remove this when verified.");

struct subformats { format format[4]; };

namespace traits
{	enum value : short {

	none = 0,

	is_bc = 1 << 0,
	is_unorm = 1 << 1,
	is_srgb = 1 << 2,
	has_alpha = 1 << 3,
	is_depth = 1 << 4,
	is_planar = 1 << 5,
	is_yuv = 1 << 6,
	paletted = 1 << 7,
	subsurface1_bias1 = 1 << 8,
	subsurface2_bias1 = 1 << 9,
	subsurface3_bias1 = 1 << 10,

};}

struct format_info
{
	const char* string;
	uint fourcc;
	struct bit_size bit_size;
	struct subformats subformats;
	ushort element_size : 5; // [0,16] bytes per pixel or block
	ushort num_channels : 3; // [0,4]
	ushort num_subformats : 3; // [1,4]
	short traits;
};

static const ushort kMinMip = 1;
static const ushort kMinMipBC = 4;
static const ushort kMinMipYUV = 2;

static const bit_size kUnknownBits = {0,0,0,0};
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

static const fourcc kUnknownFCC = oFCC('????');

static const subformats kNoSubformats = {format::unknown, format::unknown, format::unknown, format::unknown};
static const subformats kSFD_R8_R8 = {format::r8_unorm, format::r8_unorm, format::unknown, format::unknown};
static const subformats kSFD_R8_RG8 = {format::r8_unorm, format::r8g8_unorm, format::unknown, format::unknown};
static const subformats kSFD_RG8_RG8 = {format::r8g8_unorm, format::r8g8_unorm, format::unknown, format::unknown};
static const subformats kSFD_R16_RG16 = {format::r16_unorm, format::r16g16_unorm, format::unknown, format::unknown};
static const subformats kSFD_BC4_BC4 = {format::bc4_unorm, format::bc4_unorm, format::unknown, format::unknown};
static const subformats kSFD_BC4_BC5 = {format::bc4_unorm, format::bc5_unorm, format::unknown, format::unknown};
static const subformats kSFD_BC5_BC5 = {format::bc5_unorm, format::bc5_unorm, format::unknown, format::unknown};
static const subformats kSFD_R8_4 = {format::r8_unorm, format::r8_unorm, format::r8_unorm, format::r8_unorm};
static const subformats kSFD_R8_3 = {format::r8_unorm, format::r8_unorm, format::r8_unorm, format::unknown};
static const subformats kSFD_BC4_4 = {format::bc4_unorm, format::bc4_unorm, format::bc4_unorm, format::bc4_unorm};
static const subformats kSFD_BC4_3 = {format::bc4_unorm, format::bc4_unorm, format::bc4_unorm, format::unknown};

#define FPERM(srgb,depth,typeless,unorm,x,a) { format::srgb, format::depth, format::typeless, format::a8_unorm, format::x, format::a }

static const format_info sFormatInfo[] = 
{
  { "unknown",                    kUnknownFCC,  kUnknownBits, kNoSubformats,    0, 0, 0, traits::none },
  { "r32g32b32a32_typeless",      oFCC('?i4 '), kBS_4_32,     kNoSubformats,  128, 4, 1, traits::has_alpha },
  { "r32g32b32a32_float",         oFCC('f4  '), kBS_4_32,     kNoSubformats,  128, 4, 1, traits::has_alpha },
  { "r32g32b32a32_uint",          oFCC('ui4 '), kBS_4_32,     kNoSubformats,  128, 4, 1, traits::has_alpha },
  { "r32g32b32a32_sint",          oFCC('si4 '), kBS_4_32,     kNoSubformats,  128, 4, 1, traits::has_alpha },
  { "r32g32b32_typeless",         oFCC('?i3 '), kBS_3_32,     kNoSubformats,   12, 3, 1, traits::has_alpha },
  { "r32g32b32_float",            oFCC('f3  '), kBS_3_32,     kNoSubformats,   12, 3, 1, traits::none },
  { "r32g32b32_uint",             oFCC('ui3 '), kBS_3_32,     kNoSubformats,   12, 3, 1, traits::none },
  { "r32g32b32_sint",             oFCC('si3 '), kBS_3_32,     kNoSubformats,   12, 3, 1, traits::none },
  { "r16g16b16a16_typeless",      oFCC('?s4 '), kBS_4_16,     kNoSubformats,    8, 4, 1, traits::has_alpha },
  { "r16g16b16a16_float",         oFCC('h4  '), kBS_4_16,     kNoSubformats,    8, 4, 1, traits::has_alpha },
  { "r16g16b16a16_unorm",         oFCC('h4u '), kBS_4_16,     kNoSubformats,    8, 4, 1, traits::is_unorm|traits::has_alpha },
  { "r16g16b16a16_uint",          oFCC('us4 '), kBS_4_16,     kNoSubformats,    8, 4, 1, traits::has_alpha },
  { "r16g16b16a16_snorm",         oFCC('h4s '), kBS_4_16,     kNoSubformats,    8, 4, 1, traits::has_alpha },
  { "r16g16b16a16_sint",          oFCC('ss4 '), kBS_4_16,     kNoSubformats,    8, 4, 1, traits::has_alpha },
  { "r32g32_typeless",            oFCC('?i2 '), kBS_2_32,     kNoSubformats,    8, 2, 1, traits::none },
  { "r32g32_float",               oFCC('f2  '), kBS_2_32,     kNoSubformats,    8, 2, 1, traits::none },
  { "r32g32_uint",                oFCC('ui2 '), kBS_2_32,     kNoSubformats,    8, 2, 1, traits::none },
  { "r32g32_sint",                oFCC('si2 '), kBS_2_32,     kNoSubformats,    8, 2, 1, traits::none },
  { "r32g8x24_typeless",          kUnknownFCC,  {32,8,0,24},  kNoSubformats,    8, 3, 1, traits::none },
  { "d32_float_s8x24_uint",       kUnknownFCC,  {32,8,0,24},  kNoSubformats,    8, 3, 1, traits::is_depth },
  { "r32_float_x8x24_typeless",   kUnknownFCC,  {32,8,0,24},  kNoSubformats,    8, 3, 1, traits::is_depth },
  { "x32_typeless_g8x24_uint",    kUnknownFCC,  {32,8,0,24},  kNoSubformats,    8, 3, 1, traits::none },
  { "r10g10b10a2_typeless",       kUnknownFCC,  kBS_DEC3N,    kNoSubformats,    4, 4, 1, traits::has_alpha },
  { "r10g10b10a2_unorm",          kUnknownFCC,  kBS_DEC3N,    kNoSubformats,    4, 4, 1, traits::is_unorm|traits::has_alpha },
  { "r10g10b10a2_uint",           kUnknownFCC,  kBS_DEC3N,    kNoSubformats,    4, 4, 1, traits::has_alpha },
  { "r11g11b10_float",            kUnknownFCC,  {11,11,10,0}, kNoSubformats,    4, 3, 1, traits::none },
  { "r8g8b8a8_typeless",          oFCC('?c4 '), kBS_4_8,      kNoSubformats,    4, 4, 1, traits::has_alpha },
  { "r8g8b8a8_unorm",             oFCC('c4u '), kBS_4_8,      kNoSubformats,    4, 4, 1, traits::is_unorm|traits::has_alpha },
  { "r8g8b8a8_unorm_srgb",        oFCC('c4us'), kBS_4_8,      kNoSubformats,    4, 4, 1, traits::is_unorm|traits::is_srgb|traits::has_alpha },
  { "r8g8b8a8_uint",              oFCC('uc4 '), kBS_4_8,      kNoSubformats,    4, 4, 1, traits::has_alpha },
  { "r8g8b8a8_snorm",             oFCC('c4s '), kBS_4_8,      kNoSubformats,    4, 4, 1, traits::has_alpha },
  { "r8g8b8a8_sint",              oFCC('sc4 '), kBS_4_8,      kNoSubformats,    4, 4, 1, traits::has_alpha },
  { "r16g16_typeless",            oFCC('?s2 '), kBS_2_16,     kNoSubformats,    4, 2, 1, traits::none },
  { "r16g16_float",               oFCC('h2  '), kBS_2_16,     kNoSubformats,    4, 2, 1, traits::none },
  { "r16g16_unorm",               oFCC('h2u '), kBS_2_16,     kNoSubformats,    4, 2, 1, traits::is_unorm },
  { "r16g16_uint",                oFCC('us2 '), kBS_2_16,     kNoSubformats,    4, 2, 1, traits::none },
  { "r16g16_snorm",               oFCC('h2s '), kBS_2_16,     kNoSubformats,    4, 2, 1, traits::none },
  { "r16g16_sint",                oFCC('ss2 '), kBS_2_16,     kNoSubformats,    4, 2, 1, traits::none },
  { "r32_typeless",               oFCC('?i1 '), kBS_1_32,     kNoSubformats,    4, 1, 1, traits::is_depth },
  { "d32_float",                  oFCC('f1d '), kBS_1_32,     kNoSubformats,    4, 1, 1, traits::is_depth },
  { "r32_float",                  oFCC('f1  '), kBS_1_32,     kNoSubformats,    4, 1, 1, traits::none },
  { "r32_uint",                   oFCC('ui1 '), kBS_1_32,     kNoSubformats,    4, 1, 1, traits::none },
  { "r32_sint",                   oFCC('si1 '), kBS_1_32,     kNoSubformats,    4, 1, 1, traits::none },
  { "r24g8_typeless",             kUnknownFCC,  kBS_DS,       kNoSubformats,    4, 2, 1, traits::is_depth },
  { "d24_unorm_s8_uint",          kUnknownFCC,  kBS_DS,       kNoSubformats,    4, 2, 1, traits::is_unorm|traits::is_depth },
  { "r24_unorm_x8_typeless",      kUnknownFCC,  kBS_DS,       kNoSubformats,    4, 2, 1, traits::is_unorm|traits::is_depth },
  { "x24_typeless_g8_uint",       kUnknownFCC,  kBS_DS,       kNoSubformats,    4, 2, 1, traits::is_depth },
  { "r8g8_typeless",              oFCC('?c2 '), kBS_2_8,      kNoSubformats,    2, 2, 1, traits::none },
  { "r8g8_unorm",                 oFCC('uc2u'), kBS_2_8,      kNoSubformats,    2, 2, 1, traits::is_unorm },
  { "r8g8_uint",                  oFCC('ui2 '), kBS_2_8,      kNoSubformats,    2, 2, 1, traits::none },
  { "r8g8_snorm",                 oFCC('uc2s'), kBS_2_8,      kNoSubformats,    2, 2, 1, traits::none },
  { "r8g8_sint",                  oFCC('si2 '), kBS_2_8,      kNoSubformats,    2, 2, 1, traits::none },
  { "r16_typeless",               oFCC('?s1 '), kBS_1_16,     kNoSubformats,    2, 1, 1, traits::is_depth },
  { "r16_float",                  oFCC('h1  '), kBS_1_16,     kNoSubformats,    2, 1, 1, traits::none },
  { "d16_unorm",                  oFCC('h1ud'), kBS_1_16,     kNoSubformats,    2, 1, 1, traits::is_unorm|traits::is_depth },
  { "r16_unorm",                  oFCC('h1u '), kBS_1_16,     kNoSubformats,    2, 1, 1, traits::is_unorm },
  { "r16_uint",                   oFCC('us1 '), kBS_1_16,     kNoSubformats,    2, 1, 1, traits::none },
  { "r16_snorm",                  oFCC('h1s '), kBS_1_16,     kNoSubformats,    2, 1, 1, traits::none },
  { "r16_sint",                   oFCC('ss1 '), kBS_1_16,     kNoSubformats,    2, 1, 1, traits::none },
  { "r8_typeless",                oFCC('?c1 '), kBS_1_8,      kNoSubformats,    1, 1, 1, traits::none },
  { "r8_unorm",                   oFCC('uc1u'), kBS_1_8,      kNoSubformats,    1, 1, 1, traits::is_unorm },
  { "r8_uint",                    oFCC('uc1 '), kBS_1_8,      kNoSubformats,    1, 1, 1, traits::none },
  { "r8_snorm",                   oFCC('uc1s'), kBS_1_8,      kNoSubformats,    1, 1, 1, traits::none },
  { "r8_sint",                    oFCC('sc1 '), kBS_1_8,      kNoSubformats,    1, 1, 1, traits::none },
  { "a8_unorm",                   oFCC('ac1u'), kBS_1_8,      kNoSubformats,    1, 1, 1, traits::is_unorm|traits::has_alpha },
  { "r1_unorm",                   oFCC('bitu'), {1,0,0,0},    kNoSubformats,    1, 1, 1, traits::is_unorm },
  { "r9g9b9e5_sharedexp",         kUnknownFCC, {9,9,9,5},     kNoSubformats,    4, 4, 1, traits::none },
  { "r8g8_b8g8_unorm",            kUnknownFCC, kBS_4_8,       kNoSubformats,    4, 4, 1, traits::is_unorm },
  { "g8r8_g8b8_unorm",            kUnknownFCC, kBS_4_8,       kNoSubformats,    4, 4, 1, traits::is_unorm },
  { "bc1_typeless",               oFCC('BC1?'), kBS_565,      kNoSubformats,    8, 3, 1, traits::is_bc },
  { "bc1_unorm",                  oFCC('BC1u'), kBS_565,      kNoSubformats,    8, 3, 1, traits::is_bc|traits::is_unorm },
  { "bc1_unorm_srgb",             oFCC('BC1s'), kBS_565,      kNoSubformats,    8, 3, 1, traits::is_bc|traits::is_unorm|traits::is_srgb },
  { "bc2_typeless",               oFCC('BC2?'), {5,6,5,4},    kNoSubformats,   16, 4, 1, traits::is_bc|traits::has_alpha },
  { "bc2_unorm",                  oFCC('BC2u'), {5,6,5,4},    kNoSubformats,   16, 4, 1, traits::is_bc|traits::is_unorm|traits::has_alpha },
  { "bc2_unorm_srgb",             oFCC('BC2s'), {5,6,5,4},    kNoSubformats,   16, 4, 1, traits::is_bc|traits::is_unorm|traits::is_srgb|traits::has_alpha },
  { "bc3_typeless",               oFCC('BC3?'), {5,6,5,8},    kNoSubformats,   16, 4, 1, traits::is_bc|traits::has_alpha },
  { "bc3_unorm",                  oFCC('BC3u'), {5,6,5,8},    kNoSubformats,   16, 4, 1, traits::is_bc|traits::is_unorm|traits::has_alpha },
  { "bc3_unorm_srgb",             oFCC('BC3s'), {5,6,5,8},    kNoSubformats,   16, 4, 1, traits::is_bc|traits::is_unorm|traits::is_srgb|traits::has_alpha },
  { "bc4_typeless",               oFCC('BC4?'), kBS_1_8,      kNoSubformats,    8, 1, 1, traits::is_bc },
  { "bc4_unorm",                  oFCC('BC4u'), kBS_1_8,      kNoSubformats,    8, 1, 1, traits::is_bc|traits::is_unorm },
  { "bc4_snorm",                  oFCC('BC4s'), kBS_1_8,      kNoSubformats,    8, 1, 1, traits::is_bc },
  { "bc5_typeless",               oFCC('BC5?'), kBS_2_8,      kNoSubformats,   16, 2, 1, traits::is_bc },
  { "bc5_unorm",                  oFCC('BC5u'), kBS_2_8,      kNoSubformats,   16, 2, 1, traits::is_bc|traits::is_unorm },
  { "bc5_snorm",                  oFCC('BC5s'), kBS_2_8,      kNoSubformats,   16, 2, 1, traits::is_bc },
  { "b5g6r5_unorm",               kUnknownFCC,  kBS_565,      kNoSubformats,    2, 3, 1, traits::is_unorm },
  { "b5g5r5a1_unorm",             kUnknownFCC,  {5,6,5,1},    kNoSubformats,    2, 4, 1, traits::is_unorm|traits::has_alpha },
  { "b8g8r8a8_unorm",             oFCC('c4u '), kBS_4_8,      kNoSubformats,    4, 4, 1, traits::is_unorm|traits::has_alpha },
  { "b8g8r8x8_unorm",             oFCC('c4u '), kBS_4_8,      kNoSubformats,    4, 4, 1, traits::is_unorm },
  { "r10g10b10_xr_bias_a2_unorm", kUnknownFCC,  kBS_DEC3N,    kNoSubformats,    4, 4, 1, traits::is_unorm },
  { "b8g8r8a8_typeless",          kUnknownFCC,  kBS_4_8,      kNoSubformats,    4, 4, 1, traits::none },
  { "b8g8r8a8_unorm_srgb",        kUnknownFCC,  kBS_4_8,      kNoSubformats,    4, 4, 1, traits::is_unorm|traits::is_srgb },
  { "b8g8r8x8_typeless",          kUnknownFCC,  kBS_4_8,      kNoSubformats,    4, 4, 1, traits::none },
  { "b8g8r8x8_unorm_srgb",        kUnknownFCC,  kBS_4_8,      kNoSubformats,    4, 4, 1, traits::is_unorm|traits::is_srgb },
  { "bc6h_typeless",              oFCC('BC6?'), kUnknownBits, kNoSubformats,   16, 3, 1, traits::is_bc },
  { "bc6h_uf16",                  oFCC('BC6u'), kUnknownBits, kNoSubformats,   16, 3, 1, traits::is_bc },
  { "bc6h_sf16",                  oFCC('BC6s'), kUnknownBits, kNoSubformats,   16, 3, 1, traits::is_bc },
  { "bc7_typeless",               oFCC('BC7?'), kUnknownBits, kNoSubformats,   16, 4, 1, traits::is_bc|traits::has_alpha },
  { "bc7_unorm",                  oFCC('BC7u'), kUnknownBits, kNoSubformats,   16, 4, 1, traits::is_bc|traits::is_unorm|traits::has_alpha },
  { "bc7_unorm_srgb",             oFCC('BC7s'), kUnknownBits, kNoSubformats,   16, 4, 1, traits::is_bc|traits::is_unorm|traits::is_srgb|traits::has_alpha },
  { "ayuv",                       oFCC('AYUV'), kBS_4_4,      kNoSubformats,   16, 4, 1, traits::is_unorm|traits::has_alpha|traits::is_yuv },
  { "y410",                       oFCC('Y410'), kBS_DEC3N,    {format::r10g10b10a2_unorm,format::unknown}, 4, 4, 1, traits::is_unorm|traits::has_alpha|traits::is_yuv },
  { "y416",                       oFCC('Y416'), kBS_4_16,     {format::b8g8r8a8_unorm,format::unknown}, 4, 4, 1, traits::is_unorm|traits::has_alpha|traits::is_yuv },
  { "nv12",                       oFCC('NV12'), kBS_3_8,      kSFD_R8_RG8,      1, 3, 2, traits::is_unorm|traits::is_yuv },
  { "p010",                       oFCC('P010'), {10,10,10,0}, kSFD_R16_RG16,    2, 3, 2, traits::is_unorm|traits::is_planar|traits::is_yuv },
  { "p016",                       oFCC('P016'), kBS_3_16,     kSFD_R16_RG16,    2, 3, 2, traits::is_unorm|traits::is_planar|traits::is_yuv },
  { "420_opaque",                 oFCC('420O'), kBS_3_8,      kSFD_R8_RG8,      1, 3, 2, traits::is_unorm|traits::is_planar|traits::is_yuv },
  { "yuy2",                       oFCC('YUY2'), kBS_4_8,      kSFD_R8_RG8,      4, 4, 1, traits::is_unorm|traits::has_alpha|traits::is_yuv },
  { "y210",                       oFCC('Y210'), kBS_4_16,     kNoSubformats,    2, 3, 1, traits::is_unorm|traits::is_yuv },
  { "y216",                       oFCC('Y216'), kBS_4_16,     kNoSubformats,    2, 3, 1, traits::is_unorm|traits::is_yuv },
  { "nv11",                       kUnknownFCC,  kBS_3_8,      kSFD_R8_RG8,      1, 3, 2, traits::is_unorm|traits::is_planar|traits::is_yuv },
  { "ia44",                       oFCC('IA44'), {4,0,0,4},    kNoSubformats,    1, 2, 1, traits::has_alpha|traits::paletted }, // index-alpha
  { "ai44",                       oFCC('AI44'), {4,0,0,4},    kNoSubformats,    1, 2, 1, traits::has_alpha|traits::paletted }, // alpha-index
  { "p8",                         oFCC('P8  '), kUnknownBits, kNoSubformats,    1, 1, 1, traits::paletted },
  { "a8p8",                       oFCC('A8P8'), kUnknownBits, kNoSubformats,    2, 2, 1, traits::has_alpha|traits::paletted },
  { "b4g4r4a4_unorm",             oFCC('n4u '), kBS_4_4,      kNoSubformats,    2, 4, 1, traits::is_unorm|traits::has_alpha },
  { "r8g8b8_unorm",               oFCC('c3u '), kBS_3_8,      kNoSubformats,    3, 3, 1, traits::is_unorm },
  { "r8g8b8_unorm_srgb",          kUnknownFCC,	kBS_3_8,      kNoSubformats,    3, 3, 1, traits::is_unorm|traits::is_srgb },
	{ "r8g8b8x8_unorm",             kUnknownFCC,	kBS_4_8,      kNoSubformats,		4, 4, 1, traits::is_unorm },
  { "r8g8b8x8_unorm_srgb",        kUnknownFCC,	kBS_4_8,      kNoSubformats,    4, 4, 1, traits::is_unorm|traits::is_srgb },
	{ "b8g8r8_unorm",               kUnknownFCC,	kBS_3_8,      kNoSubformats,    3, 3, 1, traits::is_unorm },
  { "b8g8r8_unorm_srgb",          kUnknownFCC,	kBS_3_8,      kNoSubformats,    3, 3, 1, traits::is_unorm|traits::is_srgb },
  { "a8b8g8r8_unorm",             oFCC('abgr'), kBS_4_8,      kNoSubformats,    4, 4, 1, traits::is_unorm|traits::has_alpha },
  { "a8b8g8r8_unorm_srgb",        kUnknownFCC,	kBS_4_8,      kNoSubformats,    4, 4, 1, traits::is_unorm|traits::is_srgb|traits::has_alpha },
  { "x8b8g8r8_unorm",             oFCC('xbgr'), kBS_4_8,      kNoSubformats,    4, 4, 1, traits::is_unorm },
  { "x8b8g8r8_unorm_srgb",        kUnknownFCC,	kBS_4_8,      kNoSubformats,    4, 4, 1, traits::is_unorm|traits::is_srgb },
  { "y8_u8_v8_unorm",             oFCC('yuv8'), kBS_3_8,      kSFD_R8_3,        1, 3, 3, traits::is_unorm|traits::is_planar|traits::is_yuv|traits::subsurface1_bias1|traits::subsurface2_bias1 },
  { "y8_a8_u8_v8_unorm",          oFCC('auv8'), kBS_3_8,      kSFD_R8_4,        1, 3, 4, traits::is_unorm|traits::has_alpha|traits::is_planar|traits::is_yuv|traits::subsurface2_bias1|traits::subsurface3_bias1 },
  { "ybc4_ubc4_vbc4_unorm",       oFCC('yuvb'), kBS_4_8,      kSFD_BC4_3,       8, 3, 3, traits::is_bc|traits::is_unorm|traits::is_planar|traits::is_yuv|traits::subsurface1_bias1|traits::subsurface2_bias1 },
  { "ybc4_abc4_ubc4_vbc4_unorm",  oFCC('auvb'), kBS_4_8,      kSFD_BC4_4,       8, 3, 4, traits::is_bc|traits::is_unorm|traits::has_alpha|traits::is_planar|traits::is_yuv|traits::subsurface2_bias1|traits::subsurface3_bias1 },
  { "y8_u8v8_unorm",              oFCC('yv8u'), kBS_3_8,      kSFD_R8_RG8,      1, 3, 2, traits::is_unorm|traits::is_planar|traits::is_yuv|traits::subsurface1_bias1 },
  { "y8a8_u8v8_unorm",            oFCC('av8u'), kBS_4_8,      kSFD_RG8_RG8,     2, 4, 2, traits::is_unorm|traits::has_alpha|traits::is_planar|traits::is_yuv|traits::subsurface1_bias1 },
  { "ybc4_uvbc5_unorm",           oFCC('yvbu'), kBS_3_8,      kSFD_BC4_BC5,     8, 3, 2, traits::is_bc|traits::is_unorm|traits::is_planar|traits::is_yuv|traits::subsurface1_bias1 },
  { "yabc5_uvbc5_unorm",          oFCC('avbu'), kBS_4_8,      kSFD_BC5_BC5,    16, 4, 2, traits::is_bc|traits::is_unorm|traits::has_alpha|traits::is_planar|traits::is_yuv|traits::subsurface1_bias1 },
};
static_assert(oCOUNTOF(sFormatInfo) == (int)format::count, "array mismatch");

	} // namespace surface

const char* as_string(const surface::format& f)
{
	return (f < surface::format::count) ? surface::sFormatInfo[int(f)].string : "unknown";
}

char* to_string(char* _StrDestination, size_t _SizeofStrDestination, const surface::format& f)
{
	return strlcpy(_StrDestination, as_string(f), _SizeofStrDestination) < _SizeofStrDestination ? _StrDestination : nullptr;
}

bool from_string(surface::format* _pFormat, const char* _StrSource)
{
	*_pFormat = surface::format::unknown;
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

const char* as_string(const surface::cube_face& f)
{
	switch (f)
	{
		case surface::cube_face::posx: return "posx";
		case surface::cube_face::negx: return "negx";
		case surface::cube_face::posy: return "posy";
		case surface::cube_face::negy: return "negy";
		case surface::cube_face::posz: return "posz";
		case surface::cube_face::negz: return "negz";
		default: break;
	}
	return "?";
}

oDEFINE_TO_STRING(surface::cube_face)
oDEFINE_FROM_STRING_ENUM_CLASS(surface::cube_face)

const char* as_string(const surface::semantic& s)
{
	switch (s)
	{
		case surface::semantic::unknown: return "unknown";
		case surface::semantic::vertex_position: return "vertex_position";
		case surface::semantic::vertex_normal: return "vertex_normal";
		case surface::semantic::vertex_tangent: return "vertex_tangent";
		case surface::semantic::vertex_texcoord: return "vertex_texcoord";
		case surface::semantic::vertex_color: return "vertex_color";
		case surface::semantic::index: return "index";
		case surface::semantic::color: return "color";
		case surface::semantic::tangent_normal: return "tangent_normal";
		case surface::semantic::world_normal: return "world_normal";
		case surface::semantic::specular: return "specular";
		case surface::semantic::diffuse: return "diffuse";
		case surface::semantic::height: return "height";
		case surface::semantic::noise: return "noise";
		case surface::semantic::intensity: return "intensity";
		case surface::semantic::custom1d: return "custom1d";
		case surface::semantic::color_correction1d: return "color_correction1d";
		case surface::semantic::custom3d: return "custom3d";
		case surface::semantic::color_correction3d: return "color_correction3d";
		case surface::semantic::customcube: return "customcube";
		case surface::semantic::colorcube: return "colorcube";
		case surface::semantic::custom: return "custom";
		default: break;
	}
	return "?";
}

oDEFINE_TO_STRING(surface::semantic)
oDEFINE_FROM_STRING_ENUM_CLASS(surface::semantic)

namespace surface {

format as_srgb(const format& f)
{
	switch (f)
	{
		case format::r8g8b8a8_typeless:
		case format::r8g8b8a8_unorm:
		case format::r8g8b8a8_unorm_srgb:
		case format::r8g8b8a8_uint:
		case format::r8g8b8a8_snorm:
		case format::r8g8b8a8_sint: return format::r8g8b8a8_unorm_srgb;
		case format::bc1_typeless:
		case format::bc1_unorm:
		case format::bc1_unorm_srgb: return format::bc1_unorm_srgb;
		case format::bc2_typeless:
		case format::bc2_unorm:
		case format::bc2_unorm_srgb: return format::bc2_unorm_srgb;
		case format::bc3_typeless:
		case format::bc3_unorm:
		case format::bc3_unorm_srgb: return format::bc3_unorm_srgb;
		case format::b8g8r8a8_typeless:
		case format::b8g8r8a8_unorm_srgb: return format::b8g8r8a8_unorm_srgb;
		case format::b8g8r8x8_typeless:
		case format::b8g8r8x8_unorm_srgb: return format::b8g8r8x8_unorm_srgb;
		case format::bc7_typeless:
		case format::bc7_unorm:
		case format::bc7_unorm_srgb: return format::bc7_unorm_srgb;
		case format::r8g8b8_unorm:
		case format::r8g8b8_unorm_srgb: return format::r8g8b8_unorm_srgb;
		case format::r8g8b8x8_unorm:
		case format::r8g8b8x8_unorm_srgb: return format::r8g8b8x8_unorm_srgb;
		case format::b8g8r8_unorm:
		case format::b8g8r8_unorm_srgb: return format:: b8g8r8_unorm_srgb;
		case format::a8b8g8r8_unorm:
		case format::a8b8g8r8_unorm_srgb: return format::a8b8g8r8_unorm_srgb;
		case format::x8b8g8r8_unorm:
		case format::x8b8g8r8_unorm_srgb: return format::x8b8g8r8_unorm_srgb;
		default: break;
	}
	return format::unknown;
}

format as_depth(const format& f)
{
	switch (f)
	{
		case format::d32_float_s8x24_uint:
		case format::r32_float_x8x24_typeless:
		case format::x32_typeless_g8x24_uint: return format::d32_float_s8x24_uint;
		case format::r32_typeless:
		case format::d32_float:
		case format::r32_float: return format::d32_float;
		case format::r24g8_typeless:
		case format::d24_unorm_s8_uint:
		case format::r24_unorm_x8_typeless:
		case format::x24_typeless_g8_uint: return format::d24_unorm_s8_uint;
		case format::r16_typeless:
		case format::r16_float:
		case format::d16_unorm:
		case format::r16_unorm:
		case format::r16_uint:
		case format::r16_snorm:
		case format::r16_sint: return format::d16_unorm;
		default: break;
	}
	return format::unknown;
}

format as_typeless(const format& f)
{
	switch (f)
	{
		case format::r32g32b32a32_typeless:
		case format::r32g32b32a32_float:
		case format::r32g32b32a32_uint:
		case format::r32g32b32a32_sint: return format::r32g32b32a32_typeless;
		case format::r32g32b32_typeless:
		case format::r32g32b32_float:
		case format::r32g32b32_uint:
		case format::r32g32b32_sint: return format::r32g32b32_typeless;
		case format::r16g16b16a16_typeless:
		case format::r16g16b16a16_float:
		case format::r16g16b16a16_unorm:
		case format::r16g16b16a16_uint:
		case format::r16g16b16a16_snorm:
		case format::r16g16b16a16_sint: return format::r16g16b16a16_typeless;
		case format::r32g32_typeless:
		case format::r32g32_float:
		case format::r32g32_uint:
		case format::r32g32_sint: return format::r32g32_typeless;
		case format::r32g8x24_typeless: 
		case format::d32_float_s8x24_uint: return format::r32g8x24_typeless;
		case format::r32_float_x8x24_typeless:
		case format::x32_typeless_g8x24_uint: return format::r32_float_x8x24_typeless;
		case format::r10g10b10a2_typeless:
		case format::r10g10b10a2_unorm:
		case format::r10g10b10a2_uint: return format::r10g10b10a2_typeless;
		case format::r8g8b8a8_typeless:
		case format::r8g8b8a8_unorm:
		case format::r8g8b8a8_unorm_srgb:
		case format::r8g8b8a8_uint:
		case format::r8g8b8a8_snorm:
		case format::r8g8b8a8_sint: return format::r8g8b8a8_typeless;
		case format::r16g16_typeless:
		case format::r16g16_float:
		case format::r16g16_unorm:
		case format::r16g16_uint:
		case format::r16g16_snorm:
		case format::r16g16_sint: return format::r16g16_typeless;
		case format::r32_typeless:
		case format::d32_float:
		case format::r32_float:
		case format::r32_uint:
		case format::r32_sint: return format::r32_typeless;
		case format::r24g8_typeless:
		case format::d24_unorm_s8_uint:
		case format::r24_unorm_x8_typeless:
		case format::x24_typeless_g8_uint: return format::r24g8_typeless;
		case format::r8g8_typeless:
		case format::r8g8_unorm:
		case format::r8g8_uint:
		case format::r8g8_snorm:
		case format::r8g8_sint: return format::r8g8_typeless;
		case format::r16_typeless:
		case format::r16_float:
		case format::d16_unorm:
		case format::r16_unorm:
		case format::r16_uint:
		case format::r16_snorm:
		case format::r16_sint: return format::r16_typeless;
		case format::r8_typeless:
		case format::r8_unorm:
		case format::r8_uint:
		case format::r8_snorm:
		case format::r8_sint: return format::r8_unorm;
		case format::bc1_typeless:
		case format::bc1_unorm:
		case format::bc1_unorm_srgb: return format::bc1_typeless;
		case format::bc2_typeless:
		case format::bc2_unorm:
		case format::bc2_unorm_srgb: return format::bc2_typeless;
		case format::bc3_typeless:
		case format::bc3_unorm:
		case format::bc3_unorm_srgb: return format::bc3_typeless;
		case format::bc4_typeless:
		case format::bc4_unorm:
		case format::bc4_snorm: return format::bc4_typeless;
		case format::bc5_typeless:
		case format::bc5_unorm:
		case format::bc5_snorm: return format::bc5_typeless;
		case format::b8g8r8a8_typeless:
		case format::b8g8r8a8_unorm_srgb: return format::b8g8r8a8_typeless;
		case format::b8g8r8x8_typeless:
		case format::b8g8r8x8_unorm_srgb: return format::b8g8r8x8_typeless;
		case format::bc6h_typeless:
		case format::bc6h_uf16:
		case format::bc6h_sf16: return format::bc6h_typeless;
		case format::bc7_typeless:
		case format::bc7_unorm:
		case format::bc7_unorm_srgb: return format::bc7_typeless;
		default: break;
	}
	return format::unknown;
}

format as_unorm(const format& f)
{
	switch (f)
	{
		case format::r16g16b16a16_typeless:
		case format::r16g16b16a16_float:
		case format::r16g16b16a16_unorm:
		case format::r16g16b16a16_uint:
		case format::r16g16b16a16_snorm:
		case format::r16g16b16a16_sint: return format::r16g16b16a16_unorm;
		case format::r10g10b10a2_typeless:
		case format::r10g10b10a2_unorm:
		case format::r10g10b10a2_uint: return format::r10g10b10a2_unorm;
		case format::r8g8b8a8_typeless:
		case format::r8g8b8a8_unorm:
		case format::r8g8b8a8_unorm_srgb:
		case format::r8g8b8a8_uint:
		case format::r8g8b8a8_snorm:
		case format::r8g8b8a8_sint: return format::r8g8b8a8_unorm;
		case format::r16g16_typeless:
		case format::r16g16_float:
		case format::r16g16_unorm:
		case format::r16g16_uint:
		case format::r16g16_snorm:
		case format::r16g16_sint: return format::r16g16_unorm;
		case format::r24g8_typeless:
		case format::d24_unorm_s8_uint:
		case format::r24_unorm_x8_typeless:
		case format::x24_typeless_g8_uint: return format::r24_unorm_x8_typeless;
		case format::r8g8_typeless:
		case format::r8g8_unorm:
		case format::r8g8_uint:
		case format::r8g8_snorm:
		case format::r8g8_sint: return format::r8g8_unorm;
		case format::r16_typeless:
		case format::r16_float:
		case format::d16_unorm:
		case format::r16_unorm:
		case format::r16_uint:
		case format::r16_snorm:
		case format::r16_sint: return format::r16_unorm;
		case format::r8_typeless:
		case format::r8_unorm:
		case format::r8_uint:
		case format::r8_snorm:
		case format::r8_sint: return format::r8_unorm;
		case format::bc1_typeless:
		case format::bc1_unorm:
		case format::bc1_unorm_srgb: return format::bc1_unorm;
		case format::bc2_typeless:
		case format::bc2_unorm:
		case format::bc2_unorm_srgb: return format::bc2_unorm;
		case format::bc3_typeless:
		case format::bc3_unorm:
		case format::bc3_unorm_srgb: return format::bc3_unorm;
		case format::bc4_typeless:
		case format::bc4_unorm:
		case format::bc4_snorm: return format::bc4_unorm;
		case format::bc5_typeless:
		case format::bc5_unorm:
		case format::bc5_snorm: return format::bc5_unorm;
		case format::b8g8r8a8_typeless:
		case format::b8g8r8a8_unorm_srgb: return format::b8g8r8a8_unorm;
		case format::b8g8r8x8_typeless:
		case format::b8g8r8x8_unorm_srgb: return format::b8g8r8x8_unorm;
		case format::bc7_typeless:
		case format::bc7_unorm:
		case format::bc7_unorm_srgb: return format::bc7_unorm;
		case format::b4g4r4a4_unorm: return format::b4g4r4a4_unorm;
		case format::r8g8b8_unorm:
		case format::r8g8b8_unorm_srgb: return format::r8g8b8_unorm;
		case format::r8g8b8x8_unorm:
		case format::r8g8b8x8_unorm_srgb: return format::r8g8b8x8_unorm;
		case format::b8g8r8_unorm:
		case format::b8g8r8_unorm_srgb: return format::b8g8r8_unorm;
		case format::a8b8g8r8_unorm:
		case format::a8b8g8r8_unorm_srgb: return format::a8b8g8r8_unorm;
		case format::x8b8g8r8_unorm:
		case format::x8b8g8r8_unorm_srgb: return format::x8b8g8r8_unorm;
		case format::y8_u8_v8_unorm:
		case format::y8_a8_u8_v8_unorm:
		case format::ybc4_ubc4_vbc4_unorm:
		case format::ybc4_abc4_ubc4_vbc4_unorm:
		case format::y8_u8v8_unorm:
		case format::y8a8_u8v8_unorm:
		case format::ybc4_uvbc5_unorm:
		case format::yabc5_uvbc5_unorm: return f;
		default: break;
	}
	return format::unknown;
}

format as_x(const format& f)
{
	switch (f)
	{
		case format::r8g8b8_unorm:
		case format::r8g8b8a8_unorm:
		case format::r8g8b8x8_unorm: return format::r8g8b8x8_unorm;
		case format::r8g8b8_unorm_srgb:
		case format::r8g8b8a8_unorm_srgb:
		case format::r8g8b8x8_unorm_srgb: return format::r8g8b8x8_unorm_srgb;
		case format::b8g8r8_unorm:
		case format::b8g8r8a8_unorm:
		case format::b8g8r8x8_unorm: return format::b8g8r8x8_unorm;
		case format::b8g8r8a8_typeless:
		case format::b8g8r8x8_typeless: return format::b8g8r8x8_typeless;
		case format::b8g8r8_unorm_srgb:
		case format::b8g8r8a8_unorm_srgb:
		case format::b8g8r8x8_unorm_srgb: return format::b8g8r8x8_unorm_srgb;
		case format::a8b8g8r8_unorm:
		case format::x8b8g8r8_unorm: return format::x8b8g8r8_unorm;
		case format::a8b8g8r8_unorm_srgb:
		case format::x8b8g8r8_unorm_srgb: return format::x8b8g8r8_unorm_srgb;
		default: break;
	}
	return format::unknown;
}

format as_a(const format& f)
{
	switch (f)
	{
		case format::r8g8b8_unorm:
		case format::r8g8b8a8_unorm:
		case format::r8g8b8x8_unorm: return format::r8g8b8a8_unorm;
		case format::r8g8b8_unorm_srgb:
		case format::r8g8b8a8_unorm_srgb:
		case format::r8g8b8x8_unorm_srgb: return format::r8g8b8a8_unorm_srgb;
		case format::b8g8r8_unorm:
		case format::b8g8r8a8_unorm:
		case format::b8g8r8x8_unorm: return format::b8g8r8a8_unorm;
		case format::b8g8r8a8_typeless:
		case format::b8g8r8x8_typeless: return format::b8g8r8a8_typeless;
		case format::b8g8r8_unorm_srgb:
		case format::b8g8r8a8_unorm_srgb:
		case format::b8g8r8x8_unorm_srgb: return format::b8g8r8a8_unorm_srgb;
		case format::a8b8g8r8_unorm:
		case format::x8b8g8r8_unorm: return format::a8b8g8r8_unorm;
		case format::a8b8g8r8_unorm_srgb:
		case format::x8b8g8r8_unorm_srgb: return format::a8b8g8r8_unorm_srgb;
		default: break;
	}
	return format::unknown;
}

format as_noax(const format& f)
{
	switch (f)
	{
		case format::r8g8b8_unorm:
		case format::r8g8b8a8_unorm:
		case format::r8g8b8x8_unorm: return format::r8g8b8_unorm;
		case format::r8g8b8_unorm_srgb:
		case format::r8g8b8a8_unorm_srgb:
		case format::r8g8b8x8_unorm_srgb: return format::r8g8b8_unorm_srgb;
		case format::a8b8g8r8_unorm:
		case format::x8b8g8r8_unorm:
		case format::b8g8r8_unorm:
		case format::b8g8r8a8_unorm:
		case format::b8g8r8x8_unorm: return format::b8g8r8_unorm;
		case format::b8g8r8_unorm_srgb:
		case format::b8g8r8a8_unorm_srgb:
		case format::b8g8r8x8_unorm_srgb:;
		case format::a8b8g8r8_unorm_srgb:
		case format::x8b8g8r8_unorm_srgb: return format::b8g8r8_unorm_srgb;
		default: break;
	}
	return format::unknown;
}

static const format_info& finf(const format& f)
{
	return sFormatInfo[(int) (f < format::count ? f : format::unknown)];
}

static bool has_trait(const format& f, uint traits)
{
	return !!(finf(f).traits & traits);
}

bool is_block_compressed(const format& f)
{
	return has_trait(f, traits::is_bc);
}

bool is_depth(const format& f)
{
	return has_trait(f, traits::is_depth);
}

bool has_alpha(const format& f)
{
	return has_trait(f, traits::has_alpha);
}

bool is_unorm(const format& f)
{
	return has_trait(f, traits::is_unorm);
}

bool is_srgb(const format& f)
{
	return has_trait(f, traits::is_srgb);
}

bool is_planar(const format& f)
{
	return has_trait(f, traits::is_planar);
}

bool is_yuv(const format& f)
{
	return has_trait(f, traits::is_yuv);
}

uint num_channels(const format& f)
{
	return finf(f).num_channels;
}

uint num_subformats(const format& f)
{
	return finf(f).num_subformats;
}

uint subsample_bias(const format& f, uint subsurface)
{
	const auto& i = finf(f);
	if (subsurface == 1 && i.traits & traits::subsurface1_bias1) return 1;
	if (subsurface == 2 && i.traits & traits::subsurface2_bias1) return 1;
	if (subsurface == 3 && i.traits & traits::subsurface3_bias1) return 1;
	return 0;
}

uint element_size(const format& f, uint subsurface)
{
	return subsurface ? element_size(finf(f).subformats.format[subsurface]) : finf(f).element_size;
}

uint2 min_dimensions(const format& f)
{
	if (is_block_compressed(f)) return kMinMipBC;
	else if (is_yuv(f)) return kMinMipYUV;
	return kMinMip;
}

uint bits(const format& f)
{
	if (f == format::r1_unorm) return 1;
	return 8 * element_size(f);
}

bit_size channel_bits(const format& f)
{
	return finf(f).bit_size;
}

format subformat(const format& f, uint subsurface)
{
	const auto& i = finf(f);
	if (!!(i.traits & traits::is_yuv))
	{
		if (i.num_subformats < subsurface)
			return format::unknown;
		return i.subformats.format[subsurface];
	}
	return f;
}

fourcc to_fourcc(const format& f)
{
	return finf(f).fourcc;
}

format from_fourcc(const fourcc& fcc)
{
	oFORI(i, sFormatInfo)
	{
		if (fcc == sFormatInfo[i].fourcc)
			return format(i);
	}
	return format::unknown;
}

format closest_nv12(const format& f)
{
	if (num_subformats(f) == 2) // already nv12
		return f;
	if (num_subformats(f) == 3) // no alpha
		return (is_block_compressed(f)) ? format::ybc4_uvbc5_unorm : format::y8_u8v8_unorm;
	if (is_block_compressed(f)) // alpha
		return format::yabc5_uvbc5_unorm;
	return format::y8a8_u8v8_unorm;
}

uint num_mips(bool mips, const uint3& mip0dimensions)
{
	// Rules of mips are to go to 1x1... so a 1024x8 texture has many more than 4
	// mips.

	if (!mips)
		return 0;

	uint nMips = 1;
	uint3 mip = max(uint3(1), mip0dimensions);

	while (any(mip != 1))
	{
		nMips++;
		mip = max(uint3(1), mip / uint3(2));
	}

	return nMips;
}

uint dimension(const format& f, uint mip0dimension, uint miplevel, uint subsurface)
{
	oCHECK_DIM(f, mip0dimension);
	oCHECK_ARG(f != format::unknown, "Unknown surface format passed to surface::dimension");
	const uint subsampleBias = subsample_bias(f, subsurface);
	uint d = ::max(1u, mip0dimension >> (miplevel + subsampleBias));
	return is_block_compressed(f) ? static_cast<uint>(byte_align(d, 4)) : d;
}

uint2 dimensions(const format& f, const uint2& mip0dimensions, uint miplevel, uint subsurface)
{
	return uint2(dimension(f, mip0dimensions.x, miplevel, subsurface)
						, dimension(f, mip0dimensions.y, miplevel, subsurface));
}

uint3 dimensions(const format& f, const uint3& mip0dimensions, uint miplevel, uint subsurface)
{
	return uint3(dimension(f, mip0dimensions.x, miplevel, subsurface)
						, dimension(f, mip0dimensions.y, miplevel, subsurface)
						, dimension(format::r32_uint, mip0dimensions.z, miplevel, subsurface)); // no block-compression alignment for depth
}

uint dimension_npot(const format& f, uint mip0dimension, uint miplevel, uint subsurface)
{
	oCHECK_DIM(f, mip0dimension);

	format NthSurfaceFormat = subformat(f, subsurface);
	const auto MipLevelBias = subsample_bias(f, subsurface);
	const auto MinDimension = min_dimensions(f);

	auto d = ::max(1u, mip0dimension >> (miplevel + MipLevelBias));
	auto NPOTDim = is_block_compressed(NthSurfaceFormat) ? static_cast<uint>(byte_align(d, 4)) : d;

	if (subsurface == 0 && subsample_bias(f, 1) != 0)
		NPOTDim = ::max(MinDimension.x, NPOTDim & ~(MinDimension.x-1)); // always even down to 2x2

	return NPOTDim;
}

uint2 dimensions_npot(const format& f, const uint2& mip0dimensions, uint miplevel, uint subsurface)
{
	oCHECK_DIM2(f, mip0dimensions);

	format NthSurfaceFormat = subformat(f, subsurface);
	const auto MipLevelBias = subsample_bias(f, subsurface);
	const auto MinDimensions = min_dimensions(f);

	uint2 d;
	d.x = ::max(1u, mip0dimensions.x >> (miplevel + MipLevelBias));
	d.y = ::max(1u, mip0dimensions.y >> (miplevel + MipLevelBias));

	if (is_block_compressed(NthSurfaceFormat))
	{
		d.x = byte_align(d.x, 4);
		d.y = byte_align(d.y, 4);
	}

	if (subsurface == 0 && subsample_bias(f, 1) != 0)
	{
		d.x = ::max(MinDimensions.x, d.x & ~(MinDimensions.x-1)); // always even down to 2x2
		d.y = ::max(MinDimensions.y, d.y & ~(MinDimensions.y-1)); // always even down to 2x2
	}
	return d;
}

uint3 dimensions_npot(const format& f, const uint3& mip0dimensions, uint miplevel, uint subsurface)
{
	auto dimxy = dimensions_npot(f, mip0dimensions.xy(), miplevel, subsurface);
	return uint3(dimxy.x, dimxy.y
		, dimension_npot(format::r32_uint, mip0dimensions.z, miplevel, subsurface)); // no block-compression alignment for depth
}

uint row_size(const format& f, uint mipwidth, uint subsurface)
{
	oCHECK_DIM(f, mipwidth);
	uint w = dimension(f, mipwidth, 0, subsurface);
	if (is_block_compressed(f)) // because the atom is a 4x4 block
		w /= 4;
	const auto s = element_size(f, subsurface);
	return byte_align(w * s, s);
}

uint row_pitch(const info& inf, uint miplevel, uint subsurface)
{
	const auto nMips = num_mips(inf.mip_layout, inf.dimensions);
	if (nMips && miplevel >= nMips)
		throw invalid_argument("invalid miplevel");

	switch (inf.mip_layout)
	{
		case mip_layout::none: 
			return row_size(inf.format, inf.dimensions, subsurface);
		case mip_layout::tight: 
			return row_size(inf.format, dimension_npot(inf.format, inf.dimensions.x, miplevel, subsurface), subsurface);
		case mip_layout::below: 
		{
			const auto mip0RowSize = row_size(inf.format, inf.dimensions.x, subsurface);
			if (nMips > 2)
			{
					return ::max(mip0RowSize, 
					row_size(inf.format, dimension_npot(inf.format, inf.dimensions.x, 1, subsurface)) + 
					row_size(inf.format, dimension_npot(inf.format, inf.dimensions.x, 2, subsurface)) );
			}
			else
				return mip0RowSize;
		}

		case mip_layout::right: 
		{
			const auto mip0RowSize = row_size(inf.format, inf.dimensions.x, subsurface);
			if (nMips > 1)
				return mip0RowSize + row_size(inf.format, dimension_npot(inf.format, inf.dimensions.x, 1, subsurface), subsurface);
			else
				return mip0RowSize;
		}

		oNODEFAULT;
	}
}

uint depth_pitch(const info& inf, uint miplevel, uint subsurface)
{
	auto mipDimensions = dimensions_npot(inf.format, inf.dimensions, miplevel, 0);
	return row_pitch(inf, miplevel, subsurface) * num_rows(inf.format, mipDimensions.xy(), subsurface);
}

uint num_columns(const format& f, uint mipwidth, uint subsurface)
{
	oCHECK_DIM(f, mipwidth);
	auto widthInPixels = dimension(f, mipwidth, subsurface);
	return is_block_compressed(f) ? ::max(1u, widthInPixels/4) : widthInPixels;
}

uint num_rows(const format& f, uint mipheight, uint subsurface)
{
	oCHECK_DIM(f, mipheight);
	auto heightInPixels = dimension_npot(f, mipheight, 0, subsurface);
	return is_block_compressed(f) ? ::max(1u, heightInPixels/4) : heightInPixels;
}

uint mip_size(const format& f, const uint2& mipdimensions, uint subsurface)
{
	oCHECK_DIM2(f, mipdimensions);
	return row_size(f, mipdimensions, subsurface) * num_rows(f, mipdimensions, subsurface);
}

static int offset_none(const info& inf, uint miplevel, uint subsurface)
{
	oCHECK(miplevel == 0, "mip_layout::none doesn't have mip levels");
	uint offset = 0;
	for (uint i = 0; i < subsurface; i++)
		offset += total_size(inf, i);
	return offset;
}

static uint offset_tight(const info& inf, uint miplevel, uint subsurface)
{
	auto mip0dimensions = inf.dimensions;
	uint offset = 0;
	uint mip = 0;
	while (mip != miplevel)
	{
		auto previousMipDimensions = dimensions_npot(inf.format, mip0dimensions, mip, subsurface);
		offset += mip_size(inf.format, previousMipDimensions, subsurface);
		mip++;
	}
	return offset;
}

static int offset_below(const info& inf, uint miplevel, uint subsurface)
{
	if (0 == miplevel)
		return 0;

	auto mip0dimensions = inf.dimensions;
	auto mip1dimensions = dimensions_npot(inf.format, inf.dimensions, 1, subsurface);
	auto surfaceRowPitch = row_pitch(inf, 0, subsurface);

	// Step down when moving from Mip0 to Mip1
	auto offset = surfaceRowPitch * num_rows(inf.format, mip0dimensions, subsurface);
	if (1 == miplevel)
		return offset;

	// Step right when moving from Mip1 to Mip2
	offset += row_size(inf.format, mip1dimensions, subsurface);

	// Step down for all of the other MIPs
	uint mip = 2;
	while (mip != miplevel)
	{
		auto previousMipDimensions = dimensions_npot(inf.format, mip0dimensions, mip, subsurface);
		offset += surfaceRowPitch * num_rows(inf.format, previousMipDimensions, subsurface);
		mip++;
	}		

	return offset;
}

static uint offset_right(const info& inf, uint miplevel, uint subsurface)
{
	if (0 == miplevel)
		return 0;

	auto mip0dimensions = inf.dimensions;
	auto surfaceRowPitch = row_pitch(inf, 0, subsurface);

	// Step right when moving from Mip0 to Mip1
	auto offset = row_size(inf.format, mip0dimensions, subsurface);

	// Step down for all of the other MIPs
	uint mip = 1;
	while (mip != miplevel)
	{
		auto previousMipDimensions = dimensions_npot(inf.format, mip0dimensions, mip, subsurface);
		offset += surfaceRowPitch * num_rows(inf.format, previousMipDimensions, subsurface);
		mip++;
	}		

	return offset;
}

uint offset(const info& inf, uint miplevel, uint subsurface)
{
	const auto nMips = num_mips(inf.mip_layout, inf.dimensions);
	if (nMips && miplevel >= nMips) 
		throw invalid_argument("invalid miplevel");

	switch (inf.mip_layout)
	{
		case mip_layout::none: return offset_none(inf, miplevel, subsurface);
		case mip_layout::tight: return offset_tight(inf, miplevel, subsurface);
		case mip_layout::below: return offset_below(inf, miplevel, subsurface);
		case mip_layout::right: return offset_right(inf, miplevel, subsurface);
		oNODEFAULT;
	}
}

uint slice_pitch(const info& inf, uint subsurface)
{
	uint pitch = 0;

	switch (inf.mip_layout)
	{
		case mip_layout::none: case mip_layout::right: case mip_layout::below:
			return mip_size(inf.format, slice_dimensions(inf, 0), subsurface);

		case mip_layout::tight:
		{
			// Sum the size of all mip levels
			int3 dimensions = inf.dimensions;
			int nMips = num_mips(inf.mip_layout, dimensions);
			while (nMips > 0)
			{
				pitch += mip_size(inf.format, dimensions.xy(), subsurface) * dimensions.z;
				dimensions = ::max(int3(1,1,1), dimensions / int3(2,2,2));
				nMips--;
			}

			// Align slicePitch to mip0RowPitch
			const uint mip0RowPitch = row_pitch(inf, 0, subsurface);
			pitch = (((pitch + (mip0RowPitch - 1)) / mip0RowPitch) * mip0RowPitch);
			break;
		}
		oNODEFAULT;
	}

	return pitch;
}

uint total_size(const info& inf)
{
	uint size = 0;
	const int nSurfaces = num_subformats(inf.format);
	for (int i = 0; i < nSurfaces; i++)
	{
		// byte_align is needed here to avoid a memory corruption crash. I'm not 
		// sure why it is needed, but I think that size is a memory structure 
		// containing all surface sizes, so they are all expected to be aligned.
		size += total_size(inf, i);
	}
	return size;
}

uint total_size(const info& inf, uint subsurface)
{
	return slice_pitch(inf, subsurface) * safe_array_size(inf);
}

uint2 dimensions(const info& inf, uint subsurface)
{
	auto sliceDimensions = slice_dimensions(inf, subsurface);
	return uint2(sliceDimensions.x, sliceDimensions.y * safe_array_size(inf));
}

uint2 slice_dimensions(const info& inf, uint subsurface)
{
	auto mip0dimensions = dimensions_npot(inf.format, inf.dimensions, 0, subsurface);
	switch (inf.mip_layout)
	{
		case mip_layout::none:
			return uint2(mip0dimensions.x, (mip0dimensions.y * mip0dimensions.z));
		
		case mip_layout::tight:
		{
			const auto surfaceSlicePitch = slice_pitch(inf, subsurface);
			const auto mip0RowPitch = row_pitch(inf, 0, subsurface);
			return uint2(mip0dimensions.x, (surfaceSlicePitch / mip0RowPitch));
		}
		case mip_layout::below: 
		{
			auto nMips = num_mips(inf.mip_layout, mip0dimensions);
			auto mip1dimensions = nMips > 1 ? dimensions_npot(inf.format, mip0dimensions, 1, subsurface) : uint3(0);
			auto mip2dimensions = nMips > 2 ? dimensions_npot(inf.format, mip0dimensions, 2, subsurface) : uint3(0);

			auto mip0height = mip0dimensions.y * mip0dimensions.z;
			auto mip1height = mip1dimensions.y * mip1dimensions.z;
			auto mip2andUpHeight = mip2dimensions.y * mip2dimensions.z;
			for (uint mip = 3; mip < nMips; mip++)
			{
				auto mipNdimensions = dimensions_npot(inf.format, mip0dimensions, mip, subsurface);
				mip2andUpHeight += mipNdimensions.y * mipNdimensions.z;
			}
			return uint2(::max(mip0dimensions.x, mip1dimensions.x + mip2dimensions.x), (mip0height + ::max(mip1height, mip2andUpHeight)));
		}
		case mip_layout::right: 
		{
			auto nMips = num_mips(inf.mip_layout, mip0dimensions);
			auto mip1dimensions = nMips > 1 ? dimensions_npot(inf.format, mip0dimensions, 1, subsurface) : int3(0);

			auto mip0height = mip0dimensions.y * mip0dimensions.z;
			auto mip1andUpHeight = mip1dimensions.y * mip1dimensions.z;
			for (uint mip = 2; mip < nMips; mip++)
			{
				int3 mipNdimensions = dimensions_npot(inf.format, mip0dimensions, mip, subsurface);
				mip1andUpHeight += mipNdimensions.y * mipNdimensions.z;
			}
			return int2(mip0dimensions.x + mip1dimensions.x, ::max(mip0height, mip1andUpHeight));
		}
		oNODEFAULT;
	}
}

subresource_info subresource(const info& inf, uint subresource)
{
	subresource_info subinf;
	int nMips = num_mips(inf.mip_layout, inf.dimensions);
	unpack_subresource(subresource, nMips, inf.array_size, &subinf.mip_level, &subinf.array_slice, &subinf.subsurface);
	if (inf.array_size && subinf.array_slice >= safe_array_size(inf)) throw invalid_argument("array slice index is out of range");
	if (subinf.subsurface >= num_subformats(inf.format)) throw invalid_argument("subsurface index is out of range for the specified surface");
	subinf.dimensions = dimensions_npot(inf.format, inf.dimensions, subinf.mip_level, subinf.subsurface);
	subinf.format = inf.format;
	return subinf;
}

info subsurface(const info& inf, uint subsurface, uint miplevel, uint2* out_byte_dimensions)
{
	info subinf;
	subinf.dimensions = dimensions_npot(inf.format, inf.dimensions, miplevel, subsurface);
	subinf.array_size = inf.array_size;
	subinf.format = subformat(inf.format, subsurface);
	subinf.mip_layout = inf.mip_layout;
	if (out_byte_dimensions)
	{
		out_byte_dimensions->x = row_size(inf.format, inf.dimensions);
		out_byte_dimensions->y = num_rows(inf.format, inf.dimensions);
	}
	return inf;
}

uint subresource_size(const subresource_info& subresource_info)
{
	return mip_size(subresource_info.format, subresource_info.dimensions, subresource_info.subsurface);
}

uint subresource_offset(const info& inf, uint subresource, uint depth)
{
	oASSERT(depth < inf.dimensions.z, "Depth index is out of range");
	subresource_info subinf = surface::subresource(inf, subresource);
	uint off = offset(inf, subinf.mip_level, subinf.subsurface);
	if (depth)
		off += depth_pitch(inf, subinf.mip_level, subinf.subsurface) * depth;
	else if (subinf.array_slice > 0)
		off += slice_pitch(inf, subinf.subsurface) * subinf.array_slice;
	return off;
}

const_mapped_subresource get_const_mapped_subresource(const info& inf, uint subresource, uint depth, const void* surface_bytes, uint2* out_byte_dimensions)
{
	subresource_info subinf = surface::subresource(inf, subresource);

	const_mapped_subresource cmapped;
	cmapped.row_pitch = row_pitch(inf, subinf.mip_level, subinf.subsurface);
	cmapped.depth_pitch = depth_pitch(inf, subinf.mip_level, subinf.subsurface);
	cmapped.data = byte_add(surface_bytes, subresource_offset(inf, subresource, depth));

	if (out_byte_dimensions)
		*out_byte_dimensions = byte_dimensions(inf.format, subinf.dimensions.xy(), subinf.subsurface);
	return cmapped;
}

mapped_subresource get_mapped_subresource(const info& inf, uint subresource, uint depth, void* surface_bytes, uint2* out_byte_dimensions)
{
	const_mapped_subresource msr = get_const_mapped_subresource(inf, subresource, depth, surface_bytes, out_byte_dimensions);
	return (mapped_subresource&)msr;
}

void update(const info& inf, uint subresource, uint depth, void* dst_surface, const void* src_surface, uint src_row_pitch, const copy_option& option)
{
	uint2 ByteDimensions;
	mapped_subresource msr = get_mapped_subresource(inf, subresource, depth, dst_surface, &ByteDimensions);
	memcpy2d(msr.data, msr.row_pitch, src_surface, src_row_pitch, ByteDimensions.x, ByteDimensions.y, option == copy_option::flip_vertically);
}

void copy(const info& inf, uint subresource, uint depth, const void* src_surface, void* dst_surface, uint dst_row_pitch, const copy_option& option)
{
	uint2 ByteDimensions;
	const_mapped_subresource msr = get_const_mapped_subresource(inf, subresource, depth, src_surface, &ByteDimensions);
	memcpy2d(dst_surface, dst_row_pitch, msr.data, msr.row_pitch, ByteDimensions.x, ByteDimensions.y, option == copy_option::flip_vertically);
}

void copy(const info& inf, const const_mapped_subresource& _Source, const mapped_subresource& dst, const copy_option& option)
{
	memcpy2d(dst.data, dst.row_pitch, _Source.data, _Source.row_pitch, inf.dimensions.x*element_size(inf.format), inf.dimensions.y, option == copy_option::flip_vertically);
}

void copy(const subresource_info& subresource_info, const const_mapped_subresource& _Source, const mapped_subresource& dst, const copy_option& option)
{
	memcpy2d(dst.data, dst.row_pitch, _Source.data, _Source.row_pitch, subresource_info.dimensions.x*element_size(subresource_info.format), subresource_info.dimensions.y, option == copy_option::flip_vertically);
}

void put(const subresource_info& subresource_info, const mapped_subresource& dst, const uint2& _Coordinate, color _Color)
{
	const int elSize = element_size(format(subresource_info.format));
	uchar* p = (uchar*)dst.data + (_Coordinate.y * dst.row_pitch) + (_Coordinate.x * elSize);
	int rr, gg, bb, aa;
	_Color.decompose(&rr, &gg, &bb, &aa);
	uchar r = (uchar)rr, g = (uchar)gg, b = (uchar)bb, a = (uchar)aa;
	switch (subresource_info.format)
	{
		case format::r8g8b8a8_unorm: *p++ = r; *p++ = g; *p++ = b; *p++ = a; break;
		case format::r8g8b8_unorm: *p++ = r; *p++ = g; *p++ = b; break;
		case format::b8g8r8a8_unorm: *p++ = b; *p++ = g; *p++ = r; *p++ = a; break;
		case format::b8g8r8_unorm: *p++ = b; *p++ = g; *p++ = r; break;
		case format::r8_unorm: *p++ = r; break;
		default: throw invalid_argument("unsupported format");
	}
}

color get(const subresource_info& subresource_info, const const_mapped_subresource& _Source, const uint2& _Coordinate)
{
	const int elSize = element_size(subresource_info.format);
	const uchar* p = (const uchar*)_Source.data + (_Coordinate.y * _Source.row_pitch) + (_Coordinate.x * elSize);
	int r=0, g=0, b=0, a=255;
	switch (subresource_info.format)
	{
		case format::r8g8b8a8_unorm: r = *p++; g = *p++; b = *p++; a = *p++; break;
		case format::r8g8b8_unorm: r = *p++; g = *p++; b = *p++; break;
		case format::b8g8r8a8_unorm: b = *p++; g = *p++; r = *p++; a = *p++; break;
		case format::b8g8r8_unorm: b = *p++; g = *p++; r = *p++; break;
		case format::r8_unorm: r = *p++; g=r; b=r; break;
		default: break;
	}
	
	return color(r,g,b,a);
}

// Returns the nth mip where the mip level best-fits into a tile. i.e. several
// tiles are no longer required to store all the mip data, and all mip levels
// after the one returned by this function can together fit into one mip level.
static uint best_fit_mip_level(const info& inf, const uint2& tiledimensions)
{
	if (mip_layout::none == inf.mip_layout)
		return 0;

	uint nthMip = 0;
	auto mip = inf.dimensions;
	while (any(mip != uint3(1)))
	{
		if (all(inf.dimensions.xy() <= tiledimensions))
			break;

		nthMip++;
		mip = ::max(uint3(1), mip / uint3(2));
	}

	return nthMip;
}

uint num_slice_tiles(const info& inf, const uint2& tiledimensions)
{
	oCHECK_NOT_PLANAR(inf.format);

	if (mip_layout::none == inf.mip_layout)
		return num_tiles(inf.dimensions, tiledimensions);

	uint numTiles = 0;
	uint lastMip = 1 + best_fit_mip_level(inf, tiledimensions);
	for (uint i = 0; i <= lastMip; i++)
	{
		auto mipDim = dimensions(inf.format, inf.dimensions, i);
		numTiles += num_tiles(mipDim, tiledimensions);
	}

	return numTiles;
}

uint dimension_in_tiles(int _MipDimension, int _TileDimension)
{
	uint div = _MipDimension / _TileDimension;
	if (0 != (_MipDimension % _TileDimension))
		div++;
	return div;
}

uint2 dimensions_in_tiles(const uint2& mipdimensions, const uint2& tiledimensions)
{
	return uint2(dimension_in_tiles(mipdimensions.x, tiledimensions.x)
		, dimension_in_tiles(mipdimensions.y, tiledimensions.y));
}

uint3 dimensions_in_tiles(const uint3& mipdimensions, const uint2& tiledimensions)
{
	return uint3(dimension_in_tiles(mipdimensions.x, tiledimensions.x)
		, dimension_in_tiles(mipdimensions.y, tiledimensions.y)
		, mipdimensions.z);
}

uint num_tiles(const uint2& mipdimensions, const uint2& tiledimensions)
{
	auto mipDimInTiles = dimensions_in_tiles(mipdimensions, tiledimensions);
	return mipDimInTiles.x * mipDimInTiles.y;
}

uint num_tiles(const uint3& mipdimensions, const uint2& tiledimensions)
{
	auto mipDimInTiles = dimensions_in_tiles(mipdimensions, tiledimensions);
	return mipDimInTiles.x * mipDimInTiles.y;
}

// calculate the tile id of the first tile in the specified slice
static uint slice_first_tile_id(const info& inf, const uint2& tiledimensions, uint slice)
{
	oCHECK_NOT_PLANAR(inf.format);
	auto numTilesPerSlice = num_slice_tiles(inf, tiledimensions);
	return slice * numTilesPerSlice;
}

// how many tiles from the startID to start the specified mip
static uint tile_id_offset(const info& inf, const uint2& tiledimensions, uint miplevel)
{
	oCHECK_NOT_PLANAR(inf.format);
	if (mip_layout::none == inf.mip_layout)
		return 0;
	uint numTiles = 0;
	uint nMips = ::min(miplevel, 1u + best_fit_mip_level(inf, tiledimensions));
	for (uint i = 0; i < nMips; i++)
	{
		auto mipDim = dimensions(inf.format, inf.dimensions, i);
		numTiles += num_tiles(mipDim, tiledimensions);
	}

	return numTiles;
}

static uint mip_first_tile_id(const info& inf, const uint2& tiledimensions, uint miplevel, uint slice)
{
	oCHECK_NOT_PLANAR(inf.format);
	auto sliceStartID = slice_first_tile_id(inf, tiledimensions, slice);
	auto mipIDOffset = tile_id_offset(inf, tiledimensions, miplevel);
	return sliceStartID + mipIDOffset;
}

bool use_large_pages(const info& inf, const uint2& tiledimensions, uint small_page_size_bytes, uint large_page_size_bytes)
{
	oCHECK_NOT_PLANAR(inf.format);
	auto surfaceSize = mip_size(inf.format, inf.dimensions);
	if (surfaceSize < (large_page_size_bytes / 4))
		return false;

	auto surfacePitch = row_pitch(inf);

	// number of rows before we get a page miss
	float numRowsPerPage = small_page_size_bytes / static_cast<float>(surfacePitch);

	auto tileByteWidth = row_size(inf.format, tiledimensions);
	
	// estimate how many bytes we would work on in a tile before encountering a 
	// tlb cache miss... not precise, but should be close enough for our purpose 
	// here. 
	float numBytesPerTLBMiss = tileByteWidth * numRowsPerPage - numeric_limits<float>::epsilon(); 
	
	// If we are not going to get at least half a small page size of work done per tlb miss, better to use large pages instead.
	return numBytesPerTLBMiss <= (small_page_size_bytes / 2);
}

uint calc_tile_id(const info& inf, const tile_info& _TileInfo, uint2* out_position)
{
	oCHECK_NOT_PLANAR(inf.format);
	int mipStartTileID = mip_first_tile_id(inf, _TileInfo.dimensions, _TileInfo.mip_level, _TileInfo.array_slice);
	int2 PositionInTiles = _TileInfo.position / _TileInfo.dimensions;
	int2 mipDim = dimensions(inf.format, inf.dimensions.xy(), _TileInfo.mip_level);
	int2 mipDimInTiles = dimensions_in_tiles(mipDim, _TileInfo.dimensions);
	int tileID = mipStartTileID + (mipDimInTiles.x * PositionInTiles.y) + PositionInTiles.x;
	if (out_position)
		*out_position = PositionInTiles * _TileInfo.dimensions;
	return tileID;
}

tile_info get_tile(const info& inf, const uint2& tiledimensions, uint tileid)
{
	tile_info tileinf;

	oCHECK_NOT_PLANAR(inf.format);
	auto numTilesPerSlice = num_slice_tiles(inf, tiledimensions);
	tileinf.dimensions = tiledimensions;
	tileinf.array_slice = tileid / numTilesPerSlice;
	if (::max(1u, tileinf.array_slice) >= safe_array_size(inf))
		throw invalid_argument("TileID is out of range for the specified mip dimensions");

	uint firstTileInMip = 0;
	auto mipDim = inf.dimensions;
	tileinf.mip_level = 0;
	uint nthTileIntoSlice = tileid % numTilesPerSlice; 

	if (nthTileIntoSlice > 0)
	{
		do 
		{
			mipDim = dimensions(inf.format, inf.dimensions, ++tileinf.mip_level);
			firstTileInMip += num_tiles(mipDim, tiledimensions);

		} while (nthTileIntoSlice < firstTileInMip);
	}
	
	auto tileOffsetFromMipStart = nthTileIntoSlice - firstTileInMip;
	auto mipDimInTiles = dimensions_in_tiles(mipDim, tiledimensions);
	auto positionInTiles = uint2(tileOffsetFromMipStart % mipDimInTiles.x, tileOffsetFromMipStart / mipDimInTiles.y);
	tileinf.position = positionInTiles * tiledimensions;
	return tileinf;
}

void enumerate_pixels(const info& inf
	, const const_mapped_subresource& mapped
	, const function<void(const void* _pPixel)>& enumerator)
{
	const void* pRow = mapped.data;
	const void* pEnd = byte_add(pRow, inf.dimensions.y * mapped.row_pitch); // should this be depth/slice pitch?
	const int FormatSize = element_size(inf.format);
	for (; pRow < pEnd; pRow = byte_add(pRow, mapped.row_pitch))
	{
		const void* pPixel = pRow;
		const void* pRowEnd = byte_add(pPixel, inf.dimensions.x * FormatSize);
		for (; pPixel < pRowEnd; pPixel = byte_add(pPixel, FormatSize))
			enumerator(pPixel);
	}
}

void enumerate_pixels(const info& inf
	, const mapped_subresource& mapped
	, const function<void(void* _pPixel)>& enumerator)
{
	void* pRow = mapped.data;
	void* pEnd = byte_add(pRow, inf.dimensions.y * mapped.row_pitch); // should this be depth/slice pitch?
	const int FormatSize = element_size(inf.format);
	for (; pRow < pEnd; pRow = byte_add(pRow, mapped.row_pitch))
	{
		void* pPixel = pRow;
		void* pRowEnd = byte_add(pPixel, inf.dimensions.x * FormatSize);
		for (; pPixel < pRowEnd; pPixel = byte_add(pPixel, FormatSize))
			enumerator(pPixel);
	}
}

// Calls the specified function on each pixel of two same-formatted images.
void enumerate_pixels(const info& inf
	, const const_mapped_subresource& mapped1
	, const const_mapped_subresource& mapped2
	, const function<void(const void* oRESTRICT _pPixel1, const void* oRESTRICT _pPixel2)>& enumerator)
{
	const void* pRow1 = mapped1.data;
	const void* pRow2 = mapped2.data;
	const void* pEnd1 = byte_add(pRow1, inf.dimensions.y * mapped1.row_pitch);
	const int FormatSize = element_size(inf.format);
	while (pRow1 < pEnd1)
	{
		const void* pPixel1 = pRow1;
		const void* pPixel2 = pRow2;
		const void* pRowEnd1 = byte_add(pPixel1, inf.dimensions.x * FormatSize);
		while (pPixel1 < pRowEnd1)
		{
			enumerator(pPixel1, pPixel2);
			pPixel1 = byte_add(pPixel1, FormatSize);
			pPixel2 = byte_add(pPixel2, FormatSize);
		}

		pRow1 = byte_add(pRow1, mapped1.row_pitch);
		pRow2 = byte_add(pRow2, mapped2.row_pitch);
	}
}

void enumerate_pixels(const info& _SurfaceInfoInput
	, const const_mapped_subresource& mappedInput1
	, const const_mapped_subresource& mappedInput2
	, const info& _SurfaceInfoOutput
	, mapped_subresource& mappedOutput
	, const function<void(const void* oRESTRICT _pPixel1, const void* oRESTRICT _pPixel2, void* oRESTRICT _pPixelOut)>& enumerator)
{
	if (any(_SurfaceInfoInput.dimensions != _SurfaceInfoOutput.dimensions))
		throw invalid_argument(formatf("Dimensions mismatch In(%dx%d) != Out(%dx%d)"
			, _SurfaceInfoInput.dimensions.x
			, _SurfaceInfoInput.dimensions.y
			, _SurfaceInfoOutput.dimensions.x
			, _SurfaceInfoOutput.dimensions.y));
	
	const void* pRow1 = mappedInput1.data;
	const void* pRow2 = mappedInput2.data;
	const void* pEnd1 = byte_add(pRow1, _SurfaceInfoInput.dimensions.y * mappedInput1.row_pitch);
	void* pRowOut = mappedOutput.data;
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
			enumerator(pPixel1, pPixel2, pOutPixel);
			pPixel1 = byte_add(pPixel1, InputFormatSize);
			pPixel2 = byte_add(pPixel2, InputFormatSize);
			pOutPixel = byte_add(pOutPixel, OutputFormatSize);
		}

		pRow1 = byte_add(pRow1, mappedInput1.row_pitch);
		pRow2 = byte_add(pRow2, mappedInput2.row_pitch);
		pRowOut = byte_add(pRowOut, mappedOutput.row_pitch);
	}
}

typedef void (*rms_enumerator)(const void* oRESTRICT _pPixel1, const void* oRESTRICT _pPixel2, void* oRESTRICT _pPixelOut, atomic<uint>* _pAccum);

static void sum_squared_diff_r8_to_r8(const void* oRESTRICT _pPixel1, const void* oRESTRICT _pPixel2, void* oRESTRICT _pPixelOut, atomic<uint>* _pAccum)
{
	const uchar* p1 = (const uchar*)_pPixel1;
	const uchar* p2 = (const uchar*)_pPixel2;
	uchar absDiff = uchar (abs(*p1 - *p2));
	*(uchar*)_pPixelOut = absDiff;
	_pAccum->fetch_add(uint(absDiff * absDiff));
}

static void sum_squared_diff_b8g8r8_to_r8(const void* oRESTRICT _pPixel1, const void* oRESTRICT _pPixel2, void* oRESTRICT _pPixelOut, atomic<uint>* _pAccum)
{
	const uchar* p = (const uchar*)_pPixel1;
	uchar b = *p++; uchar g = *p++; uchar r = *p++;
	float L1 = color(r, g, b, 255).luminance();
	p = (const uchar*)_pPixel2;
	b = *p++; g = *p++; r = *p++;
	float L2 = color(r, g, b, 255).luminance();
	uchar absDiff = f32ton8(abs(L1 - L2));
	*(uchar*)_pPixelOut = absDiff;
	_pAccum->fetch_add(uint(absDiff * absDiff));
}

static void sum_squared_diff_b8g8r8a8_to_r8(const void* oRESTRICT _pPixel1, const void* oRESTRICT _pPixel2, void* oRESTRICT _pPixelOut, atomic<uint>* _pAccum)
{
	const uchar* p = (const uchar*)_pPixel1;
	uchar b = *p++; uchar g = *p++; uchar r = *p++; uchar a = *p++;
	float L1 = color(r, g, b, a).luminance();
	p = (const uchar*)_pPixel2;
	b = *p++; g = *p++; r = *p++; a = *p++; 
	float L2 = color(r, g, b, a).luminance();
	uchar absDiff = f32ton8(abs(L1 - L2));
	*(uchar*)_pPixelOut = absDiff;
	_pAccum->fetch_add(uint(absDiff * absDiff));
}

static rms_enumerator get_rms_enumerator(format in_format, format out_format)
{
	#define IO(i,o) ((uint(i)<<16) | uint(o))
	uint req = IO(in_format, out_format);

	switch (req)
	{
		case IO(format::r8_unorm, format::r8_unorm): return sum_squared_diff_r8_to_r8;
		case IO(format::b8g8r8_unorm, format::r8_unorm): return sum_squared_diff_b8g8r8_to_r8;
		case IO(format::b8g8r8a8_unorm, format::r8_unorm): return sum_squared_diff_b8g8r8a8_to_r8;
		default: break;
	}

	throw invalid_argument(formatf("%s -> %s not supported", as_string(in_format), as_string(out_format)));
	#undef IO
}

float calc_rms(const info& inf
	, const const_mapped_subresource& mapped1
	, const const_mapped_subresource& mapped2)
{
	rms_enumerator en = get_rms_enumerator(inf.format, format::r8_unorm);
	atomic<uint> SumOfSquares(0);
	uint DummyPixelOut[4]; // largest a pixel can ever be currently

	enumerate_pixels(inf
		, mapped1
		, mapped2
		, bind(en, _1, _2, &DummyPixelOut, &SumOfSquares));

	return sqrt(SumOfSquares / float(inf.dimensions.x * inf.dimensions.y));
}

float calc_rms(const info& _SurfaceInfoInput
	, const const_mapped_subresource& mappedInput1
	, const const_mapped_subresource& mappedInput2
	, const info& _SurfaceInfoOutput
	, mapped_subresource& mappedOutput)
{
	function<void(const void* _pPixel1, const void* _pPixel2, void* _pPixelOut)> Fn;

	rms_enumerator en = get_rms_enumerator(_SurfaceInfoInput.format, _SurfaceInfoOutput.format);
	atomic<uint> SumOfSquares(0);

	enumerate_pixels(_SurfaceInfoInput
		, mappedInput1
		, mappedInput2
		, _SurfaceInfoOutput
		, mappedOutput
		, bind(en, _1, _2, _3, &SumOfSquares));

	return sqrt(SumOfSquares / float(_SurfaceInfoInput.dimensions.x * _SurfaceInfoInput.dimensions.y));
}

typedef void (*histogramenumerator)(const void* _pPixel, atomic<uint>* _Histogram);

static void histogram_r8_unorm_8bit(const void* _pPixel, atomic<uint>* _Histogram)
{
	uchar c = *(const uchar*)_pPixel;
	_Histogram[c]++;
}

static void histogram_b8g8r8a8_unorm_8bit(const void* _pPixel, atomic<uint>* _Histogram)
{
	const uchar* p = (const uchar*)_pPixel;
	uchar b = *p++; uchar g = *p++; uchar r = *p++;
	uchar Index = f32ton8(color(r, g, b, 255).luminance());
	_Histogram[Index]++;
}

static void histogram_r16_unorm_16bit(const void* _pPixel, atomic<uint>* _Histogram)
{
	unsigned short c = *(const unsigned short*)_pPixel;
	_Histogram[c]++;
}

static void histogram_r16_float_16bit(const void* _pPixel, atomic<uint>* _Histogram)
{
	half h = saturate(*(const half*)_pPixel);
	unsigned short c = static_cast<unsigned short>(round(65535.0f * h));
	_Histogram[c]++;
}

histogramenumerator get_histogramenumerator(const format& f, int _Bitdepth)
{
	#define IO(f,b) ((uint(f) << 16) | uint(b))
	uint sel = IO(f, _Bitdepth);
	switch (sel)
	{
		case IO(format::r8_unorm, 8): return histogram_r8_unorm_8bit;
		case IO(format::b8g8r8a8_unorm, 8): return histogram_b8g8r8a8_unorm_8bit;
		case IO(format::r16_unorm, 16): return histogram_r16_unorm_16bit;
		case IO(format::r16_float, 16): return histogram_r16_float_16bit;
		default: break;
	}

	throw invalid_argument(formatf("%dbit histogram on %s not supported", _Bitdepth, as_string(f)));

	#undef IO
}

void histogram8(const info& inf, const const_mapped_subresource& mapped, uint _Histogram[256])
{
	atomic<uint> H[256];
	memset(H, 0, sizeof(uint) * 256);
	histogramenumerator en = get_histogramenumerator(inf.format, 8);
	enumerate_pixels(inf, mapped, bind(en, _1, H));
	memcpy(_Histogram, H, sizeof(uint) * 256);
}

void histogram16(const info& inf, const const_mapped_subresource& mapped, uint _Histogram[65536])
{
	atomic<uint> H[65536];
	memset(H, 0, sizeof(uint) * 65536);
	histogramenumerator en = get_histogramenumerator(inf.format, 16);
	enumerate_pixels(inf, mapped, bind(en, _1, H));
	memcpy(_Histogram, H, sizeof(uint) * 65536);
}

	} // namespace surface
} // namespace ouro
