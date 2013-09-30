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
#include <oBase/surface_convert.h>
#include <oBase/memory.h>
#include <oStd/mutex.h>

namespace ouro {
	namespace surface {

info get_info_png(const void* _pBuffer, size_t _BufferSize);
std::shared_ptr<char> encode_png(const buffer* _pBuffer, size_t* _pSize, alpha_option::value _Option, compression::value _Compression);
std::shared_ptr<buffer> decode_png(const void* _pBuffer, size_t _BufferSize, alpha_option::value _Option);

info get_info_jpg(const void* _pBuffer, size_t _BufferSize);
std::shared_ptr<char> encode_jpg(const buffer* _pBuffer, size_t* _pSize, alpha_option::value _Option, compression::value _Compression);
std::shared_ptr<buffer> decode_jpg(const void* _pBuffer, size_t _BufferSize, alpha_option::value _Option);
		
file_format::value get_file_format(const void* _pBuffer, size_t _BufferSize)
{
	static const unsigned char png_sig[8] = { 137, 80, 78, 71, 13, 10, 26, 10 };
	static const unsigned char jpg_sig1[4] = { 0xff, 0xd8, 0xff, 0xe0 };
	static const char jpg_sig2[5] = "JFIF";

	return (_BufferSize >= 8 && !memcmp(png_sig, _pBuffer, sizeof(png_sig)))
		? file_format::png
		: (_BufferSize >= 11 
			&& !memcmp(jpg_sig1, _pBuffer, sizeof(jpg_sig1)) 
			&& !memcmp(jpg_sig2, ((char*)_pBuffer) + 6, sizeof(jpg_sig2)))
			? file_format::jpg
			: file_format::unknown;
}

info get_info(const void* _pBuffer, size_t _BufferSize)
{
	switch (get_file_format(_pBuffer, _BufferSize))
	{
		case file_format::png: return std::move(get_info_png(_pBuffer, _BufferSize));
		case file_format::jpg: return std::move(get_info_jpg(_pBuffer, _BufferSize));
		default: break;
	}
	throw std::exception("unknown image encoding");
}
	
std::shared_ptr<char> encode(const buffer* _pBuffer
	, size_t* _pSize
	, file_format::value _FileFormat
	, alpha_option::value _Option
	, compression::value _Compression)
{
	switch (_FileFormat)
	{
		case file_format::png: return std::move(encode_png(_pBuffer, _pSize, _Option, _Compression));
		case file_format::jpg: return std::move(encode_jpg(_pBuffer, _pSize, _Option, _Compression));
		default: break;
	}
	throw std::exception("unknown image encoding");
}

std::shared_ptr<buffer> decode(const void* _pBuffer, size_t _BufferSize, alpha_option::value _Option)
{
	switch (get_file_format(_pBuffer, _BufferSize))
	{
		case file_format::png: return std::move(decode_png(_pBuffer, _BufferSize, _Option));
		case file_format::jpg: return std::move(decode_jpg(_pBuffer, _BufferSize, _Option));
		default: break;
	}
	throw std::exception("unknown image encoding");
}

	} // namespace surface
} // namespace ouro
