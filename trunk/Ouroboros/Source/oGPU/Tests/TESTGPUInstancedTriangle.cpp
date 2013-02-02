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
#include <oGPU/oGPUViewConstants.h>
#include <oGPU/oGPUDrawConstants.h>

struct GPU_InstancedTriangle : public oTest
{
	oRef<oGPUDevice> Device;
	oRef<oGPUCommandList> CL;
	oRef<oGPUPipeline> Pipeline;
	oRef<oGPUInstanceList> InstanceList;
	oRef<oGPUMesh> Mesh;
	oRef<oGPUBuffer> ViewConstants;
	oRef<oGPUBuffer> DrawConstants;
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

		float4x4 V = oCreateLookAtLH(float3(0.0f, 0.0f, -3.5f), oZERO3, float3(0.0f, 1.0f, 0.0f));

		oGPURenderTarget::DESC RTDesc;
		_pPrimaryRenderTarget->GetDesc(&RTDesc);
		float4x4 P = oCreatePerspectiveLH(oPIf/4.0f, RTDesc.Dimensions.x / oCastAsFloat(RTDesc.Dimensions.y), 0.001f, 1000.0f);

		{
			oSURFACE_MAPPED_SUBRESOURCE msr;
			CL->Reserve(InstanceList, 0, &msr);
			oGPU_TEST_INSTANCE* pInstances = (oGPU_TEST_INSTANCE*)msr.pData;
			{
				pInstances[0].Translation = float3(-0.5f, 0.5f, 0.0f);
				pInstances[1].Translation = float3(0.5f, -0.5f, 0.0f);

				float rotationStep = Device->GetFrameID() * 1.0f;
				pInstances[0].Rotation = oCreateRotationQ(float3(radians(rotationStep) * 0.75f, radians(rotationStep), radians(rotationStep) * 0.5f));
				pInstances[1].Rotation = oCreateRotationQ(float3(radians(rotationStep) * 0.5f, radians(rotationStep), radians(rotationStep) * 0.75f));
			}
			CL->Commit(InstanceList, 0, msr, oGPU_BOX(2));
		}

		uint DrawID = 0;

		if (!Device->BeginFrame())
			return;
		CL->Begin();

		oGPUCommitBuffer(CL, ViewConstants, oGPUViewConstants(V, P, RTDesc.Dimensions, 0));
		oGPUCommitBuffer(CL, DrawConstants, oGPUDrawConstants(float4x4::Identity, V, P, 0, DrawID++));

		CL->Clear(_pPrimaryRenderTarget, oGPU_CLEAR_COLOR_DEPTH_STENCIL);
		CL->SetBlendState(oGPU_OPAQUE);
		CL->SetDepthStencilState(oGPU_DEPTH_TEST_AND_WRITE);
		CL->SetSurfaceState(oGPU_TWO_SIDED);
		CL->SetBuffers(0, 2, &ViewConstants); // let the set run from ViewConstants to DrawConstants
		CL->SetPipeline(Pipeline);
		CL->SetRenderTarget(_pPrimaryRenderTarget);
		CL->Draw(Mesh, 0, InstanceList);

		CL->End();
		Device->EndFrame();
	}

	RESULT Run(char* _StrStatus, size_t _SizeofStrStatus) override
	{
		Once = false;

		static const int sSnapshotFrames[] = { 1, 23, 46 };
		static const bool kIsDevMode = false;
		oGPU_TEST_WINDOW_INIT Init(kIsDevMode, oBIND(&GPU_InstancedTriangle::Render, this, oBIND1), "GPU_InstancedTriangle", sSnapshotFrames);

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
			if (!oGPUTestGetPipeline(oGPU_TEST_TRANSFORMED_WHITE_INSTANCED, &pld))
				return false;

			oGPU_INSTANCE_LIST_DESC ild;
			ild.InputSlot = 1;
			ild.MaxNumInstances = 2;
			ild.NumInstances = 0;
			ild.NumVertexElements = pld.NumElements;
			memcpy(ild.VertexElements, pld.pElements, sizeof(oGPU_VERTEX_ELEMENT) * pld.NumElements);
			if (!Device->CreateInstanceList("InstanceList", ild, &InstanceList))
				return false;

			if (!Device->CreatePipeline(pld.DebugName, pld, &Pipeline))
				return false;

			// Create geometry off the non-instanced version, i.e. the one without
			// the instanced channel.

			// @oooii-tony: Maybe there should be a util that can recognize and skip
			// instanced elements?
			if (!oGPUTestGetPipeline(oGPU_TEST_TRANSFORMED_WHITE, &pld))
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

oTEST_REGISTER(GPU_InstancedTriangle);
