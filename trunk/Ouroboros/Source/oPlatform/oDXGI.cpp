/**************************************************************************
 * The MIT License                                                        *
 * Copyright (c) 2013 OOOii.                                              *
 * antony.arciuolo@oooii.com                                              *
 * kevin.myers@oooii.com                                                  *
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
#include <oBasis/oFixedString.h>
#include <oPlatform/oDisplay.h>
#include <oPlatform/oMsgBox.h>
#include <oPlatform/oModule.h>
#include <oPlatform/oSingleton.h>
#include <oPlatform/Windows/oD3D11.h>
#include <oPlatform/Windows/oDXGI.h>
#include <oPlatform/Windows/oWinRect.h>
#include <oPlatform/Windows/oWinWindowing.h>
#include "SoftLink/oWinDXGI.h"

#if oDXVER >= oDXVER_10

// {CF4B314B-E3BC-4DCF-BDE7-86040A0ED295}
static const GUID oWKPDID_PreFullscreenMode = { 0xcf4b314b, 0xe3bc, 0x4dcf, { 0xbd, 0xe7, 0x86, 0x4, 0xa, 0xe, 0xd2, 0x95 } };

const oGUID& oGetGUID(threadsafe const IDXGISwapChain* threadsafe const*) { return (const oGUID&)__uuidof(IDXGISwapChain); }

oSURFACE_FORMAT oDXGIToSurfaceFormat(DXGI_FORMAT _Format)
{
	// @oooii-tony: For now, oSURFACE_FORMAT and DXGI_FORMAT are the same thing.
	return static_cast<oSURFACE_FORMAT>(_Format <= DXGI_FORMAT_BC7_UNORM_SRGB ? _Format : DXGI_FORMAT_UNKNOWN);
}

DXGI_FORMAT oDXGIFromSurfaceFormat(oSURFACE_FORMAT _Format)
{
	if (_Format <= DXGI_FORMAT_BC7_UNORM_SRGB)
		return static_cast<DXGI_FORMAT>(_Format);

	if (oSurfaceFormatIsYUV(_Format) && oSurfaceFormatGetNumSubformats(_Format) > 1)
	{
		// Until DXGI_FORMAT gets its extended YUV formats, assume when using this 
		// API return the dominant plane (usually plane0).
		return oDXGIFromSurfaceFormat(oSurfaceGetSubformat(_Format, 0));
	}

	return DXGI_FORMAT_UNKNOWN;
}

bool oDXGICreateFactory(IDXGIFactory** _ppFactory)
{
	HRESULT hr = oWinDXGI::Singleton()->CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)_ppFactory); 
	if (FAILED(hr))
		return oWinSetLastError(hr);
	return true;
}

bool oDXGICreateSwapChain(IUnknown* _pDevice, bool _Fullscreen, UINT _Width, UINT _Height, DXGI_FORMAT _Format, UINT RefreshRateN, UINT RefreshRateD, HWND _hWnd, bool _EnableGDICompatibility, IDXGISwapChain** _ppSwapChain)
{
	if (!_pDevice)
		return oErrorSetLast(oERROR_INVALID_PARAMETER);

	DXGI_SWAP_CHAIN_DESC d;
	d.BufferDesc.Width = _Width;
	d.BufferDesc.Height = _Height;
	d.BufferDesc.RefreshRate.Numerator = RefreshRateN;
	d.BufferDesc.RefreshRate.Denominator = RefreshRateD;
	d.BufferDesc.Format = _Format;
	d.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	d.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	d.SampleDesc.Count = 1;
	d.SampleDesc.Quality = 0;
	d.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT | DXGI_USAGE_SHADER_INPUT;
	d.BufferCount = 3;
	d.OutputWindow = _hWnd;
	d.Windowed = !_Fullscreen;
	d.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	
	d.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	if (_EnableGDICompatibility)
	{
		#if 1
			// @oooii-tony: 8/31/2011, DX Jun2010 SDK: If DXGI_SWAP_CHAIN_FLAG_GDI_COMPATIBLE 
			// is specified, DXGISwapChain can cause a DXGI_ERROR_DEVICE_REMOVED when 
			// going from fullscreen to windowed, basically indicating a crash in the driver.

			// @oooii-tony: 2/15/2012, DX Jun2010 SDK: This seems not to crash anymore -
			// so it was probably a driver bug. Reenable this for now to play around again
			// with GDI interop to prevent flicker.

			d.Flags |= DXGI_SWAP_CHAIN_FLAG_GDI_COMPATIBLE;
		#else
			oTRACE("GDI compatibility requested, but the code has been disabled");
		#endif
	}
	
	oRef<IDXGIDevice> D3DDevice;
	oVB_RETURN2(_pDevice->QueryInterface(&D3DDevice));

	oRef<IDXGIAdapter> Adapter;
	oVB_RETURN2(D3DDevice->GetAdapter(&Adapter));

	oRef<IDXGIFactory> Factory;
	oVB_RETURN2(Adapter->GetParent(__uuidof(IDXGIFactory), (void**)&Factory));
	oVB_RETURN2(Factory->CreateSwapChain(_pDevice, &d, _ppSwapChain));
	
	// Auto Alt-Enter is nice, but there are threading issues to be concerned 
	// about, so we need to control this explicitly in applications.
	oVB_RETURN2(Factory->MakeWindowAssociation(_hWnd, DXGI_MWA_NO_ALT_ENTER));

	return true;
}

static float oDXGIGetRefreshRate(const DXGI_MODE_DESC& _Mode)
{
	return _Mode.RefreshRate.Numerator / static_cast<float>(_Mode.RefreshRate.Denominator);
}

static DXGI_RATIONAL oDXGIGetRefreshRate(int _RefreshRate)
{
	DXGI_RATIONAL r;
	r.Numerator = _RefreshRate;
	r.Denominator = 1;
	return r;
}

bool oDXGISwapChainResizeBuffers(IDXGISwapChain* _pSwapChain, const int2& _NewSize, HWND _hErrorMsgParent)
{
	DXGI_SWAP_CHAIN_DESC d;
	_pSwapChain->GetDesc(&d);
	HRESULT HR = _pSwapChain->ResizeBuffers(d.BufferCount, _NewSize.x, _NewSize.y, d.BufferDesc.Format, d.Flags);
	if (HR == DXGI_ERROR_INVALID_CALL)
	{
		oMSGBOX_DESC mb;
		mb.ParentNativeHandle = _hErrorMsgParent;
		mb.Type = oMSGBOX_ERR;
		oErrorSetLast(oERROR_REFUSED, "Cannot resize DXGISwapChain buffers because there still are dependent resources in client code. Ensure all dependent resources are freed before resize occurs. The application will be terminated now.");
		oMsgBox(mb, oErrorGetLastString());
		// There's no moving past this error, so terminate...
		std::terminate();
		//return false;
	}
	else if (FAILED(HR))
		return oWinSetLastError(HR);
	return true;
}

bool oDXGISetFullscreenState(IDXGISwapChain* _pSwapChain, const oDXGI_FULLSCREEN_STATE& _State)
{
	// This function is overly complex for a few reasons:
	// DXGI can error out and/or deadlock if not careful, so this function puts on the traing wheels
	// DXGI API is insufficient to get max HW performance. This double-sets render targets to ensure
	// HW fullscreen perf is optimal
	// Restoring windowed mode uses registry settings, not last settings, so support that as well.

	if (!_pSwapChain)
		return oErrorSetLast(oERROR_INVALID_PARAMETER);

	// Confirm we're on the same thread as the message pump
	// http://msdn.microsoft.com/en-us/library/ee417025(v=vs.85).aspx
	// "Multithreading and DXGI"
	DXGI_SWAP_CHAIN_DESC SCDesc;
	_pSwapChain->GetDesc(&SCDesc);

	if (GetCurrentThreadId() != GetWindowThreadProcessId(SCDesc.OutputWindow, nullptr))
		return oErrorSetLast(oERROR_WRONG_THREAD, "oDXGISetFullscreenState called from thread %d for hwnd %x pumping messages on thread %d", GetCurrentThreadId(), SCDesc.OutputWindow, GetWindowThreadProcessId(SCDesc.OutputWindow, nullptr));

	// Go fullscreen on whatever output is the current one (client code should position window on 
	// output for fullscreen first).
	oRef<IDXGIOutput> Output;
	HRESULT HR = _pSwapChain->GetContainingOutput(&Output);
	if (HR == DXGI_ERROR_UNSUPPORTED)
	{
		RECT r;
		oVB(GetWindowRect(SCDesc.OutputWindow, &r));
		oRef<IDXGIFactory1> Factory;
		oVERIFY(oDXGIGetFactory(_pSwapChain, &Factory));
		oRef<IDXGIOutput> Output;
		oVERIFY(oDXGIFindOutput(Factory, oWinRectPosition(r), &Output));
		oRef<IDXGIAdapter1> Adapter;
		oVERIFY(oDXGIGetAdapter(Output, &Adapter));
		DXGI_ADAPTER_DESC1 adesc;
		Adapter->GetDesc1(&adesc);

		if (FAILED(_pSwapChain->GetContainingOutput(&Output)))
		{
			oStringL WinTitle;
			oWinGetText(WinTitle.c_str(), WinTitle.capacity(), SCDesc.OutputWindow);
			oErrorSetLast(oERROR_REFUSED, "SetFullscreenState failed on adapter \"%s\" because the IDXGISwapChain created with the associated window entitled \"%s\" was created with another adapter. Cross-adapter exclusive mode is not currently (DXGI 1.1) supported.", oStringS(adesc.Description), WinTitle);
		}
		else 
			oErrorSetLast(oERROR_INVALID_PARAMETER, "SetFullscreenState failed though the attempt was on the same adapters with which the swapchain was created");

		return false;
	}

	else oVB_RETURN2(HR);

	DXGI_OUTPUT_DESC ODesc;
	Output->GetDesc(&ODesc);

	if (_State.Fullscreen && _State.RememberCurrentSettings)
	{
		oDISPLAY_MODE mode;
		mode.Size = oWinRectSize(ODesc.DesktopCoordinates);
		mode.RefreshRate = static_cast<int>(oDXGIGetRefreshRate(SCDesc.BufferDesc));
		oVB_RETURN2(_pSwapChain->SetPrivateData(oWKPDID_PreFullscreenMode, sizeof(mode), &mode));
	}

	SCDesc.BufferDesc.Width = _State.Size.x == oDEFAULT ? oWinRectW(ODesc.DesktopCoordinates) : _State.Size.x;
	SCDesc.BufferDesc.Height = _State.Size.y == oDEFAULT ? oWinRectH(ODesc.DesktopCoordinates) : _State.Size.y;

	if (_State.RefreshRate == oDEFAULT)
		SCDesc.BufferDesc.RefreshRate = oDXGIGetRefreshRate(0);
	else
		SCDesc.BufferDesc.RefreshRate = oDXGIGetRefreshRate(_State.RefreshRate);

	DXGI_MODE_DESC closestMatch;
	oV(Output->FindClosestMatchingMode(&SCDesc.BufferDesc, &closestMatch, nullptr));

	if (closestMatch.Width != SCDesc.BufferDesc.Width || closestMatch.Height != SCDesc.BufferDesc.Height || oEqual(oDXGIGetRefreshRate(SCDesc.BufferDesc), oDXGIGetRefreshRate(closestMatch)))
		oTRACE("SetFullscreenState initiated by HWND 0x%x asked for %dx%d@%dHz and will instead set the closest match %dx%d@%.02fHz", SCDesc.OutputWindow, SCDesc.BufferDesc.Width, SCDesc.BufferDesc.Height, SCDesc.BufferDesc.RefreshRate.Numerator, closestMatch.Width, closestMatch.Height, oDXGIGetRefreshRate(closestMatch));

	// Move to 0,0 on the screen because resize targets will resize the window,
	// in which case the evaluation of which output the window is on will change.
	oWinSetPosition(SCDesc.OutputWindow, int2(ODesc.DesktopCoordinates.left, ODesc.DesktopCoordinates.top));

	if (_State.Fullscreen)
	{
		_pSwapChain->ResizeBuffers(SCDesc.BufferCount, closestMatch.Width, closestMatch.Height, SCDesc.BufferDesc.Format, SCDesc.Flags);
		oVB_RETURN2(_pSwapChain->ResizeTarget(&closestMatch));
	}

	oVB_RETURN2(_pSwapChain->SetFullscreenState(_State.Fullscreen, nullptr));
	// Ensure the refresh rate is matched against the fullscreen mode
	// http://msdn.microsoft.com/en-us/library/ee417025(v=vs.85).aspx
	// "Full-Screen Issues"
	if (_State.Fullscreen)
	{
		_pSwapChain->GetDesc(&SCDesc);
		SCDesc.BufferDesc.RefreshRate.Numerator = 0;
		SCDesc.BufferDesc.RefreshRate.Denominator = 0;
		oVB_RETURN2(_pSwapChain->ResizeTarget(&SCDesc.BufferDesc));
	}

	else
	{
		int DispIndex = oDXGIFindDisplayIndex(Output);

		if (_State.RememberCurrentSettings)
		{
			// Restore prior resolution (not the one necessarily saved in the registry)
			oDISPLAY_MODE mode;
			UINT size;
			oVB_RETURN2(_pSwapChain->GetPrivateData(oWKPDID_PreFullscreenMode, &size, &mode));
			if (size != sizeof(oDISPLAY_MODE))
				return oErrorSetLast(oERROR_INVALID_PARAMETER, "Failed to restore prior display state: size retrieved (%u) does not match size requested (%u)", size, sizeof(oDISPLAY_MODE));

			#ifdef _DEBUG
				oStringS rate;
				if (mode.RefreshRate == oDEFAULT)
					oPrintf(rate, "defaultHz");
				else
					oPrintf(rate, "%dHz", mode.RefreshRate);
				oTRACE("Restoring prior display mode of %dx%d@%s", mode.Size.x, mode.Size.y, rate.c_str());
			#endif

			oVERIFY(oDisplaySetMode(DispIndex, mode));
		}

		else
			oVERIFY(oDisplayResetMode(DispIndex));
	}

	// If we're calling a function called "SetFullscreenState", it'd be nice if 
	// the swapchain/window was actually IN that state when we came out of this 
	// call, so sit here and don't allow anyone else to be tricksy until we've
	// flushed the SetFullScreenState messages.
	oWinFlushMessages(SCDesc.OutputWindow, 30000);
	return true;
}

struct oSCREEN_MODE
{
	int2 Size;
	int RefreshRate;
};

bool oDXGIGetDC(ID3D11RenderTargetView* _pRTV, HDC* _phDC)
{
	oRef<ID3D11Resource> RT;
	_pRTV->GetResource(&RT);
	oRef<IDXGISurface1> DXGISurface;
	oVB_RETURN2(RT->QueryInterface(&DXGISurface));
	oVB_RETURN2(DXGISurface->GetDC(false, _phDC));
	return true;
}

bool oDXGIReleaseDC(ID3D11RenderTargetView* _pRTV, RECT* _pDirtyRect)
{
	oRef<ID3D11Resource> RT;
	_pRTV->GetResource(&RT);
	oRef<IDXGISurface1> DXGISurface;
	oVB_RETURN2(RT->QueryInterface(&DXGISurface));
	oVB_RETURN2(DXGISurface->ReleaseDC(_pDirtyRect));
	return true;
}

bool oDXGIGetDC(IDXGISwapChain* _pSwapChain, HDC* _phDC)
{
	oRef<ID3D11Texture2D> RT;
	oVB_RETURN2(_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&RT));
	oRef<IDXGISurface1> DXGISurface;
	oVB_RETURN2(RT->QueryInterface(&DXGISurface));
	oVB_RETURN2(DXGISurface->GetDC(false, _phDC));

	return true;
}

bool oDXGIReleaseDC(IDXGISwapChain* _pSwapChain, RECT* _pDirtyRect)
{
	oRef<ID3D11Texture2D> RT;
	oVB_RETURN2(_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&RT));
	oRef<IDXGISurface1> DXGISurface;
	oVB_RETURN2(RT->QueryInterface(&DXGISurface));
	oVB_RETURN2(DXGISurface->ReleaseDC(_pDirtyRect));
	return true;
}

bool oDXGIGetFeatureLevel(IDXGIAdapter* _pAdapter, D3D_FEATURE_LEVEL* _pFeatureLevel)
{
#if D3D11_MAJOR_VERSION
	// @oooii-tony: As of 8/29/2012 I get "Microsoft C++ exception: _com_error 
	// at memory location" every time CheckInterfaceSupport is called. I 
	// confirmed this occurs in DirectX sample code. try/catch doesn't quiet it 
	// either, so just don't call it and take the heavyweight action of creating 
	// the device an seeing what happens. If that fails, fall back to the no-D3D11 
	// version.
	//LARGE_INTEGER li;
	//if (_pAdapter->CheckInterfaceSupport(__uuidof(ID3D11Device), &li))
	//	return true;
	
	// Note that the out-device is null, thus this isn't that expensive a call
	if (FAILED(oD3D11::Singleton()->D3D11CreateDevice(
		_pAdapter
		, D3D_DRIVER_TYPE_UNKNOWN
		, nullptr
		, D3D11_CREATE_DEVICE_DEBUG // squelches a warning
		, nullptr
		, 0
		, D3D11_SDK_VERSION
		, nullptr
		, _pFeatureLevel
		, nullptr)))
		return false;
	return true;
#else
	return false;
#endif
}

bool oDXGIEnumAdapters(const oFUNCTION<bool(int _AdapterIndex, IDXGIAdapter* _pAdapter, const oDISPLAY_ADAPTER_DRIVER_DESC& _DriverDesc)>& _Enumerator, IDXGIFactory* _pFactory)
{
	oRef<IDXGIFactory> Factory = _pFactory;
	if (!Factory)
		oDXGICreateFactory(&Factory);

	int AdapterIndex = 0;
	oDISPLAY_ADAPTER_DRIVER_DESC add;
	oRef<IDXGIAdapter> Adapter;
	while (DXGI_ERROR_NOT_FOUND != Factory->EnumAdapters(AdapterIndex, &Adapter))
	{
		oDXGIGetAdapterDriverDesc(Adapter, &add);
		if (!_Enumerator(AdapterIndex, Adapter, add))
			return true;
		AdapterIndex++;
		Adapter = nullptr;
	}

	return true;
}

void oDXGIGetAdapterDriverDesc(IDXGIAdapter* _pAdapter, oDISPLAY_ADAPTER_DRIVER_DESC* _pDesc)
{
	DXGI_ADAPTER_DESC ad;
	_pAdapter->GetDesc(&ad);

	oWinEnumVideoDriverDesc([&](const oDISPLAY_ADAPTER_DRIVER_DESC& _Desc)
	{
		oStringM vendor, device;
		oPrintf(vendor, "VEN_%X", ad.VendorId);
		oPrintf(device, "DEV_%04X", ad.DeviceId);
		if (strstr(_Desc.PlugNPlayID, vendor) && strstr(_Desc.PlugNPlayID, device))
			*_pDesc = _Desc;
	});
}

int oDXGIGetAdapterIndex(IDXGIAdapter* _pAdapter)
{
	oRef<IDXGIFactory> Factory;
	oV(_pAdapter->GetParent(__uuidof(IDXGIFactory), (void**)&Factory));

	DXGI_ADAPTER_DESC ad, testDesc;
	_pAdapter->GetDesc(&ad);
	int Index = oInvalid;
	oDXGIEnumAdapters([&](int _AdapterIndex, IDXGIAdapter* _pTestAdapter, const oDISPLAY_ADAPTER_DRIVER_DESC& _DriverDesc)->bool
	{
		_pTestAdapter->GetDesc(&testDesc);
		if (!memcmp(&ad.AdapterLuid, &testDesc.AdapterLuid, sizeof(LUID)))
		{
			Index = oInt(_AdapterIndex);
			return false;
		}
		return true;
	}, Factory);

	return Index;
}

oVersion oDXGIGetInterfaceVersion(IDXGIAdapter* _pAdapter)
{
	LARGE_INTEGER li;
	#if D3D11_MAJOR_VERSION
		D3D_FEATURE_LEVEL FeatureLevel;
		if (oDXGIGetFeatureLevel(_pAdapter, &FeatureLevel)) return oVersion(11,0);
	#endif
	#ifdef _D3D10_1_CONSTANTS
		if (_pAdapter->CheckInterfaceSupport(__uuidof(ID3D10Device1), &li)) return oVersion(10,1);
	#endif
	#ifdef _D3D10_CONSTANTS
		if (_pAdapter->CheckInterfaceSupport(__uuidof(ID3D10Device), &li)) return oVersion(10,0);
	#endif
	return oVersion();
}

oVersion oDXGIGetFeatureLevel(IDXGIAdapter* _pAdapter)
{
#if D3D11_MAJOR_VERSION
	D3D_FEATURE_LEVEL FeatureLevel;
	if (oDXGIGetFeatureLevel(_pAdapter, &FeatureLevel))
		return oD3D11GetFeatureVersion(FeatureLevel);
#endif
	return oDXGIGetInterfaceVersion(_pAdapter);
}

bool oDXGIEnumOutputs(const oFUNCTION<bool(int _AdapterIndex, IDXGIAdapter* _pAdapter, int _OutputIndex, IDXGIOutput* _pOutput)>& _Enumerator, IDXGIFactory* _pFactory)
{
	oRef<IDXGIFactory> Factory = _pFactory;

	if (!Factory)
		oDXGICreateFactory(&Factory);

	oDXGIEnumAdapters([&](int _AdapterIndex, IDXGIAdapter* _pAdapter, const oDISPLAY_ADAPTER_DRIVER_DESC& _DriverDesc)->bool
	{
		oRef<IDXGIOutput> Output;
		int o = 0;
		while (DXGI_ERROR_NOT_FOUND != _pAdapter->EnumOutputs(o, &Output))
		{
			if (!_Enumerator(_AdapterIndex, _pAdapter, o, Output))
				return false;
			o++;
		}
		return true;
	}, Factory);

	return true;
}

static bool MatchHMONITOR(int _AdapterIndex, IDXGIAdapter* _pAdapter, int _OutputIndex, IDXGIOutput* _pOutput, HMONITOR _hMonitor, IDXGIOutput** _ppFoundOutput)
{
	DXGI_OUTPUT_DESC desc;
	_pOutput->GetDesc(&desc);
	if (desc.Monitor == _hMonitor)
	{
		_pOutput->AddRef();
		*_ppFoundOutput = _pOutput;
		return false;
	}

	return true;
}

static bool MatchPosition(int _AdapterIndex, IDXGIAdapter* _pAdapter, int _OutputIndex, IDXGIOutput* _pOutput, const int2& _VirtualDesktopPosition, IDXGIOutput** _ppFoundOutput)
{
	DXGI_OUTPUT_DESC desc;
	_pOutput->GetDesc(&desc);
	if (oWinRectContains(desc.DesktopCoordinates, _VirtualDesktopPosition))
	{
		_pOutput->AddRef();
		*_ppFoundOutput = _pOutput;
		return false;
	}

	return true;
}

bool oDXGIFindOutput(IDXGIFactory* _pFactory, HMONITOR _hMonitor, IDXGIOutput** _ppOutput)
{
	bool result = oDXGIEnumOutputs(oBIND(MatchHMONITOR, oBIND1, oBIND2, oBIND3, oBIND4, _hMonitor, _ppOutput), _pFactory);
	return result && !!*_ppOutput;
}

bool oDXGIFindOutput(IDXGIFactory* _pFactory, HWND _hWnd, IDXGIOutput** _ppOutput)
{
	return oDXGIFindOutput(_pFactory, MonitorFromWindow(_hWnd, MONITOR_DEFAULTTONEAREST), _ppOutput);
}

bool oDXGIFindOutput(IDXGIFactory* _pFactory, const int2& _VirtualDesktopPosition, IDXGIOutput** _ppOutput)
{
	bool result = oDXGIEnumOutputs(oBIND(MatchPosition, oBIND1, oBIND2, oBIND3, oBIND4, oBINDREF(_VirtualDesktopPosition), _ppOutput), _pFactory);
#ifdef _DEBUG
	auto Output = *_ppOutput;
	if(Output)
	{
		DXGI_OUTPUT_DESC OutDesc;
		Output->GetDesc(&OutDesc);
		oASSERT(oContains(oRect(OutDesc.DesktopCoordinates), _VirtualDesktopPosition), "oDXGIEnumOutputs Failed to find correct output");
	}
#endif
	return result && !!*_ppOutput;
}

bool oDXGIGetAdapter(IDXGIObject* _pObject, IDXGIAdapter** _ppAdapter)
{
	if (_pObject)
	{
		oVB_RETURN2(_pObject->GetParent(__uuidof(IDXGIAdapter), (void**)_ppAdapter));
		return true;
	}

	return false;
}

bool oDXGIGetAdapter(IDXGIObject* _pObject, IDXGIAdapter1** _ppAdapter)
{
	if (_pObject)
	{
		oVB_RETURN2(_pObject->GetParent(__uuidof(IDXGIAdapter1), (void**)_ppAdapter));
		return true;
	}

	return false;
}

bool oDXGIGetFactory(IDXGIObject* _pObject, IDXGIFactory** _ppFactory)
{
	if (_pObject)
	{
		oVB_RETURN2(_pObject->GetParent(__uuidof(IDXGIFactory), (void**)_ppFactory));
		return true;
	}

	return false;
}

bool oDXGIGetFactory(IDXGIObject* _pObject, IDXGIFactory1** _ppFactory)
{
	if (_pObject)
	{
		oVB_RETURN2(_pObject->GetParent(__uuidof(IDXGIFactory1), (void**)_ppFactory));
		return true;
	}

	return false;
}

int oDXGIFindDisplayIndex(IDXGIOutput* _pOutput)
{
	DXGI_OUTPUT_DESC odesc;
	_pOutput->GetDesc(&odesc);
	
	oDISPLAY_DESC ddesc;

	int index = 0;
	while (oDisplayEnum(index, &ddesc))
	{
		if ((HMONITOR)ddesc.NativeHandle == odesc.Monitor)
			return index;
		index++;
	}

	oErrorSetLast(oERROR_NOT_FOUND);
	return oInvalid;
}

bool oDXGIIsDepthFormat(DXGI_FORMAT _Format)
{
	switch (_Format)
	{
		case DXGI_FORMAT_D32_FLOAT:
		case DXGI_FORMAT_R32_TYPELESS:
		case DXGI_FORMAT_R24G8_TYPELESS:
		case DXGI_FORMAT_D24_UNORM_S8_UINT:
		case DXGI_FORMAT_R24_UNORM_X8_TYPELESS:
		case DXGI_FORMAT_D32_FLOAT_S8X24_UINT:
		case DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS:
		case DXGI_FORMAT_D16_UNORM:
		case DXGI_FORMAT_R16_TYPELESS: return true;
		default: return false;
	}
}

void oDXGIGetCompatibleFormats(DXGI_FORMAT _DesiredFormat, DXGI_FORMAT* _pTextureFormat, DXGI_FORMAT* _pDepthStencilViewFormat, DXGI_FORMAT* _pShaderResourceViewFormat)
{
	switch (_DesiredFormat)
	{
		case DXGI_FORMAT_R24G8_TYPELESS:
		case DXGI_FORMAT_D24_UNORM_S8_UINT:
		case DXGI_FORMAT_R24_UNORM_X8_TYPELESS:
			*_pTextureFormat = DXGI_FORMAT_R24G8_TYPELESS;
			*_pDepthStencilViewFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
			*_pShaderResourceViewFormat = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
			break;

		case DXGI_FORMAT_D32_FLOAT:
		case DXGI_FORMAT_R32_TYPELESS:
			*_pTextureFormat = DXGI_FORMAT_R32_TYPELESS;
			*_pDepthStencilViewFormat = DXGI_FORMAT_D32_FLOAT;
			*_pShaderResourceViewFormat = DXGI_FORMAT_R32_FLOAT;
			break;

		case DXGI_FORMAT_D32_FLOAT_S8X24_UINT:
		case DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS:
			*_pTextureFormat = DXGI_FORMAT_UNKNOWN;
			*_pDepthStencilViewFormat = DXGI_FORMAT_UNKNOWN;
			*_pShaderResourceViewFormat = DXGI_FORMAT_UNKNOWN;
			break;

		case DXGI_FORMAT_D16_UNORM:
		case DXGI_FORMAT_R16_TYPELESS:
			*_pTextureFormat = DXGI_FORMAT_R16_TYPELESS;
			*_pDepthStencilViewFormat = DXGI_FORMAT_D16_UNORM;
			*_pShaderResourceViewFormat = DXGI_FORMAT_R16_UNORM;
			break;

		default:
			*_pTextureFormat = _DesiredFormat;
			*_pDepthStencilViewFormat = _DesiredFormat;
			*_pShaderResourceViewFormat = _DesiredFormat;
			break;
	}
}

const char* oAsString(DXGI_FORMAT _Format)
{
	switch (_Format)
	{
		case DXGI_FORMAT_UNKNOWN: return "DXGI_FORMAT_UNKNOWN";
		case DXGI_FORMAT_R32G32B32A32_TYPELESS: return "DXGI_FORMAT_R32G32B32A32_TYPELESS";
		case DXGI_FORMAT_R32G32B32A32_FLOAT: return "DXGI_FORMAT_R32G32B32A32_FLOAT";
		case DXGI_FORMAT_R32G32B32A32_UINT: return "DXGI_FORMAT_R32G32B32A32_UINT";
		case DXGI_FORMAT_R32G32B32A32_SINT: return "DXGI_FORMAT_R32G32B32A32_SINT";
		case DXGI_FORMAT_R32G32B32_TYPELESS: return "DXGI_FORMAT_R32G32B32_TYPELESS";
		case DXGI_FORMAT_R32G32B32_FLOAT: return "DXGI_FORMAT_R32G32B32_FLOAT";
		case DXGI_FORMAT_R32G32B32_UINT: return "DXGI_FORMAT_R32G32B32_UINT";
		case DXGI_FORMAT_R32G32B32_SINT: return "DXGI_FORMAT_R32G32B32_SINT";
		case DXGI_FORMAT_R16G16B16A16_TYPELESS: return "DXGI_FORMAT_R16G16B16A16_TYPELESS";
		case DXGI_FORMAT_R16G16B16A16_FLOAT: return "DXGI_FORMAT_R16G16B16A16_FLOAT";
		case DXGI_FORMAT_R16G16B16A16_UNORM: return "DXGI_FORMAT_R16G16B16A16_UNORM";
		case DXGI_FORMAT_R16G16B16A16_UINT: return "DXGI_FORMAT_R16G16B16A16_UINT";
		case DXGI_FORMAT_R16G16B16A16_SNORM: return "DXGI_FORMAT_R16G16B16A16_SNORM";
		case DXGI_FORMAT_R16G16B16A16_SINT: return "DXGI_FORMAT_R16G16B16A16_SINT";
		case DXGI_FORMAT_R32G32_TYPELESS: return "DXGI_FORMAT_R32G32_TYPELESS";
		case DXGI_FORMAT_R32G32_FLOAT: return "DXGI_FORMAT_R32G32_FLOAT";
		case DXGI_FORMAT_R32G32_UINT: return "DXGI_FORMAT_R32G32_UINT";
		case DXGI_FORMAT_R32G32_SINT: return "DXGI_FORMAT_R32G32_SINT";
		case DXGI_FORMAT_R32G8X24_TYPELESS: return "DXGI_FORMAT_R32G8X24_TYPELESS";
		case DXGI_FORMAT_D32_FLOAT_S8X24_UINT: return "DXGI_FORMAT_D32_FLOAT_S8X24_UINT";
		case DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS: return "DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS";
		case DXGI_FORMAT_X32_TYPELESS_G8X24_UINT: return "DXGI_FORMAT_X32_TYPELESS_G8X24_UINT";
		case DXGI_FORMAT_R10G10B10A2_TYPELESS: return "DXGI_FORMAT_R10G10B10A2_TYPELESS";
		case DXGI_FORMAT_R10G10B10A2_UNORM: return "DXGI_FORMAT_R10G10B10A2_UNORM";
		case DXGI_FORMAT_R10G10B10A2_UINT: return "DXGI_FORMAT_R10G10B10A2_UINT";
		case DXGI_FORMAT_R11G11B10_FLOAT: return "DXGI_FORMAT_R11G11B10_FLOAT";
		case DXGI_FORMAT_R8G8B8A8_TYPELESS: return "DXGI_FORMAT_R8G8B8A8_TYPELESS";
		case DXGI_FORMAT_R8G8B8A8_UNORM: return "DXGI_FORMAT_R8G8B8A8_UNORM";
		case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB: return "DXGI_FORMAT_R8G8B8A8_UNORM_SRGB";
		case DXGI_FORMAT_R8G8B8A8_UINT: return "DXGI_FORMAT_R8G8B8A8_UINT";
		case DXGI_FORMAT_R8G8B8A8_SNORM: return "DXGI_FORMAT_R8G8B8A8_SNORM";
		case DXGI_FORMAT_R8G8B8A8_SINT: return "DXGI_FORMAT_R8G8B8A8_SINT";
		case DXGI_FORMAT_R16G16_TYPELESS: return "DXGI_FORMAT_R16G16_TYPELESS";
		case DXGI_FORMAT_R16G16_FLOAT: return "DXGI_FORMAT_R16G16_FLOAT";
		case DXGI_FORMAT_R16G16_UNORM: return "DXGI_FORMAT_R16G16_UNORM";
		case DXGI_FORMAT_R16G16_UINT: return "DXGI_FORMAT_R16G16_UINT";
		case DXGI_FORMAT_R16G16_SNORM: return "DXGI_FORMAT_R16G16_SNORM";
		case DXGI_FORMAT_R16G16_SINT: return "DXGI_FORMAT_R16G16_SINT";
		case DXGI_FORMAT_R32_TYPELESS: return "DXGI_FORMAT_R32_TYPELESS";
		case DXGI_FORMAT_D32_FLOAT: return "DXGI_FORMAT_D32_FLOAT";
		case DXGI_FORMAT_R32_FLOAT: return "DXGI_FORMAT_R32_FLOAT";
		case DXGI_FORMAT_R32_UINT: return "DXGI_FORMAT_R32_UINT";
		case DXGI_FORMAT_R32_SINT: return "DXGI_FORMAT_R32_SINT";
		case DXGI_FORMAT_R24G8_TYPELESS: return "DXGI_FORMAT_R24G8_TYPELESS";
		case DXGI_FORMAT_D24_UNORM_S8_UINT: return "DXGI_FORMAT_D24_UNORM_S8_UINT";
		case DXGI_FORMAT_R24_UNORM_X8_TYPELESS: return "DXGI_FORMAT_R24_UNORM_X8_TYPELESS";
		case DXGI_FORMAT_X24_TYPELESS_G8_UINT: return "DXGI_FORMAT_X24_TYPELESS_G8_UINT";
		case DXGI_FORMAT_R8G8_TYPELESS: return "DXGI_FORMAT_R8G8_TYPELESS";
		case DXGI_FORMAT_R8G8_UNORM: return "DXGI_FORMAT_R8G8_UNORM";
		case DXGI_FORMAT_R8G8_UINT: return "DXGI_FORMAT_R8G8_UINT";
		case DXGI_FORMAT_R8G8_SNORM: return "DXGI_FORMAT_R8G8_SNORM";
		case DXGI_FORMAT_R8G8_SINT: return "DXGI_FORMAT_R8G8_SINT";
		case DXGI_FORMAT_R16_TYPELESS: return "DXGI_FORMAT_R16_TYPELESS";
		case DXGI_FORMAT_R16_FLOAT: return "DXGI_FORMAT_R16_FLOAT";
		case DXGI_FORMAT_D16_UNORM: return "DXGI_FORMAT_D16_UNORM";
		case DXGI_FORMAT_R16_UNORM: return "DXGI_FORMAT_R16_UNORM";
		case DXGI_FORMAT_R16_UINT: return "DXGI_FORMAT_R16_UINT";
		case DXGI_FORMAT_R16_SNORM: return "DXGI_FORMAT_R16_SNORM";
		case DXGI_FORMAT_R16_SINT: return "DXGI_FORMAT_R16_SINT";
		case DXGI_FORMAT_R8_TYPELESS: return "DXGI_FORMAT_R8_TYPELESS";
		case DXGI_FORMAT_R8_UNORM: return "DXGI_FORMAT_R8_UNORM";
		case DXGI_FORMAT_R8_UINT: return "DXGI_FORMAT_R8_UINT";
		case DXGI_FORMAT_R8_SNORM: return "DXGI_FORMAT_R8_SNORM";
		case DXGI_FORMAT_R8_SINT: return "DXGI_FORMAT_R8_SINT";
		case DXGI_FORMAT_A8_UNORM: return "DXGI_FORMAT_A8_UNORM";
		case DXGI_FORMAT_R1_UNORM: return "DXGI_FORMAT_R1_UNORM";
		case DXGI_FORMAT_R9G9B9E5_SHAREDEXP: return "DXGI_FORMAT_R9G9B9E5_SHAREDEXP";
		case DXGI_FORMAT_R8G8_B8G8_UNORM: return "DXGI_FORMAT_R8G8_B8G8_UNORM";
		case DXGI_FORMAT_G8R8_G8B8_UNORM: return "DXGI_FORMAT_G8R8_G8B8_UNORM";
		case DXGI_FORMAT_BC1_TYPELESS: return "DXGI_FORMAT_BC1_TYPELESS";
		case DXGI_FORMAT_BC1_UNORM: return "DXGI_FORMAT_BC1_UNORM";
		case DXGI_FORMAT_BC1_UNORM_SRGB: return "DXGI_FORMAT_BC1_UNORM_SRGB";
		case DXGI_FORMAT_BC2_TYPELESS: return "DXGI_FORMAT_BC2_TYPELESS";
		case DXGI_FORMAT_BC2_UNORM: return "DXGI_FORMAT_BC2_UNORM";
		case DXGI_FORMAT_BC2_UNORM_SRGB: return "DXGI_FORMAT_BC2_UNORM_SRGB";
		case DXGI_FORMAT_BC3_TYPELESS: return "DXGI_FORMAT_BC3_TYPELESS";
		case DXGI_FORMAT_BC3_UNORM: return "DXGI_FORMAT_BC3_UNORM";
		case DXGI_FORMAT_BC3_UNORM_SRGB: return "DXGI_FORMAT_BC3_UNORM_SRGB";
		case DXGI_FORMAT_BC4_TYPELESS: return "DXGI_FORMAT_BC4_TYPELESS";
		case DXGI_FORMAT_BC4_UNORM: return "DXGI_FORMAT_BC4_UNORM";
		case DXGI_FORMAT_BC4_SNORM: return "DXGI_FORMAT_BC4_SNORM";
		case DXGI_FORMAT_BC5_TYPELESS: return "DXGI_FORMAT_BC5_TYPELESS";
		case DXGI_FORMAT_BC5_UNORM: return "DXGI_FORMAT_BC5_UNORM";
		case DXGI_FORMAT_BC5_SNORM: return "DXGI_FORMAT_BC5_SNORM";
		case DXGI_FORMAT_B5G6R5_UNORM: return "DXGI_FORMAT_B5G6R5_UNORM";
		case DXGI_FORMAT_B5G5R5A1_UNORM: return "DXGI_FORMAT_B5G5R5A1_UNORM";
		case DXGI_FORMAT_B8G8R8A8_UNORM: return "DXGI_FORMAT_B8G8R8A8_UNORM";
		case DXGI_FORMAT_B8G8R8X8_UNORM: return "DXGI_FORMAT_B8G8R8X8_UNORM";
		case DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM: return "DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM";
		case DXGI_FORMAT_B8G8R8A8_TYPELESS: return "DXGI_FORMAT_B8G8R8A8_TYPELESS";
		case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB: return "DXGI_FORMAT_B8G8R8A8_UNORM_SRGB";
		case DXGI_FORMAT_B8G8R8X8_TYPELESS: return "DXGI_FORMAT_B8G8R8X8_TYPELESS";
		case DXGI_FORMAT_B8G8R8X8_UNORM_SRGB: return "DXGI_FORMAT_B8G8R8X8_UNORM_SRGB";
		case DXGI_FORMAT_BC6H_TYPELESS: return "DXGI_FORMAT_BC6H_TYPELESS";
		case DXGI_FORMAT_BC6H_UF16: return "DXGI_FORMAT_BC6H_UF16";
		case DXGI_FORMAT_BC6H_SF16: return "DXGI_FORMAT_BC6H_SF16";
		case DXGI_FORMAT_BC7_TYPELESS: return "DXGI_FORMAT_BC7_TYPELESS";
		case DXGI_FORMAT_BC7_UNORM: return "DXGI_FORMAT_BC7_UNORM";
		case DXGI_FORMAT_BC7_UNORM_SRGB: return "DXGI_FORMAT_BC7_UNORM_SRGB";
		//case DXGI_FORMAT_AYUV: return "DXGI_FORMAT_AYUV";
		//case DXGI_FORMAT_Y410: return "DXGI_FORMAT_Y410";
		//case DXGI_FORMAT_Y416: return "DXGI_FORMAT_Y416";
		//case DXGI_FORMAT_NV12: return "DXGI_FORMAT_NV12";
		//case DXGI_FORMAT_P010: return "DXGI_FORMAT_P010";
		//case DXGI_FORMAT_P016: return "DXGI_FORMAT_P016";
		//case DXGI_FORMAT_420_OPAQUE: return "DXGI_FORMAT_420_OPAQUE";
		//case DXGI_FORMAT_YUY2: return "DXGI_FORMAT_YUY2";
		//case DXGI_FORMAT_Y210: return "DXGI_FORMAT_Y210";
		//case DXGI_FORMAT_Y216: return "DXGI_FORMAT_Y216";
		//case DXGI_FORMAT_NV11: return "DXGI_FORMAT_NV11";
		//case DXGI_FORMAT_AI44: return "DXGI_FORMAT_AI44";
		//case DXGI_FORMAT_IA44: return "DXGI_FORMAT_IA44";
		//case DXGI_FORMAT_P8: return "DXGI_FORMAT_P8";
		//case DXGI_FORMAT_A8P8: return "DXGI_FORMAT_A8P8";
		//case DXGI_FORMAT_B4G4R4A4_UNORM: return "DXGI_FORMAT_B4G4R4A4_UNORM";
		oNODEFAULT;
	}
}

#endif // oDXVER >= oDXVER_10
