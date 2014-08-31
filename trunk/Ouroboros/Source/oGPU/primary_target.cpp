// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.
#include <oGPU/primary_target.h>
#include <oGPU/depth_target.h>
#include <oBase/color.h>
#include "d3d_debug.h"
#include "d3d_util.h"
#include "dxgi_util.h"

#include <oGUI/window.h>

using namespace ouro::gpu::d3d;

namespace ouro { namespace gpu {

Device* get_device(device& dev);
DeviceContext* get_dc(command_list& cl);

primary_target::primary_target()
	: swapchain(nullptr)
	, npresents(0)
{
}

void primary_target::initialize(window* win, device& dev, bool enable_os_render)
{
	deinitialize();
	rws.resize(1);

	oCHECK_ARG(!win->render_target(), "The specified window is already associated with a render target and cannot be reassociated.");
	window_shape s = win->shape();
	oCHECK_ARG(!has_statusbar(s.style), "A window used for rendering must not have a status bar");

	intrusive_ptr<SwapChain> sc = dxgi::make_swap_chain(get_device(dev)
		, false
		, max(int2(1,1), s.client_size)
		, false
		, surface::format::b8g8r8a8_unorm
		, 0
		, 0
		, (HWND)win->native_handle()
		, enable_os_render);

	win->render_target(true);

	swapchain = sc;
	sc->AddRef();

	DXGI_SWAP_CHAIN_DESC desc;
	sc->GetDesc(&desc);
	internal_resize(uint2(desc.BufferDesc.Width, desc.BufferDesc.Height), &dev);
}

void primary_target::deinitialize()
{
	basic_deinitialize();
	
	lock_guard<shared_mutex> lock(mutex);
	
	oSAFE_RELEASEV(swapchain);
}

void primary_target::internal_resize(const uint2& dimensions, device* dev)
{
	oCHECK0(swapchain);
	oCHECK0(rws[0] || dev);

	if (any(dimensions == uint2(0,0)))
	{
		deinitialize();
		return;
	}

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
	if (rws[0])
	{
		intrusive_ptr<Resource> r;
		((View*)rws[0])->GetResource(&r);
		D3D_TEXTURE_DESC d = get_texture_desc(r);
		Current = uint2(d.Width, d.Height);
	}

	if (any(Current != New) || !rws[0])
	{
		mstring TargetName;
		oTRACEA("%s %s Resize %dx%d -> %dx%d", type_name(typeid(*this).name()), name(TargetName, TargetName.capacity()), Current.x, Current.y, New.x, New.y);

		intrusive_ptr<Device> D3DDevice;
		if (dev)
			D3DDevice = get_device(*dev);
		else
			((View*)rws[0])->GetDevice(&D3DDevice);

		oSAFE_RELEASEV(ro);
		oSAFE_RELEASEV(rws[0]);
		oSAFE_RELEASEV(crw);

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
				
		auto rtv = make_rtv(SwapChainTexture);
		auto srv = make_srv(SwapChainTexture, DXGI_FORMAT_UNKNOWN, 0);

		D3D11_TEXTURE2D_DESC desc;
		SwapChainTexture->GetDesc(&desc);
		if (desc.BindFlags & D3D11_BIND_UNORDERED_ACCESS)
		{
			intrusive_ptr<UnorderedAccessView> uav;
			oV(D3DDevice->CreateUnorderedAccessView(SwapChainTexture, nullptr, &uav));

			uav->AddRef();
			crw = uav;
		}

		rtv->AddRef();
		rws[0] = rtv;

		srv->AddRef();
		ro = srv;
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

}}
