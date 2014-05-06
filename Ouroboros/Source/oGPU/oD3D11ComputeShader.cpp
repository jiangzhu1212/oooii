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
#include "oD3D11ComputeShader.h"
#include "oD3D11Device.h"

oGPU_NAMESPACE_BEGIN

oDEFINE_DEVICE_MAKE(compute_kernel)
oDEVICE_CHILD_CTOR(compute_kernel)
	, DebugName(_Info.debug_name)
{
	if (!_Info.cs)
		oTHROW_INVARG("A buffer of valid compute shader bytecode must be specified");
	oD3D11_DEVICE();
	ComputeShader = (ID3D11ComputeShader*)make_compute_shader(_Device.get(), _Info.cs, _Info.debug_name);
}

compute_kernel_info d3d11_compute_kernel::get_info() const
{
	compute_kernel_info i;
	i.debug_name = DebugName;
	i.cs = nullptr;
	return i;
}

oGPU_NAMESPACE_END