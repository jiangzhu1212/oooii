/**************************************************************************
 * The MIT License                                                        *
 * Copyright (c) 2013 Antony Arciuolo.                                    *
 * arciuolo@gmail.com                                                     *
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
#include "d3d11.h"
#include <oConcurrency/mutex.h>

// {772E2A04-4C2D-447A-8DA8-91F258EFA68C}
oDECLARE_GPUDEVICECHILD_IMPLEMENTATION(oD3D11, RenderTarget, 0x772e2a04, 0x4c2d, 0x447a, 0x8d, 0xa8, 0x91, 0xf2, 0x58, 0xef, 0xa6, 0x8c)
{
	oDEFINE_GPUDEVICECHILD_INTERFACE_EXPLICIT_QI();
	oDECLARE_GPUDEVICECHILD_CTOR(oD3D11, RenderTarget);
	oD3D11RenderTarget(oGPUDevice* _pDevice, IDXGISwapChain* _pSwapChain, ouro::surface::format _DepthStencilFormat, const char* _Name, bool* _pSuccess);
	~oD3D11RenderTarget();

	void GetDesc(DESC* _pDesc) const threadsafe override;
	void SetClearDesc(const ouro::gpu::clear_info& _ClearInfo) threadsafe override;
	void Resize(const int3& _NewDimensions) override;
	void GetTexture(int _MRTIndex, oGPUTexture** _ppTexture) override;
	void GetDepthTexture(oGPUTexture** _ppTexture) override;
	std::shared_ptr<ouro::surface::buffer> CreateSnapshot(int _MRTIndex) override;

	inline void Set(ID3D11DeviceContext* _pContext) { _pContext->OMSetRenderTargets(Desc.mrt_count, (ID3D11RenderTargetView* const*)RTVs.data(), DSV); }

	std::array<ouro::intrusive_ptr<oGPUTexture>, ouro::gpu::max_num_mrts> Textures;
	std::array<ouro::intrusive_ptr<ID3D11RenderTargetView>, ouro::gpu::max_num_mrts> RTVs;
	ouro::intrusive_ptr<oGPUTexture> DepthStencilTexture;
	ouro::intrusive_ptr<ID3D11DepthStencilView> DSV;
	ouro::intrusive_ptr<IDXGISwapChain> SwapChain;

	void ClearResources();

	// Creates the depth buffer according to the Desc.DepthStencilFormat value
	void RecreateDepthBuffer(const int2& _Dimensions);

	oConcurrency::shared_mutex DescMutex;
	DESC Desc;
};

bool oD3D11CreateRenderTarget(oGPUDevice* _pDevice, const char* _Name, IDXGISwapChain* _pSwapChain, ouro::surface::format _DepthStencilFormat, oGPURenderTarget** _ppRenderTarget);

#endif