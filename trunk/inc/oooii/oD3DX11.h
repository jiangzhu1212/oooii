// $(header)

// Soft-link D3DX11
#pragma once
#ifndef oD3DX11_h
#define oD3DX11_h

#include <oooii/oModule.h>
#include <oooii/oSingleton.h>

// _____________________________________________________________________________
// Soft-link

struct oD3DX11 : oModuleSingleton<oD3DX11>
{
	oD3DX11();
	~oD3DX11();

	HRESULT (__stdcall *D3DX11CreateTextureFromMemory)(ID3D11Device *pDevice, LPCVOID pSrcData, SIZE_T SrcDataSize, D3DX11_IMAGE_LOAD_INFO *pLoadInfo, ID3DX11ThreadPump *pPump, ID3D11Resource **ppTexture, HRESULT *pHResult);
	HRESULT (__stdcall *D3DX11LoadTextureFromTexture)(ID3D11DeviceContext *pContext, ID3D11Resource *pSrcTexture, D3DX11_TEXTURE_LOAD_INFO *pLoadInfo, ID3D11Resource *pDstTexture);

protected:
	oHMODULE hD3DX11;
};

#endif // oD3DX11_h
