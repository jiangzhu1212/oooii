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
#include <oGPU/raw_buffer.h>
#include <oCore/windows/win_error.h>
#include <oCore/windows/win_util.h>
#include "d3d_debug.h"
#include "d3d_util.h"
#include "dxgi_util.h"

using namespace ouro::gpu::d3d;

namespace ouro { namespace gpu {

Device* get_device(device& dev);
DeviceContext* get_dc(command_list& cl);

void raw_buffer::initialize(const char* name, device& dev, uint num_uints, const uint* src)
{
	deinitialize();
	Device* D3DDevice = get_device(dev);
	intrusive_ptr<Buffer> b = make_buffer(name, D3DDevice, sizeof(uint), num_uints, D3D11_USAGE_DEFAULT, D3D11_BIND_UNORDERED_ACCESS, D3D11_RESOURCE_MISC_DRAWINDIRECT_ARGS|D3D11_RESOURCE_MISC_BUFFER_ALLOW_RAW_VIEWS, src);
	debug_name(b, name);

	D3D11_UNORDERED_ACCESS_VIEW_DESC vd;
	vd.Format = DXGI_FORMAT_R32_TYPELESS;
	vd.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
	vd.Buffer.FirstElement = 0;
	vd.Buffer.NumElements = num_uints;
	vd.Buffer.Flags = D3D11_BUFFER_UAV_FLAG_RAW;

	oV(D3DDevice->CreateUnorderedAccessView(b, &vd, (UnorderedAccessView**)&rw));
	debug_name((View*)rw, name);
}

uint raw_buffer::num_uints() const
{
	D3D11_UNORDERED_ACCESS_VIEW_DESC vd;
	((UnorderedAccessView*)rw)->GetDesc(&vd);
	return vd.Buffer.NumElements;
}

void raw_buffer::update(command_list& cl, uint offset_in_uints, uint num_uints, const void* src)
{
	update_buffer(get_dc(cl), (View*)rw, offset_in_uints * sizeof(uint), num_uints * sizeof(uint), src);
}

}}