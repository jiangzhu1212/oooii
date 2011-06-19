// $(header)
#include "oD3D11RenderTarget.h"
#include "oD3D11Device.h"
#include <oooii/oD3D11.h>
#include <oooii/oErrno.h>
#include <oooii/oSurface.h>

oDEFINE_GFXRESOURCE_CREATE(RenderTarget)

oBEGIN_DEFINE_GFXAPI_CTOR(D3D11, RenderTarget)
{
	ID3D11Device* D3DDevice = 0;
	oVERIFY(Device->QueryInterface(oGetGUID(ID3D11Device), (void**)&D3DDevice));

	bool oD3D11CreateRenderTarget(D3DDevice, _Desc.Width, Desc.Height, Desc.ArraySize, DXGI_FORMAT _Format, ID3D11Texture2D** _ppRenderTarget, ID3D11View** _ppRenderTargetView, ID3D11ShaderResourceView** _ppShaderResourceView);
	
	*_pSuccess = oD3D11CreateTexture2D(D3DDevice, _Desc.Width, _Desc.Height, _Desc.NumSlices, oSurface::GetPlatformFormat<DXGI_FORMAT>(_Desc.ColorFormat), _Desc.GenerateMips ? oD3D11_MIPPED_RENDER_TARGET : oD3D11_RENDER_TARGET, &Texture, &SRV);	
}
