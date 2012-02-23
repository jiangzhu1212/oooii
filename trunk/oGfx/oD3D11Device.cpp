/**************************************************************************
 * The MIT License                                                        *
 * Copyright (c) 2011 Antony Arciuolo & Kevin Myers                       *
 *                                                                        *
 * Permission is hereby granted, free of charge, to any person obtaining  *
 * a copy of this software and associated documentation files (the        *
 * "Software"), to deal in the Software without restriction, including    *
 * without limitation the rights to use, copy, modify, merge, publish,    *
 * distribute, sublicense, and/or sell copies of the Software, and to     *
 * permit persons to whom the Software is furnished to do so, subject to  *
 * the following conditions:                                              *
 *                                                                        *
 * The above copyright notice and this permission notice shall be         *
 * included in all copies or substantial portions of the Software.        *
 *                                                                        *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,        *
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF     *
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND                  *
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE *
 * LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION *
 * OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION  *
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.        *
 **************************************************************************/
#include "oD3D11Device.h"
#include "oD3D11CommandList.h"
#include <oBasis/oFor.h>
#include <oPlatform/oD3D11.h>
#include <oPlatform/oDXGI.h>

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

bool oGfxDeviceCreate(const oGfxDevice::DESC& _Desc, threadsafe oGfxDevice** _ppDevice)
{
	oD3D11_DEVICE_DESC DevDesc("oGfxDevice");
	DevDesc.MinimumAPIFeatureLevel = _Desc.Version;
	DevDesc.Accelerated = !_Desc.UseSoftwareEmulation;
	DevDesc.Debug = _Desc.EnableDebugReporting;

	oRef<ID3D11Device> Device;
	if (!oD3D11CreateDevice(DevDesc, &Device))
		return false; // pass through error

	bool success = false;
	oCONSTRUCT(_ppDevice, oD3D11Device(Device, _Desc, &success));
	return success;
}

template<typename StateT, size_t size> bool StateExists(size_t _Index, StateT (&_States)[size])
{
	for (size_t j = 0; j < _Index; j++)
		if (_States[_Index] == _States[j])
			return true;
	return false;
}

