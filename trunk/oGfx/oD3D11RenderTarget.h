// $(header)
#pragma once
#ifndef oD3D11RenderTarget2_h
#define oD3D11RenderTarget2_h

#include "oGfxCommon.h"
#include <oooii/oD3D11.h>
#include <oooii/oWindow.h>

oDECLARE_GFXDEVICECHILD_IMPLEMENTATION(oD3D11, RenderTarget2)
{
	oDEFINE_GFXDEVICECHILD_INTERFACE();
	oDECLARE_GFXDEVICECHILD_CTOR(oD3D11, RenderTarget2);
	oD3D11RenderTarget2(threadsafe oGfxDevice* _pDevice, threadsafe oWindow* _pWindow, oSurface::FORMAT _DSFormat, const char* _Name, bool* _pSuccess);

	void GetDesc(DESC* _pDesc) const threadsafe override;
	void SetClearDesc(const CLEAR_DESC& _ClearDesc) threadsafe override;
	void Resize(uint _Width, uint _Height) override;

	oRef<ID3D11Texture2D> Texture[MAX_MRT_COUNT];
	oRef<ID3D11RenderTargetView> RTVs[MAX_MRT_COUNT];
	oRef<ID3D11ShaderResourceView> SRVs[MAX_MRT_COUNT];
	oRef<ID3D11Texture2D> Depth;
	oRef<ID3D11DepthStencilView> DSV;
	oRef<ID3D11ShaderResourceView> SRVDepth;

	// Creates the depth buffer according to the Desc.DepthStencilFormat value
	void RecreateDepthBuffer(const uint2& _Dimensions);

	// @oooii-tony: Bootstrapping from home where I have a DX10 card only,
	// so should this DX10 stuff be in here? in an entirely different object?
	// Should it even be allowed? First get anything up...

	// @oooii-tony: Maybe the RT that binds to an oWindow should be a 
	// separate impl obj.

	oRef<ID3D10Texture2D> D3D10Texture;
	oRef<threadsafe oWindow> Window; // do I need this, or just the underlying swap chain?

	oRef<IDXGISwapChain> DXGISwapChain;
	oRWMutex DescMutex;
	DESC Desc;
};

#endif
