// $(header)
#pragma once
#ifndef oD3D11Texture_h
#define oD3D11Texture_h

#include <oGfx/oGfx.h>
#include "oGfxCommon.h"
#include <oooii/oD3D11.h>

oDECLARE_GFXRESOURCE_IMPLEMENTATION(oD3D11, Texture, TEXTURE)
{
	oDEFINE_GFXRESOURCE_INTERFACE();
	oDECLARE_GFXRESOURCE_CTOR(oD3D11, Texture);
	oRef<ID3D11Texture2D> Texture;
	oRef<ID3D11ShaderResourceView> SRV;
};

#endif
