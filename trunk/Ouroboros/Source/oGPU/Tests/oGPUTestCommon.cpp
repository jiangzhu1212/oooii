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
#include <oCore/filesystem.h>

#include <oBasis/oMath.h>

using namespace ouro;

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
		init.Version = version(10,0); // for broader compatibility
		init.DriverDebugLevel = oGPU_DEBUG_NORMAL;
		if (!oGPUDeviceCreate(init, &Device))
			return false; // pass through error
	}

	{
		ouro::window::init i;
		i.title = _Title;
		i.alt_f4_closes = true;
		i.event_hook = std::bind(&oGPUTestApp::OnEvent, this, std::placeholders::_1);
		i.shape.state = DevMode ? ouro::window_state::restored : ouro::window_state::hidden;
		i.shape.style = ouro::window_style::sizable;
		i.shape.client_size = _Size;
		Window = ouro::window::make(i);
	}

	if (!Device->CreatePrimaryRenderTarget(Window.get(), surface::d24_unorm_s8_uint, true, &PrimaryRenderTarget))
		return false; // pass through error

	Device->GetImmediateCommandList(&CommandList);
	return true;
}

void oGPUTestApp::OnEvent(const oGUI_EVENT_DESC& _Event)
{
	switch (_Event.Type)
	{
		case ouro::gui_event::closing:
			Running = false;
			break;
		default:
			break;
	}
}

bool oGPUTestApp::CheckSnapshot(oTest* _pTest)
{
	const int FrameID = Device->GetFrameID();
	if (SnapshotFrames.end() != find(SnapshotFrames, FrameID))
	{
		std::shared_ptr<surface::buffer> snap = PrimaryRenderTarget->CreateSnapshot(0);
		if (!_pTest->TestImage(snap, NthSnapshot))
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
	oASSERT(Window->is_window_thread(), "Run must be called from same thread that created the window");
	bool AllFramesSucceeded = true;

	// Flush window init
	Window->flush_messages();

	if (!Initialize())
		return false; // pass through error

	while (Running && DevMode)
	{
		Window->flush_messages();
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
	PrimaryRenderTarget->SetClearColor(AlmostBlack);

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
	float4x4 V = make_lookat_lh(float3(0.0f, 0.0f, -4.5f), oZERO3, float3(0.0f, 1.0f, 0.0f));

	oGPURenderTarget::DESC RTDesc;
	PrimaryRenderTarget->GetDesc(&RTDesc);
	float4x4 P = make_perspective_lh(oDEFAULT_FOVY_RADIANS, RTDesc.Dimensions.x / oCastAsFloat(RTDesc.Dimensions.y), 0.001f, 1000.0f);

	float rotationStep = GetRotationStep();
	float4x4 W = make_rotation(float3(radians(rotationStep) * 0.75f, radians(rotationStep), radians(rotationStep) * 0.5f));

	CommandList->Begin();

	oGPUCommitBuffer(CommandList, TestConstants, oGPUTestConstants(W, V, P, White));

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

std::shared_ptr<surface::buffer> surface_load(const path& _Path, surface::alpha_option::value _Option)
{
	size_t size = 0;
	auto b = filesystem::load(_Path, filesystem::load_option::binary_read, &size);
	return surface::decode(b.get(), size, _Option);
}

std::shared_ptr<ouro::surface::buffer> make_1D(int _Width)
{
	surface::info si;
	si.dimensions = int3(_Width, 1, 1);
	si.format = surface::b8g8r8a8_unorm;
	auto s = surface::buffer::make(si);

	{
		surface::lock_guard lock(s);
		static const color sConsoleColors[] = { Black, Navy, Green, Teal, Maroon, Purple, Olive, Silver, Gray, Blue, Lime, Aqua, Red, Fuchsia, Yellow, White };
		color* texture1Ddata = (color*)lock.mapped.data;
		for (int i = 0; i < si.dimensions.x; i++)
			texture1Ddata[i] = sConsoleColors[i % oCOUNTOF(sConsoleColors)];
	}

	return s;
}