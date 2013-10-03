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

namespace ouro {
	namespace surface {

static surface::format to_format(int _Type, int _BitDepth)
{
	if (_BitDepth <= 8)
	{
		switch (_Type)
		{
			case PNG_COLOR_TYPE_GRAY: return surface::r8_unorm;
			case PNG_COLOR_TYPE_RGB: return surface::b8g8r8_unorm;
			case PNG_COLOR_TYPE_RGB_ALPHA: return surface::b8g8r8a8_unorm;
			default: break;
		}
	}
	return surface::unknown;
}

info get_info_png(const void* _pBuffer, size_t _BufferSize)
{
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
	rs.data = _pBuffer;
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
	i.layout = image;
	i.dimensions = int3(w, h, 1);
	i.array_size = 1;
	return i;
}

std::shared_ptr<char> encode_png(const buffer* _pBuffer
	, size_t* _pSize
	, alpha_option::value _Option
	, compression::value _Compression)
{
	const buffer* pSource = _pBuffer;
	std::shared_ptr<const buffer> Converted;

	info si = _pBuffer->get_info();
	switch (_Option)
	{
		case alpha_option::force_alpha:
		{
			if (si.format == r8_unorm)
				throw std::exception("can't force alpha on r8_unorm");

			if (si.format == r8g8b8_unorm || si.format == b8g8r8_unorm)
			{
				Converted = _pBuffer->convert(r8g8b8a8_unorm);
				si = Converted->get_info();
				pSource = Converted.get();
			}

			break;
		}

		case alpha_option::force_no_alpha:
		{
			if (si.format == r8g8b8a8_unorm || si.format == b8g8r8a8_unorm)
			{
				Converted = _pBuffer->convert(r8g8b8_unorm);
				si = Converted->get_info();
				pSource = Converted.get();
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
	switch (_Compression)
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
		case r8_unorm: color_type = PNG_COLOR_TYPE_GRAY; break;
		case b8g8r8_unorm: 
		case r8g8b8_unorm: color_type = PNG_COLOR_TYPE_RGB; break;
		case b8g8r8a8_unorm: 
		case r8g8b8a8_unorm: color_type = PNG_COLOR_TYPE_RGB_ALPHA; break;
		default: throw std::exception("invalid format");
	}

	// how big a buffer?

	//std::shared_ptr<char>

	png_set_IHDR(png_ptr, info_ptr, si.dimensions.x, si.dimensions.y, 8
		, color_type, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

	png_write_info(png_ptr, info_ptr);
	
	if (si.format == b8g8r8_unorm || si.format == b8g8r8a8_unorm)
		png_set_bgr(png_ptr);

	{
		std::vector<unsigned char*> rows;
		rows.resize(si.dimensions.y);
		const_mapped_subresource msr;
		shared_lock lock(pSource);
		rows[0] = (unsigned char*)lock.mapped.data;
		for (int y = 1; y < si.dimensions.y; y++)
			rows[y] = byte_add(rows[y-1], lock.mapped.row_pitch);
		png_write_image(png_ptr, rows.data());
		png_write_end(png_ptr, info_ptr);
	}

	std::shared_ptr<char> buffer((char*)ws.data, free);
	if (_pSize)
		*_pSize = ws.size;
	return std::move(buffer);
}

std::shared_ptr<buffer> decode_png(const void* _pBuffer, size_t _BufferSize, alpha_option::value _Option)
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
	rs.data = _pBuffer;
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
	si.layout = image;
	si.dimensions = int3(w, h, 1);
	si.array_size = 1;
	switch (color_type)
	{
		case PNG_COLOR_TYPE_GRAY:
			si.format = r8_unorm;
			if (depth < 8)
				png_set_expand_gray_1_2_4_to_8(png_ptr);
			break;
		case PNG_COLOR_TYPE_PALETTE:
			si.format = b8g8r8_unorm;
			png_set_palette_to_rgb(png_ptr);
			break;
		case PNG_COLOR_TYPE_RGB:
			si.format = b8g8r8_unorm;
			if (_Option == alpha_option::force_alpha)
			{
				si.format = b8g8r8a8_unorm;
				png_set_add_alpha(png_ptr, 0xff, PNG_FILLER_AFTER);
			}
			break;
		case PNG_COLOR_TYPE_RGB_ALPHA:
			si.format = b8g8r8a8_unorm;
			if (_Option == alpha_option::force_no_alpha)
			{
				si.format = b8g8r8_unorm;
				png_set_strip_alpha(png_ptr);
			}
			break;
		default:
		case PNG_COLOR_TYPE_GRAY_ALPHA:
			throw std::exception("unsupported gray/alpha");
	}

	// Set up the surface buffer
	png_read_update_info(png_ptr, info_ptr);
	std::shared_ptr<buffer> b = buffer::make(si);
	{
		std::vector<unsigned char*> rows;
		rows.resize(si.dimensions.y);
		lock_guard lock(b);

		rows[0] = (unsigned char*)lock.mapped.data;
		for (int y = 1; y < si.dimensions.y; y++)
			rows[y] = byte_add(rows[y-1], lock.mapped.row_pitch);

		png_read_image(png_ptr, rows.data());
	}
	return std::move(b);
}

	} // namespace surface
} // namespace ouro
