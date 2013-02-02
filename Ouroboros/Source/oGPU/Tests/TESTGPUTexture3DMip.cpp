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

struct TESTGPUTexture3DMip : public oTest
{
	oRef<oGPUDevice> Device;
	oRef<oGPUCommandList> CL;
	oRef<oGPUPipeline> Pipeline;
	oRef<oGPUTexture> Texture;
	oRef<oGPUMesh> Mesh;
	oRef<oGPUBuffer> ViewConstants;
	oRef<oGPUBuffer> DrawConstants;
	bool Once;

	bool CreateResources(threadsafe oGPUWindow* _pWindow)
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
		if (!oGPUTestGetPipeline(oGPU_TEST_TEXTURE_3D, &pld))
			return false;

		if (!Device->CreatePipeline(pld.DebugName, pld, &Pipeline))
			return false;

		if (!oGPUTestInitCube(Device, "Cube", pld.pElements, pld.NumElements, &Mesh))
			return false;

		oRef<oImage> images[3];
		if (!oImageLoad("file://DATA/Test/Textures/Red.png", oImage::FORCE_ALPHA, &images[0]))
			return false;
		if (!oImageLoad("file://DATA/Test/Textures/Green.png", oImage::FORCE_ALPHA, &images[1]))
			return false;
		if (!oImageLoad("file://DATA/Test/Textures/Blue.png", oImage::FORCE_ALPHA, &images[2]))
			return false;

		if (!oGPUCreateTexture3DMip(Device, (const oImage**)&images[0], oCOUNTOF(images), &Texture))
			return false;

		return true;
	}

	void Render(oGPURenderTarget* _pPrimaryRenderTarget)
	{
		if (!Once)
		{
			oGPU_CLEAR_DESC CD;
			CD.ClearColor[0] = std::AlmostBlack;
			_pPrimaryRenderTarget->SetClearDesc(CD);

			Once = true;
		}

		float4x4 V = oCreateLookAtLH(float3(0.0f, 0.0f, -4.5f), oZERO3, float3(0.0f, 1.0f, 0.0f));

		oGPURenderTarget::DESC RTDesc;
		_pPrimaryRenderTarget->GetDesc(&RTDesc);
		float4x4 P = oCreatePerspectiveLH(oPIf/4.0f, RTDesc.Dimensions.x / oCastAsFloat(RTDesc.Dimensions.y), 0.001f, 1000.0f);

		float rotationStep = Device->GetFrameID() * 1.0f;
		float4x4 W = oCreateRotation(float3(radians(rotationStep) * 0.75f, radians(rotationStep), radians(rotationStep) * 0.5f));

		uint DrawID = 0;

		if (!Device->BeginFrame())
			return;
		CL->Begin();

		oGPUCommitBuffer(CL, ViewConstants, oGPUViewConstants(V, P, RTDesc.Dimensions, 0));
		oGPUCommitBuffer(CL, DrawConstants, oGPUDrawConstants(W, V, P, 0, DrawID++));

		CL->Clear(_pPrimaryRenderTarget, oGPU_CLEAR_COLOR_DEPTH_STENCIL);
		CL->SetBlendState(oGPU_OPAQUE);
		CL->SetDepthStencilState(oGPU_DEPTH_TEST_AND_WRITE);
		CL->SetSurfaceState(oGPU_FRONT_FACE);
		CL->SetBuffers(0, 2, &ViewConstants); // let the set run from ViewConstants to DrawConstants
		oGPU_SAMPLER_STATE s = oGPU_LINEAR_WRAP;
		CL->SetSamplers(0, 1, &s);
		CL->SetShaderResources(0, 1, &Texture);
		CL->SetPipeline(Pipeline);
		CL->SetRenderTarget(_pPrimaryRenderTarget);
		CL->Draw(Mesh, 0);

		CL->End();
		Device->EndFrame();
	}

	RESULT Run(char* _StrStatus, size_t _SizeofStrStatus) override
	{
		Once = false;

		static const int sSnapshotFrames[] = { 0, 2 };
		static const bool kIsDevMode = false;
		oGPU_TEST_WINDOW_INIT Init(kIsDevMode, oBIND(&TESTGPUTexture3DMip::Render, this, oBIND1), "TESTGPUTexture", sSnapshotFrames);

		oStd::future<oRef<oImage>> Snapshots[oCOUNTOF(sSnapshotFrames)];
		oRef<threadsafe oGPUWindow> Window;
		oTESTB0(oGPUTestCreateWindow(Init, oBIND(&TESTGPUTexture3DMip::CreateResources, this, oBIND1), Snapshots, &Window));

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

oTEST_REGISTER(TESTGPUTexture3DMip);