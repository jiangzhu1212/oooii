// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.
#pragma once
#ifndef oGPU_index_buffer_h
#define oGPU_index_buffer_h

#include <oBase/types.h>

namespace ouro { namespace gpu {

class device;
class command_list;

class index_buffer
{
public:
	index_buffer() : impl(nullptr) {}
	~index_buffer() { deinitialize(); }
	void initialize(const char* name, device& dev, uint num_indices, const ushort* indices = nullptr);
	void deinitialize();

	char* name(char* dst, size_t dst_size) const;
	uint num_indices() const;
	void set(command_list& cl, uint start_index = 0) const;
	void update(command_list& cl, uint index_offset, uint num_indices, const ushort* indices);
	void* get_buffer() const { return impl; }

private:
	void* impl;
};
	
}}

#endif
