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
