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
#include <oSurface/fill.h>
#include <oBase/colors.h>
#include <oBase/throw.h>
#include <oBase/timer.h>
#include <oBase/fixed_string.h>
#include <oCore/filesystem.h>

#include "../../test_services.h"

namespace ouro { 
	namespace tests {

static bool kSaveToDesktop = false;

static void compare_checkboards(const int2& dimensions, const surface::format& format, const surface::file_format& file_format, float max_rms)
{
	oTRACE("testing codec %s -> %s", as_string(format), as_string(file_format));

	// Create a buffer with a known format
	surface::info si;
	si.format = format;
	si.layout = surface::image;
	si.dimensions = int3(dimensions, 1);
	surface::texel_buffer known(si);
	size_t knownSize = known.size();
	{
		surface::lock_guard lock(known);
		surface::fill_checkerboard((color*)lock.mapped.data, lock.mapped.row_pitch, si.dimensions.xy(), si.dimensions.xy() / 2, blue, red);
	}

	size_t EncodedSize = 0;
	scoped_allocation encoded;

	{
		scoped_timer("encode");
		encoded = surface::encode(known
			, file_format
			, surface::alpha_option::force_no_alpha
			, surface::compression::none);
	}

	sstring buf;
	format_bytes(buf, EncodedSize, 2);
	oTRACE("encoded %s", buf.c_str());

	if (kSaveToDesktop)
	{
		mstring fname;
		snprintf(fname, "encoded_from_known_%s.%s", as_string(file_format), as_string(file_format));
		filesystem::save(filesystem::desktop_path() / path(fname), encoded, encoded.size());
	}

	surface::texel_buffer decoded;
	{
		scoped_timer("decode");
		decoded = surface::decode(encoded, encoded.size(), surface::alpha_option::force_alpha);
	}

	{
		size_t decodedSize = decoded.size();

		if (known.size() != decoded.size())
			oTHROW(io_error, "encoded %u but got %u on decode", knownSize, decodedSize);

		if (kSaveToDesktop)
		{
			size_t Size = 0;
			encoded = surface::encode(decoded
				, surface::file_format::bmp
				, surface::alpha_option::force_no_alpha
				, surface::compression::none);
			mstring fname;
			snprintf(fname, "encoded_from_decoded_%s.%s", as_string(file_format), as_string(file_format));
			filesystem::save(filesystem::desktop_path() / path(fname), encoded, encoded.size());
		}

		float rms = surface::calc_rms(known, decoded);
		if (rms > max_rms)
			oTHROW(io_error, "encoded/decoded bytes mismatch for %s", as_string(file_format));
	}
}

void TESTsurface_codec(test_services& _Services)
{
	compare_checkboards(int2(11,21), surface::b8g8r8a8_unorm, surface::file_format::tga, 1.0f);
	compare_checkboards(int2(11,21), surface::b8g8r8a8_unorm, surface::file_format::bmp, 1.0f);
	compare_checkboards(int2(11,21), surface::b8g8r8a8_unorm, surface::file_format::png, 1.0f);
	//compare_checkboards(int2(11,21), surface::b8g8r8a8_unorm, surface::file_format::jpg, 4.0f);
}

	} // namespace tests
} // namespace ouro
