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
#include <oBasis/oLockThis.h>
#include <oPlatform/Windows/oD3D11.h>
#include <oPlatform/Windows/oDXGI.h>

#include "oD3D11Buffer.h"
#include "oD3D11InstanceList.h"
#include "oD3D11LineList.h"
#include "oD3D11Mesh.h"
#include "oD3D11Pipeline.h"
#include "oD3D11Texture.h"

#include "NoopCSByteCode.h"

const oGUID& oGetGUID(threadsafe const oD3D11Device* threadsafe const *)
{
	// {882CABF3-9344-40BE-B3F9-A17A2F920751}
	static const oGUID oIID_D3D11Device = { 0x882cabf3, 0x9344, 0x40be, { 0xb3, 0xf9, 0xa1, 0x7a, 0x2f, 0x92, 0x7, 0x51 } };
	return oIID_D3D11Device;
}

struct DrawOrderEqual
{
	int DrawOrder;

	DrawOrderEqual(int _DrawOrder)
		: DrawOrder(_DrawOrder)
	{}

	bool operator()(const oGPUCommandList* _pCommandList)
	{
		oGPUCommandList::DESC d;
		_pCommandList->GetDesc(&d);
		return d.DrawOrder == DrawOrder;
	}
};

bool ByDrawOrder(const oGPUCommandList* _pCommandList1, const oGPUCommandList* _pCommandList2)
{
	oGPUCommandList::DESC d1, d2;
	_pCommandList1->GetDesc(&d1);
	_pCommandList2->GetDesc(&d2);
	return d1.DrawOrder > d2.DrawOrder;
};

bool oGPUDeviceCreate(const oGPUDevice::INIT& _Init, oGPUDevice** _ppDevice)
{
	oRef<ID3D11Device> Device;
	if (!oD3D11CreateDevice(_Init, false, &Device))
		return false; // pass through error

	bool success = false;
	oCONSTRUCT(_ppDevice, oD3D11Device(Device, _Init, &success));
	return success;
}

template<typename StateT, size_t size> bool StateExists(size_t _Index, StateT (&_States)[size])
{
	for (size_t j = 0; j < _Index; j++)
		if (_States[_Index] == _States[j])
			return true;
	return false;
}

