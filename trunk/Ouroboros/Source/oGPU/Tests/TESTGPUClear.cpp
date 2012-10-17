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

#include <oPlatform/Windows/oDXGI.h>
#include <oPlatform/Windows/oGDI.h>
#include <oPlatform/Windows/oWinRect.h>

struct TESTGPUClear : public oTest
{
	void Render(oGPURenderTarget* _pPrimaryRenderTarget)
	{
		oRef<oGPUDevice> Device;
		_pPrimaryRenderTarget->GetDevice(&Device);

		static oColor sClearColors[] = { std::OOOiiGreen, std::White };

		oRef<oGPUCommandList> ICL;
		Device->GetImmediateCommandList(&ICL);

		if (!Device->BeginFrame())
			return;

		oGPU_CLEAR_DESC CD;
		CD.ClearColor[0] = sClearColors[Device->GetFrameID() % oCOUNTOF(sClearColors)];
		_pPrimaryRenderTarget->SetClearDesc(CD);

		ICL->Clear(_pPrimaryRenderTarget, oGPU_CLEAR_COLOR_DEPTH_STENCIL);
		Device->EndFrame();
	}

	RESULT Run(char* _StrStatus, size_t _SizeofStrStatus) override
	{
		static const int sSnapshotFrames[] = { 0, 1 };
		static const bool kIsDevMode = false;
		oGPU_TEST_WINDOW_INIT Init(kIsDevMode, oBIND(&TESTGPUClear::Render, this, oBIND1), "TESTGPUClear", sSnapshotFrames);

		oStd::future<oRef<oImage>> Snapshots[oCOUNTOF(sSnapshotFrames)];
		oRef<threadsafe oGPUWindow> Window;
		oTESTB0(oGPUTestCreateWindow(Init, oGPUTestPrerenderInitNoop, Snapshots, &Window));

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

oTEST_REGISTER(TESTGPUClear);
