// $(header)
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
