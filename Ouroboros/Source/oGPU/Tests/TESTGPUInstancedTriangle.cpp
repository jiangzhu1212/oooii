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
#include <oGPU/constant_buffer.h>

#include <oBasis/oMath.h>

using namespace ouro::gpu;

namespace ouro {
	namespace tests {

static const int sSnapshotFrames[] = { 1, 23, 46 };
static const bool kIsDevMode = false;

struct gpu_test_instanced_triangle : public gpu_test
{
	gpu_test_instanced_triangle() : gpu_test("GPU test: instanced triangle", kIsDevMode, sSnapshotFrames) {}

	void initialize() override
	{
		Pipeline = Device->make_pipeline1(oGPUTestGetPipeline(oGPU_TEST_TRANSFORMED_WHITE));
		TestConstants.initialize("TestConstants", Device.get(), sizeof(oGPUTestConstants));
		InstanceList.initialize("Instances", Device.get(), sizeof(oGPU_TEST_INSTANCE), 2);
		Mesh.initialize_first_triangle(Device.get());
	}
	
	void render() override
	{
		float4x4 V = make_lookat_lh(float3(0.0f, 0.0f, -3.5f), oZERO3, float3(0.0f, 1.0f, 0.0f));

		uint2 dimensions = PrimaryColorTarget.dimensions();
		float4x4 P = make_perspective_lh(oDEFAULT_FOVY_RADIANS, dimensions.x / static_cast<float>(dimensions.y), 0.001f, 1000.0f);

		oGPU_TEST_INSTANCE instances[2];
		instances[0].Translation = float3(-0.5f, 0.5f, 0.0f);
		instances[1].Translation = float3(0.5f, -0.5f, 0.0f);

		float rotationStep = Device->frame_id() * 1.0f;
		instances[0].Rotation = make_quaternion(float3(radians(rotationStep) * 0.75f, radians(rotationStep), radians(rotationStep) * 0.5f));
		instances[1].Rotation = make_quaternion(float3(radians(rotationStep) * 0.5f, radians(rotationStep), radians(rotationStep) * 0.75f));

		InstanceList.update(CommandList.get(), instances);

		CommandList->begin();

		TestConstants.update(CommandList.get(), oGPUTestConstants(oIDENTITY4x4, V, P, white));

		BlendState.set(CommandList.get(), blend_state::opaque);
		DepthStencilState.set(CommandList.get(), depth_stencil_state::test_and_write);
		RasterizerState.set(CommandList.get(), rasterizer_state::two_sided);
		SamplerState.set(CommandList.get(), 0, sampler_state::linear_wrap);

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
	constant_buffer InstanceList;
	constant_buffer TestConstants;
	util_mesh Mesh;
};

oGPU_COMMON_TEST(instanced_triangle);

	} // namespace tests
} // namespace ouro

