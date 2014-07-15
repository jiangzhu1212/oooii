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
#include <oGPU/color_target.h>
#include <oGPU/depth_target.h>
#include "d3d_debug.h"
#include "d3d_util.h"
#include "dxgi_util.h"

using namespace ouro::gpu::d3d;

namespace ouro { namespace gpu {

Device* get_device(device& dev);
DeviceContext* get_dc(command_list& cl);

void color_target::initialize(const char* name, device& dev, surface::format format, uint width, uint height, uint array_size, bool mips)
{
	deinitialize();
	
	oCHECK_ARG(!surface::is_depth(format), "format %s cannot be a depth format", as_string(format));
	oCHECK_ARG(!surface::is_block_compressed(format), "format %s cannot be a block compressed format", as_string(format));

	Device* D3DDevice = get_device(dev);
	DXGI_FORMAT fmt = dxgi::from_surface_format(format);

	auto t = make_texture_2d(name, get_device(dev), format, width, height, array_size, D3D11_BIND_RENDER_TARGET, mips, false);
	auto srv = make_srv(t, format, array_size);
	srv->AddRef();
	ro = srv;

	{
		D3D11_RENDER_TARGET_VIEW_DESC d;
		d.Format = fmt;

		if (array_size)
		{
			d.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
			d.Texture2DArray.MipSlice = 0;
			d.Texture2DArray.ArraySize = 1;

			rws.resize(array_size);
			for (uint i = 0; i < array_size; i++)
			{
				d.Texture2DArray.FirstArraySlice = i;
				oV(D3DDevice->CreateRenderTargetView(t, &d, (RenderTargetView**)&rws[i]));
				debug_name((RenderTargetView*)rws[i], name);
			}
		}
	
		else
		{
			d.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
			d.Texture2D.MipSlice = 0;
			rws.resize(1);
			oV(D3DDevice->CreateRenderTargetView(t, &d, (RenderTargetView**)&rws[0]));
			debug_name((RenderTargetView*)rws[0], name);
		}
	}
}

void color_target::deinitialize()
{
	oSAFE_RELEASEV(ro);

	for (auto rw : rws)
	{
		oSAFE_RELEASEV(rw);
	}

	rws.clear();
	rws.shrink_to_fit();
}

uint2 color_target::dimensions() const
{
	intrusive_ptr<Texture2D> t;
	((View*)ro)->GetResource((Resource**)&t);
	D3D11_TEXTURE2D_DESC d;
	t->GetDesc(&d);
	return uint2(d.Width, d.Height);
}

uint color_target::array_size() const
{
	intrusive_ptr<Texture2D> t;
	((View*)ro)->GetResource((Resource**)&t);
	D3D11_TEXTURE2D_DESC d;
	t->GetDesc(&d);
	return d.ArraySize;
}

void color_target::set_draw_target(command_list& cl, uint num_colors, color_target* const* colors, depth_target* depth, const viewport& vp)
{
	set_viewports(cl, colors[0]->dimensions(), &vp, 1);

	RenderTargetView* rtvs[D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT];
	for (uint i = 0; i < oCOUNTOF(rtvs); i++)
		rtvs[i] = (RenderTargetView*)colors[i]->get_target();
	get_dc(cl)->OMSetRenderTargets(num_colors, num_colors ? rtvs : nullptr, depth ? (DepthStencilView*)depth->get_target() : nullptr);
}

void color_target::set_draw_target(command_list& cl, uint num_colors, color_target* const* colors, const uint* color_indices, depth_target* depth, uint depth_index, const viewport& vp)
{
	set_viewports(cl, colors[0]->dimensions(), &vp, 1);
	
	RenderTargetView* rtvs[D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT];
	for (uint i = 0; i < oCOUNTOF(rtvs); i++)
		rtvs[i] = (RenderTargetView*)colors[i]->get_target(color_indices[i]);
	get_dc(cl)->OMSetRenderTargets(num_colors, num_colors ? rtvs : nullptr, depth ? (DepthStencilView*)depth->get_target(depth_index) : nullptr);
}

void color_target::set_draw_target(command_list& cl, uint index, depth_target* depth, uint depth_index, const viewport& vp)
{
	set_viewports(cl, dimensions(), &vp, 1);
	get_dc(cl)->OMSetRenderTargets(1, (RenderTargetView* const*)&rws[index], depth ? (DepthStencilView*)depth->get_target(depth_index) : nullptr);
}

void color_target::clear(command_list& cl, const color& c, uint index)
{
	float fcolor[4];
	c.decompose(&fcolor[0], &fcolor[1], &fcolor[2], &fcolor[3]);
	get_dc(cl)->ClearRenderTargetView((RenderTargetView*)rws[index], fcolor);
}

void color_target::generate_mips(command_list& cl)
{
	get_dc(cl)->GenerateMips((ShaderResourceView*)ro);
}

}}
