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
