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
#include <oBase/surface_codec.h>
#include <oBase/throw.h>

namespace ouro {
	namespace surface {

info get_info_jpg(const void* _pBuffer, size_t _BufferSize)
{
	oTHROW(function_not_supported, "jpg not supported");
}

std::shared_ptr<char> encode_jpg(const buffer* _pBuffer
	, size_t* _pSize
	, alpha_option::value _Option
	, compression::value _Compression)
{
	oTHROW(function_not_supported, "jpg not supported");
}

std::shared_ptr<buffer> decode_jpg(const void* _pBuffer, size_t _BufferSize, alpha_option::value _Option)
{
	oTHROW(function_not_supported, "jpg not supported");
}

	} // namespace surface
} // namespace ouro
