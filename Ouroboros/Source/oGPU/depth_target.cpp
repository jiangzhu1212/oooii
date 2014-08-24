/**************************************************************************
 * The MIT License                                                        *
 * Copyright (c) 2014 Antony Arciuolo.                                    *
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

Device* get_device(device& dev);
DeviceContext* get_dc(command_list& cl);

void depth_target::initialize(const char* name, device& dev, surface::format format, uint width, uint height, uint array_size, bool mips, uint supersampling)
{
	internal_initialize(name, get_device(dev), format, width, height, array_size, mips, supersampling);
}

void depth_target::internal_initialize(const char* name, void* dev, surface::format format, uint width, uint height, uint array_size, bool mips, uint supersampling)
{
	deinitialize();

	if (width == 0 || height == 0)
		return;

	oCHECK_ARG(surface::is_depth(format), "format %s is not a depth format", as_string(format));
	
	Device* D3DDevice = (Device*)dev;

	auto t = make_texture_2d(name, D3DDevice, format, width, height, array_size, D3D11_BIND_DEPTH_STENCIL, mips);

	DXGI_FORMAT tf, df, sf;
	dxgi::get_compatible_formats(dxgi::from_surface_format(format), &tf, &df, &sf);

	auto srv = make_srv(t, sf, array_size);
	srv->AddRef();
	ro = srv;

	if (array_size)
	{
		rws.resize(array_size);
		for (uint i = 0; i < array_size; i++)
		{
			auto dsv = make_dsv(t, i);
			dsv->AddRef();
			rws[i] = dsv;
		}
	}
	
	else
	{
		rws.resize(1);
		auto dsv = make_dsv(t);
		dsv->AddRef();
		rws[0] = dsv;
	}
}

void depth_target::deinitialize()
{
	oSAFE_RELEASEV(ro);
	
	for (auto rw : rws)
	{
		oSAFE_RELEASEV(rw);
	}

	rws.clear();
	rws.shrink_to_fit();
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

void depth_target::resize(const uint2& dimensions)
{
	ShaderResourceView* srv = (ShaderResourceView*)ro;

	intrusive_ptr<Resource> r;
	srv->GetResource(&r);
	D3D_TEXTURE_DESC desc = get_texture_desc(r);
	intrusive_ptr<Device> D3DDevice;
	r->GetDevice(&D3DDevice);

	mstring n;
	debug_name(n, r);
	internal_initialize(n, D3DDevice, dxgi::to_surface_format(desc.Format), dimensions.x, dimensions.y, is_array(srv) ? desc.ArraySize : 0, desc.Mips, 0);
}

void depth_target::set(command_list& cl, uint index, const viewport& vp)
{
	set_viewports(cl, dimensions(), &vp, 1);
	get_dc(cl)->OMSetRenderTargets(0, nullptr, (DepthStencilView*)get_target(index));
}

void depth_target::clear(command_list& cl, uint index, float depth)
{
	get_dc(cl)->ClearDepthStencilView((DepthStencilView*)rws[index], D3D11_CLEAR_DEPTH, depth, 0);
}

void depth_target::clear_stencil(command_list& cl, uint index, uchar stencil)
{
	get_dc(cl)->ClearDepthStencilView((DepthStencilView*)rws[index], D3D11_CLEAR_STENCIL, 0.0f, stencil);
}

void depth_target::clear_depth_stencil(command_list& cl, uint index, float depth, uchar stencil)
{
	get_dc(cl)->ClearDepthStencilView((DepthStencilView*)rws[index], D3D11_CLEAR_DEPTH|D3D11_CLEAR_STENCIL, depth, stencil);
}

void depth_target::generate_mips(command_list& cl)
{
	get_dc(cl)->GenerateMips((ShaderResourceView*)ro);
}

}}
