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
// PickBuffer
#pragma once
#ifndef oGfxPickBuffer_h
#define oGfxPickBuffer_h

#include <oGPU/oGPU.h>

static const uint oGPU_MAX_NUM_PICKS_PER_FRAME = 16;

// {B83197F4-6064-4098-8EDA-042D554AD146}
oDEFINE_GUID_S(oGfxPickBuffer, 0xb83197f4, 0x6064, 0x4098, 0x8e, 0xda, 0x4, 0x2d, 0x55, 0x4a, 0xd1, 0x46);
struct oGfxPickBuffer : oInterface
{
public:
	oDEFINE_REFCOUNT_INTERFACE(RefCount);
	oDEFINE_TRIVIAL_QUERYINTERFACE(oGfxPickBuffer);

	oGfxPickBuffer(ouro::gpu::device* _pDevice, const void* _pComputeShader, bool* bSuccess);

	void PIMap(ouro::gpu::command_list* _pCommandList, int2** _Picks);
	void PIUnmap(ouro::gpu::command_list* _pCommandList);
	void PDraw(ouro::gpu::command_list* _pCommandList, ouro::gpu::texture* _pPickRenderTargetTexture);
	//Map outputs from the pick shader. This must be an immediate context. And if a deferred context was
	//	used for any other functions in this class, those must be submitted before mapping the outputs.
	void POMap(uint** _Picks);
	void POUnmap();

private:
	oRefCount RefCount;
	int2 PicksInputBuffer[oGPU_MAX_NUM_PICKS_PER_FRAME];
	std::shared_ptr<ouro::gpu::texture> PicksInput;
	std::shared_ptr<ouro::gpu::buffer> PicksOutput;
	std::shared_ptr<ouro::gpu::buffer> PicksStaging;
 	std::shared_ptr<ouro::gpu::compute_kernel> PickResourceShader;
};

oAPI bool oGfxPickBufferCreate(ouro::gpu::device* _pDevice, const void* _pComputeShader, oGfxPickBuffer** _ppPickBuffer);

#endif // oGfxPickBuffer_h
