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
#include <oSurface/tests/oSurfaceTestRequirements.h>
#include <oBase/timer.h>

namespace ouro { 
	namespace tests {

static void compare_checkboards(const int2& _InDimensions, surface::format _Format, surface::file_format::value _FileFormat)
{
	oTRACE("testing codec %s -> %s", as_string(_Format), as_string(_FileFormat));

	// Create a buffer with a known format
	surface::info si;
	si.format = _Format;
	si.layout = surface::image;
	si.dimensions = int3(_InDimensions, 1);
	si.array_size = 1;
	std::shared_ptr<surface::buffer> known = surface::buffer::make(si);
	size_t knownSize = known->size();
	{
		surface::lock_guard lock(known);
		surface::fill_checkerboard((color*)lock.mapped.data, lock.mapped.row_pitch, si.dimensions.xy(), si.dimensions.xy() / 2, Azure, Salmon);
	}

	size_t EncodedSize = 0;
	std::shared_ptr<char> encoded;

	{
		scoped_timer("encode");
		encoded = surface::encode(known.get()
			, &EncodedSize
			, _FileFormat
			, surface::alpha_option::force_no_alpha
			, surface::compression::none);
	}

	sstring buf;
	format_bytes(buf, EncodedSize, 2);
	oTRACE("encoded %s", buf.c_str());

	std::shared_ptr<surface::buffer> decoded;
	{
		scoped_timer("decode");
		decoded = surface::decode(encoded.get(), EncodedSize, surface::alpha_option::force_alpha);
	}

	{
		size_t decodedSize = decoded->size();

		if (known->size() != decoded->size())
			oTHROW(io_error, "encoded %u but got %u on decode", knownSize, decodedSize);

		surface::shared_lock kn(known);
		surface::shared_lock dec(decoded);

		// todo: change this to an rms calc and thus be able to support compression
		// tests with more leeway in error.
		if (memcmp(kn.mapped.data, dec.mapped.data, decodedSize))
			oTHROW(io_error, "encoded/decoded bytes mismatch");
	}
}

void TESTsurface_codec(requirements& _Requirements)
{
	compare_checkboards(int2(11,21), surface::b8g8r8a8_unorm, surface::file_format::png);
}

	} // namespace tests
} // namespace ouro
