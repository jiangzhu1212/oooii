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
#pragma once
#ifndef oGPUTestCommon_h
#define oGPUTestCommon_h

#include <oPlatform/oTest.h>
#include <oStd/oStdFuture.h>
#include <oGPU/oGPUUtil.h>
#include "oGPUTestPipelines.h"
#include "oGPUTestHLSL.h"

#define oDEFINE_GPU_TEST(_TestName) \
	struct _TestName : public oTest \
	{ \
		RESULT Run(char* _StrStatus, size_t _SizeofStrStatus) override \
		{	oCONCAT(_TestName,_App) App; \
			oTESTB0(App.Run(this)); \
			return SUCCESS; \
		} \
	}; \
	oTEST_REGISTER(_TestName);

class oGPUTestApp
{
public:
	oGPUTestApp(const char* _Title, bool _DevMode, const int* _pSnapshotFrameIDs, size_t _NumSnapshotFrameIDs, const int2& _Size = int2(640, 480))
	{
		if (!Create(_Title, _DevMode, _pSnapshotFrameIDs, _NumSnapshotFrameIDs, _Size))
			oThrowLastError();
	}
	
	template<size_t size>
	oGPUTestApp(const char* _Title, bool _DevMode, const int (&_pSnapshotFrameIDs)[size], const int2& _Size = int2(640, 480))
	{
		if (!Create(_Title, _DevMode, _pSnapshotFrameIDs, size, _Size))
			oThrowLastError();
	}

	// Called once before rendering begins
	virtual bool Initialize() { return true; }

	// Infrastructure calls BeginFrane/EndFrame, the rest is up to this function
	virtual bool Render() = 0;

	bool Run(oTest* _pTest);

	threadsafe oWindow* GetWindow() { return Window; }
	oGPUDevice* GetDevice() { return Device; }
	oGPUCommandList* GetCommandList() { return CommandList; }
	oGPURenderTarget* GetPrimaryRenderTarget() { return PrimaryRenderTarget; }

private:
	void OnEvent(const oGUI_EVENT_DESC& _Event);
	bool Create(const char* _Title, bool _DevMode, const int* _pSnapshotFrameIDs, size_t _NumSnapshotFrameIDs, const int2& _Size);

	// Call this after each Device::EndFrame() and before Device::Present to 
	// check if the current frame has been registered for testing. If so do the
	// test, oTRACEA out any failure and flag ultimate failure for later. If an
	// early-out is desired, response to a false return value, otherwise this will 
	// have Run return false when it is finished.
	bool CheckSnapshot(oTest* _pTest);

	std::vector<int> SnapshotFrames;

	int NthSnapshot;
	bool Running;
	bool DevMode;
	bool AllSnapshotsSucceeded;

protected:
	oStd::ref<oWindow> Window;
	oStd::ref<oGPUDevice> Device;
	oStd::ref<oGPUCommandList> CommandList;
	oStd::ref<oGPURenderTarget> PrimaryRenderTarget;
};

class oGPUTextureTestApp : public oGPUTestApp
{
public:
	oGPUTextureTestApp(const char* _Title, bool _DevMode, const int2& _Size = int2(640, 480))
		: oGPUTestApp(_Title, _DevMode, sSnapshotFrames, oCOUNTOF(sSnapshotFrames), _Size)
	{}

	virtual oGPU_TEST_PIPELINE GetPipeline() = 0;
	virtual bool CreateTexture() = 0;
	virtual float GetRotationStep();

	bool Initialize() override;
	bool Render() override;

protected:
	oStd::ref<oGPUPipeline> Pipeline;
	oStd::ref<oGPUTexture> Texture;
	oStd::ref<oGPUUtilMesh> Mesh;
	oStd::ref<oGPUBuffer> TestConstants;

	static const int sSnapshotFrames[2];
};

#endif
