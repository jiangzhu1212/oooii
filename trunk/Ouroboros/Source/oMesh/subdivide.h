// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.
#pragma once
#ifndef oMesh_subdivide_h
#define oMesh_subdivide_h

#include <oBase/types.h>
#include <vector>

namespace ouro { namespace mesh {

typedef void (*append_midpoint_fn)(uint i0, uint i1, void* data);

// Subdivides each triangle in a mesh by finding the midpoints of each 
// edge and creating a new point there, generating 4 new triangles to 
// replace the prior triangle. This does not duplicate points for 
// triangles that share an edge. It is recommended that vectors  
// containing vertex data should reserve 2*num_edges additional vertices
// before calling this function to limit the number of reallocations.
// in_out_num_edges: The valid number of edges in the specified mesh 
//            must be passed. This value will be filled with the new 
//            edge count after subdivision. In this way subdivide() can 
//            be called recursively up to the desired subdivision level.
// num_vertices: the number of vertices before subdivision. This is used
//               to prepare edge lists.
// indices: List of indices. This will be modified during subdivision
// append_midpoint: a callback to handle all interpolants when a new
//                  vertex is required.
void subdivide(uint* in_out_num_edges, std::vector<uint>& indices, uint num_vertices, append_midpoint_fn append_midpoint, void* data);

}}

#endif
