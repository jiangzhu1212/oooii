/**************************************************************************
 * The MIT License                                                        *
 * Copyright (c) 2013 OOOii.                                              *
 * antony.arciuolo@oooii.com                                              *
 * kevin.myers@oooii.com                                                  *
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

struct GPU_LineList : public oTest
{
	oRef<oGPUDevice> Device;
	oRef<oGPUCommandList> CL;
	oRef<oGPUPipeline> Pipeline;
	oRef<oGPULineList> LineList;
	bool Once;

	void Render(oGPURenderTarget* _pPrimaryRenderTarget)
	{
		if (!Once)
		{
			oGPU_CLEAR_DESC CD;
			CD.ClearColor[0] = std::AlmostBlack;
			_pPrimaryRenderTarget->SetClearDesc(CD);

			Once = true;
		}

		if (!Device->BeginFrame())
			return;
		CL->Begin();

		oSURFACE_MAPPED_SUBRESOURCE msr;
		CL->Reserve(LineList, 0, &msr);
		oGPU_LINE* pLines = (oGPU_LINE*)msr.pData;
		oGPULineList::DESC lld;
		LineList->GetDesc(&lld);

		static const float3 TrianglePoints[] = { float3(-0.75f, -0.667f, 0.0f), float3(0.0f, 0.667f, 0.0f), float3(0.75f, -0.667f, 0.0f) };

		pLines[0].StartColor = std::Red;
		pLines[0].EndColor = std::Green;
		pLines[1].StartColor = std::Green;
		pLines[1].EndColor = std::Blue;
		pLines[2].StartColor = std::Blue;
		pLines[2].EndColor = std::Red;

		pLines[0].Start = TrianglePoints[0];
		pLines[0].End = TrianglePoints[1];
		pLines[1].Start = TrianglePoints[1];
		pLines[1].End = TrianglePoints[2];
		pLines[2].Start = TrianglePoints[2];
		pLines[2].End = TrianglePoints[0];

		CL->Commit(LineList, 0, msr, oGPU_BOX(3));

		CL->Clear(_pPrimaryRenderTarget, oGPU_CLEAR_COLOR_DEPTH_STENCIL);
		CL->SetBlendState(oGPU_OPAQUE);
		CL->SetDepthStencilState(oGPU_DEPTH_STENCIL_NONE);
		CL->SetPipeline(Pipeline);
		CL->SetRenderTarget(_pPrimaryRenderTarget);
		CL->Draw(LineList);

		CL->End();
		Device->EndFrame();
	}

	RESULT Run(char* _StrStatus, size_t _SizeofStrStatus) override
	{
		Once = false;

		static const int sSnapshotFrames[] = { 0 };
		static const bool kIsDevMode = false;
		oGPU_TEST_WINDOW_INIT Init(kIsDevMode, oBIND(&GPU_LineList::Render, this, oBIND1), "GPU_LineList", sSnapshotFrames);

		oStd::future<oRef<oImage>> Snapshots[oCOUNTOF(sSnapshotFrames)];
		oRef<threadsafe oGPUWindow> Window;
		oTESTB0(oGPUTestCreateWindow(Init, [&](threadsafe oGPUWindow* _pWindow)->bool
		{
			_pWindow->GetDevice(&Device);
			oGPUCommandList::DESC cld;
			cld.DrawOrder = 0;

			if (!Device->CreateCommandList("CommandList", cld, &CL))
				return false;

			oGPUPipeline::DESC pld;
			if (!oGPUTestGetPipeline(oGPU_TEST_PASS_THROUGH_COLOR, &pld))
				return false;

			if (!Device->CreatePipeline(pld.DebugName, pld, &Pipeline))
				return false;

			oGPULineList::DESC lld;
			lld.MaxNumLines = 16;
			lld.NumLines = 0;

			if (!Device->CreateLineList("LineList", lld, &LineList))
				return false;

			return true;

		}, Snapshots, &Window));

		while (Window->IsOpen())
		{
			if (!kIsDevMode && oGPUTestSnapshotsAreReady(Snapshots))
			{
				Window->Close();
				oTESTB0(oGPUTestSnapshots(this, Snapshots));
			}

			oSleep(16);
		}

		return SUCCESS;
	}
};

oTEST_REGISTER(GPU_LineList);
