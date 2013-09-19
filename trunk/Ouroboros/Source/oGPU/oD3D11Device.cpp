/**************************************************************************
 * The MIT License                                                        *
 * Copyright (c) 2013 Antony Arciuolo.                                    *
 * arciuolo@gmail.com                                                     *
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
#include <oStd/for.h>
#include <oBasis/oLockThis.h>
#include <oPlatform/Windows/oD3D11.h>
#include <oPlatform/Windows/oDXGI.h>
#include <oPlatform/Windows/oWinWindowing.h>

#include "oD3D11Buffer.h"
#include "oD3D11Pipeline.h"
#include "oD3D11Texture.h"
#include "oD3D11Query.h"

#include "NoopCSByteCode.h"

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

bool oGPUDeviceCreate(const oGPU_DEVICE_INIT& _Init, oGPUDevice** _ppDevice)
{
	oStd::intrusive_ptr<ID3D11Device> Device;
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
	oStd::lstring StateName;

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
		static_assert(oCOUNTOF(sBlends) == oGPU_BLEND_STATE_COUNT, "# blend states mismatch");

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
				snprintf(StateName, "%s.%s", _Init.DebugName.c_str(), oStd::as_string((oGPU_BLEND_STATE)i));
				oV(oD3D11SetDebugName(BlendStates[i], StateName));
			}
		}
	}

	// Depth States
	{
		static const D3D11_DEPTH_STENCIL_DESC sDepthStencils[] = 
		{
			{ FALSE, D3D11_DEPTH_WRITE_MASK_ALL, D3D11_COMPARISON_ALWAYS, FALSE, 0xFF, 0XFF, D3D11_STENCIL_OP_KEEP, D3D11_STENCIL_OP_KEEP, D3D11_STENCIL_OP_KEEP, D3D11_COMPARISON_ALWAYS, D3D11_STENCIL_OP_KEEP, D3D11_STENCIL_OP_KEEP, D3D11_STENCIL_OP_KEEP, D3D11_COMPARISON_ALWAYS },
			{ TRUE, D3D11_DEPTH_WRITE_MASK_ALL, D3D11_COMPARISON_LESS_EQUAL, FALSE, 0xFF, 0XFF, D3D11_STENCIL_OP_KEEP, D3D11_STENCIL_OP_KEEP, D3D11_STENCIL_OP_KEEP, D3D11_COMPARISON_ALWAYS, D3D11_STENCIL_OP_KEEP, D3D11_STENCIL_OP_KEEP, D3D11_STENCIL_OP_KEEP, D3D11_COMPARISON_ALWAYS },
			{ TRUE, D3D11_DEPTH_WRITE_MASK_ZERO, D3D11_COMPARISON_LESS_EQUAL, FALSE, 0xFF, 0XFF, D3D11_STENCIL_OP_KEEP, D3D11_STENCIL_OP_KEEP, D3D11_STENCIL_OP_KEEP, D3D11_COMPARISON_ALWAYS, D3D11_STENCIL_OP_KEEP, D3D11_STENCIL_OP_KEEP, D3D11_STENCIL_OP_KEEP, D3D11_COMPARISON_ALWAYS },
		};
		oFORI(i, DepthStencilStates)
		{
			oV(_pDevice->CreateDepthStencilState(&sDepthStencils[i], &DepthStencilStates[i]));
			if (!StateExists(i, DepthStencilStates))
			{
				snprintf(StateName, "%s.%s", _Init.DebugName.c_str(), oStd::as_string((oGPU_DEPTH_STENCIL_STATE)i));
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
		};
		static_assert(oCOUNTOF(sFills) == oGPU_SURFACE_STATE_COUNT, "# surface states mismatch");

		static const D3D11_CULL_MODE sCulls[] = 
		{
			D3D11_CULL_BACK,
			D3D11_CULL_FRONT,
			D3D11_CULL_NONE,
			D3D11_CULL_BACK,
			D3D11_CULL_FRONT,
			D3D11_CULL_NONE,
		};
		static_assert(oCOUNTOF(sCulls) == oGPU_SURFACE_STATE_COUNT, "# surface states mismatch");

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
				snprintf(StateName, "%s.%s", _Init.DebugName.c_str(), oStd::as_string((oGPU_SURFACE_STATE)i));
				oV(oD3D11SetDebugName(SurfaceStates[i], StateName));
			}
		}
	}

	// Sampler States
	{
		static const int NUM_ADDRESS_STATES = 6;
		static const int NUM_MIP_BIAS_LEVELS = oGPU_SAMPLER_STATE_COUNT / NUM_ADDRESS_STATES;

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
		oStd::mstring StateName;

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

				snprintf(StateName, "%s.%s", _Init.DebugName.c_str(), oStd::as_string(oGPU_SAMPLER_STATE(NUM_ADDRESS_STATES * bias + state)));
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
		oV(D3DDevice->CreateComputeShader(NoopCSByteCode, oD3D11GetHLSLByteCodeSize(NoopCSByteCode), nullptr, &NoopCS));

		oStd::sstring CSName;
		oD3D11GetDebugName(CSName, _pDevice);
		oStd::sncatf(CSName, "NoopCS");
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
	if (_InterfaceID == oGUID_oD3D11Device || _InterfaceID == oGUID_oGPUDevice)
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

	else if (_InterfaceID == (const oGUID&)__uuidof(IDXGISwapChain))
	{
		oConcurrency::lock_guard<oConcurrency::shared_mutex> lock(SwapChainMutex);
		SwapChain->AddRef();
		*_ppInterface = SwapChain;
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

bool oD3D11Device::CreatePrimaryRenderTarget(oWindow* _pWindow, oSURFACE_FORMAT _DepthStencilFormat, bool _EnableOSRendering, oGPURenderTarget** _ppPrimaryRenderTarget)
{
	if (SwapChain)
		return oErrorSetLast(std::errc::protocol_error, "There already exists a primary render target, only one can exist for a given device at a time.");

	oGUI_WINDOW_SHAPE_DESC s = _pWindow->GetShape();
	if (oGUIStyleHasStatusBar(s.Style))
		return oErrorSetLast(std::errc::invalid_argument, "A window used for rendering must not have a status bar");

	if (!oDXGICreateSwapChain(D3DDevice
		, false
		, __max(1, s.ClientSize.x)
		, __max(1, s.ClientSize.y)
		, false
		, DXGI_FORMAT_B8G8R8A8_UNORM
		, 0
		, 0
		, (HWND)_pWindow->GetNativeHandle()
		, _EnableOSRendering
		, &SwapChain))
		return false; // pass through error

	oStd::sstring RTName;
	snprintf(RTName, "%s.PrimaryRT", GetName());
	if (!oD3D11CreateRenderTarget(this, RTName, SwapChain, _DepthStencilFormat, _ppPrimaryRenderTarget))
		return false;
	return true;
}

bool oD3D11Device::CLInsert(oGPUCommandList* _pCommandList) threadsafe
{
	auto pThis = oLockThis(CommandListsInsertRemoveMutex);

	oGPUCommandList::DESC d;
	_pCommandList->GetDesc(&d);
	auto it = oStd::find_if(pThis->CommandLists, DrawOrderEqual(d.DrawOrder));

	if (it == pThis->CommandLists.end())
	{
		oStd::sorted_insert(pThis->CommandLists, _pCommandList, ByDrawOrder);
		return true;
	}

	return oErrorSetLast(std::errc::operation_in_progress, "Entry with DrawOrder %d already exists", d.DrawOrder);
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

void oD3D11Device::MEMCommit(ID3D11DeviceContext* _pDeviceContext, oGPUResource* _pResource, int _Subresource, const oSURFACE_MAPPED_SUBRESOURCE& _Source, const oSURFACE_BOX& _Subregion) threadsafe
{
	int D3DSubresource = oInvalid;
	oGPU_RESOURCE_TYPE type = _pResource->GetType();

	oASSERT(type != oGPU_MESH, "Do not use GPU mesh directly, use its buffers");
	ID3D11Resource* pD3DResource = oD3D11GetSubresource(_pResource, _Subresource, &D3DSubresource);

	D3D11_BOX box;
	D3D11_BOX* pBox = nullptr;

	if (!oSurfaceBoxIsEmpty(_Subregion))
	{
		uint StructureByteStride = 1;
		if (type == oGPU_BUFFER)
		{
			oGPU_BUFFER_DESC d;
			oD3D11BufferGetDesc(static_cast<ID3D11Buffer*>(pD3DResource), &d);
			StructureByteStride = __max(1, d.StructByteSize);
			oASSERT(_Subregion.Top == 0 && _Subregion.Bottom == 1, "Buffer subregion must have Top == 0 and Bottom == 1");
		}

		box.left = _Subregion.Left * StructureByteStride;
		box.top = _Subregion.Top;
		box.right = _Subregion.Right * StructureByteStride;
		box.bottom = _Subregion.Bottom; 
		box.front = _Subregion.Front;
		box.back = _Subregion.Back;
		pBox = &box;
	}

	oD3D11UpdateSubresource(_pDeviceContext, pD3DResource, D3DSubresource, pBox, _Source.pData, _Source.RowPitch, _Source.DepthPitch);
	HeapLock(hHeap);
	if (oStd::find_and_erase(thread_cast<oD3D11Device*>(this)->HeapAllocations, _Source.pData)) // safe because vector is protected with HeapLock
		HeapFree(hHeap, 0, _Source.pData);
	HeapUnlock(hHeap);
}

void oD3D11Device::CLRemove(oGPUCommandList* _pCommandList) threadsafe
{
	auto pThis = oLockThis(CommandListsInsertRemoveMutex);
	oStd::find_and_erase(pThis->CommandLists, _pCommandList);
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
	oConcurrency::lock_guard<oConcurrency::shared_mutex> lock2(CommandListsBeginEndMutex);

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

void oD3D11Device::RTReleaseSwapChain() threadsafe
{
	oConcurrency::lock_guard<oConcurrency::shared_mutex> lock(SwapChainMutex);
	SwapChain = nullptr;
}

bool oD3D11Device::MapRead(oGPUResource* _pReadbackResource, int _Subresource, oSURFACE_MAPPED_SUBRESOURCE* _pMappedSubresource, bool _bBlocking)
{
	int D3DSubresourceIndex = 0;
	ID3D11Resource* r = oD3D11GetSubresource(_pReadbackResource, _Subresource, &D3DSubresourceIndex);

	D3D11_MAPPED_SUBRESOURCE msr;
	HRESULT hr = ImmediateContext->Map(r, D3DSubresourceIndex, D3D11_MAP_READ, _bBlocking ? 0 : D3D11_MAP_FLAG_DO_NOT_WAIT, &msr);
	if (!_bBlocking && hr == DXGI_ERROR_WAS_STILL_DRAWING)
		return oErrorSetLast(std::errc::operation_would_block);
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

bool oD3D11Device::ReadQuery(oGPUQuery* _pQuery, void* _pData, uint _SizeofData)
{
	return static_cast<oD3D11Query*>(_pQuery)->ReadQuery(ImmediateContext, _pData, _SizeofData);
}

bool oD3D11Device::BeginFrame()
{
	FrameMutex.lock_shared();
	oStd::atomic_increment(&FrameID);
	return true;
}

void oD3D11Device::EndFrame()
{
	DrawCommandLists();
	FrameMutex.unlock_shared();
}

oGUI_DRAW_CONTEXT oD3D11Device::BeginOSFrame()
{
	SwapChainMutex.lock_shared();
	if (!SwapChain)
	{
		SwapChainMutex.unlock_shared();
		return nullptr;
	}

	HDC hDeviceDC = nullptr;
	if (!oDXGIGetDC(SwapChain, &hDeviceDC))
	{
		SwapChainMutex.unlock_shared();
		return nullptr;
	}

	return (oGUI_DRAW_CONTEXT)hDeviceDC;
}

void oD3D11Device::EndOSFrame()
{
	oVERIFY(oDXGIReleaseDC(SwapChain, nullptr));
	SwapChainMutex.unlock_shared();
}

bool oD3D11Device::IsFullscreenExclusive() const
{
	oConcurrency::shared_lock<oConcurrency::shared_mutex> lock(SwapChainMutex);
	if (!SwapChain)
		return false;

	BOOL FS = FALSE;
	const_cast<oD3D11Device*>(this)->SwapChain->GetFullscreenState(&FS, nullptr);
	return !!FS;
}

bool oD3D11Device::SetFullscreenExclusive(bool _Fullscreen)
{
	oConcurrency::lock_guard<oConcurrency::shared_mutex> lock(SwapChainMutex);
	if (!SwapChain)
		return oErrorSetLast(std::errc::protocol_error, "no primary render target has been created");

	DXGI_SWAP_CHAIN_DESC SCD;
	SwapChain->GetDesc(&SCD);
	if (oWinGetParent(SCD.OutputWindow))
		return oErrorSetLast(std::errc::operation_not_permitted, "child windows cannot go full screen exclusive");

	BOOL FS = FALSE;
	SwapChain->GetFullscreenState(&FS, nullptr);
	if (_Fullscreen != !!FS)
	{
		// This can throw an exception for some reason, but there's no DXGI error, and everything seems just fine.
		// so ignore?
		SwapChain->SetFullscreenState(_Fullscreen, nullptr);
	}

	return true;
}

bool oD3D11Device::Present(int _SyncInterval)
{
	oConcurrency::lock_guard<oConcurrency::shared_mutex> lock(SwapChainMutex);

	if (!SwapChain)
		return oErrorSetLast(std::errc::operation_not_permitted, "Present() must only be called on the primary render target");

	DXGI_SWAP_CHAIN_DESC SCD;
	SwapChain->GetDesc(&SCD);

	if (!oWinIsWindowThread(SCD.OutputWindow))
		return oErrorSetLast(std::errc::operation_not_permitted, "Present() must be called from the window thread");

	if (!oWinIsWindowThread(SCD.OutputWindow))
		return oErrorSetLast(std::errc::no_such_device, "Present() must be called from the window thread");

	HRESULT hr = SwapChain->Present(_SyncInterval, 0);
	if (FAILED(hr))
		return oErrorSetLast(std::errc::no_such_device, "GPU device has been reset or removed");

	return true;
}
