// $(header)
#include "SYS4D3D11Material.h"
#include "SYS4D3D11Device.h"
#include <oooii/oD3D11.h>
#include <oooii/oErrno.h>

SYS4_DEFINE_GPURESOURCE_CREATE(Material)

oD3D11Material::oD3D11Material(threadsafe oGPUDevice* _pDevice, const DESC& _Desc, const char* _Name, const char* _CacheName, bool* _pSuccess)
	: SYS4ResourceBaseMixin(_pDevice, _Desc, _Name, _CacheName)
{
	ID3D11Device* D3DDevice = 0;
	oVERIFY(Device->QueryInterface(oGetGUID(ID3D11Device), (void**)&D3DDevice));
	*_pSuccess = oD3D11CreateConstantBuffer(_pD3DDevice, TRUE, &_Desc, sizeof(_Desc), 1, &Constants));
}
