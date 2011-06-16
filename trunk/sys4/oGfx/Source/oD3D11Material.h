// $(header)
#pragma once
#ifndef oD3D11Material_h
#define oD3D11Material_h

#include <SYS4/SYS4Render.h>
#include "SYS4ResourceBaseMixin.h"

struct oD3D11Material : oGPUMesh, oNoncopyable, SYS4ResourceBaseMixin<oGPUMaterial, oD3D11Material, oGPUResource::MATERIAL>
{
	SYS4_DEFINE_GPURESOURCE_INTERFACE();
	
	oD3D11Material(threadsafe oGPUDevice* _pDevice, const DESC& _Desc, bool* _pSuccess);

	oRef<ID3D11Buffer> Constants;
};

#endif
