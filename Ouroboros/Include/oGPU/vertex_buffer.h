// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.
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
