// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.
#pragma once
#ifndef oGPU_constant_buffer_h
#define oGPU_constant_buffer_h

#include <oBase/types.h>

namespace ouro { namespace gpu {

class device;
class command_list;

class constant_buffer
{
public:
	constant_buffer() : impl(nullptr) {}
	~constant_buffer() { deinitialize(); }

	void initialize(const char* name, device& dev, uint struct_stride, uint num_structs = 1, const void* src = nullptr);
	void deinitialize();

	char* name(char* dst, size_t dst_size) const;
	void set(command_list& cl, uint slot) const;
	static void set(command_list& cl, uint slot, uint num_buffers, const constant_buffer* const* buffers);
	void update(command_list& cl, const void* src);
	template<typename T> void update(command_list& cl, const T& src) { update(cl, (const void*)&src); }
	void* get_buffer() const { return impl; }

private:
	void* impl;
};

}}

#endif
