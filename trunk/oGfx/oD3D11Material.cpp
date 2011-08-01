// $(header)
#include "oD3D11Material.h"
#include "oD3D11Device.h"

oDEFINE_GFXDEVICE_CREATE(oD3D11, Material);
oBEGIN_DEFINE_GFXRESOURCE_CTOR(oD3D11, Material)
{
	oD3D11DEVICE();
	*_pSuccess = oD3D11CreateConstantBuffer(D3DDevice, _Name, true, 0, _Desc.ByteSize, _Desc.ArraySize, &Constants);
}