oD3D11Device::oD3D11Device(ID3D11Device* _pDevice, const oGPUDevice::INIT& _Init, bool* _pSuccess)
	: D3DDevice(_pDevice)
	, FrameID(oInvalid)
	, hHeap(HeapCreate(0, oMB(10), 0))
	, IsSoftwareEmulation(_Init.UseSoftwareEmulation)
{
	*_pSuccess = false;

	if (!oD3D11DeviceGetDesc(_pDevice, IsSoftwareEmulation, &Desc.Initialize()))
		return; // pass through error

	HeapAllocations.reserve(500);

	D3DDevice->GetImmediateContext(&ImmediateContext);
	oStringL StateName;

	// Blend States
	{
		static const D3D11_RENDER_TARGET_BLEND_DESC sBlends[] =
		{
			{ FALSE, D3D11_BLEND_ONE, D3D11_BLEND_ZERO, D3D11_BLEND_OP_ADD, D3D11_BLEND_ONE, D3D11_BLEND_ZERO, D3D11_BLEND_OP_ADD, D3D11_COLOR_WRITE_ENABLE_ALL },
			{ FALSE, D3D11_BLEND_ONE, D3D11_BLEND_ZERO, D3D11_BLEND_OP_ADD, D3D11_BLEND_ONE, D3D11_BLEND_ZERO, D3D11_BLEND_OP_ADD, D3D11_COLOR_WRITE_ENABLE_ALL },
			{ TRUE, D3D11_BLEND_ONE, D3D11_BLEND_ONE, D3D11_BLEND_OP_ADD, D3D11_BLEND_ONE, D3D11_BLEND_ONE, D3D11_BLEND_OP_ADD, D3D11_COLOR_WRITE_ENABLE_ALL },
			{ TRUE, D3D11_BLEND_SRC_ALPHA, D3D11_BLEND_ONE, D3D11_BLEND_OP_ADD, D3D11_BLEND_ZERO, D3D11_BLEND_ZERO, D3D11_BLEND_OP_ADD, D3D11_COLOR_WRITE_ENABLE_ALL },
			{ TRUE, D3D11_BLEND_DEST_COLOR, D3D11_BLEND_ZERO, D3D11_BLEND_OP_ADD, D3D11_BLEND_ZERO, D3D11_BLEND_ZERO, D3D11_BLEND_OP_ADD, D3D11_COLOR_WRITE_ENABLE_ALL },
			{ TRUE, D3D11_BLEND_INV_DEST_COLOR, D3D11_BLEND_ONE, D3D11_BLEND_OP_ADD, D3D11_BLEND_ZERO, D3D11_BLEND_ZERO, D3D11_BLEND_OP_ADD, D3D11_COLOR_WRITE_ENABLE_ALL },
			{ TRUE, D3D11_BLEND_SRC_ALPHA, D3D11_BLEND_INV_SRC_ALPHA, D3D11_BLEND_OP_ADD, D3D11_BLEND_ZERO, D3D11_BLEND_ZERO, D3D11_BLEND_OP_ADD, D3D11_COLOR_WRITE_ENABLE_ALL },
			{ TRUE, D3D11_BLEND_ONE, D3D11_BLEND_ONE, D3D11_BLEND_OP_MIN, D3D11_BLEND_ONE, D3D11_BLEND_ONE, D3D11_BLEND_OP_MIN, D3D11_COLOR_WRITE_ENABLE_ALL },
			{ TRUE, D3D11_BLEND_ONE, D3D11_BLEND_ONE, D3D11_BLEND_OP_MAX, D3D11_BLEND_ONE, D3D11_BLEND_ONE, D3D11_BLEND_OP_MAX, D3D11_COLOR_WRITE_ENABLE_ALL },
		};
		static_assert(oCOUNTOF(sBlends) == oGPU_BLEND_NUM_STATES, "# blend states mismatch");

		D3D11_BLEND_DESC desc = {0};
		oFORI(i, BlendStates)
		{
			desc.AlphaToCoverageEnable = FALSE;
			desc.IndependentBlendEnable = FALSE;

			for (uint j = 0; j < oCOUNTOF(desc.RenderTarget); j++)
				desc.RenderTarget[j] = sBlends[i];

			oV(_pDevice->CreateBlendState(&desc, &BlendStates[i]));

			if (!StateExists(i, BlendStates))
			{
				oPrintf(StateName, "%s.%s", _Init.DebugName.c_str(), oAsString((oGPU_BLEND_STATE)i));
				oV(oD3D11SetDebugName(BlendStates[i], StateName));
			}
		}
	}

	// Depth States
	{
		static const D3D11_DEPTH_STENCIL_DESC sDepthStencils[] = 
		{
			{ FALSE, D3D11_DEPTH_WRITE_MASK_ALL, D3D11_COMPARISON_ALWAYS, FALSE, 0xFF, 0XFF, D3D11_STENCIL_OP_KEEP, D3D11_STENCIL_OP_KEEP, D3D11_STENCIL_OP_KEEP, D3D11_COMPARISON_ALWAYS, D3D11_STENCIL_OP_KEEP, D3D11_STENCIL_OP_KEEP, D3D11_STENCIL_OP_KEEP, D3D11_COMPARISON_ALWAYS },
			{ TRUE, D3D11_DEPTH_WRITE_MASK_ALL, D3D11_COMPARISON_LESS, FALSE, 0xFF, 0XFF, D3D11_STENCIL_OP_KEEP, D3D11_STENCIL_OP_KEEP, D3D11_STENCIL_OP_KEEP, D3D11_COMPARISON_ALWAYS, D3D11_STENCIL_OP_KEEP, D3D11_STENCIL_OP_KEEP, D3D11_STENCIL_OP_KEEP, D3D11_COMPARISON_ALWAYS },
			{ TRUE, D3D11_DEPTH_WRITE_MASK_ZERO, D3D11_COMPARISON_LESS, FALSE, 0xFF, 0XFF, D3D11_STENCIL_OP_KEEP, D3D11_STENCIL_OP_KEEP, D3D11_STENCIL_OP_KEEP, D3D11_COMPARISON_ALWAYS, D3D11_STENCIL_OP_KEEP, D3D11_STENCIL_OP_KEEP, D3D11_STENCIL_OP_KEEP, D3D11_COMPARISON_ALWAYS },
		};
		oFORI(i, DepthStencilStates)
		{
			oV(_pDevice->CreateDepthStencilState(&sDepthStencils[i], &DepthStencilStates[i]));
			if (!StateExists(i, DepthStencilStates))
			{
				oPrintf(StateName, "%s.%s", _Init.DebugName.c_str(), oAsString((oGPU_DEPTH_STENCIL_STATE)i));
				oV(oD3D11SetDebugName(DepthStencilStates[i], StateName));
			}
		}
	}
	// Surface States
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
		static_assert(oCOUNTOF(sFills) == oGPU_SURFACE_NUM_STATES, "# surface states mismatch");

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
		static_assert(oCOUNTOF(sCulls) == oGPU_SURFACE_NUM_STATES, "# surface states mismatch");

		D3D11_RASTERIZER_DESC desc;
		memset(&desc, 0, sizeof(desc));
		desc.FrontCounterClockwise = FALSE;
		desc.DepthClipEnable = TRUE;

		for (size_t i = 0; i < oCOUNTOF(SurfaceStates); i++)
		{
			desc.FillMode = sFills[i];
			desc.CullMode = sCulls[i];
			oV(_pDevice->CreateRasterizerState(&desc, &SurfaceStates[i]));
	
			if (!StateExists(i, SurfaceStates))
			{
				oPrintf(StateName, "%s.%s", _Init.DebugName.c_str(), oAsString((oGPU_SURFACE_STATE)i));
				oV(oD3D11SetDebugName(SurfaceStates[i], StateName));
			}
		}
	}

	// Sampler States
	{
		static const int NUM_ADDRESS_STATES = 6;
		static const int NUM_MIP_BIAS_LEVELS = oGPU_SAMPLER_NUM_STATES / NUM_ADDRESS_STATES;

		static const D3D11_FILTER sFilters[] = 
		{
			D3D11_FILTER_MIN_MAG_MIP_POINT,
			D3D11_FILTER_MIN_MAG_MIP_POINT,
			D3D11_FILTER_MIN_MAG_MIP_LINEAR,
			D3D11_FILTER_MIN_MAG_MIP_LINEAR,
			D3D11_FILTER_COMPARISON_ANISOTROPIC,
			D3D11_FILTER_COMPARISON_ANISOTROPIC,
		};
		static_assert(oCOUNTOF(sFilters) == NUM_ADDRESS_STATES, "# sampler states mismatch");

		static const D3D11_TEXTURE_ADDRESS_MODE sAddresses[] = 
		{
			D3D11_TEXTURE_ADDRESS_CLAMP,
			D3D11_TEXTURE_ADDRESS_WRAP,
			D3D11_TEXTURE_ADDRESS_CLAMP,
			D3D11_TEXTURE_ADDRESS_WRAP,
			D3D11_TEXTURE_ADDRESS_CLAMP,
			D3D11_TEXTURE_ADDRESS_WRAP,
		};
		static_assert(oCOUNTOF(sAddresses) == NUM_ADDRESS_STATES, "# sampler states mismatch");

		static const FLOAT sBiases[] =
		{
			0.0f,
			-1.0f,
			-2.0f,
			1.0f,
			2.0f,
		};
		static_assert(oCOUNTOF(sBiases) == NUM_MIP_BIAS_LEVELS, "# mip bias levels mismatch");

		D3D11_SAMPLER_DESC desc;
		memset(&desc, 0, sizeof(desc));
		oStringM StateName;

		for (size_t bias = 0, i = 0; bias < NUM_MIP_BIAS_LEVELS; bias++)
		{
			desc.MipLODBias = sBiases[bias];
			for (size_t state = 0; state < NUM_ADDRESS_STATES; state++)
			{
				desc.Filter = sFilters[state];
				desc.AddressU = desc.AddressV = desc.AddressW = sAddresses[state];
				desc.MaxLOD = FLT_MAX; // documented default
				desc.MaxAnisotropy = 16; // documented default
				desc.ComparisonFunc = D3D11_COMPARISON_NEVER;  // documented default

				oPrintf(StateName, "%s.%s", _Init.DebugName.c_str(), oAsString(oGPU_SAMPLER_STATE(NUM_ADDRESS_STATES * bias + state)));
				oV(_pDevice->CreateSamplerState(&desc, &SamplerStates[i]));
				oV(oD3D11SetDebugName(SamplerStates[i], StateName));
				i++;
			}
		}
	}
	
	// Set up some null buffers that will be used to reset parts of the API that
	// do no easily transition between compute and rasterization pipelines.
	{
		// ends up being all set to oInvalid/-1/D3D11_KEEP_UNORDERED_ACCESS_VIEWS 
		// which means leave value alone
		memset(NoopUAVInitialCounts, 0xff, sizeof(NoopUAVInitialCounts));

		// set to an array of nulls
		memset(NullUAVs, 0, sizeof(NullUAVs));
		memset(NullRTVs, 0, sizeof(NullRTVs));
	}

	// Set up a noop compute shader to flush for SetCounter()
	{
		oV(D3DDevice->CreateComputeShader(NoopCSByteCode, oHLSLGetByteCodeSize(NoopCSByteCode), nullptr, &NoopCS));

		oStringS CSName;
		oD3D11GetDebugName(CSName, _pDevice);
		oStrAppendf(CSName, "NoopCS");
		oVERIFY(oD3D11SetDebugName(NoopCS, CSName));
	}

	*_pSuccess = true;
}

