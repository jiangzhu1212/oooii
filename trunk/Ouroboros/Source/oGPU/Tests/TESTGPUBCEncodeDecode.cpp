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
#include <oPlatform/oImage.h>
#include <oPlatform/oTest.h>

struct GPU_BCEncodeDecode : public oTest
{
	RESULT LoadOriginalAndSaveConverted(char* _StrStatus, size_t _SizeofStrStatus, oGPUDevice* _pDevice, oSURFACE_FORMAT _TargetFormat, const char* _OriginalPath, const char* _ConvertedPath)
	{
		oStd::intrusive_ptr<oBuffer> OriginalFile;
		oTESTB(oBufferLoad(_OriginalPath, &OriginalFile), "Failed to load %s", _OriginalPath);

		oGPU_TEXTURE_DESC d;
		d.Dimensions = int3(oDEFAULT, oDEFAULT, 1);
		d.ArraySize = oDEFAULT;
		d.Format = oSURFACE_UNKNOWN;

		oStd::intrusive_ptr<oGPUTexture> OriginalAsTexture;
		oTESTB(oGPUTextureLoad(_pDevice, d, "Source Texture", OriginalFile->GetData(), OriginalFile->GetSize(), &OriginalAsTexture), "Failed to parse %s", OriginalFile->GetName());

		oStd::intrusive_ptr<oGPUTexture> ConvertedTexture;
		oTESTB(oGPUSurfaceConvert(OriginalAsTexture, _TargetFormat, &ConvertedTexture), "Failed to convert %s to %s", OriginalFile->GetName(), oStd::as_string(_TargetFormat));

		oStreamDelete(_ConvertedPath);

		oTESTB(oGPUTextureSave(ConvertedTexture, oGPU_FILE_FORMAT_DDS, _ConvertedPath), "Failed to save %s", _ConvertedPath);
		return oTest::SUCCESS;
	}

	RESULT LoadConvertedAndConvertToImage(char* _StrStatus, size_t _SizeofStrStatus, oGPUDevice* _pDevice, const char* _ConvertedPath, oImage** _ppConvertedImage)
	{
		oStd::intrusive_ptr<oBuffer> ConvertedFile;
		oTESTB(oBufferLoad(_ConvertedPath, &ConvertedFile), "Failed to load %s", _ConvertedPath);

		oGPUTexture::DESC td;
		td.Dimensions = int3(oDEFAULT, oDEFAULT, 1);
		td.ArraySize = oDEFAULT;
		td.Format = oSURFACE_UNKNOWN;
		td.Type = oGPU_TEXTURE_2D_READBACK;

		oStd::intrusive_ptr<oGPUTexture> ConvertedFileAsTexture;
		oTESTB(oGPUTextureLoad(_pDevice, td, "Converted Texture", ConvertedFile->GetData(), ConvertedFile->GetSize(), &ConvertedFileAsTexture), "Failed to parse %s", ConvertedFile->GetName());

		oStd::intrusive_ptr<oGPUTexture> BGRATexture;
		oTESTB(oGPUSurfaceConvert(ConvertedFileAsTexture, oSURFACE_B8G8R8A8_UNORM, &BGRATexture), "Failed to convert %s to BGRA", ConvertedFile->GetName());

		BGRATexture->GetDesc(&td);

		oImage::DESC d;
		d.Dimensions = td.Dimensions.xy();
		d.Format = oImageFormatFromSurfaceFormat(td.Format);
		d.RowPitch = oImageCalcRowPitch(d.Format, d.Dimensions.x);

		oStd::intrusive_ptr<oImage> ConvertedImage;
		oTESTB(oImageCreate("ConvertedImage", d, &ConvertedImage), "Failed to create a compatible oImage");

		oSURFACE_MAPPED_SUBRESOURCE msrSource;
		_pDevice->MapRead(BGRATexture, 0, &msrSource, true);
		int2 ByteDimensions = oSurfaceMipCalcByteDimensions(td.Format, td.Dimensions);
		oStd::memcpy2d(ConvertedImage->GetData(), d.RowPitch, msrSource.pData, msrSource.RowPitch, ByteDimensions.x, ByteDimensions.y);
		_pDevice->UnmapRead(BGRATexture, 0);

		*_ppConvertedImage = ConvertedImage;
		(*_ppConvertedImage)->Reference();
		return oTest::SUCCESS;
	}

