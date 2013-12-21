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
#include "oD3D11Buffer.h"
#include "oD3D11Device.h"

using namespace ouro::d3d11;

oDEFINE_GPUDEVICE_CREATE(oD3D11, Buffer);
oBEGIN_DEFINE_GPURESOURCE_CTOR(oD3D11, Buffer)
{
	oD3D11DEVICE();
	ID3D11UnorderedAccessView** ppUAV = _Desc.type == ouro::gpu::buffer_type::unordered_structured_append ? &UAVAppend : &UAV;
	Buffer = make_buffer(D3DDevice, _Name, _Desc, nullptr, ppUAV, &SRV);
	Desc = get_info(Buffer);
	if (_Desc.type == ouro::gpu::buffer_type::unordered_structured_append)
		UAV = make_unflagged_copy(*ppUAV);
	*_pSuccess = true;
}

int2 oD3D11Buffer::GetByteDimensions(int _Subresource) const threadsafe
{
	return int2(Desc.struct_byte_size, Desc.array_size);
}
