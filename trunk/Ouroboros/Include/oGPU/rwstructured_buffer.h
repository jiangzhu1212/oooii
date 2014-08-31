// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.
#pragma once
#ifndef oGPU_rwstructured_buffer_h
#define oGPU_rwstructured_buffer_h

#include <oGPU/compute_target.h>

namespace ouro { namespace gpu {

class device;
class command_list;
class compute_shader;

class rwstructured_buffer : public compute_target
{
public:
	rwstructured_buffer() : noop(nullptr) {}
	~rwstructured_buffer() { deinitialize(); }

	void initialize(const char* name, device& dev, uint struct_stride, uint num_structs, const void* src = nullptr);
	void initialize_append(const char* name, device& dev, uint struct_stride, uint num_structs, const void* src = nullptr);
	void initialize_counter(const char* name, device& dev, uint struct_stride, uint num_structs, const void* src = nullptr);
	template<typename T> void initialize(const char* name, device& dev, uint num_structs, const T* structs = nullptr) { initialize(name, dev, num_structs, sizeof(T), structs); }
	template<typename T> void initialize_append(const char* name, device& dev, uint num_structs, const T* structs = nullptr) { initialize_append(name, dev, num_structs, sizeof(T), structs); }
	template<typename T> void initialize_counter(const char* name, device& dev, uint num_structs, const T* structs = nullptr) { initialize_counter(name, dev, num_structs, sizeof(T), structs); }

	uint struct_stride() const;
	uint num_structs() const;
	
	void set(command_list& cl, uint slot);

	void update(command_list& cl, uint struct_offset, uint num_structs, const void* src);

	template<typename BufferT> void copy_counter_to(command_list& cl, BufferT& destination, uint offset_in_uints) { internal_copy_counter(cl, destination.get_buffer(), offset_in_uints); }
	
	void set_counter(command_list& cl, uint value);

private:
	compute_shader* noop;
	void internal_copy_counter(command_list& cl, void* dst_buffer_impl, uint offset_in_uints);
};
	
}}

#endif
