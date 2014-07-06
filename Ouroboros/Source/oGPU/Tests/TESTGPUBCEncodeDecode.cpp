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
#include <oGPU/oGPU.h>
#include <oPlatform/oTest.h>

#include "../../test_services.h"

using namespace ouro::gpu;

namespace ouro {
	namespace tests {

static void load_original_and_save_converted(test_services& _Services, device* _pDevice, surface::format _TargetFormat, const char* _OriginalPath, const path& _ConvertedPath)
{
	scoped_allocation OriginalFile = _Services.load_buffer(_OriginalPath);

	texture1_info i;
	i.dimensions = ushort3(0, 0, 1);
	i.array_size = 0;
	i.format = surface::unknown;

#if 0

	std::shared_ptr<texture> OriginalAsTexture = ?;
	oTESTB(oGPUTextureLoad(_pDevice, d, "Source Texture", OriginalFile->GetData(), OriginalFile->GetSize(), &OriginalAsTexture), "Failed to parse %s", OriginalFile->GetName());

	std::shared_ptr<texture> ConvertedTexture;
	oTESTB(oGPUSurfaceConvert(OriginalAsTexture, _TargetFormat, &ConvertedTexture), "Failed to convert %s to %s", OriginalFile->GetName(), ouro::as_string(_TargetFormat));

	filesystem::remove(_ConvertedPath);

	oTESTB(oGPUTextureSave(ConvertedTexture, oGPU_FILE_FORMAT_DDS, _ConvertedPath), "Failed to save %s", _ConvertedPath);
#else
	oTHROW(permission_denied, "oGPUTextureLoad needs a new impl");
#endif
}

static std::shared_ptr<surface::buffer> load_converted_and_convert_to_image(test_services& _Services, device* _pDevice, const char* _ConvertedPath)
{
	scoped_allocation ConvertedFile = _Services.load_buffer(_ConvertedPath);

	texture1_info i;
	i.dimensions = ushort3(0, 0, 1);
	i.array_size = 0;
	i.format = surface::unknown;
	i.type = texture_type::readback_2d;

	std::shared_ptr<texture1> ConvertedFileAsTexture;
	std::shared_ptr<texture1> BGRATexture;
#if 0
	oTESTB(oGPUTextureLoad(_pDevice, i, "Converted Texture", ConvertedFile->GetData(), ConvertedFile->GetSize(), &ConvertedFileAsTexture), "Failed to parse %s", ConvertedFile->GetName());

	oTESTB(oGPUSurfaceConvert(ConvertedFileAsTexture, surface::b8g8r8a8_unorm, &BGRATexture), "Failed to convert %s to BGRA", ConvertedFile->GetName());
#else
	if (1)
		oTHROW(permission_denied, "oGPUTextureLoad needs a new impl");
#endif

	i = BGRATexture->get_info();

	surface::info si;
	si.format = i.format;
	si.dimensions = i.dimensions;
	std::shared_ptr<surface::buffer> ConvertedImage = surface::buffer::make(si);

	surface::mapped_subresource msrSource;
	_pDevice->map_read(BGRATexture, 0, &msrSource, true);
	ConvertedImage->update_subresource(0, msrSource);
	_pDevice->unmap_read(BGRATexture, 0);

	return ConvertedImage;
}

static void convert_and_test(test_services& _Services, device* _pDevice, surface::format _TargetFormat, const char* _FilenameSuffix, uint _NthTest)
{
	path TestImagePath = "Test/Textures/lena_1.png";

	char fn[64];
	snprintf(fn, "%s%s.dds", TestImagePath.basename().c_str(), _FilenameSuffix);
	path ConvertedPath = ouro::filesystem::temp_path() / fn;

	oTRACEA("Converting image to %s (may take a while)...", as_string(_TargetFormat));
	load_original_and_save_converted(_Services, _pDevice, _TargetFormat, TestImagePath, ConvertedPath);

	oTRACEA("Converting image back from %s (may take a while)...", as_string(_TargetFormat));
	std::shared_ptr<surface::buffer> ConvertedImage = load_converted_and_convert_to_image(_Services, _pDevice, ConvertedPath);

	// Even on different series AMD cards there is a bit of variation so use a 
	// more forgiving tolerance
	const float MORE_FORGIVING_TOLERANCE = 7.2f;
	if (_TargetFormat == surface::bc7_unorm)
		_Services.check(ConvertedImage, _NthTest, MORE_FORGIVING_TOLERANCE);
	else
		_Services.check(ConvertedImage, _NthTest);
}

void TESTbccodec(test_services& _Services)
{
	std::shared_ptr<device> Device;
	{
		device_init i("TESTBCEncDec Temp Device");
		#ifdef _DEBUG
			i.driver_debug_level = gpu::debug_level::normal;
		#endif
		try { Device = device::make(i); }
		catch (std::exception&)
		{
			static const char* msg = "Non-D3D or Pre-D3D11 HW detected: Using the non-accelerated path will take too long, so this test will be skipped.";
			_Services.report(msg);
			oTHROW(permission_denied, msg);
		}
	}

	device_info i = Device->get_info();

	if (i.vendor != vendor::nvidia && i.vendor != vendor::amd)
		oTHROW(permission_denied, "%s gpu device not trusted to run the DXSDK BC7 sample code correctly, so skip this test.", as_string(i.vendor));

	convert_and_test(_Services, Device.get(), surface::bc7_unorm, "_BC7", 0);
	convert_and_test(_Services, Device.get(), surface::bc6h_sf16, "_BC6HS", 1);
	convert_and_test(_Services, Device.get(), surface::bc6h_uf16, "_BC6HU", 2);
}

	} // namespace tests
} // namespace ouro
