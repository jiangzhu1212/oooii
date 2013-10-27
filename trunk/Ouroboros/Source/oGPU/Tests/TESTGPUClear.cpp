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

#include <oPlatform/Windows/oDXGI.h>
#include <oGUI/Windows/oGDI.h>
#include <oGUI/Windows/oWinRect.h>

#include <oPlatform/oWindow.h>

using namespace ouro;

static const int sSnapshotFrames[] = { 0, 1 };
static const bool kIsDevMode = false;

class GPU_Clear_App : public oGPUTestApp
{
public:
	GPU_Clear_App() : oGPUTestApp("GPU_Clear", kIsDevMode, sSnapshotFrames) {}

	bool Render() override
	{
		static color sClearColors[] = { OOOiiGreen, White };
		PrimaryRenderTarget->SetClearColor(sClearColors[Device->GetFrameID() % oCOUNTOF(sClearColors)]);
		CommandList->Begin();
		CommandList->Clear(PrimaryRenderTarget, oGPU_CLEAR_COLOR_DEPTH_STENCIL);
		CommandList->End();
		return true;
	}
};

oDEFINE_GPU_TEST(GPU_Clear)