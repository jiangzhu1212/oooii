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
		TestConstants = Device->make_buffer<oGPUTestConstants>("TestConstants");
		PLPassThrough = Device->make_pipeline1(oGPUTestGetPipeline(oGPU_TEST_PASS_THROUGH));
		Triangle = make_first_triangle(Device);
		PLTexture = Device->make_pipeline1(oGPUTestGetPipeline(oGPU_TEST_TEXTURE_2D));
		Cube = make_first_cube(Device);

		clear_info ci;
		ci.clear_color[0] = deep_sky_blue;

		{
			render_target_info i;
			i.dimensions = ushort3(256, 256, 1);
			i.array_size = 1;
			i.mrt_count = 1;
			i.format[0] = surface::b8g8r8a8_unorm;
			i.depth_stencil_format = surface::d24_unorm_s8_uint;
			i.clear = ci;
			RenderTarget = Device->make_render_target("RenderTarget", i);
		}
	}

	void render() override
	{
		float4x4 V = make_lookat_lh(float3(0.0f, 0.0f, -4.5f), oZERO3, float3(0.0f, 1.0f, 0.0f));

		render_target_info RTI = PrimaryRenderTarget->get_info();
		float4x4 P = make_perspective_lh(oDEFAULT_FOVY_RADIANS, RTI.dimensions.x / oCastAsFloat(RTI.dimensions.y), 0.001f, 1000.0f);

		float rotationStep = Device->frame_id() * 1.0f;
		float4x4 W = make_rotation(float3(radians(rotationStep) * 0.75f, radians(rotationStep), radians(rotationStep) * 0.5f));

		// DrawOrder should be respected in out-of-order submits, so show that here
		// but executing on the main scene, THEN the render target, but because the
		// draw order of the command lists defines the render target before the 
		// main scene, this should come out as a cube with a triangle texture.

		std::shared_ptr<texture> Texture = RenderTarget->get_texture(0);

		render_main_scene(CLMainScene.get(), Texture.get(), PrimaryRenderTarget.get());
		render_to_target(CLRenderTarget.get(), RenderTarget.get());
	}

private:
	std::shared_ptr<command_list> CLMainScene;
	std::shared_ptr<command_list> CLRenderTarget;
	std::shared_ptr<pipeline1> PLPassThrough;
	std::shared_ptr<pipeline1> PLTexture;
	std::shared_ptr<render_target> RenderTarget;
	std::shared_ptr<util_mesh> Cube;
	std::shared_ptr<util_mesh> Triangle;
	std::shared_ptr<buffer> TestConstants;

	void render_to_target(command_list* _pCommandList, render_target* _pTarget)
	{
		_pCommandList->begin();
		_pCommandList->clear(_pTarget, clear_type::color_depth_stencil);
		_pCommandList->set_blend_state(blend_state::opaque);
		_pCommandList->set_depth_stencil_state(depth_stencil_state::none);
		_pCommandList->set_surface_state(surface_state::front_face);
		_pCommandList->set_pipeline(PLPassThrough);
		_pCommandList->set_render_target(_pTarget);
		Triangle->draw(_pCommandList);
		_pCommandList->end();
	}

	void render_main_scene(command_list* _pCommandList, texture* _pTexture, render_target* _pTarget)
	{
		float4x4 V = make_lookat_lh(float3(0.0f, 0.0f, -4.5f), oZERO3, float3(0.0f, 1.0f, 0.0f));

		render_target_info RTI = PrimaryRenderTarget->get_info();
		float4x4 P = make_perspective_lh(oDEFAULT_FOVY_RADIANS, RTI.dimensions.x / oCastAsFloat(RTI.dimensions.y), 0.001f, 1000.0f);

		float rotationStep = Device->frame_id() * 1.0f;
		float4x4 W = make_rotation(float3(radians(rotationStep) * 0.75f, radians(rotationStep), radians(rotationStep) * 0.5f));

		_pCommandList->begin();

		commit_buffer(_pCommandList, TestConstants.get(), oGPUTestConstants(W, V, P, white));

		_pCommandList->clear(_pTarget, clear_type::color_depth_stencil);
		_pCommandList->set_blend_state(blend_state::opaque);
		_pCommandList->set_depth_stencil_state(depth_stencil_state::test_and_write);
		_pCommandList->set_surface_state(surface_state::front_face);
		_pCommandList->set_buffer(0, TestConstants);
		_pCommandList->set_sampler(0, sampler_type::linear_wrap);
		_pCommandList->set_shader_resource(0, _pTexture);
		_pCommandList->set_pipeline(PLTexture);
		_pCommandList->set_render_target(_pTarget);
		Cube->draw(_pCommandList);
		_pCommandList->end();
	}
};

oGPU_COMMON_TEST(render_target);

	} // namespace tests
} // namespace ouro
