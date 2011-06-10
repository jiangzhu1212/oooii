// $(header)
#include "SYS3D3D11Mesh.h"
#include "SYS4D3D11Device.h"
#include <oooii/oD3D11.h>
#include <oooii/oErrno.h>

bool oD3D11Device::CreateMesh(const char* _Name, const oGPUMesh::DESC& _Desc, threadsafe oGPUMesh** _ppMesh) threadsafe
{
	if (!_Name || !*_Name)
	{
		oSetLastError(EINVAL, "A proper name must be specified");
		return false;
	}

	if (!_ppMesh)
	{
		oSetLastError(EINVAL, "A valid address for a mesh pointer must be specified");
		return false;
	}

	bool success = false;
	oCONSTRUCT(_ppMesh, oD3D11Mesh(this, _Desc, &success));
	return success;
}

oD3D11Mesh::oD3D11Mesh(threadsafe oGPUDevice* _pDevice, const DESC& _Desc, const char* _Name, const char* _CacheName, bool* _pSuccess)
	: SYS4ResourceBaseMixin(_pDevice, _Desc, _Name, _CacheName)
{
	*_pSuccess = false;
	ID3D11Device* D3DDevice = 0;
	oVERIFY(Device->QueryInterface(oGetGUID(ID3D11Device), (void**)&D3DDevice));

	if (_Desc.NumIndices)
		oVERIFY(oD3D11CreateIndexBuffer(D3DDevice, true, 0, _Desc.NumIndices, _Desc.NumIndices < 65535, &Indices));

	if (_Desc.NumVertices)
	{
		oVERIFY(oD3D11CreateVertexBuffer(D3DDevice, true, 0, _Desc.NumVertices, sizeof(VERTEX), &Vertices));
		oVERIFY(oD3D11CreateVertexBuffer(D3DDevice, true, 0, _Desc.NumVertices, sizeof(SKINNING), &Skinning));
	}

	*_pSuccess = true;
}
