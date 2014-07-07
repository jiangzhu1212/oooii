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
#include <oGPU/depth_target.h>
#include <oGPU/color_target.h>

#include <oBasis/oMath.h>

using namespace ouro::gpu;

namespace ouro {
	namespace tests {

static const int sSnapshotFrames[] = { 0, 50 };
static const bool kIsDevMode = false;

struct gpu_test_render_target : public gpu_test
{
	gpu_test_render_target() : gpu_test("GPU test: render_target", kIsDevMode, sSnapshotFrames) {}

	void initialize() override
	{
		command_list_info i;
		
		i.draw_order = 1;
		CLMainScene = Device->make_command_list("CLMainScene", i);

		i.draw_order = 0;
		CLRenderTarget = Device->make_command_list("CLRenderTarget", i);
		TestConstants.initialize("TestConstants", Device.get(), sizeof(oGPUTestConstants));
		PLPassThrough = Device->make_pipeline1(oGPUTestGetPipeline(oGPU_TEST_PASS_THROUGH));
		Triangle.initialize_first_triangle(Device.get());
		PLTexture = Device->make_pipeline1(oGPUTestGetPipeline(oGPU_TEST_TEXTURE_2D));
		Cube.initialize_first_cube(Device.get());

		ColorTarget.initialize("ColorTarget", Device.get(), surface::b8g8r8a8_unorm, 256, 256, 0, false);
		DepthTarget.initialize("DepthTarget", Device.get(), surface::d24_unorm_s8_uint, 256, 256, 0, false, 0);
	}

	void render() override
	{
		float4x4 V = make_lookat_lh(float3(0.0f, 0.0f, -4.5f), oZERO3, float3(0.0f, 1.0f, 0.0f));

		uint2 dimensions = PrimaryColorTarget.dimensions();
		float4x4 P = make_perspective_lh(oDEFAULT_FOVY_RADIANS, dimensions.x / static_cast<float>(dimensions.y), 0.001f, 1000.0f);

		float rotationStep = Device->frame_id() * 1.0f;
		float4x4 W = make_rotation(float3(radians(rotationStep) * 0.75f, radians(rotationStep), radians(rotationStep) * 0.5f));

		// DrawOrder should be respected in out-of-order submits so show that here
		// by executing the main scene THEN the render target but because the
		// draw order of the command lists defines the render target before the 
		// main scene this should come out as a cube with a triangle texture.

		render_main_scene(CLMainScene.get(), ColorTarget);
		render_to_target(CLRenderTarget.get(), ColorTarget);
	}

private:
	std::shared_ptr<command_list> CLMainScene;
	std::shared_ptr<command_list> CLRenderTarget;
	std::shared_ptr<pipeline1> PLPassThrough;
	std::shared_ptr<pipeline1> PLTexture;
	color_target ColorTarget;
	depth_target DepthTarget;
	util_mesh Cube;
	util_mesh Triangle;
	constant_buffer TestConstants;

	void render_to_target(command_list* _pCommandList, color_target& rt)
	{
		_pCommandList->begin();
		rt.clear(_pCommandList, deep_sky_blue);
		_pCommandList->set_blend_state(blend_state::opaque);
		_pCommandList->set_depth_stencil_state(depth_stencil_state::none);
		_pCommandList->set_rasterizer_state(rasterizer_state::front_face);
		_pCommandList->set_pipeline(PLPassThrough);
		rt.set_draw_target(_pCommandList);
		Triangle.draw(_pCommandList);
		_pCommandList->end();
	}

	void render_main_scene(command_list* _pCommandList, resource& texture)
	{
		float4x4 V = make_lookat_lh(float3(0.0f, 0.0f, -4.5f), oZERO3, float3(0.0f, 1.0f, 0.0f));

		uint2 dimensions = PrimaryColorTarget.dimensions();
		float4x4 P = make_perspective_lh(oDEFAULT_FOVY_RADIANS, dimensions.x / static_cast<float>(dimensions.y), 0.001f, 1000.0f);

		float rotationStep = Device->frame_id() * 1.0f;
		float4x4 W = make_rotation(float3(radians(rotationStep) * 0.75f, radians(rotationStep), radians(rotationStep) * 0.5f));

		_pCommandList->begin();

		TestConstants.update(CommandList.get(), oGPUTestConstants(W, V, P, white));

		_pCommandList->set_blend_state(blend_state::opaque);
		_pCommandList->set_depth_stencil_state(depth_stencil_state::test_and_write);
		_pCommandList->set_rasterizer_state(rasterizer_state::front_face);
		TestConstants.set(_pCommandList, 0);
		_pCommandList->set_sampler(0, sampler_state::linear_wrap);
		texture.set(_pCommandList, 0);
		_pCommandList->set_pipeline(PLTexture);
		PrimaryColorTarget.clear(_pCommandList, get_clear_color());
		PrimaryColorTarget.set_draw_target(_pCommandList, PrimaryDepthTarget);
		Cube.draw(_pCommandList);
		_pCommandList->end();
	}
};

oGPU_COMMON_TEST(render_target);

	} // namespace tests
} // namespace ouro
