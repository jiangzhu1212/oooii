// $(header)
#pragma once
#ifndef oD3D11Material_h
#define oD3D11Material_h

#include <oGfx/oGfx.h>
#include "oGfxCommon.h"
#include <oooii/oD3D11.h>

oDECLARE_GFXRESOURCE_IMPLEMENTATION(oD3D11, Material, MATERIAL)
{
	oDEFINE_GFXRESOURCE_INTERFACE();
	oDECLARE_GFXRESOURCE_CTOR(oD3D11, Material);
	oRef<ID3D11Buffer> Constants;
};

#endif
