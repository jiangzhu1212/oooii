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
#include <oGPU/oGPUUtilMesh.h>
#include <oGPU/oGPUUtil.h>
#include <oGPU/vertex_layout.h>
#include <oBase/finally.h>
#include <oMesh/mesh.h>

namespace ouro {
	namespace gpu {

static_assert(vertex_buffer::max_num_slots == mesh::max_num_slots, "size mismatch");
static_assert(vertex_buffer::max_num_elements == mesh::max_num_elements, "size mismatch");

util_mesh::util_mesh()
	: num_prims(0)
	, num_slots(0)
{
}

void util_mesh::initialize(const char* name, device* dev, const mesh::info& _info, const ushort* _indices, const void** _vertices)
{
	if (_info.num_ranges != 1)
		oTHROW_INVARG("mesh range must be 1");

	info = _info;
	num_prims = mesh::num_primitives(info);

	indices.initialize(name, dev, info.num_indices, _indices);

	num_slots = 0;
	for (uint slot = 0; slot < mesh::max_num_slots; slot++)
	{
		uint VertexSize = calc_vertex_size(info.elements, slot);
		if (!VertexSize)
			continue;

		vertices[slot].initialize(name, dev, info.num_vertices, VertexSize, _vertices ? _vertices[slot] : nullptr);
		num_slots = slot + 1; // record the highest one since vertex buffers are set in one go
	}
}

void util_mesh::initialize(const char* name, device* dev, const mesh::element_array& elements, const mesh::primitive* prim)
{
	auto prim_info = prim->get_info();
	oCHECK(prim_info.num_ranges == 1, "unexpected number of ranges");

	info = prim_info;
	info.elements = elements;
	num_prims = mesh::num_primitives(info);

	auto source = prim->get_source();

	// copy indices
	{
		ushort* SrcIndices = new ushort[info.num_indices];
		finally FreeIndices([&] { if (SrcIndices) delete [] SrcIndices; });
		mesh::copy_indices(SrcIndices, source.indices, info.num_indices);
		indices.initialize(name, dev, info.num_indices, SrcIndices);
	}

	// copy vertices
	num_slots = 0;
	for (uint slot = 0; slot < mesh::max_num_slots; slot++)
	{
		uint VertexSize = mesh::calc_vertex_size(elements, slot);
		if (!VertexSize)
			continue;

		void* dsts[mesh::max_num_slots];
		memset(dsts, 0, sizeof(void*) * mesh::max_num_slots);

		{
			dsts[slot] = new char[VertexSize * info.num_vertices];
			finally FreeDsts([&] { if (dsts[slot]) delete [] dsts[slot]; });
			mesh::copy_vertices(dsts, elements, source.streams, prim_info.elements, info.num_vertices);
			vertices[slot].initialize(name, dev, prim_info.num_vertices, VertexSize, dsts[slot]);
		}

		num_slots = slot + 1; // record the highest one since vertex buffers are set in one go
	}
}

void util_mesh::initialize_first_triangle(device* dev)
{
	static const float3 sExtents(0.8f, 0.7f, 0.01f);
	static const float X = 0.75f;
	static const float Y = 0.667f;

	mesh::info mi;
	mi.local_space_bound = aaboxf(aaboxf::min_max, -sExtents, sExtents);
	mi.num_indices = 3;
	mi.num_vertices = 3;
	mi.num_ranges = 1;
	mi.elements[0] = mesh::element(mesh::semantic::position, 0, mesh::format::xyz32_float, 0);
	mi.face_type = mesh::face_type::front_ccw;
	mi.primitive_type = mesh::primitive_type::triangles;
	mi.vertex_scale_shift = 0;

	static const ushort sIndices[] = { 0, 1, 2 };
	static const float3 sPositions[] = { float3(-X, -Y, 0.0f), float3(0.0f, Y, 0.0f), float3(X, -Y, 0.0f), };

	const void* elements[vertex_buffer::max_num_slots];
	memset(elements, 0, sizeof(void*) * oCOUNTOF(elements));
	elements[0] = sPositions;
	initialize("First Triangle", dev, mi, sIndices, elements);
}

void util_mesh::initialize_first_cube(device* dev)
{
	mesh::primitive::box_init i;
	i.semantics = mesh::primitive::flag_positions|mesh::primitive::flag_texcoords;
	i.face_type = mesh::face_type::front_ccw;
	i.divide = 1;
	i.color = white;
	i.bound = aaboxf(aaboxf::min_max, float3(-1.0f), float3(1.0f));
	i.flipv = false;

	mesh::primitive::unique_ptr prim(mesh::primitive::make(i));

	mesh::element_array Elements;
	Elements[0] = mesh::element(mesh::semantic::position, 0, mesh::format::xyz32_float, 0);
	Elements[1] = mesh::element(mesh::semantic::texcoord, 0, mesh::format::xy32_float, 0);
	
	initialize("First Cube", dev, Elements, prim.get());
}

void util_mesh::deinitialize()
{
	indices.deinitialize();
	for (auto& v : vertices)
		v.deinitialize();
}

void util_mesh::draw(command_list* cl)
{
	indices.set(cl);
	vertex_buffer::set(cl, 0, num_slots, vertices.data());
	vertex_buffer::draw(cl, indices.num_indices(), 0, 0);
}

}}
