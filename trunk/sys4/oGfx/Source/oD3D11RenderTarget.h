// $(header)
#pragma once
#ifndef oD3D11RenderTarget2_h
#define oD3D11RenderTarget2_h

#include "oGfxCommon.h"
#include <oooii/oD3D11.h>

oDECLARE_GFXDEVICECHILD_IMPLEMENTATION(oD3D11, RenderTarget2)
{
	oDEFINE_GFXDEVICECHILD_INTERFACE();
	oDECLARE_GFXDEVICECHILD_CTOR(oD3D11, RenderTarget2);

	void GetDesc(DESC* _pDesc) const threadsafe override;
	void SetClearDesc(const CLEAR_DESC& _ClearDesc) threadsafe override;
	void Resize(uint _Width, uint _Height) override;

	oRef<ID3D11Texture2D> Texture[MAX_MRT_COUNT];
	oRef<ID3D11RenderTargetView> RTVs[MAX_MRT_COUNT];
	oRef<ID3D11ShaderResourceView> SRVs[MAX_MRT_COUNT];
	oRef<ID3D11Texture2D> Depth;
	oRef<ID3D11DepthStencilView> DSV;
	oRef<ID3D11ShaderResourceView> SRVDepth;

	oRef<ID3D11Device> Device;
	oRWMutex DescMutex;
	DESC Desc;
};

#endif
