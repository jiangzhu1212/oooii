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
#include <oGPU/oGPU.h>
#include <oGPU/oGPUUtil.h>

struct GPU_GenerateMips : public oTest
{
	RESULT TestImageMips1D(char* _StrStatus, size_t _SizeofStrStatus, oGPUDevice* _pDevice, int _Width, oSURFACE_LAYOUT _Layout, int _StartIndex)
	{
		oImage::DESC imageDesc;
		imageDesc.Dimensions = int2(_Width, 1);
		imageDesc.Format = oImage::BGRA32;
		imageDesc.RowPitch = oImageCalcRowPitch(imageDesc.Format, imageDesc.Dimensions.x);
		oStd::intrusive_ptr<oImage> image;
		oImageCreate("GPU_Texture1D", imageDesc, &image);

		oStd::intrusive_ptr<oBuffer> buffer;
		int surfaceBufferSize = oImageCalcSize(imageDesc.Format, imageDesc.Dimensions);
		void *pSurfaceBuffer = oBuffer::New(surfaceBufferSize);
		oBufferCreate("GPU_Texture1D buffer", pSurfaceBuffer, surfaceBufferSize, oBuffer::Delete, &buffer);

		static const oStd::color sConsoleColors[] = { oStd::Black, oStd::Navy, oStd::Green, oStd::Teal, oStd::Maroon, oStd::Purple, oStd::Olive, oStd::Silver, oStd::Gray, oStd::Blue, oStd::Lime, oStd::Aqua, oStd::Red, oStd::Fuchsia, oStd::Yellow, oStd::White };

		oStd::color* texture1Ddata = (oStd::color*)buffer->GetData();
		for (int i=0; i<imageDesc.Dimensions.x; ++i)
		{
			texture1Ddata[i] = sConsoleColors[i % oCOUNTOF(sConsoleColors)];
		}

		image->CopyData(buffer->GetData(), imageDesc.RowPitch);

		oSURFACE_DESC sd;
		sd.Dimensions = int3(imageDesc.Dimensions, 1);
		sd.Format = oImageFormatToSurfaceFormat(imageDesc.Format);
		sd.Layout = _Layout;

		oStd::intrusive_ptr<oImage> mipImage;
		oTESTB(oImageCreate("TestImageMips1D", sd, &mipImage), "Failed to create image for mipped surface");

		oTESTB(oGPUGenerateMips(_pDevice, (const oImage**)&image, 1, sd, oGPU_TEXTURE_1D_MAP, mipImage), "Failed to generate mips with the GPU");

		oTESTI2(mipImage, _StartIndex);

		return SUCCESS;
	}

	RESULT TestImageMips2D(char* _StrStatus, size_t _SizeofStrStatus, oGPUDevice* _pDevice, char* _pFilename, oSURFACE_LAYOUT _Layout, int _StartIndex)
	{
		oStd::intrusive_ptr<oImage> image;
		oTESTB(oImageLoad(_pFilename, oImage::FORCE_ALPHA, &image), "Failed to load image");

		oImage::DESC id;
		image->GetDesc(&id);

		oSURFACE_DESC sd;
		sd.Dimensions = int3(id.Dimensions, 1);
		sd.Format = oImageFormatToSurfaceFormat(id.Format);
		sd.Layout = _Layout;

		oStd::intrusive_ptr<oImage> mipImage;
		oTESTB(oImageCreate("TestImageMips2D", sd, &mipImage), "Failed to create image for mipped surface");

		oTESTB(oGPUGenerateMips(_pDevice, (const oImage**)&image, 1, sd, oGPU_TEXTURE_2D_MAP, mipImage), "Failed to generate mips with the GPU");

		oTESTI2(mipImage, _StartIndex);

		return SUCCESS;
	}

