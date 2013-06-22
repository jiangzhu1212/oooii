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

struct GPU_Triangle : public oTest
{
	oRef<oGPUDevice> Device;
	oRef<oGPUCommandList> CL;
	oRef<oGPUPipeline> Pipeline;
	oRef<oGPUUtilMesh> Mesh;
	bool Once;

	void Render(oGPURenderTarget* _pPrimaryRenderTarget)
	{
		if (!Once)
		{
			oGPU_CLEAR_DESC CD;
			CD.ClearColor[0] = oStd::AlmostBlack;
			_pPrimaryRenderTarget->SetClearDesc(CD);

			Once = true;
		}

		if (!Device->BeginFrame())
			return;
		CL->Begin();

		CL->Clear(_pPrimaryRenderTarget, oGPU_CLEAR_COLOR_DEPTH_STENCIL);
		CL->SetBlendState(oGPU_OPAQUE);
		CL->SetDepthStencilState(oGPU_DEPTH_STENCIL_NONE);
		CL->SetSurfaceState(oGPU_FRONT_FACE);
		CL->SetPipeline(Pipeline);
		CL->SetRenderTarget(_pPrimaryRenderTarget);
		oGPUUtilMeshDraw(CL, Mesh);

		CL->End();
		Device->EndFrame();
	}

	RESULT Run(char* _StrStatus, size_t _SizeofStrStatus) override
	{
		Once = false;

		static const int sSnapshotFrames[] = { 0 };
		static const bool kIsDevMode = false;

		oGPU_TEST_WINDOW_INIT Init(kIsDevMode, oBIND(&GPU_Triangle::Render, this, oBIND1), "GPU_Triangle", sSnapshotFrames);

		oStd::future<oRef<oImage>> Snapshots[oCOUNTOF(sSnapshotFrames)];
		oRef<threadsafe oGPUWindow> Window;
		oTESTB0(oGPUTestCreateWindow(Init, [&](threadsafe oGPUWindow* _pWindow)->bool
		{
			_pWindow->GetDevice(&Device);
			oGPUCommandList::DESC cld;
			cld.DrawOrder = 0;

			Device->GetImmediateCommandList(&CL);

			oGPUPipeline::DESC pld;
			if (!oGPUTestGetPipeline(oGPU_TEST_PASS_THROUGH, &pld))
				return false;

			if (!Device->CreatePipeline(pld.DebugName, pld, &Pipeline))
				return false;

			if (!oGPUTestInitFirstTriangle(Device, "Triangle", pld.pElements, pld.NumElements, &Mesh))
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

oTEST_REGISTER(GPU_Triangle);
