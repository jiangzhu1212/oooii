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
#include "d3d_resource.h"
#include <oCore/windows/win_util.h>
#include "d3d_debug.h"
#include "dxgi_util.h"

namespace ouro {
	namespace gpu {
		namespace d3d {
			namespace detail {

// {C3A9AAF5-F08E-4896-B3CE-1364EFD8E254}
static const GUID oGUID_CpuAccessView = { 0xc3a9aaf5, 0xf08e, 0x4896, { 0xb3, 0xce, 0x13, 0x64, 0xef, 0xd8, 0xe2, 0x54 } };

struct oD3DCpuAccessView : View
{
	oD3DCpuAccessView(Resource* pResource, const D3D11_SHADER_RESOURCE_VIEW_DESC* pDesc) : resource(pResource) { ref = 1; desc = *pDesc; }
	
	ULONG AddRef() override { return ++ref; }
	ULONG Release() override { ULONG r = --ref; if (!r) delete this; return r; }
	HRESULT QueryInterface(REFGUID guid, void** ppvObject) override { if (!ppvObject) return E_POINTER; if (guid == oGUID_CpuAccessView || guid == __uuidof(View) || guid == __uuidof(IUnknown) || guid == __uuidof(DeviceChild)) { AddRef(); *ppvObject = this; return S_OK; } return E_NOINTERFACE; }
	void GetDevice(Device** ppDevice) override { resource->GetDevice(ppDevice); }
	HRESULT GetPrivateData(REFGUID guid, UINT* pDataSize, void* pData) override { return E_NOTIMPL; }
	HRESULT SetPrivateData(REFGUID guid, UINT DataSize, const void* pData) override { return E_NOTIMPL; }
	HRESULT SetPrivateDataInterface(REFGUID guid, const IUnknown* pData) override { return E_NOTIMPL; }
	void GetResource(Resource** ppResource) override { resource->AddRef(); *ppResource = resource; }
	void GetDesc(D3D11_SHADER_RESOURCE_VIEW_DESC* pDesc) { *pDesc = desc; }
private:
	intrusive_ptr<Resource> resource;
	std::atomic<ULONG> ref;
	D3D11_SHADER_RESOURCE_VIEW_DESC desc;
};

static HRESULT oD3DCreateCpuAccessView(Resource* r, const D3D11_SHADER_RESOURCE_VIEW_DESC* pDesc, oD3DCpuAccessView** ppView)
{
	if (!r || !ppView)
		return E_POINTER;
	try { *ppView = new oD3DCpuAccessView(r, pDesc); }
	catch (std::exception&) { return E_OUTOFMEMORY; }
	return S_OK;
}

static uint cpu_write_flags(D3D11_USAGE usage)
{
	switch (usage)
	{
		case D3D11_USAGE_DYNAMIC: return D3D11_CPU_ACCESS_WRITE;
		case D3D11_USAGE_STAGING: return D3D11_CPU_ACCESS_WRITE|D3D11_CPU_ACCESS_READ;
		default: return 0;
	}
}

static uint misc_flags(const resource_type::value& type, bool render_target, bool has_mips)
{
	uint misc = 0;
	if (render_target && has_mips)
		misc |= D3D11_RESOURCE_MISC_GENERATE_MIPS;

	switch (type)
	{
		case resource_type::constant_buffer:
		case resource_type::index_buffer:
		case resource_type::vertex_buffer:
		case resource_type::structured_buffer:
		case resource_type::structured_append:
		case resource_type::structured_counter: misc |= D3D11_RESOURCE_MISC_BUFFER_STRUCTURED; break;
		case resource_type::raw_buffer: misc |= D3D11_RESOURCE_MISC_DRAWINDIRECT_ARGS|D3D11_RESOURCE_MISC_BUFFER_ALLOW_RAW_VIEWS; break;
		case resource_type::texturecube:
		case resource_type::texturecube_array: misc |= D3D11_RESOURCE_MISC_TEXTURECUBE; break;
		default: break;
	}
	return misc;
}

static uint bind_flags(const resource_type::value& type, const resource_usage::value& usage, bool depth_format)
{
	uint flags = 0;

	switch (usage)
	{
		case resource_usage::infrequent_updates:  flags |= D3D11_BIND_SHADER_RESOURCE;
		case resource_usage::target:  flags |= depth_format ? (D3D11_BIND_SHADER_RESOURCE|D3D11_BIND_DEPTH_STENCIL) : (D3D11_BIND_SHADER_RESOURCE|D3D11_BIND_RENDER_TARGET);
		case resource_usage::unordered: flags |= D3D11_BIND_SHADER_RESOURCE|D3D11_BIND_UNORDERED_ACCESS;
		default: break;
	}

	return flags;
}

#define oDXGI_FORMAT_FROM_FILE ((DXGI_FORMAT)-3)
// returns the same value as pBindFlags receives
uint init(const resource_info& info
	, DXGI_FORMAT* pFormat
	, D3D11_USAGE* pUsage
	, uint* pCPUAccessFlags
	, uint* pBindFlags
	, uint* pMipLevels
	, uint* pMiscFlags)
{
	DXGI_FORMAT DSVF, SRVF;
	dxgi::get_compatible_formats(dxgi::from_surface_format(info.format), pFormat, &DSVF, &SRVF);
	if (*pFormat == DXGI_FORMAT_UNKNOWN)
		*pFormat = oDXGI_FORMAT_FROM_FILE;
	*pUsage = info.usage == resource_usage::readback ? D3D11_USAGE_STAGING : D3D11_USAGE_DEFAULT;
	*pCPUAccessFlags = cpu_write_flags(*pUsage);
	*pBindFlags = bind_flags(info.type, info.usage, *pFormat != oDXGI_FORMAT_FROM_FILE && surface::is_depth(dxgi::to_surface_format(*pFormat)));
	*pMipLevels = info.mips ? 0 : 1;
	*pMiscFlags = misc_flags(info.type, info.usage == resource_usage::target, info.mips);
	return *pBindFlags;
}

static void check(const resource_info& info)
{
	switch (info.usage)
	{
		case resource_usage::target:
			if (info.type < resource_type::texturecube)
				oTHROW_INVARG("a render target must be a texture");
			break;
		default:
			break;
	}

	switch (info.type)
	{
		case resource_type::constant_buffer:
			if (!byte_aligned(info.dimensions.x, 16) || info.dimensions.x > 65535)
				oTHROW_INVARG("A constant buffer must specify a dimensions.x that is 16-byte-aligned and <= 65535 bytes. (size %u bytes specified)", info.dimensions.x);
			break;
		case resource_type::index_buffer:
			if (info.format != surface::r16_uint && info.format != surface::r32_uint)
				oTHROW_INVARG("An index buffer must specify a format of r16_uint or r32_uint only (%s specified).", as_string(info.format));
			if (info.dimensions.x && info.dimensions.x != surface::element_size(info.format))
				oTHROW_INVARG("An index buffer must specify struct_byte_size properly, or set it to 0.");
			break;
		case resource_type::raw_buffer:
			if (info.format != surface::r32_typeless)
				oTHROW_INVARG("unordered_raw must be r32_typeless format");
			if (info.dimensions.x != sizeof(uint))
				throw std::invalid_argument("A raw buffer must specify a struct_byte_size of 4.");
			if (info.array_size < 3)
				throw std::invalid_argument("A raw buffer must have at least 3 elements.");
			break;
		case resource_type::unstructured_buffer:
			if (info.format == surface::unknown)
				oTHROW_INVARG("An unordered, unstructured buffer requires a valid surface format to be specified.");
			break;
		case resource_type::texturecube:
		case resource_type::texturecube_array:
			if ((info.array_size % 6) != 0)
				oTHROW_INVARG("Cube maps must have an array_size that is a multiple of 6, array_size=%d specified", info.array_size);
			break;
		case resource_type::texture3d:
			if (info.array_size > 1)
				oTHROW_INVARG("3d textures don't support slices, array_size=%d specified", info.array_size);
		default:
			break;
	}
}

static oD3D_VIEW_DIMENSION get_view_type(const resource_usage::value& usage, const surface::format& format)
{
	switch (usage)
	{
		case resource_usage::infrequent_updates: return oD3D_VIEW_DIMENSION_SHADER_RESOURCE;
		case resource_usage::target: return surface::is_depth(format) ? oD3D_VIEW_DIMENSION_DEPTH_STENCIL : oD3D_VIEW_DIMENSION_RENDER_TARGET;
		case resource_usage::unordered: return oD3D_VIEW_DIMENSION_UNORDERED_ACCESS;
		case resource_usage::readback: return oD3D_VIEW_DIMENSION_CPU_ACCESS;
		default: break;
	}
	return oD3D_VIEW_DIMENSION_UNKNOWN;
}

#if 0
static oD3D_VIEW_DIMENSION get_view_type(uint bind_flags)
{
	// This is a priority heuristic. Use other APIs to derive simpler types if required.
	if (bind_flags & D3D11_BIND_SHADER_RESOURCE) return oD3D_VIEW_DIMENSION_SHADER_RESOURCE;
	else if (bind_flags & D3D11_BIND_RENDER_TARGET) return oD3D_VIEW_DIMENSION_RENDER_TARGET;
	else if (bind_flags & D3D11_BIND_DEPTH_STENCIL) return oD3D_VIEW_DIMENSION_DEPTH_STENCIL;
	else if (bind_flags & D3D11_BIND_UNORDERED_ACCESS) return oD3D_VIEW_DIMENSION_UNORDERED_ACCESS;
	else if (bind_flags == 0) return oD3D_VIEW_DIMENSION_CPU_ACCESS;
	return oD3D_VIEW_DIMENSION_UNKNOWN;
}
#endif

static resource_usage::value get_usage(D3D11_USAGE usage, uint bind_flags)
{
	if (bind_flags & D3D11_BIND_UNORDERED_ACCESS)
		return resource_usage::unordered;
	else if (bind_flags & (D3D11_BIND_RENDER_TARGET|D3D11_BIND_DEPTH_STENCIL))
		return resource_usage::target;
	else if (usage == D3D11_USAGE_STAGING)
		return resource_usage::readback;
	return resource_usage::infrequent_updates;
}

static resource_type::value get_buffer_type(uint bind_flags, uint misc_flags, uint uav_flags)
{
	if ((uav_flags & D3D11_BUFFER_UAV_FLAG_COUNTER) || (misc_flags & (D3D11_RESOURCE_MISC_DRAWINDIRECT_ARGS|D3D11_RESOURCE_MISC_BUFFER_ALLOW_RAW_VIEWS)))
		return resource_type::raw_buffer;
	else if (misc_flags & D3D11_RESOURCE_MISC_BUFFER_STRUCTURED)
	{
		if (uav_flags & D3D11_BUFFER_UAV_FLAG_APPEND)
			return resource_type::structured_append;
		else if (uav_flags & D3D11_BUFFER_UAV_FLAG_COUNTER)
			return resource_type::structured_counter;
		return resource_type::structured_buffer;
	}
	else if (bind_flags & D3D11_BIND_CONSTANT_BUFFER)
		return resource_type::constant_buffer;
	else if (bind_flags & D3D11_BIND_INDEX_BUFFER)
		return resource_type::index_buffer;
	else if (bind_flags & D3D11_BIND_VERTEX_BUFFER)
		return resource_type::vertex_buffer;
	return resource_type::unstructured_buffer;
}

template<typename ResourceDescT> static void init_non_dimensions(const ResourceDescT& desc, bool is_array
	, const resource_type::value& type, resource_info* info)
{
	info->type = (desc.MiscFlags & D3D11_RESOURCE_MISC_TEXTURECUBE) ? resource_type::texturecube : type;
	if (is_array)
	{
		if (info->type >= resource_type::texturecube && info->type != resource_type::texture3d)
			info->type = resource_type::value(info->type + 4);
	}
	else
		info->array_size = 0;

	info->usage = detail::get_usage(desc.Usage, desc.BindFlags);
	info->format = dxgi::to_surface_format(desc.Format);
	info->mips = desc.MipLevels != 1;
}

template<typename ResourceDescT> static void set_staging(ResourceDescT* desc)
{
	desc->Usage = D3D11_USAGE_STAGING;
	desc->CPUAccessFlags = D3D11_CPU_ACCESS_READ;
	desc->BindFlags = 0;
}

				} // namespace detail

oD3D_VIEW_DIMENSION get_type(View* v)
{
	intrusive_ptr<View> view;
	if (SUCCEEDED(v->QueryInterface(__uuidof(ShaderResourceView), (void**)&view)))
		return oD3D_VIEW_DIMENSION_SHADER_RESOURCE;
	else if (SUCCEEDED(v->QueryInterface(__uuidof(RenderTargetView), (void**)&view)))
		return oD3D_VIEW_DIMENSION_RENDER_TARGET;
	else if (SUCCEEDED(v->QueryInterface(__uuidof(DepthStencilView), (void**)&view)))
		return oD3D_VIEW_DIMENSION_DEPTH_STENCIL;
	else if (SUCCEEDED(v->QueryInterface(__uuidof(UnorderedAccessView), (void**)&view)))
		return oD3D_VIEW_DIMENSION_UNORDERED_ACCESS;
	else if (SUCCEEDED(v->QueryInterface(detail::oGUID_CpuAccessView, (void**)&view)))
		return oD3D_VIEW_DIMENSION_CPU_ACCESS;
	return oD3D_VIEW_DIMENSION_UNKNOWN;
}

oD3D_VIEW_DESC get_desc(View* v)
{
	oD3D_VIEW_DESC desc;
	desc.Type = get_type(v);
	switch (desc.Type)
	{
		case oD3D_VIEW_DIMENSION_SHADER_RESOURCE: static_cast<ShaderResourceView*>(v)->GetDesc(&desc.SRV); break;
		case oD3D_VIEW_DIMENSION_RENDER_TARGET: static_cast<RenderTargetView*>(v)->GetDesc(&desc.RTV); break;
		case oD3D_VIEW_DIMENSION_DEPTH_STENCIL: static_cast<DepthStencilView*>(v)->GetDesc(&desc.DSV); break;
		case oD3D_VIEW_DIMENSION_UNORDERED_ACCESS: static_cast<UnorderedAccessView*>(v)->GetDesc(&desc.UAV); break;
		case oD3D_VIEW_DIMENSION_CPU_ACCESS: static_cast<detail::oD3DCpuAccessView*>(v)->GetDesc(&desc.CAV); break;
		default: break;
	}
	return desc;
}

oD3D_RESOURCE_DESC get_desc(Resource* r)
{
	oD3D_RESOURCE_DESC desc;
	D3D11_RESOURCE_DIMENSION type = D3D11_RESOURCE_DIMENSION_UNKNOWN;
	r->GetType(&type);
	switch (type)
	{
		case D3D11_RESOURCE_DIMENSION_BUFFER: static_cast<Buffer*>(r)->GetDesc(&desc.BufferDesc); break;
		case D3D11_RESOURCE_DIMENSION_TEXTURE1D: static_cast<Texture1D*>(r)->GetDesc(&desc.Texture1DDesc); break;
		case D3D11_RESOURCE_DIMENSION_TEXTURE2D: static_cast<Texture2D*>(r)->GetDesc(&desc.Texture2DDesc); break;
		case D3D11_RESOURCE_DIMENSION_TEXTURE3D: static_cast<Texture3D*>(r)->GetDesc(&desc.Texture3DDesc); break;
		default: break;
	}
	return desc;
}

oD3D_VIEW_DESC to_view_desc(const resource_info& info)
{
	oD3D_VIEW_DESC desc;
	DXGI_FORMAT TF, DSVF, SRVF;
	dxgi::get_compatible_formats(dxgi::from_surface_format(info.format), &TF, &DSVF, &SRVF);

	const int textureI = info.type - resource_type::texturecube;
	desc.Type = detail::get_view_type(info.usage, info.format);
	switch (desc.Type)
	{
		case oD3D_VIEW_DIMENSION_UNORDERED_ACCESS:
		{
			static const D3D11_UAV_DIMENSION UAVTextureDimensions[] = { D3D11_UAV_DIMENSION_TEXTURE2DARRAY, D3D11_UAV_DIMENSION_TEXTURE1D, D3D11_UAV_DIMENSION_TEXTURE2D, D3D11_UAV_DIMENSION_TEXTURE3D, D3D11_UAV_DIMENSION_TEXTURE1DARRAY, D3D11_UAV_DIMENSION_TEXTURE2DARRAY };
			uint flags = 0;
			switch (info.type)
			{
				case resource_type::raw_buffer: flags = D3D11_BUFFER_UAV_FLAG_RAW; break;
				case resource_type::structured_append: flags = D3D11_BUFFER_UAV_FLAG_APPEND; break;
				case resource_type::structured_counter: flags = D3D11_BUFFER_UAV_FLAG_COUNTER; break;
				default: break;
			}

			desc.UAV = CD3D11_UNORDERED_ACCESS_VIEW_DESC
				(textureI < 0 ? D3D11_UAV_DIMENSION_BUFFER : UAVTextureDimensions[textureI]
				, TF
				, 0
				, textureI < 0 ? info.array_size : 0
				, info.type == resource_type::texture3d ? info.dimensions.z : info.array_size
				, flags);
			break;
		}

		case oD3D_VIEW_DIMENSION_DEPTH_STENCIL:
		{
			if (desc.Type != D3D11_RESOURCE_DIMENSION_TEXTURE2D)
				oTHROW_INVARG("Unsupported resource dimension (%s)", as_string(desc.Type));
			desc.DSV = CD3D11_DEPTH_STENCIL_VIEW_DESC(D3D11_DSV_DIMENSION_TEXTURE2D, DSVF, 0, info.array_size, 0);
			break;
		}

		case oD3D_VIEW_DIMENSION_RENDER_TARGET:
		{
			static const D3D11_RTV_DIMENSION RTVTextureDimensions[] = { D3D11_RTV_DIMENSION_TEXTURE2DARRAY, D3D11_RTV_DIMENSION_TEXTURE1D, D3D11_RTV_DIMENSION_TEXTURE2D, D3D11_RTV_DIMENSION_TEXTURE3D, D3D11_RTV_DIMENSION_TEXTURE1DARRAY, D3D11_RTV_DIMENSION_TEXTURE2DARRAY };
			desc.RTV = CD3D11_RENDER_TARGET_VIEW_DESC
				(textureI < 0 ? D3D11_RTV_DIMENSION_BUFFER : RTVTextureDimensions[textureI]
				, dxgi::from_surface_format(info.format)
				, 0
				, textureI < 0 ? info.array_size : 0
				, info.type == resource_type::texture3d ? info.dimensions.z : info.array_size);
			break;
		}

		case oD3D_VIEW_DIMENSION_CPU_ACCESS:
		case oD3D_VIEW_DIMENSION_SHADER_RESOURCE:
		{
			D3D11_SRV_DIMENSION SRVType;
			const bool IsArray = info.type >= resource_type::texturecube_array;
			switch (desc.Type)
			{
				case D3D11_RESOURCE_DIMENSION_TEXTURE1D:
					SRVType = IsArray ? D3D11_SRV_DIMENSION_TEXTURE1DARRAY : D3D11_SRV_DIMENSION_TEXTURE1D;
					break;
				case D3D11_RESOURCE_DIMENSION_TEXTURE2D:
					if (info.type == resource_type::texturecube || info.type == resource_type::texturecube_array)
						SRVType = IsArray ? D3D11_SRV_DIMENSION_TEXTURECUBEARRAY : D3D11_SRV_DIMENSION_TEXTURECUBE;
					else
						SRVType = IsArray ? D3D11_SRV_DIMENSION_TEXTURE2DARRAY : D3D11_SRV_DIMENSION_TEXTURE2D;
					break;
				case D3D11_RESOURCE_DIMENSION_TEXTURE3D:
					SRVType = D3D11_SRV_DIMENSION_TEXTURE3D;
					break;
				oNODEFAULT;
			}

			uint ArraySizeOrMips = textureI < 0 ? info.array_size : __max(1, surface::num_mips(info.mips, info.dimensions));
			uint ArraySize = info.type == resource_type::texturecube_array ? info.array_size / 6 : info.array_size;
			desc.SRV = CD3D11_SHADER_RESOURCE_VIEW_DESC(SRVType, TF, 0, ArraySizeOrMips, 0, ArraySize, 0);
			break;
		}
		default:
			break;
	}
	return desc;
}

oD3D_VIEW_ONLY_DESC from_view_desc(const oD3D_VIEW_DESC& desc)
{
	DXGI_FORMAT format = DXGI_FORMAT_UNKNOWN;
	uint size = 0;
	uint UAVFlags = 0;
	
	oD3D_VIEW_ONLY_DESC vodesc;
	switch (desc.Type)
	{
		case oD3D_VIEW_DIMENSION_CPU_ACCESS:
		case oD3D_VIEW_DIMENSION_SHADER_RESOURCE:
		{
			D3D_SRV_DIMENSION type = desc.SRV.ViewDimension;
			vodesc.is_array = type == D3D_SRV_DIMENSION_TEXTURE1DARRAY || type == D3D_SRV_DIMENSION_TEXTURE2DARRAY || type == D3D_SRV_DIMENSION_TEXTURE2DARRAY || type == D3D_SRV_DIMENSION_TEXTURECUBEARRAY || type == D3D_SRV_DIMENSION_TEXTURE2DMSARRAY;
			format = desc.SRV.Format;
			size = desc.SRV.Buffer.ElementWidth;
			break;
		}	
		case oD3D_VIEW_DIMENSION_RENDER_TARGET:
		{
			D3D11_RTV_DIMENSION type = desc.RTV.ViewDimension;
			vodesc.is_array = type == D3D11_RTV_DIMENSION_TEXTURE1DARRAY || type == D3D11_RTV_DIMENSION_TEXTURE2DARRAY || type == D3D11_RTV_DIMENSION_TEXTURE2DARRAY || type == D3D11_RTV_DIMENSION_TEXTURE2DMSARRAY;
			format = desc.RTV.Format;
			break;
		}
		
		case oD3D_VIEW_DIMENSION_DEPTH_STENCIL:
		{
			D3D11_DSV_DIMENSION type = desc.DSV.ViewDimension;
			vodesc.is_array = type == D3D11_DSV_DIMENSION_TEXTURE1DARRAY || type == D3D11_DSV_DIMENSION_TEXTURE2DARRAY || type == D3D11_DSV_DIMENSION_TEXTURE2DARRAY || type == D3D11_DSV_DIMENSION_TEXTURE2DMSARRAY;
			format = desc.DSV.Format;
			break;
		}
		
		case oD3D_VIEW_DIMENSION_UNORDERED_ACCESS:
		{
			D3D11_UAV_DIMENSION type = desc.UAV.ViewDimension;
			vodesc.is_array = type == D3D11_UAV_DIMENSION_TEXTURE1DARRAY || type == D3D11_UAV_DIMENSION_TEXTURE2DARRAY;
			format = desc.UAV.Format;
			UAVFlags = desc.UAV.Buffer.Flags;
			break;
		}
		default:
			break;
	}

	vodesc.format = dxgi::to_surface_format(format);
	vodesc.struct_byte_size = size ? size : surface::element_size(vodesc.format);
	vodesc.uav_flags = UAVFlags;
	return vodesc;
}

oD3D_RESOURCE_DESC to_resource_desc(const resource_info& info)
{
	detail::check(info);
	oD3D_RESOURCE_DESC desc;
	switch (info.type)
	{
		case resource_type::texture1d:
		case resource_type::texture1d_array:
		{
			desc.Type = D3D11_RESOURCE_DIMENSION_TEXTURE1D;
			desc.Texture1DDesc.Width = info.dimensions.x;
			desc.Texture1DDesc.ArraySize = __max(1, info.array_size);
			detail::init(info, &desc.Texture1DDesc.Format, &desc.Texture1DDesc.Usage, &desc.Texture1DDesc.CPUAccessFlags, &desc.Texture1DDesc.BindFlags, &desc.Texture1DDesc.MipLevels, &desc.Texture1DDesc.MiscFlags);
			break;
		}

		case resource_type::texture2d:
		case resource_type::texturecube:
		case resource_type::texture2d_array:
		case resource_type::texturecube_array:
		{
			desc.Type = D3D11_RESOURCE_DIMENSION_TEXTURE2D;
			desc.Texture2DDesc.Width = info.dimensions.x;
			desc.Texture2DDesc.Height = info.dimensions.y;
			desc.Texture2DDesc.ArraySize = __max(1, info.array_size);
			desc.Texture2DDesc.SampleDesc.Count = 1;
			desc.Texture2DDesc.SampleDesc.Quality = 0;
			detail::init(info, &desc.Texture2DDesc.Format, &desc.Texture2DDesc.Usage, &desc.Texture2DDesc.CPUAccessFlags, &desc.Texture2DDesc.BindFlags, &desc.Texture2DDesc.MipLevels, &desc.Texture2DDesc.MiscFlags);
			break;
		}

		case resource_type::texture3d:
		{
			desc.Type = D3D11_RESOURCE_DIMENSION_TEXTURE3D;
			desc.Texture3DDesc.Width = info.dimensions.x;
			desc.Texture3DDesc.Height = info.dimensions.y;
			desc.Texture3DDesc.Depth = info.dimensions.z;
			detail::init(info, &desc.Texture3DDesc.Format, &desc.Texture3DDesc.Usage, &desc.Texture3DDesc.CPUAccessFlags, &desc.Texture3DDesc.BindFlags, &desc.Texture3DDesc.MipLevels, &desc.Texture3DDesc.MiscFlags);
			break;
		}

		default:
		{
			desc.Type = D3D11_RESOURCE_DIMENSION_BUFFER;
			DXGI_FORMAT format = DXGI_FORMAT_UNKNOWN;
			UINT MipLevels = 0;
			desc.BufferDesc.ByteWidth = info.dimensions.x * info.array_size;
			desc.BufferDesc.StructureByteStride = info.dimensions.x;
			detail::init(info, &format, &desc.BufferDesc.Usage, &desc.BufferDesc.CPUAccessFlags, &desc.BufferDesc.BindFlags, &MipLevels, &desc.BufferDesc.MiscFlags);
			break;
		}
	}

	return desc;
}

resource_info from_resource_desc(const oD3D_RESOURCE_DESC& desc
	, const oD3D_VIEW_ONLY_DESC& view_only_desc
	, D3D11_USAGE* out_usage)
{
	resource_info info;
	switch (desc.Type)
	{
		case D3D11_RESOURCE_DIMENSION_TEXTURE1D:
		{
			info.dimensions = ushort3(static_cast<ushort>(desc.Texture1DDesc.Width), 1, 1);
			info.array_size = static_cast<ushort>(desc.Texture1DDesc.ArraySize);
			detail::init_non_dimensions(desc.Texture1DDesc, view_only_desc.is_array || desc.Texture1DDesc.ArraySize > 1, resource_type::texture1d, &info);
			if (out_usage) *out_usage = desc.Texture1DDesc.Usage;
			break;
		}

		case D3D11_RESOURCE_DIMENSION_TEXTURE2D:
		{
			info.dimensions = ushort3(static_cast<ushort>(desc.Texture2DDesc.Width)
				, static_cast<ushort>(desc.Texture2DDesc.Height), 1);
			info.array_size = static_cast<ushort>(desc.Texture2DDesc.ArraySize);
			detail::init_non_dimensions(desc.Texture2DDesc, view_only_desc.is_array || desc.Texture2DDesc.ArraySize > 1, resource_type::texture2d, &info);
			if (out_usage) *out_usage = desc.Texture2DDesc.Usage;
			break;
		}

		case D3D11_RESOURCE_DIMENSION_TEXTURE3D:
		{
			info.dimensions = ushort3(static_cast<ushort>(desc.Texture3DDesc.Width)
				, static_cast<ushort>(desc.Texture3DDesc.Height), static_cast<ushort>(desc.Texture3DDesc.Depth));
			info.array_size = 1;
			detail::init_non_dimensions(desc.Texture3DDesc, false, resource_type::texture3d, &info);
			if (out_usage) *out_usage = desc.Texture3DDesc.Usage;
			break;
		}

		case D3D11_RESOURCE_DIMENSION_BUFFER:
		{
			info.type = detail::get_buffer_type(desc.BufferDesc.BindFlags, desc.BufferDesc.MiscFlags, view_only_desc.uav_flags);
			info.usage = detail::get_usage(desc.BufferDesc.Usage, desc.BufferDesc.BindFlags);
			info.format = view_only_desc.format;
			info.mips = false;
			oASSERT(struct_byte_size == desc.BufferDesc.StructureByteStride, "apparently this is not always true");
			info.dimensions = ushort3(static_cast<ushort>(desc.BufferDesc.StructureByteStride)
				, ushort(desc.BufferDesc.ByteWidth / desc.BufferDesc.StructureByteStride), 0);
			info.array_size = info.dimensions.y;
			if (out_usage) *out_usage = desc.BufferDesc.Usage;
			break;
		};

		oNODEFAULT;
	}

	return info;
}

intrusive_ptr<Resource> make_resource(Device* dev, const oD3D_RESOURCE_DESC& desc, const char* debug_name, const D3D11_SUBRESOURCE_DATA* init_data)
{
	intrusive_ptr<Resource> r;
	switch (desc.Type)
	{
		case D3D11_RESOURCE_DIMENSION_BUFFER:
			oV(dev->CreateBuffer(&desc.BufferDesc, init_data, (Buffer**)&r));
			break;
		case D3D11_RESOURCE_DIMENSION_TEXTURE1D:
			oV(dev->CreateTexture1D(&desc.Texture1DDesc, init_data, (Texture1D**)&r));
			break;
		case D3D11_RESOURCE_DIMENSION_TEXTURE2D:
			oV(dev->CreateTexture2D(&desc.Texture2DDesc, init_data, (Texture2D**)&r));
			break;
		case D3D11_RESOURCE_DIMENSION_TEXTURE3D:
			oV(dev->CreateTexture3D(&desc.Texture3DDesc, init_data, (Texture3D**)&r));
			break;
		oNODEFAULT;
	}
	d3d::debug_name(r, debug_name);
	return r;
}

intrusive_ptr<View> make_view(Resource* r, const oD3D_VIEW_DESC& desc, const char* debug_name)
{
	intrusive_ptr<Device> dev;
	r->GetDevice(&dev);
	intrusive_ptr<View> v;

	switch (desc.Type)
	{
		case oD3D_VIEW_DIMENSION_SHADER_RESOURCE: oV(dev->CreateShaderResourceView(r, &desc.SRV, (ShaderResourceView**)&v)); break;
		case oD3D_VIEW_DIMENSION_RENDER_TARGET: oV(dev->CreateRenderTargetView(r, &desc.RTV, (RenderTargetView**)&v)); break;
		case oD3D_VIEW_DIMENSION_DEPTH_STENCIL: oV(dev->CreateDepthStencilView(r, &desc.DSV, (DepthStencilView**)&v)); break;
		case oD3D_VIEW_DIMENSION_UNORDERED_ACCESS: oV(dev->CreateUnorderedAccessView(r, &desc.UAV, (UnorderedAccessView**)&v)); break;
		case oD3D_VIEW_DIMENSION_CPU_ACCESS: oV(detail::oD3DCreateCpuAccessView(r, &desc.CAV, (detail::oD3DCpuAccessView**)&v)); break;
		default:
			oTHROW_INVARG("the specified resource cannot have a view (probably a readback usage)");
			break;
	}

	d3d::debug_name(v, debug_name);
	return v;
}

resource_info get_info(View* v)
{
	intrusive_ptr<Resource> r;
	v->GetResource(&r);
	oD3D_RESOURCE_DESC rdesc = get_desc(r);
	oD3D_VIEW_DESC vdesc = get_desc(v);
	oD3D_VIEW_ONLY_DESC vodesc = from_view_desc(vdesc);
	return from_resource_desc(rdesc, vodesc);
}

void copy(Resource* r, uint subresource, surface::mapped_subresource* dst_subresource, bool flip_vertically)
{
	intrusive_ptr<Device> dev;
	r->GetDevice(&dev);
	intrusive_ptr<DeviceContext> dc;
	dev->GetImmediateContext(&dc);

	oD3D_RESOURCE_DESC rdesc = get_desc(r);
	oD3D_VIEW_ONLY_DESC vodesc;
	resource_info info = from_resource_desc(rdesc, vodesc);
	if (info.usage != resource_usage::readback)
	{
		mstring buf;
		oTHROW_INVARG("The specified resource %s does not have CPU read access", debug_name(buf, r));
	}

	D3D11_MAPPED_SUBRESOURCE source;
	oV(dc->Map(r, subresource, D3D11_MAP_READ, 0, &source));
	if (!source.pData)
		oTHROW(no_buffer_space, "mapped a null pointer");
	int2 ByteDimensions = info.type >= resource_type::texturecube ? surface::byte_dimensions(info.format, info.dimensions) : int2(info.dimensions.x * info.dimensions.y, 1);
	memcpy2d(dst_subresource->data, dst_subresource->row_pitch, source.pData, source.RowPitch, ByteDimensions.x, ByteDimensions.y, flip_vertically);
	dc->Unmap(r, subresource);
}

intrusive_ptr<Resource> make_cpu_copy(Resource* r)
{
	intrusive_ptr<Device> dev;
	r->GetDevice(&dev);
	intrusive_ptr<DeviceContext> dc;
	dev->GetImmediateContext(&dc);

	intrusive_ptr<Resource> CPUCopy;

	oD3D_RESOURCE_DESC desc = get_desc(r);
	switch (desc.Type)
	{
		case D3D11_RESOURCE_DIMENSION_BUFFER:
		{
			detail::set_staging(&desc.BufferDesc);
			oV(dev->CreateBuffer(&desc.BufferDesc, nullptr, (Buffer**)&CPUCopy));
			break;
		}

		case D3D11_RESOURCE_DIMENSION_TEXTURE1D:
		{
			detail::set_staging(&desc.Texture1DDesc);
			oV(dev->CreateTexture1D(&desc.Texture1DDesc, nullptr, (Texture1D**)&CPUCopy));
			break;
		}

		case D3D11_RESOURCE_DIMENSION_TEXTURE2D:
		{
			detail::set_staging(&desc.Texture2DDesc);
			oV(dev->CreateTexture2D(&desc.Texture2DDesc, nullptr, (Texture2D**)&CPUCopy));
			break;
		}

		case D3D11_RESOURCE_DIMENSION_TEXTURE3D:
		{
			detail::set_staging(&desc.Texture3DDesc);
			oV(dev->CreateTexture3D(&desc.Texture3DDesc, nullptr, (Texture3D**)&CPUCopy));
			break;
		}

		default:
			oTHROW_INVARG("unknown resource type");
	}

	// set the debug name
	{
		lstring rname;
		debug_name(rname, r);

		lstring copyName;
		snprintf(copyName, "%s.readback", rname.c_str());

		debug_name(CPUCopy, copyName);
	}

	dc->CopyResource(CPUCopy, r);
	dc->Flush();
	return CPUCopy;
}

intrusive_ptr<UnorderedAccessView> make_unflagged_copy(UnorderedAccessView* v)
{
	D3D11_UNORDERED_ACCESS_VIEW_DESC desc;
	v->GetDesc(&desc);
	if (desc.ViewDimension != D3D11_UAV_DIMENSION_BUFFER)
		oTHROW_INVARG("Only D3D11_UAV_DIMENSION_BUFFER views supported");
	desc.Buffer.Flags = 0;
	intrusive_ptr<Resource> r;
	v->GetResource(&r);
	intrusive_ptr<Device> dev;
	r->GetDevice(&dev);
	intrusive_ptr<UnorderedAccessView> unflagged;
	oV(dev->CreateUnorderedAccessView(r, &desc, &unflagged));
	return unflagged;
}

std::shared_ptr<surface::buffer> make_snapshot(Texture2D* t)
{
	intrusive_ptr<Resource> CPUTexture = make_cpu_copy(t);
	Texture2D* CPUTexture2D = (Texture2D*)CPUTexture.c_ptr();

	oD3D_RESOURCE_DESC desc = get_desc(CPUTexture2D);
	int3 dimensions(desc.Texture2DDesc.Width, desc.Texture2DDesc.Height, 1);
	DXGI_FORMAT format = desc.Texture2DDesc.Format;
	if (format == DXGI_FORMAT_UNKNOWN)
		oTHROW_INVARG("The specified texture's format %s is not supported by oImage", as_string(format));

	surface::info si;
	si.format = surface::b8g8r8a8_unorm;
	si.dimensions = dimensions;
	std::shared_ptr<surface::buffer> s = surface::buffer::make(si);

	surface::lock_guard lock(s);
	copy(CPUTexture2D, 0, &lock.mapped);

	return s;
}

void trace_desc(const D3D11_TEXTURE2D_DESC& desc, const char* prefix)
{
	#define oD3D11_TRACE_UINT_S(struct_, x) oTRACE("%s" #x "=%u", oSAFESTR(_Prefix), struct_.x)
	#define oD3D11_TRACE_ENUM_S(struct_, x) oTRACE("%s" #x "=%s", oSAFESTR(_Prefix), as_string(struct_.x))
	#define oD3D11_TRACE_FLAGS_S(_FlagEnumType, struct_, _FlagsVar, _AllZeroString) do { char buf[512]; strbitmask(buf, struct_._FlagsVar, _AllZeroString, as_string<_FlagEnumType>); oTRACE("%s" #_FlagsVar "=%s", oSAFESTR(prefix), buf); } while(false)

	#define oD3D11_TRACE_UINT(x) oD3D11_TRACE_UINT_S(desc, x)
	#define oD3D11_TRACE_ENUM(x) oD3D11_TRACE_ENUM_S(desc, x)
	#define oD3D11_TRACE_FLAGS(_FlagEnumType, _FlagsVar, _AllZeroString) oD3D11_TRACE_FLAGS_S(_FlagEnumType, desc, _FlagsVar, _AllZeroString)

	oD3D11_TRACE_UINT(Width);
	oD3D11_TRACE_UINT(Height);
	oD3D11_TRACE_UINT(MipLevels);
	oD3D11_TRACE_UINT(ArraySize);
	oD3D11_TRACE_ENUM(Format);
	oD3D11_TRACE_UINT(SampleDesc.Count);
	oD3D11_TRACE_UINT(SampleDesc.Quality);
	oD3D11_TRACE_ENUM(Usage);
	oD3D11_TRACE_FLAGS(D3D11_BIND_FLAG, BindFlags, "(none)");
	oD3D11_TRACE_FLAGS(D3D11_CPU_ACCESS_FLAG, CPUAccessFlags, "(none)");
	oD3D11_TRACE_FLAGS(D3D11_RESOURCE_MISC_FLAG, MiscFlags, "(none)");

	#undef oD3D11_TRACE_UINT
	#undef oD3D11_TRACE_ENUM
	#undef oD3D11_TRACE_FLAGS
}

intrusive_ptr<Resource> make_second_texture(Device* dev, const oD3D_RESOURCE_DESC& desc, const char* debug_name)
{
	if (desc.Type != D3D11_RESOURCE_DIMENSION_TEXTURE2D)
		oTHROW_INVARG("only Texture2Ds currently supported");
	surface::format format = dxgi::to_surface_format(desc.Texture2DDesc.Format);
	oCHECK(surface::num_subformats(format) <= 2, "Many-plane textures not supported");
	if (surface::num_subformats(format) == 2)
	{
		// To keep YUV textures singular to prepare for new YUV-based DXGI formats
		// coming, create a private data companion texture.
		oD3D_RESOURCE_DESC desc2(desc);
		desc2.Texture2DDesc.Format = dxgi::from_surface_format(surface::subformat(format, 1));
		auto dimensions = surface::dimensions_npot(format, int3(desc.Texture2DDesc.Width, desc.Texture2DDesc.Height, 1), 0, 1);
		desc2.Texture2DDesc.Width = dimensions.x;
		desc2.Texture2DDesc.Height = dimensions.y;
		return make_resource(dev, desc2, debug_name);
	}
	return nullptr;
}

		} // namespace d3d
	} // namespace gpu
} // namespace ouro
