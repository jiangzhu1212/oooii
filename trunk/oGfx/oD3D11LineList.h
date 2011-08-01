// $(header)
#pragma once
#ifndef oD3D11LineList_h
#define oD3D11LineList_h

#include <oGfx/oGfx.h>
#include "oGfxCommon.h"
#include <oooii/oD3D11.h>

oDECLARE_GFXRESOURCE_IMPLEMENTATION(oD3D11, LineList, LINELIST)
{
	oDEFINE_GFXRESOURCE_INTERFACE();
	oDECLARE_GFXRESOURCE_CTOR(oD3D11, LineList);
	oRef<ID3D11Buffer> Lines;

	void Resize(uint _MaxNumLines) override;
	void SetNumLines(uint _NumLines) override;
};

#endif
