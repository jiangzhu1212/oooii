// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.
#include <oMesh/model.h>
#include <oMesh/mesh.h>
#include <oMemory/fnv1a.h>

namespace ouro { namespace mesh {

static uint calc_vertex_size(const element_array& elements, uint* out_nslots = 0)
{
	uint vertex_size = 0;
	uint nslots = 0;
	for (uint slot = 0; slot < max_num_slots; slot++)
	{
		uint slot_size = calc_vertex_size(elements, slot);
		if (slot_size)
		{
			vertex_size += slot_size;
			nslots++;
		}
	}

	if (out_nslots)
		*out_nslots = nslots;
	return vertex_size;
}

void model::initialize_placement()
{
	data = (uchar*)(this + 1);
	// everything else should be already set up
}

void model::initialize(const model_info& i, const char** material_names, const allocator& a)
{
	info = i;
	dealloc = (ullong)a.deallocate;
	
	// sum up component sizes
	uint nslots = 0;
	uint sizeof_vertex = calc_vertex_size(i.elements, &nslots);

	uint subset_data_size = sizeof(model_subset) * i.num_subsets;

	uint rt_vertex_size = sizeof(ullong) * nslots;
	uint rt_indices_size = sizeof(ullong);
	uint rt_materials_size = sizeof(ullong) * i.num_subsets;
	uint rt_names_size = sizeof(ullong) * i.num_subsets;

	uint vertex_slots_size = sizeof(uint) * nslots;
	uint vertex_data_size = sizeof_vertex * i.num_vertices;
	uint material_hashes_size = sizeof(ullong) * i.num_subsets;
	uint indices_data_size = sizeof(ushort) * i.num_indices;

	uint material_names_size = 0;
	for (uint subset = 0; subset < i.num_subsets; subset++)
		material_names_size += (uint)strlen(material_names[subset]) + 1;

	uint total_bytes = subset_data_size
		+ rt_vertex_size + rt_indices_size + rt_materials_size + rt_names_size
		+ vertex_slots_size + vertex_data_size + material_hashes_size + indices_data_size
		+ material_names_size;

	// allocate
	data = (uchar*)a.allocate(total_bytes, memory_alignment::align_default, "model data");
	oASSERT(data, "failed alloc");

	// setup offsets into data
	rt_vertex_slots_offset = 0 + subset_data_size;
	rt_indices_offset = rt_vertex_slots_offset + rt_vertex_size;
	rt_materials_offset = rt_indices_offset + rt_indices_size;
	rt_material_names_offset = rt_materials_offset + rt_materials_size;
	
	vertex_slots_offset = rt_material_names_offset + rt_names_size;
	material_hashes_offset = vertex_slots_offset + vertex_slots_size + vertex_data_size;
	indices_offset = material_hashes_offset + material_hashes_size;
	material_names_offset = indices_offset + indices_data_size;

	// set pointer memory to nullptr
	memset(data+rt_vertex_slots_offset, 0, vertex_slots_offset - rt_vertex_slots_offset);

	// assign material names
	ullong* material_hash = (ullong*)(data+material_hashes_offset);
	char* name = (char*)(data+material_names_offset);
	for (uint subset = 0; subset < i.num_subsets; subset++)
	{
		const char* material_name = material_names[subset];
		*material_hash++ = fnv1a<ullong>(material_name);
		name += strlcpy(name, material_name, material_names_size) + 1;
	}
}

void model::deinitialize()
{
	if (dealloc)
	{
		((deallocate_fn)dealloc)(data);
		data = nullptr;
		rt_vertex_slots_offset = 0;
		rt_indices_offset = 0;
		rt_materials_offset = 0;
		rt_material_names_offset = 0;
		vertex_slots_offset = 0;
		material_hashes_offset = 0;
		indices_offset = 0;
		material_names_offset = 0;
		dealloc = 0;
	}
}

}}
