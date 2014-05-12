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

#define oCHECK_IS_TEXTURE(_pResource) do \
{	D3D11_RESOURCE_DIMENSION type; \
	_pResource->GetType(&type); \
	if (type != D3D11_RESOURCE_DIMENSION_TEXTURE1D && type != D3D11_RESOURCE_DIMENSION_TEXTURE2D && type != D3D11_RESOURCE_DIMENSION_TEXTURE3D) \
	{	mstring buf; \
		oTHROW_INVARG("Only textures types are currently supported. (resource %s)", debug_name(buf, _pResource)); \
	} \
} while (false)

// {13BA565C-4766-49C4-8C1C-C1F459F00A65}
static const GUID oWKPDID_BUFFER_INFO = { 0x13ba565c, 0x4766, 0x49c4, { 0x8c, 0x1c, 0xc1, 0xf4, 0x59, 0xf0, 0xa, 0x65 } };

void set_info(Resource* _pBuffer, const buffer_info& _Desc)
{
	oV(_pBuffer->SetPrivateData(oWKPDID_BUFFER_INFO, sizeof(_Desc), &_Desc));
}

buffer_info get_info(const Resource* _pBuffer)
{
	unsigned int size = sizeof(buffer_info);
	buffer_info i;
	oV(const_cast<Resource*>(_pBuffer)->GetPrivateData(oWKPDID_BUFFER_INFO, &size, &i));
	return i;
}

static unsigned int cpu_write_flags(D3D11_USAGE _Usage)
{
	switch (_Usage)
	{
		case D3D11_USAGE_DYNAMIC: return D3D11_CPU_ACCESS_WRITE;
		case D3D11_USAGE_STAGING: return D3D11_CPU_ACCESS_WRITE|D3D11_CPU_ACCESS_READ;
		default: return 0;
	}
}

