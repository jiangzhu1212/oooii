/**************************************************************************
 * The MIT License                                                        *
 * Copyright (c) 2014 Antony Arciuolo.                                    *
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
		Mesh.initialize_first_triangle(Device);

		pipeline p;
		p.input = gpu::intrinsic::vertex_layout::pos;
		p.vs = gpu::intrinsic::vertex_shader::pass_through_pos;
		p.ps = gpu::intrinsic::pixel_shader::white;
		return p;
	}

	void render() override
	{
		command_list& cl = get_command_list();
		BlendState.set(cl, blend_state::opaque);
		DepthStencilState.set(cl, depth_stencil_state::none);
		RasterizerState.set(cl, rasterizer_state::front_face);
		VertexLayout.set(cl, mesh::primitive_type::triangles);
		VertexShader.set(cl);
		PixelShader.set(cl);
		PrimaryColorTarget.clear(cl, get_clear_color());
		PrimaryDepthTarget.clear(cl);
		PrimaryColorTarget.set_draw_target(cl, PrimaryDepthTarget);
		Mesh.draw(cl);
	}

private:
	util_mesh Mesh;
};

oGPU_COMMON_TEST(triangle);

	} // namespace tests
} // namespace ouro
