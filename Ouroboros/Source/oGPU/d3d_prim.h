// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.
#pragma once
#ifndef oGPU_d3d_prim_h
#define oGPU_d3d_prim_h

#include <oMesh/mesh.h>

enum D3D_PRIMITIVE_TOPOLOGY;

namespace ouro { namespace gpu { namespace d3d {

D3D_PRIMITIVE_TOPOLOGY from_primitive_type(const mesh::primitive_type& type);
mesh::primitive_type to_primitive_type(D3D_PRIMITIVE_TOPOLOGY type);

// Returns the number of elements as described the specified topology given
// the number of primitives. An element can refer to indices or vertices, but
// basically if there are 3 lines, then there are 6 elements. If there are 3
// lines in a line strip, then there are 4 elements.
uint num_elements(D3D_PRIMITIVE_TOPOLOGY topology, uint num_primitives);

}}}

#endif
