// $(header)
#include "SYS3D3D11RenderTarget.h"
#include "SYS4D3D11Device.h"
#include <oooii/oD3D11.h>
#include <oooii/oErrno.h>
#include <oooii/oSurface.h>

SYS4_DEFINE_GPURESOURCE_CREATE(RenderTarget)

oD3D11RenderTarget::oD3D11RenderTarget(threadsafe oGPUDevice* _pDevice, const DESC& _Desc, const char* _Name, const char* _CacheName, bool* _pSuccess)
	: SYS4ResourceBaseMixin(_pDevice, _Desc, _Name, _CacheName)
{
	ID3D11Device* D3DDevice = 0;
	oVERIFY(Device->QueryInterface(oGetGUID(ID3D11Device), (void**)&D3DDevice));

	bool oD3D11CreateRenderTarget(D3DDevice, _Desc.Width, Desc.Height, Desc.ArraySize, DXGI_FORMAT _Format, ID3D11Texture2D** _ppRenderTarget, ID3D11View** _ppRenderTargetView, ID3D11ShaderResourceView** _ppShaderResourceView);
	
	*_pSuccess = oD3D11CreateTexture2D(D3DDevice, _Desc.Width, _Desc.Height, _Desc.NumSlices, oSurface::GetPlatformFormat<DXGI_FORMAT>(_Desc.ColorFormat), _Desc.GenerateMips ? oD3D11_MIPPED_RENDER_TARGET : oD3D11_RENDER_TARGET, &Texture, &SRV);	
}
