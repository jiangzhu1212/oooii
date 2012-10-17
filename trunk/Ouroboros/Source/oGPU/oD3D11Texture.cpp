/**************************************************************************
 * The MIT License                                                        *
 * Copyright (c) 2011 Antony Arciuolo & Kevin Myers                       *
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
#include <oBasis/oSurface.h>
#include <oPlatform/Windows/oDXGI.h>

oDEFINE_GPUDEVICE_CREATE(oD3D11, Texture);
oD3D11Texture::oD3D11Texture(oGPUDevice* _pDevice, const DESC& _Desc, const char* _Name, bool* _pSuccess, ID3D11Texture2D* _pTexture)
	: oGPUResourceMixin(_pDevice, _Desc, _Name)
	, Texture(_pTexture)
{
	// NOTE: The desc might be garbage or incorrect if a D3D texture is specified
	// explicitly, so sync them up here.

	if (_pTexture)
	{
		oD3D11GetTextureDesc(_pTexture, &Desc);
		oD3D11SetDebugName(_pTexture, _Name);
		*_pSuccess = true;
	}

	else
	{
		oD3D11DEVICE();
		*_pSuccess = oD3D11CreateTexture(D3DDevice, _Name, _Desc, nullptr, &Texture, &SRV);
	}

	if (!oGPUTextureTypeIsReadback(_Desc.Type))
	{
		if (!SRV && *_pSuccess)
		{
			oStringM name;
			oPrintf(name, "%s.SRV", _Name);
			if (!oD3D11CreateShaderResourceView(name, Texture, &SRV))
				*_pSuccess = false; // pass through error
		}

		if (*_pSuccess && oGPUTextureTypeIsUnordered(_Desc.Type))
		{
			if (!oD3D11CreateUnorderedAccessView(_Name, Texture, 0, 0, &UAV))
				*_pSuccess = false;
		}
	}
}

int2 oD3D11Texture::GetByteDimensions(int _Subresource) const threadsafe
{
	const oGPUTexture::DESC& d = thread_cast<oD3D11Texture*>(this)->Desc;
	oSURFACE_DESC sd;
	sd.Dimensions = d.Dimensions;
	sd.Format = d.Format;
	sd.NumSlices = d.NumSlices;
	sd.Layout = oSURFACE_LAYOUT_IMAGE;
	oSURFACE_SUBRESOURCE_DESC srd;
	return oSurfaceSubresourceCalcByteDimensions(sd, _Subresource);
}
