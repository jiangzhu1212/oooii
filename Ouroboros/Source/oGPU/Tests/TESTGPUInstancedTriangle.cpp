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

static const int sSnapshotFrames[] = { 0, 22, 45 };
static const bool kIsDevMode = false;

struct gpu_test_instanced_triangle : public gpu_test
{
	gpu_test_instanced_triangle() : gpu_test("GPU test: instanced triangle", kIsDevMode, sSnapshotFrames) {}

	pipeline initialize() override
	{
		TestConstants.initialize("TestConstants", Device.get(), sizeof(oGfxDrawConstants));
		InstanceList.initialize("Instances", Device.get(), sizeof(oGfxTestInstance), 2);
		Mesh.initialize_first_triangle(Device.get());

		pipeline p;
		p.input = gfx::vertex_input::pos;
		p.vs = gfx::vertex_shader::test_instanced;
		p.ps = gfx::pixel_shader::white;
		return p;
	}
	
	void render() override
	{
		float4x4 V = make_lookat_lh(float3(0.0f, 0.0f, -3.5f), oZERO3, float3(0.0f, 1.0f, 0.0f));

		uint2 dimensions = PrimaryColorTarget.dimensions();
		float4x4 P = make_perspective_lh(oDEFAULT_FOVY_RADIANS, dimensions.x / static_cast<float>(dimensions.y), 0.001f, 1000.0f);

		oGfxTestInstance instances[2];
		instances[0].Translation = float3(-0.5f, 0.5f, 0.0f);
		instances[1].Translation = float3(0.5f, -0.5f, 0.0f);

		float rotationStep = Device->frame_id() * 1.0f;
		instances[0].Rotation = make_quaternion(float3(radians(rotationStep) * 0.75f, radians(rotationStep), radians(rotationStep) * 0.5f));
		instances[1].Rotation = make_quaternion(float3(radians(rotationStep) * 0.5f, radians(rotationStep), radians(rotationStep) * 0.75f));

		InstanceList.update(CommandList.get(), instances);

		CommandList->begin();

		oGfxDrawConstants c(oIDENTITY4x4, V, P, aaboxf());
		c.Color = white;
		TestConstants.update(CommandList.get(), c);

		BlendState.set(CommandList.get(), blend_state::opaque);
		DepthStencilState.set(CommandList.get(), depth_stencil_state::test_and_write);
		RasterizerState.set(CommandList.get(), rasterizer_state::two_sided);
		SamplerState.set(CommandList.get(), sampler_state::linear_wrap, sampler_state::linear_wrap);
		VertexLayout.set(CommandList.get(), mesh::primitive_type::triangles);
		VertexShader.set(CommandList.get());
		PixelShader.set(CommandList.get());

		const constant_buffer* CBs[2] = { &TestConstants, &InstanceList };
		constant_buffer::set(CommandList.get(), oGFX_DRAW_CONSTANTS_REGISTER, 2, CBs);

		PrimaryColorTarget.clear(CommandList.get(), get_clear_color());
		PrimaryDepthTarget.clear(CommandList.get());
		PrimaryColorTarget.set_draw_target(CommandList.get(), PrimaryDepthTarget);
		
		Mesh.draw(CommandList.get(), 2);

		CommandList->end();
	}

private:
	constant_buffer InstanceList;
	constant_buffer TestConstants;
	util_mesh Mesh;
};

oGPU_COMMON_TEST(instanced_triangle);

	} // namespace tests
} // namespace ouro

