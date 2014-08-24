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
// Runtime representation of vertex and index information
#pragma once
#ifndef oMesh_model_h
#define oMesh_model_h

#include <oMesh/mesh.h>

namespace ouro { namespace mesh {

struct model_subset
{
	enum flag
	{
		skinned = 1,
		atested = 2,
		blended = 4,
	};

	uint start_vertex;
	uint num_vertices;
	uint start_index;
	uint num_indices;
	ushort material_index;
	ushort padA;
	uint flags;
	float2 material_lod_dist;
};
static_assert(sizeof(model_subset) == 32, "size mismatch");

struct model_subset_range
{
	model_subset_range() : start_range(0), num_ranges(0) {}

	ushort start_range;
	ushort num_ranges;
};
static_assert(sizeof(model_subset_range) == 4, "size mismatch");

struct model_lod
{
	// first 3 subsets are for shadow casting
	// opaque_color.start_range should always be 3

	enum
	{
		shadow_opaque,
		shadow_atest,
		shadow_blend,
	};

	model_subset_range opaque_color;
	model_subset_range atest_color;
	model_subset_range blend_color;
	model_subset_range collision;
};
static_assert(sizeof(model_lod) == 16, "size mismatch");

struct model_info
{
	model_info()
		: num_vertices(0)
		, num_indices(0)
		, num_subsets(0)
		, log2scale(0)
		, flags(0)
		, extents(0.0f, 0.0f, 0.0f)
		, avg_edge_length(0.0f)
		, lod_distances(0.0f, 0.0f)
		, avg_texel_density(0.0f, 0.0f)
	{}

	uint num_vertices;
	uint num_indices;
	ushort num_subsets;
	ushort log2scale; // for ushort4 compressed verts: [-32768,32767] -> [-1,1] * 2^n wheren n is this number.
	uint flags;
	spheref bounding_sphere; // inscribed in the obb
	float3 extents; // forms an obb from the sphere's center
	float avg_edge_length;
	float2 lod_distances;
	float2 avg_texel_density;
	element_array elements;
	model_lod lods[3];
};
static_assert(sizeof(model_info) == (64 + sizeof(element_array) + 3*sizeof(model_lod)), "size mismatch");

class model
{
public:

	model();
	~model();

	// call when the memory for this object is directly loaded from disk
	void initialize_placement();

	void initialize(const model_info& i, const char** material_names, const allocator& a = default_allocator);
	void deinitialize();

	model_info get_info() const { return info; }


	// these are set up at initialize time so there is no mutator
	ullong material_hash(uint subset_index) const { return ((const ullong*)(data+material_hashes_offset))[subset_index]; }
	const char* material_name(uint subset_index) const { return ((const char**)(data+rt_material_names_offset))[subset_index]; }


	// indexed by subset
	const model_subset* subsets() const { return (const model_subset*)(data+0); }
	model_subset* subsets() { return (model_subset*)(data+0); }


	// indexed by slot, then has num_vertices * the size of the vertex elements in that slot
	const void* vertices(uint slot) const { const uint slot_data_offset = ((const uint*)(data+vertex_slots_offset))[slot]; return ((const void*)(data+slot_data_offset)); }
	void* vertices(uint slot) { const uint slot_data_offset = ((const uint*)(data+vertex_slots_offset))[slot]; return ((void*)(data+slot_data_offset)); }

	template<typename T> T** rt_vertices(uint slot) const { return ((T**)(data+rt_vertex_slots_offset)); }
	template<typename T> T** rt_vertices() { return ((T**)(data+rt_vertex_slots_offset)); }


	// direct access to indices
	const ushort* indices() const { return (const ushort*)(data+indices_offset); }
	ushort* indices() { return (ushort*)(data+indices_offset); }
	
	template<typename T> T* rt_indices() const { return (T*)(data+rt_indices_offset); }
	template<typename T> T* rt_indices() { return (T*)(data+rt_indices_offset); }


	// indexed by subset
	template<typename T> T** rt_materials() const { return (T**)(data+rt_materials_offset); }
	template<typename T> T** rt_materials() { return (T**)(data+rt_materials_offset); }

private:
	model_info info;
	uchar* data;
	
	// 64-bit space is reserved in format for runtime pointer resolution
	uint rt_vertex_slots_offset;
	uint rt_indices_offset;
	uint rt_materials_offset;
	uint rt_material_names_offset;

	// original data
	uint vertex_slots_offset; // offsets for each slot for data pointer (same count as described by elements) vertex data immediately follows
	uint material_hashes_offset; // one for each subset
	uint indices_offset;
	uint material_names_offset; // one for each subset

	ullong dealloc;
};

}}

#endif
