// $(header)
#pragma once
#ifndef oD3D11Mesh_h
#define oD3D11Mesh_h

#include <oGfx/oGfx.h>
#include "oGfxCommon.h"
#include <oooii/oD3D11.h>

oDECLARE_GFXRESOURCE_IMPLEMENTATION(oD3D11, Mesh, MESH)
{
	oDEFINE_GFXRESOURCE_INTERFACE();
	oDECLARE_GFXRESOURCE_CTOR(oD3D11, Mesh);
	std::vector<RANGE> Ranges;
	oRef<ID3D11Buffer> Indices;
	oRef<ID3D11Buffer> Vertices[3];
};

#endif
