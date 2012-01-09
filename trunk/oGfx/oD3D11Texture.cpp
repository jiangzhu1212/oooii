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
#include <oooii/oSurface.h>

oDEFINE_GFXDEVICE_CREATE(oD3D11, Texture);
oBEGIN_DEFINE_GFXRESOURCE_CTOR(oD3D11, Texture)
{
	oD3D11DEVICE();
	
	switch (_Desc.Type)
	{
		case TEXTURE2D:
			*_pSuccess = oD3D11CreateTexture2D(D3DDevice, _Name, _Desc.Width, _Desc.Height, _Desc.NumSlices, oSurface::GetPlatformFormat<DXGI_FORMAT>(_Desc.ColorFormat), oD3D11_MIPPED_TEXTURE, &Texture, &SRV);	
			break;

		case TEXTURE3D:
			oSetLastError(ENOSYS, "TEXTURE3D not supported");
			*_pSuccess = false;
			break;

		case CUBEMAP:
			oSetLastError(ENOSYS, "CUBEMAP not supported");
			*_pSuccess = false;
			break;

		default: oASSUME(0);
	}
}
