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

Texture1D* CreateTexture1D(const char* name, Device* dev, surface::format format, uint width, uint array_size, bool mips)
{
	CD3D11_TEXTURE1D_DESC d(dxgi::from_surface_format(format)
		, width
		, array_size
		, mips ? 0 : 1);

	Texture1D* t = nullptr;
	oV(dev->CreateTexture1D(&d, nullptr, &t));
	debug_name(t, name);
	return t;
}

Texture2D* CreateTexture2D(const char* name, Device* dev, surface::format format, uint width, uint height, uint array_size, bool mips, bool cube, bool target)
{
	CD3D11_TEXTURE2D_DESC d(dxgi::from_surface_format(format)
		, width
		, height
		, cube ? __min(6, array_size*6) : array_size
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

Texture3D* CreateTexture3D(const char* name, Device* dev, surface::format format, uint width, uint height, uint depth, bool mips)
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

ShaderResourceView* CreateSRV(Resource* r, DXGI_FORMAT format, uint array_size)
{
	D3D11_RESOURCE_DIMENSION type = D3D11_RESOURCE_DIMENSION_UNKNOWN;
	r->GetType(&type);

	D3D11_SHADER_RESOURCE_VIEW_DESC d;
	d.Format = format;

	switch (type)
	{
		case D3D11_RESOURCE_DIMENSION_TEXTURE2D:
		{
			D3D11_TEXTURE2D_DESC desc;
			((Texture2D*)r)->GetDesc(&desc);
			d.ViewDimension = (desc.MiscFlags & D3D11_RESOURCE_MISC_TEXTURECUBE) ? D3D_SRV_DIMENSION_TEXTURE2D : D3D_SRV_DIMENSION_TEXTURECUBE;
			break;
		}
		
		case D3D11_RESOURCE_DIMENSION_TEXTURE1D: d.ViewDimension = D3D_SRV_DIMENSION_TEXTURE1D; break;
		case D3D11_RESOURCE_DIMENSION_TEXTURE3D: d.ViewDimension = D3D_SRV_DIMENSION_TEXTURE3D; array_size = 0; break;
		oNODEFAULT;
	}

	if (array_size)
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

ShaderResourceView* CreateSRV(Resource* r, surface::format format, uint array_size)
{
	return CreateSRV(r, dxgi::from_surface_format(format), array_size);
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

Buffer* make_buffer(const char* name, Device* dev, uint num_elements, uint element_stride, uint bind_flags, uint misc_flags, const void* init_data)
{
	D3D11_BUFFER_DESC d = {0};
	d.ByteWidth = num_elements * element_stride;
	d.Usage = D3D11_USAGE_DEFAULT;
	d.BindFlags = bind_flags;
	d.CPUAccessFlags = 0;
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

template<typename ResourceDescT>
static void set_staging(ResourceDescT* desc)
{
	desc->Usage = D3D11_USAGE_STAGING;
	desc->CPUAccessFlags = D3D11_CPU_ACCESS_READ;
	desc->BindFlags = 0;
}

Resource* make_cpu_copy(Resource* src)
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

	dc->CopyResource(CPUCopy, src);
	dc->Flush();
	return CPUCopy;
}

void update_buffer(DeviceContext* dc, Resource* r, uint byte_offset, uint num_bytes, const void* src)
{
	D3D11_BOX box;
	box.left = byte_offset;
	box.top = 0;
	box.front = 0;
	box.right = box.left + num_bytes;
	box.bottom = 1;
	box.back = 1;
	dc->UpdateSubresource(r, 0, &box, src, 0, 0);
}

void update_buffer(DeviceContext* dc, View* v, uint byte_offset, uint num_bytes, const void* src)
{
	intrusive_ptr<Resource> r;
	v->GetResource(&r);
	update_buffer(dc, r, byte_offset, num_bytes, src);
}

// Return the size stored inside D3D11-era bytecode. This can be used anywhere bytecode size is expected.
uint bytecode_size(const void* bytecode)
{
	// Discovered empirically
	return bytecode ? ((const uint*)bytecode)[6] : 0;
}

}}}
