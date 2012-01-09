// $(header)
#include <oPlatform/oGPU.h>
#include <oBasis/oAssert.h>
#include <oPlatform/oDXGI.h>
#include <oBasis/oRef.h>

bool oGPUEnum(unsigned int _Index, oGPU_DESC* _pDesc)
{
	#if oDXVER >= oDXVER_10
		oRef<IDXGIFactory1> pFactory;
		oDXGICreateFactory(&pFactory);
		if (pFactory)
		{
			IDXGIAdapter* pAdapter = 0;
			if (DXGI_ERROR_NOT_FOUND == pFactory->EnumAdapters(_Index, &pAdapter))
				return false;

			DXGI_ADAPTER_DESC desc;
			pAdapter->GetDesc(&desc);
			oStrConvert(_pDesc->Description, oCOUNTOF(_pDesc->Description), desc.Description);
			_pDesc->VRAM = desc.DedicatedVideoMemory;
			_pDesc->DedicatedSystemMemory = desc.DedicatedSystemMemory;
			_pDesc->SharedSystemMemory = desc.SharedSystemMemory;
			_pDesc->Index = _Index;
			_pDesc->D3DVersion = oDXGIGetD3DVersion(pAdapter);
			pAdapter->Release();
			return true;
		}
	#else
		oTRACE("oGPUEnum not yet implemented on pre-DX10 Windows");
	#endif
	return false;
}
