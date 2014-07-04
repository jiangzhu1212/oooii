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
#include <oGPU/index_buffer.h>
#include <oCore/windows/win_error.h>
#include "d3d_debug.h"
#include "d3d_util.h"

using namespace ouro::gpu::d3d;

namespace ouro { namespace gpu {

Device* get_device(device* dev);
DeviceContext* get_dc(command_list* cl);

void index_buffer::initialize(const char* name, device* dev, uint num_indices, const ushort* indices)
{
	deinitialize();
	impl = (void*)make_buffer(name, get_device(dev), num_indices, sizeof(ushort), D3D11_BIND_INDEX_BUFFER, 0, indices);
}

void index_buffer::deinitialize()
{
	if (impl)
		((Buffer*)impl)->Release();
	impl = nullptr;
}

char* index_buffer::name(char* dst, size_t dst_size) const
{
	return impl ? debug_name(dst, dst_size, (Buffer*)impl) : "uninitialized";
}

uint index_buffer::num_indices() const
{
	D3D11_BUFFER_DESC d;
	((Buffer*)impl)->GetDesc(&d);
	return d.ByteWidth / sizeof(ushort);
}

void index_buffer::set(command_list* cl, uint start_index) const
{
	get_dc(cl)->IASetIndexBuffer((Buffer*)impl, DXGI_FORMAT_R16_UINT, start_index * sizeof(ushort));
}

void index_buffer::update(command_list* cl, uint index_offset, uint num_indices, const ushort* indices)
{
	update_buffer(get_dc(cl), (Buffer*)impl, index_offset * sizeof(ushort), num_indices * sizeof(ushort), indices);
}

}}
