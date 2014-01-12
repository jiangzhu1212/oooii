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
#include <oCore/windows/win_util.h>
#include <oGUI/Windows/oWinWindowing.h>

#include "d3d11_util.h"
#include "dxgi_util.h"

//#include <oPlatform/Windows/oWinWindowing.h>

#include "oD3D11Buffer.h"
#include "oD3D11Pipeline.h"
#include "oD3D11Texture.h"
#include "oD3D11Query.h"

#include "CSNoop.h"

using namespace ouro;
using namespace ouro::d3d11;

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
		return d.draw_order == DrawOrder;
	}
};

bool ByDrawOrder(const oGPUCommandList* _pCommandList1, const oGPUCommandList* _pCommandList2)
{
	oGPUCommandList::DESC d1, d2;
	_pCommandList1->GetDesc(&d1);
	_pCommandList2->GetDesc(&d2);
	return d1.draw_order > d2.draw_order;
};

bool oGPUDeviceCreate(const ouro::gpu::device_init& _Init, oGPUDevice** _ppDevice)
{
	intrusive_ptr<ID3D11Device> Device;
	try { Device = make_device(_Init); }
	catch (std::exception& e) { return oErrorSetLast(e); }

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
	, FrameID(ouro::invalid)
	, hHeap(HeapCreate(0, oMB(10), 0))
	, IsSoftwareEmulation(_Init.use_software_emulation)
	, SupportsDeferredCommandLists(d3d11::supports_deferred_contexts(_pDevice))
{
	*_pSuccess = false;

	Desc = get_info(_pDevice, IsSoftwareEmulation);

	HeapAllocations.reserve(500);

	D3DDevice->GetImmediateContext(&ImmediateContext);
	lstring StateName;

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
		static_assert(oCOUNTOF(sBlends) == ouro::gpu::blend_state::count, "# blend states mismatch");

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
				snprintf(StateName, "%s.%s", _Init.debug_name.c_str(), ouro::as_string((ouro::gpu::blend_state::value)i));
				debug_name(BlendStates[i], StateName);
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
				snprintf(StateName, "%s.%s", _Init.debug_name.c_str(), ouro::as_string((ouro::gpu::depth_stencil_state::value)i));
				debug_name(DepthStencilStates[i], StateName);
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
		static_assert(oCOUNTOF(sFills) == ouro::gpu::surface_state::count, "# surface states mismatch");

		static const D3D11_CULL_MODE sCulls[] = 
		{
			D3D11_CULL_BACK,
			D3D11_CULL_FRONT,
			D3D11_CULL_NONE,
			D3D11_CULL_BACK,
			D3D11_CULL_FRONT,
			D3D11_CULL_NONE,
		};
		static_assert(oCOUNTOF(sCulls) == ouro::gpu::surface_state::count, "# surface states mismatch");

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
				snprintf(StateName, "%s.%s", _Init.debug_name.c_str(), ouro::as_string((ouro::gpu::surface_state::value)i));
				debug_name(SurfaceStates[i], StateName);
			}
		}
	}

	// Sampler States
	{
		static const int NUM_ADDRESS_STATES = 6;
		static const int NUM_MIP_BIAS_LEVELS = ouro::gpu::sampler_type::count / NUM_ADDRESS_STATES;

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
		mstring StateName;

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

				snprintf(StateName, "%s.%s", _Init.debug_name.c_str(), ouro::as_string(ouro::gpu::sampler_type::value(NUM_ADDRESS_STATES * bias + state)));
				oV(_pDevice->CreateSamplerState(&desc, &SamplerStates[i]));
				debug_name(SamplerStates[i], StateName);
				i++;
			}
		}
	}
	
	// Set up some null buffers that will be used to reset parts of the API that
	// do no easily transition between compute and rasterization pipelines.
	{
		// ends up being all set to ouro::invalid/-1/D3D11_KEEP_UNORDERED_ACCESS_VIEWS 
		// which means leave value alone
		memset(NoopUAVInitialCounts, 0xff, sizeof(NoopUAVInitialCounts));

		// set to an array of nulls
		memset(NullUAVs, 0, sizeof(NullUAVs));
		memset(NullRTVs, 0, sizeof(NullRTVs));
	}

	// Set up a noop compute shader to flush for SetCounter()
	{
		oV(D3DDevice->CreateComputeShader(CSNoop, byte_code_size(CSNoop), nullptr, &NoopCS));

		sstring CSName;
		debug_name(CSName, _pDevice);
		sncatf(CSName, "NoopCS");
		debug_name(NoopCS, CSName);
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
		oStd::lock_guard<shared_mutex> lock(thread_cast<shared_mutex&>(SwapChainMutex));
		SwapChain->AddRef();
		*_ppInterface = SwapChain;
	}

	return !!*_ppInterface;
}