	RESULT TestImageMips3D(char* _StrStatus, size_t _SizeofStrStatus, oGPUDevice* _pDevice, char* _pFilename, oSURFACE_LAYOUT _Layout, int _StartIndex)
	{
		oStd::intrusive_ptr<oImage> images[5];
		oTESTB(oImageLoad(_pFilename, oImage::FORCE_ALPHA, &images[0]), "Failed to load image");
		images[4] = images[3] = images[2] = images[1] = images[0];

		oImage::DESC id;
		images[0]->GetDesc(&id);

		oSURFACE_DESC sd;
		sd.Dimensions = int3(id.Dimensions, oCOUNTOF(images));
		sd.Format = oImageFormatToSurfaceFormat(id.Format);
		sd.Layout = _Layout;

		oStd::intrusive_ptr<oImage> mipImage;
		oTESTB(oImageCreate("TestImageMips3D", sd, &mipImage), "Failed to create image for mipped surface");

		oTESTB(oGPUGenerateMips(_pDevice, (const oImage**)&images, oCOUNTOF(images), sd, oGPU_TEXTURE_3D_MAP, mipImage), "Failed to generate mips with the GPU");

		oTESTI2(mipImage, _StartIndex);

		return SUCCESS;
	}

	RESULT TestImageMipsCube(char* _StrStatus, size_t _SizeofStrStatus, oGPUDevice* _pDevice, oSURFACE_LAYOUT _Layout, int _StartIndex)
	{
		oStd::intrusive_ptr<oImage> images[6];
		oTESTB(oImageLoad("file://DATA/Test/Textures/CubePosX.png", oImage::FORCE_ALPHA, &images[0]), "Failed to load image +X");
		oTESTB(oImageLoad("file://DATA/Test/Textures/CubeNegX.png", oImage::FORCE_ALPHA, &images[1]), "Failed to load image -X");
		oTESTB(oImageLoad("file://DATA/Test/Textures/CubePosY.png", oImage::FORCE_ALPHA, &images[2]), "Failed to load image +Y");
		oTESTB(oImageLoad("file://DATA/Test/Textures/CubeNegY.png", oImage::FORCE_ALPHA, &images[3]), "Failed to load image -Y");
		oTESTB(oImageLoad("file://DATA/Test/Textures/CubePosZ.png", oImage::FORCE_ALPHA, &images[4]), "Failed to load image +Z");
		oTESTB(oImageLoad("file://DATA/Test/Textures/CubeNegZ.png", oImage::FORCE_ALPHA, &images[5]), "Failed to load image -Z");

		oImage::DESC id;
		images[0]->GetDesc(&id);

		oSURFACE_DESC sd;
		sd.Dimensions = int3(id.Dimensions, 1);
		sd.Format = oImageFormatToSurfaceFormat(id.Format);
		sd.ArraySize = oCOUNTOF(images);
		sd.Layout = _Layout;

		oStd::intrusive_ptr<oImage> mipImage;
		oTESTB(oImageCreate("TestImageMipsCube", sd, &mipImage), "Failed to create image for mipped surface");

		oTESTB(oGPUGenerateMips(_pDevice, (const oImage**)&images, oCOUNTOF(images), sd, oGPU_TEXTURE_CUBE_MAP, mipImage), "Failed to generate mips with the GPU");

		oTESTI2(mipImage, _StartIndex);

		return SUCCESS;
	}

