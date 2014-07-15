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
#include <oGPU/texture3d.h>

using namespace ouro::gpu;

namespace ouro {
	namespace tests {

static const bool kIsDevMode = false;

struct gpu_test_texture3d : public gpu_texture_test
{
	gpu_test_texture3d() : gpu_texture_test("GPU test: texture3d", kIsDevMode) {}

	pipeline get_pipeline() override { pipeline p; p.input = gfx::vertex_input::pos_uvw; p.vs = gfx::vertex_shader::texture3d; p.ps = gfx::pixel_shader::texture3d; return p; } 
	resource* make_test_texture() override
	{
		auto red = surface_load(filesystem::data_path() / "Test/Textures/Red.png", false, surface::alpha_option::force_alpha);
		auto green = surface_load(filesystem::data_path() / "Test/Textures/Green.png", false, surface::alpha_option::force_alpha);
		auto blue = surface_load(filesystem::data_path() / "Test/Textures/Blue.png", false, surface::alpha_option::force_alpha);

		const surface::buffer* images[3];
		images[0] = red.get();
		images[1] = green.get();
		images[2] = blue.get();
		auto image = surface::buffer::make(images, 3, surface::buffer::image3d);
		t.initialize("Test 3D", Device, *image, false);
		return &t;
	}

	texture3d t;
};

oGPU_COMMON_TEST(texture3d);

	} // namespace tests
} // namespace ouro
