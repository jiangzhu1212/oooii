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
#include <oSurface/codec.h>
#include <oBase/byte.h>
#include <oBase/finally.h>
#include <oBase/throw.h>
#include <libpng/png.h>
#include <zlib/zlib.h>
#include <vector>

struct read_state
{
	const void* data;
	size_t offset;
};

struct write_state
{
	void* data;
	size_t size;
	size_t capacity;
};

static void user_read_data(png_structp png_ptr, png_bytep data, png_size_t length)
{
	read_state& r = *(read_state*)png_get_io_ptr(png_ptr);
	if (data) memcpy(data, ouro::byte_add(r.data, r.offset), length);
	r.offset += length;
}

void user_write_data(png_structp png_ptr, png_bytep data, png_size_t length)
{
	write_state& w = *(write_state*)png_get_io_ptr(png_ptr);
	size_t NewSize = w.size + length;

	if (NewSize > w.capacity)
	{
		w.capacity = NewSize + (NewSize / 2);
		w.data = realloc(w.data, w.capacity);
	}

	memcpy(ouro::byte_add(w.data, w.size), data, length);
	w.size += length;
}

void user_flush_data(png_structp png_ptr)
{
}

namespace ouro { namespace surface {

static surface::format to_format(int _Type, int _BitDepth)
{
	if (_BitDepth <= 8)
	{
		switch (_Type)
		{
			case PNG_COLOR_TYPE_GRAY: return surface::format::r8_unorm;
			case PNG_COLOR_TYPE_RGB: return surface::format::b8g8r8_unorm;
			case PNG_COLOR_TYPE_RGB_ALPHA: return surface::format::b8g8r8a8_unorm;
			default: break;
		}
	}
	return surface::format::unknown;
}

info get_info_png(const void* buffer, size_t size)
{
	static const uchar png_sig[8] = { 137, 80, 78, 71, 13, 10, 26, 10 };
	if (size < 8 || memcmp(png_sig, buffer, sizeof(png_sig)))
		return info();
	
	// initialze libpng with user functions pointing to _pBuffer
	png_infop info_ptr = nullptr;
	png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
	if (!png_ptr)
		throw std::exception("get_info_png failed");
	finally cleanup_png_ptr([&] { png_destroy_read_struct(&png_ptr, &info_ptr, nullptr); });
	info_ptr = png_create_info_struct(png_ptr);
	if (!info_ptr)
		throw std::exception("get_info_png failed");

	#pragma warning(disable:4611) // interaction between '_setjmp' and C++ object destruction is non-portable
	if (setjmp(png_jmpbuf(png_ptr)))
		throw std::exception("png read failed");
	#pragma warning(default:4611) // interaction between '_setjmp' and C++ object destruction is non-portable

	read_state rs;
	rs.data = buffer;
	rs.offset = 0;
	png_set_read_fn(png_ptr, &rs, user_read_data);
	png_read_info(png_ptr, info_ptr);

	// Read initial information and configure the decoder according
	// to surface policy (bgr, always unpaletted, alpha_option).
	unsigned int w = 0, h = 0;
	int depth = 0, color_type = 0;
	png_get_IHDR(png_ptr, info_ptr, &w, &h, &depth, &color_type, nullptr, nullptr, nullptr);
	
	surface::info i;
	i.format = to_format(color_type, depth);
	i.layout = layout::image;
	i.dimensions = int3(w, h, 1);
	return i;
}

scoped_allocation encode_png(const texel_buffer& b
	, const alpha_option& option
	, const compression& compression)
{
	const texel_buffer* pSource = &b;
	texel_buffer Converted;

	info si = b.get_info();
	switch (option)
	{
		case alpha_option::force_alpha:
		{
			if (si.format == format::r8_unorm)
				throw std::exception("can't force alpha on r8_unorm");

			if (si.format == format::r8g8b8_unorm || si.format == format::b8g8r8_unorm)
			{
				Converted = b.convert(format::r8g8b8a8_unorm);
				si = Converted.get_info();
				pSource = &Converted;
			}

			break;
		}

		case alpha_option::force_no_alpha:
		{
			if (si.format == format::r8g8b8a8_unorm || si.format == format::b8g8r8a8_unorm)
			{
				Converted = b.convert(format::r8g8b8_unorm);
				si = Converted.get_info();
				pSource = &Converted;
			}
			break;
		}

		default:
			break;
	}

	// initialize libpng with user functions pointing to _pBuffer
	png_infop info_ptr = nullptr;
	png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
	if (!png_ptr)
		throw std::exception("png read failed");
	finally cleanup_png_ptr([&] { png_destroy_write_struct(&png_ptr, &info_ptr); });
	info_ptr = png_create_info_struct(png_ptr);
	if (!info_ptr)
		throw std::exception("png read failed");

	#pragma warning(disable:4611) // interaction between '_setjmp' and C++ object destruction is non-portable
	if (setjmp(png_jmpbuf(png_ptr)))
		throw std::exception("png read failed");
	#pragma warning(default:4611) // interaction between '_setjmp' and C++ object destruction is non-portable

	write_state ws;
	ws.capacity = si.dimensions.y * si.dimensions.x * element_size(si.format);
	ws.capacity += (ws.capacity / 2);
	ws.data = malloc(ws.capacity); // use uncompressed size as an estimate to reduce reallocs
	ws.size = 0;
	png_set_write_fn(png_ptr, &ws, user_write_data, user_flush_data);

	int zcomp = Z_NO_COMPRESSION;
	switch (compression)
	{
		case compression::none: zcomp = Z_NO_COMPRESSION; break;
		case compression::low: zcomp = Z_BEST_SPEED; break;
		case compression::medium: zcomp = Z_DEFAULT_COMPRESSION; break;
		case compression::high: zcomp = Z_BEST_COMPRESSION; break;
		default: throw std::exception("invalid compression");
	}
	png_set_compression_level(png_ptr, zcomp);

	int color_type = 0;
	switch (si.format)
	{
		case format::r8_unorm: color_type = PNG_COLOR_TYPE_GRAY; break;
		case format::b8g8r8_unorm: 
		case format::r8g8b8_unorm: color_type = PNG_COLOR_TYPE_RGB; break;
		case format::b8g8r8a8_unorm: 
		case format::r8g8b8a8_unorm: color_type = PNG_COLOR_TYPE_RGB_ALPHA; break;
		default: throw std::exception("invalid format");
	}

	// how big a buffer?
	png_set_IHDR(png_ptr, info_ptr, si.dimensions.x, si.dimensions.y, 8
		, color_type, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

	png_write_info(png_ptr, info_ptr);
	
	if (si.format == format::b8g8r8_unorm || si.format == format::b8g8r8a8_unorm)
		png_set_bgr(png_ptr);

	{
		std::vector<uchar*> rows;
		rows.resize(si.dimensions.y);
		const_mapped_subresource msr;
		shared_lock lock(pSource);
		rows[0] = (uchar*)lock.mapped.data;
		for (uint y = 1; y < si.dimensions.y; y++)
			rows[y] = byte_add(rows[y-1], lock.mapped.row_pitch);
		png_write_image(png_ptr, rows.data());
		png_write_end(png_ptr, info_ptr);
	}

	return scoped_allocation(ws.data, ws.size, free);
}

texel_buffer decode_png(const void* buffer, size_t size, const alpha_option& option, const layout& _Layout)
{
	// initialze libpng with user functions pointing to _pBuffer
	png_infop info_ptr = nullptr;
	png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
	if (!png_ptr)
		throw std::exception("png read failed");
	finally cleanup_png_ptr([&] { png_destroy_read_struct(&png_ptr, &info_ptr, nullptr); });
	info_ptr = png_create_info_struct(png_ptr);
	if (!info_ptr)
		throw std::exception("png read failed");

	#pragma warning(disable:4611) // interaction between '_setjmp' and C++ object destruction is non-portable
	if (setjmp(png_jmpbuf(png_ptr)))
		throw std::exception("png read failed");
	#pragma warning(default:4611) // interaction between '_setjmp' and C++ object destruction is non-portable

	read_state rs;
	rs.data = buffer;
	rs.offset = 0;
	png_set_read_fn(png_ptr, &rs, user_read_data);
	png_read_info(png_ptr, info_ptr);

	// Read initial information and configure the decoder according
	// to surface policy (bgr, always unpaletted, alpha_option).
	unsigned int w = 0, h = 0;
	int depth = 0, color_type = 0;
	png_get_IHDR(png_ptr, info_ptr, &w, &h, &depth, &color_type, nullptr, nullptr, nullptr);
	
	if (depth == 16)
		png_set_strip_16(png_ptr);

	png_set_bgr(png_ptr);

	info si;
	si.layout = _Layout;
	si.dimensions = int3(w, h, 1);
	switch (color_type)
	{
		case PNG_COLOR_TYPE_GRAY:
			si.format = format::r8_unorm;
			if (depth < 8)
				png_set_expand_gray_1_2_4_to_8(png_ptr);
			break;
		case PNG_COLOR_TYPE_PALETTE:
			si.format = format::b8g8r8_unorm;
			png_set_palette_to_rgb(png_ptr);
			break;
		case PNG_COLOR_TYPE_RGB:
			si.format = format::b8g8r8_unorm;
			if (option == alpha_option::force_alpha)
			{
				si.format = format::b8g8r8a8_unorm;
				png_set_add_alpha(png_ptr, 0xff, PNG_FILLER_AFTER);
			}
			break;
		case PNG_COLOR_TYPE_RGB_ALPHA:
			si.format = format::b8g8r8a8_unorm;
			if (option == alpha_option::force_no_alpha)
			{
				si.format = format::b8g8r8_unorm;
				png_set_strip_alpha(png_ptr);
			}
			break;
		default:
		case PNG_COLOR_TYPE_GRAY_ALPHA:
			throw std::exception("unsupported gray/alpha");
	}

	// Set up the surface buffer
	png_read_update_info(png_ptr, info_ptr);
	texel_buffer b(si);
	{
		std::vector<uchar*> rows;
		rows.resize(si.dimensions.y);
		lock_guard lock(b);

		rows[0] = (uchar*)lock.mapped.data;
		for (uint y = 1; y < si.dimensions.y; y++)
			rows[y] = byte_add(rows[y-1], lock.mapped.row_pitch);

		png_read_image(png_ptr, rows.data());
	}
	return b;
}

}}
