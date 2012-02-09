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
// APIs to make using DXGI easier.
#pragma once
#ifndef oDXGI_h
#define oDXGI_h

#include <oBasis/oSurface.h>
#include <oBasis/oVersion.h>
#include <oPlatform/oWindows.h>

#if oDXVER >= oDXVER_10

	// Convert to an oSURFACE_FORMAT from a DXGI_FORMAT. If the format is not 
	// supported this will return oSURFACE_UNKNOWN.
	oSURFACE_FORMAT oDXGIToSurfaceFormat(DXGI_FORMAT _Format);

	// Convert to an oSURFACE_FORMAT from a DXGI_FORMAT. If the format is not 
	// supported this will return DXGI_FORMAT_UNKNOWN.
	DXGI_FORMAT oDXGIFromSurfaceFormat(oSURFACE_FORMAT _Format);

	// IDXGIFactory is special as it loads DLLs so it can not be statically held
	// as it can not be released from DLLmain, so always create it.
	bool oDXGICreateFactory(IDXGIFactory1** _ppFactory);

	bool oDXGICreateSwapChain(IUnknown* _pDevice, bool _Fullscreen, UINT _Width, UINT _Height, DXGI_FORMAT _Format, UINT RefreshRateN, UINT RefreshRateD, HWND _hWnd, IDXGISwapChain** _ppSwapChain);
	
	// This sets the fullscreen state and flushes the resulting messages. This
	// must be called from the same thread that created the window and is 
	// processing its messages. If not, it will return false and oErrorGetLast()
	// will return EWRONGTHREAD. _RefreshRate will be expanded in a manner to
	// take maximum advantage of HW acceleration (enable swap-not-blit). When
	// setting _Fullscreen false, this will restore the desktop to whatever 
	// settings were ACTIVE when oDXGISetFullscreenState was called with 
	// _Fullscreen set to true. It uses oDXGISetPreFullscreenMode to do this 
	// because the default DXGI behavior is to restore to registry (original)
	// settings and ignore any state the user might've set in the application.
	bool oDXGISetFullscreenState(IDXGISwapChain* _pSwapChain, bool _Fullscreen, const int2& _FullscreenSize = int2(oDEFAULT, oDEFAULT), int _FullscreenRefreshRate = oDEFAULT);

	// Associates the specified parameters with the specified swap chain through
	// PrivateData.
	bool oDXGISetPreFullscreenMode(IDXGISwapChain* _pSwapChain, const int2& _Size, int _RefreshRate);

	// Retrieves any prior value set.
	bool oDXGIGetPreFullscreenMode(IDXGISwapChain* _pSwapChain, int2* _pSize, int* _pRefreshRate);

	// Returns the numeric version of the highest level of D2D the specified 
	// adapter supports.
	
	// Returns the highest DirectX version number of the API interface that can be 
	// instantiated (i.e. whether you can create an ID3D10Device or an 
	// ID3D11Device). This is related more to the OS version than the HW 
	// capabilities.
	oVersion oDXGIGetInterfaceVersion(IDXGIAdapter* _pAdapter);

	// Returns the highest D3D feature level version that can be HW accelerated by 
	// the specified adapter. For example even though an ID3D11Device can be 
	// instantiated the HW may only support DX10 features.
	oVersion oDXGIGetFeatureLevel(IDXGIAdapter* _pAdapter);

	// A bit of syntactic sugar on top of GetParent()
	bool oDXGIGetAdapter(IDXGIObject* _pObject, IDXGIAdapter1** _ppAdapter);
	bool oDXGIGetFactory(IDXGIObject* _pObject, IDXGIFactory1** _ppFactory);

	// List all outputs in DXGI-ordinal order.
	bool oDXGIEnumOutputs(IDXGIFactory* _pFactory, oFUNCTION<bool(unsigned int _AdapterIndex, IDXGIAdapter* _pAdapter, unsigned int _OutputIndex, IDXGIOutput* _pOutput)> _EnumFunction);

	// Find an output with the specified monitor handle
	bool oDXGIFindOutput(IDXGIFactory* _pFactory, HMONITOR _hMonitor, IDXGIOutput** _ppOutput);

	// Find an output that contains the specified virtual desktop coordinate
	bool oDXGIFindOutput(IDXGIFactory* _pFactory, const int2& _VirtualDesktopPosition, IDXGIOutput** _ppOutput);

	// Returns an index fit for use with the oDisplay interface. This is not the
	// same as the index of iteration used for EnumAdapters/EnumOutputs. If not
	// found, this will return oInvalid.
	unsigned int oDXGIFindDisplayIndex(IDXGIOutput* _pOutput);

	// Returns true if the specified format can be bound as a depth-stencil format
	bool oDXGIIsDepthFormat(DXGI_FORMAT _Format);

	// When creating a texture that can be used with a depth-stencil view and also
	// as a shader resource view, the texture should be created with a typeless 
	// flavor of the desired format. This returns the true depth version of that
	// format.
	DXGI_FORMAT oDXGIGetDepthCompatibleFormat(DXGI_FORMAT _TypelessDepthFormat);

	// Return the format necessary to use in a shader resource view when reading
	// from a format used for a depth-stencil view.
	DXGI_FORMAT oDXGIGetColorCompatibleFormat(DXGI_FORMAT _DepthFormat);

#endif
#endif
