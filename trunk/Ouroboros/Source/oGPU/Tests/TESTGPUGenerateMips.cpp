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
#include "oGPUTestCommon.h"

using namespace ouro;

struct GPU_GenerateMips : public oTest
{
	RESULT TestImageMips1D(char* _StrStatus, size_t _SizeofStrStatus, oGPUDevice* _pDevice, int _Width, ouro::surface::layout _Layout, int _StartIndex)
	{
		auto image = make_1D(_Width);
		ouro::surface::info si = image->get_info();
		si.layout = _Layout;
		auto mipchain = ouro::surface::buffer::make(si);
		const auto* i = image.get();
		oTESTB(oGPUGenerateMips(_pDevice, &i, 1, si, ouro::gpu::texture_type::default_1d, mipchain.get()), "Failed to generate mips with the GPU");
		mipchain->flatten();
		oTESTI2(mipchain, _StartIndex);
		return SUCCESS;
	}

	RESULT TestImageMips2D(char* _StrStatus, size_t _SizeofStrStatus, oGPUDevice* _pDevice, const path& _Path, ouro::surface::layout _Layout, int _StartIndex)
	{
		auto image = surface_load(_Path);
		auto si = image->get_info();
		si.layout = _Layout;
		auto mipchain = ouro::surface::buffer::make(si);
		const auto* i = image.get();
		oTESTB(oGPUGenerateMips(_pDevice, &i, 1, si, ouro::gpu::texture_type::default_2d, mipchain.get()), "Failed to generate mips with the GPU");
		mipchain->flatten();
		oTESTI2(mipchain, _StartIndex);
		return SUCCESS;
	}

	RESULT TestImageMips3D(char* _StrStatus, size_t _SizeofStrStatus, oGPUDevice* _pDevice, const path& _Path, ouro::surface::layout _Layout, int _StartIndex)
	{
		std::shared_ptr<ouro::surface::buffer> images[5];
		images[0] = surface_load(_Path); 
		images[4] = images[3] = images[2] = images[1] = images[0];

		auto si = images[0]->get_info();
		si.dimensions.z = oCOUNTOF(images);
		si.layout = _Layout;
		auto mipchain = ouro::surface::buffer::make(si);

		const ouro::surface::buffer* img[5];
		for (int i = 0; i < oCOUNTOF(img); i++)
			img[i] = images[i].get();
		
		oTESTB(oGPUGenerateMips(_pDevice, img, oCOUNTOF(img), si, ouro::gpu::texture_type::default_3d, mipchain.get()), "Failed to generate mips with the GPU");
		mipchain->flatten();
		oTESTI2(mipchain, _StartIndex);
		return SUCCESS;
	}

	RESULT TestImageMipsCube(char* _StrStatus, size_t _SizeofStrStatus, oGPUDevice* _pDevice, ouro::surface::layout _Layout, int _StartIndex)
	{
		std::shared_ptr<ouro::surface::buffer> images[6];
		images[0] = surface_load(filesystem::data_path() / "Test/Textures/CubePosX.png"); 
		images[1] = surface_load(filesystem::data_path() / "Test/Textures/CubeNegX.png"); 
		images[2] = surface_load(filesystem::data_path() / "Test/Textures/CubePosY.png"); 
		images[3] = surface_load(filesystem::data_path() / "Test/Textures/CubeNegY.png"); 
		images[4] = surface_load(filesystem::data_path() / "Test/Textures/CubePosZ.png"); 
		images[5] = surface_load(filesystem::data_path() / "Test/Textures/CubeNegZ.png"); 

		auto si = images[0]->get_info();
		si.layout = _Layout;
		si.array_size = oCOUNTOF(images);
		auto mipchain = ouro::surface::buffer::make(si);

		const ouro::surface::buffer* img[6];
		for (int i = 0; i < oCOUNTOF(img); i++)
			img[i] = images[i].get();
		
		oTESTB(oGPUGenerateMips(_pDevice, img, oCOUNTOF(img), si, ouro::gpu::texture_type::default_cube, mipchain.get()), "Failed to generate mips with the GPU");
		mipchain->flatten();
		oTESTI2(mipchain, _StartIndex);

		return SUCCESS;
	}

	RESULT Run(char* _StrStatus, size_t _SizeofStrStatus) override
	{
		oGPUDevice::INIT init("GPU_GenerateMips");
		init.version = version(10,0); // for more compatibility when running on varied machines
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
		res = TestImageMips2D(_StrStatus, _SizeofStrStatus, Device, filesystem::data_path() / "Test/Textures/lena_npot.png", ouro::surface::tight, 6);
		if (SUCCESS != res) 
			return res;

		res = TestImageMips2D(_StrStatus, _SizeofStrStatus, Device, filesystem::data_path() / "Test/Textures/lena_npot.png", ouro::surface::below, 7);
		if (SUCCESS != res) 
			return res;

		res = TestImageMips2D(_StrStatus, _SizeofStrStatus, Device, filesystem::data_path() / "Test/Textures/lena_npot.png", ouro::surface::right, 8);
		if (SUCCESS != res) 
			return res;

		// 2D power of 2
		res = TestImageMips2D(_StrStatus, _SizeofStrStatus, Device, filesystem::data_path() / "Test/Textures/lena_1.png", ouro::surface::tight, 9);
		if (SUCCESS != res) 
			return res;

		res = TestImageMips2D(_StrStatus, _SizeofStrStatus, Device, filesystem::data_path() / "Test/Textures/lena_1.png", ouro::surface::below, 10);
		if (SUCCESS != res) 
			return res;

		res = TestImageMips2D(_StrStatus, _SizeofStrStatus, Device, filesystem::data_path() / "Test/Textures/lena_1.png", ouro::surface::right, 11);
		if (SUCCESS != res) 
			return res;

		// 3D non power of 2
		res = TestImageMips3D(_StrStatus, _SizeofStrStatus, Device, filesystem::data_path() / "Test/Textures/lena_npot.png", ouro::surface::tight, 12);
		if (SUCCESS != res) 
			return res;

		res = TestImageMips3D(_StrStatus, _SizeofStrStatus, Device, filesystem::data_path() / "Test/Textures/lena_npot.png", ouro::surface::below, 13);
		if (SUCCESS != res) 
			return res;

		res = TestImageMips3D(_StrStatus, _SizeofStrStatus, Device, filesystem::data_path() / "Test/Textures/lena_npot.png", ouro::surface::right, 14);
		if (SUCCESS != res) 
			return res;

		// 3D power of 2
		res = TestImageMips3D(_StrStatus, _SizeofStrStatus, Device, filesystem::data_path() / "Test/Textures/lena_1.png", ouro::surface::tight, 15);
		if (SUCCESS != res) 
			return res;

		res = TestImageMips3D(_StrStatus, _SizeofStrStatus, Device, filesystem::data_path() / "Test/Textures/lena_1.png", ouro::surface::below, 16);
		if (SUCCESS != res) 
			return res;

		res = TestImageMips3D(_StrStatus, _SizeofStrStatus, Device, filesystem::data_path() / "Test/Textures/lena_1.png", ouro::surface::right, 17);
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
