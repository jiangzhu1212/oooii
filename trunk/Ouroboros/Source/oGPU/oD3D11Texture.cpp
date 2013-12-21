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

using namespace ouro;
using namespace ouro::d3d11;

static bool CreateSecondTexture(oGPUDevice* _pDevice, const char* _Texture1Name, const ouro::gpu::texture_info& _Texture1Desc, oGPUTexture** _ppTexture2)
{
	oASSERT(ouro::surface::num_subformats(_Texture1Desc.format) <= 2, "Many-plane textures not supported");
	if (ouro::surface::num_subformats(_Texture1Desc.format) == 2)
	{
		// To keep YUV textures singular to prepare for new YUV-based DXGI formats
		// coming, create a private data companion texture.
		ouro::gpu::texture_info Texture2Desc(_Texture1Desc);
		Texture2Desc.format = ouro::surface::subformat(_Texture1Desc.format, 1);
		Texture2Desc.dimensions = ouro::surface::dimensions_npot(_Texture1Desc.format, _Texture1Desc.dimensions, 0, 1);

		mstring Texture2Name(_Texture1Name);
		sncatf(Texture2Name, ".Texture2");
		
		return _pDevice->CreateTexture(Texture2Name, Texture2Desc, _ppTexture2);
	}

	return true;
}

oDEFINE_GPUDEVICE_CREATE(oD3D11, Texture);
oD3D11Texture::oD3D11Texture(oGPUDevice* _pDevice, const DESC& _Desc, const char* _Name, bool* _pSuccess, ID3D11Resource* _pTexture)
	: oGPUResourceMixin(_pDevice, _Desc, _Name)
	, Texture((ID3D11Texture2D*)_pTexture)
{
	// NOTE: The desc might be garbage or incorrect if a D3D texture is specified
	// explicitly, so sync them up here.

	if (_pTexture)
	{
		Desc = get_texture_info(_pTexture);

		if (!gpu::is_2d(Desc.type))
		{
			oErrorSetLast(std::errc::invalid_argument, "the specified texture must be 2D");
			return;
		}

		debug_name(_pTexture, _Name);
		*_pSuccess = true;
	}

	else
	{
		oD3D11DEVICE();
		new_texture New = make_texture(D3DDevice, _Name, _Desc);
		Texture = New.pTexture2D;
		SRV = New.pSRV;
		*_pSuccess = true;
	}

	if (*_pSuccess)
	{
		if (!gpu::is_readback(_Desc.type))
		{
			if (!SRV && *_pSuccess)
			{
				mstring name;
				snprintf(name, "%s.SRV", _Name);
				SRV = make_srv(name, Texture);
			}

			if (*_pSuccess && gpu::is_unordered(_Desc.type))
				UAV = make_uav(_Name, Texture, 0, 0);
		}

		*_pSuccess = CreateSecondTexture(_pDevice, _Name, _Desc, (oGPUTexture**)&Texture2);
	}
}

int2 oD3D11Texture::GetByteDimensions(int _Subresource) const threadsafe
{
	const oGPUTexture::DESC& d = thread_cast<oD3D11Texture*>(this)->Desc;
	int numMips = ouro::surface::num_mips(gpu::is_mipped(d.type), d.dimensions); 
	int mipLevel, sliceIndex, surfaceIndex;
	ouro::surface::unpack_subresource(_Subresource, numMips, d.array_size, &mipLevel, &sliceIndex, &surfaceIndex);
	if (surfaceIndex > 0)
		return Texture2->GetByteDimensions(ouro::surface::calc_subresource(mipLevel, sliceIndex, surfaceIndex - 1, numMips, d.array_size));

	ouro::surface::info inf;
	inf.dimensions = d.dimensions;
	inf.format = d.format;
	inf.array_size = d.array_size;
	inf.layout = gpu::is_mipped(d.type) ? ouro::surface::tight : ouro::surface::image;
	return ouro::surface::byte_dimensions(inf, _Subresource);
}