intrusive_ptr<Buffer> make_buffer(Device* _pDevice
	, const char* _DebugName
	, const gpu::buffer_info& _Info
	, const void* _pInitBuffer
	, UnorderedAccessView** _ppUAV
	, ShaderResourceView** _ppSRV)
{
	D3D11_USAGE Usage = D3D11_USAGE_DEFAULT;
	unsigned int BindFlags = 0;
	surface::format Format = _Info.format;

	switch (_Info.type)
	{
		case gpu::buffer_type::constant:
			BindFlags = D3D11_BIND_CONSTANT_BUFFER;
			if (!byte_aligned(_Info.struct_byte_size, 16) || _Info.struct_byte_size > 65535)
				throw std::invalid_argument(formatf("A constant buffer must specify a struct_byte_size that is 16-byte-aligned and <= 65536 bytes. (size %u bytes specified)", _Info.struct_byte_size));
			break;
		case gpu::buffer_type::readback:
			Usage = D3D11_USAGE_STAGING;
			break;
		case gpu::buffer_type::index:
			if (_Info.format != surface::r16_uint && _Info.format != surface::r32_uint)
				throw std::invalid_argument(formatf("An index buffer must specify a format of r16_uint or r32_uint only (%s specified).", as_string(_Info.format)));
			if (_Info.struct_byte_size != invalid && _Info.struct_byte_size != static_cast<unsigned int>(surface::element_size(_Info.format)))
				throw std::invalid_argument("An index buffer must specify struct_byte_size properly, or set it to 0.");
			BindFlags = D3D11_BIND_INDEX_BUFFER;
			break;
		case gpu::buffer_type::index_readback:
			Usage = D3D11_USAGE_STAGING;
			break;
		case gpu::buffer_type::vertex:
			BindFlags = D3D11_BIND_VERTEX_BUFFER;
			break;
		case gpu::buffer_type::vertex_readback:
			Usage = D3D11_USAGE_STAGING;
			break;
		case gpu::buffer_type::unordered_raw:
			BindFlags = D3D11_BIND_UNORDERED_ACCESS;
			Format = surface::r32_typeless;
			if (_Info.struct_byte_size != sizeof(unsigned int))
				throw std::invalid_argument("A raw buffer must specify a struct_byte_size of 4.");
			if (_Info.array_size < 3)
				throw std::invalid_argument("A raw buffer must have at least 3 elements.");
			break;
		case gpu::buffer_type::unordered_unstructured:
			BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
			if (Format == surface::unknown)
				throw std::invalid_argument("An unordered, unstructured buffer requires a valid surface format to be specified.");
			break;
		case gpu::buffer_type::unordered_structured:
		case gpu::buffer_type::unordered_structured_append:
		case gpu::buffer_type::unordered_structured_counter:
			BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
			break;
		oNODEFAULT;
	}

	// @tony: Why do I need to do things this way? IBs/VBs seem fine, but constant 
	// buffers seem affected by this.
	if (BindFlags != D3D11_BIND_INDEX_BUFFER && BindFlags != D3D11_BIND_VERTEX_BUFFER)
	{
		if (Usage == D3D11_USAGE_DEFAULT && D3D_FEATURE_LEVEL_11_0 > _pDevice->GetFeatureLevel())
			Usage = D3D11_USAGE_DYNAMIC;
	}

	unsigned int ElementStride = _Info.struct_byte_size;
	if (ElementStride == invalid && Format != surface::unknown)
		ElementStride = surface::element_size(Format);

	if (ElementStride == 0 || ElementStride == invalid)
		throw std::invalid_argument("A structured buffer requires a valid non-zero buffer size to be specified.");

	D3D11_BUFFER_DESC desc;
	desc.ByteWidth = ElementStride * _Info.array_size;
	desc.Usage = Usage;
	desc.BindFlags = BindFlags;
	desc.StructureByteStride = ElementStride;
	desc.CPUAccessFlags = cpu_write_flags(desc.Usage);
	desc.MiscFlags = 0;
	if (_Info.type >= gpu::buffer_type::unordered_structured)
		desc.MiscFlags |= D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	if (_Info.type == gpu::buffer_type::unordered_raw)
		desc.MiscFlags |= D3D11_RESOURCE_MISC_DRAWINDIRECT_ARGS|D3D11_RESOURCE_MISC_BUFFER_ALLOW_RAW_VIEWS;

	D3D11_SUBRESOURCE_DATA SRD;
	SRD.pSysMem = _pInitBuffer;
	intrusive_ptr<Buffer> Buffer;

	oV(_pDevice->CreateBuffer(&desc, _pInitBuffer ? &SRD : nullptr, &Buffer));
	debug_name(Buffer, _DebugName);
	
	// Add this mainly for index buffers so they can describe their own 
	// StructureByteStride.
	// @tony: Is this becoming defunct? This is meant so that D3D11 objects can be 
	// self-describing, but with a clean and not-D3D11 oGPU_BUFFER_DESC, does that 
	// hold all the info needed and we just ensure it always gets populated as 
	// expected (unlike D3D11's StructByteSize)? Probably, but this needs to stick 
	// around a bit longer until it can truly be orphaned.
	gpu::buffer_info i(_Info);
	i.struct_byte_size = as_ushort(ElementStride);
	d3d::set_info(Buffer, i);

	if (_Info.type >= gpu::buffer_type::unordered_raw)
	{
		D3D11_UNORDERED_ACCESS_VIEW_DESC UAVD;
		UAVD.Format = dxgi::from_surface_format(Format);
		UAVD.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
		UAVD.Buffer.FirstElement = 0;
		UAVD.Buffer.NumElements = _Info.array_size;
		UAVD.Buffer.Flags = 0;

		switch (_Info.type)
		{
			case gpu::buffer_type::unordered_raw: UAVD.Buffer.Flags = D3D11_BUFFER_UAV_FLAG_RAW; break;
			case gpu::buffer_type::unordered_structured_append: UAVD.Buffer.Flags = D3D11_BUFFER_UAV_FLAG_APPEND; break;
			case gpu::buffer_type::unordered_structured_counter: UAVD.Buffer.Flags = D3D11_BUFFER_UAV_FLAG_COUNTER; break;
			default: break;
		}

		oV(_pDevice->CreateUnorderedAccessView(Buffer, &UAVD, _ppUAV));
		debug_name(*_ppUAV, _DebugName);

		if (_Info.type >= gpu::buffer_type::unordered_structured)
		{
			oV(_pDevice->CreateShaderResourceView(Buffer, nullptr, _ppSRV));
			debug_name(*_ppSRV, _DebugName);
		}
	}
	
	else if (_ppUAV)
		*_ppUAV = nullptr;

	return Buffer;
}

