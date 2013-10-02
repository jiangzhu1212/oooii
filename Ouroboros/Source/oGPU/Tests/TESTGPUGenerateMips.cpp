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

using namespace ouro;

struct GPU_GenerateMips : public oTest
{
	RESULT TestImageMips1D(char* _StrStatus, size_t _SizeofStrStatus, oGPUDevice* _pDevice, int _Width, ouro::surface::layout _Layout, int _StartIndex)
	{
		oImage::DESC imageDesc;
		imageDesc.Dimensions = int2(_Width, 1);
		imageDesc.Format = oImage::BGRA32;
		imageDesc.RowPitch = oImageCalcRowPitch(imageDesc.Format, imageDesc.Dimensions.x);
		intrusive_ptr<oImage> image;
		oImageCreate("GPU_Texture1D", imageDesc, &image);

		intrusive_ptr<oBuffer> buffer;
		int surfaceBufferSize = oImageCalcSize(imageDesc.Format, imageDesc.Dimensions);
		void *pSurfaceBuffer = oBuffer::New(surfaceBufferSize);
		oBufferCreate("GPU_Texture1D buffer", pSurfaceBuffer, surfaceBufferSize, oBuffer::Delete, &buffer);

		static const color sConsoleColors[] = { Black, Navy, Green, Teal, Maroon, Purple, Olive, Silver, Gray, Blue, Lime, Aqua, Red, Fuchsia, Yellow, White };

		color* texture1Ddata = (color*)buffer->GetData();
		for (int i=0; i<imageDesc.Dimensions.x; ++i)
		{
			texture1Ddata[i] = sConsoleColors[i % oCOUNTOF(sConsoleColors)];
		}

		image->CopyData(buffer->GetData(), imageDesc.RowPitch);

		ouro::surface::info inf;
		inf.dimensions = int3(imageDesc.Dimensions, 1);
		inf.format = oImageFormatToSurfaceFormat(imageDesc.Format);
		inf.layout = _Layout;

		intrusive_ptr<oImage> mipImage;
		oTESTB(oImageCreate("TestImageMips1D", inf, &mipImage), "Failed to create image for mipped surface");

		oTESTB(oGPUGenerateMips(_pDevice, (const oImage**)&image, 1, inf, oGPU_TEXTURE_1D_MAP, mipImage), "Failed to generate mips with the GPU");

		oTESTI2(mipImage, _StartIndex);

		return SUCCESS;
	}

	RESULT TestImageMips2D(char* _StrStatus, size_t _SizeofStrStatus, oGPUDevice* _pDevice, char* _pFilename, ouro::surface::layout _Layout, int _StartIndex)
	{
		intrusive_ptr<oImage> image;
		oTESTB(oImageLoad(_pFilename, oImage::FORCE_ALPHA, &image), "Failed to load image");

		oImage::DESC id;
		image->GetDesc(&id);

		ouro::surface::info inf;
		inf.dimensions = int3(id.Dimensions, 1);
		inf.format = oImageFormatToSurfaceFormat(id.Format);
		inf.layout = _Layout;

		intrusive_ptr<oImage> mipImage;
		oTESTB(oImageCreate("TestImageMips2D", inf, &mipImage), "Failed to create image for mipped surface");

		oTESTB(oGPUGenerateMips(_pDevice, (const oImage**)&image, 1, inf, oGPU_TEXTURE_2D_MAP, mipImage), "Failed to generate mips with the GPU");

		oTESTI2(mipImage, _StartIndex);

		return SUCCESS;
	}

	RESULT TestImageMips3D(char* _StrStatus, size_t _SizeofStrStatus, oGPUDevice* _pDevice, char* _pFilename, ouro::surface::layout _Layout, int _StartIndex)
	{
		intrusive_ptr<oImage> images[5];
		oTESTB(oImageLoad(_pFilename, oImage::FORCE_ALPHA, &images[0]), "Failed to load image");
		images[4] = images[3] = images[2] = images[1] = images[0];

		oImage::DESC id;
		images[0]->GetDesc(&id);

		ouro::surface::info inf;
		inf.dimensions = int3(id.Dimensions, oCOUNTOF(images));
		inf.format = oImageFormatToSurfaceFormat(id.Format);
		inf.layout = _Layout;

		intrusive_ptr<oImage> mipImage;
		oTESTB(oImageCreate("TestImageMips3D", inf, &mipImage), "Failed to create image for mipped surface");

		oTESTB(oGPUGenerateMips(_pDevice, (const oImage**)&images, oCOUNTOF(images), inf, oGPU_TEXTURE_3D_MAP, mipImage), "Failed to generate mips with the GPU");

		oTESTI2(mipImage, _StartIndex);

		return SUCCESS;
	}

	RESULT TestImageMipsCube(char* _StrStatus, size_t _SizeofStrStatus, oGPUDevice* _pDevice, ouro::surface::layout _Layout, int _StartIndex)
	{
		intrusive_ptr<oImage> images[6];
		oTESTB(oImageLoad("file://DATA/Test/Textures/CubePosX.png", oImage::FORCE_ALPHA, &images[0]), "Failed to load image +X");
		oTESTB(oImageLoad("file://DATA/Test/Textures/CubeNegX.png", oImage::FORCE_ALPHA, &images[1]), "Failed to load image -X");
		oTESTB(oImageLoad("file://DATA/Test/Textures/CubePosY.png", oImage::FORCE_ALPHA, &images[2]), "Failed to load image +Y");
		oTESTB(oImageLoad("file://DATA/Test/Textures/CubeNegY.png", oImage::FORCE_ALPHA, &images[3]), "Failed to load image -Y");
		oTESTB(oImageLoad("file://DATA/Test/Textures/CubePosZ.png", oImage::FORCE_ALPHA, &images[4]), "Failed to load image +Z");
		oTESTB(oImageLoad("file://DATA/Test/Textures/CubeNegZ.png", oImage::FORCE_ALPHA, &images[5]), "Failed to load image -Z");

		oImage::DESC id;
		images[0]->GetDesc(&id);

		ouro::surface::info inf;
		inf.dimensions = int3(id.Dimensions, 1);
		inf.format = oImageFormatToSurfaceFormat(id.Format);
		inf.array_size = oCOUNTOF(images);
		inf.layout = _Layout;

		intrusive_ptr<oImage> mipImage;
		oTESTB(oImageCreate("TestImageMipsCube", inf, &mipImage), "Failed to create image for mipped surface");

		oTESTB(oGPUGenerateMips(_pDevice, (const oImage**)&images, oCOUNTOF(images), inf, oGPU_TEXTURE_CUBE_MAP, mipImage), "Failed to generate mips with the GPU");

		oTESTI2(mipImage, _StartIndex);

		return SUCCESS;
	}

