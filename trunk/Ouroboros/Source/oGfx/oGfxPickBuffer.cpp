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

#if 0

using namespace ouro;
using namespace ouro::gpu;

bool oGfxPickBufferCreate(device* _pDevice, const void* _pComputeShader, oGfxPickBuffer** _ppPickBuffer)
{
	bool success = false;
	oCONSTRUCT(_ppPickBuffer, oGfxPickBuffer(_pDevice, _pComputeShader, &success));
	return success;
}

oGfxPickBuffer::oGfxPickBuffer(device* _pDevice, const void* _pComputeShader, bool* bSuccess)
{
	texture_info d;
	d.dimensions = ushort3(oGPU_MAX_NUM_PICKS_PER_FRAME, 1, 1);
	d.array_size = 1;
	d.format = ouro::surface::r32g32_sint;
	d.type = texture_type::default_2d;
	PicksInput = _pDevice->make_texture("oGfxPickBuffer.PicksInput", d);

	buffer_info BufferDesc;
	BufferDesc.type = gpu::buffer_type::unordered_structured;
	BufferDesc.struct_byte_size = sizeof(uint);
	BufferDesc.array_size = oGPU_MAX_NUM_PICKS_PER_FRAME;
	PicksOutput = _pDevice->make_buffer("oGfxPickBuffer.PicksOutput", BufferDesc);

	BufferDesc.type = gpu::buffer_type::readback;
	PicksStaging = _pDevice->make_buffer("oGfxPickBuffer.PicksStaging", BufferDesc);

	compute_kernel_info descComputeShader;
	descComputeShader.cs = _pComputeShader;
 	PickResourceShader = _pDevice->make_compute_kernel("oGfxPickBuffer.PickResourceShader", descComputeShader);

	*bSuccess = true;
}

void oGfxPickBuffer::PIMap(command_list* _pCommandList, int2** _Picks)
{
	*_Picks = (int2*)PicksInputBuffer;
}

void oGfxPickBuffer::PIUnmap(command_list* _pCommandList)
{
	ouro::surface::mapped_subresource mappedInput;
	mappedInput.data = PicksInputBuffer;
	mappedInput.row_pitch = 0x80;
	mappedInput.depth_pitch = 0x80;
	_pCommandList->commit(PicksInput, 0, mappedInput);
}

void oGfxPickBuffer::PDraw(command_list* _pCommandList, texture* _pPickRenderTargetTexture)
{
	_pCommandList->set_shader_resource(0, _pPickRenderTargetTexture);
	_pCommandList->set_shader_resource(1, PicksInput);

	// @oooii-jeffrey: Shouldn't this be for both textures?
	sampler_state::value state = sampler_state::point_clamp;
	_pCommandList->set_samplers(1, 1, &state);
	_pCommandList->set_unordered_resource(0, PicksOutput);
	_pCommandList->dispatch(PickResourceShader, int3(1, 1, 1));
	_pCommandList->copy(PicksStaging, PicksOutput);
}

void oGfxPickBuffer::POMap(uint** _Picks) 
{
	std::shared_ptr<device> Device = PicksStaging->get_device();
	ouro::surface::mapped_subresource mappedStaging;
	// @oooii-jeffrey: This call is blocking/spin-locking because that was what the original D3D11 implementation did, this may still need some thought...
	oVERIFY(Device->map_read(PicksStaging, 0, &mappedStaging, true));
	*_Picks = (uint*)mappedStaging.data;
}

void oGfxPickBuffer::POUnmap() 
{
	std::shared_ptr<device> Device = PicksStaging->get_device();
	Device->unmap_read(PicksStaging, 0);
}
#endif