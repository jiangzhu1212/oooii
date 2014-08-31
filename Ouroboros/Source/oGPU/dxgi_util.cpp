// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.
#include "dxgi_util.h"

#include <oBase/fixed_string.h>
#include <oCore/windows/win_error.h>
#include <oCore/windows/win_util.h>
#include <d3d11.h>

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

namespace gpu { namespace dxgi {

surface::format to_surface_format(DXGI_FORMAT _Format)
{
	// surface::format and DXGI_FORMAT are mostly the same thing.
	return static_cast<surface::format>(_Format <= DXGI_FORMAT_BC7_UNORM_SRGB ? _Format : DXGI_FORMAT_UNKNOWN);
}

DXGI_FORMAT from_surface_format(surface::format _Format)
{
	if ((int)_Format <= DXGI_FORMAT_BC7_UNORM_SRGB)
		return static_cast<DXGI_FORMAT>(_Format);

	if (surface::is_yuv(_Format) && surface::num_subformats(_Format) > 1)
	{
		// Until DXGI_FORMAT gets its extended YUV formats, assume when using this 
		// API return the dominant plane (usually plane0).
		return from_surface_format(surface::subformat(_Format, 0));
	}

	return DXGI_FORMAT_UNKNOWN;
}

bool is_depth(DXGI_FORMAT _Format)
{
	switch (_Format)
	{
		case DXGI_FORMAT_D32_FLOAT_S8X24_UINT:
		case DXGI_FORMAT_D24_UNORM_S8_UINT:
		case DXGI_FORMAT_D16_UNORM:
		case DXGI_FORMAT_D32_FLOAT:
			return true;
		default: break;
	}
	return false;
}

bool is_block_compressed(DXGI_FORMAT _Format)
{
	switch (_Format)
	{
		case DXGI_FORMAT_BC1_TYPELESS:
		case DXGI_FORMAT_BC1_UNORM:
		case DXGI_FORMAT_BC1_UNORM_SRGB:
		case DXGI_FORMAT_BC2_TYPELESS:
		case DXGI_FORMAT_BC2_UNORM:
		case DXGI_FORMAT_BC2_UNORM_SRGB:
		case DXGI_FORMAT_BC3_TYPELESS:
		case DXGI_FORMAT_BC3_UNORM:
		case DXGI_FORMAT_BC3_UNORM_SRGB:
		case DXGI_FORMAT_BC4_TYPELESS:
		case DXGI_FORMAT_BC4_UNORM:
		case DXGI_FORMAT_BC4_SNORM:
		case DXGI_FORMAT_BC5_TYPELESS:
		case DXGI_FORMAT_BC5_UNORM:
		case DXGI_FORMAT_BC5_SNORM:
		case DXGI_FORMAT_BC6H_TYPELESS:
		case DXGI_FORMAT_BC6H_UF16:
		case DXGI_FORMAT_BC6H_SF16:
		case DXGI_FORMAT_BC7_TYPELESS:
		case DXGI_FORMAT_BC7_UNORM:
		case DXGI_FORMAT_BC7_UNORM_SRGB:
			return true;
		default: break;
	}
	return false;
}

uint get_size(DXGI_FORMAT _Format, uint plane)
{
	oASSERT(plane == 0, "other planes not implemented yet");
	uint sSizes[] = 
	{
		0, // DXGI_FORMAT_UNKNOWN
		16, // DXGI_FORMAT_R32G32B32A32_TYPELESS
		16, // DXGI_FORMAT_R32G32B32A32_FLOAT
		16, // DXGI_FORMAT_R32G32B32A32_UINT
		16, // DXGI_FORMAT_R32G32B32A32_SINT
		12, // DXGI_FORMAT_R32G32B32_TYPELESS
		12, // DXGI_FORMAT_R32G32B32_FLOAT
		12, // DXGI_FORMAT_R32G32B32_UINT
		12, // DXGI_FORMAT_R32G32B32_SINT
		8, // DXGI_FORMAT_R16G16B16A16_TYPELESS
		8, // DXGI_FORMAT_R16G16B16A16_FLOAT
		8, // DXGI_FORMAT_R16G16B16A16_UNORM
		8, // DXGI_FORMAT_R16G16B16A16_UINT
		8, // DXGI_FORMAT_R16G16B16A16_SNORM
		8, // DXGI_FORMAT_R16G16B16A16_SINT
		8, // DXGI_FORMAT_R32G32_TYPELESS
		8, // DXGI_FORMAT_R32G32_FLOAT
		8, // DXGI_FORMAT_R32G32_UINT
		8, // DXGI_FORMAT_R32G32_SINT
		8, // DXGI_FORMAT_R32G8X24_TYPELESS
		8, // DXGI_FORMAT_D32_FLOAT_S8X24_UINT
		8, // DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS
		8, // DXGI_FORMAT_X32_TYPELESS_G8X24_UINT
		4, // DXGI_FORMAT_R10G10B10A2_TYPELESS
		4, // DXGI_FORMAT_R10G10B10A2_UNORM
		4, // DXGI_FORMAT_R10G10B10A2_UINT
		4, // DXGI_FORMAT_R11G11B10_FLOAT
		4, // DXGI_FORMAT_R8G8B8A8_TYPELESS
		4, // DXGI_FORMAT_R8G8B8A8_UNORM
		4, // DXGI_FORMAT_R8G8B8A8_UNORM_SRGB
		4, // DXGI_FORMAT_R8G8B8A8_UINT
		4, // DXGI_FORMAT_R8G8B8A8_SNORM
		4, // DXGI_FORMAT_R8G8B8A8_SINT
		4, // DXGI_FORMAT_R16G16_TYPELESS
		4, // DXGI_FORMAT_R16G16_FLOAT
		4, // DXGI_FORMAT_R16G16_UNORM
		4, // DXGI_FORMAT_R16G16_UINT
		4, // DXGI_FORMAT_R16G16_SNORM
		4, // DXGI_FORMAT_R16G16_SINT
		4, // DXGI_FORMAT_R32_TYPELESS
		4, // DXGI_FORMAT_D32_FLOAT
		4, // DXGI_FORMAT_R32_FLOAT
		4, // DXGI_FORMAT_R32_UINT
		4, // DXGI_FORMAT_R32_SINT
		4, // DXGI_FORMAT_R24G8_TYPELESS
		4, // DXGI_FORMAT_D24_UNORM_S8_UINT
		4, // DXGI_FORMAT_R24_UNORM_X8_TYPELESS
		4, // DXGI_FORMAT_X24_TYPELESS_G8_UINT
		2, // DXGI_FORMAT_R8G8_TYPELESS
		2, // DXGI_FORMAT_R8G8_UNORM
		2, // DXGI_FORMAT_R8G8_UINT
		2, // DXGI_FORMAT_R8G8_SNORM
		2, // DXGI_FORMAT_R8G8_SINT
		2, // DXGI_FORMAT_R16_TYPELESS
		2, // DXGI_FORMAT_R16_FLOAT
		2, // DXGI_FORMAT_D16_UNORM
		2, // DXGI_FORMAT_R16_UNORM
		2, // DXGI_FORMAT_R16_UINT
		2, // DXGI_FORMAT_R16_SNORM
		2, // DXGI_FORMAT_R16_SINT
		1, // DXGI_FORMAT_R8_TYPELESS
		1, // DXGI_FORMAT_R8_UNORM
		1, // DXGI_FORMAT_R8_UINT
		1, // DXGI_FORMAT_R8_SNORM
		1, // DXGI_FORMAT_R8_SINT
		1, // DXGI_FORMAT_A8_UNORM
		1, // DXGI_FORMAT_R1_UNORM
		4, // DXGI_FORMAT_R9G9B9E5_SHAREDEXP
		4, // DXGI_FORMAT_R8G8_B8G8_UNORM
		4, // DXGI_FORMAT_G8R8_G8B8_UNORM
		8, // DXGI_FORMAT_BC1_TYPELESS
		8, // DXGI_FORMAT_BC1_UNORM
		8, // DXGI_FORMAT_BC1_UNORM_SRGB
		16, // DXGI_FORMAT_BC2_TYPELESS
		16, // DXGI_FORMAT_BC2_UNORM
		16, // DXGI_FORMAT_BC2_UNORM_SRGB
		16, // DXGI_FORMAT_BC3_TYPELESS
		16, // DXGI_FORMAT_BC3_UNORM
		16, // DXGI_FORMAT_BC3_UNORM_SRGB
		8, // DXGI_FORMAT_BC4_TYPELESS
		8, // DXGI_FORMAT_BC4_UNORM
		8, // DXGI_FORMAT_BC4_SNORM
		16, // DXGI_FORMAT_BC5_TYPELESS
		16, // DXGI_FORMAT_BC5_UNORM
		16, // DXGI_FORMAT_BC5_SNORM
		2, // DXGI_FORMAT_B5G6R5_UNORM
		2, // DXGI_FORMAT_B5G5R5A1_UNORM
		4, // DXGI_FORMAT_B8G8R8A8_UNORM
		4, // DXGI_FORMAT_B8G8R8X8_UNORM
		4, // DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM
		4, // DXGI_FORMAT_B8G8R8A8_TYPELESS
		4, // DXGI_FORMAT_B8G8R8A8_UNORM_SRGB
		4, // DXGI_FORMAT_B8G8R8X8_TYPELESS
		4, // DXGI_FORMAT_B8G8R8X8_UNORM_SRGB
		16, // DXGI_FORMAT_BC6H_TYPELESS
		16, // DXGI_FORMAT_BC6H_UF16
		16, // DXGI_FORMAT_BC6H_SF16
		16, // DXGI_FORMAT_BC7_TYPELESS
		16, // DXGI_FORMAT_BC7_UNORM
		16, // DXGI_FORMAT_BC7_UNORM_SRGB
		2, // DXGI_FORMAT_AYUV
		4, // DXGI_FORMAT_Y410
		4, // DXGI_FORMAT_Y416
		1, // DXGI_FORMAT_NV12
		16, // DXGI_FORMAT_P010
		16, // DXGI_FORMAT_P016
		8, // DXGI_FORMAT_420_OPAQUE
		4, // DXGI_FORMAT_YUY2
		8, // DXGI_FORMAT_Y210
		8, // DXGI_FORMAT_Y216
		1, // DXGI_FORMAT_NV11
		8, // DXGI_FORMAT_AI44
		8, // DXGI_FORMAT_IA44
		1, // DXGI_FORMAT_P8
		2, // DXGI_FORMAT_A8P8
		2, // DXGI_FORMAT_B4G4R4A4_UNORM
	};
	static_assert(oCOUNTOF(sSizes) == 116, "array mismatch");
	return sSizes[_Format];
}

intrusive_ptr<IDXGIAdapter> get_adapter(const adapter::id& adapter_id)
{
	intrusive_ptr<IDXGIFactory> Factory;
	oV(CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)&Factory));

