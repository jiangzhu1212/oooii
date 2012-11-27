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
#pragma once
#ifndef oD3D11Device_h
#define oD3D11Device_h

#include <oGPU/oGPU.h>
#include <oGPU/oGPUDrawConstants.h>
#include <oBasis/oNoncopyable.h>
#include <oPlatform/Windows/oWindows.h>
#include <oPlatform/Windows/oD3D11.h>
#include <vector>

#define oD3D11DEVICE() \
	oRef<ID3D11Device> D3DDevice; \
	oVERIFY(Device->QueryInterface(oGetGUID<ID3D11Device>(), &D3DDevice));

// Call reg/unreg from device children implementations
#define oDEVICE_REGISTER_THIS() oVERIFY(static_cast<threadsafe oD3D11Device*>(Device.c_ptr())->CLInsert(this))
#define oDEVICE_UNREGISTER_THIS() static_cast<threadsafe oD3D11Device*>(Device.c_ptr())->CLRemove(this)

#define oDEVICE_LOCK_SUBMIT() static_cast<threadsafe oD3D11Device*>(Device.c_ptr())->CLLockSubmit()
#define oDEVICE_UNLOCK_SUBMIT() static_cast<threadsafe oD3D11Device*>(Device.c_ptr())->CLUnlockSubmit()

struct oD3D11Device : oGPUDevice, oNoncopyable
{
	// _____________________________________________________________________________
	// Interface API

	oDEFINE_REFCOUNT_INTERFACE(RefCount);
	bool QueryInterface(const oGUID& _InterfaceID, threadsafe void** _ppInterface) threadsafe override;

	oD3D11Device(ID3D11Device* _pDevice, const oGPUDevice::INIT& _Init, bool* _pSuccess);
	~oD3D11Device();

	void GetDesc(DESC* _pDesc) const threadsafe override;
	const char* GetName() const threadsafe override;
	uint GetFrameID() const threadsafe override;
	void GetImmediateCommandList(oGPUCommandList** _ppCommandList) override;

	bool CreateCommandList(const char* _Name, const oGPUCommandList::DESC& _Desc, oGPUCommandList** _ppCommandList) override;
	bool CreateLineList(const char* _Name, const oGPULineList::DESC& _Desc, oGPULineList** _ppLineList) override;
	bool CreatePipeline(const char* _Name, const oGPUPipeline::DESC& _Desc, oGPUPipeline** _ppPipeline) override;
	bool CreateComputeShader(const char* _Name, const oGPUComputeShader::DESC& _Desc, oGPUComputeShader** _ppComputeShader) override;
	bool CreateRenderTarget(const char* _Name, const oGPURenderTarget::DESC& _Desc, oGPURenderTarget** _ppRenderTarget) override;
	bool CreateBuffer(const char* _Name, const oGPUBuffer::DESC& _Desc, oGPUBuffer** _ppBuffer) override;
	bool CreateMesh(const char* _Name, const oGPUMesh::DESC& _Desc, oGPUMesh** _ppMesh) override;
	bool CreateInstanceList(const char* _Name, const oGPUInstanceList::DESC& _Desc, oGPUInstanceList** _ppInstanceList) override;
	bool CreateTexture(const char* _Name, const oGPUTexture::DESC& _Desc, oGPUTexture** _ppTexture) override;

	bool MapRead(oGPUResource* _pReadbackResource, int _Subresource, oSURFACE_MAPPED_SUBRESOURCE* _pMappedSubresource, bool _bBlocking=false) override;
	void UnmapRead(oGPUResource* _pReadbackResource, int _Subresource) override;

	bool BeginFrame() override;
	void EndFrame() override;

	// _____________________________________________________________________________
	// Implementation API

	// NOTE: for MEMReserve/MEMCommit, threadsafety is a bit strange. We have 
	// command lists which are by design single-threaded, but must access this 
	// same device for memory allocation, then we have oGPUDevice which is all
	// threadsafe, but through serialization/synchronization. Thus the meaning of 
	// threadsafe here is that it can be called from oD3D11CommandList, however it
	// does NOT mean in general that multiple threads can update the same resource 
	// on the same context (though it can if the contexts are different). In 
	// short, don't call this unless you fully understand the differences in usage
	// and the partial-threadsafety guaranteed by design of device v. command list.
	// NOTE: oD3D11Device protects calls to the public Reserve/Commit with its own
	// mutex, guaranteeing its threadsafety for the shared immediate context.

	// Implements Reserve() in the specified context (immediate or deferred).
	void MEMReserve(ID3D11DeviceContext* _pDeviceContext, oGPUResource* _pResource, int _Subresource, oSURFACE_MAPPED_SUBRESOURCE* _pMappedSubresource) threadsafe;

	// Implements Commit() in the specified context (immediate or deferred).
	void MEMCommit(ID3D11DeviceContext* _pDeviceContext, oGPUResource* _pResource, int _Subresource, oSURFACE_MAPPED_SUBRESOURCE& _Source, const oGPU_BOX& _Subregion) threadsafe;

	// Registers a command list in the submission vector sorted by its DrawOrder.
	// If there is already a command list at the specified order, then this 
	// returns false and does not insert the specified command list (basically 
	// noop'ing).
	
	bool CLInsert(oGPUCommandList* _pCommandList) threadsafe;
	void CLRemove(oGPUCommandList* _pCommandList) threadsafe;
	void CLLockSubmit() threadsafe;
	void CLUnlockSubmit() threadsafe;
	void DrawCommandLists() threadsafe;

	// _____________________________________________________________________________
	// Members

	oRef<ID3D11Device> D3DDevice;
	oRef<ID3D11DeviceContext> ImmediateContext;
	// @oooii-tony:
	// This repeats what's in oD3D11, but this is designed to replace the oD3D11 components,
	// so this will be favored eventually...
	oRef<ID3D11BlendState> BlendStates[oGPU_BLEND_NUM_STATES];
	oRef<ID3D11RasterizerState> SurfaceStates[oGPU_SURFACE_NUM_STATES];
	oRef<ID3D11DepthStencilState> DepthStencilStates[oGPU_DEPTH_STENCIL_NUM_STATES];
	oRef<ID3D11SamplerState> SamplerStates[oGPU_SAMPLER_NUM_STATES];

	// used to flush an explicit setting of a UAV counter.
	oRef<ID3D11ComputeShader> NoopCS;

	// These will hold null/noop values and are initialized only at construction
	// time, so it's safe to access from command lists in multiple threads.
	uint NoopUAVInitialCounts[oGPU_MAX_NUM_UNORDERED_BUFFERS];
	ID3D11UnorderedAccessView* NullUAVs[oGPU_MAX_NUM_UNORDERED_BUFFERS];
	ID3D11RenderTargetView* NullRTVs[oGPU_MAX_NUM_MRTS];

	// For this heap there shouldn't be too many simultaneous allocations - 
	// remember this is to support Map/Unmap style API, so maybe on a thread 
	// there's 10 maps at the same time? at most? and then there's 12 threads? So
	// for so small a set a linear search is faster than a hash.
	// Concurrency for this vector is assured by HeapLock/HeapUnlock since changes
	// to this vector occur at the same time as heap operations.
	std::vector<void*> HeapAllocations;
	HANDLE hHeap;

	oInitOnce<DESC> Desc;
	oRefCount RefCount;
	uint FrameID;

	oMutex CommandListsInsertRemoveMutex;
	oSharedMutex CommandListsBeginEndMutex;
	std::vector<oGPUCommandList*> CommandLists; // non-oRefs to avoid circular refs

	bool IsSoftwareEmulation;
};

#endif
