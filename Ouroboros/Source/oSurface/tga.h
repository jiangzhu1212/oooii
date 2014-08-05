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
// Standard structs and definitions for the targa (.tga) image file format.
#ifndef tga_h
#define tga_h

#include <stdint.h>

enum tga_data_type_field
{
  no_image_data = 0,
  paletted = 1,
  rgb = 2,
  black_and_white = 3,
  rle_paletted = 9,
  rle_rgb = 10,
  compressed_black_and_white = 11,
  compressed_paletted = 32,
  compressed_paletted_quadtree = 33,
};

#pragma pack(push,1)
struct tga_header
{
  uint8_t id_length;
  uint8_t paletted_type;
  uint8_t data_type_field;
  uint16_t paletted_origin;
  uint16_t paletted_length;
  uint8_t paletted_depth;
  uint16_t x_origin;
  uint16_t y_origin;
  uint16_t width;
  uint16_t height;
  uint8_t bpp;
  uint8_t image_descriptor;
};
#pragma pack(pop)

inline bool tga_is_valid_dtf(uint8_t dtf)
{
	switch (dtf)
	{
		case no_image_data:
		case paletted:
		case rgb:
		case black_and_white:
		case rle_paletted:
		case rle_rgb:
		case compressed_black_and_white:
		case compressed_paletted:
		case compressed_paletted_quadtree:
			return true;
		default: break;
	}
	return false;
}

#endif
