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
// Facade for encoding and decoding several image formats
#pragma once
#ifndef oSurface_codec_h
#define oSurface_codec_h

#include <oSurface/texel_buffer.h>
#include <oSurface/surface.h>

namespace ouro { namespace surface {

enum class file_format : uchar
{
	unknown,
	bmp,
	dds,
	jpg,
	png,
	psd,
	tga,
};

enum class compression : uchar
{
	none,
  low,
  medium,
  high,
};

// checks the file extension
file_format get_file_format(const char* path);

// checks the first few bytes/header info
file_format get_file_format(const void* buffer, size_t size);

// converts the first few bytes of a supported format into a surface::info
// if not recognized the returned format will be surface::unknown
info get_info(const void* buffer, size_t size);

// returns a buffer ready to be written to disk in the specified format.
// this may use the specified allocator to convert the texel buffer to 
// the input format for the codec.
scoped_allocation encode(const texel_buffer& b
	, const file_format& fmt
	, const format& desired_format = format::unknown
	, const compression& compression = compression::low);

// Parses the in-memory formatted buffer into a surface.
texel_buffer decode(const void* buffer
	, size_t size
	, const format& desired_format = format::unknown
	, const mip_layout& layout = mip_layout::none);

inline texel_buffer decode(const scoped_allocation& buffer
	, const format& desired_format = format::unknown
	, const mip_layout& layout = mip_layout::none) { return decode(buffer, buffer.size(), desired_format, layout); }

}}

#endif
