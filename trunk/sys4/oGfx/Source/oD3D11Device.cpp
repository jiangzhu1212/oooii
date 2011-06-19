// $(header)
#include "SYS3D3D11Device.h"
#include <oooii/oD3D11.h>
#include <oooii/oErrno.h>

template<typename T, typename containerT, class CompareT> size_t oSortedInsert(containerT& _Container, const T& _Item, CompareT _Compare)
{
	containerT::iterator it = _Container.begin();
	for (; it != _Container.end(); ++it)
		if (_Compare(*it, _Item))
			break;
	it = _Container.insert(it, _Item);
	return std::distance(_Container.begin(), it);
}

template<typename T, typename Alloc, class CompareT> size_t oSortedInsert(std::vector<T, Alloc>& _Vector, const T& _Item) { return oSTL::detail::oSortedInsert<T, std::vector<T, Alloc>, CompareT>(_Vector, _Item, _Compare); }

bool ByDrawOrder(const oGfxDeviceContext* _pContext1, const oGfxDeviceContext* _pContext2)
{
	oGfxDeviceContext::DESC d1, d2;
	_pContext1->GetDesc(&d1);
	_pContext2->GetDesc(&d2);
	return d1.DrawOrder < d2.DrawOrder;
};

bool oCreateGPUDevice(const oGfxDevice::DESC& _Desc, threadsafe oGfxDevice** _ppDevice)
{
	oRef<IDXGIFactory1> pFactory;
	oCreateDXGIFactory(&pFactory);
	if (!pFactory)
	{
		oSetLastError(ENOSYS, "Failed to create DXGI factory");
		return false;
	}

	oRef<ID3D11Device> D3DDevice;
	oRef<IDXGIAdapter> pAdapter;
	while (DXGI_ERROR_NOT_FOUND != pFactory->EnumAdapters(actualGPUIndex, &pAdapter))
	{
		D3D_FEATURE_LEVEL FeatureLevel;
		oD3D11::Singleton()->D3D11CreateDevice(
			_Desc.UseSoftwareEmulation ? 0 : pAdapter
			, _Desc.UseSoftwareEmulation ? D3D_DRIVER_TYPE_REFERENCE : D3D_DRIVER_TYPE_UNKNOWN
			, 0
			, _Desc.EnableDebugReporting ? D3D11_CREATE_DEVICE_DEBUG : 0
			, 0
			, 0
			, D3D11_SDK_VERSION
			, &D3DDevice
			, &FeatureLevel
			, 0);

		if (oGetD3DVersion(FeatureLevel) == _Desc.Version)
			break;

		D3DDevice = 0;
	}

	if (!D3DDevice)
	{
		oSetLastError(ENOSYS, "Version %.03f of the GPU driver could not be found", _Desc.Version);
		return false;
	}

	bool success = false;
	oCONSTRUCT(_ppDevice, oD3D11Device(D3DDevice, _Desc, &success));
	return success;
}

oD3D11Device::oD3D11Device(ID3D11Device* _pD3DDevice, const oGfxDevice::DESC& _Desc, bool* _pSuccess)
	: D3DDevice(_pDevice)
	, Desc(_Desc)
	, RasterizerState(_pDevice)
	, BlendState(_pDevice)
	, DepthStencilState(_pDevice)
	, SamplerState(_pDevice)
{
	*_pSuccess = false;
	D3DDevice->GetImmediateContext(&ImmediateContext);

	// TODO: Create shaders/pipeline resources
	// TODO: Create system-wide resources
}

void oD3D11Device::Insert(oGfxContext* _pContext) threadsafe
{
	oMutex::ScopedLock lock(ContextsMutex);
	oSortedInsert(Contexts, _pContext, ByDrawOrder);
}

void oD3D11Device::Remove(oGfxContext* _pContext) threadsafe
{
	oMutex::ScopedLock lock(ContextsMutex);
	oFindAndErase(Contexts, _pContext);
}