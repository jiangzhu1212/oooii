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
