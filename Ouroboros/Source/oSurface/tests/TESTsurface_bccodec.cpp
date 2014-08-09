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
#include <oPlatform/oTest.h>
#include <oSurface/surface.h>
#include <oSurface/codec.h>

#include "../../test_services.h"

using namespace ouro::surface;

namespace ouro {
	namespace tests {

static void convert_and_test(test_services& services, const format& target_format, const char* filename_suffix, uint nth_test)
{
	path TestImagePath = "Test/Textures/lena_1.png";

	auto file = services.load_buffer(TestImagePath);
	auto source = decode(file, file.size());
	
	oTRACEA("Converting image to %s...", as_string(target_format));
	auto converted1 = source.convert(has_alpha(target_format) ? format::r8g8b8a8_unorm : format::r8g8b8x8_unorm);
	auto converted2 = converted1.convert(target_format);
	auto converted_encoded = encode(converted2, file_format::dds);

	// load the confirmed-good file and compare
	sstring fname;
	snprintf(fname, "GoldenImages/oSurface_bccodec%u.dds", nth_test);
	auto golden_file = services.load_buffer(fname);
	
	oCHECK(converted_encoded.size() == golden_file.size(), "size mismatch (%u != %u)", converted_encoded.size(), golden_file.size());
	oCHECK(!memcmp(converted_encoded, golden_file, golden_file.size()), "bytes mismatch");
}

void TESTsurface_bccodec(test_services& services)
{
	convert_and_test(services, format::bc1_unorm, "_BC1", 0);
	convert_and_test(services, format::bc3_unorm, "_BC3", 1);
	convert_and_test(services, format::bc7_unorm, "_BC7", 2);
	//convert_and_test(services, format::bc6h_uf16, "_BC6HU", 2);
}

	} // namespace tests
} // namespace ouro