oD3D11Device::oD3D11Device(ID3D11Device* _pDevice, const oGfxDevice::DESC& _Desc, bool* _pSuccess)
	: D3DDevice(_pDevice)
	, Desc(_Desc)
	, DrawID(oInvalid)
	, FrameID(oInvalid)
{
	*_pSuccess = false;

	if (!oD3D11SetDebugName(_pDevice, _Desc.DebugName))
		return; // pass through error

	D3DDevice->GetImmediateContext(&ImmediateContext);

	oStringL StateName;

	// OMStates
	{
		static const D3D11_RENDER_TARGET_BLEND_DESC sBlends[] =
		{
			{ FALSE, D3D11_BLEND_ONE, D3D11_BLEND_ZERO, D3D11_BLEND_OP_ADD, D3D11_BLEND_ONE, D3D11_BLEND_ZERO, D3D11_BLEND_OP_ADD, D3D11_COLOR_WRITE_ENABLE_ALL },
			{ FALSE, D3D11_BLEND_ONE, D3D11_BLEND_ZERO, D3D11_BLEND_OP_ADD, D3D11_BLEND_ONE, D3D11_BLEND_ZERO, D3D11_BLEND_OP_ADD, D3D11_COLOR_WRITE_ENABLE_ALL },
			{ TRUE, D3D11_BLEND_ONE, D3D11_BLEND_ONE, D3D11_BLEND_OP_ADD, D3D11_BLEND_ONE, D3D11_BLEND_ONE, D3D11_BLEND_OP_ADD, D3D11_COLOR_WRITE_ENABLE_ALL },
			{ TRUE, D3D11_BLEND_SRC_ALPHA, D3D11_BLEND_ONE, D3D11_BLEND_OP_ADD, D3D11_BLEND_ZERO, D3D11_BLEND_ZERO, D3D11_BLEND_OP_ADD, D3D11_COLOR_WRITE_ENABLE_ALL },
			{ TRUE, D3D11_BLEND_SRC_ALPHA, D3D11_BLEND_INV_SRC_ALPHA, D3D11_BLEND_OP_ADD, D3D11_BLEND_ZERO, D3D11_BLEND_ZERO, D3D11_BLEND_OP_ADD, D3D11_COLOR_WRITE_ENABLE_ALL },
		};
		static_assert(oCOUNTOF(sBlends) == oOMNUMSTATES, "");

		D3D11_BLEND_DESC desc = {0};
		for (size_t i = 0; i < oCOUNTOF(OMStates); i++)
		{
			desc.AlphaToCoverageEnable = FALSE;
			desc.IndependentBlendEnable = FALSE;
			desc.RenderTarget[0] = sBlends[i];
			oV(_pDevice->CreateBlendState(&desc, &OMStates[i]));

			if (!StateExists(i, OMStates))
			{
				sprintf_s(StateName, "%s.%s", _Desc.DebugName.c_str(), oAsString((oOMSTATE)i));
				oV(oD3D11SetDebugName(OMStates[i], StateName));
			}
		}
	}

	// RSStates
	{
		static const D3D11_FILL_MODE sFills[] = 
		{
			D3D11_FILL_SOLID,
			D3D11_FILL_SOLID,
			D3D11_FILL_SOLID,
			D3D11_FILL_WIREFRAME,
			D3D11_FILL_WIREFRAME,
			D3D11_FILL_WIREFRAME,
			D3D11_FILL_SOLID,
			D3D11_FILL_SOLID,
			D3D11_FILL_SOLID,
		};
		static_assert(oCOUNTOF(sFills) == oRSNUMSTATES, "");

		static const D3D11_CULL_MODE sCulls[] = 
		{
			D3D11_CULL_BACK,
			D3D11_CULL_FRONT,
			D3D11_CULL_NONE,
			D3D11_CULL_BACK,
			D3D11_CULL_FRONT,
			D3D11_CULL_NONE,
			D3D11_CULL_BACK,
			D3D11_CULL_FRONT,
			D3D11_CULL_NONE,
		};
		static_assert(oCOUNTOF(sCulls) == oRSNUMSTATES, "");

		D3D11_RASTERIZER_DESC desc;
		memset(&desc, 0, sizeof(desc));
		desc.FrontCounterClockwise = FALSE;
		desc.DepthClipEnable = TRUE;

		for (size_t i = 0; i < oCOUNTOF(RSStates); i++)
		{
			desc.FillMode = sFills[i];
			desc.CullMode = sCulls[i];
			oV(_pDevice->CreateRasterizerState(&desc, &RSStates[i]));
	
			if (!StateExists(i, RSStates))
			{
				sprintf_s(StateName, "%s.%s", _Desc.DebugName.c_str(), oAsString((oRSSTATE)i));
				oV(oD3D11SetDebugName(RSStates[i], StateName));
			}
		}
	}

	// SAStates
	{
		static const D3D11_FILTER sFilters[] = 
		{
			D3D11_FILTER_MIN_MAG_MIP_POINT,
			D3D11_FILTER_MIN_MAG_MIP_POINT,
			D3D11_FILTER_MIN_MAG_MIP_LINEAR,
			D3D11_FILTER_MIN_MAG_MIP_LINEAR,
			D3D11_FILTER_COMPARISON_ANISOTROPIC,
			D3D11_FILTER_COMPARISON_ANISOTROPIC,
		};
		static_assert(oCOUNTOF(sFilters) == oSANUMSTATES, "");

		static const D3D11_TEXTURE_ADDRESS_MODE sAddresses[] = 
		{
			D3D11_TEXTURE_ADDRESS_CLAMP,
			D3D11_TEXTURE_ADDRESS_WRAP,
			D3D11_TEXTURE_ADDRESS_CLAMP,
			D3D11_TEXTURE_ADDRESS_WRAP,
			D3D11_TEXTURE_ADDRESS_CLAMP,
			D3D11_TEXTURE_ADDRESS_WRAP,
		};
		static_assert(oCOUNTOF(sAddresses) == oSANUMSTATES, "");

		static const FLOAT sBiases[] =
		{
			0.0f,
			-1.0f,
			-2.0f,
			1.0f,
			2.0f,
		};
		static_assert(oCOUNTOF(sBiases) == oMBNUMSTATES, "");

		D3D11_SAMPLER_DESC desc;
		memset(&desc, 0, sizeof(desc));
		for (size_t state = 0; state < oSANUMSTATES; state++)
		{
			desc.Filter = sFilters[state];
			desc.AddressU = desc.AddressV = desc.AddressW = sAddresses[state];
			desc.MaxLOD = FLT_MAX; // documented default
			desc.MaxAnisotropy = 16; // documented default
			desc.ComparisonFunc = D3D11_COMPARISON_NEVER;  // documented default

			for (size_t bias = 0; bias < oMBNUMSTATES; bias++)
			{
				desc.MipLODBias = sBiases[bias];
				_pDevice->CreateSamplerState(&desc, &SAStates[state][bias]);
				sprintf_s(StateName, "%s.%s.%s", _Desc.DebugName.c_str(), oAsString((oSASTATE)state), oAsString((oMBSTATE)bias));
				oV(oD3D11SetDebugName(SAStates[state][bias], StateName));
			}
		}
	}

	oD3D11_CHECK_STRUCT_SIZE(oDeferredViewConstants);
	oVERIFY(oD3D11CreateConstantBuffer(D3DDevice, "oGfxDevice.ViewConstants", true, 0, sizeof(oDeferredViewConstants), 1, &ViewConstants));

	oD3D11_CHECK_STRUCT_SIZE(oGfxDrawConstants);
	oVERIFY(oD3D11CreateConstantBuffer(D3DDevice, "oGfxDevice.DrawConstants", true, 0, sizeof(oGfxDrawConstants), 1, &DrawConstants));

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

	else if (_InterfaceID == (const oGUID&)__uuidof(ID3D11Device))
	{
		D3DDevice->AddRef();
		*_ppInterface = D3DDevice;
	}

	else if (_InterfaceID == (const oGUID&)__uuidof(ID3D11DeviceContext))
	{
		ImmediateContext->AddRef();
		*_ppInterface = ImmediateContext;
	}

	return !!*_ppInterface;
}

