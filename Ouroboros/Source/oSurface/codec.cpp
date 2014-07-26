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
#include <oBase/memory.h>
#include <oBase/string.h>

namespace ouro {

const char* as_string(const surface::file_format::value& ff)
{
	switch (ff)
	{
		case surface::file_format::bmp: return "bmp";
		case surface::file_format::jpg: return "jpeg";
		case surface::file_format::png: return "png";
		case surface::file_format::tga: return "tga";
		default: break;
	}
	return "?";
}

	namespace surface {

format alpha_option_format(const format& fmt, const alpha_option::value& option)
{
	switch (option)
	{
		case alpha_option::force_alpha:
			switch (fmt)
			{
				case r8g8b8_unorm: return r8g8b8a8_unorm;
				case r8g8b8_unorm_srgb: return r8g8b8a8_unorm_srgb;
				case b8g8r8_unorm: return b8g8r8a8_unorm;
				case b8g8r8_unorm_srgb: return b8g8r8a8_unorm_srgb;
				case x8b8g8r8_unorm: return a8b8g8r8_unorm;
				case x8b8g8r8_unorm_srgb: return a8b8g8r8_unorm_srgb;
				case b5g6r5_unorm: return b5g5r5a1_unorm;
				default: break;
			}
			break;

		case alpha_option::force_no_alpha:
			switch (fmt)
			{
				case r8g8b8a8_unorm: return r8g8b8_unorm;
				case r8g8b8a8_unorm_srgb: return r8g8b8_unorm_srgb;
				case b8g8r8a8_unorm: return b8g8r8_unorm;
				case b8g8r8a8_unorm_srgb: return b8g8r8_unorm_srgb;
				case a8b8g8r8_unorm: return x8b8g8r8_unorm;
				case a8b8g8r8_unorm_srgb: return x8b8g8r8_unorm_srgb;
				case b5g5r5a1_unorm: return b5g6r5_unorm;
				default: break;
			}
			break;

		default:
			break;
	}

	return fmt;
}

file_format::value get_file_format(const char* path)
{
	const char* ext = rstrstr(path, ".");
	if (!_stricmp(ext, ".bmp")) return file_format::bmp;
	if (!_stricmp(ext, ".jpg")) return file_format::jpg;
	if (!_stricmp(ext, ".png")) return file_format::png;
	if (!_stricmp(ext, ".tga")) return file_format::tga;
	return file_format::unknown;
}

#define DEFINE_API(ext) \
	info get_info_##ext(const void* buffer, size_t size); \
	std::shared_ptr<char> encode_##ext(const buffer& buffer, size_t* out_size, const alpha_option::value& option, const compression::value& compression); \
	buffer decode_##ext(const void* buffer, size_t size, const alpha_option::value& option, const layout& layout);

DEFINE_API(bmp) DEFINE_API(jpg) DEFINE_API(png) DEFINE_API(tga)

file_format::value get_file_format(const void* _buffer, size_t size)
{
	#define TEST_INFO(ext) if (get_info_##ext(_buffer, size).format != unknown) return file_format::##ext
	TEST_INFO(bmp);
	TEST_INFO(jpg);
	TEST_INFO(png);
	TEST_INFO(tga);
	return file_format::unknown;
}

info get_info(const void* _pBuffer, size_t _BufferSize)
{
	switch (get_file_format(_pBuffer, _BufferSize))
	{
		case file_format::png: return get_info_png(_pBuffer, _BufferSize);
		case file_format::jpg: return get_info_jpg(_pBuffer, _BufferSize);
		case file_format::bmp: return get_info_bmp(_pBuffer, _BufferSize);
		default: break;
	}
	throw std::exception("unknown image encoding");
}
	
std::shared_ptr<char> encode(const buffer& _Buffer
	, size_t* _pSize
	, const file_format::value& _FileFormat
	, const alpha_option::value& _Option
	, const compression::value& _Compression)
{
	switch (_FileFormat)
	{
		case file_format::bmp: return encode_bmp(_Buffer, _pSize, _Option, _Compression);
		case file_format::jpg: return encode_jpg(_Buffer, _pSize, _Option, _Compression);
		case file_format::png: return encode_png(_Buffer, _pSize, _Option, _Compression);
		case file_format::tga: return encode_tga(_Buffer, _pSize, _Option, _Compression);
		default: break;
	}
	throw std::exception("unknown image encoding");
}

buffer decode(const void* _pBuffer, size_t _BufferSize, const alpha_option::value& _Option, const layout& _Layout)
{
	switch (get_file_format(_pBuffer, _BufferSize))
	{
		case file_format::bmp: return decode_bmp(_pBuffer, _BufferSize, _Option, _Layout);
		case file_format::jpg: return decode_jpg(_pBuffer, _BufferSize, _Option, _Layout);
		case file_format::png: return decode_png(_pBuffer, _BufferSize, _Option, _Layout);
		case file_format::tga: return decode_tga(_pBuffer, _BufferSize, _Option, _Layout);
		default: break;
	}
	throw std::exception("unknown image encoding");
}

	} // namespace surface
} // namespace ouro
