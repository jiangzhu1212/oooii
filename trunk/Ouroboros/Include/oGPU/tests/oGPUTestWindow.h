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
#pragma once
#ifndef oGPUTestWindow_h
#define oGPUTestWindow_h

#include <oPlatform/oImage.h>
#include <oGPU/oGPU.h>
#include <oGPU/oGPUUtil.h>
#include <oGPU/oGPUWindow.h>

// Helper objects for running oGPU Based unit tests
struct oGPU_TEST_WINDOW_INIT
{
	oGPU_TEST_WINDOW_INIT(bool _DevMode, const oFUNCTION<void(oGPURenderTarget* _pPrimaryRenderTarget)>& _RenderFunction, const char* _Title, const int* _pSnapshotFrameIDs, size_t _NumSnapshotFrameIDs, const int2& _Size = int2(640, 480))
		: pSnapshotFrameIDs(_pSnapshotFrameIDs)
		, NumSnapshots(oInt(_NumSnapshotFrameIDs))
	{
		CTORCommon(_DevMode, _RenderFunction, _Title, _Size);
	}

	template<size_t size> oGPU_TEST_WINDOW_INIT(bool _DevMode, const oFUNCTION<void(oGPURenderTarget* _pPrimaryRenderTarget)>& _RenderFunction, const char* _Title, const int (&_pSnapshotFrameIDs)[size], const int2& _Size = int2(640, 480))
		: pSnapshotFrameIDs(_pSnapshotFrameIDs)
		, NumSnapshots(oInt(size))
	{
		CTORCommon(_DevMode, _RenderFunction, _Title, _Size);
	}

	oGPU_TEST_WINDOW_INIT(bool _DevMode, const oFUNCTION<void(oGPURenderTarget* _pPrimaryRenderTarget)>& _RenderFunction, const char* _Title = "oGPUTestWindow", const int2& _Size = int2(640, 480))
		: pSnapshotFrameIDs(nullptr)
		, NumSnapshots(0)
	{
		CTORCommon(_DevMode, _RenderFunction, _Title, _Size);
	}

	oGPUDevice::INIT DeviceInit;
	oGPU_WINDOW_INIT GPUWindowInit;

	const int* pSnapshotFrameIDs;
	int NumSnapshots;

private:
	void CTORCommon(bool _DevMode, const oFUNCTION<void(oGPURenderTarget* _pPrimaryRenderTarget)>& _RenderFunction, const char* _Title, const int2& _Size)
	{
		DeviceInit.DebugName = "oGPUTest.Device";
		DeviceInit.Version = oVersion(10,0); // a more compatible/widely testable feature level
		DeviceInit.DriverDebugLevel = oGPU_DEBUG_NORMAL;

		GPUWindowInit.WindowTitle = oSAFESTR(_Title);
		GPUWindowInit.WinDesc.AllowAltEnter = false;
		GPUWindowInit.WinDesc.DefaultEraseBackground = false;
		GPUWindowInit.WinDesc.ClientSize = _Size;
		GPUWindowInit.WinDesc.HasFocus = _DevMode;
		GPUWindowInit.WinDesc.State = _DevMode ? oGUI_WINDOW_RESTORED : oGUI_WINDOW_MINIMIZED;
		GPUWindowInit.RenderFunction = _RenderFunction;
		GPUWindowInit.DepthStencilFormat = oSURFACE_D24_UNORM_S8_UINT;
	}
};

bool oGPUTestCreateWindow(const oGPU_TEST_WINDOW_INIT& _Init, const oFUNCTION<bool(threadsafe oGPUWindow* _pWindow)>& _PrerenderInit, oStd::future<oRef<oImage>>* _pFutureSnapshots, size_t _NumFutureSnapshots, threadsafe oGPUWindow** _ppWindow);
template<size_t size> bool oGPUTestCreateWindow(const oGPU_TEST_WINDOW_INIT& _Init, const oFUNCTION<bool(threadsafe oGPUWindow* _pWindow)>& _PrerenderInit, oStd::future<oRef<oImage>> (&_pFutureSnapshots)[size], threadsafe oGPUWindow** _ppWindow) { return oGPUTestCreateWindow(_Init, _PrerenderInit, _pFutureSnapshots, size, _ppWindow); }

inline bool oGPUTestPrerenderInitNoop(threadsafe oGPUWindow* _pWindow) { return true; }

bool oGPUTestSnapshotsAreReady(oStd::future<oRef<oImage>>* _pFutureSnapshots, size_t _NumSnapshots);
template<size_t size> bool oGPUTestSnapshotsAreReady(oStd::future<oRef<oImage>> (&_pFutureSnapshots)[size]) { return oGPUTestSnapshotsAreReady(_pFutureSnapshots, size); }

bool oGPUTestSnapshots(oTest* _pTest, oStd::future<oRef<oImage>>* _pFutureSnapshots, size_t _NumSnapshots);
template<size_t size> bool oGPUTestSnapshots(oTest* _pTest, oStd::future<oRef<oImage>> (&_pFutureSnapshots)[size]) { return oGPUTestSnapshots(_pTest, _pFutureSnapshots, size); }

bool oGPUTestInitFirstTriangle(oGPUDevice* _pDevice, const char* _Name, const oGPU_VERTEX_ELEMENT* _pElements, uint _NumElements, oGPUUtilMesh** _ppTri);

bool oGPUTestInitCube(oGPUDevice* _pDevice, const char* _Name, const oGPU_VERTEX_ELEMENT* _pElements, uint _NumElements, oGPUUtilMesh** _ppMesh);

#endif //oGPUTestWindow_h