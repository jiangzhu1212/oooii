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
#include <oGPU/render_target.h>
#include <oCore/windows/win_util.h>
#include "d3d_debug.h"
#include "d3d_resource.h"
#include "d3d_types.h"
#include "dxgi_util.h"

#include <oGUI/windows/oWinWindowing.h>

#if 0

using namespace ouro::gpu::d3d;

namespace ouro {
	namespace gpu {
		namespace d3d {

#if 0
struct texture
{
	union
	{
		Resource* resource;
		Buffer* buffer;
		Texture1D* texture1d;
		Texture2D* texture2d;
		Texture3D* texture3d;
	};

	union
	{
		View* view;
		ShaderResourceView* srv;
		DepthStencilView* dsv;
		UnorderedAccessView* uav;
	};
};
#endif

		} // namespace d3d

#define oSCD DXGI_SWAP_CHAIN_DESC SCD; oV(((IDXGISwapChain*)swap_chain)->GetDesc(&SCD));

render_target::render_target()
	: depth(nullptr)
	, swap_chain(nullptr)
{
	textures.fill(nullptr);
	targets.fill(nullptr);
}

render_target::render_target(device* dev, const render_target_info& _info, const char* name)
	: depth(nullptr)
	, swap_chain(nullptr)
	, info(_info)
{
	textures.fill(nullptr);
	targets.fill(nullptr);

	for (uint i = 0; i < oCOUNTOF(info.format); i++)
		if (surface::is_yuv(info.format[i]))
			oTHROW_INVARG("YUV render targets are not supported (format %s specified)", as_string(info.format[i]));

	// invalidate width/height to force allocation in this call to resize
	info.dimensions = ushort3(0,0,0);
	resize(_info.dimensions);
	name_resources(name);
}

render_target::render_target(device* dev, void* _swap_chain, const surface::format& depth_stencil_format, const char* name)
	: depth(nullptr)
	, swap_chain(_swap_chain)
{
	textures.fill(nullptr);
	targets.fill(nullptr);

	oSCD
	if (oWinIsRenderTarget(SCD.OutputWindow))
		oTHROW_INVARG("The specified window is already associated with a render target and cannot be reassociated.");
	
	oWinSetIsRenderTarget(SCD.OutputWindow);
	info.depth_stencil_format = depth_stencil_format;
	info.format[0] = dxgi::to_surface_format(SCD.BufferDesc.Format);
	resize(ushort3((ushort)SCD.BufferDesc.Width, (ushort)SCD.BufferDesc.Height, 1));
}

render_target::~render_target()
{
	clear_resources();
	if (swap_chain)
	{
		oSCD
		void release_swap_chain(device* dev);
		release_swap_chain((device*)dev);
		oWinSetIsRenderTarget(SCD.OutputWindow, false);
	}
}

static void rt_debug_name(texture* t, const char* type, int index, const char* name)
{
	mstring n;
	intrusive_ptr<Resource> r;
	((View*)t)->GetResource(&r);
	if (index == -1)
		snprintf(n, "%s.%s", type, name);
	else
		snprintf(n, "%s.%s%d", type, name, index);
	debug_name(r, n);
}

void render_target::name_resources(const char* name)
{
	int i = 0;
	for (texture* t : textures)
		rt_debug_name(t, "texture", i++, name);

	i = 0;
	for (texture* t : targets)
		rt_debug_name(t, "target", i++, name);

	rt_debug_name(depth, "depth", -1, name);
}

void render_target::clear_resources()
{
	for (texture*& t : textures)
		if (t) ((ID3D11View*)t)->Release();
	for (texture*& t : targets)
		if (t) ((ID3D11View*)t)->Release();
	if (depth)
		((ID3D11View*)depth)->Release();
}

render_target_info render_target::get_info() const
{
	render_target_info i(info);
	if (swap_chain)
	{
		oSCD
		i.dimensions = int3(as_int(SCD.BufferDesc.Width), as_int(SCD.BufferDesc.Height), 1);
		i.format[0] = dxgi::to_surface_format(SCD.BufferDesc.Format);
		i.depth_stencil_format = surface::unknown;

		if (depth)
		{
			intrusive_ptr<Resource> r;
			((View*)depth)->GetResource(&r);
			texture_info ti = get_texture_info(r);
			i.depth_stencil_format = ti.format;
		}
	}

	return i;
}

void render_target::set_clear_colors(uint num_clear_colors, const color* clear_colors)
{
}

void render_target::resize(const ushort3& new_dimensions)
{
	ushort3 New = new_dimensions;
	if (swap_chain)
	{
		IDXGISwapChain* sc = (IDXGISwapChain*)swap_chain;

		BOOL IsFullScreen = FALSE;
		intrusive_ptr<IDXGIOutput> Output;
		sc->GetFullscreenState(&IsFullScreen, &Output);
		if (IsFullScreen)
		{
			DXGI_OUTPUT_DESC OD;
			Output->GetDesc(&OD);
			New = ushort3(ushort(OD.DesktopCoordinates.right - OD.DesktopCoordinates.left), ushort(OD.DesktopCoordinates.bottom - OD.DesktopCoordinates.top), 1);
		}
	}

	if (any(info.dimensions != New))
	{
		oTRACE("%s %s Resize %dx%dx%d -> %dx%dx%d", type_name(typeid(*this).name()), name(), Info.dimensions.x, Info.dimensions.y, Info.dimensions.z, _NewDimensions.x, _NewDimensions.y, _NewDimensions.z);
		clear_resources();

		if (New.x && New.y && New.z)
		{
			if (swap_chain)
			{
				if (New.z != 1)
					oTHROW_INVARG("New.z must be set to 1 for primary render target");
				dev->immediate()->clear_render_target_and_unordered_resources();
				
				{
					IDXGISwapChain* sc = (IDXGISwapChain*)swap_chain;
					dxgi::resize_buffers(sc, New.xy());
					intrusive_ptr<ID3D11Texture2D> SwapChainTexture;
					oV(sc->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&SwapChainTexture));
					///Textures[0] = std::make_shared<d3d11_texture>(Device, name(), texture_info(), SwapChainTexture);

					intrusive_ptr<View> RTV = make_rtv(name(), SwapChainTexture);
					RTV->AddRef();
					targets[0] = (texture*)RTV.c_ptr();
				}

				info.array_size = 0;
				info.mrt_count = 1;
				info.type = texture_type::render_target_2d;
			}

			else
			{
				for (int i = 0; i < Info.mrt_count; i++)
				{
					lstring n;
					snprintf(n, "%s%02d", name(), i);
					texture_info ti;
					ti.dimensions = New;
					ti.format = Info.format[i];
					ti.array_size = Info.array_size;
					ti.type = gpu::make_render_target(Info.type);
					Textures[i] = Device->make_texture(n, ti);
					make_rtv(name(), static_cast<d3d11_texture*>(Textures[i].get())->pTexture2D, RTVs[0]);
				}
			}

			recreate_depth(New.xy());
		}
		
		Info.dimensions = New;
	}
}

std::shared_ptr<surface::buffer> render_target::snapshot(int mrt_index)
{
	return nullptr;
}

	} // namespace gpu
} // namespace ouro

#if 0

std::shared_ptr<render_target> make_render_target(std::shared_ptr<device>& _Device, const char* _Name, IDXGISwapChain* _pSwapChain, surface::format _DepthStencilFormat)
{
	if (!_pSwapChain)
		oTHROW_INVARG("A valid swap chain must be specified");
	return std::make_shared<d3d11_render_target>(_Device, _Name, _pSwapChain, _DepthStencilFormat);
}

void d3d11_render_target::set_clear_info(const clear_info& _ClearInfo)
{
	Info.clear = _ClearInfo;
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

std::shared_ptr<texture> d3d11_render_target::get_texture(int _MRTIndex)
{
	oCHECK(_MRTIndex < Info.mrt_count, "Invalid MRT index");
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

#endif
#endif