void oD3D11Device::CLInsert(oGfxCommandList* _pCommandList) threadsafe
{
	oLockGuard<oMutex> lock(CommandListsInsertRemoveMutex);
	oSortedInsert(ProtectedCommandLists(), _pCommandList, ByDrawOrder);
}

void oD3D11Device::CLRemove(oGfxCommandList* _pCommandList) threadsafe
{
	oLockGuard<oMutex> lock(CommandListsInsertRemoveMutex);
	oFindAndErase(ProtectedCommandLists(), _pCommandList);
}

void oD3D11Device::CLLockSubmit() threadsafe
{
	CommandListsBeginEndMutex.lock_shared();
}

void oD3D11Device::CLUnlockSubmit() threadsafe
{
	CommandListsBeginEndMutex.unlock_shared();
}

void oD3D11Device::DrawCommandLists() threadsafe
{
	oLockGuard<oMutex> lock1(CommandListsInsertRemoveMutex);
	oLockGuard<oSharedMutex> lock2(CommandListsBeginEndMutex);

	oFOR(oGfxCommandList* pGfxCommandList, ProtectedCommandLists())
	{
		oD3D11CommandList* c = static_cast<oD3D11CommandList*>(pGfxCommandList);
		if (c->CommandList)
		{
			ImmediateContext->ExecuteCommandList(c->CommandList, FALSE);
			c->CommandList = nullptr;
		}
	}
}

bool oD3D11Device::BeginFrame() threadsafe
{
	DrawID = 0;
	oStd::atomic_increment(&FrameID);
	return true;
}

void oD3D11Device::EndFrame() threadsafe
{
	DrawCommandLists();
}

uint oD3D11Device::GetFrameID() const threadsafe
{
	return FrameID;
}
