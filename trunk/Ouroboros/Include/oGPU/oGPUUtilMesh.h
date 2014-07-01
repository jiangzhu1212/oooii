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
// A very simple mesh container that mainly provides very simple creation
// and drawing, mostly used for unit testing and samples.
#pragma once
#ifndef oGPU_util_mesh_h
#define oGPU_util_mesh_h

#include <oGPU/oGPU.h>
#include <oMesh/obj.h>
#include <oMesh/primitive.h>

namespace ouro { namespace gpu {

class device;
class command_list;

class util_mesh
{
public:
	util_mesh();
	~util_mesh() { deinitialize(); }

	void initialize(const char* name, device* dev, const mesh::info& info);
	void initialize(const char* name, device* dev, const mesh::element_array& elements, const mesh::primitive* prim);

	// Creates a very simple front-facing triangle that can be rendered with all-
	// identity world, view, projection matrices. This is useful for very simple 
	// tests and first bring-up.
	void initialize_first_triangle(device* dev);
	
	// Creates a very simple unit cube. This is useful for bringing up world, view,
	// projection transforms quickly.
	void initialize_first_cube(device* dev);

	void deinitialize();

	inline mesh::info get_info() const { return info; }
	inline buffer* index_buffer() { return indices.get(); }
	inline const buffer* index_buffer() const { return indices.get(); }
	inline buffer* vertex_buffer(uint index = 0) { return vertices[index].get(); }
	inline const buffer* vertex_buffer(uint index = 0) const { return vertices[index].get(); }
	inline void vertex_buffers(const buffer* buffers[mesh::max_num_slots]) const
	{
		for (auto const& b : vertices)
			*buffers++ = b.get();
	}

	void draw(command_list* cl);

private:
	std::shared_ptr<buffer> indices;
	std::array<std::shared_ptr<buffer>, mesh::max_num_slots> vertices;
	mesh::info info;
	uint num_prims;
	uint num_slots;
};

}}

#endif
