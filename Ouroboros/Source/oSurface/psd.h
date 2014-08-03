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
// Standard structs and definitions for the Adobe Photoshop (.psd) image 
// file format.
#ifndef psd_h
#define psd_h

#include <stdint.h>

#ifndef PSD_RESTRICT
#define PSD_RESTRICT __restrict
#endif

enum psd_constants
{
  signature = 0x38425053, // '8BPS'
  version = 1,
  max_dimension = 30000,
  min_channels = 1,
  max_channels = 56,
};

enum psd_bpp
{
  k1 = 1,
  k8 = 8,
  k16 = 16,
  k32 = 32,
};

enum class psd_color_mode : ushort
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

enum class psd_compression : ushort
{
  // no compression
  raw = 0,

  // image data starts with the byte counts for all the scan lines 
  // (rows * channels), with each count stored as a two-byte value. The 
  // RLE compressed data follows, with each scan line compressed 
  // separately. The RLE compression is the same compression algorithm 
  // used by the Macintosh ROM routine PackBits, and the TIFF standard.
  rle = 1,
  zip = 2,
  zip_prediction = 3,
};

#pragma pack(push,1)
struct psd_header
{
  uint32_t signature;
  uint16_t version;
  uint8_t reserved[6];
  uint16_t num_channels;
  uint32_t height;
  uint32_t width;
  uint16_t bpp;
  uint16_t color_mode;
};
#pragma pack(pop)

inline uint32_t psd_swap(uint32_t x) { return (x<<24) | ((x<<8) & 0x00ff0000) | ((x>>8) & 0x0000ff00) | (x>>24); }
inline uint16_t psd_swap(const uint16_t x) { return (x<<8) | (x>>8); }

// To get to the flattened bits first look at the uint32_t right after
// the header (color mode data section) length. Offset by that value to
// a uint32_t for the image resources section length. Offset by that 
// value to a uint32_t that is the layer and mask section length. Offset
// by that to get to the red plane, which is width*pixelsize*height then
// green then blue then alpha then other channels.

inline bool psd_validate(const void* buffer, size_t size, psd_header* out_header)
{
  if (size < sizeof(psd_header))
		return false;

	auto hh = (const psd_header*)buffer;
	out_header->signature = psd_swap(hh->signature);
	out_header->version = psd_swap(hh->version);
	out_header->reserved[0] = 0;
	out_header->reserved[1] = 0;
	out_header->reserved[2] = 0;
	out_header->reserved[3] = 0;
	out_header->reserved[4] = 0;
	out_header->reserved[5] = 0;
  out_header->num_channels = psd_swap(hh->num_channels);
  out_header->height = psd_swap(hh->height);
  out_header->width = psd_swap(hh->width);
  out_header->bpp = psd_swap(hh->bpp);
  out_header->color_mode = psd_swap(hh->color_mode);

  if (out_header->signature != psd_constants::signature 
    || out_header->version != psd_constants::version
    || out_header->num_channels < psd_constants::min_channels 
    || out_header->num_channels > psd_constants::max_channels
    || out_header->height == 0 || out_header->height > psd_constants::max_dimension 
    || out_header->width == 0 || out_header->width > psd_constants::max_dimension )
    return nullptr;
    
  switch (out_header->bpp)
  {
    case psd_bpp::k1: case psd_bpp::k8: case psd_bpp::k16: case psd_bpp::k32: break;
    default: return false;
  }
  
  switch (out_header->color_mode)
  {
    case psd_color_mode::rgb:
    case psd_color_mode::bitmap: case psd_color_mode::grayscale: 
    case psd_color_mode::indexed: case psd_color_mode::cmyk:
    case psd_color_mode::multichannel: case psd_color_mode::duotone: 
    case psd_color_mode::lab: break;
    default: return false;
  }
  
  return true;
}

// copies the planar-formatted sources to dst as interleaved. To swap rgb <-> bgr
// pass in the planar source swapped.
template<typename T>
void interleave_channels(T* PSD_RESTRICT dst, size_t row_pitch, size_t num_rows, bool dst_has_alpha
	, const void* PSD_RESTRICT red, const void* PSD_RESTRICT green, const void* PSD_RESTRICT blue, const void* PSD_RESTRICT alpha)
{
  const T* PSD_RESTRICT r = (const T*)red;
  const T* PSD_RESTRICT g = (const T*)green;
  const T* PSD_RESTRICT b = (const T*)blue;
  const T* PSD_RESTRICT a = (const T*)alpha;
  
  if (dst_has_alpha)
  {
    for (size_t y = 0; y < num_rows; y++)
    {
      T* row_end = (T*)((uint8_t*)dst + row_pitch);
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
    for (size_t y = 0; y < num_rows; y++)
    {
      T* row_end = (T*)((uint8_t*)dst + row_pitch);
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
