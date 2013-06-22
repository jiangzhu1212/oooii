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

struct GPU_RenderTarget : public oTest
{
	oRef<oGPUDevice> Device;
	oRef<oGPUCommandList> CLMainScene;
	oRef<oGPUCommandList> CLRenderTarget;
	oRef<oGPUPipeline> PLPassThrough;
	oRef<oGPUPipeline> PLTexture;
	oRef<oGPURenderTarget> RenderTarget;
	oRef<oGPUUtilMesh> Cube;
	oRef<oGPUUtilMesh> Triangle;
	oRef<oGPUBuffer> TestConstants;
	bool Once;

	bool CreateResources(threadsafe oGPUWindow* _pWindow)
	{
		_pWindow->GetDevice(&Device);
		oGPUCommandList::DESC cld;
		cld.DrawOrder = 1;

		if (!Device->CreateCommandList("CLMainScene", cld, &CLMainScene))
			return false;

		cld.DrawOrder = 0;
		if (!Device->CreateCommandList("CLRenderTarget", cld, &CLRenderTarget))
			return false;

		oGPUBuffer::DESC DCDesc;
		DCDesc.StructByteSize = sizeof(oGPUTestConstants);
		if (!Device->CreateBuffer("TestConstants", DCDesc, &TestConstants))
			return false;

		oGPUPipeline::DESC PassThroughDesc;
		if (!oGPUTestGetPipeline(oGPU_TEST_PASS_THROUGH, &PassThroughDesc))
			return false;

		if (!Device->CreatePipeline(PassThroughDesc.DebugName, PassThroughDesc, &PLPassThrough))
			return false;

		if (!oGPUTestInitFirstTriangle(Device, "Triangle", PassThroughDesc.pElements, PassThroughDesc.NumElements, &Triangle))
			return false;

		oGPUPipeline::DESC TextureDesc;
		if (!oGPUTestGetPipeline(oGPU_TEST_TEXTURE_2D, &TextureDesc))
			return false;

		if (!Device->CreatePipeline(TextureDesc.DebugName, TextureDesc, &PLTexture))
			return false;

		if (!oGPUTestInitCube(Device, "Cube", TextureDesc.pElements, TextureDesc.NumElements, &Cube))
			return false;

		oGPU_CLEAR_DESC cd;
		cd.ClearColor[0] = oStd::DeepSkyBlue;

		oGPURenderTarget::DESC rtd;
		rtd.Dimensions = int3(256, 256, 1);
		rtd.ArraySize = 1;
		rtd.MRTCount = 1;
		rtd.Format[0] = oSURFACE_B8G8R8A8_UNORM;
		rtd.DepthStencilFormat = oSURFACE_D24_UNORM_S8_UINT;
		rtd.ClearDesc = cd;
		if (!Device->CreateRenderTarget("RenderTarget", rtd, &RenderTarget))
			return false;

		return true;
	}

	void RenderToTarget(oGPUCommandList* _pCommandList, oGPURenderTarget* _pTarget)
	{
		_pCommandList->Begin();
		_pCommandList->Clear(_pTarget, oGPU_CLEAR_COLOR_DEPTH_STENCIL);
		_pCommandList->SetBlendState(oGPU_OPAQUE);
		_pCommandList->SetDepthStencilState(oGPU_DEPTH_STENCIL_NONE);
		_pCommandList->SetSurfaceState(oGPU_FRONT_FACE);
		_pCommandList->SetPipeline(PLPassThrough);
		_pCommandList->SetRenderTarget(_pTarget);
		oGPUUtilMeshDraw(_pCommandList, Triangle);
		_pCommandList->End();
	}

