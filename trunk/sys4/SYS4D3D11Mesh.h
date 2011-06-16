// $(header)
#pragma once
#ifndef oD3D11Mesh_h
#define oD3D11Mesh_h

#include <SYS4/SYS4Render.h>
#include "SYS4ResourceBaseMixin.h"

struct oD3D11Mesh : oGPUMesh, oNoncopyable, SYS4ResourceBaseMixin<oGPUMesh, oD3D11Mesh, oGPUResource::MESH>
{
	SYS4_DEFINE_GPURESOURCE_INTERFACE();
	
	oD3D11Mesh(threadsafe oGPUDevice* _pDevice, const DESC& _Desc, bool* _pSuccess);

	std::vector<RANGE> Ranges;
	oRef<ID3D11Buffer> Indices;
	oRef<ID3D11Buffer> Vertices;
	oRef<ID3D11Buffer> Skinning;
};

#endif
