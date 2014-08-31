// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.
#include <oGPU/texturecube.h>
#include <oGPU/device.h>
#include "d3d_debug.h"
#include "d3d_util.h"
#include "dxgi_util.h"

using namespace ouro::gpu::d3d;

namespace ouro { namespace gpu {

Device* get_device(device& dev);
DeviceContext* get_dc(command_list& cl);

void texturecube::initialize(const char* name, device& dev, surface::format format, uint width, uint height, uint array_size_in_num_cubes, bool mips)
{
	deinitialize();
	
	oCHECK(!surface::is_depth(format), "format %s cannot be a depth format", as_string(format));

	Device* D3DDevice = get_device(dev);
	DXGI_FORMAT fmt = dxgi::from_surface_format(format);

	auto t = make_texture_2d(name, get_device(dev), format, width, height, array_size_in_num_cubes, 0, mips, true);
	auto srv = make_srv(t, format, array_size_in_num_cubes);
	srv->AddRef();
	ro = srv;
}

void texturecube::initialize(const char* name, device& dev, const surface::image& src, bool mips)
{
	auto si = src.get_info();
	oCHECK(si.is_cube(), "a cubemap texel buffer must be specified");
	oCHECK((si.array_size % 6) == 0, "cubemaps require a multiple of 6 for their array_size value (%u specified)", si.array_size);
	oCHECK(!mips || (mips && si.mip_layout != surface::mip_layout::none), "source buffer does not contain mips for mipped texture");

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

void texturecube::deinitialize()
{
	oSAFE_RELEASEV(ro);
}

uint2 texturecube::dimensions() const
{
	intrusive_ptr<Texture2D> t;
	((View*)ro)->GetResource((Resource**)&t);
	D3D11_TEXTURE2D_DESC d;
	t->GetDesc(&d);
	return uint2(d.Width, d.Height);
}

uint texturecube::array_size() const
{
	intrusive_ptr<Texture2D> t;
	((View*)ro)->GetResource((Resource**)&t);
	D3D11_TEXTURE2D_DESC d;
	t->GetDesc(&d);
	return d.ArraySize;
}

}}
