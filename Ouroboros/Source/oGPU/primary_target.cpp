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
#include <oGPU/primary_target.h>
#include <oGPU/depth_target.h>
#include <oBase/color.h>
#include "d3d_debug.h"
#include "d3d_util.h"
#include "dxgi_util.h"

#include <oGUI/window.h>
#include <oGUI/windows/oWinWindowing.h> // todo: put set/get is_render_target into window.h

using namespace ouro::gpu::d3d;

namespace ouro { namespace gpu {

Device* get_device(device* dev);
DeviceContext* get_dc(command_list* cl);

primary_target::primary_target()
	: swapchain(nullptr)
	, rw(nullptr)
	, npresents(0)
{
}

void primary_target::initialize(window* win, device* dev, bool enable_os_render)
{
	deinitialize();

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

	DXGI_SWAP_CHAIN_DESC desc;
	sc->GetDesc(&desc);

	oCHECK_ARG(!oWinIsRenderTarget(desc.OutputWindow), "The specified window is already associated with a render target and cannot be reassociated.");
	oWinSetIsRenderTarget(desc.OutputWindow);

	swapchain = sc;
	sc->AddRef();
	internal_resize(uint2(desc.BufferDesc.Width, desc.BufferDesc.Height), dev);
}

void primary_target::deinitialize()
{
	oSAFE_RELEASEV(ro);
	oSAFE_RELEASEV(rw);
	
	lock_guard<shared_mutex> lock(mutex);
	
	if (swapchain)
	{
		((SwapChain*)swapchain)->Release();
		swapchain = nullptr;
	}
}

uint2 primary_target::dimensions() const
{
	if (!rw)
		return uint2(0, 0);

	intrusive_ptr<Resource> r;
	((View*)rw)->GetResource(&r);
	D3D_TEXTURE_DESC desc = get_texture_desc(r);
	return uint2(desc.Width, desc.Height);
}

std::shared_ptr<surface::buffer> primary_target::make_snapshot()
{
	if (!ro)
		oTHROW(resource_unavailable_try_again, "The render target is minimized or not available for snapshot.");
	intrusive_ptr<Texture2D> t;
	((View*)ro)->GetResource((Resource**)&t);

	return d3d::make_snapshot(t);
}

void primary_target::internal_resize(const uint2& dimensions, device* dev)
{
	oCHECK0(swapchain);
	oCHECK0(rw || dev);

	ushort2 New = dimensions;
	{
		BOOL IsFullScreen = FALSE;
		intrusive_ptr<IDXGIOutput> Output;
		((SwapChain*)swapchain)->GetFullscreenState(&IsFullScreen, &Output);
		if (IsFullScreen)
		{
			DXGI_OUTPUT_DESC desc;
			Output->GetDesc(&desc);
			New = uint2(desc.DesktopCoordinates.right - desc.DesktopCoordinates.left, 
				desc.DesktopCoordinates.bottom - desc.DesktopCoordinates.top);
		}
	}

	ushort2 Current(0, 0);
	if (rw)
	{
		intrusive_ptr<Resource> r;
		((View*)rw)->GetResource(&r);
		D3D_TEXTURE_DESC d = get_texture_desc(r);
		Current = uint2(d.Width, d.Height);
	}

	if (any(Current != New) || !rw)
	{
		mstring TargetName;
		oTRACEA("%s %s Resize %dx%d -> %dx%d", type_name(typeid(*this).name()), name(TargetName, TargetName.capacity()), Current.x, Current.y, New.x, New.y);

		intrusive_ptr<Device> D3DDevice;
		if (dev)
			D3DDevice = get_device(dev);
		else
			((View*)rw)->GetDevice(&D3DDevice);

		oSAFE_RELEASEV(ro);
		oSAFE_RELEASEV(rw);

		intrusive_ptr<DeviceContext> dc;
		D3DDevice->GetImmediateContext(&dc);

		unset_all_draw_targets(dc);
		unset_all_dispatch_targets(dc);

		SwapChain* sc = (SwapChain*)swapchain;
		dxgi::resize_buffers(sc, New.xy());

		// now rebuild derived resources
		intrusive_ptr<Texture2D> SwapChainTexture;
		oV(sc->GetBuffer(0, __uuidof(Texture2D), (void**)&SwapChainTexture));
		debug_name(SwapChainTexture, "primary target");
				
		rw = make_rtv(SwapChainTexture);
		ro = make_srv(SwapChainTexture, DXGI_FORMAT_UNKNOWN, 0);
	}
}

void* primary_target::begin_os_frame()
{
	mutex.lock_shared();
	if (!swapchain)
	{
		mutex.unlock_shared();
		return nullptr;
	}

	return dxgi::get_dc((SwapChain*)swapchain);
}

void primary_target::end_os_frame()
{
	dxgi::release_dc((SwapChain*)swapchain);
	mutex.unlock_shared();
}

bool primary_target::is_fullscreen_exclusive() const
{
	shared_lock<shared_mutex> lock(mutex);
	if (!swapchain)
		return false;

	BOOL FS = FALSE;
	((SwapChain*)swapchain)->GetFullscreenState(&FS, nullptr);
	return !!FS;
}

void primary_target::set_fullscreen_exclusive(bool fullscreen)
{
	lock_guard<shared_mutex> lock(mutex);
	if (!swapchain)
		oTHROW(protocol_error, "no primary render target has been created");
	dxgi::set_fullscreen_exclusive((SwapChain*)swapchain, fullscreen);
}

void primary_target::present(uint interval)
{
	lock_guard<shared_mutex> lock(mutex);
	if (!swapchain)
		oTHROW(protocol_error, "no primary render target has been created");
	dxgi::present((SwapChain*)swapchain, interval);
	npresents++;
}

void primary_target::set_draw_target(command_list* cl, depth_target* depth, uint depth_index, const viewport& vp)
{
	set_viewports(cl, dimensions(), &vp, 1);
	get_dc(cl)->OMSetRenderTargets(1, (RenderTargetView* const*)&rw, depth ? (DepthStencilView*)depth->get_target(depth_index) : nullptr);
}

void primary_target::clear(command_list* cl, const color& c)
{
	float fcolor[4];
	c.decompose(&fcolor[0], &fcolor[1], &fcolor[2], &fcolor[3]);
	get_dc(cl)->ClearRenderTargetView((RenderTargetView*)rw, fcolor);
}

}}
