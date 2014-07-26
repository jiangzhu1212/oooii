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

#if 0

static void load_original_and_save_converted(test_services& services, surface::format target_format, const char* original_path, const path& converted_path)
{
	scoped_allocation file = services.load_buffer(original_path);
	auto buffer = decode(file, file.size());
	auto converted = buffer->convert(surface::x8b8g8r8_unorm);
	
	filesystem::remove(converted_path);

	// save to dds.
}

static std::shared_ptr<surface::texel_buffer> load_converted_and_convert_to_image(test_services& _Services, const char* _ConvertedPath)
{
	scoped_allocation ConvertedFile = _Services.load_buffer(_ConvertedPath);
#if 0

	texture1_info i;
	i.dimensions = ushort3(0, 0, 1);
	i.array_size = 0;
	i.format = surface::unknown;
	i.type = texture_type::readback_2d;


	std::shared_ptr<texture1> ConvertedFileAsTexture;
	std::shared_ptr<texture1> BGRATexture;

	oTESTB(oGPUTextureLoad(_pDevice, i, "Converted Texture", ConvertedFile->GetData(), ConvertedFile->GetSize(), &ConvertedFileAsTexture), "Failed to parse %s", ConvertedFile->GetName());

	oTESTB(oGPUSurfaceConvert(ConvertedFileAsTexture, surface::b8g8r8a8_unorm, &BGRATexture), "Failed to convert %s to BGRA", ConvertedFile->GetName());

	i = BGRATexture->get_info();

	surface::info si;
	si.format = i.format;
	si.dimensions = i.dimensions;
	std::shared_ptr<surface::texel_buffer> ConvertedImage = surface::texel_buffer::make(si);

	surface::mapped_subresource msrSource;
	_pDevice->map_read(BGRATexture, 0, &msrSource, true);
	ConvertedImage->update_subresource(0, msrSource);
	_pDevice->unmap_read(BGRATexture, 0);
#endif
	return nullptr;//ConvertedImage;
}

static void convert_and_test(test_services& services, surface::format target_format, const char* filename_suffix, uint nth_test)
{
	path TestImagePath = "Test/Textures/lena_1.png";

	char fn[64];
	snprintf(fn, "%s%s.dds", TestImagePath.basename().c_str(), filename_suffix);
	path ConvertedPath = ouro::filesystem::temp_path() / fn;

	oTRACEA("Converting image to %s (may take a while)...", as_string(target_format));
	load_original_and_save_converted(services, target_format, TestImagePath, ConvertedPath);

	oTRACEA("Converting image back from %s (may take a while)...", as_string(target_format));
	std::shared_ptr<surface::texel_buffer> ConvertedImage = load_converted_and_convert_to_image(services, ConvertedPath);

	// Even on different series AMD cards there is a bit of variation so use a 
	// more forgiving tolerance
	// THIS WONT BE TRUE WITH A CPU ENCODER (I HOPE)
	//const float kMoreForgivingTolerance = 7.2f;
	//if (target_format == surface::bc7_unorm)
	//	services.check(ConvertedImage, nth_test, kMoreForgivingTolerance);
	//else
		services.check(ConvertedImage, nth_test);
}
#endif
void TESTbccodec(test_services& services)
{
	oTHROW(operation_not_supported, "need a dds writer");
	//convert_and_test(services, surface::bc7_unorm, "_BC7", 0);
	//convert_and_test(services, surface::bc6h_sf16, "_BC6HS", 1);
	//convert_and_test(services, surface::bc6h_uf16, "_BC6HU", 2);
}

	} // namespace tests
} // namespace ouro
