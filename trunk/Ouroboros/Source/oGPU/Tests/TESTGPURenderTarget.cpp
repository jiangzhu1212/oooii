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
#include <oGPU/command_list.h>

#include <oBasis/oMath.h>

using namespace ouro::gpu;

namespace ouro {
	namespace tests {

static const int sSnapshotFrames[] = { 0, 50 };
static const bool kIsDevMode = true;//false;

struct gpu_test_render_target : public gpu_test
{
	gpu_test_render_target() : gpu_test("GPU test: render_target", kIsDevMode, sSnapshotFrames) {}

	pipeline initialize() override
	{
		CLMainScene.initialize("CLMainScene", Device, 1);
		CLRenderTarget.initialize("CLRenderTarget", Device, 0);
		
		TestConstants.initialize("TestConstants", Device, sizeof(oGfxDrawConstants));
		Triangle.initialize_first_triangle(Device);

		MainVertexLayout.initialize("Main layout", Device, gfx::elements(gfx::vertex_input::pos_uv), gfx::vs_byte_code(gfx::vertex_input::pos_uv));
		MainVertexShader.initialize("Main layout", Device, gfx::byte_code(gfx::vertex_shader::texture2d));
		MainPixelShader.initialize("Main layout", Device, gfx::byte_code(gfx::pixel_shader::texture2d));

		Cube.initialize_first_cube(Device);

		ColorTarget.initialize("ColorTarget", Device, surface::b8g8r8a8_unorm, 256, 256, 0, false);
		DepthTarget.initialize("DepthTarget", Device, surface::d24_unorm_s8_uint, 256, 256, 0, false, 0);
	
		pipeline p;
		p.input = gfx::vertex_input::pos;
		p.vs = gfx::vertex_shader::pass_through_pos;
		p.ps = gfx::pixel_shader::white;
		return p;
	}

	void render() override
	{
		float4x4 V = make_lookat_lh(float3(0.0f, 0.0f, -4.5f), oZERO3, float3(0.0f, 1.0f, 0.0f));

		uint2 dimensions = PrimaryColorTarget.dimensions();
		float4x4 P = make_perspective_lh(oDEFAULT_FOVY_RADIANS, dimensions.x / static_cast<float>(dimensions.y), 0.001f, 1000.0f);

		float rotationStep = FrameID * 1.0f;
		float4x4 W = make_rotation(float3(radians(rotationStep) * 0.75f, radians(rotationStep), radians(rotationStep) * 0.5f));

		// DrawOrder should be respected in out-of-order submits so show that here
		// by executing the main scene THEN the render target but because the
		// draw order of the command lists defines the render target before the 
		// main scene this should come out as a cube with a triangle texture.

		render_main_scene(CLMainScene, ColorTarget);
		render_to_target(CLRenderTarget, ColorTarget);

		CLRenderTarget.flush();
		CLMainScene.flush();
	}

private:
	command_list CLMainScene;
	command_list CLRenderTarget;
	vertex_layout MainVertexLayout;
	vertex_shader MainVertexShader;
	pixel_shader MainPixelShader;
	color_target ColorTarget;
	depth_target DepthTarget;
	util_mesh Cube;
	util_mesh Triangle;
	constant_buffer TestConstants;

	void render_to_target(command_list& cl, color_target& rt)
	{
		rt.clear(cl, deep_sky_blue);
		BlendState.set(cl, blend_state::opaque);
		DepthStencilState.set(cl, depth_stencil_state::none);
		RasterizerState.set(cl, rasterizer_state::front_face);
		SamplerState.set(cl, sampler_state::linear_wrap, sampler_state::linear_wrap);
		VertexLayout.set(cl, mesh::primitive_type::triangles);
		VertexShader.set(cl);
		PixelShader.set(cl);
		rt.set_draw_target(cl);
		Triangle.draw(cl);
		cl.flush();
	}

	void render_main_scene(command_list& cl, resource& texture)
	{
		float4x4 V = make_lookat_lh(float3(0.0f, 0.0f, -4.5f), oZERO3, float3(0.0f, 1.0f, 0.0f));

		uint2 dimensions = PrimaryColorTarget.dimensions();
		float4x4 P = make_perspective_lh(oDEFAULT_FOVY_RADIANS, dimensions.x / static_cast<float>(dimensions.y), 0.001f, 1000.0f);

		float rotationStep = FrameID * 1.0f;
		float4x4 W = make_rotation(float3(radians(rotationStep) * 0.75f, radians(rotationStep), radians(rotationStep) * 0.5f));

		oGfxDrawConstants c(oIDENTITY4x4, V, P, aaboxf());
		c.Color = white;
		TestConstants.update(cl, c);

		BlendState.set(cl, blend_state::opaque);
		DepthStencilState.set(cl, depth_stencil_state::test_and_write);
		RasterizerState.set(cl, rasterizer_state::front_face);
		SamplerState.set(cl, sampler_state::linear_wrap, sampler_state::linear_wrap);
		TestConstants.set(cl, oGFX_DRAW_CONSTANTS_REGISTER);
		texture.set(cl, 0);

		MainVertexLayout.set(cl, mesh::primitive_type::triangles);
		MainVertexShader.set(cl);
		MainPixelShader.set(cl);

		PrimaryColorTarget.clear(cl, get_clear_color());
		PrimaryDepthTarget.clear(cl);
		PrimaryColorTarget.set_draw_target(cl, PrimaryDepthTarget);
		Cube.draw(cl);
		cl.flush();
	}
};

oGPU_COMMON_TEST(render_target);

	} // namespace tests
} // namespace ouro
