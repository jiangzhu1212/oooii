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
#include "d3d_util.h"
#include "d3d_debug.h"
#include "dxgi_util.h"
#include <d3d11.h>
#include <oGPU/shader.h>
#include <oCore/windows/win_util.h>

namespace ouro { namespace gpu { namespace d3d {

#define oCHECK_IS_TEXTURE(_pResource) do \
{	D3D11_RESOURCE_DIMENSION type; \
	_pResource->GetType(&type); \
	if (type != D3D11_RESOURCE_DIMENSION_TEXTURE1D && type != D3D11_RESOURCE_DIMENSION_TEXTURE2D && type != D3D11_RESOURCE_DIMENSION_TEXTURE3D) \
	{	mstring buf; \
		oTHROW_INVARG("Only textures types are currently supported. (resource %s)", debug_name(buf, _pResource)); \
	} \
} while (false)

Texture1D* make_texture_1d(const char* name, Device* dev, surface::format format, uint width, uint array_size, bool mips)
{
	CD3D11_TEXTURE1D_DESC d(dxgi::from_surface_format(format)
		, width
		, __max(1, array_size)
		, mips ? 0 : 1);

	Texture1D* t = nullptr;
	oV(dev->CreateTexture1D(&d, nullptr, &t));
	debug_name(t, name);
	return t;
}

Texture2D* make_texture_2d(const char* name, Device* dev, surface::format format, uint width, uint height, uint array_size, bool mips, bool cube, bool target)
{
	CD3D11_TEXTURE2D_DESC d(dxgi::from_surface_format(format)
		, width
		, height
		, cube ? __min(6, array_size*6) : __max(1, array_size)
		, mips ? 0 : 1
		, D3D11_BIND_SHADER_RESOURCE | (target ? D3D11_BIND_RENDER_TARGET : 0)
		, D3D11_USAGE_DEFAULT
		, 0
		, 1
		, 0
		, (target && mips ? D3D11_RESOURCE_MISC_GENERATE_MIPS : 0) | (cube ? D3D11_RESOURCE_MISC_TEXTURECUBE : 0));

	Texture2D* t = nullptr;
	oV(dev->CreateTexture2D(&d, nullptr, &t));
	debug_name(t, name);
	return t;
}

Texture3D* make_texture_3d(const char* name, Device* dev, surface::format format, uint width, uint height, uint depth, bool mips)
{
	CD3D11_TEXTURE3D_DESC d(dxgi::from_surface_format(format)
		, width
		, height
		, depth
		, mips ? 0 : 1);

	Texture3D* t = nullptr;
	oV(dev->CreateTexture3D(&d, nullptr, &t));
	debug_name(t, name);
	return t;
}

D3D_TEXTURE_DESC get_texture_desc(Resource* r)
{
	D3D_TEXTURE_DESC d;
	r->GetType(&d.type);
	switch (d.type)
	{
		case D3D11_RESOURCE_DIMENSION_TEXTURE1D:
		{
			D3D11_TEXTURE1D_DESC desc;
			static_cast<Texture1D*>(r)->GetDesc(&desc);
			d.dimensions = ushort3(static_cast<ushort>(desc.Width), 1, 1);
			d.array_size = static_cast<ushort>(desc.ArraySize);
			d.format = desc.Format;
			d.usage = desc.Usage;
			break;
		}

		case D3D11_RESOURCE_DIMENSION_TEXTURE2D:
		{
			D3D11_TEXTURE2D_DESC desc;
			static_cast<Texture2D*>(r)->GetDesc(&desc);
			d.dimensions = ushort3(static_cast<ushort>(desc.Width), static_cast<ushort>(desc.Height), 1);
			d.array_size = static_cast<ushort>(desc.ArraySize);
			d.format = desc.Format;
			d.usage = desc.Usage;
			break;
		}

		case D3D11_RESOURCE_DIMENSION_TEXTURE3D:
		{
			D3D11_TEXTURE3D_DESC desc;
			static_cast<Texture3D*>(r)->GetDesc(&desc);
			d.dimensions = ushort3(static_cast<ushort>(desc.Width), static_cast<ushort>(desc.Height), static_cast<ushort>(desc.Depth));
			d.array_size = 1;
			d.format = desc.Format;
			d.usage = desc.Usage;
			break;
		}

		case D3D11_RESOURCE_DIMENSION_BUFFER:
		{
			D3D11_BUFFER_DESC desc;
			static_cast<Buffer*>(r)->GetDesc(&desc);
			const ushort stride = static_cast<ushort>(__max(1, desc.StructureByteStride));
			const ushort num = static_cast<ushort>(desc.ByteWidth / stride);
			d.dimensions = ushort3(stride, num, 1);
			d.array_size = num;
			d.format = DXGI_FORMAT_UNKNOWN;
			d.usage = desc.Usage;
			break;
		};

		oNODEFAULT;
	}

	return d;
}

ShaderResourceView* make_srv(Resource* r, DXGI_FORMAT format, uint array_size)
{
	D3D_TEXTURE_DESC desc = get_texture_desc(r);

	D3D11_SHADER_RESOURCE_VIEW_DESC d;
	d.Format = format != DXGI_FORMAT_UNKNOWN ? format : desc.format;

	bool is_array = !!array_size;

	switch (desc.type)
	{
		case D3D11_RESOURCE_DIMENSION_TEXTURE2D:
		{
			D3D11_TEXTURE2D_DESC desc;
			((Texture2D*)r)->GetDesc(&desc);
			d.ViewDimension = (desc.MiscFlags & D3D11_RESOURCE_MISC_TEXTURECUBE) ? D3D_SRV_DIMENSION_TEXTURECUBE : D3D_SRV_DIMENSION_TEXTURE2D;
			is_array = array_size > 6;
			break;
		}
		
		case D3D11_RESOURCE_DIMENSION_TEXTURE1D: d.ViewDimension = D3D_SRV_DIMENSION_TEXTURE1D; break;
		case D3D11_RESOURCE_DIMENSION_TEXTURE3D: d.ViewDimension = D3D_SRV_DIMENSION_TEXTURE3D; array_size = 0; break;
		oNODEFAULT;
	}
	
	// because of unions, this works for all texture types
	if (is_array)
	{
		d.ViewDimension = D3D_SRV_DIMENSION(d.ViewDimension + 1);
		d.Texture2DArray.MostDetailedMip = 0;
		d.Texture2DArray.MipLevels = ~0u;
		d.Texture2DArray.FirstArraySlice = 0;
		d.Texture2DArray.ArraySize = array_size;
	}

	else
	{
		d.Texture2D.MostDetailedMip = 0;
		d.Texture2D.MipLevels = ~0u;
	}

	intrusive_ptr<Device> dev;
	r->GetDevice(&dev);

	ShaderResourceView* srv = nullptr;
	oV(dev->CreateShaderResourceView(r, &d, &srv));

	mstring name;
	debug_name(srv, debug_name(name, r));
	return srv;
}

ShaderResourceView* make_srv(Resource* r, surface::format format, uint array_size)
{
	return make_srv(r, dxgi::from_surface_format(format), array_size);
}

View* make_draw_target(Resource* r)
{
	intrusive_ptr<Device> Device;
	r->GetDevice(&Device);
	HRESULT hr = S_OK;
	D3D_TEXTURE_DESC desc = get_texture_desc(r);

	intrusive_ptr<View> View;
	if (dxgi::is_depth(desc.format))
	{
		oCHECK_ARG(desc.type == D3D11_RESOURCE_DIMENSION_TEXTURE2D, "unsupported resource dimension (%s)", as_string(desc.type));
		
		D3D11_DEPTH_STENCIL_VIEW_DESC dsvd;
		DXGI_FORMAT TF, SRVF;
		dxgi::get_compatible_formats(desc.format, &TF, &dsvd.Format, &SRVF);
		dsvd.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		dsvd.Texture2D.MipSlice = 0;
		dsvd.Flags = 0;

		oV(Device->CreateDepthStencilView(r, &dsvd, (DepthStencilView**)&View));
	}
	else
		oV(Device->CreateRenderTargetView(r, nullptr, (RenderTargetView**)&View));

	mstring n;
	debug_name(n, r);
	debug_name(View, n);
	return View;
}

void set_cbs(DeviceContext* dc, uint slot, uint num_buffers, Buffer* const* buffers, uint gpu_stage_flags)
{
	if (gpu_stage_flags & stage_flag::vertex) dc->VSSetConstantBuffers(slot, num_buffers, buffers);
	if (gpu_stage_flags & stage_flag::hull) dc->HSSetConstantBuffers(slot, num_buffers, buffers);
	if (gpu_stage_flags & stage_flag::domain) dc->DSSetConstantBuffers(slot, num_buffers, buffers);
	if (gpu_stage_flags & stage_flag::geometry) dc->GSSetConstantBuffers(slot, num_buffers, buffers);
	if (gpu_stage_flags & stage_flag::pixel) dc->PSSetConstantBuffers(slot, num_buffers, buffers);
	if (gpu_stage_flags & stage_flag::compute) dc->CSSetConstantBuffers(slot, num_buffers, buffers);
}

static bool has_unordered_buffers(uint num_srvs, ShaderResourceView* const* srvs)
{
	if (!srvs)
		return false;

	for (uint i = 0; i < num_srvs; i++)
	{
		if (!srvs[i])
			continue;
		
		intrusive_ptr<UnorderedAccessView> uav;
		if (SUCCEEDED(srvs[i]->QueryInterface(IID_PPV_ARGS(&uav))))
			return true;
	}
	
	return false;
}

void set_srvs(DeviceContext* dc, uint slot, uint num_srvs, ShaderResourceView* const* srvs, uint gpu_stage_flags)
{
	if (has_unordered_buffers(num_srvs, srvs))
		gpu_stage_flags &= ~(stage_flag::vertex|stage_flag::hull|stage_flag::domain|stage_flag::geometry);

	if (gpu_stage_flags & stage_flag::vertex) dc->VSSetShaderResources(slot, num_srvs, srvs);
	if (gpu_stage_flags & stage_flag::hull) dc->HSSetShaderResources(slot, num_srvs, srvs);
	if (gpu_stage_flags & stage_flag::domain) dc->DSSetShaderResources(slot, num_srvs, srvs);
	if (gpu_stage_flags & stage_flag::geometry) dc->GSSetShaderResources(slot, num_srvs, srvs);
	if (gpu_stage_flags & stage_flag::pixel) dc->PSSetShaderResources(slot, num_srvs, srvs);
	if (gpu_stage_flags & stage_flag::compute) dc->CSSetShaderResources(slot, num_srvs, srvs);
}

void set_samplers(DeviceContext* dc, uint slot, uint num_samplers, const SamplerState* const* samplers, uint gpu_stage_flags)
{
	SamplerState* const* s = const_cast<SamplerState* const*>(samplers);
	if (gpu_stage_flags & stage_flag::vertex) dc->VSSetSamplers(slot, num_samplers, s);
	if (gpu_stage_flags & stage_flag::hull) dc->HSSetSamplers(slot, num_samplers, s);
	if (gpu_stage_flags & stage_flag::domain) dc->DSSetSamplers(slot, num_samplers, s);
	if (gpu_stage_flags & stage_flag::geometry) dc->GSSetSamplers(slot, num_samplers, s);
	if (gpu_stage_flags & stage_flag::pixel) dc->PSSetSamplers(slot, num_samplers, s);
	if (gpu_stage_flags & stage_flag::compute)dc->CSSetSamplers(slot, num_samplers, s);
}

Buffer* make_buffer(const char* name, Device* dev, uint element_stride, uint num_elements, D3D11_USAGE usage, uint bind_flags, uint misc_flags, const void* init_data)
{
	D3D11_BUFFER_DESC d = {0};
	d.ByteWidth = num_elements * element_stride;
	d.Usage = usage;
	d.BindFlags = bind_flags;
	d.CPUAccessFlags = usage == D3D11_USAGE_STAGING ? D3D11_CPU_ACCESS_READ : 0;
	d.MiscFlags = misc_flags;
	d.StructureByteStride = (misc_flags & D3D11_RESOURCE_MISC_BUFFER_STRUCTURED) ? element_stride : 0;

	D3D11_SUBRESOURCE_DATA data;
	data.pSysMem = init_data;
	data.SysMemPitch = d.ByteWidth;
	data.SysMemSlicePitch = d.ByteWidth;

	Buffer* b = nullptr;
	oV(dev->CreateBuffer(&d, init_data ? &data : nullptr, &b));
	debug_name(b, name);
	return b;
}

void make_structured(const char* name, Device* dev, uint struct_stride, uint num_structs, const void* src, uint uav_flags, ShaderResourceView** out_srv, UnorderedAccessView** out_uav)
{
	intrusive_ptr<Buffer> buffer = make_buffer(name, dev, struct_stride, num_structs, D3D11_USAGE_DEFAULT, D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS, D3D11_RESOURCE_MISC_BUFFER_STRUCTURED, src);

	if (out_srv)
	{
		oV(dev->CreateShaderResourceView(buffer, nullptr, out_srv));
		debug_name(*out_srv, name);
	}
	
	if (out_uav)
	{
		D3D11_UNORDERED_ACCESS_VIEW_DESC ud;
		ud.Format = DXGI_FORMAT_UNKNOWN;
		ud.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
		ud.Buffer.FirstElement = 0;
		ud.Buffer.NumElements = num_structs;
		ud.Buffer.Flags = uav_flags;

		oV(dev->CreateUnorderedAccessView(buffer, &ud, out_uav));
		debug_name(*out_uav, name);
	}
}

template<typename ResourceDescT>
static void set_staging(ResourceDescT* desc)
{
	desc->Usage = D3D11_USAGE_STAGING;
	desc->CPUAccessFlags = D3D11_CPU_ACCESS_READ;
	desc->BindFlags = 0;
}

Resource* make_cpu_copy(Resource* src, bool do_copy)
{
	intrusive_ptr<Device> dev;
	src->GetDevice(&dev);
	intrusive_ptr<DeviceContext> dc;
	dev->GetImmediateContext(&dc);

	Resource* CPUCopy;

	D3D11_RESOURCE_DIMENSION type = D3D11_RESOURCE_DIMENSION_UNKNOWN;
	src->GetType(&type);

	switch (type)
	{
		case D3D11_RESOURCE_DIMENSION_BUFFER:
		{
			D3D11_BUFFER_DESC d;
			((Buffer*)src)->GetDesc(&d);
			set_staging(&d);
			oV(dev->CreateBuffer(&d, nullptr, (Buffer**)&CPUCopy));
			break;
		}

		case D3D11_RESOURCE_DIMENSION_TEXTURE1D:
		{
			D3D11_TEXTURE1D_DESC d;
			((Texture1D*)src)->GetDesc(&d);
			set_staging(&d);
			oV(dev->CreateTexture1D(&d, nullptr, (Texture1D**)&CPUCopy));
			break;
		}

		case D3D11_RESOURCE_DIMENSION_TEXTURE2D:
		{
			D3D11_TEXTURE2D_DESC d;
			((Texture2D*)src)->GetDesc(&d);
			set_staging(&d);
			oV(dev->CreateTexture2D(&d, nullptr, (Texture2D**)&CPUCopy));
			break;
		}

		case D3D11_RESOURCE_DIMENSION_TEXTURE3D:
		{
			D3D11_TEXTURE3D_DESC d;
			((Texture3D*)src)->GetDesc(&d);
			set_staging(&d);
			oV(dev->CreateTexture3D(&d, nullptr, (Texture3D**)&CPUCopy));
			break;
		}

		default:
			oTHROW_INVARG("unknown resource type");
	}

	// set the debug name
	{
		lstring rname;
		debug_name(rname, src);

		lstring copyName;
		snprintf(copyName, "%s.cpu_copy", rname.c_str());

		debug_name(CPUCopy, copyName);
	}

	if (do_copy)
	{
		dc->CopyResource(CPUCopy, src);
		dc->Flush();
	}

	return CPUCopy;
}

void copy(Resource* r, uint subresource, surface::mapped_subresource& dst, bool flip_vertically)
{
	intrusive_ptr<Resource> res(r);

	oCHECK_IS_TEXTURE(r);
	intrusive_ptr<Device> D3DDevice;
	r->GetDevice(&D3DDevice);
	intrusive_ptr<DeviceContext> dc;
	D3DDevice->GetImmediateContext(&dc);

	D3D_TEXTURE_DESC desc = get_texture_desc(r);
	if (desc.usage != D3D11_USAGE_STAGING)
		res = make_cpu_copy(r, true);

	D3D11_MAPPED_SUBRESOURCE mapped;
	oV(dc->Map(res, subresource, D3D11_MAP_READ, 0, &mapped));

	int2 ByteDimensions = surface::byte_dimensions(dxgi::to_surface_format(desc.format), int3(desc.dimensions.x, desc.dimensions.y, 1));
	memcpy2d(dst.data, dst.row_pitch, mapped.pData, mapped.RowPitch, ByteDimensions.x, ByteDimensions.y, flip_vertically);
	dc->Unmap(res, subresource);
}

std::shared_ptr<surface::buffer> make_snapshot(Texture2D* t)
{
	oCHECK_ARG(t, "invalid texture");
	intrusive_ptr<Resource> CPUResource = make_cpu_copy(t);

	D3D_TEXTURE_DESC desc = get_texture_desc(CPUResource);
	oCHECK_ARG(desc.format != DXGI_FORMAT_UNKNOWN, "The specified texture's format %s is not supported by surface::buffer", as_string(desc.format));

	surface::info si;
	si.format = surface::b8g8r8a8_unorm;
	si.dimensions = int3(desc.dimensions.x, desc.dimensions.y, 1);
	std::shared_ptr<surface::buffer> s = surface::buffer::make(si);

	surface::lock_guard lock(s);
	copy(CPUResource, 0, lock.mapped);

	return s;
}

void update_buffer(DeviceContext* dc, Buffer* b, uint byte_offset, uint num_bytes, const void* src)
{
	D3D11_BOX box;
	box.left = byte_offset;
	box.top = 0;
	box.front = 0;
	box.right = box.left + num_bytes;
	box.bottom = 1;
	box.back = 1;
	dc->UpdateSubresource(b, 0, &box, src, 0, 0);
}

void update_buffer(DeviceContext* dc, View* v, uint byte_offset, uint num_bytes, const void* src)
{
	intrusive_ptr<Resource> r;
	v->GetResource(&r);
	oASSERT(D3D11_RESOURCE_DIMENSION_BUFFER == get_type(v), "must call on a buffer");
	update_buffer(dc, (Buffer*)r.c_ptr(), byte_offset, num_bytes, src);
}

void update_texture(DeviceContext* dc, bool device_supports_deferred_contexts, Resource* r, uint subresource, const surface::const_mapped_subresource& src, const surface::box& region)
{
	D3D11_BOX box;
	D3D11_BOX* pBox = nullptr;
	if (!region.empty())
	{
		box.left = region.left;
		box.top = region.top;
		box.right = region.right;
		box.bottom = region.bottom; 
		box.front = region.front;
		box.back = region.back;
		pBox = &box;
	}

	uchar* pAdjustedData = (uchar*)src.data;

	if (pBox && !device_supports_deferred_contexts)
	{
		auto d = get_texture_desc(r);

		if (dxgi::is_block_compressed(d.format))
		{
			pBox->left /= 4;
			pBox->right /= 4;
			pBox->top /= 4;
			pBox->bottom /= 4;
		}

		pAdjustedData -= (pBox->front * src.depth_pitch) - (pBox->top * src.row_pitch) - (pBox->left * dxgi::get_size(d.format));
	}

	dc->UpdateSubresource(r, subresource, pBox, pAdjustedData, src.row_pitch, src.depth_pitch);
}

static UnorderedAccessView* s_NullUAVs[D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT];
static uint s_NoopInitialCounts[D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT] = { ~0u, ~0u, ~0u, ~0u, ~0u, ~0u, ~0u, ~0u, };
static_assert(oCOUNTOF(s_NoopInitialCounts) == D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT, "array mismatch");

void unset_all_draw_targets(DeviceContext* dc)
{
	dc->OMSetRenderTargetsAndUnorderedAccessViews(0, nullptr, nullptr, 0
		, D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT, s_NullUAVs, s_NoopInitialCounts);
}

void unset_all_dispatch_targets(DeviceContext* dc)
{
	dc->CSSetUnorderedAccessViews(0, D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT, s_NullUAVs, s_NoopInitialCounts);
}

// Return the size stored inside D3D11-era bytecode. This can be used anywhere bytecode size is expected.
uint bytecode_size(const void* bytecode)
{
	// Discovered empirically
	return bytecode ? ((const uint*)bytecode)[6] : 0;
}

}}}