intrusive_ptr<Resource> make_cpu_copy(Resource* _pResource)
{
	intrusive_ptr<Device> D3D11Device;
	_pResource->GetDevice(&D3D11Device);
	intrusive_ptr<DeviceContext> D3D11DeviceContext;
	D3D11Device->GetImmediateContext(&D3D11DeviceContext);

	lstring RTName;
	debug_name(RTName, _pResource);

	lstring copyName;
	snprintf(copyName, "%s.CPUCopy", RTName.c_str());

	intrusive_ptr<Resource> CPUCopy;

	D3D11_RESOURCE_DIMENSION type;
	_pResource->GetType(&type);
	switch (type)
	{
		case D3D11_RESOURCE_DIMENSION_BUFFER:
		{
			D3D11_BUFFER_DESC d;
			static_cast<Buffer*>(_pResource)->GetDesc(&d);
			d.Usage = D3D11_USAGE_STAGING;
			d.CPUAccessFlags = /*D3D11_CPU_ACCESS_WRITE|*/D3D11_CPU_ACCESS_READ;
			d.BindFlags = 0;
			oV(D3D11Device->CreateBuffer(&d, nullptr, (Buffer**)&CPUCopy));
			debug_name(CPUCopy, copyName);
			break;
		}

		case D3D11_RESOURCE_DIMENSION_TEXTURE1D:
		case D3D11_RESOURCE_DIMENSION_TEXTURE2D:
		case D3D11_RESOURCE_DIMENSION_TEXTURE3D:
		{
			gpu::texture_info i = get_texture_info(_pResource);
			i.type = gpu::make_readback(i.type);
			new_texture NewTexture = make_texture(D3D11Device, copyName, i, nullptr);
			CPUCopy = NewTexture.pResource;
			break;
		}

		default:
			throw std::invalid_argument("unknown resource type");
	}

	try
	{
		gpu::buffer_info i = d3d::get_info(_pResource);
		set_info(CPUCopy, i);
	}
	catch(std::exception&) {}
	
	D3D11DeviceContext->CopyResource(CPUCopy, _pResource);
	D3D11DeviceContext->Flush();
	return CPUCopy;
}

void copy(Resource* _pTexture
	, unsigned int _Subresource
	, surface::mapped_subresource* _pDstSubresource
	, bool _FlipVertically)
{
	if (!_pTexture)
		oTHROW_INVARG0();

	oCHECK_IS_TEXTURE(_pTexture);

	intrusive_ptr<Device> Device;
	_pTexture->GetDevice(&Device);
	intrusive_ptr<DeviceContext> D3DDeviceContext;
	Device->GetImmediateContext(&D3DDeviceContext);

	gpu::texture_info info = get_texture_info(_pTexture);

	if (!gpu::is_readback(info.type))
	{
		mstring buf;
		oTHROW_INVARG("The specified texture %s does not have CPU read access", debug_name(buf, _pTexture));
	}

	D3D11_MAPPED_SUBRESOURCE source;
	oV(D3DDeviceContext->Map(_pTexture, _Subresource, D3D11_MAP_READ, 0, &source));

	int2 ByteDimensions = surface::byte_dimensions(info.format, info.dimensions);
	memcpy2d(_pDstSubresource->data, _pDstSubresource->row_pitch, source.pData, source.RowPitch, ByteDimensions.x, ByteDimensions.y, _FlipVertically);
	D3DDeviceContext->Unmap(_pTexture, _Subresource);
}