	intrusive_ptr<IDXGIAdapter> Adapter;
	if (DXGI_ERROR_NOT_FOUND == Factory->EnumAdapters(*(int*)&adapter_id, &Adapter))
		oTHROW(no_such_device, "adapter id=%d not found", *(int*)&adapter_id);
	return Adapter;
}

adapter::info get_info(IDXGIAdapter* adapter)
{
	DXGI_ADAPTER_DESC ad;
	adapter->GetDesc(&ad);

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

	return adapter_info;
}

intrusive_ptr<SwapChain> make_swap_chain(IUnknown* dev
	, bool fullscreen
	, const int2& dimensions
	, bool auto_change_monitor_resolution
	, surface::format _Format
	, uint refresh_rate_numerator
	, uint refresh_rate_denominator
	, HWND hwnd
	, bool enable_gdi_compatibility)
{
	if (!dev)
		throw std::invalid_argument("a valid device must be specified");

	DXGI_SWAP_CHAIN_DESC d;
	d.BufferDesc.Width = dimensions.x;
	d.BufferDesc.Height = dimensions.y;
	d.BufferDesc.RefreshRate.Numerator = refresh_rate_numerator;
	d.BufferDesc.RefreshRate.Denominator = refresh_rate_denominator;
	d.BufferDesc.Format = from_surface_format(_Format);
	d.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	d.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	d.SampleDesc.Count = 1;
	d.SampleDesc.Quality = 0;
	d.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT | DXGI_USAGE_SHADER_INPUT/* | DXGI_USAGE_UNORDERED_ACCESS*/; // causes createswapchain to fail... why?
	d.BufferCount = 3;
	d.OutputWindow = hwnd;
	d.Windowed = !fullscreen;
	d.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	
	d.Flags = 0;
	if (auto_change_monitor_resolution) d.Flags |= DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	if (enable_gdi_compatibility) d.Flags |= DXGI_SWAP_CHAIN_FLAG_GDI_COMPATIBLE;
	
	intrusive_ptr<IDXGIDevice> D3DDevice;
	oV(dev->QueryInterface(&D3DDevice));

	intrusive_ptr<IDXGIAdapter> Adapter;
	oV(D3DDevice->GetAdapter(&Adapter));

	intrusive_ptr<IDXGIFactory> Factory;
	oV(Adapter->GetParent(__uuidof(IDXGIFactory), (void**)&Factory));
	
	intrusive_ptr<SwapChain> SwapChain;
	oV(Factory->CreateSwapChain(dev, &d, &SwapChain));
	
	// DXGI_MWA_NO_ALT_ENTER seems bugged from comments at bottom of this link:
	// http://stackoverflow.com/questions/2353178/disable-alt-enter-in-a-direct3d-directx-application
	oV(Factory->MakeWindowAssociation(hwnd, DXGI_MWA_NO_WINDOW_CHANGES|DXGI_MWA_NO_ALT_ENTER));

	return SwapChain;
}

