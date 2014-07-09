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
#include "d3dx11_util.h"
#include "d3d_debug.h"
#include "d3d_device.h"
#include "d3d_util.h"
#include "d3d11_util.h"
#include "dxgi_util.h"
#include <oBase/assert.h>

namespace ouro {

const char* as_string(const D3DX11_FILTER_FLAG& _Flag)
{
	switch (_Flag)
	{
		case D3DX11_FILTER_NONE: return "D3DX11_FILTER_NONE";
		case D3DX11_FILTER_POINT: return "D3DX11_FILTER_POINT";
		case D3DX11_FILTER_LINEAR: return "D3DX11_FILTER_LINEAR";
		case D3DX11_FILTER_TRIANGLE: return "D3DX11_FILTER_TRIANGLE";
		case D3DX11_FILTER_BOX: return "D3DX11_FILTER_BOX";
		case D3DX11_FILTER_MIRROR_U: return "D3DX11_FILTER_MIRROR_U";
		case D3DX11_FILTER_MIRROR_V: return "D3DX11_FILTER_MIRROR_V";
		case D3DX11_FILTER_MIRROR_W: return "D3DX11_FILTER_MIRROR_W";
		case D3DX11_FILTER_MIRROR: return "D3DX11_FILTER_MIRROR";
		case D3DX11_FILTER_DITHER: return "D3DX11_FILTER_DITHER";
		case D3DX11_FILTER_DITHER_DIFFUSION: return "D3DX11_FILTER_DITHER_DIFFUSION";
		case D3DX11_FILTER_SRGB_IN: return "D3DX11_FILTER_SRGB_IN";
		case D3DX11_FILTER_SRGB_OUT: return "D3DX11_FILTER_SRGB_OUT";
		case D3DX11_FILTER_SRGB: return "D3DX11_FILTER_SRGB";
		default: break;
	}
	return "?";
}

