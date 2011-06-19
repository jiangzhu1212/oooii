// $(header)
#include "oD3D11Mesh.h"
#include "oD3D11Device.h"

oDEFINE_GFXDEVICE_CREATE(oD3D11, Mesh);
oBEGIN_DEFINE_GFXRESOURCE_CTOR(oD3D11, Mesh)
{
	*_pSuccess = false;
	oD3D11DEVICE();

	if (_Desc.NumIndices)
		oVERIFY(oD3D11CreateIndexBuffer(D3DDevice, _Name, true, 0, _Desc.NumIndices, _Desc.NumIndices < 65535, &Indices));

	if (_Desc.NumVertices)
	{
		oVERIFY(oD3D11CreateVertexBuffer(D3DDevice, _Name, true, 0, _Desc.NumVertices, sizeof(VERTEX), &Vertices));
		//oVERIFY(oD3D11CreateVertexBuffer(D3DDevice, _Name, true, 0, _Desc.NumVertices, sizeof(SKINNING), &Skinning));
	}

	*_pSuccess = true;
}
