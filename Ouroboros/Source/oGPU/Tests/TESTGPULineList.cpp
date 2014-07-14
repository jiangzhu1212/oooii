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
		LineList.initialize("LineList", Device.get(), sizeof(oGPU_LINE_VERTEX), 6);

		pipeline p;
		p.input = gfx::vertex_input::pos_color;
		p.vs = gfx::vertex_shader::pass_through_pos_color;
		p.ps = gfx::pixel_shader::vertex_color;

		return p;
	}

	void render()
	{
		CommandList->begin();

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

		LineList.update(CommandList.get(), 0, 6, lines);
		LineList.set(CommandList.get(), 0);

		BlendState.set(CommandList.get(), blend_state::opaque);
		DepthStencilState.set(CommandList.get(), depth_stencil_state::none);
		RasterizerState.set(CommandList.get(), rasterizer_state::two_sided);
		SamplerState.set(CommandList.get(), sampler_state::linear_wrap, sampler_state::linear_wrap);

		VertexLayout.set(CommandList.get(), mesh::primitive_type::lines);
		VertexShader.set(CommandList.get());
		PixelShader.set(CommandList.get());

		PrimaryColorTarget.clear(CommandList.get(), get_clear_color());
		PrimaryDepthTarget.clear(CommandList.get());
		PrimaryColorTarget.set_draw_target(CommandList.get(), PrimaryDepthTarget);

		LineList.draw_unindexed(CommandList.get(), 6, 0);

		CommandList->end();
	}

private:
	vertex_buffer LineList;
};

oGPU_COMMON_TEST(lines);

	} // namespace tests
} // namespace ouro