	RESULT ConvertAndTest(char* _StrStatus, size_t _SizeofStrStatus, oGPUDevice* _pDevice, oSURFACE_FORMAT _TargetFormat, const char* _FilenameSuffix, unsigned int _NthTest)
	{
		static const char* TestImageFilename = "Test/Textures/lena_1.png";

		oStd::path ImagePath;
		oTESTB0(FindInputFile(ImagePath, TestImageFilename));

		char base[64];
		oGetFilebase(base, ImagePath);
		char fn[64];
		oPrintf(fn, "%s%s.dds", base, _FilenameSuffix);

		oStd::path ConvertedPath;
		oTESTB0(BuildPath(ConvertedPath, fn, oTest::TEMP));

		oTRACE("Converting image to %s (may take a while)...", oStd::as_string(_TargetFormat));
		RESULT res = LoadOriginalAndSaveConverted(_StrStatus, _SizeofStrStatus, _pDevice, _TargetFormat, ImagePath, ConvertedPath);
		if (SUCCESS != res)
			return res;

		oTRACE("Converting image back from %s (may take a while)...", oStd::as_string(_TargetFormat));
		oStd::intrusive_ptr<oImage> ConvertedImage;
		res = LoadConvertedAndConvertToImage(_StrStatus, _SizeofStrStatus, _pDevice, ConvertedPath, &ConvertedImage);
		if (SUCCESS != res)
				return res;

		// Even on different series AMD cards there is a bit of variation, so use a 
		// more forgiving tolerance
		if (_TargetFormat == oSURFACE_BC7_UNORM)
			oTESTI_CUSTOM_TOLERANCE(ConvertedImage, _NthTest, oDEFAULT, 7.2f, oDEFAULT);
		else
			oTESTI2(ConvertedImage, _NthTest);

		return SUCCESS;
	}

	RESULT Run(char* _StrStatus, size_t _SizeofStrStatus) override
	{
		oGPUDevice::INIT DeviceInit("TESTBCEncDec Temp Device");
		#ifdef _DEBUG
			DeviceInit.DriverDebugLevel = oGPU_DEBUG_NORMAL;
		#endif

		oStd::intrusive_ptr<oGPUDevice> Device;
		if (!oGPUDeviceCreate(DeviceInit, &Device))
		{
			#if 1
				oPrintf(_StrStatus, _SizeofStrStatus, "Non-D3D or Pre-D3D11 HW detected: Using the non-accelerated path will take too long, so this test will be skipped.");
				return SKIPPED;
			#else
				DeviceInit.UseSoftwareEmulation = true;
				oTESTB(oGPUDeviceCreate(DeviceInit, &Device), "Failed to create device");
			#endif
		}

		RESULT res = ConvertAndTest(_StrStatus, _SizeofStrStatus, Device, oSURFACE_BC7_UNORM, "_BC7", 0);
		if (SUCCESS != res)
			return res;

		res = ConvertAndTest(_StrStatus, _SizeofStrStatus, Device, oSURFACE_BC6H_SF16, "_BC6HS", 1);
		if (SUCCESS != res)
			return res;

		res = ConvertAndTest(_StrStatus, _SizeofStrStatus, Device, oSURFACE_BC6H_UF16, "_BC6HU", 2);
		if (SUCCESS != res)
			return res;

		return SUCCESS;
	}
};

oTEST_REGISTER(GPU_BCEncodeDecode);
