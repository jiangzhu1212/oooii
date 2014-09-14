// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.
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

	}
}