void resize_buffers(SwapChain* sc, const int2& new_size)
{
	oCHECK(all(new_size > int2(0,0)), "resize to a 0 dimension is not supported");
	DXGI_SWAP_CHAIN_DESC d;
	sc->GetDesc(&d);
	HRESULT HR = sc->ResizeBuffers(d.BufferCount, new_size.x, new_size.y, d.BufferDesc.Format, d.Flags);
	if (HR == DXGI_ERROR_INVALID_CALL)
		oTHROW(permission_denied, "Cannot resize DXGISwapChain buffers because there still are dependent resources in client code. Ensure all dependent resources are freed before resize occurs.");
}

HDC get_dc(SwapChain* sc)
{
	intrusive_ptr<d3d::Texture2D> RT;
	oV(sc->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&RT));
	intrusive_ptr<IDXGISurface1> DXGISurface;
	oV(RT->QueryInterface(&DXGISurface));
	//oTRACE("GetDC() exception below (if it happens) cannot be try-catch caught, so ignore it or don't use GDI drawing.");
	HDC hDC = nullptr;
	oV(DXGISurface->GetDC(false, &hDC));
	return hDC;
}

void release_dc(SwapChain* sc, RECT* _pDirtyRect)
{
	intrusive_ptr<d3d::Texture2D> RT;
	oV(sc->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&RT));
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

