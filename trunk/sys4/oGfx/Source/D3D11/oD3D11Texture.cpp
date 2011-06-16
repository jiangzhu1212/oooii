// $(header)
#include "SYS3D3D11Texture.h"
#include "SYS4D3D11Device.h"
#include <oooii/oD3D11.h>
#include <oooii/oErrno.h>
#include <oooii/oSurface.h>

#if 0

SYS4_DEFINE_GPURESOURCE_CREATE(Texture)

oD3D11Texture::oD3D11Texture(threadsafe oGPUDevice* _pDevice, const DESC& _Desc, const char* _Name, const char* _CacheName, bool* _pSuccess)
	: SYS4ResourceBaseMixin(_pDevice, _Desc, _Name, _CacheName)
{
	ID3D11Device* D3DDevice = 0;
	oVERIFY(Device->QueryInterface(oGetGUID(ID3D11Device), (void**)&D3DDevice));
	
	switch (_Desc.Type)
	{
		case TEXTURE2D:
			*_pSuccess = oD3D11CreateTexture2D(D3DDevice, _Desc.Width, _Desc.Height, _Desc.NumSlices, oSurface::GetPlatformFormat<DXGI_FORMAT>(_Desc.ColorFormat), oD3D11_MIPPED_TEXTURE, &Texture, &SRV);	
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
#endif