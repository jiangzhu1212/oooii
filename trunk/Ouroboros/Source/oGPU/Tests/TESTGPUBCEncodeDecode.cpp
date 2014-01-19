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
#include <oPlatform/oStream.h>
#include <oPlatform/oStreamUtil.h>
#include <oPlatform/oTest.h>

using namespace ouro;

struct GPU_BCEncodeDecode : public oTest
{
	RESULT LoadOriginalAndSaveConverted(char* _StrStatus, size_t _SizeofStrStatus, oGPUDevice* _pDevice, ouro::surface::format _TargetFormat, const char* _OriginalPath, const char* _ConvertedPath)
	{
		intrusive_ptr<oBuffer> OriginalFile;
		oTESTB(oBufferLoad(_OriginalPath, &OriginalFile), "Failed to load %s", _OriginalPath);

		ouro::gpu::texture_info d;
		d.dimensions = ushort3(0, 0, 1);
		d.array_size = 0;
		d.format = ouro::surface::unknown;

		intrusive_ptr<oGPUTexture> OriginalAsTexture;
		oTESTB(oGPUTextureLoad(_pDevice, d, "Source Texture", OriginalFile->GetData(), OriginalFile->GetSize(), &OriginalAsTexture), "Failed to parse %s", OriginalFile->GetName());

		intrusive_ptr<oGPUTexture> ConvertedTexture;
		oTESTB(oGPUSurfaceConvert(OriginalAsTexture, _TargetFormat, &ConvertedTexture), "Failed to convert %s to %s", OriginalFile->GetName(), ouro::as_string(_TargetFormat));

		oStreamDelete(_ConvertedPath);

		oTESTB(oGPUTextureSave(ConvertedTexture, oGPU_FILE_FORMAT_DDS, _ConvertedPath), "Failed to save %s", _ConvertedPath);
		return oTest::SUCCESS;
	}

	RESULT LoadConvertedAndConvertToImage(char* _StrStatus, size_t _SizeofStrStatus, oGPUDevice* _pDevice, const char* _ConvertedPath, std::shared_ptr<ouro::surface::buffer>* _ppConvertedImage)
	{
		intrusive_ptr<oBuffer> ConvertedFile;
		oTESTB(oBufferLoad(_ConvertedPath, &ConvertedFile), "Failed to load %s", _ConvertedPath);

		oGPUTexture::DESC td;
		td.dimensions = ushort3(0, 0, 1);
		td.array_size = 0;
		td.format = ouro::surface::unknown;
		td.type = ouro::gpu::texture_type::readback_2d;

		intrusive_ptr<oGPUTexture> ConvertedFileAsTexture;
		oTESTB(oGPUTextureLoad(_pDevice, td, "Converted Texture", ConvertedFile->GetData(), ConvertedFile->GetSize(), &ConvertedFileAsTexture), "Failed to parse %s", ConvertedFile->GetName());

		intrusive_ptr<oGPUTexture> BGRATexture;
		oTESTB(oGPUSurfaceConvert(ConvertedFileAsTexture, ouro::surface::b8g8r8a8_unorm, &BGRATexture), "Failed to convert %s to BGRA", ConvertedFile->GetName());

		BGRATexture->GetDesc(&td);

		ouro::surface::info si;
		si.format = td.format;
		si.dimensions = td.dimensions;
		std::shared_ptr<ouro::surface::buffer> ConvertedImage = ouro::surface::buffer::make(si);

		ouro::surface::mapped_subresource msrSource;
		_pDevice->MapRead(BGRATexture, 0, &msrSource, true);
		ConvertedImage->update_subresource(0, msrSource);
		_pDevice->UnmapRead(BGRATexture, 0);

		*_ppConvertedImage = ConvertedImage;
		return oTest::SUCCESS;
	}

	RESULT ConvertAndTest(char* _StrStatus, size_t _SizeofStrStatus, oGPUDevice* _pDevice, ouro::surface::format _TargetFormat, const char* _FilenameSuffix, unsigned int _NthTest)
	{
		static const char* TestImageFilename = "Test/Textures/lena_1.png";

		path ImagePath;
		oTESTB0(FindInputFile(ImagePath, TestImageFilename));

		char fn[64];
		snprintf(fn, "%s%s.dds", ImagePath.basename().c_str(), _FilenameSuffix);

		path ConvertedPath;
		oTESTB0(BuildPath(ConvertedPath, fn, oTest::TEMP));

		oTRACE("Converting image to %s (may take a while)...", ouro::as_string(_TargetFormat));
		RESULT res = LoadOriginalAndSaveConverted(_StrStatus, _SizeofStrStatus, _pDevice, _TargetFormat, ImagePath, ConvertedPath);
		if (SUCCESS != res)
			return res;

		oTRACE("Converting image back from %s (may take a while)...", ouro::as_string(_TargetFormat));
		std::shared_ptr<ouro::surface::buffer> ConvertedImage;
		res = LoadConvertedAndConvertToImage(_StrStatus, _SizeofStrStatus, _pDevice, ConvertedPath, &ConvertedImage);
		if (SUCCESS != res)
				return res;

		// Even on different series AMD cards there is a bit of variation, so use a 
		// more forgiving tolerance
		if (_TargetFormat == ouro::surface::bc7_unorm)
			oTESTI_CUSTOM_TOLERANCE(ConvertedImage, _NthTest, oDEFAULT, 7.2f, oDEFAULT);
		else
			oTESTI2(ConvertedImage, _NthTest);

		return SUCCESS;
	}

	RESULT Run(char* _StrStatus, size_t _SizeofStrStatus) override
	{
		oGPUDevice::INIT DeviceInit("TESTBCEncDec Temp Device");
		#ifdef _DEBUG
			DeviceInit.driver_debug_level = gpu::debug_level::normal;
		#endif

		intrusive_ptr<oGPUDevice> Device;
		if (!oGPUDeviceCreate(DeviceInit, &Device))
		{
			#if 1
				snprintf(_StrStatus, _SizeofStrStatus, "Non-D3D or Pre-D3D11 HW detected: Using the non-accelerated path will take too long, so this test will be skipped.");
				return SKIPPED;
			#else
				DeviceInit.use_software_emulation = true;
				oTESTB(oGPUDeviceCreate(DeviceInit, &Device), "Failed to create device");
			#endif
		}

		oGPUDevice::DESC desc;
		Device->GetDesc(&desc);

		if (desc.vendor != vendor::nvidia && desc.vendor != vendor::amd)
		{
			snprintf(_StrStatus, _SizeofStrStatus, "%s gpu device not trusted to run the DXSDK BC7 sample code correctly, so skip this test.", as_string(desc.vendor));
			return SKIPPED;
		}

		RESULT res = ConvertAndTest(_StrStatus, _SizeofStrStatus, Device, ouro::surface::bc7_unorm, "_BC7", 0);
		if (SUCCESS != res)
			return res;

		res = ConvertAndTest(_StrStatus, _SizeofStrStatus, Device, ouro::surface::bc6h_sf16, "_BC6HS", 1);
		if (SUCCESS != res)
			return res;

		res = ConvertAndTest(_StrStatus, _SizeofStrStatus, Device, ouro::surface::bc6h_uf16, "_BC6HU", 2);
		if (SUCCESS != res)
			return res;

		return SUCCESS;
	}
};

oTEST_REGISTER(GPU_BCEncodeDecode);