	RESULT Run(char* _StrStatus, size_t _SizeofStrStatus) override
	{
		oGPUDevice::INIT init("GPU_GenerateMips");
		init.Version = version(10,0); // for more compatibility when running on varied machines
		intrusive_ptr<oGPUDevice> Device;
		oTESTB0(oGPUDeviceCreate(init, &Device));

		// 1D non power of 2
		RESULT res = TestImageMips1D(_StrStatus, _SizeofStrStatus, Device, 227, ouro::surface::tight, 0);
		if (SUCCESS != res) 
			return res;

		res = TestImageMips1D(_StrStatus, _SizeofStrStatus, Device, 227, ouro::surface::below, 1);
		if (SUCCESS != res) 
			return res;

		res = TestImageMips1D(_StrStatus, _SizeofStrStatus, Device, 227, ouro::surface::right, 2);
		if (SUCCESS != res) 
			return res;

		// 1D power of 2
		res = TestImageMips1D(_StrStatus, _SizeofStrStatus, Device, 512, ouro::surface::tight, 3);
		if (SUCCESS != res) 
			return res;

		res = TestImageMips1D(_StrStatus, _SizeofStrStatus, Device, 512, ouro::surface::below, 4);
		if (SUCCESS != res) 
			return res;

		res = TestImageMips1D(_StrStatus, _SizeofStrStatus, Device, 512, ouro::surface::right, 5);
		if (SUCCESS != res) 
			return res;

		// 2D non power of 2
		res = TestImageMips2D(_StrStatus, _SizeofStrStatus, Device, "file://DATA/Test/Textures/lena_npot.png", ouro::surface::tight, 6);
		if (SUCCESS != res) 
			return res;

		res = TestImageMips2D(_StrStatus, _SizeofStrStatus, Device, "file://DATA/Test/Textures/lena_npot.png", ouro::surface::below, 7);
		if (SUCCESS != res) 
			return res;

		res = TestImageMips2D(_StrStatus, _SizeofStrStatus, Device, "file://DATA/Test/Textures/lena_npot.png", ouro::surface::right, 8);
		if (SUCCESS != res) 
			return res;

		// 2D power of 2
		res = TestImageMips2D(_StrStatus, _SizeofStrStatus, Device, "file://DATA/Test/Textures/lena_1.png", ouro::surface::tight, 9);
		if (SUCCESS != res) 
			return res;

		res = TestImageMips2D(_StrStatus, _SizeofStrStatus, Device, "file://DATA/Test/Textures/lena_1.png", ouro::surface::below, 10);
		if (SUCCESS != res) 
			return res;

		res = TestImageMips2D(_StrStatus, _SizeofStrStatus, Device, "file://DATA/Test/Textures/lena_1.png", ouro::surface::right, 11);
		if (SUCCESS != res) 
			return res;

		// 3D non power of 2
		res = TestImageMips3D(_StrStatus, _SizeofStrStatus, Device, "file://DATA/Test/Textures/lena_npot.png", ouro::surface::tight, 12);
		if (SUCCESS != res) 
			return res;

		res = TestImageMips3D(_StrStatus, _SizeofStrStatus, Device, "file://DATA/Test/Textures/lena_npot.png", ouro::surface::below, 13);
		if (SUCCESS != res) 
			return res;

		res = TestImageMips3D(_StrStatus, _SizeofStrStatus, Device, "file://DATA/Test/Textures/lena_npot.png", ouro::surface::right, 14);
		if (SUCCESS != res) 
			return res;

		// 3D power of 2
		res = TestImageMips3D(_StrStatus, _SizeofStrStatus, Device, "file://DATA/Test/Textures/lena_1.png", ouro::surface::tight, 15);
		if (SUCCESS != res) 
			return res;

		res = TestImageMips3D(_StrStatus, _SizeofStrStatus, Device, "file://DATA/Test/Textures/lena_1.png", ouro::surface::below, 16);
		if (SUCCESS != res) 
			return res;

		res = TestImageMips3D(_StrStatus, _SizeofStrStatus, Device, "file://DATA/Test/Textures/lena_1.png", ouro::surface::right, 17);
		if (SUCCESS != res) 
			return res;

		// Cube power of 2
		res = TestImageMipsCube(_StrStatus, _SizeofStrStatus, Device, ouro::surface::tight, 18);
		if (SUCCESS != res) 
			return res;

		res = TestImageMipsCube(_StrStatus, _SizeofStrStatus, Device, ouro::surface::below, 19);
		if (SUCCESS != res) 
			return res;

		res = TestImageMipsCube(_StrStatus, _SizeofStrStatus, Device, ouro::surface::right, 20);
		if (SUCCESS != res) 
			return res;

		return SUCCESS;
	}
};

oTEST_REGISTER(GPU_GenerateMips);