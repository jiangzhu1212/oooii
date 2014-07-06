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
#ifndef oGPU_render_target_h
#define oGPU_render_target_h

#include "oGPUCommon.h"

oGPU_NAMESPACE_BEGIN

oDEVICE_CHILD_CLASS(render_target)
{
	oDEVICE_CHILD_DECLARATION(render_target)
	d3d11_render_target(std::shared_ptr<device>& _Device, const char* _Name, IDXGISwapChain* _pSwapChain, surface::format _DepthStencilFormat);
	~d3d11_render_target();
	render_target_info get_info() const override;
	void set_clear_depth_stencil(float _Depth, uchar _Stencil) override;
	void set_clear_color(uint _Index, color _Color) override;
	void resize(const int3& _NewDimensions) override;
	std::shared_ptr<texture1> get_texture(int _MRTIndex) override;
	std::shared_ptr<texture1> get_depth_texture() override;
	std::shared_ptr<surface::buffer> make_snapshot(int _MRTIndex) override;

	inline void set(ID3D11DeviceContext* _pContext) { _pContext->OMSetRenderTargets(Info.num_mrts, (ID3D11RenderTargetView* const*)RTVs.data(), DSV); }

	std::array<std::shared_ptr<texture1>, max_num_mrts> Textures;
	std::array<intrusive_ptr<ID3D11RenderTargetView>, max_num_mrts> RTVs;
	std::shared_ptr<texture1> DepthStencilTexture;
	intrusive_ptr<ID3D11DepthStencilView> DSV;
	intrusive_ptr<IDXGISwapChain> SwapChain;
	render_target_info Info;

	void clear_resources();

	// Creates the depth buffer according to the Desc.DepthStencilFormat value
	void recreate_depth(const int2& _Dimensions);
};

oGPU_NAMESPACE_END

#endif