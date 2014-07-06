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
#include <oGPU/texture1d.h>
#include <oGPU/oGPU.h>
#include "d3d_debug.h"
#include "d3d_util.h"

using namespace ouro::gpu::d3d;

namespace ouro { namespace gpu {

Device* get_device(device* dev);
DeviceContext* get_dc(command_list* cl);

void texture1d::initialize(const char* name, device* dev, surface::format format, uint width, uint array_size, bool mips)
{
	deinitialize();
	oCHECK_ARG(!surface::is_depth(format), "format %s cannot be a depth format", as_string(format));
	intrusive_ptr<Texture1D> t = make_texture_1d(name, get_device(dev), format, width, array_size, mips);
	ro = make_srv(t, format, array_size);
}

void texture1d::initialize(const char* name, device* dev, const surface::buffer& src, bool mips)
{
	auto si = src.get_info();
	initialize(name, dev, si.format, si.dimensions.x, si.array_size, mips);

	const int NumMips = surface::num_mips(mips, si.dimensions);
	const int nSubresources = surface::num_subresources(si);

	command_list* cl = dev->immediate();
	for (int subresource = 0; subresource < nSubresources; subresource++)
	{
		surface::shared_lock lock(src, subresource);
		update(cl, subresource, lock.mapped);
	}
}

uint texture1d::width() const
{
	intrusive_ptr<Texture1D> t;
	((View*)ro)->GetResource((Resource**)&t);
	D3D11_TEXTURE1D_DESC d;
	t->GetDesc(&d);
	return d.Width;
}

uint texture1d::array_size() const
{
	intrusive_ptr<Texture1D> t;
	((View*)ro)->GetResource((Resource**)&t);
	D3D11_TEXTURE1D_DESC d;
	t->GetDesc(&d);
	return d.ArraySize;
}

void texture1d::update(command_list* cl, uint subresource, const surface::const_mapped_subresource& src, const surface::box& region)
{
	intrusive_ptr<Resource> r;
	((View*)ro)->GetResource(&r);
	update_texture(get_dc(cl), true, r, subresource, src, region);
}

}}
