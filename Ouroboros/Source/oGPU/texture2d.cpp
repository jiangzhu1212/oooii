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
#include <oGPU/texture2d.h>
#include <oGPU/device.h>
#include "d3d_debug.h"
#include "d3d_util.h"
#include "dxgi_util.h"

using namespace ouro::gpu::d3d;

namespace ouro { namespace gpu {

Device* get_device(device& dev);
DeviceContext* get_dc(command_list& cl);

void texture2d::initialize(const char* name, device& dev, surface::format format, uint width, uint height, uint array_size, bool mips)
{
	deinitialize();
	oCHECK_ARG(!surface::is_depth(format), "format %s cannot be a depth format", as_string(format));
	auto t = make_texture_2d(name, get_device(dev), format, width, height, array_size, 0, mips);
	auto srv = make_srv(t, format, array_size);
	srv->AddRef();
	ro = srv;
}

void texture2d::initialize(const char* name, device& dev, const surface::texel_buffer& src, bool mips)
{
	auto si = src.get_info();

	oCHECK(!mips || (mips && si.layout != surface::layout::image), "source buffer does not contain mips for mipped texture");

	initialize(name, dev, si.format, si.dimensions.x, si.dimensions.y, si.array_size, mips);

	const int NumMips = surface::num_mips(mips, si.dimensions);
	const int nSubresources = surface::num_subresources(si);

	command_list& cl = dev.immediate();
	for (int subresource = 0; subresource < nSubresources; subresource++)
	{
		surface::shared_lock lock(src, subresource);
		update(cl, subresource, lock.mapped);
	}
}

void texture2d::deinitialize()
{
	oSAFE_RELEASEV(ro);
}

uint2 texture2d::dimensions() const
{
	intrusive_ptr<Texture2D> t;
	((View*)ro)->GetResource((Resource**)&t);
	D3D11_TEXTURE2D_DESC d;
	t->GetDesc(&d);
	return uint2(d.Width, d.Height);
}

uint texture2d::array_size() const
{
	intrusive_ptr<Texture2D> t;
	((View*)ro)->GetResource((Resource**)&t);
	D3D11_TEXTURE2D_DESC d;
	t->GetDesc(&d);
	return d.ArraySize;
}

}}
