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

static const int sSnapshotFrames[] = { 0 };
static const bool kIsDevMode = false;

struct oGPU_LINE_VERTEX
{
	float3 Position;
	oStd::color Color;
};

struct oGPU_LINE
{
	float3 Start;
	oStd::color StartColor;
	float3 End;
	oStd::color EndColor;
};

class GPU_LineList_App : public oGPUTestApp
{
public:
	GPU_LineList_App() : oGPUTestApp("GPU_LineList", kIsDevMode, sSnapshotFrames) {}

	bool Initialize()
	{
		PrimaryRenderTarget->SetClearColor(oStd::AlmostBlack);

		oGPUPipeline::DESC pld;
		if (!oGPUTestGetPipeline(oGPU_TEST_PASS_THROUGH_COLOR, &pld))
			return false;

		if (!Device->CreatePipeline(pld.DebugName, pld, &Pipeline))
			return false;

		oGPU_BUFFER_DESC bd;
		bd.Type = oGPU_BUFFER_VERTEX;
		bd.StructByteSize = sizeof(oGPU_LINE_VERTEX);
		bd.ArraySize = 6;

		if (!Device->CreateBuffer("LineList", bd, &LineList))
			return false;

		return true;
	}

	bool Render()
	{
		CommandList->Begin();

		oSURFACE_MAPPED_SUBRESOURCE msr;
		CommandList->Reserve(LineList, 0, &msr);
		oGPU_LINE* pLines = (oGPU_LINE*)msr.pData;

		static const float3 TrianglePoints[] = { float3(-0.75f, -0.667f, 0.0f), float3(0.0f, 0.667f, 0.0f), float3(0.75f, -0.667f, 0.0f) };

		pLines[0].StartColor = oStd::Red;
		pLines[0].EndColor = oStd::Green;
		pLines[1].StartColor = oStd::Green;
		pLines[1].EndColor = oStd::Blue;
		pLines[2].StartColor = oStd::Blue;
		pLines[2].EndColor = oStd::Red;

		pLines[0].Start = TrianglePoints[0];
		pLines[0].End = TrianglePoints[1];
		pLines[1].Start = TrianglePoints[1];
		pLines[1].End = TrianglePoints[2];
		pLines[2].Start = TrianglePoints[2];
		pLines[2].End = TrianglePoints[0];

		CommandList->Commit(LineList, 0, msr, oSURFACE_BOX(6));

		CommandList->Clear(PrimaryRenderTarget, oGPU_CLEAR_COLOR_DEPTH_STENCIL);
		CommandList->SetBlendState(oGPU_OPAQUE);
		CommandList->SetDepthStencilState(oGPU_DEPTH_STENCIL_NONE);
		CommandList->SetPipeline(Pipeline);
		CommandList->SetRenderTarget(PrimaryRenderTarget);
		CommandList->Draw(nullptr, 0, 1, &LineList, 0, 3);
		CommandList->End();
		return true;
	}

private:
	oRef<oGPUPipeline> Pipeline;
	oRef<oGPUBuffer> LineList;
};

oDEFINE_GPU_TEST(GPU_LineList)