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
#pragma once
#ifndef oD3D11Device_h
#define oD3D11Device_h

#include <oGPU/oGPU.h>
#include <oGPU/state.h>
#include "oGPUCommon.h"

#define oD3D11_DEVICE() ID3D11Device* D3DDevice = static_cast<d3d11_device*>(Device.get())->d3d_device()

oGPU_NAMESPACE_BEGIN

class d3d11_device : public device, public std::enable_shared_from_this<d3d11_device>
{
public:
	d3d11_device(const device_init& _Init);
	~d3d11_device();

	// implementation accessors
	
	inline ID3D11Device* d3d_device() { return D3DDevice; }
	inline ID3D11DeviceContext* d3d_immediate_context() { return ImmediateContext; }
	inline IDXGISwapChain* dxgi_swapchain() { return SwapChain; }

	std::shared_ptr<device> get_shared() { return shared_from_this(); }

	// interface

	std::shared_ptr<command_list> get_immediate_command_list() override { return ImmediateCommandList; }

	inline command_list* immediate() override { return ImmediateCommandList.get(); }

	device_info get_info() const override;
	const char* name() const override;
	int frame_id() const override;

	std::shared_ptr<command_list> make_command_list(const char* _Name, const command_list_info& _Info) override;
	std::shared_ptr<pipeline1> make_pipeline1(const char* _Name, const pipeline1_info& _Info) override;
	std::shared_ptr<query> make_query(const char* _Name, const query_info& _Info) override;
	std::shared_ptr<texture1> make_texture1(const char* _Name, const texture1_info& _Info) override;

	bool map_read(resource1* _pReadbackResource, int _Subresource, surface::mapped_subresource* _pMappedSubresource, bool _Blocking = false) override;
	void unmap_read(resource1* _pReadbackResource, int _Subresource) override;
	bool read_query(query* _pQuery, void* _pData, uint _SizeofData) override;
	bool begin_frame() override;
	void end_frame() override;
	draw_context_handle begin_os_frame() override;
	void end_os_frame() override;
	bool is_fullscreen_exclusive() const override;
	void set_fullscreen_exclusive(bool _Fullscreen) override;
	void present(uint _PresentInterval) override;

	// _____________________________________________________________________________
	// Implementation API

	// Implements Reserve() in the specified context (immediate or deferred).
	surface::mapped_subresource reserve(ID3D11DeviceContext* _pDeviceContext, resource1* _pResource, int _Subresource);

	// Implements Commit() in the specified context (immediate or deferred).
	void commit(ID3D11DeviceContext* _pDeviceContext, resource1* _pResource, int _Subresource, const surface::mapped_subresource& _Source, const surface::box& _Subregion);

	// Registers a command list in the submission vector sorted by its DrawOrder.
	// If there is already a command list at the specified order, then this 
	// returns false and does not insert the specified command list (basically 
	// noop'ing).
	
	void insert(command_list* _pCommandList);
	void remove(command_list* _pCommandList);
	void block_submission();
	void unblock_submission();

	void draw_command_lists();
	void release_swap_chain();

	void intialize_immediate_context();

protected:
	// _____________________________________________________________________________
	// Members

	friend class d3d11_command_list;

	intrusive_ptr<ID3D11Device> D3DDevice;
	intrusive_ptr<ID3D11DeviceContext> ImmediateContext;
	intrusive_ptr<IDXGISwapChain> SwapChain;

	sampler_states SamplerStates;
	rasterizer_states RasterizerStates;
	blend_states BlendStates;
	depth_stencil_states DepthStencilStates;

	// used to flush an explicit setting of a UAV counter.
	compute_shader NoopCS;

	std::shared_ptr<command_list> ImmediateCommandList;

	// For this heap there shouldn't be too many simultaneous allocations - 
	// remember this is to support Map/Unmap style API, so maybe on a thread 
	// there's 10 maps at the same time? at most? and then there's 12 threads? So
	// for so small a set a linear search is faster than a hash.
	// Concurrency for this vector is assured by HeapLock/HeapUnlock since changes
	// to this vector occur at the same time as heap operations.
	std::vector<void*> HeapAllocations;
	HANDLE hHeap;

	std::mutex CommandListsInsertRemoveMutex;
	shared_mutex CommandListsBeginEndMutex;
	shared_mutex FrameMutex;
	std::vector<command_list*> CommandLists; // non-oRefs to avoid circular refs

	device_info Info;
	std::atomic<int> FrameID;

	bool IsSoftwareEmulation;
	bool SupportsDeferredCommandLists;

	d3d11_device(const d3d11_device&)/* = delete */;
	const d3d11_device& operator=(const d3d11_device&)/* = delete */;
};

oGPU_NAMESPACE_END

#endif
