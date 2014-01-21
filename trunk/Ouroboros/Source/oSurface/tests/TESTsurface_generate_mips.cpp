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
#include <oSurface/codec.h>
#include <oBase/path.h>
//#include <oBase/timer.h>
//#include <vector>

#include "../../test_services.h"

namespace ouro { 
	namespace tests {

static const surface::filter::value kFilter = surface::filter::lanczos2;

static std::shared_ptr<surface::buffer> surface_load(test_services& _Services, const path& _Path, surface::alpha_option::value _Option = surface::alpha_option::preserve)
{
	size_t size = 0;
	auto b = _Services.load_buffer(_Path, &size);
	return surface::decode(b.get(), size, _Option);
}

static std::shared_ptr<surface::buffer> make_test_1d(int _Width)
{
	surface::info si;
	si.dimensions = int3(_Width, 1, 1);
	si.format = surface::b8g8r8a8_unorm;
	auto s = surface::buffer::make(si);

	{
		surface::lock_guard lock(s);
		static const color sConsoleColors[] = { Black, Navy, Green, Teal, Maroon, Purple, Olive, Silver, Gray, Blue, Lime, Aqua, Red, Fuchsia, Yellow, White };
		color* texture1Ddata = (color*)lock.mapped.data;
		for (int i = 0; i < si.dimensions.x; i++)
			texture1Ddata[i] = sConsoleColors[i % oCOUNTOF(sConsoleColors)];
	}

	return s;
}

static std::shared_ptr<surface::buffer> load_test_cube(test_services& _Services)
{
	const char* face_paths[6] =
	{
		"Test/Textures/CubePosX.png",
		"Test/Textures/CubeNegX.png",
		"Test/Textures/CubePosY.png",
		"Test/Textures/CubeNegY.png",
		"Test/Textures/CubePosZ.png",
		"Test/Textures/CubeNegZ.png",
	};

	auto image = surface_load(_Services, face_paths[0]);

	auto si = image->get_info();
	si.array_size = oCOUNTOF(face_paths);
	auto cube_image = surface::buffer::make(si);
	cube_image->copy_from(0, image.get(), 0);

	for (int i = 1; i < oCOUNTOF(face_paths); i++)
	{
		image = surface_load(_Services, face_paths[i]);
		int subresource = surface::calc_subresource(0, i, 0, 0, si.array_size);
		cube_image->copy_from(subresource, image.get(), 0);
	}

	return cube_image;
}

static void test_mipchain(test_services& _Services, const surface::buffer* _pImage, surface::filter::value _Filter, surface::layout _Layout, int _StartIndex)
{
	auto si = _pImage->get_info();
	si.layout = _Layout;
	auto mipchain = surface::buffer::make(si);
	mipchain->clear();

	int nSlices = max(si.array_size, si.dimensions.z);
	int nMips = surface::num_mips(si.layout, si.dimensions);

	if (si.dimensions.z != 1)
	{
		int subresource = surface::calc_subresource(0, 0, 0, nMips, si.array_size);
		surface::box region;
		region.right = si.dimensions.x;
		region.bottom = si.dimensions.y;
		surface::shared_lock lock(_pImage, subresource);
		for (int i = 0; i < nSlices; i++)
		{
			region.front = i;
			region.back = i + 1;

			mipchain->update_subresource(subresource, region, lock.mapped);
			lock.mapped.data = byte_add(lock.mapped.data, lock.mapped.depth_pitch);
		}
	}

	else
	{
		int nSlices = max(1, si.array_size);
		for (int i = 0; i < nSlices; i++)
		{
			int DstSubresource = surface::calc_subresource(0, i, 0, nMips, nSlices);
			int SrcSubresource = surface::calc_subresource(0, i, 0, 0, nSlices);
			mipchain->copy_from(DstSubresource, _pImage, SrcSubresource);
		}
	}

	mipchain->generate_mips(_Filter);
	mipchain->flatten();
	_Services.check(mipchain.get(), _StartIndex);
}

static void test_mipchain_layouts(test_services& _Services, const surface::buffer* _pImage, surface::filter::value _Filter, int _StartIndex)
{
	test_mipchain(_Services, _pImage, _Filter, surface::tight, _StartIndex);
	test_mipchain(_Services, _pImage, _Filter, surface::below, _StartIndex+1);
	test_mipchain(_Services, _pImage, _Filter, surface::right, _StartIndex+2);
}

void TESTsurface_generate_mips(test_services& _Services)
{
	auto image = make_test_1d(227); // 1D NPOT
	test_mipchain_layouts(_Services, image.get(), kFilter, 0);

	image = make_test_1d(512); // 1D POT
	test_mipchain_layouts(_Services, image.get(), kFilter, 3);

	image = surface_load(_Services, "Test/Textures/lena_npot.png"); // 2D NPOT
	test_mipchain_layouts(_Services, image.get(), kFilter, 6);

	image = surface_load(_Services, "Test/Textures/lena_1.png"); // 2D POT
	test_mipchain_layouts(_Services, image.get(), kFilter, 9);

	{
		image = surface_load(_Services, "Test/Textures/lena_npot.png"); // 2D NPOT
		const surface::buffer* images[5] = { image.get(), image.get(), image.get(), image.get(), image.get() };
		auto image3d = surface::buffer::make(images, oCOUNTOF(images), surface::buffer::image3d);
		test_mipchain_layouts(_Services, image3d.get(), kFilter, 12);
	}
	{
		image = surface_load(_Services, "Test/Textures/lena_1.png"); // 2D POT
		const surface::buffer* images[5] = { image.get(), image.get(), image.get(), image.get(), image.get() };
		auto image3d = surface::buffer::make(images, oCOUNTOF(images), surface::buffer::image3d);
		test_mipchain_layouts(_Services, image3d.get(), kFilter, 15);
	}

	image = load_test_cube(_Services);
	test_mipchain_layouts(_Services, image.get(), kFilter, 18);
}

	} // namespace tests
} // namespace ouro
