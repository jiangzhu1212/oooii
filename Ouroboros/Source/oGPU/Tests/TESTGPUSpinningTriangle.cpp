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

#include <oBasis/oMath.h>

using namespace ouro::gpu;

namespace ouro {
	namespace tests {

static const int sSnapshotFrames[] = { 0, 2, 4, 6 };
static const bool kIsDevMode = false;

class gpu_test_spinning_triangle : public gpu_test
{
public:
	gpu_test_spinning_triangle() : gpu_test("GPU test: spinning triangle", kIsDevMode, sSnapshotFrames) {}

	void initialize() override
	{
		PrimaryRenderTarget->set_clear_color(almost_black);
		TestConstants = Device->make_buffer<oGPUTestConstants>("TestConstants");
		Pipeline = Device->make_pipeline1(oGPUTestGetPipeline(oGPU_TEST_TRANSFORMED_WHITE));
		Mesh = make_first_triangle(Device);
	}

	void render() override
	{
		float4x4 V = make_lookat_lh(float3(0.0f, 0.0f, -2.5f), oZERO3, float3(0.0f, 1.0f, 0.0f));

		render_target_info RTI = PrimaryRenderTarget->get_info();
		float4x4 P = make_perspective_lh(oDEFAULT_FOVY_RADIANS, RTI.dimensions.x / oCastAsFloat(RTI.dimensions.y), 0.001f, 1000.0f);

		// this is -1 because there was a code change that resulted in begin_frame()
		// being moved out of the Render function below so it updated the FrameID
		// earlier than this code was ready for. If golden images are updated, this
		// could go away.
		float rotationRate = (Device->frame_id()-1) * 2.0f;
		float4x4 W = make_rotation(float3(0.0f, radians(rotationRate), 0.0f));

		uint DrawID = 0;

		CommandList->begin();

		commit_buffer(CommandList.get(), TestConstants.get(), oGPUTestConstants(W, V, P, white));

		CommandList->clear(PrimaryRenderTarget, clear_type::color_depth_stencil);
		CommandList->set_blend_state(blend_state::opaque);
		CommandList->set_depth_stencil_state(depth_stencil_state::test_and_write);
		CommandList->set_surface_state(surface_state::two_sided);
		CommandList->set_buffer(0, TestConstants);
		CommandList->set_pipeline(Pipeline);
		CommandList->set_render_target(PrimaryRenderTarget);
		Mesh->draw(CommandList);

		CommandList->end();
	}

private:
	std::shared_ptr<pipeline1> Pipeline;
	std::shared_ptr<util_mesh> Mesh;
	std::shared_ptr<buffer> TestConstants;
};

oGPU_COMMON_TEST(spinning_triangle);

	} // namespace tests
} // namespace ouro
