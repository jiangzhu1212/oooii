// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.
#include <oGPU/texture1d.h>
#include <oGPU/device.h>
#include "d3d_debug.h"
#include "d3d_util.h"

using namespace ouro::gpu::d3d;

namespace ouro { namespace gpu {

Device* get_device(device& dev);
DeviceContext* get_dc(command_list& cl);

void texture1d::initialize(const char* name, device& dev, surface::format format, uint width, uint array_size, bool mips)
{
	deinitialize();
	oCHECK(!surface::is_depth(format), "format %s cannot be a depth format", as_string(format));
	auto t = make_texture_1d(name, get_device(dev), format, width, array_size, mips);
	auto srv = make_srv(t, format, array_size);
	srv->AddRef();
	ro = srv;
}

void texture1d::initialize(const char* name, device& dev, const surface::image& src, bool mips)
{
	auto si = src.get_info();
	oCHECK(si.is_1d(), "a 1d texel buffer must be specified");
	oCHECK(!mips || (mips && si.mip_layout != surface::mip_layout::none), "source buffer does not contain mips for mipped texture");
	initialize(name, dev, si.format, si.dimensions.x, si.array_size, mips);

	const int NumMips = surface::num_mips(mips, si.dimensions);
	const int nSubresources = surface::num_subresources(si);

	command_list& cl = dev.immediate();
	for (int subresource = 0; subresource < nSubresources; subresource++)
	{
		surface::shared_lock lock(src, subresource);
		update(cl, subresource, lock.mapped);
	}
}

void texture1d::deinitialize()
{
	oSAFE_RELEASEV(ro);
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

}}
