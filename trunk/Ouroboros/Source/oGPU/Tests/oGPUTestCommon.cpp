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
#include "oGPUTestCommon.h"
#include <oGPU/oGPUUtil.h>
#include <oGPUTestColorPSByteCode.h>

static bool oGPUTestEnableRender(threadsafe oGPUWindow* _pWindow)
{
	oGUI_WINDOW_DESC* wd = nullptr;
	if (!_pWindow->Map(&wd))
		return false;
	wd->EnableMainLoopEvent = true;
	_pWindow->Unmap();
	return true;
}

static bool oGPUTestScheduleSnapshots(threadsafe oWindow* _pWindow, const int* _pFrameIDs, oStd::future<oRef<oImage>>* _pFutureSnapshots, size_t _NumSnapshots)
{
	for (size_t i = 0; i < _NumSnapshots; i++)
	{
		oStd::future<oRef<oImage>> Snapshot = _pWindow->CreateSnapshot(_pFrameIDs[i]);
		_pFutureSnapshots[i] = std::move(Snapshot);
	}

	return true;
}

bool oGPUTestSnapshotsAreReady(oStd::future<oRef<oImage>>* _pFutureSnapshots, size_t _NumSnapshots)
{
	for (size_t i = 0; i < _NumSnapshots; i++)
		if (!_pFutureSnapshots[i].is_ready())
			return false;
	return true;
}

bool oGPUTestCreateWindow(const oGPU_TEST_WINDOW_INIT& _Init, const oFUNCTION<bool(threadsafe oGPUWindow* _pWindow)>& _PrerenderInit, oStd::future<oRef<oImage>>* _pFutureSnapshots, size_t _NumFutureSnapshots, threadsafe oGPUWindow** _ppWindow)
{
	if (_Init.NumSnapshots > 0 && static_cast<unsigned int>(_Init.NumSnapshots) != _NumFutureSnapshots)
		return oErrorSetLast(std::errc::invalid_argument, "_Init.NumSnapshots != _NumFutureSnapshots (%u != %u)", _Init.NumSnapshots, _NumFutureSnapshots);

	oRef<oGPUDevice> Device;
	if (!oGPUDeviceCreate(_Init.DeviceInit, &Device))
		return false; // pass through error

	oGPU_WINDOW_INIT GPUInit = _Init.GPUWindowInit;

	// Render happens on idle. To get a snapshot of the 0th frame and call init
	// code, we need to schedule the shots and init before rendering passes the 
	// 0th frame.
	GPUInit.WinDesc.EnableMainLoopEvent = false;

	oRef<threadsafe oGPUWindow> Window;
	if (!oGPUWindowCreate(GPUInit, Device, &Window))
		return false;

	oVERIFY(oGPUTestScheduleSnapshots(Window, _Init.pSnapshotFrameIDs, _pFutureSnapshots, _Init.NumSnapshots));

	if (_PrerenderInit && !_PrerenderInit(Window))
		return false;

	if (_Init.pSnapshotFrameIDs && _Init.NumSnapshots)
		oVERIFY(oGPUTestEnableRender(Window));

	Window->Reference();
	*_ppWindow = Window;
	return true;
}

bool oGPUTestSnapshots(oTest* _pTest, oStd::future<oRef<oImage>>* _pFutureSnapshots, size_t _NumSnapshots)
{
	bool AllSucceeded = true;
	for (unsigned int i = 0; i < _NumSnapshots; i++)
	{
		oRef<oImage> Image;

		try { Image = _pFutureSnapshots[i].get(); }
		catch (std::exception& e) { return oErrorSetLast(e); }

		if (!_pTest->TestImage(Image, i))
		{
			oTRACEA("%s: Image(%u) %s: %s", _pTest->GetName(), i, oErrorAsString(oErrorGetLast()), oErrorGetLastString());
			AllSucceeded = false;
		}
	}

	if (!AllSucceeded)
		oErrorSetLast(std::errc::protocol_error, "Image compares failed, see debug output/log for specifics.");

	return AllSucceeded;
}

bool oGPUTestInitFirstTriangle(oGPUDevice* _pDevice, const char* _Name, const oGPU_VERTEX_ELEMENT* _pElements, uint _NumElements, oGPUUtilMesh** _ppTri)
{
	oGPUUtilMesh::DESC md;
	md.NumIndices = 3;
	md.NumVertices = 3;
	md.NumRanges = 1;
	md.LocalSpaceBounds = oAABoxf(oAABoxf::min_max, float3(-0.8f, -0.7f, -0.01f), float3(0.8f, 0.7f, 0.01f));
	md.NumVertexElements = _NumElements;

	std::copy(_pElements, _pElements + _NumElements, md.VertexElements.begin());
	oRef<oGPUUtilMesh> Mesh;
	if (!oGPUUtilMeshCreate(_pDevice, _Name, md, 1, &Mesh))
		return false; // pass through error

	oRef<oGPUCommandList> ICL;
	_pDevice->GetImmediateCommandList(&ICL);

	oSURFACE_CONST_MAPPED_SUBRESOURCE msr;
	static const ushort Indices[] = { 0, 1, 2 };
	msr.pData = (void*)Indices;
	msr.RowPitch = sizeof(ushort);
	msr.DepthPitch = sizeof(Indices);
	ICL->Commit(Mesh->GetIndexBuffer(), 0, msr);

	static const float3 Positions[] = { float3(-0.75f, -0.667f, 0.0f), float3(0.0f, 0.667f, 0.0f), float3(0.75f, -0.667f, 0.0f) };
	msr.pData = (void*)Positions;
	msr.RowPitch = sizeof(float3);
	msr.DepthPitch = sizeof(Positions);
	ICL->Commit(Mesh->GetVertexBuffer(), 0, msr);

	Mesh->Reference();
	*_ppTri = Mesh;
	return true;
}

