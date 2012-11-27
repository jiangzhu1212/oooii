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

#include "oGPUCommon.h"
#include <oGPU/oGPUWindow.h>
#include <oPlatform/Windows/oD3D11.h>

oDECLARE_GPUDEVICECHILD_IMPLEMENTATION(oD3D11, RenderTarget)
{
	oDEFINE_GPUDEVICECHILD_INTERFACE();
	oDECLARE_GPUDEVICECHILD_CTOR(oD3D11, RenderTarget);
	oD3D11RenderTarget(oGPUDevice* _pDevice, threadsafe oGPUWindow* _pWindow, oSURFACE_FORMAT _DepthStencilFormat, const char* _Name, bool* _pSuccess);

	void GetDesc(DESC* _pDesc) const threadsafe override;
	void SetClearDesc(const oGPU_CLEAR_DESC& _ClearDesc) threadsafe override;
	void Resize(const int3& _NewDimensions) override;
	void GetTexture(int _MRTIndex, oGPUTexture** _ppTexture) override;
	void GetDepthTexture(oGPUTexture** _ppTexture) override;

	inline void Set(ID3D11DeviceContext* _pContext) { _pContext->OMSetRenderTargets(Desc.MRTCount, (ID3D11RenderTargetView* const*)RTVs, DSV); }

	oRef<oGPUTexture> Textures[oGPU_MAX_NUM_MRTS];
	oRef<oGPUTexture> DepthStencilTexture;
	oRef<ID3D11RenderTargetView> RTVs[oGPU_MAX_NUM_MRTS];
	oRef<ID3D11DepthStencilView> DSV;

	// Retain a reference to the window associated with the swap chain when this 
	// render target is constructed with a window because we'll need to resync/
	// query it constantly for up-to-date DESC information.
	threadsafe oGPUWindow* Window;

	void ResizeLock();
	bool ResizeUnlock();

	// Creates the depth buffer according to the Desc.DepthStencilFormat value
	void RecreateDepthBuffer(const int2& _Dimensions);

	oSharedMutex DescMutex;
	DESC Desc;
};

// Private construction of a render target based on an oGPUWindow. Remember 
// oGPUWindow is responsible for the swap chain, so it's all really based off
// that.
bool oD3D11CreateRenderTarget(oGPUDevice* _pDevice, const char* _Name, threadsafe oGPUWindow* _pWindow, oSURFACE_FORMAT _DepthStencilFormat, oGPURenderTarget** _ppRenderTarget);

#endif