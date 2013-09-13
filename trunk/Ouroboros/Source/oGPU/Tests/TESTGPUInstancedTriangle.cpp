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

static const int sSnapshotFrames[] = { 1, 23, 46 };
static const bool kIsDevMode = false;

struct GPU_InstancedTriangle_App : public oGPUTestApp
{
	GPU_InstancedTriangle_App() : oGPUTestApp("GPU_InstancedTriangle", kIsDevMode, sSnapshotFrames) {}

	bool Initialize() override
	{
		PrimaryRenderTarget->SetClearColor(oStd::AlmostBlack);

		oGPUBuffer::DESC DCDesc;
		DCDesc.StructByteSize = sizeof(oGPUTestConstants);
		if (!Device->CreateBuffer("TestConstants", DCDesc, &TestConstants))
			return false;

		oGPUPipeline::DESC pld;
		if (!oGPUTestGetPipeline(oGPU_TEST_TRANSFORMED_WHITE_INSTANCED, &pld))
			return false;

		oGPU_BUFFER_DESC bd;
		bd.Type = oGPU_BUFFER_VERTEX;
		bd.ArraySize = 2;
		bd.StructByteSize = oGPUCalcVertexSize(pld.pElements, pld.NumElements, 1);

		if (!Device->CreateBuffer("InstanceList", bd, &InstanceList))
			return false;

		if (!Device->CreatePipeline(pld.DebugName, pld, &Pipeline))
			return false;

		// Create geometry off the non-instanced version, i.e. the one without
		// the instanced channel.

		// @oooii-tony: Maybe there should be a util that can recognize and skip
		// instanced elements?
		if (!oGPUTestGetPipeline(oGPU_TEST_TRANSFORMED_WHITE, &pld))
			return false;

		if (!oGPUUtilCreateFirstTriangle(Device, pld.pElements, pld.NumElements, &Mesh))
			return false;

		return true;
	}
	
	bool Render() override
	{
		float4x4 V = oCreateLookAtLH(float3(0.0f, 0.0f, -3.5f), oZERO3, float3(0.0f, 1.0f, 0.0f));

		oGPURenderTarget::DESC RTDesc;
		PrimaryRenderTarget->GetDesc(&RTDesc);
		float4x4 P = oCreatePerspectiveLH(oDEFAULT_FOVY_RADIANS, RTDesc.Dimensions.x / oCastAsFloat(RTDesc.Dimensions.y), 0.001f, 1000.0f);

		{
			oSURFACE_MAPPED_SUBRESOURCE msr;
			CommandList->Reserve(InstanceList, 0, &msr);
			oGPU_TEST_INSTANCE* pInstances = (oGPU_TEST_INSTANCE*)msr.pData;
			{
				pInstances[0].Translation = float3(-0.5f, 0.5f, 0.0f);
				pInstances[1].Translation = float3(0.5f, -0.5f, 0.0f);

				float rotationStep = Device->GetFrameID() * 1.0f;
				pInstances[0].Rotation = oCreateRotationQ(float3(radians(rotationStep) * 0.75f, radians(rotationStep), radians(rotationStep) * 0.5f));
				pInstances[1].Rotation = oCreateRotationQ(float3(radians(rotationStep) * 0.5f, radians(rotationStep), radians(rotationStep) * 0.75f));
			}
			CommandList->Commit(InstanceList, 0, msr);
		}

		CommandList->Begin();

		oGPUCommitBuffer(CommandList, TestConstants, oGPUTestConstants(oIDENTITY4x4, V, P, oStd::White));

		CommandList->Clear(PrimaryRenderTarget, oGPU_CLEAR_COLOR_DEPTH_STENCIL);
		CommandList->SetBlendState(oGPU_OPAQUE);
		CommandList->SetDepthStencilState(oGPU_DEPTH_TEST_AND_WRITE);
		CommandList->SetSurfaceState(oGPU_TWO_SIDED);
		CommandList->SetBuffers(0, 1, &TestConstants);
		CommandList->SetPipeline(Pipeline);
		CommandList->SetRenderTarget(PrimaryRenderTarget);
		
		const oGPUBuffer* pBuffers[2] = { Mesh->GetVertexBuffer(), InstanceList };
		CommandList->Draw(Mesh->GetIndexBuffer(), 0, 2, pBuffers, 0, 1, 0, 2);

		CommandList->End();
		return true;
	}

private:
	oStd::ref<oGPUPipeline> Pipeline;
	oStd::ref<oGPUBuffer> InstanceList;
	oStd::ref<oGPUUtilMesh> Mesh;
	oStd::ref<oGPUBuffer> TestConstants;
};

oDEFINE_GPU_TEST(GPU_InstancedTriangle)