template<typename DescT> static void fill_non_dimensions(const DescT& _Desc, bool _AsArray, texture_type::value _BasicType, texture_info* _pInfo)
{
	if (_Desc.MiscFlags & D3D11_RESOURCE_MISC_TEXTURECUBE)
		_BasicType = texture_type::default_cube;

	_pInfo->format = dxgi::to_surface_format(_Desc.Format);

	_pInfo->type = _BasicType;
	if (_Desc.MipLevels > 1)
		_pInfo->type = add_mipped(_pInfo->type);

	if (_AsArray)
		_pInfo->type = add_array(_pInfo->type);
	else
		_pInfo->array_size = 0;

	if (_Desc.Usage == D3D11_USAGE_STAGING)
		_pInfo->type = make_readback(_pInfo->type);

	if (_Desc.BindFlags & (D3D11_BIND_RENDER_TARGET|D3D11_BIND_DEPTH_STENCIL))
		_pInfo->type = make_render_target(_pInfo->type);

	if (_Desc.BindFlags & D3D11_BIND_UNORDERED_ACCESS)
	{
		oASSERT(_pInfo->type == texture_type::default_2d, "Invalid/unhandled type");
		_pInfo->type = texture_type::unordered_2d;
	}
}

texture_info get_texture_info(Resource* _pResource, bool _AsArray, D3D11_USAGE* _pUsage)
{
	texture_info info;

	D3D11_RESOURCE_DIMENSION type = D3D11_RESOURCE_DIMENSION_UNKNOWN;
	_pResource->GetType(&type);
	switch (type)
	{
		case D3D11_RESOURCE_DIMENSION_TEXTURE1D:
		{
			D3D11_TEXTURE1D_DESC desc;
			static_cast<Texture1D*>(_pResource)->GetDesc(&desc);
			info.dimensions = ushort3(static_cast<unsigned short>(desc.Width), 1, 1);
			info.array_size = static_cast<unsigned short>(desc.ArraySize);
			fill_non_dimensions(desc, _AsArray || desc.ArraySize > 1, texture_type::default_1d, &info);
			if (_pUsage) *_pUsage = desc.Usage;
			break;
		}

		case D3D11_RESOURCE_DIMENSION_TEXTURE2D:
		{
			D3D11_TEXTURE2D_DESC desc;
			static_cast<Texture2D*>(_pResource)->GetDesc(&desc);
			info.dimensions = ushort3(static_cast<unsigned short>(desc.Width)
				, static_cast<unsigned short>(desc.Height), 1);
			info.array_size = static_cast<unsigned short>(desc.ArraySize);
			fill_non_dimensions(desc, _AsArray || desc.ArraySize > 1, texture_type::default_2d, &info);
			if (_pUsage) *_pUsage = desc.Usage;
			break;
		}

		case D3D11_RESOURCE_DIMENSION_TEXTURE3D:
		{
			D3D11_TEXTURE3D_DESC desc;
			static_cast<Texture3D*>(_pResource)->GetDesc(&desc);
			info.dimensions = ushort3(static_cast<unsigned short>(desc.Width)
				, static_cast<unsigned short>(desc.Height)
				, static_cast<unsigned short>(desc.Depth));
			info.array_size = 1;
			fill_non_dimensions(desc, false, texture_type::default_3d, &info);
			if (_pUsage) *_pUsage = desc.Usage;
			break;
		}

		case D3D11_RESOURCE_DIMENSION_BUFFER:
		{
			buffer_info i = get_info(_pResource);
			D3D11_BUFFER_DESC desc;
			static_cast<Buffer*>(_pResource)->GetDesc(&desc);
			info.dimensions = ushort3(i.struct_byte_size, static_cast<ushort>(i.array_size), 1);
			info.array_size = i.array_size;
			info.format = i.format;
			if (_pUsage) *_pUsage = desc.Usage;
			break;
		};

		oNODEFAULT;
	}

	return info;
}

