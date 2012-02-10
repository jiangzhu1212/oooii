/**************************************************************************
 * The MIT License                                                        *
 * Copyright (c) 2011 Antony Arciuolo & Kevin Myers                       *
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
#include <oPlatform/oD3D11.h>
#include <oPlatform/oFile.h>
#include <oPlatform/oImage.h>
#include <oPlatform/oSystem.h>
#include <oPlatform/oTest.h>

struct TESTBCEncodeDecode : public oTest
{
	RESULT LoadOriginalAndSaveConverted(char* _StrStatus, size_t _SizeofStrStatus, ID3D11Device* _pDevice, DXGI_FORMAT _TargetFormat, const char* _OriginalPath, const char* _ConvertedPath)
	{
		oRef<oBuffer> OriginalFile;
		oTESTB(oBufferCreate(_OriginalPath, &OriginalFile), "Failed to load %s", _OriginalPath);

		oRef<ID3D11Texture2D> OriginalAsTexture;
		oTESTB(oD3D11Load(_pDevice, DXGI_FORMAT_FROM_FILE, oD3D11_DYNAMIC_TEXTURE, "Source Texture", OriginalFile->GetData(), OriginalFile->GetSize(), (ID3D11Resource**)&OriginalAsTexture), "Failed to parse %s", OriginalFile->GetName());

		oRef<ID3D11Texture2D> ConvertedTexture;
		oTESTB(oD3D11Convert(OriginalAsTexture, _TargetFormat, &ConvertedTexture), "Failed to convert %s to %s", OriginalFile->GetName(), oAsString(_TargetFormat));

		oFileDelete(_ConvertedPath);
		oTESTB(oD3D11Save(ConvertedTexture, D3DX11_IFF_DDS, _ConvertedPath), "Failed to save %s", _ConvertedPath);
		return oTest::SUCCESS;
	}

	RESULT LoadConvertedAndConvertToImage(char* _StrStatus, size_t _SizeofStrStatus, ID3D11Device* _pDevice, const char* _ConvertedPath, oImage** _ppConvertedImage)
	{
		oRef<oBuffer> ConvertedFile;
		oTESTB(oBufferCreate(_ConvertedPath, &ConvertedFile), "Failed to load %s", _ConvertedPath);

		oRef<ID3D11Texture2D> ConvertedFileAsTexture;
		oTESTB(oD3D11Load(_pDevice, DXGI_FORMAT_FROM_FILE, oD3D11_DYNAMIC_TEXTURE, "Converted Texture", ConvertedFile->GetData(), ConvertedFile->GetSize(), (ID3D11Resource**)&ConvertedFileAsTexture), "Failed to parse %s", ConvertedFile->GetName());

		oRef<ID3D11Texture2D> BGRATexture;
		oTESTB(oD3D11Convert(ConvertedFileAsTexture, DXGI_FORMAT_B8G8R8A8_UNORM, &BGRATexture), "Failed to convert %s to BGRA", ConvertedFile->GetName());

		D3D11_TEXTURE2D_DESC TexDesc;
		BGRATexture->GetDesc(&TexDesc);

		oImage::DESC d;
		d.Dimensions = int2(TexDesc.Width, TexDesc.Height);
		d.Format = oImage::BGRA32;
		d.RowPitch = oImageCalcRowPitch(d.Format, d.Dimensions.x);

		oRef<oImage> ConvertedImage;
		oTESTB(oImageCreate("ConvertedImage", d, &ConvertedImage), "Failed to create a compatible oImage");
		oTESTB(oD3D11CopyToBuffer(BGRATexture, ConvertedImage->GetData(), d.RowPitch, true), "Failed to copy texture data to oImage");

		*_ppConvertedImage = ConvertedImage;
		(*_ppConvertedImage)->Reference();
		return oTest::SUCCESS;
	}

	RESULT ConvertAndTest(char* _StrStatus, size_t _SizeofStrStatus, ID3D11Device* _pDevice, DXGI_FORMAT _TargetFormat, const char* _FilenameSuffix, unsigned int _NthTest)
	{
		static const char* TestImageFilename = "Test/Textures/lena_1.png";

		oStringPath ImagePath;
		oTESTB0(FindInputFile(ImagePath, TestImageFilename));

		char base[64];
		oGetFilebase(base, ImagePath);
		char fn[64];
		sprintf_s(fn, "%s%s.dds", base, _FilenameSuffix);

		oStringPath ConvertedPath;
		oTESTB0(BuildPath(ConvertedPath, fn, oTest::TEMP));

		oTRACE("Converting image to %s (may take a while)...", oAsString(_TargetFormat));
		RESULT res = LoadOriginalAndSaveConverted(_StrStatus, _SizeofStrStatus, _pDevice, _TargetFormat, ImagePath, ConvertedPath);
		if (SUCCESS != res)
			return res;

		oTRACE("Converting image back from %s (may take a while)...", oAsString(_TargetFormat));
		oRef<oImage> ConvertedImage;
		res = LoadConvertedAndConvertToImage(_StrStatus, _SizeofStrStatus, _pDevice, ConvertedPath, &ConvertedImage);
		if (SUCCESS != res)
				return res;

		oTESTI2(ConvertedImage, _NthTest);
		return SUCCESS;
	}

	RESULT Run(char* _StrStatus, size_t _SizeofStrStatus) override
	{
		oD3D11_DEVICE_DESC DeviceDesc("TESTBCEncDec Temp Device");
		#ifdef _DEBUG
			DeviceDesc.Debug = true;
		#endif
		oRef<ID3D11Device> D3DDevice;
		if (!oD3D11CreateDevice(DeviceDesc, &D3DDevice))
		{
			DeviceDesc.Accelerated = false;
			oTESTB(oD3D11CreateDevice(DeviceDesc, &D3DDevice), "Failed to create device");
		}

		RESULT res = ConvertAndTest(_StrStatus, _SizeofStrStatus, D3DDevice, DXGI_FORMAT_BC7_UNORM, "_BC7", 0);
		if (SUCCESS != res)
			return res;

		res = ConvertAndTest(_StrStatus, _SizeofStrStatus, D3DDevice, DXGI_FORMAT_BC6H_SF16, "_BC6HS", 1);
		if (SUCCESS != res)
			return res;

		res = ConvertAndTest(_StrStatus, _SizeofStrStatus, D3DDevice, DXGI_FORMAT_BC6H_UF16, "_BC6HU", 2);
		if (SUCCESS != res)
			return res;

		return SUCCESS;
	}
};

oTEST_REGISTER(TESTBCEncodeDecode);