oD3D11Device::~oD3D11Device()
{
	if (hHeap)
		HeapDestroy(hHeap);
}

bool oD3D11Device::QueryInterface(const oGUID& _InterfaceID, threadsafe void** _ppInterface) threadsafe
{
	*_ppInterface = nullptr;
	if (_InterfaceID == oGetGUID<oD3D11Device>() || _InterfaceID == oGetGUID<oGPUDevice>())
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

void oD3D11Device::GetDesc(DESC* _pDesc) const threadsafe
{
	*_pDesc = *Desc;
}

const char* oD3D11Device::GetName() const threadsafe
{
	return Desc->DebugName;
}

uint oD3D11Device::GetFrameID() const threadsafe
{
	return FrameID;
}

void oD3D11Device::GetImmediateCommandList(oGPUCommandList** _ppCommandList)
{
	oGPUCommandList::DESC CLDesc;
	CLDesc.DrawOrder = oInvalid;
	oVERIFY(oD3D11Device::CreateCommandList("Immediate", CLDesc, _ppCommandList));
}

bool oD3D11Device::CLInsert(oGPUCommandList* _pCommandList) threadsafe
{
	auto pThis = oLockThis(CommandListsInsertRemoveMutex);

	oGPUCommandList::DESC d;
	_pCommandList->GetDesc(&d);
	auto it = oStdFindIf(pThis->CommandLists, DrawOrderEqual(d.DrawOrder));

	if (it == pThis->CommandLists.end())
	{
		oSortedInsert(pThis->CommandLists, _pCommandList, ByDrawOrder);
		return true;
	}

	return oErrorSetLast(oERROR_REDUNDANT, "Entry with DrawOrder %d already exists", d.DrawOrder);
}

void oD3D11Device::MEMReserve(ID3D11DeviceContext* _pDeviceContext, oGPUResource* _pResource, int _Subresource, oSURFACE_MAPPED_SUBRESOURCE* _pMappedSubresource) threadsafe
{
	int2 ByteDimensions = _pResource->GetByteDimensions(_Subresource);
	size_t size = ByteDimensions.x * ByteDimensions.y;
	HeapLock(hHeap);
	void* p = HeapAlloc(hHeap, 0, size);
	thread_cast<oD3D11Device*>(this)->HeapAllocations.push_back(p); // safe because vector is protected with HeapLock
	HeapUnlock(hHeap);
	_pMappedSubresource->pData = p;
	_pMappedSubresource->RowPitch = ByteDimensions.x;
	_pMappedSubresource->DepthPitch = oUInt(size);
}

void oD3D11Device::MEMCommit(ID3D11DeviceContext* _pDeviceContext, oGPUResource* _pResource, int _Subresource, oSURFACE_MAPPED_SUBRESOURCE& _Source, const oGPU_BOX& _Subregion) threadsafe
{
	int D3DSubresource = oInvalid;
	oGPU_RESOURCE_TYPE type = _pResource->GetType();

	// @oooii-tony: This two-time call to HeapLock is a bit lame, but to hide the 
	// detail differences of map/unmap & updateSubresource we need to ensure that 
	// a reserved source is always fully updated, but really we want this heap 
	// lock to be very limited.
	#ifdef _DEBUG
		HeapLock(hHeap);
		bool IsReserveMemory = thread_cast<oD3D11Device*>(this)->HeapAllocations.end() != oStdFind(thread_cast<oD3D11Device*>(this)->HeapAllocations, _Source.pData);
		HeapUnlock(hHeap);

		// If user memory, subregions are allowed. In either case, line and instance 
		// lists are special since their _Subregion is used to describe the count of 
		// valid items AFTER update, not what to actually update.
		oASSERT(!IsReserveMemory || _Subregion.Left == _Subregion.Right || type == oGPU_LINE_LIST || type == oGPU_INSTANCE_LIST, "Attempting to commit memory that had been reserved. Reserve() occurs on whole subresources, no subregions of a subresource so proceeding will commit not only the subregion, but the potentially uninitialized area of the entire subresource.");
	#endif

	switch (type)
	{
		case oGPU_MESH: 
		{
			bool FallThroughToDefaultCase = true;

			switch (_Subresource)
			{
				case oGPU_MESH_RANGES:
				{
					// @oooii-tony: This is a hack assert. Someone should implement code 
					// properly such that this assert can be removed.
					oASSERT(_Subregion.Left == _Subregion.Right, "Non-empty subregion not yet supported");

					oD3D11Mesh* m = static_cast<oD3D11Mesh*>(_pResource);
					memcpy(oGetData(m->Ranges), _Source.pData, oGetDataSize(m->Ranges));
					FallThroughToDefaultCase = false;
					break;
				}

				case oGPU_MESH_INDICES:
				{
					oD3D11Mesh* m = static_cast<oD3D11Mesh*>(_pResource);
					ID3D11Resource* pD3DResource = oD3D11GetSubresource(_pResource, _Subresource, &D3DSubresource);

					oD3D11_BUFFER_TOPOLOGY BT;
					oD3D11GetBufferTopology(pD3DResource, &BT);
					oASSERT(BT.ElementStride == _Source.RowPitch, "The specified RowPitch %u does not match the oGPU_MESH_INDICES expected ElementStride of %u. If there are less than 65535 vertices, the stride/pitch should be 2 and the source data an array of ushorts, else the stride/pitch should be 4 and the source data an array of uints.", _Source.RowPitch, BT.ElementStride);
					break;
				}

				default:
					break;
			}

			if (!FallThroughToDefaultCase)
				break;
		}
		
		default:
		{
			ID3D11Resource* pD3DResource = oD3D11GetSubresource(_pResource, _Subresource, &D3DSubresource);
			if (_Subregion.Left == _Subregion.Right || type == oGPU_LINE_LIST || type == oGPU_INSTANCE_LIST)
			{
				oD3D11UpdateSubresource(_pDeviceContext, pD3DResource, D3DSubresource, nullptr, _Source.pData, _Source.RowPitch, _Source.DepthPitch);
			}
			else if (_Subregion.Left != _Subregion.Right && _Subregion.Top != _Subregion.Bottom && _Subregion.Front != _Subregion.Back)
			{
				uint StructureByteStride = 1;
				if (type == oGPU_BUFFER)
				{
					D3D11_BUFFER_DESC d;
					static_cast<ID3D11Buffer*>(pD3DResource)->GetDesc(&d);
					StructureByteStride = __max(1, d.StructureByteStride);
					oASSERT(_Subregion.Top == 0 && _Subregion.Bottom == 1, "");
				}

				D3D11_BOX box;
				box.left = _Subregion.Left * StructureByteStride;
				box.top = _Subregion.Top;
				box.right = _Subregion.Right * StructureByteStride;
				box.bottom = _Subregion.Bottom; 
				box.front = _Subregion.Front;
				box.back = _Subregion.Back;
				oD3D11UpdateSubresource(_pDeviceContext, pD3DResource, D3DSubresource, &box, _Source.pData, _Source.RowPitch, _Source.DepthPitch);
			}
			break;
		}
	}

	switch (type)
	{
		case oGPU_INSTANCE_LIST:
		{
			uint NewCount = 0;
			if (_Subregion.Right)
			{
				oASSERT(_Subregion.Left == 0 && _Subregion.Top == 0 && _Subregion.Front == 0, "_Subregion Left,Top,Front must be 0 for oGPUInstanceLists.");
				oASSERT(_Subregion.Bottom == 1 && _Subregion.Back == 1, "_Subregion Bottom,Back must be 1 for oGPUInstanceLists.");
				NewCount = _Subregion.Right;
			}

			static_cast<oD3D11InstanceList*>(_pResource)->SetNumInstances(NewCount);
			break;
		}

		case oGPU_LINE_LIST:
		{
			uint NewCount = 0;
			if (_Subregion.Right)
			{
				oASSERT(_Subregion.Left == 0 && _Subregion.Top == 0 && _Subregion.Front == 0, "_Subregion Left,Top,Front must be 0 for oGPULineLists.");
				oASSERT(_Subregion.Bottom == 1 && _Subregion.Back == 1, "_Subregion Bottom,Back must be 1 for oGPULineLists.");
				NewCount = _Subregion.Right;
			}
			static_cast<oD3D11LineList*>(_pResource)->SetNumLines(NewCount);
			break;
		}

		default: break;
	}

	HeapLock(hHeap);
	if (oFindAndErase(thread_cast<oD3D11Device*>(this)->HeapAllocations, _Source.pData)) // safe because vector is protected with HeapLock
		HeapFree(hHeap, 0, _Source.pData);
	HeapUnlock(hHeap);
}

void oD3D11Device::CLRemove(oGPUCommandList* _pCommandList) threadsafe
{
	auto pThis = oLockThis(CommandListsInsertRemoveMutex);
	oFindAndErase(pThis->CommandLists, _pCommandList);
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
	auto pThis = oLockThis(CommandListsInsertRemoveMutex);
	oLockGuard<oSharedMutex> lock2(CommandListsBeginEndMutex);

	oFOR(oGPUCommandList* pGPUCommandList, pThis->CommandLists)
	{
		oD3D11CommandList* c = static_cast<oD3D11CommandList*>(pGPUCommandList);
		if (c->CommandList)
		{
			ImmediateContext->ExecuteCommandList(c->CommandList, FALSE);
			c->CommandList = nullptr;
		}
	}
}

bool oD3D11Device::MapRead(oGPUResource* _pReadbackResource, int _Subresource, oSURFACE_MAPPED_SUBRESOURCE* _pMappedSubresource, bool _bBlocking)
{
	int D3DSubresourceIndex = 0;
	ID3D11Resource* r = oD3D11GetSubresource(_pReadbackResource, _Subresource, &D3DSubresourceIndex);

	D3D11_MAPPED_SUBRESOURCE msr;
	HRESULT hr = ImmediateContext->Map(r, D3DSubresourceIndex, D3D11_MAP_READ, _bBlocking ? 0 : D3D11_MAP_FLAG_DO_NOT_WAIT, &msr);
	if (!_bBlocking && hr == DXGI_ERROR_WAS_STILL_DRAWING)
		return oErrorSetLast(oERROR_BLOCKING);
	if (FAILED(hr))
		return oWinSetLastError();
	_pMappedSubresource->pData = msr.pData;
	_pMappedSubresource->RowPitch = msr.RowPitch;
	_pMappedSubresource->DepthPitch = msr.DepthPitch;
	return true;
}

void oD3D11Device::UnmapRead(oGPUResource* _pReadbackResource, int _Subresource)
{
	int D3DSubresourceIndex = 0;
	ID3D11Resource* r = oD3D11GetSubresource(_pReadbackResource, _Subresource, &D3DSubresourceIndex);
	ImmediateContext->Unmap(r, D3DSubresourceIndex);
}

bool oD3D11Device::BeginFrame()
{
	oStd::atomic_increment(&FrameID);
	return true;
}

void oD3D11Device::EndFrame()
{
	DrawCommandLists();
}
