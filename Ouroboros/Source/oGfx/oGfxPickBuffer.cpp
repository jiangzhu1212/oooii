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
#include <oGfx/oGfxPickBuffer.h>

bool oGfxPickBufferCreate(oGPUDevice* _pDevice, const void* _pComputeShader, oGfxPickBuffer** _ppPickBuffer)
{
	bool success = false;
	oCONSTRUCT(_ppPickBuffer, oGfxPickBuffer(_pDevice, _pComputeShader, &success));
	return success;
}

oGfxPickBuffer::oGfxPickBuffer(oGPUDevice* _pDevice, const void* _pComputeShader, bool* bSuccess)
{
	oGPUTexture::DESC d;
	d.Dimensions = int3(oGPU_MAX_NUM_PICKS_PER_FRAME, 1, 1);
	d.ArraySize = 1;
	d.Format = oSURFACE_R32G32_SINT;
	d.Type = oGPU_TEXTURE_2D_MAP;
	oVERIFY(_pDevice->CreateTexture("oGfxPickBuffer.PicksInput", d, &PicksInput));

	oGPUBuffer::DESC BufferDesc;
	BufferDesc.Type = oGPU_BUFFER_UNORDERED_STRUCTURED;
	BufferDesc.StructByteSize = sizeof(uint);
	BufferDesc.ArraySize = oGPU_MAX_NUM_PICKS_PER_FRAME;
	oVERIFY(_pDevice->CreateBuffer("oGfxPickBuffer.PicksOutput", BufferDesc, &PicksOutput));

	BufferDesc.Type = oGPU_BUFFER_READBACK;
	oVERIFY(_pDevice->CreateBuffer("oGfxPickBuffer.PicksStaging", BufferDesc, &PicksStaging));

	oGPUComputeShader::DESC descComputeShader;
	descComputeShader.pComputeShader = _pComputeShader;
 	oVERIFY(_pDevice->CreateComputeShader("oGfxPickBuffer.PickResourceShader", descComputeShader, &PickResourceShader));

	*bSuccess = true;
}

void oGfxPickBuffer::PIMap(oGPUCommandList* _pCommandList, int2** _Picks)
{
	*_Picks = (int2*)PicksInputBuffer;
}

void oGfxPickBuffer::PIUnmap(oGPUCommandList* _pCommandList)
{
	oSURFACE_MAPPED_SUBRESOURCE mappedInput;
	mappedInput.pData = PicksInputBuffer;
	mappedInput.RowPitch = 0x80;
	mappedInput.DepthPitch = 0x80;
	_pCommandList->Commit(PicksInput, 0, mappedInput);
}

void oGfxPickBuffer::PDraw(oGPUCommandList* _pCommandList, oGPUTexture* _pPickRenderTargetTexture)
{
	_pCommandList->SetShaderResources(0, 1, &_pPickRenderTargetTexture);
	_pCommandList->SetShaderResources(1, 1, &PicksInput);

	// @oooii-jeffrey: Shouldn't this be for both textures?
	oGPU_SAMPLER_STATE state = oGPU_POINT_CLAMP;
	_pCommandList->SetSamplers(1, 1, &state);
	_pCommandList->SetUnorderedResources(0, 1, &PicksOutput);
	_pCommandList->Dispatch(PickResourceShader, int3(1, 1, 1));
	_pCommandList->Copy(PicksStaging, PicksOutput);
}

void oGfxPickBuffer::POMap(uint** _Picks) 
{
	oStd::ref<oGPUDevice> Device;
	PicksStaging->GetDevice(&Device);
	oSURFACE_MAPPED_SUBRESOURCE mappedStaging;
	// @oooii-jeffrey: This call is blocking/spin-locking because that was what the original D3D11 implementation did, this may still need some thought...
	oVERIFY(Device->MapRead(PicksStaging, 0, &mappedStaging, true));
	*_Picks = (uint*)mappedStaging.pData;
}

void oGfxPickBuffer::POUnmap() 
{
	oStd::ref<oGPUDevice> Device;
	PicksStaging->GetDevice(&Device);
	Device->UnmapRead(PicksStaging, 0);
}