// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.
#include "d3d_prim.h"
#include <d3d11.h>

using namespace ouro::gpu::d3d;

namespace ouro { namespace gpu { namespace d3d {

D3D_PRIMITIVE_TOPOLOGY from_primitive_type(const mesh::primitive_type& type)
{
	return D3D_PRIMITIVE_TOPOLOGY(type);
}

mesh::primitive_type to_primitive_type(D3D_PRIMITIVE_TOPOLOGY type)
{
	return mesh::primitive_type(type);
}

uint num_elements(D3D_PRIMITIVE_TOPOLOGY topology, uint num_primitives)
{
	switch (topology)
	{
		case D3D_PRIMITIVE_TOPOLOGY_POINTLIST: return num_primitives;
		case D3D_PRIMITIVE_TOPOLOGY_LINELIST: return num_primitives * 2;
		case D3D_PRIMITIVE_TOPOLOGY_LINESTRIP: return num_primitives + 1;
		case D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST: return num_primitives * 3;
		case D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP: return num_primitives + 2;
		case D3D_PRIMITIVE_TOPOLOGY_LINELIST_ADJ: return num_primitives * 2 * 2;
		case D3D_PRIMITIVE_TOPOLOGY_LINESTRIP_ADJ: return (num_primitives + 1) * 2;
		case D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST_ADJ: return num_primitives * 3 * 2;
		case D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP_ADJ: return (num_primitives + 2) * 2;
		case D3D_PRIMITIVE_TOPOLOGY_UNDEFINED: return 0;
		default: break;
	}
	return num_primitives * (topology -D3D11_PRIMITIVE_TOPOLOGY_1_CONTROL_POINT_PATCHLIST+1);
}

}}}
