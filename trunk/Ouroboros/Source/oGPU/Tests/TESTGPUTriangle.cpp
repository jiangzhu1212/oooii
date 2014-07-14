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

using namespace ouro::gpu;

namespace ouro {
	namespace tests {

static const int sSnapshotFrames[] = { 0 };
static const bool kIsDevMode = false;

struct gpu_test_triangle : public gpu_test
{
	gpu_test_triangle() : gpu_test("GPU test: triangle", kIsDevMode, sSnapshotFrames) {}

	pipeline initialize() override
	{
		Mesh.initialize_first_triangle(Device.get());

		pipeline p;
		p.input = gfx::vertex_input::pos;
		p.vs = gfx::vertex_shader::pass_through_pos;
		p.ps = gfx::pixel_shader::white;
		return p;
	}

	void render() override
	{
		CommandList->begin();
		BlendState.set(CommandList.get(), blend_state::opaque);
		DepthStencilState.set(CommandList.get(), depth_stencil_state::none);
		RasterizerState.set(CommandList.get(), rasterizer_state::front_face);
		VertexLayout.set(CommandList.get(), mesh::primitive_type::triangles);
		VertexShader.set(CommandList.get());
		PixelShader.set(CommandList.get());
		PrimaryColorTarget.clear(CommandList.get(), get_clear_color());
		PrimaryDepthTarget.clear(CommandList.get());
		PrimaryColorTarget.set_draw_target(CommandList.get(), PrimaryDepthTarget);
		Mesh.draw(CommandList.get());
		CommandList->end();
	}

private:
	util_mesh Mesh;
};

oGPU_COMMON_TEST(triangle);

	} // namespace tests
} // namespace ouro
