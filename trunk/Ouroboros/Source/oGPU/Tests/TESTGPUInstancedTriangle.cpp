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

static const int sSnapshotFrames[] = { 1, 23, 46 };
static const bool kIsDevMode = false;

struct gpu_test_instanced_triangle : public gpu_test
{
	gpu_test_instanced_triangle() : gpu_test("GPU test: instanced triangle", kIsDevMode, sSnapshotFrames) {}

	void initialize() override
	{
		{
			buffer_info i;
			i.struct_byte_size = sizeof(oGPUTestConstants);
			TestConstants = Device->make_buffer("TestConstants", i);
		}

		Pipeline = Device->make_pipeline1(oGPUTestGetPipeline(oGPU_TEST_TRANSFORMED_WHITE));
		InstanceList = Device->make_buffer<oGPU_TEST_INSTANCE>("Instances", 2);
		Mesh = make_first_triangle(Device);
	}
	
	void render() override
	{
		float4x4 V = make_lookat_lh(float3(0.0f, 0.0f, -3.5f), oZERO3, float3(0.0f, 1.0f, 0.0f));

		render_target_info RTI = PrimaryRenderTarget->get_info();
		float4x4 P = make_perspective_lh(oDEFAULT_FOVY_RADIANS, RTI.dimensions.x / oCastAsFloat(RTI.dimensions.y), 0.001f, 1000.0f);

		{
			surface::mapped_subresource msr = CommandList->reserve(InstanceList, 0);
			oGPU_TEST_INSTANCE* pInstances = (oGPU_TEST_INSTANCE*)msr.data;
			{
				pInstances[0].Translation = float3(-0.5f, 0.5f, 0.0f);
				pInstances[1].Translation = float3(0.5f, -0.5f, 0.0f);

				float rotationStep = Device->frame_id() * 1.0f;
				pInstances[0].Rotation = make_quaternion(float3(radians(rotationStep) * 0.75f, radians(rotationStep), radians(rotationStep) * 0.5f));
				pInstances[1].Rotation = make_quaternion(float3(radians(rotationStep) * 0.5f, radians(rotationStep), radians(rotationStep) * 0.75f));
			}
			CommandList->commit(InstanceList, 0, msr);
		}

		CommandList->begin();

		commit_buffer(CommandList.get(), TestConstants.get(), oGPUTestConstants(oIDENTITY4x4, V, P, white));

		CommandList->clear(PrimaryRenderTarget, clear_type::color_depth_stencil);
		CommandList->set_blend_state(blend_state::opaque);
		CommandList->set_depth_stencil_state(depth_stencil_state::test_and_write);
		CommandList->set_surface_state(surface_state::two_sided);
		
		const buffer* CBs[2] = { TestConstants.get(), InstanceList.get() };
		
		CommandList->set_buffers(0, CBs);
		CommandList->set_pipeline(Pipeline);
		CommandList->set_render_target(PrimaryRenderTarget);
		
		Mesh->draw(CommandList);

		CommandList->end();
	}

private:
	std::shared_ptr<pipeline1> Pipeline;
	std::shared_ptr<buffer> InstanceList;
	std::shared_ptr<util_mesh> Mesh;
	std::shared_ptr<buffer> TestConstants;
};

oGPU_COMMON_TEST(instanced_triangle);

	} // namespace tests
} // namespace ouro

