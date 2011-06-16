// $(header)
#pragma once
#ifndef oD3D11Texture_h
#define oD3D11Texture_h

#include <SYS4/SYS4Render.h>
#include "SYS4ResourceBaseMixin.h"

struct oD3D11Texture : oGPUMesh, oNoncopyable, SYS4ResourceBaseMixin<oGPUTexture, oD3D11Texture, oGPUResource::TEXTURE>
{
	SYS4_DEFINE_GPURESOURCE_INTERFACE();
	
	oD3D11Texture(threadsafe oGPUDevice* _pDevice, const DESC& _Desc, bool* _pSuccess);

	oRef<ID3D11Texture2D> Texture;
	oRef<ID3D11ShaderResourceView> SRV;
};

#endif