void get_compatible_formats(DXGI_FORMAT desired_format, DXGI_FORMAT* out_texture_format, DXGI_FORMAT* out_dsv_format, DXGI_FORMAT* out_srv_format)
{
	switch (desired_format)
	{
		case DXGI_FORMAT_R24G8_TYPELESS:
		case DXGI_FORMAT_D24_UNORM_S8_UINT:
		case DXGI_FORMAT_R24_UNORM_X8_TYPELESS:
			*out_texture_format = DXGI_FORMAT_R24G8_TYPELESS;
			*out_dsv_format = DXGI_FORMAT_D24_UNORM_S8_UINT;
			*out_srv_format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
			break;

		case DXGI_FORMAT_D32_FLOAT:
		case DXGI_FORMAT_R32_TYPELESS:
			*out_texture_format = DXGI_FORMAT_R32_TYPELESS;
			*out_dsv_format = DXGI_FORMAT_D32_FLOAT;
			*out_srv_format = DXGI_FORMAT_R32_FLOAT;
			break;

		case DXGI_FORMAT_D32_FLOAT_S8X24_UINT:
		case DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS:
			*out_texture_format = DXGI_FORMAT_UNKNOWN;
			*out_dsv_format = DXGI_FORMAT_UNKNOWN;
			*out_srv_format = DXGI_FORMAT_UNKNOWN;
			break;

		case DXGI_FORMAT_D16_UNORM:
		case DXGI_FORMAT_R16_TYPELESS:
			*out_texture_format = DXGI_FORMAT_R16_TYPELESS;
			*out_dsv_format = DXGI_FORMAT_D16_UNORM;
			*out_srv_format = DXGI_FORMAT_R16_UNORM;
			break;

		default:
			*out_texture_format = desired_format;
			*out_dsv_format = desired_format;
			*out_srv_format = desired_format;
			break;
	}
}

void set_fullscreen_exclusive(SwapChain* sc, bool fullscreen_exclusive)
{
	DXGI_SWAP_CHAIN_DESC SCD;
	sc->GetDesc(&SCD);
	if (GetParent(SCD.OutputWindow))
		oTHROW(operation_not_permitted, "child windows cannot go full screen exclusive");

	BOOL FS = FALSE;
	sc->GetFullscreenState(&FS, nullptr);
	if (fullscreen_exclusive != !!FS)
	{
		// This can throw an exception for some reason, but there's no DXGI error, and everything seems just fine.
		// so ignore?
		sc->SetFullscreenState(fullscreen_exclusive, nullptr);
	}
}

void present(SwapChain* sc, uint interval)
{
	DXGI_SWAP_CHAIN_DESC SCD;
	sc->GetDesc(&SCD);

	std::thread::id tid = astid(GetWindowThreadProcessId(SCD.OutputWindow, nullptr));

	if (tid != std::this_thread::get_id())
		oTHROW(operation_not_permitted, "Present() must be called from the window thread");

	if (tid != std::this_thread::get_id())
		oTHROW(no_such_device, "Present() must be called from the window thread");

	HRESULT hr = sc->Present(interval, 0);
	if (FAILED(hr))
		oTHROW(no_such_device, "GPU device has been reset or removed");
}

}}}
