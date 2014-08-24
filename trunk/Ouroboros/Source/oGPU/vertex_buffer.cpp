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
#include <oGPU/vertex_buffer.h>
#include <oCore/windows/win_error.h>
#include "d3d_debug.h"
#include "d3d_util.h"

using namespace ouro::gpu::d3d;

namespace ouro { namespace gpu {

Device* get_device(device& dev);
DeviceContext* get_dc(command_list& cl);

void vertex_buffer::initialize(const char* name, device& dev, uint vertex_stride, uint num_vertices, const void* vertices)
{
	deinitialize();
	auto b = make_buffer(name, get_device(dev), num_vertices, vertex_stride, D3D11_USAGE_DEFAULT, D3D11_BIND_VERTEX_BUFFER, 0, vertices);
	b->AddRef();
	impl = b;
	vstride = vertex_stride;
}

void vertex_buffer::deinitialize()
{
	oSAFE_RELEASEV(impl);
}

char* vertex_buffer::name(char* dst, size_t dst_size) const
{
	return impl ? debug_name(dst, dst_size, (Buffer*)impl) : "uninitialized";
}

uint vertex_buffer::num_vertices() const
{
	D3D11_BUFFER_DESC d;
	((Buffer*)impl)->GetDesc(&d);
	return d.ByteWidth / stride();
}

void vertex_buffer::set(command_list& cl, uint slot, uint byte_offset) const
{
	get_dc(cl)->IASetVertexBuffers(slot, 1, (Buffer**)&impl, &vstride, &byte_offset);
}

void vertex_buffer::set(command_list& cl, uint slot, uint num_buffers, vertex_buffer* const* buffers, const uint* byte_offsets)
{
	static const uint zeros[D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT];
	uint strides[D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT];
	Buffer* bufs[D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT];
	for (uint i = 0; i < num_buffers; i++)
	{
		strides[i] = buffers[i]->stride();
		bufs[i] = (Buffer*)buffers[i]->impl;
	}		
	
	get_dc(cl)->IASetVertexBuffers(slot, num_buffers, bufs, strides, byte_offsets ? byte_offsets : zeros);
}

void vertex_buffer::set(command_list& cl, uint slot, uint num_buffers, const vertex_buffer* buffers, const uint* byte_offsets)
{
	static const uint zeros[D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT];
	uint strides[D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT];
	Buffer* bufs[D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT];
	for (uint i = 0; i < num_buffers; i++)
	{
		strides[i] = buffers[i].stride();
		bufs[i] = (Buffer*)buffers[i].impl;
	}		
	
	get_dc(cl)->IASetVertexBuffers(slot, num_buffers, bufs, strides, byte_offsets ? byte_offsets : zeros);
}

void vertex_buffer::update(command_list& cl, uint vertex_offset, uint num_vertices, const void* vertices)
{
	update_buffer(get_dc(cl), (Buffer*)impl, vertex_offset * stride(), num_vertices * stride(), vertices);
}

void vertex_buffer::draw(command_list& cl, uint num_indices, uint first_index_index, int per_index_offset, uint num_instances)
{
	get_dc(cl)->DrawIndexedInstanced(num_indices, num_instances, first_index_index, per_index_offset, 0);
}

void vertex_buffer::draw_unindexed(command_list& cl, uint num_vertices, uint first_vertex_index, uint num_instances)
{
	get_dc(cl)->DrawInstanced(num_vertices, num_instances, first_vertex_index, 0);
}

}}
