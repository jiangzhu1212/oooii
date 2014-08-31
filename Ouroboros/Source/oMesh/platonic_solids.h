// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.
#pragma once
#ifndef oMesh_platonic_solids_h
#define oMesh_platonic_solids_h

#include <oBase/types.h>

namespace ouro { namespace mesh {

struct platonic_info
{
	const float3* positions;
	const uint* indices;
	uint num_indices;
	uint num_vertices;
	uint num_edges;
	uint num_faces;
};

namespace platonic
{	enum value {

	tetrahedron,
	hexahedron, // cube
	octahedron,
	dodecahedron,
	icosahedron,

	count,

};}

platonic_info get_platonic_info(const platonic::value& type);

}}

#endif
