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
#include "d3d_resource.h"

oGPU_NAMESPACE_BEGIN

oDEFINE_DEVICE_MAKE(buffer)
oRESOURCE_CTOR(buffer)
{
	oD3D11_DEVICE();
	ID3D11UnorderedAccessView** ppUAV = Info.type == buffer_type::unordered_structured_append ? &UAVAppend : &UAV;
	Buffer = d3d::make_buffer(D3DDevice, _Name, Info, nullptr, ppUAV, &SRV);
	Info = d3d::get_info(Buffer);
	if (Info.type == buffer_type::unordered_structured_append)
		UAV = d3d::make_unflagged_copy(*ppUAV);
}

uint2 d3d11_buffer::byte_dimensions(int _Subresource) const
{
	return uint2(Info.struct_byte_size, Info.array_size);
}

oGPU_NAMESPACE_END
