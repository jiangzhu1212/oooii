// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.
#pragma once
#ifndef oGPU_readback_buffer_h
#define oGPU_readback_buffer_h

namespace ouro { namespace gpu {

class device;
class command_list;

class readback_buffer
{
public:
	readback_buffer() : impl(nullptr), bytes(0) {}
	~readback_buffer() { deinitialize(); }

	void initialize(const char* name, device& dev, uint element_stride, uint num_elements = 1);
	template<typename BufferT> void initialize(const BufferT& buffer, bool make_immediate_copy = false) { internal_initialize(buffer.get_buffer(), make_immediate_copy); }

	void deinitialize();
	void* get_buffer() const { return impl; }

	uint size() const { return bytes; }

	template<typename T> void copy_from(command_list& cl, const T& buffer) { internal_copy_from(cl, buffer.get_buffer()); }

	// returns false if this could not be read without blocking. Any invalid condition will throw.
	bool copy_to(void* dst, size_t dst_size, bool blocking = true) const;

private:
	void* impl;
	uint bytes;

	void internal_initialize(void* buffer_impl, bool make_immediate_copy);
	void internal_copy_from(command_list& cl, void* buffer_impl);
};
	
}}

#endif
