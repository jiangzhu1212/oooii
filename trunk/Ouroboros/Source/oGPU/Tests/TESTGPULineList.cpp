// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.
#include <oPlatform/oTest.h>
#include "oGPUTestCommon.h"
#include <oGPU/vertex_buffer.h>

using namespace ouro::gpu;

namespace ouro {
	namespace tests {

static const int sSnapshotFrames[] = { 0 };
static const bool kIsDevMode = false;

struct oGPU_LINE_VERTEX
{
	float3 Position;
	color Color;
};

struct oGPU_LINE
{
	float3 Start;
	color StartColor;
	float3 End;
	color EndColor;
};

class gpu_test_lines : public gpu_test
{
public:
	gpu_test_lines() : gpu_test("GPU test: lines", kIsDevMode, sSnapshotFrames) {}
	~gpu_test_lines() { LineList.deinitialize(); }
	pipeline initialize()
	{
		LineList.initialize("LineList", Device, sizeof(oGPU_LINE_VERTEX), 6);

		pipeline p;
		p.input = gpu::intrinsic::vertex_layout::pos_color;
		p.vs = gpu::intrinsic::vertex_shader::pass_through_pos_color;
		p.ps = gpu::intrinsic::pixel_shader::vertex_color;

		return p;
	}

	void render()
	{
		command_list& cl = get_command_list();

		static const float3 TrianglePoints[] = { float3(-0.75f, -0.667f, 0.0f), float3(0.0f, 0.667f, 0.0f), float3(0.75f, -0.667f, 0.0f) };
		
		oGPU_LINE lines[3];

		lines[0].StartColor = red;
		lines[0].EndColor = green;
		lines[1].StartColor = green;
		lines[1].EndColor = blue;
		lines[2].StartColor = blue;
		lines[2].EndColor = red;

		lines[0].Start = TrianglePoints[0];
		lines[0].End = TrianglePoints[1];
		lines[1].Start = TrianglePoints[1];
		lines[1].End = TrianglePoints[2];
		lines[2].Start = TrianglePoints[2];
		lines[2].End = TrianglePoints[0];

		LineList.update(cl, 0, 6, lines);
		LineList.set(cl, 0);

		BlendState.set(cl, blend_state::opaque);
		DepthStencilState.set(cl, depth_stencil_state::none);
		RasterizerState.set(cl, rasterizer_state::two_sided);
		SamplerState.set(cl, sampler_state::linear_wrap, sampler_state::linear_wrap);

		VertexLayout.set(cl, mesh::primitive_type::lines);
		VertexShader.set(cl);
		PixelShader.set(cl);

		PrimaryColorTarget.clear(cl, get_clear_color());
		PrimaryDepthTarget.clear(cl);
		PrimaryColorTarget.set_draw_target(cl, PrimaryDepthTarget);

		LineList.draw_unindexed(cl, 6, 0);
	}

private:
	vertex_buffer LineList;
};

oGPU_COMMON_TEST(lines);

	} // namespace tests
} // namespace ouro
