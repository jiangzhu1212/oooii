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
#include <oGPU/depth_target.h>
#include "d3d_debug.h"
#include "d3d_util.h"
#include "dxgi_util.h"

using namespace ouro::gpu::d3d;

namespace ouro { namespace gpu {

Device* get_device(device* dev);
DeviceContext* get_dc(command_list* cl);

void depth_target::initialize(const char* name, device* dev, surface::format format, uint width, uint height, uint array_size, bool mips, uint supersampling)
{
	deinitialize();
	oCHECK_ARG(surface::is_depth(format), "format %s is not a depth format", as_string(format));
	
	Device* D3DDevice = get_device(dev);

	DXGI_FORMAT tf, df, sf;
	dxgi::get_compatible_formats(dxgi::from_surface_format(format), &tf, &df, &sf);

	CD3D11_TEXTURE2D_DESC d(tf
		, width
		, height
		, array_size
		, mips ? 0 : 1
		, D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_DEPTH_STENCIL
		, D3D11_USAGE_DEFAULT
		, 0
		, 1
		, 0
		, D3D11_RESOURCE_MISC_GENERATE_MIPS);

	intrusive_ptr<Texture2D> t;
	oV(D3DDevice->CreateTexture2D(&d, nullptr, &t));
	debug_name(t, name);

	ro = make_srv(t, sf, array_size);

	{
		D3D11_DEPTH_STENCIL_VIEW_DESC dd;
		dd.Format = df;

		if (array_size)
		{
			dd.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DARRAY;
			dd.Texture2DArray.MipSlice = 0;
			dd.Texture2DArray.ArraySize = 1;
		
			rws.resize(array_size);
			for (uint i = 0; i < array_size; i++)
			{
				dd.Texture2DArray.FirstArraySlice = i;
				oV(D3DDevice->CreateDepthStencilView(t, &dd, (DepthStencilView**)&rws[i]));
				debug_name((DepthStencilView*)rws[i], name);
			}
		}
	
		else
		{
			dd.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
			dd.Texture2D.MipSlice = 0;
			rws.resize(1);
			oV(D3DDevice->CreateDepthStencilView(t, &dd, (DepthStencilView**)&rws[0]));
			debug_name((DepthStencilView*)rws[0], name);
		}
	}
}

void depth_target::deinitialize()
{
	if (ro)
		((View*)ro)->Release();
	ro = nullptr;
	
	for (auto rw : rws)
		((View*)rw)->Release();
	
	rws.clear();
	rws.shrink_to_fit();
}

char* depth_target::name(char* dst, size_t dst_size) const
{
	return ro ? debug_name(dst, dst_size, (View*)ro) : "uninitialized";
}

surface::format depth_target::format() const
{
	intrusive_ptr<Texture2D> t;
	((View*)ro)->GetResource((Resource**)&t);
	D3D11_TEXTURE2D_DESC d;
	t->GetDesc(&d);
	return dxgi::to_surface_format(d.Format);
}

uint2 depth_target::dimensions() const
{
	intrusive_ptr<Texture2D> t;
	((View*)ro)->GetResource((Resource**)&t);
	D3D11_TEXTURE2D_DESC d;
	t->GetDesc(&d);
	return uint2(d.Width, d.Height);
}

uint depth_target::array_size() const
{
	intrusive_ptr<Texture2D> t;
	((View*)ro)->GetResource((Resource**)&t);
	D3D11_TEXTURE2D_DESC d;
	t->GetDesc(&d);
	return d.ArraySize;
}

void depth_target::set(command_list* cl, uint index)
{
	get_dc(cl)->OMSetRenderTargets(0, nullptr, (DepthStencilView*)get_target(index));
}

void depth_target::clear_depth(command_list* cl, uint index, float depth)
{
	get_dc(cl)->ClearDepthStencilView((DepthStencilView*)rws[index], D3D11_CLEAR_DEPTH, depth, 0);
}

void depth_target::clear_stencil(command_list* cl, uint index, uchar stencil)
{
	get_dc(cl)->ClearDepthStencilView((DepthStencilView*)rws[index], D3D11_CLEAR_STENCIL, 0.0f, stencil);
}

void depth_target::clear_depth_stencil(command_list* cl, uint index, float depth, uchar stencil)
{
	get_dc(cl)->ClearDepthStencilView((DepthStencilView*)rws[index], D3D11_CLEAR_DEPTH|D3D11_CLEAR_STENCIL, depth, stencil);
}

void depth_target::generate_mips(command_list* cl)
{
	get_dc(cl)->GenerateMips((ShaderResourceView*)ro);
}

}}
