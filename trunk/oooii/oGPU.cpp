// $(header)
#include <oooii/oGPU.h>
#include <oooii/oStddef.h>
#include <oooii/oString.h>
#include <oooii/oWindows.h>
#include <oooii/oRef.h>

bool oGPU::GetDesc(unsigned int _GPUIndex, oGPU::DESC* _pDesc)
{
	#if oDXVER >= oDXVER_10
	oRef<IDXGIFactory1> pFactory;
	oCreateDXGIFactory(&pFactory);
	if (pFactory)
	{
		IDXGIAdapter* pAdapter = 0;
		if (DXGI_ERROR_NOT_FOUND == pFactory->EnumAdapters(_GPUIndex, &pAdapter))
			return false;

		DXGI_ADAPTER_DESC desc;
		pAdapter->GetDesc(&desc);
		oStrConvert(_pDesc->Description, oCOUNTOF(_pDesc->Description), desc.Description);
		_pDesc->VRAM = desc.DedicatedVideoMemory;
		_pDesc->DedicatedSystemMemory = desc.DedicatedSystemMemory;
		_pDesc->SharedSystemMemory = desc.SharedSystemMemory;
		_pDesc->Index = _GPUIndex;
		_pDesc->D3DVersion = oDXGIGetD3DVersion(pAdapter);
		pAdapter->Release();
		return true;
	}
	#else
		oTRACE("oGPU::GetDesc not yet implemented on pre-DX10 Windows");
	#endif
	return false;
}
