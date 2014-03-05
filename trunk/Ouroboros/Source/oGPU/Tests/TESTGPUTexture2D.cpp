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

struct gpu_test_texture2d : public gpu_texture_test
{
	gpu_test_texture2d() : gpu_texture_test("GPU test: texture2d", kIsDevMode) {}

	oGPU_TEST_PIPELINE get_pipeline() override { return oGPU_TEST_TEXTURE_2D; }
	std::shared_ptr<texture> make_test_texture() override
	{
		auto image = surface_load(filesystem::data_path() / "Test/Textures/lena_1.png", surface::alpha_option::force_alpha);
		auto* p = image.get();
		return make_texture(Device, "Test 2D", &p, 1, texture_type::default_2d);
	}
};

oGPU_COMMON_TEST(texture2d);

	} // namespace tests
} // namespace ouro