bool oGPUTestInitCube(oGPUDevice* _pDevice, const char* _Name, const oGPU_VERTEX_ELEMENT* _pElements, uint _NumElements, oGPUUtilMesh** _ppMesh)
{
	oGeometry::LAYOUT layout;
	layout.Positions = true;
	layout.Texcoords = true;

	oGeometryFactory::BOX_DESC bd;
	bd.FaceType = oGeometry::FRONT_CCW;
	bd.Bounds = oAABoxf(oAABoxf::min_max, float3(-1.0f), float3(1.0f));
	bd.Divide = 1;
	bd.Color = oStd::White;
	bd.FlipTexcoordV = false;

	oRef<oGeometryFactory> Factory;
	if (!oGeometryFactoryCreate(&Factory))
		return false;

	oRef<oGeometry> geo;
	if (!Factory->Create(bd, layout, &geo))
		return false;
	
	return oGPUUtilMeshCreate(_pDevice, _Name, _pElements, _NumElements, geo, _ppMesh);
}

oTest::RESULT oGPUTextureTest::Run(char* _StrStatus, size_t _SizeofStrStatus)
{
	Once = false;

	static const int sSnapshotFrames[] = { 0, 2 };
	static const bool kIsDevMode = false;
	oGPU_TEST_WINDOW_INIT Init(kIsDevMode, oBIND(&oGPUTextureTest::Render, this, oBIND1), "GPU_Texture", sSnapshotFrames);

	oStd::future<oRef<oImage>> Snapshots[oCOUNTOF(sSnapshotFrames)];
	oRef<threadsafe oGPUWindow> Window;
	oTESTB0(oGPUTestCreateWindow(Init, oBIND(&oGPUTextureTest::CreateResources, this, oBIND1), Snapshots, &Window));

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

bool oGPUTextureTest::CreateResources(threadsafe oGPUWindow* _pWindow)
{
	_pWindow->GetDevice(&Device);
	oGPUCommandList::DESC cld;
	cld.DrawOrder = 0;

	if (!Device->CreateCommandList("CommandList", cld, &CL))
		return false;

	oGPUBuffer::DESC DCDesc;
	DCDesc.StructByteSize = sizeof(oGPUTestConstants);
	if (!Device->CreateBuffer("TestConstants", DCDesc, &TestConstants))
		return false;

	oGPUPipeline::DESC pld;
	if (!oGPUTestGetPipeline(GetPipeline(), &pld))
		return false;

	if (!Device->CreatePipeline(pld.DebugName, pld, &Pipeline))
		return false;

	if (!oGPUTestInitCube(Device, "Cube", pld.pElements, pld.NumElements, &Mesh))
		return false;

	return CreateTexture();
}

float oGPUTextureTest::GetRotationStep()
{
	// this is -1 because there was a code change that resulted in BeginFrame()
	// being moved out of the Render function below so it updated the FrameID
	// earlier than this code was ready for. If golden images are updated, this
	// could go away.
	return (Device->GetFrameID()-1) * 1.0f;
}

void oGPUTextureTest::Render(oGPURenderTarget* _pPrimaryRenderTarget)
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

	float rotationStep = GetRotationStep();
	float4x4 W = oCreateRotation(float3(radians(rotationStep) * 0.75f, radians(rotationStep), radians(rotationStep) * 0.5f));

	uint DrawID = 0;

	CL->Begin();

	oGPUCommitBuffer(CL, TestConstants, oGPUTestConstants(W, V, P, oStd::White));

	CL->Clear(_pPrimaryRenderTarget, oGPU_CLEAR_COLOR_DEPTH_STENCIL);
	CL->SetBlendState(oGPU_OPAQUE);
	CL->SetDepthStencilState(oGPU_DEPTH_TEST_AND_WRITE);
	CL->SetSurfaceState(oGPU_FRONT_FACE);
	CL->SetBuffers(0, 1, &TestConstants);
	oGPU_SAMPLER_STATE s = oGPU_LINEAR_WRAP;
	CL->SetSamplers(0, 1, &s);
	CL->SetShaderResources(0, 1, &Texture);
	CL->SetPipeline(Pipeline);
	CL->SetRenderTarget(_pPrimaryRenderTarget);
	oGPUUtilMeshDraw(CL, Mesh);

	CL->End();
}
