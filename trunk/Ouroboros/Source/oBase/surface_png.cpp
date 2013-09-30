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
#include <oBase/finally.h>
#include <oBase/surface_codec.h>
#include <oBase/throw.h>
#include <lodepng/lodepng.h>
#include <pnglite/pnglite.h>

#define LODE_CALL(fn) do \
{ uint lodeerror__ = fn; \
	if (lodeerror__) throw std::exception(lodepng_error_text(lodeerror__)); \
} while(false)

#define LITE_CALL(fn) do \
{ int liteerror__ = fn; \
	if (liteerror__) throw std::exception(png_error_string(liteerror__)); \
} while(false)

namespace ouro {
	namespace surface {

static surface::format to_format(LodePNGColorType _Type, int _BitDepth)
{
	if (_BitDepth <= 8)
	{
		switch (_Type)
		{
			case LCT_GREY: return surface::r8_unorm;
			case LCT_RGB: return surface::r8g8b8_unorm;
			case LCT_RGBA: return surface::r8g8b8a8_unorm;
			default: break;
		}
	}
	return surface::unknown;
}

info get_info_png(const void* _pBuffer, size_t _BufferSize)
{
	unsigned int w = 0, h = 0;
	LodePNGState state;
	lodepng_state_init(&state);
	finally cleanup([&] { lodepng_state_cleanup(&state); });
	LODE_CALL(lodepng_inspect(&w, &h, &state, (const unsigned char*)_pBuffer, _BufferSize));
	surface::info i;
	i.format = to_format(state.info_png.color.colortype, state.info_png.color.bitdepth);
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
	LodePNGState state;
	lodepng_state_init(&state);
	finally cleanup([&] { lodepng_state_cleanup(&state); });
	state.info_raw.bitdepth = 8;
	state.encoder.add_id = false;

	info si = _pBuffer->get_info();
	switch (si.format)
	{
		case b8g8r8a8_unorm:
		case r8g8b8a8_unorm: state.info_raw.colortype = LCT_RGBA; break;
		case b8g8r8_unorm:
		case r8g8b8_unorm: state.info_raw.colortype = LCT_RGB; break;
		case r8_unorm: state.info_raw.colortype = LCT_GREY; break;
		default: throw std::invalid_argument("invalid format");
	}

	switch (_Compression)
	{
		case compression::none:
			state.encoder.zlibsettings.btype = 0;
			state.encoder.zlibsettings.minmatch = 0;
			state.encoder.zlibsettings.nicematch = 0;
			state.encoder.zlibsettings.lazymatching = 0;
			state.encoder.auto_convert = LAC_NO;
			state.encoder.text_compression = 0;
			break;
		case compression::low:
			//state.encoder.zlibsettings.nicematch = 128;
			//state.encoder.zlibsettings.lazymatching = 1;
			//state.encoder.zlibsettings.windowsize = 2048;
			break;
		case compression::medium:
			state.encoder.zlibsettings.nicematch = 193;
			state.encoder.zlibsettings.lazymatching = 1;
			state.encoder.zlibsettings.windowsize = 16384;
			break;
		case compression::high:
			state.encoder.zlibsettings.nicematch = 258;
			state.encoder.zlibsettings.lazymatching = 1;
			state.encoder.zlibsettings.windowsize = 32768;
			break;

		default:
			throw std::invalid_argument("invalid compression");
	}

	// Prepare the source buffer for conversion.
	format DestinationFormat = si.format;
	switch (_Option)
	{
		case alpha_option::preserve:
			switch (si.format)
			{
				case b8g8r8a8_unorm: DestinationFormat = r8g8b8a8_unorm; break;
				case b8g8r8_unorm: DestinationFormat = r8g8b8_unorm; break;
				default: break;
			}
			break;
		case alpha_option::force_alpha:
			switch (si.format)
			{
				case r8g8b8a8_unorm: case b8g8r8a8_unorm: break; // already have alpha
				case r8g8b8_unorm: DestinationFormat = r8g8b8a8_unorm; break;
				case b8g8r8_unorm: DestinationFormat = r8g8b8a8_unorm; break;
				default: throw std::invalid_argument("invalid format for alpha option");
			}
			break;
		case alpha_option::force_no_alpha:
			switch (si.format)
			{
				case r8g8b8a8_unorm: DestinationFormat = r8g8b8_unorm; break;
				case b8g8r8a8_unorm: DestinationFormat = r8g8b8_unorm; break;
				case r8g8b8_unorm: case b8g8r8_unorm: break; // already have alpha
				default: throw std::invalid_argument("invalid format for alpha option");
			}
			break;
		default:
			throw std::invalid_argument("invalid alpha option");
	}

	std::shared_ptr<const surface::buffer> Converted;
	const surface::buffer* pSource = nullptr;
	
	if (si.format == DestinationFormat)
		pSource = _pBuffer;
	else
	{
		Converted = _pBuffer->convert(DestinationFormat);
		pSource = Converted.get();
	}

	// encode the raw surface as a png-in-memory
	unsigned char* png = nullptr;
	size_t pngsize = 0;
	surface::shared_lock lock(pSource);
	LODE_CALL(lodepng_encode(&png, &pngsize, (const unsigned char*)lock.mapped.data, si.dimensions.x, si.dimensions.y, &state));
	if (_pSize)
		*_pSize = pngsize;

	std::shared_ptr<char> buffer((char*)png, free);
	return std::move(buffer);
}

std::shared_ptr<buffer> decode_png(const void* _pBuffer, size_t _BufferSize, alpha_option::value _Option)
{
	unsigned char* pOut = nullptr;
	finally DeleteOut([&] { if (pOut) free(pOut); });
	unsigned int w = 0, h = 0;
	LodePNGState state;
	lodepng_state_init(&state);
	finally cleanup([&] { lodepng_state_cleanup(&state); });
	LODE_CALL(lodepng_inspect(&w, &h, &state, (const unsigned char*)_pBuffer, _BufferSize));
	info si;
	si.format = to_format(state.info_png.color.colortype, state.info_png.color.bitdepth);
	si.layout = image;
	si.dimensions = int3(w, h, 1);
	si.array_size = 1;

	switch (_Option)
	{
		case alpha_option::preserve: LODE_CALL(lodepng_decode_memory(&pOut, &w, &h, (const unsigned char*)_pBuffer, _BufferSize, state.info_png.color.colortype, 8)); break;
		case alpha_option::force_alpha: LODE_CALL(lodepng_decode32(&pOut, &w, &h, (const unsigned char*)_pBuffer, _BufferSize)); break;
		case alpha_option::force_no_alpha: LODE_CALL(lodepng_decode24(&pOut, &w, &h, (const unsigned char*)_pBuffer, _BufferSize)); break;
		default: throw std::invalid_argument("invalid alpha option");
	}

	std::shared_ptr<buffer> b = buffer::make(si, pOut);
	// prevent finally from affecting anything now that the buffer has been 
	// handled off.
	pOut = nullptr;

	switch (si.format)
	{
		case r8g8b8a8_unorm: b->swizzle(b8g8r8a8_unorm); break;
		case r8g8b8_unorm: b->swizzle(b8g8r8_unorm); break;
		default: break;
	}
	
	return std::move(b);
}

	} // namespace surface
} // namespace ouro
