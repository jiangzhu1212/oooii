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

namespace ouro { namespace surface {

// add format extension to this list and it will propagate to all the apis below
#define FOREACH_EXT(macro) macro(bmp) macro(dds) macro(jpg) macro(png) macro(psd) macro(tga)

// _____________________________________________________________________________
// Boilerplate (don't use directly, they're registered with the functions below)

#define DECLARE_CODEC(ext) \
	info get_info_##ext(const void* buffer, size_t size); \
	scoped_allocation encode_##ext(const texel_buffer& b, const alpha_option& option, const compression& compression); \
	texel_buffer decode_##ext(const void* buffer, size_t size, const alpha_option& option, const mip_layout& layout);

#define GET_FILE_FORMAT_EXT(ext) if (!_stricmp(extension, "." #ext)) return file_format::##ext;
#define GET_FILE_FORMAT_HDR(ext) if (get_info_##ext(buffer, size).format != format::unknown) return file_format::##ext;
#define GET_INFO(ext) case file_format::##ext: return get_info_##ext(buffer, size);
#define ENCODE(ext) case file_format::##ext: return encode_##ext(b, option, compression);
#define DECODE(ext) case file_format::##ext: return decode_##ext(buffer, size, option, layout);
#define AS_STRING(ext) case surface::file_format::##ext: return #ext;

FOREACH_EXT(DECLARE_CODEC)

format alpha_option_format(const format& fmt, const alpha_option& option)
{
	switch (option)
	{
		case alpha_option::force_alpha:
			switch (fmt)
			{
				case format::r8g8b8_unorm: return format::r8g8b8a8_unorm;
				case format::r8g8b8_unorm_srgb: return format::r8g8b8a8_unorm_srgb;
				case format::b8g8r8_unorm: return format::b8g8r8a8_unorm;
				case format::b8g8r8_unorm_srgb: return format::b8g8r8a8_unorm_srgb;
				case format::x8b8g8r8_unorm: return format::a8b8g8r8_unorm;
				case format::x8b8g8r8_unorm_srgb: return format::a8b8g8r8_unorm_srgb;
				case format::b5g6r5_unorm: return format::b5g5r5a1_unorm;
				default: break;
			}
			break;

		case alpha_option::force_no_alpha:
			switch (fmt)
			{
				case format::r8g8b8a8_unorm: return format::r8g8b8_unorm;
				case format::r8g8b8a8_unorm_srgb: return format::r8g8b8_unorm_srgb;
				case format::b8g8r8a8_unorm: return format::b8g8r8_unorm;
				case format::b8g8r8a8_unorm_srgb: return format::b8g8r8_unorm_srgb;
				case format::a8b8g8r8_unorm: return format::x8b8g8r8_unorm;
				case format::a8b8g8r8_unorm_srgb: return format::x8b8g8r8_unorm_srgb;
				case format::b5g5r5a1_unorm: return format::b5g6r5_unorm;
				default: break;
			}
			break;

		default:
			break;
	}

	return fmt;
}

file_format get_file_format(const char* path)
{
	const char* extension = rstrstr(path, ".");
	FOREACH_EXT(GET_FILE_FORMAT_EXT)
	return file_format::unknown;
}

file_format get_file_format(const void* buffer, size_t size)
{
	FOREACH_EXT(GET_FILE_FORMAT_HDR)
	return file_format::unknown;
}

info get_info(const void* buffer, size_t size)
{
	switch (get_file_format(buffer, size))
	{	FOREACH_EXT(GET_INFO)
		default: break;
	}
	throw std::exception("unknown image encoding");
}
	
scoped_allocation encode(const texel_buffer& b
	, const file_format& fmt
	, const alpha_option& option
	, const compression& compression)
{
	switch (fmt)
	{ FOREACH_EXT(ENCODE)
		default: break;
	}
	throw std::exception("unknown image encoding");
}

texel_buffer decode(const void* buffer, size_t size, const alpha_option& option, const mip_layout& layout)
{
	switch (get_file_format(buffer, size))
	{ FOREACH_EXT(DECODE)
		default: break;
	}
	throw std::exception("unknown image encoding");
}

	}

const char* as_string(const surface::file_format& ff)
{
	switch (ff)
	{
		FOREACH_EXT(AS_STRING)
		default: break;
	}
	return "?";
}

}
