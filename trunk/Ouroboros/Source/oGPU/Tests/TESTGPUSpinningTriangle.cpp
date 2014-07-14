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

static const int sSnapshotFrames[] = { 0, 1, 3, 5 };
static const bool kIsDevMode = false;

class gpu_test_spinning_triangle : public gpu_test
{
public:
	gpu_test_spinning_triangle() : gpu_test("GPU test: spinning triangle", kIsDevMode, sSnapshotFrames) {}

	pipeline initialize() override
	{
		TestConstants.initialize("TestConstants", Device.get(), sizeof(oGfxDrawConstants));
		Mesh.initialize_first_triangle(Device.get());

		pipeline p;
		p.input = gfx::vertex_input::pos;
		p.vs = gfx::vertex_shader::test_transform;
		p.ps = gfx::pixel_shader::white;
		return p;
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
		float rotationRate = (Device->frame_id()) * 2.0f;
		float4x4 W = make_rotation(float3(0.0f, radians(rotationRate), 0.0f));

		uint DrawID = 0;

		CommandList->begin();

		oGfxDrawConstants c(W, V, P, aaboxf());
		c.Color = white;
		TestConstants.update(CommandList.get(), c);

		BlendState.set(CommandList.get(), blend_state::opaque);
		DepthStencilState.set(CommandList.get(), depth_stencil_state::test_and_write);
		RasterizerState.set(CommandList.get(), rasterizer_state::two_sided);

		TestConstants.set(CommandList.get(), oGFX_DRAW_CONSTANTS_REGISTER);

		VertexLayout.set(CommandList.get(), mesh::primitive_type::triangles);
		VertexShader.set(CommandList.get());
		PixelShader.set(CommandList.get());

		PrimaryColorTarget.clear(CommandList.get(), get_clear_color());
		PrimaryDepthTarget.clear(CommandList.get(), 0);

		PrimaryColorTarget.set_draw_target(CommandList.get(), PrimaryDepthTarget);
		Mesh.draw(CommandList.get());

		CommandList->end();
	}

private:
	util_mesh Mesh;
	constant_buffer TestConstants;
};

oGPU_COMMON_TEST(spinning_triangle);

	} // namespace tests
} // namespace ouro
