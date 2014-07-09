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

#include "d3d_compile.h"
#include "d3d_debug.h"
#include "d3d_device.h"
#include "dxgi_util.h"
#include "CSNoop.h"

#include <oGUI/window.h>

using namespace ouro::gpu::d3d;

namespace ouro {
	namespace gpu {

std::shared_ptr<device> device::make(const device_init& _Init)
{
	auto p = std::make_shared<d3d11::d3d11_device>(_Init);
	p->intialize_immediate_context();
	return p;
}

d3d::Device* get_device(device* _pDevice) { return ((d3d11::d3d11_device*)_pDevice)->d3d_device(); }

intrusive_ptr<d3d::SwapChain> make_swapchain(device* dev, window* win, bool enable_os_render)
{
	if (((d3d11::d3d11_device*)dev)->dxgi_swapchain())
		oTHROW(protocol_error, "There already exists a primary render target, only one can exist for a given device at a time.");

	window_shape s = win->shape();
	if (has_statusbar(s.style))
		oTHROW_INVARG("A window used for rendering must not have a status bar");

	intrusive_ptr<SwapChain> sc = dxgi::make_swap_chain(get_device(dev)
		, false
		, max(int2(1,1), s.client_size)
		, false
		, surface::b8g8r8a8_unorm
		, 0
		, 0
		, (HWND)win->native_handle()
		, enable_os_render);

	return sc;
}

	} // namespace gpu
} // namespace ouro

oGPU_NAMESPACE_BEGIN

template<typename StateT, size_t size> bool state_exists(size_t _Index, StateT (&_States)[size])
{
	for (size_t j = 0; j < _Index; j++)
		if (_States[_Index] == _States[j])
			return true;
	return false;
}

d3d11_device::d3d11_device(const device_init& _Init)
	: FrameID(invalid)
	, hHeap(HeapCreate(0, oMB(10), 0))
	, IsSoftwareEmulation(_Init.use_software_emulation)
	, SupportsDeferredCommandLists(false)
{
	D3DDevice = make_device(_Init);
	SupportsDeferredCommandLists = supports_deferred_contexts(D3DDevice);

	Info = d3d::get_info(D3DDevice, IsSoftwareEmulation);

	HeapAllocations.reserve(500);

	D3DDevice->GetImmediateContext(&ImmediateContext);
	
	// Set up a noop compute shader to flush for SetCounter()
	{
		sstring CSName;
		debug_name(CSName, D3DDevice);
		sncatf(CSName, ".NoopCS");
		NoopCS.initialize(CSName, this, CSNoop);
	}

	SamplerStates.initialize(this);
	RasterizerStates.initialize(this);
	BlendStates.initialize(this);
	DepthStencilStates.initialize(this);
}

void d3d11_device::intialize_immediate_context()
{
	ImmediateCommandList = std::make_shared<d3d11_command_list>(get_shared(), ImmediateContext, invalid);
}

d3d11_device::~d3d11_device()
{
	if (hHeap)
		HeapDestroy(hHeap);
}

device_info d3d11_device::get_info() const
{
	return Info;
}

const char* d3d11_device::name() const
{
	return Info.debug_name;
}

int d3d11_device::frame_id() const
{
	return FrameID;
}

bool d3d11_device::begin_frame()
{
	FrameMutex.lock_shared();
	FrameID++;
	return true;
}

void d3d11_device::end_frame()
{
	draw_command_lists();
	FrameMutex.unlock_shared();
}

struct draw_order_equal
{
	int DrawOrder;
	draw_order_equal(int _DrawOrder) : DrawOrder(_DrawOrder) {}
	bool operator()(const command_list* _pCommandList)
	{
		command_list_info i = _pCommandList->get_info();
		return i.draw_order == DrawOrder;
	}
};

bool by_draw_order(const command_list* _pCommandList1, const command_list* _pCommandList2)
{
	command_list_info i1 = _pCommandList1->get_info();
	command_list_info i2 = _pCommandList2->get_info();
	return i1.draw_order > i2.draw_order;
};

void d3d11_device::insert(command_list* _pCommandList)
{
	std::lock_guard<std::mutex> lock(CommandListsInsertRemoveMutex);
	command_list_info i = _pCommandList->get_info();
	auto it = find_if(CommandLists, draw_order_equal(i.draw_order));
	if (it == CommandLists.end())
		sorted_insert(CommandLists, _pCommandList, by_draw_order);
	else
		oTHROW(operation_in_progress, "Entry with DrawOrder %d already exists", i.draw_order);
}

void d3d11_device::remove(command_list* _pCommandList)
{
	std::lock_guard<std::mutex> lock(CommandListsInsertRemoveMutex);
	find_and_erase(CommandLists, _pCommandList);
}

void d3d11_device::block_submission()
{
	CommandListsBeginEndMutex.lock_shared();
}

void d3d11_device::unblock_submission()
{
	CommandListsBeginEndMutex.unlock_shared();
}

void d3d11_device::draw_command_lists()
{
	std::lock_guard<std::mutex> lock(CommandListsInsertRemoveMutex);
	std::lock_guard<shared_mutex> lock2(CommandListsBeginEndMutex);

	for (command_list* cl : CommandLists)
	{
		d3d11_command_list* c = static_cast<d3d11_command_list*>(cl);
		if (c->CommandList)
		{
			ImmediateContext->ExecuteCommandList(c->CommandList, FALSE);
			c->CommandList = nullptr;
		}
	}
}

oGPU_NAMESPACE_END
