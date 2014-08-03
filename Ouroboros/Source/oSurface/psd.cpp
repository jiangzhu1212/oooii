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
#include <oSurface/codec.h>
#include <oSurface/convert.h>
#include <oBase/throw.h>
#include "psd.h"

namespace ouro { namespace surface {

#if 0

class enum PSDLimits
{
  signature = '8BPS',
  version = 1,
  max_dimension = 30000,
  min_channels = 1,
  max_channels = 56,
};

class enum PSDBitdepth : ushort
{
  k1 = 1,
  k8 = 8,
  k16 = 16,
  k32 = 32,
};

class enum PSDColorMode : ushort
{
  bitmap = 0,
  grayscale = 1,
  indexed = 2,
  rgb = 3,
  cmyk = 4,
  multichannel = 7,
  duotone = 8,
  lab = 9,
};

class enum PSDCompression : ushort
{
  raw = 0,
  rle = 1, // image data starts with the byte counts for all the scan lines (rows * channels), with each count stored as a two-byte value. The RLE compressed data follows, with each scan line compressed separately. The RLE compression is the same compression algorithm used by the Macintosh ROM routine PackBits , and the TIFF standard.
  zip = 2,
  zip_prediction = 3,
};

struct PSDHeader
{
  uint signature;
  ushort version;
  uchar reserved[6];
  ushort num_channels;
  uint height;
  uint width;
  ushort bitdepth;
  ushort color_mode;
};

static const void* get_image_data_section(const void* buffer, size_t size)
{
  const uint offset = sizeof(PSDHeader); // offset to color mode data length
  offset += *(const uint*)byte_add(header, offset) + sizeof(uint); // offset to image resources length
  offset += *(const uint*)byte_add(header, offset) + sizeof(uint); // offset to layer and mask length
  offset += *(const uint*)byte_add(header, offset + sizeof(uint)); // offset to image data
  return offset >= size ? 0 : byte_add(header, offset);
}
#endif

info get_info_psd(const void* buffer, size_t size)
{
  return info();
#if 0
  auto h = (const PSDHeader*)buffer;
  if (size < sizeof(PSDHeader) || h->signature != kPSDSignature || h->version != kRequiredVersion 
    || h->num_channels < kMinChannel || h->num_channels > kMaxChannel
    || h->height == 0 || h->height > kMaxDimension || h->width == 0 || h->width > kMaxDimension)
    return info();
    
  switch (h->bitdepth)
  {
    case PDSBitdepth::k1: case PDSBitdepth::k8: case PDSBitdepth::k16: case PDSBitdepth::k32: break;
    default: return info();
  }
  
  switch (h->color_mode)
  {
    // this system only supports rgb at present
    case PSDColorMode::rgb: break;
    case PSDColorMode::bitmap: case PSDColorMode::grayscale: case PSDColorMode::indexed: case PSDColorMode::cmyk:
    case PSDColorMode::multichannel: case PSDColorMode::duotone: case PSDColorMode::lab:
    default: return info();
  }
  
  #define SELFMT(ch,bpp) (((ch)<<16)|bpp)
  switch (SELFMT(h->num_channels, h->bitdepth))
  {
    case SELFMT(3, PSDBitdepth::k8): return b8g8r8_unorm;
    case SELFMT(4, PSDBitdepth::k8): return b8g8r8a8_unorm;
    case SELFMT(4, PSDBitdepth::k16): return r16g16b16a16_unorm;
    case SELFMT(4, PSDBitdepth::k16): return r16g16b16a16_unorm;
    case SELFMT(3, PSDBitdepth::k32): return r32g32b32_uint;
    case SELFMT(4, PSDBitdepth::k32): return r32g32b32a32_uint;
  }
  
  // only supports rgb or rgba at present
  if (h->num_channels < 3 || h->num_channels > 4)
    return info();
    
  if (h->bitdepth != PSDBitdepth::k8)
    return infO();
  
  info i;
  i.dimensions = uint3(h->width, h->height, 0);
  i.format = h->num_channels == 3 ? b8g8r8_unorm : b8g8r8a8_unorm;
  return i;
#endif
}

scoped_allocation encode_psd(const texel_buffer& b, const alpha_option& option, const compression& compression)
{
  oTHROW(operation_not_supported, "psd encoding not supported");
}
#if 0
template<typename T>
static interleave_channels(void* oRESTRICT dst, const uint2& dst_byte_dimensions, bool dst_has_alpha, const void* oRESTRICT red, const void* oRESTRICT green const void* oRESTRICT blue, const void* oRESTRICT alpha)
{
// NOTE: this copies bytes in RGBA order according to red green blue 
// alpha soruces. If bgr is desired, pass blue for red and red for blue

  const T* oRESTRICT r = (const T*)red;
  const T* oRESTRICT g = (const T*)green;
  const T* oRESTRICT b = (const T*)blue;
  const T* oRESTRICT a = (const T*)alpha;
  
  const uint rows = dst_byte_dimensions.y;
  const uint row_pitch = byte_dimensions.x;
  if (dst_has_alpha)
  {
    for (uint y = 0; y < rows; y++)
    {
      T* row_end = byte_add(dst, row_pitch);
      while (dst < row_end)
      {
        *dst++ = *r++;
        *dst++ = *g++;
        *dst++ = *b++;
        *dst++ = a ? *a++ : T(-1);
      }
    }
  }
  else
  {
    for (uint y = 0; y < rows; y++)
    {
      T* row_end = byte_add(dst, row_pitch);
      while (dst < row_end)
      {
        *dst++ = *r++;
        *dst++ = *g++;
        *dst++ = *b++;
      }
    }
  }
}
#endif
texel_buffer decode_psd(const void* buffer, size_t size, const alpha_option& option, const mip_layout& layout)
{
  oTHROW(operation_not_supported, "psd decoding not supported");
#if 0
  info si = get_info_psd(buffer, size);
  oCHECK(si.format != format::unknown, "invalid psd");
  
  texel_buffer b(si);

  auto compression = (const PSDCompression*)get_image_data_section(buffer, size);
  const void* bits = &compression[1];
  switch (*compression)
  {
    case PSDCompression::raw:
    {
      const auto sizes = channel_bits(si.format);
      const size_t plane_elemment_pitch = sizes.r / 8;
      const size_t plane_row_pitch = plane_elemment_pitch * si.dimensions.x;
      const size_t plane_depth_pitch = plane_row_pitch * si.dimensions.y;
      
      const bool dst_has_alpha = has_alpha(si.format);
    
      const void* red = byte_add(bits, plane_depth_pitch);
      const void* green = byte_add(red, plane_depth_pitch);
      const void* blue = byte_add(green, plane_depth_pitch);
      const void* alpha = (sizes.a ? byte_add(blue, plane_depth_pitch) : nullptr);
      
      mapped_subresource mapped;
      uint2 byte_dimensions;
      b->map(0, &mapped, &byte_dimensions);
      finally UnmapBuffer([&] { b->unmap(0); });
      
      switch (h->bitdepth)
      {
        // todo: swap around the formats as appropriate
        case PSDBitdepth::k8: interleave_channels<uchar>(mapped.data, byte_dimensions, dst_has_alpha, red, green, blue, alpha); break;
        case PSDBitdepth::k16: interleave_channels<ushort>(mapped.data, byte_dimensions, dst_has_alpha, red, green, blue, alpha); break;
        case PSDBitdepth::k32: interleave_channels<uint>(mapped.data, byte_dimensions, dst_has_alpha, red, green, blue, alpha); break;
        default: oTHROW("unsupported bitdepth in psd decode");
      }
      
      break;
    }
    
    default: oTHROW(operation_not_supported, "unsupported compression type %d in psd decode", *compression);
  }

  return b;
#endif
}

}}