	namespace gpu {
		namespace d3dx11 {

D3DX11_IMAGE_FILE_FORMAT from_path(const path& _Path)
{
	struct EXT_MAPPING
	{
		const char* Extension;
		D3DX11_IMAGE_FILE_FORMAT Format;
	};

	static EXT_MAPPING sExtensions[] =
	{
		{ ".bmp", D3DX11_IFF_BMP }, 
		{ ".jpg", D3DX11_IFF_JPG }, 
		{ ".png", D3DX11_IFF_PNG }, 
		{ ".dds", D3DX11_IFF_DDS }, 
		{ ".tif", D3DX11_IFF_TIFF }, 
		{ ".gif", D3DX11_IFF_GIF }, 
		{ ".wmp", D3DX11_IFF_WMP },
	};

	path ext = _Path.extension();
	oFORI(i, sExtensions)
		if (!_stricmp(sExtensions[i].Extension, ext))
			return sExtensions[i].Format;
	return D3DX11_IFF_DDS;
}

#define oD3D11_TRACE_UINT_S(struct_, x) oTRACE("%s" #x "=%u", oSAFESTR(_Prefix), struct_.x)
#define oD3D11_TRACE_ENUM_S(struct_, x) oTRACE("%s" #x "=%s", oSAFESTR(_Prefix), as_string(struct_.x))
#define oD3D11_TRACE_FLAGS_S(_FlagEnumType, struct_, _FlagsVar, _AllZeroString) do { char buf[512]; strbitmask(buf, struct_._FlagsVar, _AllZeroString, as_string<_FlagEnumType>); oTRACE("%s" #_FlagsVar "=%s", oSAFESTR(_Prefix), buf); } while(false)

void trace_image_load_info(const D3DX11_IMAGE_LOAD_INFO& _ImageLoadInfo, const char* _Prefix)
{
	#define oD3D11_TRACE_UINT(x) do \
	{	if (_ImageLoadInfo.x == D3DX11_DEFAULT) oTRACE("%s" #x "=D3DX11_DEFAULT", oSAFESTR(_Prefix)); \
		else if (_ImageLoadInfo.x == D3DX11_FROM_FILE) oTRACE("%s" #x "=D3DX11_FROM_FILE", oSAFESTR(_Prefix)); \
		else oD3D11_TRACE_UINT_S(_ImageLoadInfo, x); \
	} while(false)
	#define oD3D11_TRACE_ENUM(x) oD3D11_TRACE_ENUM_S(_ImageLoadInfo, x)
	#define oD3D11_TRACE_FLAGS(_FlagEnumType, _FlagsVar, _AllZeroString) oD3D11_TRACE_FLAGS_S(_FlagEnumType, _ImageLoadInfo, _FlagsVar, _AllZeroString)
	
	oD3D11_TRACE_UINT(Width);
	oD3D11_TRACE_UINT(Height);
	oD3D11_TRACE_UINT(Depth);
	oD3D11_TRACE_UINT(FirstMipLevel);
	oD3D11_TRACE_UINT(MipLevels);

	oD3D11_TRACE_ENUM(Usage);
	oD3D11_TRACE_FLAGS(D3D11_BIND_FLAG, BindFlags, "(none)");
	oD3D11_TRACE_FLAGS(D3D11_CPU_ACCESS_FLAG, CpuAccessFlags, "(none)");
	oD3D11_TRACE_FLAGS(D3D11_RESOURCE_MISC_FLAG, MiscFlags, "(none)");

	if (_ImageLoadInfo.Format == DXGI_FORMAT_FROM_FILE)
		oTRACE("%sFormat=DXGI_FORMAT_FROM_FILE", oSAFESTR(_Prefix));
	else
		oD3D11_TRACE_ENUM(Format);

	oD3D11_TRACE_FLAGS(D3DX11_FILTER_FLAG, Filter, "(none)");
	oD3D11_TRACE_FLAGS(D3DX11_FILTER_FLAG, MipFilter, "(none)");

	#undef oD3D11_TRACE_UINT
	#undef oD3D11_TRACE_ENUM
	#undef oD3D11_TRACE_FLAGS
}

static intrusive_ptr<d3d::Resource> prepare_source(d3d::Resource* _pResource
	, D3DX11_IMAGE_FILE_FORMAT _Format, d3d::DeviceContext** _ppDeviceContext)
{
	intrusive_ptr<d3d::Device> Device;
	_pResource->GetDevice(&Device);
	Device->GetImmediateContext(_ppDeviceContext);
	d3d::D3D_TEXTURE_DESC desc = d3d::get_texture_desc(_pResource);
	if (dxgi::is_block_compressed(desc.Format) && _Format != D3DX11_IFF_DDS)
		throw std::invalid_argument("D3DX11 can save block compressed formats only to .dds files.");
	intrusive_ptr<d3d::Resource> Source = desc.Usage == D3D11_USAGE_STAGING ? _pResource : d3d::make_cpu_copy(_pResource);
	return Source;
}

void save(d3d::Resource* _pResource, const path& _Path)
{
	D3DX11_IMAGE_FILE_FORMAT format = from_path(_Path);
	intrusive_ptr<d3d::DeviceContext> ImmediateContext;
	intrusive_ptr<d3d::Resource> Source = prepare_source(_pResource, format, &ImmediateContext);
	filesystem::create_directories(_Path.parent_path());
	oV(D3DX11SaveTextureToFileA(ImmediateContext, Source, format, _Path));
}

static intrusive_ptr<d3d::Resource> prepare_source(const surface::buffer* _pSurface, D3DX11_IMAGE_FILE_FORMAT _Format)
{
	surface::info si = _pSurface->get_info();
	intrusive_ptr<d3d::Device> Device = d3d::make_device(gpu::device_init("save_temp_device"));
	intrusive_ptr<d3d::Texture2D> t = d3d::make_texture_2d("save_temp", Device, si.format, si.dimensions.x, si.dimensions.y, si.array_size, 0, false);

	surface::shared_lock lock(_pSurface);
#if 0
	d3d::new_texture NewTexture = d3d::make_texture(Device, "save_temp_texture", info, &lock.mapped);
	return NewTexture.pResource;
#else
	oTHROW(protocol_error, "not implemented");
#endif
}

void save(const surface::buffer* _pSurface, const path& _Path)
{
	intrusive_ptr<d3d::Resource> Source = prepare_source(_pSurface, from_path(_Path));
	return save(Source, _Path);
}

void save(d3d::Resource* _pTexture, D3DX11_IMAGE_FILE_FORMAT _Format, void* _pBuffer, size_t _SizeofBuffer)
{
	intrusive_ptr<d3d::DeviceContext> ImmediateContext;
	intrusive_ptr<d3d::Resource> Source = prepare_source(_pTexture, _Format, &ImmediateContext);
	intrusive_ptr<ID3D10Blob> Blob;
	oV(D3DX11SaveTextureToMemory(ImmediateContext, Source, _Format, &Blob, 0));
	if (Blob->GetBufferSize() > _SizeofBuffer)
		oTHROW(no_buffer_space, "Buffer is too small for image");
	memcpy(_pBuffer, Blob->GetBufferPointer(), Blob->GetBufferSize());
}

void save(const surface::buffer* _pSurface, D3DX11_IMAGE_FILE_FORMAT _Format, void* _pBuffer, size_t _SizeofBuffer)
{
	intrusive_ptr<d3d::Device> Device = d3d::make_device(gpu::device_init("save_temp_device"));
	intrusive_ptr<d3d::Resource> Source = prepare_source(_pSurface, _Format);
	save(Source, _Format, _pBuffer, _SizeofBuffer); // pass through error
}

#define oDXGI_FORMAT_FROM_FILE ((DXGI_FORMAT)-3)
static D3DX11_IMAGE_LOAD_INFO get_image_load_info(const d3d::D3D_TEXTURE_DESC& d)
{
	D3DX11_IMAGE_LOAD_INFO ili;
	ili.Width = d.Width == 0 ? D3DX11_FROM_FILE : d.Width;
	ili.Height = d.Height == 0 ? D3DX11_FROM_FILE : d.Height;
	ili.Depth = d.ArraySize == 0 ? D3DX11_FROM_FILE : d.ArraySize;
	ili.FirstMipLevel = D3DX11_DEFAULT;
	ili.MiscFlags = 0;
	ili.Filter = D3DX11_FILTER_TRIANGLE;
	ili.MipFilter = D3DX11_FILTER_TRIANGLE;
	ili.pSrcInfo = nullptr;

	DXGI_FORMAT DSVF, SRVF;
	dxgi::get_compatible_formats(d.Format, &ili.Format, &DSVF, &SRVF);
	if (ili.Format == DXGI_FORMAT_UNKNOWN)
		ili.Format = oDXGI_FORMAT_FROM_FILE;

	ili.Usage = d.Usage;
	ili.CpuAccessFlags = ili.Usage == D3D11_USAGE_DYNAMIC ? D3D11_CPU_ACCESS_WRITE : (ili.Usage == D3D11_USAGE_STAGING ? D3D11_CPU_ACCESS_READ : 0);
	ili.MipLevels = d.Mips ? 0 : 1;

	ili.BindFlags = 0;
	if (ili.Usage == D3D11_USAGE_STAGING)
		ili.BindFlags = 0;

	else
	{
		ili.BindFlags = D3D11_BIND_SHADER_RESOURCE;

		if (d.is_color_target())
			ili.BindFlags |= D3D11_BIND_RENDER_TARGET;

		if (d.is_depth_target() && ili.Format != oDXGI_FORMAT_FROM_FILE && dxgi::to_surface_format(ili.Format))
			ili.BindFlags |= D3D11_BIND_DEPTH_STENCIL;

		if (d.is_unordered())
			ili.BindFlags |= D3D11_BIND_UNORDERED_ACCESS;
	}

	return ili;
}

intrusive_ptr<d3d::Resource> load(d3d::Device* dev
	, const d3d::D3D_TEXTURE_DESC& desc, const char* debug_name, const path& p)
{
	D3DX11_IMAGE_LOAD_INFO li = get_image_load_info(desc);
	intrusive_ptr<d3d::Resource> t;
	oV(D3DX11CreateTextureFromFile(dev, p, &li, nullptr, &t, nullptr));
	d3d::debug_name(t, debug_name);
	return t;
}

intrusive_ptr<d3d::Resource> load(d3d::Device* dev
	, const d3d::D3D_TEXTURE_DESC& desc
	, const char* debug_name
	, const void* src
	, size_t src_size)
{
	D3DX11_IMAGE_LOAD_INFO li = get_image_load_info(desc);
	intrusive_ptr<d3d::Resource> t;
	oV(D3DX11CreateTextureFromMemory(dev, src, src_size, &li, nullptr, &t, nullptr));
	d3d::debug_name(t, debug_name);
	return t;
}

void convert(d3d::Texture2D* _pSourceTexture, surface::format _NewFormat
	, d3d::Texture2D** _ppDestinationTexture)
{
	D3D11_TEXTURE2D_DESC desc;
	_pSourceTexture->GetDesc(&desc);

	if (_NewFormat == DXGI_FORMAT_BC7_UNORM)
	{
		if (oD3D11EncodeBC7(_pSourceTexture, true, _ppDestinationTexture))
			return;
		oTRACE("GPU BC7 encode failed, falling back to CPU encode (may take a while)...");
	}

	else if (_NewFormat == DXGI_FORMAT_BC6H_SF16)
	{
		if (oD3D11EncodeBC6HS(_pSourceTexture, true, _ppDestinationTexture))
			return;
		oTRACE("GPU BC6HS encode failed, falling back to CPU encode (may take a while)...");
	}

	else if (_NewFormat == DXGI_FORMAT_BC6H_UF16)
	{
		if (oD3D11EncodeBC6HU(_pSourceTexture, true, _ppDestinationTexture))
			return;
		oTRACE("GPU BC6HU encode failed, falling back to CPU encode (may take a while)...");
	}

	else if (desc.Format == DXGI_FORMAT_BC6H_SF16 || desc.Format == DXGI_FORMAT_BC6H_UF16 || desc.Format == DXGI_FORMAT_BC7_UNORM)
	{
		// Decode requires a CPU-accessible source because CS4x can't sample from
		// BC7 or BC6, so make a copy if needed

		intrusive_ptr<d3d::Texture2D> CPUAccessible;
		if (desc.CPUAccessFlags & D3D11_CPU_ACCESS_READ)
			CPUAccessible = _pSourceTexture;
		else
		{
			intrusive_ptr<d3d::Resource> Resource = d3d::make_cpu_copy(_pSourceTexture);
			CPUAccessible = (ID3D11Texture2D*)Resource.c_ptr();
		}

		intrusive_ptr<d3d::Texture2D> NewTexture;
		if (!oD3D11DecodeBC6orBC7(CPUAccessible, true, &NewTexture))
			throw std::exception("oD3D11DecodeBC6orBC7 failed");

		NewTexture->GetDesc(&desc);
		if (_NewFormat == desc.Format)
		{
			*_ppDestinationTexture = NewTexture;
			(*_ppDestinationTexture)->AddRef();
			return;
		}

		// recurse now that we've got a more vanilla format
		return convert(NewTexture, _NewFormat, _ppDestinationTexture);
	}

	intrusive_ptr<d3d::Device> Device;
	_pSourceTexture->GetDevice(&Device);

#if 0
	gpu::texture1_info info;
	info.dimensions = ushort3(static_cast<ushort>(desc.Width), static_cast<ushort>(desc.Height), 1);
	info.array_size = as_ushort(desc.ArraySize);
	info.format = _NewFormat;
	info.type = gpu::make_readback(info.type);

	intrusive_ptr<ID3D11Texture2D> NewTexture = d3d::make_texture(Device, "convert_temp", info).pTexture2D;
	intrusive_ptr<ID3D11DeviceContext> ImmediateContext;
	Device->GetImmediateContext(&ImmediateContext);
	oV(D3DX11LoadTextureFromTexture(ImmediateContext, _pSourceTexture, nullptr, NewTexture));
	*_ppDestinationTexture = NewTexture;
	(*_ppDestinationTexture)->AddRef();
#else
	oTHROW(protocol_error, "not implemented");
#endif
}

void convert(ID3D11Device* _pDevice
	, surface::mapped_subresource& _Destination
	, surface::format _DestinationFormat
	, surface::const_mapped_subresource& _Source
	, surface::format _SourceFormat
	, const int2& _MipDimensions)
{
#if 0
	gpu::texture1_info i;
	i.dimensions = ushort3(_MipDimensions, 1);
	i.array_size = 1;
	i.format = _SourceFormat;
	i.type = gpu::texture_type::default_2d;
	intrusive_ptr<ID3D11Texture2D> SourceTexture = d3d::make_texture(_pDevice, "convert_temp", i, &_Source).pTexture2D;
	intrusive_ptr<ID3D11Texture2D> DestinationTexture;
	oTRACE("d3d11::convert begin 0x%p (can take a while)...", SourceTexture);
	convert(SourceTexture, _DestinationFormat, &DestinationTexture);
	oTRACE("d3d11::convert end 0x%p", DestinationTexture);
	d3d::copy(DestinationTexture, 0, &_Destination);
#else
	oTHROW(protocol_error, "not implemented");
#endif
}

		} // namespace d3dx11
	} // namespace gpu
} // namespace ouro