void oD3D11Device::GetDesc(DESC* _pDesc) const threadsafe
{
	*_pDesc = thread_cast<DESC&>(Desc);
}

const char* oD3D11Device::GetName() const threadsafe
{
	return Desc.debug_name;
}

uint oD3D11Device::GetFrameID() const threadsafe
{
	return FrameID;
}

void oD3D11Device::GetImmediateCommandList(oGPUCommandList** _ppCommandList)
{
	oGPUCommandList::DESC CLDesc;
	CLDesc.draw_order = ouro::invalid;
	oVERIFY(oD3D11Device::CreateCommandList("Immediate", CLDesc, _ppCommandList));
}

bool oD3D11Device::CreatePrimaryRenderTarget(ouro::window* _pWindow, ouro::surface::format _DepthStencilFormat, bool _EnableOSRendering, oGPURenderTarget** _ppPrimaryRenderTarget)
{
	if (SwapChain)
		return oErrorSetLast(std::errc::protocol_error, "There already exists a primary render target, only one can exist for a given device at a time.");

	ouro::window_shape s = _pWindow->shape();
	if (ouro::has_statusbar(s.style))
		return oErrorSetLast(std::errc::invalid_argument, "A window used for rendering must not have a status bar");

	try
	{
		SwapChain = dxgi::make_swap_chain(D3DDevice
			, false
			, max(int2(1,1), s.client_size)
			, false
			, surface::b8g8r8a8_unorm
			, 0
			, 0
			, (HWND)_pWindow->native_handle()
			, _EnableOSRendering);
	}
	catch (std::exception& e)
	{
		return oErrorSetLast(e);
	}

	sstring RTName;
	snprintf(RTName, "%s.PrimaryRT", GetName());
	if (!oD3D11CreateRenderTarget(this, RTName, SwapChain, _DepthStencilFormat, _ppPrimaryRenderTarget))
		return false;
	return true;
}

bool oD3D11Device::CLInsert(oGPUCommandList* _pCommandList) threadsafe
{
	std::lock_guard<std::mutex> lock(thread_cast<std::mutex&>(CommandListsInsertRemoveMutex));
	oD3D11Device* pThis = thread_cast<oD3D11Device*>(this);

	oGPUCommandList::DESC d;
	_pCommandList->GetDesc(&d);
	auto it = find_if(pThis->CommandLists, DrawOrderEqual(d.draw_order));

	if (it == pThis->CommandLists.end())
	{
		sorted_insert(pThis->CommandLists, _pCommandList, ByDrawOrder);
		return true;
	}

	return oErrorSetLast(std::errc::operation_in_progress, "Entry with DrawOrder %d already exists", d.draw_order);
}

void oD3D11Device::MEMReserve(ID3D11DeviceContext* _pDeviceContext, oGPUResource* _pResource, int _Subresource, ouro::surface::mapped_subresource* _pMappedSubresource) threadsafe
{
	int2 ByteDimensions = _pResource->GetByteDimensions(_Subresource);
	size_t size = ByteDimensions.x * ByteDimensions.y;
	HeapLock(hHeap);
	void* p = HeapAlloc(hHeap, 0, size);
	thread_cast<oD3D11Device*>(this)->HeapAllocations.push_back(p); // safe because vector is protected with HeapLock
	HeapUnlock(hHeap);
	_pMappedSubresource->data = p;
	_pMappedSubresource->row_pitch = ByteDimensions.x;
	_pMappedSubresource->depth_pitch = as_uint(size);
}

