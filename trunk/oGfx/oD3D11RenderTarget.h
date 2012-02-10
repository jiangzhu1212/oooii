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
#pragma once
#ifndef oD3D11RenderTarget2_h
#define oD3D11RenderTarget2_h

#include "oGfxCommon.h"
#include <oPlatform/oD3D11.h>
#if 0
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
#endif