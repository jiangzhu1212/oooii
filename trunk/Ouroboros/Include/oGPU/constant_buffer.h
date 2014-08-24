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
