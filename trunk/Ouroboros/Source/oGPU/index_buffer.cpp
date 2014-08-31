// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.
#include <oGPU/index_buffer.h>
#include <oCore/windows/win_error.h>
#include "d3d_debug.h"
#include "d3d_util.h"

using namespace ouro::gpu::d3d;

namespace ouro { namespace gpu {

Device* get_device(device& dev);
DeviceContext* get_dc(command_list& cl);

void index_buffer::initialize(const char* name, device& dev, uint num_indices, const ushort* indices)
{
	deinitialize();
	auto b = make_buffer(name, get_device(dev), sizeof(ushort), num_indices, D3D11_USAGE_DEFAULT, D3D11_BIND_INDEX_BUFFER, 0, indices);
	b->AddRef();
	impl = b;
}

void index_buffer::deinitialize()
{
	oSAFE_RELEASEV(impl);
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

void index_buffer::set(command_list& cl, uint start_index) const
{
	get_dc(cl)->IASetIndexBuffer((Buffer*)impl, DXGI_FORMAT_R16_UINT, start_index * sizeof(ushort));
}

void index_buffer::update(command_list& cl, uint index_offset, uint num_indices, const ushort* indices)
{
	update_buffer(get_dc(cl), (Buffer*)impl, index_offset * sizeof(ushort), num_indices * sizeof(ushort), indices);
}

}}