static D3D11_SHADER_RESOURCE_VIEW_DESC get_srv_desc(const gpu::texture_info& _Info, D3D11_RESOURCE_DIMENSION _Type)
{
	D3D11_SHADER_RESOURCE_VIEW_DESC d;

	DXGI_FORMAT TF, DSVF;
	dxgi::get_compatible_formats(dxgi::from_surface_format(_Info.format), &TF, &DSVF, &d.Format);

	// All texture share basically the same memory footprint, so just write once
	d.Texture2DArray.MostDetailedMip = 0;
	d.Texture2DArray.MipLevels = __max(1, surface::num_mips(gpu::is_mipped(_Info.type), _Info.dimensions));
	d.Texture2DArray.FirstArraySlice = 0;
	d.Texture2DArray.ArraySize = _Info.array_size;

	switch (_Type)
	{
		case D3D11_RESOURCE_DIMENSION_TEXTURE1D:
			d.ViewDimension = gpu::is_array(_Info.type) ? D3D11_SRV_DIMENSION_TEXTURE1DARRAY : D3D11_SRV_DIMENSION_TEXTURE1D;
			break;
		case D3D11_RESOURCE_DIMENSION_TEXTURE2D:
			if (gpu::is_cube(_Info.type))
				d.ViewDimension = gpu::is_array(_Info.type) ? D3D11_SRV_DIMENSION_TEXTURECUBEARRAY : D3D11_SRV_DIMENSION_TEXTURECUBE;
			else
				d.ViewDimension = gpu::is_array(_Info.type) ? D3D11_SRV_DIMENSION_TEXTURE2DARRAY : D3D11_SRV_DIMENSION_TEXTURE2D;
			break;
		case D3D11_RESOURCE_DIMENSION_TEXTURE3D:
			d.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE3D;
			break;
		oNODEFAULT;
	}

	return d;
}

intrusive_ptr<ShaderResourceView> make_srv(const char* _DebugName, Resource* _pTexture, bool _AsArray)
{
	D3D11_SHADER_RESOURCE_VIEW_DESC SRVDesc;
	D3D11_SHADER_RESOURCE_VIEW_DESC* pSRVDesc = nullptr;

	gpu::texture_info info = get_texture_info(_pTexture, _AsArray);

	if (surface::is_depth(info.format))
	{
		D3D11_RESOURCE_DIMENSION type;
		_pTexture->GetType(&type);
		SRVDesc = get_srv_desc(info, type);
		pSRVDesc = &SRVDesc;
	}

	intrusive_ptr<Device> Device;
	_pTexture->GetDevice(&Device);
	intrusive_ptr<ShaderResourceView> SRV;

	oV(Device->CreateShaderResourceView(_pTexture, pSRVDesc, &SRV));
	debug_name(SRV, _DebugName);
	return SRV;
}

static D3D11_DEPTH_STENCIL_VIEW_DESC get_dsv_desc(const gpu::texture_info& _Info, D3D11_RESOURCE_DIMENSION _Type)
{
	D3D11_DEPTH_STENCIL_VIEW_DESC DSVDesc;
	if (_Type != D3D11_RESOURCE_DIMENSION_TEXTURE2D)
		throw std::invalid_argument(formatf("Unsupported resource dimension (%s)", as_string(_Type)));
	DXGI_FORMAT TF, SRVF;
	dxgi::get_compatible_formats(dxgi::from_surface_format(_Info.format), &TF, &DSVDesc.Format, &SRVF);
	DSVDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	DSVDesc.Texture2D.MipSlice = 0;
	DSVDesc.Flags = 0;
	return DSVDesc;
}

