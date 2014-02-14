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

using namespace ouro;

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

class GPU_LineList_App : public oGPUTestApp
{
public:
	GPU_LineList_App() : oGPUTestApp("GPU_LineList", kIsDevMode, sSnapshotFrames) {}

	bool Initialize()
	{
		PrimaryRenderTarget->SetClearColor(AlmostBlack);

		oGPUPipeline::DESC pld = oGPUTestGetPipeline(oGPU_TEST_PASS_THROUGH_COLOR);
		if (!Device->CreatePipeline(pld.debug_name, pld, &Pipeline))
			return false;

		gpu::buffer_info i;
		i.type = gpu::buffer_type::vertex;
		i.struct_byte_size = sizeof(oGPU_LINE_VERTEX);
		i.array_size = 6;

		if (!Device->CreateBuffer("LineList", i, &LineList))
			return false;

		return true;
	}

	bool Render()
	{
		CommandList->Begin();

		ouro::surface::mapped_subresource msr;
		CommandList->Reserve(LineList, 0, &msr);
		oGPU_LINE* pLines = (oGPU_LINE*)msr.data;

		static const float3 TrianglePoints[] = { float3(-0.75f, -0.667f, 0.0f), float3(0.0f, 0.667f, 0.0f), float3(0.75f, -0.667f, 0.0f) };

		pLines[0].StartColor = Red;
		pLines[0].EndColor = Green;
		pLines[1].StartColor = Green;
		pLines[1].EndColor = Blue;
		pLines[2].StartColor = Blue;
		pLines[2].EndColor = Red;

		pLines[0].Start = TrianglePoints[0];
		pLines[0].End = TrianglePoints[1];
		pLines[1].Start = TrianglePoints[1];
		pLines[1].End = TrianglePoints[2];
		pLines[2].Start = TrianglePoints[2];
		pLines[2].End = TrianglePoints[0];

		CommandList->Commit(LineList, 0, msr, ouro::surface::box(6));

		CommandList->Clear(PrimaryRenderTarget, ouro::gpu::clear_type::color_depth_stencil);
		CommandList->SetBlendState(ouro::gpu::blend_state::opaque);
		CommandList->SetDepthStencilState(ouro::gpu::depth_stencil_state::none);
		CommandList->SetPipeline(Pipeline);
		CommandList->SetRenderTarget(PrimaryRenderTarget);
		CommandList->Draw(nullptr, 0, 1, &LineList, 0, 3);
		CommandList->End();
		return true;
	}

private:
	intrusive_ptr<oGPUPipeline> Pipeline;
	intrusive_ptr<oGPUBuffer> LineList;
};

oDEFINE_GPU_TEST(GPU_LineList)