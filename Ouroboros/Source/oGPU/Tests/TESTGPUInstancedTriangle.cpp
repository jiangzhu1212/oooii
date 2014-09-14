// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.
#include <oPlatform/oTest.h>
#include "oGPUTestCommon.h"
#include <oGPU/constant_buffer.h>
#include <oGPU/command_list.h>

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
		InstanceList.initialize("Instances", Device, sizeof(oGpuTrivialInstanceConstants), 2);
		Mesh.initialize_first_triangle(Device);

		pipeline p;
		p.input = gpu::intrinsic::vertex_layout::pos;
		p.vs = gpu::intrinsic::vertex_shader::trivial_pos_color_instanced;
		p.ps = gpu::intrinsic::pixel_shader::white;
		return p;
	}
	
	void render() override
	{
		command_list& cl = get_command_list();

		float4x4 V = make_lookat_lh(float3(0.0f, 0.0f, -3.5f), oZERO3, float3(0.0f, 1.0f, 0.0f));

		uint2 dimensions = PrimaryColorTarget.dimensions();
		float4x4 P = make_perspective_lh(oDEFAULT_FOVY_RADIANS, dimensions.x / static_cast<float>(dimensions.y), 0.001f, 1000.0f);

		oGpuTrivialInstanceConstants instances[2];
		instances[0].translation = float3(-0.5f, 0.5f, 0.0f);
		instances[1].translation = float3(0.5f, -0.5f, 0.0f);

		float rotationStep = FrameID * 1.0f;
		instances[0].rotation = make_quaternion(float3(radians(rotationStep) * 0.75f, radians(rotationStep), radians(rotationStep) * 0.5f));
		instances[1].rotation = make_quaternion(float3(radians(rotationStep) * 0.5f, radians(rotationStep), radians(rotationStep) * 0.75f));

		InstanceList.update(cl, instances);
		TestConstants.update(cl, oGpuTrivialDrawConstants(oIDENTITY4x4, V, P));

		BlendState.set(cl, blend_state::opaque);
		DepthStencilState.set(cl, depth_stencil_state::test_and_write);
		RasterizerState.set(cl, rasterizer_state::two_sided);
		SamplerState.set(cl, sampler_state::linear_wrap, sampler_state::linear_wrap);
		VertexLayout.set(cl, mesh::primitive_type::triangles);
		VertexShader.set(cl);
		PixelShader.set(cl);

		const constant_buffer* CBs[2] = { &TestConstants, &InstanceList };
		constant_buffer::set(cl, oGPU_TRIVIAL_DRAW_CONSTANTS_SLOT, 2, CBs);

		PrimaryColorTarget.clear(cl, get_clear_color());
		PrimaryDepthTarget.clear(cl);
		PrimaryColorTarget.set_draw_target(cl, PrimaryDepthTarget);
		
		Mesh.draw(cl, 2);
	}

private:
	constant_buffer InstanceList;
	util_mesh Mesh;
};

oGPU_COMMON_TEST(instanced_triangle);

	}
}

