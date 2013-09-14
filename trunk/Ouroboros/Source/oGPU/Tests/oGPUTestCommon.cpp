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
#include "oGPUTestCommon.h"
#include <oGPU/oGPUUtil.h>
#include <oGPUTestColorPSByteCode.h>

bool oGPUTestApp::Create(const char* _Title, bool _DevMode, const int* _pSnapshotFrameIDs, size_t _NumSnapshotFrameIDs, const int2& _Size)
{
	NthSnapshot = 0;
	Running = true;
	DevMode = _DevMode;
	AllSnapshotsSucceeded = true;

	if (_pSnapshotFrameIDs && _NumSnapshotFrameIDs)
		SnapshotFrames.assign(_pSnapshotFrameIDs, _pSnapshotFrameIDs + _NumSnapshotFrameIDs);

	{
		oGPU_DEVICE_INIT init;
		init.DebugName = "oGPUTestApp.Device";
		init.Version = oStd::version(10,0); // for broader compatibility
		init.DriverDebugLevel = oGPU_DEBUG_NORMAL;
		if (!oGPUDeviceCreate(init, &Device))
			return false; // pass through error
	}

	{
		oWINDOW_INIT init;
		init.Title = _Title;
		init.AltF4Closes = true;
		init.EventHook = oBIND(&oGPUTestApp::OnEvent, this, oBIND1);
		init.Shape.State = DevMode ? oGUI_WINDOW_RESTORED : oGUI_WINDOW_HIDDEN;
		init.Shape.Style = oGUI_WINDOW_SIZABLE;
		init.Shape.ClientSize = _Size;
		if (!oWindowCreate(init, &Window))
			oThrowLastError();
	}

	if (!Device->CreatePrimaryRenderTarget(Window, oSURFACE_D24_UNORM_S8_UINT, true, &PrimaryRenderTarget))
		return false; // pass through error

	Device->GetImmediateCommandList(&CommandList);
	return true;
}

void oGPUTestApp::OnEvent(const oGUI_EVENT_DESC& _Event)
{
	switch (_Event.Type)
	{
		case oGUI_CLOSING:
			Running = false;
			break;
		default:
			break;
	}
}

bool oGPUTestApp::CheckSnapshot(oTest* _pTest)
{
	const int FrameID = Device->GetFrameID();
	if (SnapshotFrames.end() != oStd::find(SnapshotFrames, FrameID))
	{
		oStd::intrusive_ptr<oImage> Image;
		if (!PrimaryRenderTarget->CreateSnapshot(0, &Image))
			return false; // pass through error

		if (!_pTest->TestImage(Image, NthSnapshot))
		{
			oTRACEA("%s: Image(%u) %s: %s", _pTest->GetName(), NthSnapshot, oErrorAsString(oErrorGetLast()), oErrorGetLastString());
			AllSnapshotsSucceeded = false;
		}

		NthSnapshot++;
	}
	return AllSnapshotsSucceeded;
}

bool oGPUTestApp::Run(oTest* _pTest)
{
	oASSERT(Window->IsWindowThread(), "Run must be called from same thread that created the window");
	bool AllFramesSucceeded = true;

	// Flush window init
	Window->FlushMessages();

	if (!Initialize())
		return false; // pass through error

	while (Running && DevMode)
	{
		Window->FlushMessages();
		if (!Device->BeginFrame())
			return false; // pass through error

		if (!Render())
			return false; // pass through error

		Device->EndFrame();

		CheckSnapshot(_pTest);

		Device->Present(1);
	}

	if (!AllFramesSucceeded)
		return oErrorSetLast(std::errc::protocol_error, "Image compares failed, see debug output/log for specifics.");

	return true;
}

const int oGPUTextureTestApp::sSnapshotFrames[2] = { 0, 2 };

bool oGPUTextureTestApp::Initialize()
{
	PrimaryRenderTarget->SetClearColor(oStd::AlmostBlack);

	oGPUBuffer::DESC DCDesc;
	DCDesc.StructByteSize = sizeof(oGPUTestConstants);
	if (!Device->CreateBuffer("TestConstants", DCDesc, &TestConstants))
		return false;

	oGPUPipeline::DESC pld;
	if (!oGPUTestGetPipeline(GetPipeline(), &pld))
		return false;

	if (!Device->CreatePipeline(pld.DebugName, pld, &Pipeline))
		return false;

	if (!oGPUUtilCreateFirstCube(Device, pld.pElements, pld.NumElements, &Mesh))
		return false;

	return CreateTexture();
}

float oGPUTextureTestApp::GetRotationStep()
{
	// this is -1 because there was a code change that resulted in BeginFrame()
	// being moved out of the Render function below so it updated the FrameID
	// earlier than this code was ready for. If golden images are updated, this
	// could go away.
	return (Device->GetFrameID()-1) * 1.0f;
}

bool oGPUTextureTestApp::Render()
{
	float4x4 V = oCreateLookAtLH(float3(0.0f, 0.0f, -4.5f), oZERO3, float3(0.0f, 1.0f, 0.0f));

	oGPURenderTarget::DESC RTDesc;
	PrimaryRenderTarget->GetDesc(&RTDesc);
	float4x4 P = oCreatePerspectiveLH(oDEFAULT_FOVY_RADIANS, RTDesc.Dimensions.x / oCastAsFloat(RTDesc.Dimensions.y), 0.001f, 1000.0f);

	float rotationStep = GetRotationStep();
	float4x4 W = oCreateRotation(float3(radians(rotationStep) * 0.75f, radians(rotationStep), radians(rotationStep) * 0.5f));

	CommandList->Begin();

	oGPUCommitBuffer(CommandList, TestConstants, oGPUTestConstants(W, V, P, oStd::White));

	CommandList->Clear(PrimaryRenderTarget, oGPU_CLEAR_COLOR_DEPTH_STENCIL);
	CommandList->SetBlendState(oGPU_OPAQUE);
	CommandList->SetDepthStencilState(oGPU_DEPTH_TEST_AND_WRITE);
	CommandList->SetSurfaceState(oGPU_FRONT_FACE);
	CommandList->SetBuffers(0, 1, &TestConstants);
	oGPU_SAMPLER_STATE s = oGPU_LINEAR_WRAP;
	CommandList->SetSamplers(0, 1, &s);
	CommandList->SetShaderResources(0, 1, &Texture);
	CommandList->SetPipeline(Pipeline);
	CommandList->SetRenderTarget(PrimaryRenderTarget);
	oGPUUtilMeshDraw(CommandList, Mesh);

	CommandList->End();
	return true;
}
