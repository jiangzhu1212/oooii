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
#include "oD3D11Texture.h"
#include "oD3D11Device.h"
#include <oSurface/surface.h>
#include "dxgi_util.h"
#include "d3d_debug.h"
#include "d3d_resource.h"

using namespace ouro::gpu::d3d;

oGPU_NAMESPACE_BEGIN

 std::shared_ptr<texture1> make_second_texture(std::shared_ptr<device>& _Device, const char* _Texture1Name, const texture1_info& _Texture1Info)
{
	oCHECK(surface::num_subformats(_Texture1Info.format) <= 2, "Many-plane textures not supported");
	if (surface::num_subformats(_Texture1Info.format) == 2)
	{
		// To keep YUV textures singular to prepare for new YUV-based DXGI formats
		// coming, create a private data companion texture.
		texture1_info Texture2Info(_Texture1Info);
		Texture2Info.format = surface::subformat(_Texture1Info.format, 1);
		Texture2Info.dimensions = surface::dimensions_npot(_Texture1Info.format, _Texture1Info.dimensions, 0, 1);

		mstring Texture2Name(_Texture1Name);
		sncatf(Texture2Name, ".Texture2");

		return _Device->make_texture1(Texture2Name, Texture2Info);
	}

	return nullptr;
}

oDEFINE_DEVICE_MAKE(texture1)
d3d11_texture1::d3d11_texture1(std::shared_ptr<device>& _Device, const char* _Name, const texture1_info& _Info, ID3D11Resource* _pTexture)
	: resource_mixin(_Device, _Name, _Info)
	, pResource(_pTexture)
{
	// NOTE: The desc might be garbage or incorrect if a D3D texture is specified
	// explicitly, so sync them up here.
	if (pResource)
	{
		pResource->AddRef();

		Info = get_texture_info1(pResource);

		if (!is_2d(Info.type))
			oTHROW_INVARG("the specified texture must be 2D");
		debug_name(pResource, _Name);
	}

	else
	{
		oD3D11_DEVICE();
		new_texture New = make_texture(D3DDevice, _Name, _Info);
		pResource = New.pResource;
		pResource->AddRef();
		SRV = New.pSRV;
	}

	if (!is_readback(_Info.type))
	{
		if (!SRV)
		{
			mstring name;
			snprintf(name, "%s.SRV", _Name);
			SRV = make_srv(name, pResource);
		}

		if (is_unordered(_Info.type))
			UAV = make_uav(_Name, pResource, 0, 0);
	}

	Texture2 = make_second_texture(_Device, _Name, _Info);
}

d3d11_texture1::~d3d11_texture1()
{
	if (pResource)
		pResource->Release();
}

uint2 d3d11_texture1::byte_dimensions(int _Subresource) const
{
	int numMips = surface::num_mips(is_mipped(Info.type), Info.dimensions); 
	int mipLevel, sliceIndex, surfaceIndex;
	surface::unpack_subresource(_Subresource, numMips, Info.array_size, &mipLevel, &sliceIndex, &surfaceIndex);
	if (surfaceIndex > 0)
		return Texture2->byte_dimensions(surface::calc_subresource(mipLevel, sliceIndex, surfaceIndex - 1, numMips, Info.array_size));

	surface::info i;
	i.dimensions = Info.dimensions;
	i.format = Info.format;
	i.array_size = Info.array_size;
	i.layout = is_mipped(Info.type) ? surface::tight : surface::image;
	return surface::byte_dimensions(i, _Subresource);
}

oGPU_NAMESPACE_END