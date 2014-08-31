// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.
#include "subdivide.h"

namespace ouro { namespace mesh {

// Utility to find or create midpoints of triangles in a mesh. The main 
// inputs are two indices that define an edge. The other parameters are 
// caching values for the results of either finding a prior midpoint or 
// having to make a new vertex. This function is meant to be called 
// iteratively while building new midpoints of triangles in a mesh for 
// the purposes of tessellation. icurrent should be initialized to 0 and 
// be left alone. edge_starts, edge_ends and edge_mids should be able to
// accommodate as many edges as are in the mesh. For performance any
// lists or vectors that get midpoints appended should be pre-allocated
// since new midpoints will often be generated during tessellation.
//
// i0: first index of a triangle's edge for which to find a midpoint
// i0: second index of a triangle's edge for which to find a midpoint 
// icurrent: Called iteratively, icurrent keeps track of the last index 
//           where data was pulled from.
// edge_starts: First half of an edge list structure
// edge_ends: Last half of an edge list structure
// edge_mids: Keeps a log of the point in the middle of an edge whether 
//            it was found or created
// num_vertices: the number of vertices before this midpoint might be 
//               appended. This value gets updated if one is appended.
// append_midpoint: A callback to create a new midpoint between the 
//                  specified indices. Generally the function should be 
//                  implemented to access values at the specified end
//                  points, interpolate a new value and append it to the 
//                  container.
// This returns the index of the found or appended midpoint

static uint find_or_append_midpoint(uint i0, uint i1, uint& icurrent
	, uint* edge_starts, uint* edge_ends, uint* edge_mids
	, uint& num_vertices, append_midpoint_fn append_midpoint, void* data)
{
	for (uint i = 0; i < icurrent; i++)
	{
		if (((edge_starts[i] == i0) && (edge_ends[i] == i1)) 
		|| ((edge_starts[i] == i1) && (edge_ends[i] == i0)))
		{
			uint midpoint = edge_mids[i];
			icurrent--;
			edge_starts[i] = edge_starts[icurrent];
			edge_ends[i] = edge_ends[icurrent];
			edge_mids[i] = edge_mids[icurrent];
			return midpoint;
		}
	}

	// no vert found, make a new one
	edge_starts[icurrent] = i0;
	edge_ends[icurrent] = i1; 
	edge_mids[icurrent] = num_vertices;
	
	append_midpoint(i0, i1, data);
	num_vertices++;

	uint midpoint = edge_mids[icurrent++];
	return midpoint;
}

void subdivide(uint* in_out_num_edges, std::vector<uint>& indices, uint num_vertices, append_midpoint_fn append_midpoint, void* data)
{
	const uint num_indices = static_cast<uint>(indices.size());
	*in_out_num_edges = 2 * num_vertices + num_indices;
	std::vector<uint> start(*in_out_num_edges);
	std::vector<uint> end(*in_out_num_edges);
	std::vector<uint> mid(*in_out_num_edges);
	std::vector<uint> oldIndices(indices);
	indices.resize(4 * indices.size());

	uint indexFace = 0;
	uint icurrent = 0;
	const uint numFaces = num_indices / 3;
	for (uint i = 0; i < numFaces; i++) 
	{ 
		uint a = oldIndices[3*i]; 
		uint b = oldIndices[3*i+1]; 
		uint c = oldIndices[3*i+2]; 

		uint ab_mid = find_or_append_midpoint(b, a, icurrent, start.data(), end.data(), mid.data(), num_vertices, append_midpoint, data);
		uint bc_mid = find_or_append_midpoint(c, b, icurrent, start.data(), end.data(), mid.data(), num_vertices, append_midpoint, data);
		uint ca_mid = find_or_append_midpoint(a, c, icurrent, start.data(), end.data(), mid.data(), num_vertices, append_midpoint, data);

		indices[3*indexFace] = a;
		indices[3*indexFace+1] = ab_mid;
		indices[3*indexFace+2] = ca_mid;
		indexFace++;
		indices[3*indexFace] = ca_mid;
		indices[3*indexFace+1] = ab_mid;
		indices[3*indexFace+2] = bc_mid;
		indexFace++;
		indices[3*indexFace] = ca_mid;
		indices[3*indexFace+1] = bc_mid;
		indices[3*indexFace+2] = c;
		indexFace++;
		indices[3*indexFace] = ab_mid;
		indices[3*indexFace+1] = b;
		indices[3*indexFace+2] = bc_mid;
		indexFace++;
	} 
}

}}
