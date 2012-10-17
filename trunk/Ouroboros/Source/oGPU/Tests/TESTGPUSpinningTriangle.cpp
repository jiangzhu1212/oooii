/**************************************************************************
 * The MIT License                                                        *
 * Copyright (c) 2011 Antony Arciuolo & Kevin Myers                       *
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
#include <oGPU/oGPUViewConstants.h>
#include <oGPU/oGPUDrawConstants.h>

struct TESTGPUSpinningTriangle : public oTest
{
	oRef<oGPUDevice> Device;
	oRef<oGPUCommandList> CL;
	oRef<oGPUPipeline> Pipeline;
	oRef<oGPUMesh> Mesh;
	oRef<oGPUBuffer> ViewConstants;
	oRef<oGPUBuffer> DrawConstants;
	
	void Render(oGPURenderTarget* _pPrimaryRenderTarget)
	{
		static bool once = false;
		if (!once)
		{
			oGPU_CLEAR_DESC CD;
			CD.ClearColor[0] = std::AlmostBlack;
			_pPrimaryRenderTarget->SetClearDesc(CD);

			once = true;
		}

		float4x4 V = oCreateLookAtLH(float3(0.0f, 0.0f, -2.5f), oZERO3, float3(0.0f, 1.0f, 0.0f));

		oGPURenderTarget::DESC RTDesc;
		_pPrimaryRenderTarget->GetDesc(&RTDesc);
		float4x4 P = oCreatePerspectiveLH(oPIf/4.0f, RTDesc.Dimensions.x / oCastAsFloat(RTDesc.Dimensions.y), 0.001f, 1000.0f);

		float rotationRate = Device->GetFrameID() * 2.0f;
		float4x4 W = oCreateRotation(float3(0.0f, radians(rotationRate), 0.0f));

		uint DrawID = 0;

		if (!Device->BeginFrame())
			return;
		CL->Begin();

		oGPUCommitBuffer(CL, ViewConstants, oGPUViewConstants(V, P, oCastAsFloat(RTDesc.Dimensions), 0));
		oGPUCommitBuffer(CL, DrawConstants, oGPUDrawConstants(W, V, P, 0, DrawID++));

		CL->Clear(_pPrimaryRenderTarget, oGPU_CLEAR_COLOR_DEPTH_STENCIL);
		CL->SetBlendState(oGPU_OPAQUE);
		CL->SetDepthStencilState(oGPU_DEPTH_TEST_AND_WRITE);
		CL->SetSurfaceState(oGPU_TWO_SIDED);
		CL->SetConstants(0, 2, &ViewConstants); // let the set run from ViewConstants to DrawConstants
		CL->SetPipeline(Pipeline);
		CL->SetRenderTarget(_pPrimaryRenderTarget);
		CL->Draw(Mesh, 0);

		CL->End();
		Device->EndFrame();
	}

	RESULT Run(char* _StrStatus, size_t _SizeofStrStatus) override
	{
		static const int sSnapshotFrames[] = { 0, 2, 4, 6 };
		static const bool kIsDevMode = false;
		oGPU_TEST_WINDOW_INIT Init(kIsDevMode, oBIND(&TESTGPUSpinningTriangle::Render, this, oBIND1), "TESTGPUSpinningTriangle", sSnapshotFrames);

		oStd::future<oRef<oImage>> Snapshots[oCOUNTOF(sSnapshotFrames)];
		oRef<threadsafe oGPUWindow> Window;
		oTESTB0(oGPUTestCreateWindow(Init, [&](threadsafe oGPUWindow* _pWindow)->bool
		{
			_pWindow->GetDevice(&Device);
			oGPUCommandList::DESC cld;
			cld.DrawOrder = 0;

			if (!Device->CreateCommandList("CommandList", cld, &CL))
				return false;

			oGPUBuffer::DESC DCDesc;
			DCDesc.StructByteSize = sizeof(oGPUViewConstants);
			if (!Device->CreateBuffer("ViewConstants", DCDesc, &ViewConstants))
				return false;

			DCDesc.StructByteSize = sizeof(oGPUDrawConstants);
			if (!Device->CreateBuffer("DrawConstants", DCDesc, &DrawConstants))
				return false;

			oGPUPipeline::DESC pld;
			if (!oGPUTestGetPipeline(oGPU_TEST_TRANSFORMED_WHITE, &pld))
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

oTEST_REGISTER(TESTGPUSpinningTriangle);
