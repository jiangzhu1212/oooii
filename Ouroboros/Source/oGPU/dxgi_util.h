// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.
// DXGI wrapper functions to make usage easier.
#pragma once
#ifndef oGPU_dxgi_util_h
#define oGPU_dxgi_util_h

#include <oBase/intrusive_ptr.h>
#include <oCore/adapter.h>
#include <oSurface/surface.h>
#include "d3d_types.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <dxgi.h>

namespace ouro { namespace gpu { namespace dxgi {

	typedef IDXGISwapChain SwapChain;

	// Convert to an surface::format from a DXGI_FORMAT. If the format is not 
	// supported this will return surface::unknown.
	surface::format to_surface_format(DXGI_FORMAT format);

	// Convert to an surface::format from a DXGI_FORMAT. If the format is not 
	// supported this will return DXGI_FORMAT_UNKNOWN.
	DXGI_FORMAT from_surface_format(surface::format format);

	// returns true if a D* format
	bool is_depth(DXGI_FORMAT format);

	// returns true if a BC* format
	bool is_block_compressed(DXGI_FORMAT format);

	// Returns the byte size of the specified format per element (per block for BC types)
	uint get_size(DXGI_FORMAT format, uint plane = 0);

	// Returns an adapter from its ID.
	intrusive_ptr<IDXGIAdapter> get_adapter(const adapter::id& adapter_id);

	// Returns an info for the specified adapter.
	adapter::info get_info(IDXGIAdapter* adapter);

	// A wrapper around CreateSwapChain that simplifies the input a bit.
	intrusive_ptr<SwapChain> make_swap_chain(IUnknown* dev
		, bool fullscreen
		, const int2& dimensions
		, bool auto_change_monitor_resolution
		, surface::format format
		, uint refresh_rate_numerator
		, uint refresh_rate_denominator
		, HWND hwnd
		, bool enable_gdi_compatibility);

	// Calls resize_buffers with new_size, but all other parameters from GetDesc.
	// Also this encapsulates some robust/standard error reporting.
	void resize_buffers(SwapChain* sc, const int2& new_size);
	
	// Locks and returns the HDC for the render target associated with the 
	// specified swap chain. Don't render while this HDC is valid! Call release
	// when finished.
	HDC get_dc(SwapChain* sc);
	void release_dc(SwapChain* sc, RECT* dirty_rect = nullptr);
	HDC get_dc(d3d::RenderTargetView* rtv);
	void release_dc(d3d::RenderTargetView* rtv, RECT* dirty_rect = nullptr);

	// The various formats required for getting a depth buffer to happen are 
	// complicated, so centralize the mapping in this utility function. If the 
	// Desired format is not a depth format, then desired_format is assigned as a 
	// pass-thru to all three output values.
	void get_compatible_formats(DXGI_FORMAT desired_format, DXGI_FORMAT* out_texture_format = nullptr, DXGI_FORMAT* out_dsv_format = nullptr, DXGI_FORMAT* out_srv_format = nullptr);

	// Does some extra sanity checking
	void set_fullscreen_exclusive(SwapChain* sc, bool fullscreen_exclusive);

	// Does some extra sanity checking
	void present(SwapChain* sc, uint _PresentInterval);

}}}

#endif
