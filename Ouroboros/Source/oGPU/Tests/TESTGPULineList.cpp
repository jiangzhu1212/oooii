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
#include <oGPU/oGPUUtil.h>

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

	void initialize()
	{
		Pipeline = Device->make_pipeline1(oGPUTestGetPipeline(oGPU_TEST_PASS_THROUGH_COLOR));
		LineList = Device->make_vertex_buffer<oGPU_LINE_VERTEX>("LineList", 6);
	}

	void render()
	{
		CommandList->begin();

		surface::mapped_subresource msr = CommandList->reserve(LineList, 0);
		oGPU_LINE* pLines = (oGPU_LINE*)msr.data;

		static const float3 TrianglePoints[] = { float3(-0.75f, -0.667f, 0.0f), float3(0.0f, 0.667f, 0.0f), float3(0.75f, -0.667f, 0.0f) };

		pLines[0].StartColor = red;
		pLines[0].EndColor = green;
		pLines[1].StartColor = green;
		pLines[1].EndColor = blue;
		pLines[2].StartColor = blue;
		pLines[2].EndColor = red;

		pLines[0].Start = TrianglePoints[0];
		pLines[0].End = TrianglePoints[1];
		pLines[1].Start = TrianglePoints[1];
		pLines[1].End = TrianglePoints[2];
		pLines[2].Start = TrianglePoints[2];
		pLines[2].End = TrianglePoints[0];

		CommandList->commit(LineList, 0, msr, surface::box(6));

		CommandList->clear(PrimaryRenderTarget, gpu::clear_type::color_depth_stencil);
		CommandList->set_blend_state(gpu::blend_state::opaque);
		CommandList->set_depth_stencil_state(gpu::depth_stencil_state::none);
		CommandList->set_pipeline(Pipeline);
		CommandList->set_render_target(PrimaryRenderTarget);
		CommandList->draw(nullptr, 0, 1, &LineList, 0, 3);
		CommandList->end();
	}

private:
	std::shared_ptr<pipeline1> Pipeline;
	std::shared_ptr<buffer> LineList;
};

oGPU_COMMON_TEST(lines);

	} // namespace tests
} // namespace ouro
