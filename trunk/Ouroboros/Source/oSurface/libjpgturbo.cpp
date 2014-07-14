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
#include <oBase/finally.h>
#include <oBase/throw.h>
#undef JPEG_LIB_VERSION
#define JPEG_LIB_VERSION 80
#include <libjpegTurbo/jpeglib.h>

namespace ouro {
	namespace surface {

void error_exit_throw(j_common_ptr cinfo)
{
  char buffer[JMSG_LENGTH_MAX];
  (*cinfo->err->format_message)(cinfo, buffer);
	throw std::exception(buffer);
}

void dont_output_message(j_common_ptr cinfo)
{
}

// @tony: Why'd we do this again? and not to other libraries? Was this subsumed 
// by the custom turbo-jpeg effort at OOOii? Can I get rid of all this?

void* o_get_small(j_common_ptr cinfo, size_t sizeofobject)
{
	return malloc(sizeofobject);
}

void o_free_small(j_common_ptr cinfo, void* object, size_t sizeofobject)
{
	free(object);
}

void* o_get_large(j_common_ptr cinfo, size_t sizeofobject)
{
	return malloc(sizeofobject);
}

void o_free_large(j_common_ptr cinfo, void FAR* object, size_t sizeofobject)
{
	free(object);
}

size_t o_mem_available(j_common_ptr cinfo, size_t min_bytes_needed, size_t max_bytes_needed, size_t already_allocated)
{
	return max_bytes_needed;
}

long o_init(j_common_ptr cinfo)
{
	return 0;
}

void o_term(j_common_ptr cinfo)
{
}

static oooii_jpeg_memory_alloc s_jalloc =
{
	o_get_small,
	o_free_small,
	o_get_large,
	o_free_large,
	o_mem_available,
	o_init,
	o_term
};
#if 0
static J_COLOR_SPACE to_jcs(format _Format, int* _NumComponens)
{
	switch (_Format)
	{
		case r8_unorm: *_NumComponens = 1; return JCS_GRAYSCALE;
		case r8g8b8_unorm: *_NumComponens = 3; return JCS_RGB;
		case b8g8r8_unorm: *_NumComponens = 3; return JCS_EXT_BGR;
		case r8g8b8a8_unorm: *_NumComponens = 4; return JCS_EXT_RGBA;
		case b8g8r8a8_unorm: *_NumComponens = 4; return JCS_EXT_BGRA;
		default: break;
	}
	return JCS_UNKNOWN;
}
#endif
static format from_jcs(J_COLOR_SPACE _ColorSpace)
{
	switch (_ColorSpace)
	{
		case JCS_GRAYSCALE: return r8_unorm;
		case JCS_RGB: return r8g8b8_unorm;
		case JCS_EXT_BGR: return b8g8r8_unorm;
		case JCS_EXT_RGBA: return r8g8b8a8_unorm;
		case JCS_EXT_BGRA: return b8g8r8a8_unorm;
		default: break;
	}
	return unknown;
}

info get_info_jpg(const void* _pBuffer, size_t _BufferSize)
{
	jpeg_decompress_struct cinfo;
	jpeg_error_mgr jerr;
	cinfo.err = jpeg_std_error(&jerr);
	jerr.error_exit = error_exit_throw;
	jerr.output_message = dont_output_message;
	cinfo.alloc = &s_jalloc;

	jpeg_create_decompress(&cinfo);
	finally Destroy([&] { jpeg_destroy_decompress(&cinfo); });
	jpeg_mem_src(&cinfo, (unsigned char*)_pBuffer, static_cast<unsigned long>(_BufferSize));
	jpeg_read_header(&cinfo, TRUE);

	info si;
	si.format = from_jcs(cinfo.jpeg_color_space);
	si.layout = image;
	si.dimensions = int3(cinfo.image_width, cinfo.image_height, 1);
	return si;
}

std::shared_ptr<char> encode_jpg(const buffer* _pBuffer
	, size_t* _pSize
	, const alpha_option::value& _Option
	, const compression::value& _Compression)
{
#if 0
	jpeg_compress_struct cinfo;
	jpeg_error_mgr jerr;
	cinfo.err = jpeg_std_error(&jerr);
	jerr.error_exit = error_exit_throw;
	jerr.output_message = dont_output_message;
	cinfo.alloc = &s_jalloc;

	jpeg_create_compress(&cinfo);

	unsigned char* pCompressed = nullptr;
	unsigned long CompressedSize = 0;
	finally Destroy([&] { jpeg_destroy_compress(&cinfo); if (pCompressed) free(pCompressed); });

	info si = _pBuffer->get_info();
	cinfo.image_width = si.dimensions.x;
	cinfo.image_height = si.dimensions.y;
	cinfo.in_color_space = to_jcs(si.format, &cinfo.input_components);
	
	// todo: respect compression specification
	jpeg_set_defaults(&cinfo);

	int quality = 100;
	switch (_Compression)
	{
		case compression::none: quality = 100; break;
		case compression::low: quality = 95; break;
		case compression::medium: quality = 75; break;
		case compression::high: quality = 50; break;
		default: throw std::exception("invalid compression");
	}

	jpeg_set_quality(&cinfo, quality, TRUE);
	jpeg_mem_dest(&cinfo, &pCompressed, &CompressedSize);
	jpeg_start_compress(&cinfo, true);

	{
		JSAMPROW row[1];
		shared_lock lock(_pBuffer);
		while (cinfo.next_scanline < cinfo.image_height)
		{
			row[0] = (JSAMPLE*)byte_add(lock.mapped.data, cinfo.next_scanline * lock.mapped.row_pitch);
			jpeg_write_scanlines(&cinfo, row, 1);
		}
	}

	jpeg_finish_compress(&cinfo);
	std::shared_ptr<char> buffer((char*)pCompressed, free);
	pCompressed = nullptr; // prevent garbage collection
	if (_pSize)
		*_pSize = CompressedSize;
	return buffer;
#else
	throw std::exception("jpg code disabled until freeimage can be removed or reconciled with ver 80");
#endif
}

std::shared_ptr<buffer> decode_jpg(const void* _pBuffer, size_t _BufferSize, const alpha_option::value& _Option, const layout& _Layout)
{
#if 0
	jpeg_decompress_struct cinfo;
	jpeg_error_mgr jerr;
	cinfo.err = jpeg_std_error(&jerr);
	jerr.error_exit = error_exit_throw;
	jerr.output_message = dont_output_message;
	cinfo.alloc = &s_jalloc;

	jpeg_create_decompress(&cinfo);
	finally Destroy([&] { jpeg_destroy_decompress(&cinfo); });
	jpeg_mem_src(&cinfo, (unsigned char*)_pBuffer, static_cast<unsigned long>(_BufferSize));
	jpeg_read_header(&cinfo, TRUE);

	info si;
	si.format = from_jcs(cinfo.out_color_space);
	si.layout = _Layout;
	si.dimensions = int3(cinfo.image_width, cinfo.image_height, 1);

	switch (si.format)
	{
		case b8g8r8_unorm:
		case r8g8b8_unorm:
			si.format = _Option == alpha_option::force_alpha ? b8g8r8a8_unorm : b8g8r8_unorm;
			cinfo.out_color_space = alpha_option::force_alpha ? JCS_EXT_BGRA : JCS_EXT_BGR;
			cinfo.out_color_components = alpha_option::force_alpha ? 4 : 3;
			break;
		case b8g8r8a8_unorm:
		case r8g8b8a8_unorm:
			si.format = _Option == alpha_option::force_no_alpha ? b8g8r8_unorm : b8g8r8a8_unorm;
			cinfo.out_color_space = alpha_option::force_no_alpha ? JCS_EXT_BGR : JCS_EXT_BGRA;
			cinfo.out_color_components = alpha_option::force_no_alpha ? 3 : 4;
			break;
		case r8_unorm:
			break;
		default:
			throw std::exception("unsupported format");
	}

	std::shared_ptr<buffer> b = buffer::make(si);
	{
		JSAMPROW row[1];
		lock_guard lock(b);
		jpeg_start_decompress(&cinfo);

		while (cinfo.output_scanline < cinfo.output_height)
		{
			row[0] = (JSAMPLE*)byte_add(lock.mapped.data, cinfo.output_scanline * lock.mapped.row_pitch);
			jpeg_read_scanlines(&cinfo, row, 1);
		}
	}

	jpeg_finish_decompress(&cinfo);
	return b;
#else
	throw std::exception("jpg code disabled until freeimage can be removed or reconciled with ver 80");
#endif
}

	} // namespace surface
} // namespace ouro
