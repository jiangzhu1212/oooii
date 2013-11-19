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
#include "dxgi_util.h"
#include <oBase/fixed_string.h>

namespace ouro {

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
		default: break;
	}
	return "?";
}

	namespace dxgi {

const oGUID& oGetGUID(threadsafe const IDXGISwapChain* threadsafe const*) { return (const oGUID&)__uuidof(IDXGISwapChain); }

surface::format to_surface_format(DXGI_FORMAT _Format)
{
	// surface::format and DXGI_FORMAT are mostly the same thing.
	return static_cast<surface::format>(_Format <= DXGI_FORMAT_BC7_UNORM_SRGB ? _Format : DXGI_FORMAT_UNKNOWN);
}

DXGI_FORMAT from_surface_format(surface::format _Format)
{
	if (_Format <= DXGI_FORMAT_BC7_UNORM_SRGB)
		return static_cast<DXGI_FORMAT>(_Format);

	if (surface::is_yuv(_Format) && surface::num_subformats(_Format) > 1)
	{
		// Until DXGI_FORMAT gets its extended YUV formats, assume when using this 
		// API return the dominant plane (usually plane0).
		return from_surface_format(surface::subformat(_Format, 0));
	}

	return DXGI_FORMAT_UNKNOWN;
}

intrusive_ptr<IDXGIAdapter> get_adapter(const adapter::id& _AdapterID)
{
	intrusive_ptr<IDXGIFactory> Factory;
	oV(CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)&Factory));

	intrusive_ptr<IDXGIAdapter> Adapter;
	if (DXGI_ERROR_NOT_FOUND == Factory->EnumAdapters(*(int*)&_AdapterID, &Adapter))
		oTHROW(no_such_device, "adapter id=%d not found", *(int*)&_AdapterID);
	return std::move(Adapter);
}

adapter::info get_info(IDXGIAdapter* _pAdapter)
{
	DXGI_ADAPTER_DESC ad;
	_pAdapter->GetDesc(&ad);

	adapter::info adapter_info;
	adapter::enumerate([&](const adapter::info& _Info)->bool
	{
		mstring vendor, device;
		snprintf(vendor, "VEN_%X", ad.VendorId);
		snprintf(device, "DEV_%04X", ad.DeviceId);
		if (strstr(_Info.plugnplay_id, vendor) && strstr(_Info.plugnplay_id, device))
		{
			adapter_info = _Info;
			return false;
		}
		return true;
	});

	return std::move(adapter_info);
}

intrusive_ptr<IDXGISwapChain> make_swap_chain(IUnknown* _pDevice
	, bool _Fullscreen
	, const int2& _Dimensions
	, bool _AutochangeMonitorResolution
	, surface::format _Format
	, unsigned int RefreshRateN
	, unsigned int RefreshRateD
	, HWND _hWnd
	, bool _EnableGDICompatibility)
{
	if (!_pDevice)
		throw std::invalid_argument("a valid device must be specified");

	DXGI_SWAP_CHAIN_DESC d;
	d.BufferDesc.Width = _Dimensions.x;
	d.BufferDesc.Height = _Dimensions.y;
	d.BufferDesc.RefreshRate.Numerator = RefreshRateN;
	d.BufferDesc.RefreshRate.Denominator = RefreshRateD;
	d.BufferDesc.Format = from_surface_format(_Format);
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
	if (_AutochangeMonitorResolution) d.Flags |= DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	if (_EnableGDICompatibility) d.Flags |= DXGI_SWAP_CHAIN_FLAG_GDI_COMPATIBLE;
	
	intrusive_ptr<IDXGIDevice> D3DDevice;
	oV(_pDevice->QueryInterface(&D3DDevice));

	intrusive_ptr<IDXGIAdapter> Adapter;
	oV(D3DDevice->GetAdapter(&Adapter));

	intrusive_ptr<IDXGIFactory> Factory;
	oV(Adapter->GetParent(__uuidof(IDXGIFactory), (void**)&Factory));
	
	intrusive_ptr<IDXGISwapChain> SwapChain;
	oV(Factory->CreateSwapChain(_pDevice, &d, &SwapChain));
	
	// DXGI_MWA_NO_ALT_ENTER seems bugged from comments at bottom of this link:
	// http://stackoverflow.com/questions/2353178/disable-alt-enter-in-a-direct3d-directx-application
	oV(Factory->MakeWindowAssociation(_hWnd, DXGI_MWA_NO_WINDOW_CHANGES|DXGI_MWA_NO_ALT_ENTER));

	return SwapChain;
}

void resize_buffers(IDXGISwapChain* _pSwapChain, const int2& _NewSize)
{
	DXGI_SWAP_CHAIN_DESC d;
	_pSwapChain->GetDesc(&d);
	HRESULT HR = _pSwapChain->ResizeBuffers(d.BufferCount, _NewSize.x, _NewSize.y, d.BufferDesc.Format, d.Flags);
	if (HR == DXGI_ERROR_INVALID_CALL)
		oTHROW(permission_denied, "Cannot resize DXGISwapChain buffers because there still are dependent resources in client code. Ensure all dependent resources are freed before resize occurs.");
	//else if (FAILED(HR))
		//throw oStd::windows::error(HR);
}

HDC get_dc(IDXGISwapChain* _pSwapChain)
{
	intrusive_ptr<ID3D11Texture2D> RT;
	oV(_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&RT));
	intrusive_ptr<IDXGISurface1> DXGISurface;
	oV(RT->QueryInterface(&DXGISurface));
	//oTRACE("GetDC() exception below (if it happens) cannot be try-catch caught, so ignore it or don't use GDI drawing.");
	HDC hDC = nullptr;
	oV(DXGISurface->GetDC(false, &hDC));
	return hDC;
}

void release_dc(IDXGISwapChain* _pSwapChain, RECT* _pDirtyRect)
{
	intrusive_ptr<ID3D11Texture2D> RT;
	oV(_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&RT));
	intrusive_ptr<IDXGISurface1> DXGISurface;
	oV(RT->QueryInterface(&DXGISurface));
	//oTRACE("ReleaseDC() exception below (if it happens) cannot be try-catch caught, so ignore it or don't use GDI drawing.");
	oV(DXGISurface->ReleaseDC(_pDirtyRect));
}

HDC get_dc(ID3D11RenderTargetView* _pRTV)
{
	intrusive_ptr<ID3D11Resource> RT;
	_pRTV->GetResource(&RT);
	intrusive_ptr<IDXGISurface1> DXGISurface;
	oV(RT->QueryInterface(&DXGISurface));
	HDC hDC = nullptr;
	oV(DXGISurface->GetDC(false, &hDC));
	return hDC;
}

void release_dc(ID3D11RenderTargetView* _pRTV, RECT* _pDirtyRect)
{
	intrusive_ptr<ID3D11Resource> RT;
	_pRTV->GetResource(&RT);
	intrusive_ptr<IDXGISurface1> DXGISurface;
	oV(RT->QueryInterface(&DXGISurface));
	oV(DXGISurface->ReleaseDC(_pDirtyRect));
}

void get_compatible_formats(DXGI_FORMAT _DesiredFormat, DXGI_FORMAT* _pTextureFormat, DXGI_FORMAT* _pDepthStencilViewFormat, DXGI_FORMAT* _pShaderResourceViewFormat)
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

	} // namespace dxgi
} // namespace ouro
