// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.
#include <oGPU/constant_buffer.h>
#include "d3d_debug.h"
#include "d3d_util.h"

using namespace ouro::gpu::d3d;

namespace ouro { namespace gpu {

Device* get_device(device& dev);
DeviceContext* get_dc(command_list& cl);

void constant_buffer::initialize(const char* name, device& dev, uint struct_stride, uint num_structs, const void* src)
{
	deinitialize();
	const uint num_bytes = struct_stride * num_structs;
	oCHECK_ARG(byte_aligned(num_bytes, 16) && num_bytes <= 65535, "constant_buffer must be size-aligned to 16 and smaller than 64 kilobytes. (size %u bytes specified)", num_bytes);
	auto b = make_buffer(name, get_device(dev), struct_stride, num_structs, D3D11_USAGE_DEFAULT, D3D11_BIND_CONSTANT_BUFFER, 0, src);
	b->AddRef();
	impl = b;
}

void constant_buffer::deinitialize()
{
	oSAFE_RELEASEV(impl);
}

char* constant_buffer::name(char* dst, size_t dst_size) const
{
	return impl ? debug_name(dst, dst_size, (Buffer*)impl) : "uninitialized";
}

void constant_buffer::set(command_list& cl, uint slot) const
{
	set_cbs(get_dc(cl), slot, 1, (Buffer* const*)&impl);
}

void constant_buffer::set(command_list& cl, uint slot, uint num_buffers, const constant_buffer* const* buffers)
{
	Buffer* bufs[D3D11_COMMONSHADER_CONSTANT_BUFFER_REGISTER_COUNT];
	for (uint i = 0; i < num_buffers; i++)
		bufs[i] = (Buffer*)buffers[i]->impl;
	set_cbs(get_dc(cl), slot, num_buffers, bufs);
}

void constant_buffer::update(command_list& cl, const void* src)
{
	get_dc(cl)->UpdateSubresource((Buffer*)impl, 0, nullptr, src, 0, 0);
}

}}
