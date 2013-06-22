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
#ifndef oGPUTestCommon_h
#define oGPUTestCommon_h

#include <oPlatform/oTest.h>
#include <oStd/oStdFuture.h>
#include <oGPU/tests/oGPUTestWindow.h>
#include "oGPUTestPipelines.h"
#include "oGPUTestHLSL.h"


struct oGPUTextureTest : public oTest
{
	oRef<oGPUDevice> Device;
	oRef<oGPUCommandList> CL;
	oRef<oGPUPipeline> Pipeline;
	oRef<oGPUTexture> Texture;
	oRef<oGPUUtilMesh> Mesh;
	oRef<oGPUBuffer> TestConstants;
	bool Once;

	virtual enum oGPU_TEST_PIPELINE GetPipeline() = 0;
	virtual bool CreateTexture() = 0;
	bool CreateResources(threadsafe oGPUWindow* _pWindow);

	virtual float GetRotationStep();
	void Render(oGPURenderTarget* _pPrimaryRenderTarget);
	
	RESULT Run(char* _StrStatus, size_t _SizeofStrStatus) override;

protected:

	bool Finished;
};


#endif
