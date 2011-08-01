// $(header)
#include "oD3D11Device.h"
#include "oD3D11CommandList.h"
#include <oooii/oD3D11.h>
#include <oooii/oErrno.h>

const oGUID& oGetGUID(threadsafe const oD3D11Device* threadsafe const *)
{
	// {882CABF3-9344-40BE-B3F9-A17A2F920751}
	static const oGUID oIID_D3D11Device = { 0x882cabf3, 0x9344, 0x40be, { 0xb3, 0xf9, 0xa1, 0x7a, 0x2f, 0x92, 0x7, 0x51 } };
	return oIID_D3D11Device;
}

const oGUID& oGetGUID(threadsafe const ID3D11Device* threadsafe const *)
{
	// {DB6F6DDB-AC77-4E88-8253-819DF9BBF140}
	static const oGUID oIID_ID3D11Device = { 0xdb6f6ddb, 0xac77, 0x4e88, { 0x82, 0x53, 0x81, 0x9d, 0xf9, 0xbb, 0xf1, 0x40 } };
	return oIID_ID3D11Device;
}

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

bool ByDrawOrder(const oGfxCommandList* _pCommandList1, const oGfxCommandList* _pCommandList2)
{
	oGfxCommandList::DESC d1, d2;
	_pCommandList1->GetDesc(&d1);
	_pCommandList2->GetDesc(&d2);
	return d1.DrawOrder < d2.DrawOrder;
};

bool oGfxCreateDevice(const oGfxDevice::DESC& _Desc, threadsafe oGfxDevice** _ppDevice)
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
	UINT index = 0;
	while (DXGI_ERROR_NOT_FOUND != pFactory->EnumAdapters(index++, &pAdapter))
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

		D3DDevice = nullptr;
	}

	if (!D3DDevice)
	{
		oSetLastError(ENOSYS, "Version %.01f of the GPU driver could not be found", _Desc.Version);
		return false;
	}

	bool success = false;
	oCONSTRUCT(_ppDevice, oD3D11Device(D3DDevice, _Desc, &success));
	return success;
}

oD3D11Device::oD3D11Device(ID3D11Device* _pDevice, const oGfxDevice::DESC& _Desc, bool* _pSuccess)
	: D3DDevice(_pDevice)
	, Desc(_Desc)
	, RSState(_pDevice)
	, OMState(_pDevice)
	, DSState(_pDevice)
	, SAState(_pDevice)
{
	*_pSuccess = false;
	D3DDevice->GetImmediateContext(&ImmediateContext);

	oD3D11_CHECK_STRUCT_SIZE(oDeferredViewConstants);
	oVERIFY(oD3D11CreateConstantBuffer(D3DDevice, "oGfxDevice::ViewConstants", true, 0, sizeof(oDeferredViewConstants), 1, &ViewConstants));

	oD3D11_CHECK_STRUCT_SIZE(oGfxDrawConstants);
	oVERIFY(oD3D11CreateConstantBuffer(D3DDevice, "oGfxDevice::DrawConstants", true, 0, sizeof(oGfxDrawConstants), 1, &DrawConstants));

	*_pSuccess = true;
}

bool oD3D11Device::QueryInterface(const oGUID& _InterfaceID, threadsafe void** _ppInterface) threadsafe
{
	*_ppInterface = nullptr;
	if (_InterfaceID == oGetGUID<oD3D11Device>() || _InterfaceID == oGetGUID<oGfxDevice>())
	{
		Reference();
		*_ppInterface = this;
	}

	else if (_InterfaceID == oGetGUID<ID3D11Device>())
	{
		D3DDevice->AddRef();
		*_ppInterface = D3DDevice;
	}

	return !!*_ppInterface;
}

void oD3D11Device::Insert(oGfxCommandList* _pCommandList) threadsafe
{
	oMutex::ScopedLock lock(CommandListsMutex);
	oSortedInsert(ProtectedCommandLists(), _pCommandList, ByDrawOrder);
}

void oD3D11Device::Remove(oGfxCommandList* _pCommandList) threadsafe
{
	oMutex::ScopedLock lock(CommandListsMutex);
	oFindAndErase(ProtectedCommandLists(), _pCommandList);
}

void oD3D11Device::DrawCommandLists() threadsafe
{
	oMutex::ScopedLock lock(CommandListsMutex);

	for (std::vector<oGfxCommandList*>::iterator it = ProtectedCommandLists().begin(); it != ProtectedCommandLists().end(); ++it)
	{
		oD3D11CommandList* c = static_cast<oD3D11CommandList*>(*it);
		ImmediateContext->ExecuteCommandList(c->CommandList, FALSE);
		c->CommandList = 0;
	}
}

void oD3D11Device::Submit()
{
	DrawCommandLists();
}
