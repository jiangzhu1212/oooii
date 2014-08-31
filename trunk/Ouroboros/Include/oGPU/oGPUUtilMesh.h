// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.
// A very simple mesh container that mainly provides very simple creation
// and drawing, mostly used for unit testing and samples.
#pragma once
#ifndef oGPU_util_mesh_h
#define oGPU_util_mesh_h

#include <oGPU/index_buffer.h>
#include <oGPU/vertex_buffer.h>
#include <oMesh/obj.h>
#include <oMesh/primitive.h>

namespace ouro { namespace gpu {

class device;
class command_list;
class vertex_layout;

class util_mesh
{
public:
	util_mesh();
	~util_mesh() { deinitialize(); }

	// vertices must be an array of size vertex_buffer::max_num_slots
	void initialize(const char* name, device& dev, const mesh::info& info, const ushort* indices = nullptr, const void** vertices = nullptr);
	void initialize(const char* name, device& dev, const mesh::element_array& elements, const mesh::primitive* prim);

	// Creates a very simple front-facing triangle that can be rendered with all-
	// identity world, view, projection matrices. This is useful for very simple 
	// tests and first bring-up.
	void initialize_first_triangle(device& dev);
	
	// Creates a very simple unit cube. This is useful for bringing up world, view,
	// projection transforms quickly.
	void initialize_first_cube(device& dev, bool _UVWs = false);

	void deinitialize();

	inline mesh::info get_info() const { return info; }
	inline index_buffer& get_index_buffer() { return indices; }
	inline const index_buffer& getindex_buffer() const { return indices; }
	inline vertex_buffer& get_vertex_buffer(uint index = 0) { return vertices[index]; }
	inline const vertex_buffer& get_vertex_buffer(uint index = 0) const { return vertices[index]; }
	inline const vertex_buffer* get_vertex_buffers() const { return vertices.data(); }

	void draw(command_list& cl, uint num_instances = 1);

private:
	index_buffer indices;
	std::array<vertex_buffer, mesh::max_num_slots> vertices;
	mesh::info info;
	uint num_prims;
	uint num_slots;
};

}}

#endif
