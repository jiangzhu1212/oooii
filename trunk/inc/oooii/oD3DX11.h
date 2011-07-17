/**************************************************************************
 * The MIT License                                                        *
 * Copyright (c) 2011 Antony Arciuolo & Kevin Myers                       *
 *                                                                        *
 * Permission is hereby granted, free of charge, to any person obtaining  *
 * a copy of this software and associated documentation files (the        *
 * "Software"), to deal in the Software without restriction, including    *
 * without limitation the rights to use, copy, modify, merge, publish,    *
 * distribute, sublicense, and/or sell copies of the Software, and to     *
 * permit persons to whom the Software is furnished to do so, subject to  *
 * the following conditions:                                              *
 *                                                                        *
 * The above copyright notice and this permission notice shall be         *
 * included in all copies or substantial portions of the Software.        *
 *                                                                        *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,        *
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF     *
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND                  *
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE *
 * LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION *
 * OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION  *
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.        *
 **************************************************************************/

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
