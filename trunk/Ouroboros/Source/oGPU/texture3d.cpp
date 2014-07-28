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
#include <oGPU/texture3d.h>
#include <oGPU/device.h>
#include "d3d_debug.h"
#include "d3d_util.h"
#include "dxgi_util.h"

using namespace ouro::gpu::d3d;

namespace ouro { namespace gpu {

Device* get_device(device& dev);
DeviceContext* get_dc(command_list& cl);

void texture3d::initialize(const char* name, device& dev, surface::format format, uint width, uint height, uint depth, bool mips)
{
	deinitialize();
	oCHECK_ARG(!surface::is_depth(format), "format %s cannot be a depth format", as_string(format));
	auto t = make_texture_3d(name, get_device(dev), format, width, height, depth, mips);
	auto srv = make_srv(t, format, 0);
	srv->AddRef();
	ro = srv;
}

void texture3d::initialize(const char* name, device& dev, const surface::texel_buffer& src, bool mips)
{
	auto si = src.get_info();
	initialize(name, dev, si.format, si.dimensions.x, si.dimensions.y, si.dimensions.z, mips);

	const int NumMips = surface::num_mips(mips, si.dimensions);
	const int nSubresources = surface::num_subresources(si);

	command_list& cl = dev.immediate();
	for (int subresource = 0; subresource < nSubresources; subresource++)
	{
		auto sri = surface::subresource(si, subresource);
		surface::box region;
		region.right = sri.dimensions.x;
		region.bottom = sri.dimensions.y;
		surface::shared_lock lock(src, subresource);
		for (uint slice = 0; slice < sri.dimensions.z; slice++)
		{
			region.front = slice;
			region.back = slice + 1;
			update(cl, subresource, lock.mapped, region);
		}
	}
}

void texture3d::deinitialize()
{
	oSAFE_RELEASEV(ro);
}

uint3 texture3d::dimensions() const
{
	intrusive_ptr<Texture3D> t;
	((View*)ro)->GetResource((Resource**)&t);
	D3D11_TEXTURE3D_DESC d;
	t->GetDesc(&d);
	return uint3(d.Width, d.Height, d.Depth);
}

}}