	RESULT Run(char* _StrStatus, size_t _SizeofStrStatus) override
	{
		oGPUDevice::INIT init("GPU_GenerateMips");
		init.Version = oStd::version(10,0); // for more compatibility when running on varied machines
		oStd::intrusive_ptr<oGPUDevice> Device;
		oTESTB0(oGPUDeviceCreate(init, &Device));

		// 1D non power of 2
		RESULT res = TestImageMips1D(_StrStatus, _SizeofStrStatus, Device, 227, oSURFACE_LAYOUT_TIGHT, 0);
		if (SUCCESS != res) 
			return res;

		res = TestImageMips1D(_StrStatus, _SizeofStrStatus, Device, 227, oSURFACE_LAYOUT_BELOW, 1);
		if (SUCCESS != res) 
			return res;

		res = TestImageMips1D(_StrStatus, _SizeofStrStatus, Device, 227, oSURFACE_LAYOUT_RIGHT, 2);
		if (SUCCESS != res) 
			return res;


		// 1D power of 2
		res = TestImageMips1D(_StrStatus, _SizeofStrStatus, Device, 512, oSURFACE_LAYOUT_TIGHT, 3);
		if (SUCCESS != res) 
			return res;

		res = TestImageMips1D(_StrStatus, _SizeofStrStatus, Device, 512, oSURFACE_LAYOUT_BELOW, 4);
		if (SUCCESS != res) 
			return res;

		res = TestImageMips1D(_StrStatus, _SizeofStrStatus, Device, 512, oSURFACE_LAYOUT_RIGHT, 5);
		if (SUCCESS != res) 
			return res;

		// 2D non power of 2
		res = TestImageMips2D(_StrStatus, _SizeofStrStatus, Device, "file://DATA/Test/Textures/lena_npot.png", oSURFACE_LAYOUT_TIGHT, 6);
		if (SUCCESS != res) 
			return res;

		res = TestImageMips2D(_StrStatus, _SizeofStrStatus, Device, "file://DATA/Test/Textures/lena_npot.png", oSURFACE_LAYOUT_BELOW, 7);
		if (SUCCESS != res) 
			return res;

		res = TestImageMips2D(_StrStatus, _SizeofStrStatus, Device, "file://DATA/Test/Textures/lena_npot.png", oSURFACE_LAYOUT_RIGHT, 8);
		if (SUCCESS != res) 
			return res;

		// 2D power of 2
		res = TestImageMips2D(_StrStatus, _SizeofStrStatus, Device, "file://DATA/Test/Textures/lena_1.png", oSURFACE_LAYOUT_TIGHT, 9);
		if (SUCCESS != res) 
			return res;

		res = TestImageMips2D(_StrStatus, _SizeofStrStatus, Device, "file://DATA/Test/Textures/lena_1.png", oSURFACE_LAYOUT_BELOW, 10);
		if (SUCCESS != res) 
			return res;

		res = TestImageMips2D(_StrStatus, _SizeofStrStatus, Device, "file://DATA/Test/Textures/lena_1.png", oSURFACE_LAYOUT_RIGHT, 11);
		if (SUCCESS != res) 
			return res;

		// 3D non power of 2
		res = TestImageMips3D(_StrStatus, _SizeofStrStatus, Device, "file://DATA/Test/Textures/lena_npot.png", oSURFACE_LAYOUT_TIGHT, 12);
		if (SUCCESS != res) 
			return res;

		res = TestImageMips3D(_StrStatus, _SizeofStrStatus, Device, "file://DATA/Test/Textures/lena_npot.png", oSURFACE_LAYOUT_BELOW, 13);
		if (SUCCESS != res) 
			return res;

		res = TestImageMips3D(_StrStatus, _SizeofStrStatus, Device, "file://DATA/Test/Textures/lena_npot.png", oSURFACE_LAYOUT_RIGHT, 14);
		if (SUCCESS != res) 
			return res;

		// 3D power of 2
		res = TestImageMips3D(_StrStatus, _SizeofStrStatus, Device, "file://DATA/Test/Textures/lena_1.png", oSURFACE_LAYOUT_TIGHT, 15);
		if (SUCCESS != res) 
			return res;

		res = TestImageMips3D(_StrStatus, _SizeofStrStatus, Device, "file://DATA/Test/Textures/lena_1.png", oSURFACE_LAYOUT_BELOW, 16);
		if (SUCCESS != res) 
			return res;

		res = TestImageMips3D(_StrStatus, _SizeofStrStatus, Device, "file://DATA/Test/Textures/lena_1.png", oSURFACE_LAYOUT_RIGHT, 17);
		if (SUCCESS != res) 
			return res;

		// Cube power of 2
		res = TestImageMipsCube(_StrStatus, _SizeofStrStatus, Device, oSURFACE_LAYOUT_TIGHT, 18);
		if (SUCCESS != res) 
			return res;

		res = TestImageMipsCube(_StrStatus, _SizeofStrStatus, Device, oSURFACE_LAYOUT_BELOW, 19);
		if (SUCCESS != res) 
			return res;

		res = TestImageMipsCube(_StrStatus, _SizeofStrStatus, Device, oSURFACE_LAYOUT_RIGHT, 20);
		if (SUCCESS != res) 
			return res;

		return SUCCESS;
	}
};

oTEST_REGISTER(GPU_GenerateMips);
