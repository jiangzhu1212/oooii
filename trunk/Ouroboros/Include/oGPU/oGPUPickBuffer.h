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
// PickBuffer
#pragma once
#ifndef oGPUPickBuffer_h
#define oGPUPickBuffer_h

#include <oGPU/oGPU.h>
#include <oGPU/oGPUConstants.h>

struct oGPUPickBuffer : oInterface
{
public:
	oDEFINE_REFCOUNT_INTERFACE(RefCount);
	oDEFINE_TRIVIAL_QUERYINTERFACE(oGPUPickBuffer);

	oGPUPickBuffer(oGPUDevice* _pDevice, const void* _pComputeShader, bool* bSuccess);

	void PIMap(oGPUCommandList* _pCommandList, int2** _Picks);
	void PIUnmap(oGPUCommandList* _pCommandList);
	void PDraw(oGPUCommandList* _pCommandList, oGPUTexture* _pPickRenderTargetTexture);
	//Map outputs from the pick shader. This must be an immediate context. And if a deferred context was
	//	used for any other functions in this class, those must be submitted before mapping the outputs.
	void POMap(uint** _Picks);
	void POUnmap();

private:
	oRefCount RefCount;
	int2 PicksInputBuffer[oGPU_MAX_NUM_PICKS_PER_FRAME];
	oRef<oGPUTexture> PicksInput;
	oRef<oGPUBuffer> PicksOutput;
	oRef<oGPUBuffer> PicksStaging;
 	oRef<oGPUComputeShader> PickResourceShader;
};

oAPI bool oGPUPickBufferCreate(oGPUDevice* _pDevice, const void* _pComputeShader, oGPUPickBuffer** _ppPickBuffer);

#endif // oGPUPickBuffer_h