intrusive_ptr<View> make_rtv(const char* _DebugName, Resource* _pTexture)
{
	intrusive_ptr<Device> Device;
	_pTexture->GetDevice(&Device);
	HRESULT hr = S_OK;
	gpu::texture_info info = get_texture_info(_pTexture);

	intrusive_ptr<View> View;
	if (surface::is_depth(info.format))
	{
		D3D11_RESOURCE_DIMENSION type;
		_pTexture->GetType(&type);
		D3D11_DEPTH_STENCIL_VIEW_DESC dsv = get_dsv_desc(info, type);
		oV(Device->CreateDepthStencilView(_pTexture, &dsv, (DepthStencilView**)&View));
	}
	else
		oV(Device->CreateRenderTargetView(_pTexture, nullptr, (RenderTargetView**)&View));

	debug_name(View, _DebugName);
	return View;
}

intrusive_ptr<UnorderedAccessView> make_uav(const char* _DebugName
	, Resource* _pTexture, unsigned int _MipSlice, unsigned int _ArraySlice)
{
	intrusive_ptr<Device> Device;
	_pTexture->GetDevice(&Device);
	gpu::texture_info info = get_texture_info(_pTexture);
	D3D11_UNORDERED_ACCESS_VIEW_DESC UAVDesc;
	UAVDesc.Format = dxgi::from_surface_format(info.format);
	switch (info.type)
	{
		// @tony: When adding more cases to this, try to use texture_info::array_size

		case gpu::texture_type::unordered_2d:
			UAVDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
			UAVDesc.Texture2D.MipSlice = 0;
			break;
		default:
			throw std::invalid_argument(formatf("Invalid texture type %s specified for UAV creation", as_string(info.type)));
	}

	intrusive_ptr<UnorderedAccessView> UAV;
	oV(Device->CreateUnorderedAccessView(_pTexture, &UAVDesc, &UAV));
	debug_name(UAV, _DebugName);
	return UAV;
}

intrusive_ptr<UnorderedAccessView> make_unflagged_copy(UnorderedAccessView* _pSourceUAV)
{
	D3D11_UNORDERED_ACCESS_VIEW_DESC UAVD;
	_pSourceUAV->GetDesc(&UAVD);
	if (UAVD.ViewDimension != D3D11_UAV_DIMENSION_BUFFER)
		throw std::invalid_argument("Only D3D11_UAV_DIMENSION_BUFFER views supported");
	intrusive_ptr<Resource> Resource;
	_pSourceUAV->GetResource(&Resource);
	intrusive_ptr<Device> Device;
	Resource->GetDevice(&Device);
	UAVD.Buffer.Flags = 0;
	intrusive_ptr<UnorderedAccessView> UAV;
	oV(Device->CreateUnorderedAccessView(Resource, &UAVD, &UAV));
	return UAV;
}

#define oD3D11_TRACE_UINT_S(struct_, x) oTRACE("%s" #x "=%u", oSAFESTR(_Prefix), struct_.x)
#define oD3D11_TRACE_ENUM_S(struct_, x) oTRACE("%s" #x "=%s", oSAFESTR(_Prefix), as_string(struct_.x))
#define oD3D11_TRACE_FLAGS_S(_FlagEnumType, struct_, _FlagsVar, _AllZeroString) do { char buf[512]; strbitmask(buf, struct_._FlagsVar, _AllZeroString, as_string<_FlagEnumType>); oTRACE("%s" #_FlagsVar "=%s", oSAFESTR(_Prefix), buf); } while(false)

