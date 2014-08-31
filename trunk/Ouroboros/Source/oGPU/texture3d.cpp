// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.
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
	oCHECK(!surface::is_depth(format), "format %s cannot be a depth format", as_string(format));
	auto t = make_texture_3d(name, get_device(dev), format, width, height, depth, mips);
	auto srv = make_srv(t, format, 0);
	srv->AddRef();
	ro = srv;
}

void texture3d::initialize(const char* name, device& dev, const surface::image& src, bool mips)
{
	auto si = src.get_info();
	oCHECK(si.is_3d(), "a 3d texel buffer must be specified");
	oCHECK(!mips || (mips && si.mip_layout != surface::mip_layout::none), "source buffer does not contain mips for mipped texture");
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