	void RenderMainScene(oGPUCommandList* _pCommandList, oGPUTexture* _pTexture, oGPURenderTarget* _pTarget)
	{
		float4x4 V = oCreateLookAtLH(float3(0.0f, 0.0f, -4.5f), oZERO3, float3(0.0f, 1.0f, 0.0f));

		oGPURenderTarget::DESC RTDesc;
		_pTarget->GetDesc(&RTDesc);
		float4x4 P = oCreatePerspectiveLH(oDEFAULT_FOVY_RADIANS, RTDesc.Dimensions.x / oCastAsFloat(RTDesc.Dimensions.y), 0.001f, 1000.0f);

		float rotationStep = Device->GetFrameID() * 1.0f;
		float4x4 W = oCreateRotation(float3(radians(rotationStep) * 0.75f, radians(rotationStep), radians(rotationStep) * 0.5f));

		uint DrawID = 0;

		_pCommandList->Begin();

		oGPUCommitBuffer(_pCommandList, TestConstants, oGPUTestConstants(W, V, P, oStd::White));

		_pCommandList->Clear(_pTarget, oGPU_CLEAR_COLOR_DEPTH_STENCIL);
		_pCommandList->SetBlendState(oGPU_OPAQUE);
		_pCommandList->SetDepthStencilState(oGPU_DEPTH_TEST_AND_WRITE);
		_pCommandList->SetSurfaceState(oGPU_FRONT_FACE);
		_pCommandList->SetBuffers(0, 1, &TestConstants);
		oGPU_SAMPLER_STATE s = oGPU_LINEAR_WRAP;
		_pCommandList->SetSamplers(0, 1, &s);
		_pCommandList->SetShaderResources(0, 1, &_pTexture);
		_pCommandList->SetPipeline(PLTexture);
		_pCommandList->SetRenderTarget(_pTarget);
		oGPUUtilMeshDraw(_pCommandList, Cube);

		_pCommandList->End();
	}
	
	void Render(oGPURenderTarget* _pPrimaryRenderTarget)
	{
		if (!Once)
		{
			oGPU_CLEAR_DESC CD;
			CD.ClearColor[0] = oStd::AlmostBlack;
			_pPrimaryRenderTarget->SetClearDesc(CD);

			Once = true;
		}

		float4x4 V = oCreateLookAtLH(float3(0.0f, 0.0f, -4.5f), oZERO3, float3(0.0f, 1.0f, 0.0f));

		oGPURenderTarget::DESC RTDesc;
		_pPrimaryRenderTarget->GetDesc(&RTDesc);
		float4x4 P = oCreatePerspectiveLH(oDEFAULT_FOVY_RADIANS, RTDesc.Dimensions.x / oCastAsFloat(RTDesc.Dimensions.y), 0.001f, 1000.0f);

		float rotationStep = Device->GetFrameID() * 1.0f;
		float4x4 W = oCreateRotation(float3(radians(rotationStep) * 0.75f, radians(rotationStep), radians(rotationStep) * 0.5f));

		uint DrawID = 0;

		// DrawOrder should be respected in out-of-order submits, so show that here
		// but executing on the main scene, THEN the render target, but because the
		// draw order of the command lists defines the render target before the 
		// main scene, this should come out as a cube with a triangle texture.

		oRef<oGPUTexture> Texture;
		RenderTarget->GetTexture(0, &Texture);

		RenderMainScene(CLMainScene, Texture, _pPrimaryRenderTarget);
		RenderToTarget(CLRenderTarget, RenderTarget);
	}

	RESULT Run(char* _StrStatus, size_t _SizeofStrStatus) override
	{
		Once = false;

		static const int sSnapshotFrames[] = { 0, 50 };
		static const bool kIsDevMode = false;
		oGPU_TEST_WINDOW_INIT Init(kIsDevMode, oBIND(&GPU_RenderTarget::Render, this, oBIND1), "GPU_RenderTarget", sSnapshotFrames);

		oStd::future<oRef<oImage>> Snapshots[oCOUNTOF(sSnapshotFrames)];
		oRef<threadsafe oGPUWindow> Window;
		oTESTB0(oGPUTestCreateWindow(Init, oBIND(&GPU_RenderTarget::CreateResources, this, oBIND1), Snapshots, &Window));

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

oTEST_REGISTER(GPU_RenderTarget);
