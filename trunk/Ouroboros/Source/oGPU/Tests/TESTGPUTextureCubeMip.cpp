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
#include "oGPUTestCommon.h"
#include <oGPU/oGPUUtil.h>

using namespace ouro::gpu;

namespace ouro {
	namespace tests {

static const bool kIsDevMode = false;

struct gpu_test_texturecubemip : public gpu_texture_test
{
	gpu_test_texturecubemip() : gpu_texture_test("GPU test: texturecube mip", kIsDevMode) {}

	oGPU_TEST_PIPELINE get_pipeline() override { return oGPU_TEST_TEXTURE_CUBE; }
	std::shared_ptr<texture> make_test_texture() override
	{
		auto _0 = surface_load(filesystem::data_path() / "Test/Textures/CubePosX.png", surface::alpha_option::force_alpha);
		auto _1 = surface_load(filesystem::data_path() / "Test/Textures/CubeNegX.png", surface::alpha_option::force_alpha);
		auto _2 = surface_load(filesystem::data_path() / "Test/Textures/CubePosY.png", surface::alpha_option::force_alpha);
		auto _3 = surface_load(filesystem::data_path() / "Test/Textures/CubeNegY.png", surface::alpha_option::force_alpha);
		auto _4 = surface_load(filesystem::data_path() / "Test/Textures/CubePosZ.png", surface::alpha_option::force_alpha);
		auto _5 = surface_load(filesystem::data_path() / "Test/Textures/CubeNegZ.png", surface::alpha_option::force_alpha);

		const surface::buffer* images[6] = { _0.get(), _1.get(), _2.get(), _3.get(), _4.get(), _5.get() };
		return make_texture(Device, "Test cube mipped", images, oCOUNTOF(images), texture_type::mipped_cube);
	}

	float rotation_step() override
	{
		float rotationStep = (Device->frame_id()-1) * 1.0f;
		if (Device->frame_id()==0)
			rotationStep = 774.0f;
		else if (Device->frame_id()==2)
			rotationStep = 1036.0f;
		return rotationStep;
	}
};

oGPU_COMMON_TEST(texturecubemip);

	} // namespace tests
} // namespace ouro
