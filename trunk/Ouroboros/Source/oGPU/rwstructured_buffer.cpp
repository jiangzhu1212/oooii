/**************************************************************************
 * The MIT License                                                        *
 * Copyright (c) 2014 Antony Arciuolo.                                    *
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
#include <oGPU/rwstructured_buffer.h>
#include <oGPU/shader.h>
#include <oCore/windows/win_error.h>
#include <oCore/windows/win_util.h>
#include "d3d_debug.h"
#include "d3d_util.h"
#include "dxgi_util.h"

using namespace ouro::gpu::d3d;

namespace ouro { namespace gpu {

Device* get_device(device& dev);
DeviceContext* get_dc(command_list& cl);
compute_shader* get_noop_cs(device& dev);

void rwstructured_buffer::initialize(const char* name, device& dev, uint struct_stride, uint num_structs, const void* src)
{
	deinitialize();
	make_structured(name, get_device(dev), struct_stride, num_structs, src, 0, (ShaderResourceView**)&ro, (UnorderedAccessView**)&rw);
	noop = get_noop_cs(dev);
}

void rwstructured_buffer::initialize_append(const char* name, device& dev, uint struct_stride, uint num_structs, const void* src)
{
	deinitialize();
	make_structured(name, get_device(dev), struct_stride, num_structs, src, D3D11_BUFFER_UAV_FLAG_APPEND, (ShaderResourceView**)&ro, (UnorderedAccessView**)&rw);
}

void rwstructured_buffer::initialize_counter(const char* name, device& dev, uint struct_stride, uint num_structs, const void* src)
{
	deinitialize();
	make_structured(name, get_device(dev), struct_stride, num_structs, src, D3D11_BUFFER_UAV_FLAG_COUNTER, (ShaderResourceView**)&ro, (UnorderedAccessView**)&rw);
}

uint rwstructured_buffer::struct_stride() const
{
	intrusive_ptr<Buffer> b;
	((ShaderResourceView*)ro)->GetResource((Resource**)&b);
	D3D11_BUFFER_DESC d;
	b->GetDesc(&d);
	return d.StructureByteStride;
}

uint rwstructured_buffer::num_structs() const
{
	D3D11_SHADER_RESOURCE_VIEW_DESC vd;
	((ShaderResourceView*)ro)->GetDesc(&vd);
	return vd.Buffer.NumElements;
}

void rwstructured_buffer::set(command_list& cl, uint slot)
{
	set_srvs(get_dc(cl), slot, 1, (ShaderResourceView* const*)&ro);
}

void rwstructured_buffer::update(command_list& cl, uint struct_offset, uint num_structs, const void* src)
{
	const uint element_stride = struct_stride();
	update_buffer(get_dc(cl), (ShaderResourceView*)ro, struct_offset * element_stride, num_structs * element_stride, src);
}

void rwstructured_buffer::internal_copy_counter(command_list& cl, void* dst_buffer_impl, uint offset_in_uints)
{
	get_dc(cl)->CopyStructureCount((Buffer*)dst_buffer_impl, offset_in_uints * sizeof(uint), (UnorderedAccessView*)rw);
}

void rwstructured_buffer::set_counter(command_list& cl, uint value)
{
	DeviceContext* dc = get_dc(cl);

	// Executing a noop only seems to apply the initial counts if done through
	// OMSetRenderTargetsAndUnorderedAccessViews so set things up that way with
	// a false setting here, and then flush it with a dispatch of a noop.
	dc->CSSetUnorderedAccessViews(0, 0, nullptr, nullptr);
	dc->OMSetRenderTargetsAndUnorderedAccessViews(0, nullptr, nullptr, 0, 1, (UnorderedAccessView* const*)&rw, &value); // set up binding
	noop->dispatch(cl, uint3(1,1,1));
}

}}
