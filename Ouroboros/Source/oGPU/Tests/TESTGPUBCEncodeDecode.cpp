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

#include "../../test_services.h"

using namespace ouro::surface;

namespace ouro {
	namespace tests {

static void convert_and_test(test_services& services, const format& target_format, const char* filename_suffix, uint nth_test)
{
	path TestImagePath = "Test/Textures/lena_1.png";

	char fn[64];
	snprintf(fn, "%s%s.dds", TestImagePath.basename().c_str(), filename_suffix);
	path ConvertedPath = ouro::filesystem::temp_path() / fn;
	{
		filesystem::remove(ConvertedPath);

		auto file = services.load_buffer(TestImagePath);
		auto source = decode(file, file.size());
	
		oTRACEA("Converting image to %s (may take a while)...", as_string(target_format));
		auto converted1 = source.convert(format::x8b8g8r8_unorm);
		auto converted2 = converted1.convert(target_format);
		auto converted_encoded = encode(converted2, file_format::dds);
	
		filesystem::save(ConvertedPath, converted_encoded, converted_encoded.size());
	}

	// not sure how to decompress it back into something generally readable

	//oTRACEA("Converting image back from %s (may take a while)...", as_string(target_format));
	//auto file = services.load_buffer(ConvertedPath);
	//auto compressed = decode(file, file.size());
	//auto decompressed = compressed.convert(format::b8g8r8a8_unorm);
	//
	//services.check(decompressed, nth_test);
}

void TESTbccodec(test_services& services)
{
	//oTHROW(operation_not_supported, "need a dds writer");
	convert_and_test(services, format::bc1_unorm, "_BC1", 0);
	convert_and_test(services, format::bc7_unorm, "_BC7", 0);
	//convert_and_test(services, format::bc6h_sf16, "_BC6HS", 1);
	//convert_and_test(services, format::bc6h_uf16, "_BC6HU", 2);
}

	} // namespace tests
} // namespace ouro
