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

namespace ouro {
	namespace gpu {

util_mesh::util_mesh()
	: num_prims(0)
	, num_slots(0)
{
}

void util_mesh::initialize(const char* name, device* dev, const mesh::info& _info)
{
	if (_info.num_ranges != 1)
		oTHROW_INVARG("mesh range must be 1");

	info = _info;
	num_prims = mesh::num_primitives(info);

	indices = make_index_buffer(dev, name, info.num_indices, info.num_vertices);

	num_slots = 0;
	for (uint slot = 0; slot < mesh::max_num_slots; slot++)
	{
		uint VertexSize = calc_vertex_size(info.elements, slot);
		if (!VertexSize)
			continue;

		buffer_info i;
		i.type = buffer_type::vertex;
		i.array_size = info.num_vertices;
		i.struct_byte_size = as_ushort(VertexSize);
		i.format = surface::unknown;

		vertices[slot] = dev->make_buffer(name, i);
		num_slots = slot + 1; // record the highest one since vertex buffers are set in one go
	}
}

void util_mesh::initialize(const char* name, device* dev, const mesh::element_array& elements, const mesh::primitive* prim)
{
	auto prim_info = prim->get_info();

	oCHECK(prim_info.num_ranges == 1, "unexpected number of ranges");

	auto source = prim->get_source();

	// copy indices
	surface::const_mapped_subresource msrIndices;
	msrIndices.data = source.indices;
	msrIndices.row_pitch = sizeof(uint);
	indices = make_index_buffer(dev, name, prim_info.num_indices, prim_info.num_vertices, msrIndices);

	// copy vertices
	std::array<surface::mapped_subresource, mesh::max_num_slots> msrs;
	std::array<void*, mesh::max_num_slots> dsts;
	finally UnmapMSRs([&]
	{
		for (uint slot = 0; slot < mesh::max_num_slots; slot++)
		{
			if (msrs[slot].data)
				dev->immediate()->commit(vertices[slot].get(), 0, msrs[slot]);
		}
	});

	num_slots = 0;
	for (uint slot = 0; slot < mesh::max_num_slots; slot++)
	{
		uint VertexSize = mesh::calc_vertex_size(elements, slot);
		if (!VertexSize)
			continue;

		buffer_info i;
		i.type = buffer_type::vertex;
		i.array_size = prim_info.num_vertices;
		i.struct_byte_size = as_short(VertexSize);
		i.format = surface::unknown;

		vertices[slot] = dev->make_buffer(name, i);
		msrs[slot] = dev->immediate()->reserve(vertices[slot].get(), 0);
		dsts[slot] = msrs[slot].data;
		num_slots = slot + 1; // record the highest one since vertex buffers are set in one go
	}

	copy_vertices(dsts.data(), elements, source.streams, prim_info.elements, prim_info.num_vertices);

	// fill out info
	num_prims = mesh::num_primitives(prim_info);
	info = prim_info;
	info.elements = elements;
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

	initialize("First Triangle", dev, mi);

	surface::const_mapped_subresource msr;
	static const ushort sIndices[] = { 0, 1, 2 };
	msr.data = (void*)sIndices;
	msr.row_pitch = sizeof(ushort);
	msr.depth_pitch = sizeof(sIndices);
	dev->immediate()->commit(index_buffer(), 0, msr);
	
	static const float3 sPositions[] = { float3(-X, -Y, 0.0f), float3(0.0f, Y, 0.0f), float3(X, -Y, 0.0f), };
	msr.data = sPositions;
	msr.row_pitch = sizeof(float3);
	msr.depth_pitch = sizeof(sPositions);
	dev->immediate()->commit(vertex_buffer(), 0, msr);
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
}

void util_mesh::draw(command_list* cl)
{
	std::array<buffer*, mesh::max_num_slots> VBs;
	for (uint i = 0; i < num_slots; i++)
		VBs[i] = vertices[i].get();

	cl->draw(indices.get(), 0, num_slots, VBs.data(), 0, num_prims);
}

}}
