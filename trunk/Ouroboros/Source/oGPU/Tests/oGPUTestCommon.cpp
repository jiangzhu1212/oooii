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
	wd->EnableIdleEvent = true;
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
		return oErrorSetLast(oERROR_INVALID_PARAMETER, "_Init.NumSnapshots != _NumFutureSnapshots (%u != %u)", _Init.NumSnapshots, _NumFutureSnapshots);

	oRef<oGPUDevice> Device;
	if (!oGPUDeviceCreate(_Init.DeviceInit, &Device))
		return false; // pass through error

	oGPU_WINDOW_INIT GPUInit = _Init.GPUWindowInit;

	// Render happens on idle. To get a snapshot of the 0th frame and call init
	// code, we need to schedule the shots and init before rendering passes the 
	// 0th frame.
	GPUInit.WinDesc.EnableIdleEvent = false;

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
		if (!_pFutureSnapshots[i].get((oRef<oImage>*)&Image))
			return oErrorSetLast(_pFutureSnapshots[i].get_error(), _pFutureSnapshots[i].get_error_string());

		if (!_pTest->TestImage(Image, i))
		{
			oTRACEA("%s: Image(%u) %s: %s", _pTest->GetName(), i, oAsString(oErrorGetLast()), oErrorGetLastString());
			AllSucceeded = false;
		}
	}

	if (!AllSucceeded)
		oErrorSetLast(oERROR_GENERIC, "Image compares failed, see debug output/log for specifics.");

	return AllSucceeded;
}

bool oGPUTestInitFirstTriangle(oGPUDevice* _pDevice, const char* _Name, const oGPU_VERTEX_ELEMENT* _pElements, uint _NumElements, oGPUMesh** _ppTri)
{
	oGPUMesh::DESC md;
	md.NumIndices = 3;
	md.NumVertices = 3;
	md.NumRanges = 1;
	md.LocalSpaceBounds = oAABoxf(float3(-0.8f, -0.7f, -0.01f), float3(0.8f, 0.7f, 0.01f));
	md.NumVertexElements = _NumElements;
	memcpy(md.VertexElements, _pElements, sizeof(oGPU_VERTEX_ELEMENT) * _NumElements);

	oRef<oGPUMesh> Mesh;
	if (!_pDevice->CreateMesh(_Name, md, &Mesh))
		return false; // pass through error

	oRef<oGPUCommandList> ICL;
	_pDevice->GetImmediateCommandList(&ICL);

	oGPU_RANGE r;
	r.StartPrimitive = 0;
	r.NumPrimitives = 1;
	r.MinVertex = 0;
	r.MaxVertex = 3;

	oSURFACE_MAPPED_SUBRESOURCE msr;
	msr.pData = &r;
	msr.RowPitch = sizeof(r);
	msr.DepthPitch = sizeof(r);
	ICL->Commit(Mesh, oGPU_MESH_RANGES, msr);

	static const ushort Indices[] = { 0, 1, 2 };
	msr.pData = (void*)Indices;
	msr.RowPitch = sizeof(ushort);
	msr.DepthPitch = sizeof(Indices);
	ICL->Commit(Mesh, oGPU_MESH_INDICES, msr);

	static const float3 Positions[] = { float3(-0.75f, -0.667f, 0.0f), float3(0.0f, 0.667f, 0.0f), float3(0.75f, -0.667f, 0.0f) };
	msr.pData = (void*)Positions;
	msr.RowPitch = sizeof(float3);
	msr.DepthPitch = sizeof(Positions);
	ICL->Commit(Mesh, oGPU_MESH_VERTICES0, msr);

	Mesh->Reference();
	*_ppTri = Mesh;
	return true;
}

bool oGPUTestInitCube(oGPUDevice* _pDevice, const char* _Name, const oGPU_VERTEX_ELEMENT* _pElements, uint _NumElements, oGPUMesh** _ppMesh)
{
	oGeometry::LAYOUT layout;
	layout.Positions = true;
	layout.Texcoords = true;

	oGeometryFactory::BOX_DESC bd;
	bd.FaceType = oGeometry::FRONT_CCW;
	bd.Bounds = oAABoxf(float3(-1.0f), float3(1.0f));
	bd.Divide = 1;
	bd.Color = std::White;
	bd.FlipTexcoordV = false;

	oRef<oGeometryFactory> Factory;
	if (!oGeometryFactoryCreate(&Factory))
		return false;

	oRef<oGeometry> geo;
	if (!Factory->Create(bd, layout, &geo))
		return false;

	return oGPUCreateMesh(_pDevice, _Name, _pElements, _NumElements, nullptr, geo, _ppMesh);
}
