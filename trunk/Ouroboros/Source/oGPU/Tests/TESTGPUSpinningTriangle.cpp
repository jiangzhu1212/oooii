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
		TestConstants.initialize("TestConstants", Device.get(), sizeof(oGPUTestConstants));
		InstanceList.initialize("Instances", Device.get(), sizeof(oGPU_TEST_INSTANCE), 2);
		Pipeline = Device->make_pipeline1(oGPUTestGetPipeline(oGPU_TEST_TRANSFORMED_WHITE));
		Mesh.initialize_first_triangle(Device.get());
	}

	void render() override
	{
		float4x4 V = make_lookat_lh(float3(0.0f, 0.0f, -2.5f), oZERO3, float3(0.0f, 1.0f, 0.0f));

		uint2 dimensions = PrimaryColorTarget.dimensions();
		float4x4 P = make_perspective_lh(oDEFAULT_FOVY_RADIANS, dimensions.x / static_cast<float>(dimensions.y), 0.001f, 1000.0f);

		// this is -1 because there was a code change that resulted in begin_frame()
		// being moved out of the Render function below so it updated the FrameID
		// earlier than this code was ready for. If golden images are updated, this
		// could go away.
		float rotationRate = (Device->frame_id()-1) * 2.0f;
		float4x4 W = make_rotation(float3(0.0f, radians(rotationRate), 0.0f));

		uint DrawID = 0;

		CommandList->begin();

		TestConstants.update(CommandList.get(), oGPUTestConstants(W, V, P, white));

		CommandList->set_blend_state(blend_state::opaque);
		CommandList->set_depth_stencil_state(depth_stencil_state::test_and_write);
		CommandList->set_rasterizer_state(rasterizer_state::two_sided);
		TestConstants.set(CommandList.get(), 0);

		const constant_buffer* CBs[2] = { &TestConstants, &InstanceList };
		constant_buffer::set(CommandList.get(), 0, 2, CBs);

		CommandList->set_pipeline(Pipeline);
		PrimaryColorTarget.clear(CommandList.get(), get_clear_color());
		PrimaryColorTarget.set_draw_target(CommandList.get(), PrimaryDepthTarget);
		Mesh.draw(CommandList.get());

		CommandList->end();
	}

private:
	std::shared_ptr<pipeline1> Pipeline;
	util_mesh Mesh;
	constant_buffer TestConstants;
	constant_buffer InstanceList;
};

oGPU_COMMON_TEST(spinning_triangle);

	} // namespace tests
} // namespace ouro
