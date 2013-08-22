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
#include <oStd/fixed_string.h>
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

bool oDXGICreateSwapChain(IUnknown* _pDevice, bool _Fullscreen, UINT _Width, UINT _Height, bool _AutochangeMonitorResolution, DXGI_FORMAT _Format, UINT RefreshRateN, UINT RefreshRateD, HWND _hWnd, bool _EnableGDICompatibility, IDXGISwapChain** _ppSwapChain)
{
	if (!_pDevice)
		return oErrorSetLast(std::errc::invalid_argument);

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
	
	d.Flags = 0;
	
	if (_AutochangeMonitorResolution)
		d.Flags |= DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	if (_EnableGDICompatibility)
		d.Flags |= DXGI_SWAP_CHAIN_FLAG_GDI_COMPATIBLE;
	
	oRef<IDXGIDevice> D3DDevice;
	oVB_RETURN2(_pDevice->QueryInterface(&D3DDevice));

	oRef<IDXGIAdapter> Adapter;
	oVB_RETURN2(D3DDevice->GetAdapter(&Adapter));

	oRef<IDXGIFactory> Factory;
	oVB_RETURN2(Adapter->GetParent(__uuidof(IDXGIFactory), (void**)&Factory));
	oVB_RETURN2(Factory->CreateSwapChain(_pDevice, &d, _ppSwapChain));
	
	// DXGI_MWA_NO_ALT_ENTER seems bugged from comments at bottom of this link:
	// http://stackoverflow.com/questions/2353178/disable-alt-enter-in-a-direct3d-directx-application
	oVB_RETURN2(Factory->MakeWindowAssociation(_hWnd, DXGI_MWA_NO_WINDOW_CHANGES|DXGI_MWA_NO_ALT_ENTER));

	return true;
}

bool oDXGISwapChainResizeBuffers(IDXGISwapChain* _pSwapChain, const int2& _NewSize, HWND _hErrorMsgParent)
{
	DXGI_SWAP_CHAIN_DESC d;
	_pSwapChain->GetDesc(&d);
	HRESULT HR = _pSwapChain->ResizeBuffers(d.BufferCount, _NewSize.x, _NewSize.y, d.BufferDesc.Format, d.Flags);
	_pSwapChain->GetDesc(&d);
	if (HR == DXGI_ERROR_INVALID_CALL)
	{
		oErrorSetLast(std::errc::permission_denied, "Cannot resize DXGISwapChain buffers because there still are dependent resources in client code. Ensure all dependent resources are freed before resize occurs. The application will be terminated now.");
		oMsgBox(oMSGBOX_DESC(oMSGBOX_ERR, nullptr, (oGUI_WINDOW)_hErrorMsgParent), oErrorGetLastString());
		// There's no moving past this error, so terminate...
		std::terminate();
		//return false;
	}
	else if (FAILED(HR))
		return oWinSetLastError(HR);
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
	//oTRACE("GetDC() exception below (if it happens) cannot be try-catch caught, so ignore it or don't use GDI drawing.");
	oVB_RETURN2(DXGISurface->GetDC(false, _phDC));
	return true;
}

bool oDXGIReleaseDC(IDXGISwapChain* _pSwapChain, RECT* _pDirtyRect)
{
	oRef<ID3D11Texture2D> RT;
	oVB_RETURN2(_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&RT));
	oRef<IDXGISurface1> DXGISurface;
	oVB_RETURN2(RT->QueryInterface(&DXGISurface));
	//oTRACE("ReleaseDC() exception below (if it happens) cannot be try-catch caught, so ignore it or don't use GDI drawing.");
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
	// @oooii-jeffrey: Apparently D3D11_CREATE_DEVICE_DEBUG can trigger the 
	// _com_error if the debug libraries of DirectX are not (or not properly)
	// installed. Documentation however does not mention the need for this
	// flag to get the feature level, so passing 0 seems to fix that problem.
	
	// Note that the out-device is null, thus this isn't that expensive a call
	if (FAILED(oD3D11::Singleton()->D3D11CreateDevice(
		_pAdapter
		, D3D_DRIVER_TYPE_UNKNOWN
		, nullptr
		, 0 // D3D11_CREATE_DEVICE_DEBUG // squelches a warning
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
		oStd::mstring vendor, device;
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

	oErrorSetLast(std::errc::no_such_device);
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

namespace oStd {

const char* as_string(const DXGI_FORMAT& _Format)
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

} // namespace oStd

#endif // oDXVER >= oDXVER_10
