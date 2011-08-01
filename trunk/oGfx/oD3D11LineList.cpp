// $(header)
#include "oD3D11LineList.h"
#include "oD3D11Device.h"

oDEFINE_GFXDEVICE_CREATE(oD3D11, LineList);
oBEGIN_DEFINE_GFXRESOURCE_CTOR(oD3D11, LineList)
{
	*_pSuccess = false;
	oD3D11DEVICE();
	oVERIFY(oD3D11CreateVertexBuffer(D3DDevice, Name, true, 0, Desc.MaxNumLines, sizeof(LINE), &Lines));
	Desc.NumLines = 0;
	*_pSuccess = true;
}
