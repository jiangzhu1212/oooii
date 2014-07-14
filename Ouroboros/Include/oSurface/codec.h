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
// Describes encoding and decoding functions (basically file formats) for 
// surfaces.
#pragma once
#ifndef oSurface_codec_h
#define oSurface_codec_h

#include <oSurface/buffer.h>
#include <oSurface/surface.h>

namespace ouro {
	namespace surface {

namespace file_format
{	enum value {

	unknown,
	png,
	jpg,
	bmp,

};}

namespace compression
{	enum value {

	none,
  low,
  medium,
  high,

};}

namespace alpha_option
{	enum value {

	preserve,
	force_alpha,
	force_no_alpha,

};}

file_format::value get_file_format(const char* _FilePath);

// Analyzes the buffer to determine its file format
file_format::value get_file_format(const void* _pBuffer, size_t _BufferSize);

// Returns the info from a buffer formatted as a file in memory
info get_info(const void* _pBuffer, size_t _BufferSize);

// Returns a buffer ready to be written to disk in the specified format.
std::shared_ptr<char> encode(const buffer* _pBuffer
	, size_t* _pSize
	, const file_format::value& _FileFormat
	, const alpha_option::value& _Option = alpha_option::preserve
	, const compression::value& _Compression = compression::low);

// Parses the in-memory formatted buffer into a surface.
std::shared_ptr<buffer> decode(const void* _pBuffer
	, size_t _BufferSize
	, const alpha_option::value& _Option = alpha_option::preserve
	, const layout& _Layout = image);

	} // namespace surface
} // namespace ouro

#endif
