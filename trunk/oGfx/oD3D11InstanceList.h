// $(header)
#pragma once
#ifndef oD3D11InstanceList_h
#define oD3D11InstanceList_h

#include <oGfx/oGfx.h>
#include "oGfxCommon.h"
#include <oooii/oD3D11.h>

oDECLARE_GFXRESOURCE_IMPLEMENTATION(oD3D11, InstanceList, INSTANCELIST)
{
	oDEFINE_GFXRESOURCE_INTERFACE();
	oDECLARE_GFXRESOURCE_CTOR(oD3D11, InstanceList);
	oRef<ID3D11Buffer> Instances;
};

#endif
