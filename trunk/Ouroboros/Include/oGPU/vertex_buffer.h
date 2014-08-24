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
#ifndef oGPU_vertex_buffer_h
#define oGPU_vertex_buffer_h

#include <oBase/types.h>

namespace ouro { namespace gpu {

class device;
class command_list;

class vertex_buffer
{
public:
	static const uint max_num_slots = 8;
	static const uint max_num_elements = 16;

	vertex_buffer() : impl(nullptr), vstride(0) {}
	~vertex_buffer() { deinitialize(); }

	void initialize(const char* name, device& dev, uint vertex_stride, uint num_vertices, const void* vertices = nullptr);
	void deinitialize();

	char* name(char* dst, size_t dst_size) const;
	uint num_vertices() const;
	inline uint stride() const { return vstride; }
	void set(command_list& cl, uint slot, uint byte_offset = 0) const;
	static void set(command_list& cl, uint slot, uint num_buffers, vertex_buffer* const* buffers, const uint* byte_offsets = nullptr);
	static void set(command_list& cl, uint slot, uint num_buffers, const vertex_buffer* buffers, const uint* byte_offsets = nullptr);

	void update(command_list& cl, uint vertex_offset, uint num_vertices, const void* vertices);

	// draws the currently bound vertex layout, vertex buffers and index buffers
	static void draw(command_list& cl, uint num_indices, uint first_index_index, int per_index_offset, uint num_instances = 1);
	static void draw_unindexed(command_list& cl, uint num_vertices, uint first_vertex_index = 0, uint num_instances = 1);

private:
	void* impl;
	uint vstride;
};
	
}}

#endif
