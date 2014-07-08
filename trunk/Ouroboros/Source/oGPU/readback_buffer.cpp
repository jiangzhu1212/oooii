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
#include <oGPU/readback_buffer.h>
#include <oCore/windows/win_error.h>
#include <oCore/windows/win_util.h>
#include "d3d_debug.h"
#include "d3d_util.h"
#include "dxgi_util.h"

using namespace ouro::gpu::d3d;

namespace ouro { namespace gpu {

Device* get_device(device* dev);
DeviceContext* get_dc(command_list* cl);

void readback_buffer::initialize(const char* name, device* dev, uint element_stride, uint num_elements)
{
	bytes = element_stride * num_elements;
	auto b = make_buffer(name, get_device(dev), element_stride, num_elements, D3D11_USAGE_STAGING, 0, 0, nullptr);
	b->AddRef();
	impl = b;
}

void readback_buffer::internal_initialize(void* buffer_impl, bool make_immediate_copy)
{
	d3d::make_cpu_copy((d3d::Buffer*)buffer_impl, make_immediate_copy);
}

void readback_buffer::deinitialize()
{
	oSAFE_RELEASEV(impl);
}

void readback_buffer::internal_copy_from(command_list* cl, void* buffer_impl)
{
	get_dc(cl)->CopyResource((Resource*)impl, (Resource*)buffer_impl);
}

bool readback_buffer::copy_to(void* dst, size_t dst_size, bool blocking) const
{
	D3D11_MAPPED_SUBRESOURCE mapped;
	Resource* r = (Resource*)impl;
	intrusive_ptr<Device> dev;
	r->GetDevice(&dev);
	intrusive_ptr<DeviceContext> dc;
	dev->GetImmediateContext(&dc);
	HRESULT hr = dc->Map(r, 0, D3D11_MAP_READ, blocking ? 0 : D3D11_MAP_FLAG_DO_NOT_WAIT, &mapped);

	if (hr == DXGI_ERROR_WAS_STILL_DRAWING && !blocking)
		return false;

	if (hr == S_OK && !mapped.pData)
		hr = ERROR_NOT_ENOUGH_MEMORY;

	oV(hr);

	memcpy(dst, mapped.pData, dst_size);

	dc->Unmap(r, 0);
	return true;
}

}}
