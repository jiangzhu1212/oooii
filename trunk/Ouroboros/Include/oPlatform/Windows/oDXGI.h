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
// DXGI wrapper functions to make usage easier.
#pragma once
#ifndef oGPU_dxgi_h
#define oGPU_dxgi_h

#include <oCore/adapter.h>
#include <oSurface/surface.h>
#include "../Source/oStd/win.h"
#include <d3d11.h>
#include <dxgi.h>

namespace ouro {
	namespace dxgi {

	// Convert to an surface::format from a DXGI_FORMAT. If the format is not 
	// supported this will return surface::unknown.
	surface::format to_surface_format(DXGI_FORMAT _Format);

	// Convert to an surface::format from a DXGI_FORMAT. If the format is not 
	// supported this will return DXGI_FORMAT_UNKNOWN.
	DXGI_FORMAT from_surface_format(surface::format _Format);

	// Returns an adapter from its ID.
	intrusive_ptr<IDXGIAdapter> get_adapter(const adapter::id& _AdapterID);

	// Returns an info for the specified adapter.
	adapter::info get_info(IDXGIAdapter* _pAdapter);

	// A wrapper around CreateSwapChain that simplifies the input a bit.
	intrusive_ptr<IDXGISwapChain> make_swap_chain(IUnknown* _pDevice
		, bool _Fullscreen
		, const int2& _Dimensions
		, bool _AutochangeMonitorResolution
		, surface::format _Format
		, unsigned int RefreshRateN
		, unsigned int RefreshRateD
		, HWND _hWnd
		, bool _EnableGDICompatibility);

	// Calls resize_buffers with _NewSize, but all other parameters from GetDesc.
	// Also this encapsulates some robust/standard error reporting.
	void resize_buffers(IDXGISwapChain* _pSwapChain, const int2& _NewSize);
	
	// Locks and returns the HDC for the render target associated with the 
	// specified swap chain. Don't render while this HDC is valid! Call release
	// when finished.
	HDC get_dc(IDXGISwapChain* _pSwapChain);
	void release_dc(IDXGISwapChain* _pSwapChain, RECT* _pDirtyRect = nullptr);
	HDC get_dc(ID3D11RenderTargetView* _pRTV);
	void release_dc(ID3D11RenderTargetView* _pRTV, RECT* _pDirtyRect = nullptr);

	// The various formats required for getting a depth buffer to happen are 
	// complicated, so centralize the mapping in this utility function. If the 
	// Desired format is not a depth format, then _DesiredFormat is assigned as a 
	// pass-thru to all three output values.
	void get_compatible_formats(DXGI_FORMAT _DesiredFormat, DXGI_FORMAT* _pTextureFormat = nullptr, DXGI_FORMAT* _pDepthStencilViewFormat = nullptr, DXGI_FORMAT* _pShaderResourceViewFormat = nullptr);

	} // namespace dxgi
} // namespace ouro

#endif
