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
#include "oD3D11RenderTarget.h"
#include "oD3D11Device.h"
#include "oD3D11Texture.h"
#include "dxgi_util.h"

#include <oGUI/Windows/oWinWindowing.h>

oGPU_NAMESPACE_BEGIN

std::shared_ptr<render_target> make_render_target(std::shared_ptr<device>& _Device, const char* _Name, IDXGISwapChain* _pSwapChain, surface::format _DepthStencilFormat)
{
	if (!_pSwapChain)
		oTHROW_INVARG("A valid swap chain must be specified");
	return std::make_shared<d3d11_render_target>(_Device, _Name, _pSwapChain, _DepthStencilFormat);
}

oDEFINE_DEVICE_MAKE(render_target)
oDEVICE_CHILD_CTOR(render_target)
	, Info(_Info)
{
	for (uint i = 0; i < oCOUNTOF(Info.format); i++)
		if (surface::is_yuv(Info.format[i]))
			oTHROW_INVARG("YUV render targets are not supported (format %s specified)", as_string(Info.format[i]));

	// invalidate width/height to force allocation in this call to resize
	Info.dimensions = ushort3(0,0,0);
	resize(_Info.dimensions);
}

d3d11_render_target::d3d11_render_target(std::shared_ptr<device>& _Device, const char* _Name, IDXGISwapChain* _pSwapChain, surface::format _DepthStencilFormat)
	: device_child_mixin(_Device, _Name)
	, SwapChain(_pSwapChain)
{
	DXGI_SWAP_CHAIN_DESC SCD;
	SwapChain->GetDesc(&SCD);

	if (oWinIsRenderTarget(SCD.OutputWindow))
		oTHROW_INVARG("The specified window is already associated with a render target and cannot be reassociated.");
	
	oWinSetIsRenderTarget(SCD.OutputWindow);
	Info.depth_stencil_format = _DepthStencilFormat;
	Info.format[0] = dxgi::to_surface_format(SCD.BufferDesc.Format);
	resize(int3(SCD.BufferDesc.Width, SCD.BufferDesc.Height, 1));
}

d3d11_render_target::~d3d11_render_target()
{
	if (SwapChain)
	{
		static_cast<d3d11_device*>(Device.get())->release_swap_chain();
		DXGI_SWAP_CHAIN_DESC SCD;
		SwapChain->GetDesc(&SCD);
		oWinSetIsRenderTarget(SCD.OutputWindow, false);
	}
}

render_target_info d3d11_render_target::get_info() const
{
	render_target_info i(Info);
	d3d11_render_target* pThis = const_cast<d3d11_render_target*>(this);
	if (SwapChain)
	{
		DXGI_SWAP_CHAIN_DESC SCD;
		oV(pThis->SwapChain->GetDesc(&SCD));
		i.dimensions = int3(as_int(SCD.BufferDesc.Width), as_int(SCD.BufferDesc.Height), 1);
		i.format[0] = dxgi::to_surface_format(SCD.BufferDesc.Format);
		i.depth_stencil_format = DepthStencilTexture ? pThis->DepthStencilTexture->get_info().format : surface::unknown;
	}

	return i;
}

void d3d11_render_target::set_clear_depth_stencil(float _Depth, uchar _Stencil)
{
	Info.depth_clear_value = _Depth;
	Info.stencil_clear_value = _Stencil;
}

void d3d11_render_target::set_clear_color(uint _Index, color _Color)
{
	Info.clear_color[_Index] = _Color;
}

void d3d11_render_target::clear_resources()
{
	RTVs.fill(nullptr);
	Textures.fill(nullptr);
	DepthStencilTexture = nullptr;
	DSV = nullptr;
}

void d3d11_render_target::recreate_depth(const int2& _Dimensions)
{
	if (Info.depth_stencil_format != surface::unknown)
	{
		lstring n;
		snprintf(n, "%s.DS",  name());
		oD3D11_DEVICE();
		DepthStencilTexture = nullptr;
		DSV = nullptr;

		texture_info i;
		i.dimensions = ushort3(_Dimensions, 1);
		i.array_size = 0;
		i.format = Info.depth_stencil_format;
		i.type = texture_type::render_target_2d;
		new_texture New = make_texture(D3DDevice, n, i, nullptr);
		intrusive_ptr<ID3D11Texture2D> Depth = New.pTexture2D;
		DSV = New.pDSV;
		DepthStencilTexture = std::make_shared<d3d11_texture>(Device, name(), texture_info(), Depth);
	}
}

void d3d11_render_target::resize(const int3& _NewDimensions)
{
	int3 New = _NewDimensions;
	if (SwapChain)
	{
		BOOL IsFullScreen = FALSE;
		intrusive_ptr<IDXGIOutput> Output;
		SwapChain->GetFullscreenState(&IsFullScreen, &Output);
		if (IsFullScreen)
		{
			DXGI_OUTPUT_DESC OD;
			Output->GetDesc(&OD);
			New = int3(OD.DesktopCoordinates.right - OD.DesktopCoordinates.left, OD.DesktopCoordinates.bottom - OD.DesktopCoordinates.top, 1);
		}
	}

	if (any(Info.dimensions != New))
	{
		oTRACE("%s %s Resize %dx%dx%d -> %dx%dx%d", type_name(typeid(*this).name()), name(), Info.dimensions.x, Info.dimensions.y, Info.dimensions.z, _NewDimensions.x, _NewDimensions.y, _NewDimensions.z);
		clear_resources();

		if (New.x && New.y && New.z)
		{
			if (SwapChain)
			{
				oASSERT(New.z == 1, "New.z must be set to 1 for primary render target");
				Device->immediate()->clear_render_target_and_unordered_resources();
				dxgi::resize_buffers(SwapChain, New.xy());
				intrusive_ptr<ID3D11Texture2D> SwapChainTexture;
				oV(SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&SwapChainTexture));
				bool textureSuccess = false;
				Textures[0] = std::make_shared<d3d11_texture>(Device, name(), texture_info(), SwapChainTexture);
				make_rtv(name(), SwapChainTexture, RTVs[0]);
				Info.array_size = 0;
				Info.num_mrts = 1;
			}

			else
			{
				for (int i = 0; i < Info.num_mrts; i++)
				{
					lstring n;
					snprintf(n, "%s%02d", name(), i);
					texture_info ti;
					ti.dimensions = New;
					ti.format = Info.format[i];
					ti.array_size = Info.array_size;
					ti.type = Info.has_mips ? texture_type::mipped_render_target_2d : texture_type::render_target_2d;
					Textures[i] = Device->make_texture(n, ti);
					make_rtv(name(), static_cast<d3d11_texture*>(Textures[i].get())->pTexture2D, RTVs[0]);
				}
			}

			recreate_depth(New.xy());
		}
		
		Info.dimensions = New;
	}
}

std::shared_ptr<texture> d3d11_render_target::get_texture(int _MRTIndex)
{
	oCHECK(_MRTIndex < Info.num_mrts, "Invalid MRT index");
	return Textures[_MRTIndex];
}

std::shared_ptr<texture> d3d11_render_target::get_depth_texture()
{
	return DepthStencilTexture;
}

std::shared_ptr<surface::buffer> d3d11_render_target::make_snapshot(int _MRTIndex)
{
	if (!Textures[_MRTIndex])		
		oTHROW(resource_unavailable_try_again, "The render target is minimized or not available for snapshot.");
	return d3d11::make_snapshot(static_cast<d3d11_texture*>(Textures[_MRTIndex].get())->pTexture2D);
}

oGPU_NAMESPACE_END