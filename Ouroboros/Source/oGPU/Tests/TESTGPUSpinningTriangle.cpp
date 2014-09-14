// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.
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
		Mesh.initialize_first_triangle(Device);

		pipeline p;
		p.input = gpu::intrinsic::vertex_layout::pos;
		p.vs = gpu::intrinsic::vertex_shader::trivial_pos;
		p.ps = gpu::intrinsic::pixel_shader::white;
		return p;
	}

	void render() override
	{
		command_list& cl = get_command_list();

		float4x4 V = make_lookat_lh(float3(0.0f, 0.0f, -2.5f), oZERO3, float3(0.0f, 1.0f, 0.0f));

		uint2 dimensions = PrimaryColorTarget.dimensions();
		float4x4 P = make_perspective_lh(oDEFAULT_FOVY_RADIANS, dimensions.x / static_cast<float>(dimensions.y), 0.001f, 1000.0f);

		// this is -1 because there was a code change that resulted in begin_frame()
		// being moved out of the Render function below so it updated the FrameID
		// earlier than this code was ready for. If golden images are updated, this
		// could go away.
		float rotationRate = FrameID * 2.0f;
		float4x4 W = make_rotation(float3(0.0f, radians(rotationRate), 0.0f));

		uint DrawID = 0;
		TestConstants.update(cl, oGpuTrivialDrawConstants(W, V, P));

		BlendState.set(cl, blend_state::opaque);
		DepthStencilState.set(cl, depth_stencil_state::test_and_write);
		RasterizerState.set(cl, rasterizer_state::two_sided);

		TestConstants.set(cl, oGPU_TRIVIAL_DRAW_CONSTANTS_SLOT);

		VertexLayout.set(cl, mesh::primitive_type::triangles);
		VertexShader.set(cl);
		PixelShader.set(cl);

		PrimaryColorTarget.clear(cl, get_clear_color());
		PrimaryDepthTarget.clear(cl, 0);

		PrimaryColorTarget.set_draw_target(cl, PrimaryDepthTarget);
		Mesh.draw(cl);
	}

private:
	util_mesh Mesh;
};

oGPU_COMMON_TEST(spinning_triangle);

	}
}
