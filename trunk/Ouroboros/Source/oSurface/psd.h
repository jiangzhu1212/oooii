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

enum psd_constants
{
  signature = 0x38425053, // '8BPS'
  version = 1,
  max_dimension = 30000,
  min_channels = 1,
  max_channels = 56,
};

enum class psd_bpp : ushort
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

// To get to the flattened bits first look at the uint32_t right after
// the header (color mode data section) length. Offset by that value to
// a uint32_t for the image resources section length. Offset by that 
// value to a uint32_t that is the layer and mask section length. Offset
// by that to get to the red plane, which is width*pixelsize*height then
// green then blue then alpha then other channels.

inline const psd_header* psd_validate(const void* buffer, size_t size)
{
  auto h = (const psd_header*)buffer;
  if (size < sizeof(psd_header) 
    || h->signature != psd_constants::signature 
    || h->version != psd_constants::version
    || h->num_channels < psd_constants::min_channels 
    || h->num_channels > psd_constants::max_channels
    || h->height == 0 || h->height > psd_constants::max_dimension 
    || h->width == 0 || h->width > psd_constants::max_dimension )
    return nullptr;
    
  switch (h->bpp)
  {
    case psd_bpp::k1: case psd_bpp::k8: case psd_bpp::k16: case psd_bpp::k32: break;
    default: return nullptr;
  }
  
  switch (h->color_mode)
  {
    case psd_color_mode::rgb:
    case psd_color_mode::bitmap: case psd_color_mode::grayscale: 
    case psd_color_mode::indexed: case psd_color_mode::cmyk:
    case psd_color_mode::multichannel: case psd_color_mode::duotone: 
    case psd_color_mode::lab: break;
    default: return nullptr;
  }
  
  return h;
}

#endif