void oD3D11Device::MEMCommit(ID3D11DeviceContext* _pDeviceContext, oGPUResource* _pResource, int _Subresource, const ouro::surface::mapped_subresource& _Source, const ouro::surface::box& _Subregion) threadsafe
{
	int D3DSubresource = ouro::invalid;
	ouro::gpu::resource_type::value type = _pResource->GetType();

	ID3D11Resource* pD3DResource = oD3D11GetSubresource(_pResource, _Subresource, &D3DSubresource);

	D3D11_BOX box;
	D3D11_BOX* pBox = nullptr;

	if (!_Subregion.empty())
	{
		uint StructureByteStride = 1;
		if (type == ouro::gpu::resource_type::buffer)
		{
			gpu::buffer_info i = get_info(static_cast<ID3D11Buffer*>(pD3DResource));
			StructureByteStride = __max(1, i.struct_byte_size);
			oASSERT(_Subregion.top == 0 && _Subregion.bottom == 1, "Buffer subregion must have top == 0 and bottom == 1");
		}

		box.left = _Subregion.left * StructureByteStride;
		box.top = _Subregion.top;
		box.right = _Subregion.right * StructureByteStride;
		box.bottom = _Subregion.bottom; 
		box.front = _Subregion.front;
		box.back = _Subregion.back;
		pBox = &box;
	}

	update_subresource(_pDeviceContext, pD3DResource, D3DSubresource, pBox, _Source, SupportsDeferredCommandLists);

	HeapLock(hHeap);
	if (find_and_erase(thread_cast<oD3D11Device*>(this)->HeapAllocations, _Source.data)) // safe because vector is protected with HeapLock
		HeapFree(hHeap, 0, _Source.data);
	HeapUnlock(hHeap);
}

void oD3D11Device::CLRemove(oGPUCommandList* _pCommandList) threadsafe
{
	std::lock_guard<std::mutex> lock(thread_cast<std::mutex&>(CommandListsInsertRemoveMutex));
	oD3D11Device* pThis = thread_cast<oD3D11Device*>(this);
	find_and_erase(pThis->CommandLists, _pCommandList);
}

void oD3D11Device::CLLockSubmit() threadsafe
{
	thread_cast<shared_mutex&>(CommandListsBeginEndMutex).lock_shared();
}

void oD3D11Device::CLUnlockSubmit() threadsafe
{
	thread_cast<shared_mutex&>(CommandListsBeginEndMutex).unlock_shared();
}

void oD3D11Device::DrawCommandLists() threadsafe
{
	std::lock_guard<std::mutex> lock(thread_cast<std::mutex&>(CommandListsInsertRemoveMutex));
	oD3D11Device* pThis = thread_cast<oD3D11Device*>(this);

	std::lock_guard<shared_mutex> lock2(thread_cast<shared_mutex&>(CommandListsBeginEndMutex));

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
	oStd::lock_guard<shared_mutex> lock(thread_cast<shared_mutex&>(SwapChainMutex));
	SwapChain = nullptr;
}

bool oD3D11Device::MapRead(oGPUResource* _pReadbackResource, int _Subresource, ouro::surface::mapped_subresource* _pMappedSubresource, bool _bBlocking)
{
	int D3DSubresourceIndex = 0;
	ID3D11Resource* r = oD3D11GetSubresource(_pReadbackResource, _Subresource, &D3DSubresourceIndex);

	D3D11_MAPPED_SUBRESOURCE msr;
	oV(ImmediateContext->Map(r, D3DSubresourceIndex, D3D11_MAP_READ, _bBlocking ? 0 : D3D11_MAP_FLAG_DO_NOT_WAIT, &msr));
	_pMappedSubresource->data = msr.pData;
	_pMappedSubresource->row_pitch = msr.RowPitch;
	_pMappedSubresource->depth_pitch = msr.DepthPitch;
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
	FrameID++;
	return true;
}

void oD3D11Device::EndFrame()
{
	DrawCommandLists();
	FrameMutex.unlock_shared();
}

ouro::draw_context_handle oD3D11Device::BeginOSFrame()
{
	SwapChainMutex.lock_shared();
	if (!SwapChain)
	{
		SwapChainMutex.unlock_shared();
		return nullptr;
	}

	return (ouro::draw_context_handle)dxgi::get_dc(SwapChain);
}

void oD3D11Device::EndOSFrame()
{
	dxgi::release_dc(SwapChain);
	SwapChainMutex.unlock_shared();
}

bool oD3D11Device::IsFullscreenExclusive() const
{
	shared_lock<shared_mutex> lock(thread_cast<shared_mutex&>(SwapChainMutex));
	if (!SwapChain)
		return false;

	BOOL FS = FALSE;
	const_cast<oD3D11Device*>(this)->SwapChain->GetFullscreenState(&FS, nullptr);
	return !!FS;
}

bool oD3D11Device::SetFullscreenExclusive(bool _Fullscreen)
{
	oStd::lock_guard<shared_mutex> lock(SwapChainMutex);
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
	oStd::lock_guard<shared_mutex> lock(SwapChainMutex);

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
