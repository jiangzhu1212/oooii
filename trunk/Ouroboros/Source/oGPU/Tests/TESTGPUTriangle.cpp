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

static const int sSnapshotFrames[] = { 0 };
static const bool kIsDevMode = false;

struct gpu_test_triangle : public gpu_test
{
	gpu_test_triangle() : gpu_test("GPU test: triangle", kIsDevMode, sSnapshotFrames) {}

	void initialize() override
	{
		Pipeline = Device->make_pipeline1(oGPUTestGetPipeline(oGPU_TEST_PASS_THROUGH));
		Mesh = make_first_triangle(Device);
	}

	void render() override
	{
		CommandList->begin();
		CommandList->clear(PrimaryRenderTarget, clear_type::color_depth_stencil);
		CommandList->set_blend_state(blend_state::opaque);
		CommandList->set_depth_stencil_state(depth_stencil_state::none);
		CommandList->set_surface_state(surface_state::front_face);
		CommandList->set_pipeline(Pipeline);
		CommandList->set_render_target(PrimaryRenderTarget);
		Mesh->draw(CommandList);
		CommandList->end();
	}

private:
	std::shared_ptr<pipeline1> Pipeline;
	std::shared_ptr<util_mesh> Mesh;
};

oGPU_COMMON_TEST(triangle);

	} // namespace tests
} // namespace ouro