void trace_texture2d_desc(const D3D11_TEXTURE2D_DESC& _Desc, const char* _Prefix)
{
	#define oD3D11_TRACE_UINT(x) oD3D11_TRACE_UINT_S(_Desc, x)
	#define oD3D11_TRACE_ENUM(x) oD3D11_TRACE_ENUM_S(_Desc, x)
	#define oD3D11_TRACE_FLAGS(_FlagEnumType, _FlagsVar, _AllZeroString) oD3D11_TRACE_FLAGS_S(_FlagEnumType, _Desc, _FlagsVar, _AllZeroString)

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

#define oDXGI_FORMAT_FROM_FILE ((DXGI_FORMAT)-3)
void init_values(const gpu::texture_info& _Info
	, DXGI_FORMAT* _pFormat
	, D3D11_USAGE* _pUsage
	, unsigned int* _pCPUAccessFlags
	, unsigned int* _pBindFlags
	, unsigned int* _pMipLevels
	, unsigned int* _pMiscFlags)
{
	DXGI_FORMAT DSVF, SRVF;
	dxgi::get_compatible_formats(dxgi::from_surface_format(_Info.format), _pFormat, &DSVF, &SRVF);
	if (*_pFormat == DXGI_FORMAT_UNKNOWN)
		*_pFormat = oDXGI_FORMAT_FROM_FILE;

	*_pUsage = D3D11_USAGE_DEFAULT;
	if (gpu::is_readback(_Info.type))
		*_pUsage = D3D11_USAGE_STAGING;

	if (_pMiscFlags)
		*_pMiscFlags = 0;

	*_pCPUAccessFlags = cpu_write_flags(*_pUsage);
	*_pMipLevels = gpu::is_mipped(_Info.type) ? 0 : 1;

	if (_pMiscFlags && gpu::is_cube(_Info.type))
			*_pMiscFlags |= D3D11_RESOURCE_MISC_TEXTURECUBE;

	*_pBindFlags = 0;
	if (*_pUsage != D3D11_USAGE_STAGING)
	{
		*_pBindFlags = D3D11_BIND_SHADER_RESOURCE;
		if (gpu::is_render_target(_Info.type))
		{
			*_pBindFlags |= D3D11_BIND_RENDER_TARGET;

			// D3D11_RESOURCE_MISC_GENERATE_MIPS is only valid for render targets.
			// It is up to client code to handle default textures and depth textures
			// rebound for sampling.
			if (_pMiscFlags && gpu::is_mipped(_Info.type))
				*_pMiscFlags |= D3D11_RESOURCE_MISC_GENERATE_MIPS;
		}
	}

	if (surface::is_depth(dxgi::to_surface_format(*_pFormat)) && *_pFormat != oDXGI_FORMAT_FROM_FILE)
	{
		*_pBindFlags &=~ D3D11_BIND_RENDER_TARGET;
		*_pBindFlags |= D3D11_BIND_DEPTH_STENCIL;
	}

	if (gpu::is_unordered(_Info.type))
		*_pBindFlags |= D3D11_BIND_UNORDERED_ACCESS;
}

#if 0
static new_texture make_second_texture(Device* _pDevice, const char* _Texture1Name, const texture_info& _Texture1Info)
{
	oCHECK(surface::num_subformats(_Texture1Info.format) <= 2, "Many-plane textures not supported");
	if (surface::num_subformats(_Texture1Info.format) == 2)
	{
		// To keep YUV textures singular to prepare for new YUV-based DXGI formats
		// coming, create a private data companion texture.
		texture_info Texture2Info(_Texture1Info);
		Texture2Info.format = surface::subformat(_Texture1Info.format, 1);
		Texture2Info.dimensions = surface::dimensions_npot(_Texture1Info.format, _Texture1Info.dimensions, 0, 1);

		mstring Texture2Name(_Texture1Name);
		sncatf(Texture2Name, ".Texture2");

		return make_texture(_pDevice, Texture2Name, Texture2Info);
	}

	return new_texture();
}
#endif

new_texture make_texture(Device* _pDevice
	, const char* _DebugName
	, const gpu::texture_info& _Info
	, surface::const_mapped_subresource* _pInitData)
{
	new_texture NewTexture;
	bool IsShaderResource = false;
	intrusive_ptr<Resource> Texture;
	switch (gpu::get_type(_Info.type))
	{
		case gpu::texture_type::default_1d:
		{
			D3D11_TEXTURE1D_DESC desc;
			desc.Width = _Info.dimensions.x;
			desc.ArraySize = __max(1, _Info.array_size);
			init_values(_Info, &desc.Format, &desc.Usage, &desc.CPUAccessFlags, &desc.BindFlags, &desc.MipLevels, &desc.MiscFlags);
			IsShaderResource = !!(desc.BindFlags & D3D11_BIND_SHADER_RESOURCE);
			oV(_pDevice->CreateTexture1D(&desc, (D3D11_SUBRESOURCE_DATA*)_pInitData, &NewTexture.pTexture1D));
			break;
		}

		case gpu::texture_type::default_2d:
		case gpu::texture_type::default_cube:
		{
			if (gpu::is_cube(_Info.type) && _Info.array_size != 6)
				throw std::invalid_argument(formatf("Cube maps must have ArraySize == 6, ArraySize=%d specified", _Info.array_size));

			D3D11_TEXTURE2D_DESC desc;
			desc.Width = _Info.dimensions.x;
			desc.Height = _Info.dimensions.y;
			desc.ArraySize = __max(1, _Info.array_size);
			desc.SampleDesc.Count = 1;
			desc.SampleDesc.Quality = 0;
			init_values(_Info, &desc.Format, &desc.Usage, &desc.CPUAccessFlags, &desc.BindFlags, &desc.MipLevels, &desc.MiscFlags);
			IsShaderResource = !!(desc.BindFlags & D3D11_BIND_SHADER_RESOURCE);
			oV(_pDevice->CreateTexture2D(&desc, (D3D11_SUBRESOURCE_DATA*)_pInitData, &NewTexture.pTexture2D));
			break;
		}

		case gpu::texture_type::default_3d:
		{
			if (_Info.array_size > 1)
				throw std::invalid_argument(formatf("3d textures don't support slices, ArraySize=%d specified", _Info.array_size));
			D3D11_TEXTURE3D_DESC desc;
			desc.Width = _Info.dimensions.x;
			desc.Height = _Info.dimensions.y;
			desc.Depth = _Info.dimensions.z;
			init_values(_Info, &desc.Format, &desc.Usage, &desc.CPUAccessFlags, &desc.BindFlags, &desc.MipLevels, &desc.MiscFlags);
			IsShaderResource = !!(desc.BindFlags & D3D11_BIND_SHADER_RESOURCE);
			oV(_pDevice->CreateTexture3D(&desc, (D3D11_SUBRESOURCE_DATA*)_pInitData, &NewTexture.pTexture3D));
			break;
		}
		oNODEFAULT;
	}

	debug_name(NewTexture.pResource, _DebugName);
	if (IsShaderResource)
	{
		auto srv = make_srv(_DebugName, NewTexture.pResource, _Info.array_size != 0);
		NewTexture.pSRV = srv;
		NewTexture.pSRV->AddRef();
	}

	if (gpu::is_render_target(_Info.type))
	{
		auto view = make_rtv(_DebugName, NewTexture.pResource);
		NewTexture.pView = view;
		NewTexture.pView->AddRef();
	}

	return NewTexture;
}

std::shared_ptr<surface::buffer> make_snapshot(Texture2D* _pRenderTarget)
{
	if (!_pRenderTarget)
		throw std::invalid_argument("invalid render target");

	intrusive_ptr<Resource> CPUTexture = make_cpu_copy(_pRenderTarget);
	Texture2D* CPUTexture2D = (Texture2D*)CPUTexture.c_ptr();

	D3D11_TEXTURE2D_DESC d;
	CPUTexture2D->GetDesc(&d);

	if (d.Format == DXGI_FORMAT_UNKNOWN)
		throw std::invalid_argument(formatf("The specified texture's format %s is not supported by oImage", as_string(d.Format)));

	surface::info si;
	si.format = surface::b8g8r8a8_unorm;
	si.dimensions = int3(d.Width, d.Height, 1);
	std::shared_ptr<surface::buffer> s = surface::buffer::make(si);

	surface::lock_guard lock(s);
	copy(CPUTexture2D, 0, &lock.mapped);

	return s;
}



		} // namespace d3d
	} // namespace gpu
} // namespace ouro
