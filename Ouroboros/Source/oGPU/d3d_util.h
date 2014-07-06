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
#pragma once
#ifndef oGPU_d3d_util_h
#define oGPU_d3d_util_h

#include <oBase/gpu_api.h>
#include <oBase/intrusive_ptr.h>
#include <oSurface/surface.h>
#include "d3d_types.h"
#include <d3d11.h>

namespace ouro { namespace gpu { namespace d3d {

Texture1D* make_texture_1d(const char* name, Device* dev, surface::format format, uint width, uint array_size, bool mips);
Texture2D* make_texture_2d(const char* name, Device* dev, surface::format format, uint width, uint height, uint array_size, bool mips, bool cube = false, bool target = false);
Texture3D* make_texture_3d(const char* name, Device* dev, surface::format format, uint width, uint height, uint depth, bool mips);

// basic 1D 1DARRAY 2D 2DARRAY 3D CUBE CUBEARRAY types
ShaderResourceView* make_srv(Resource* r, DXGI_FORMAT format, uint array_size);
ShaderResourceView* make_srv(Resource* r, surface::format format, uint array_size);

// common struct for information from various texture types (including buffers, dimensions are (element_stride, num_elements, 1))
struct texture_info
{
	texture_info()
		: dimensions(0,0,0)
		, array_size(0)
		, type(D3D11_RESOURCE_DIMENSION_UNKNOWN)
		, format(DXGI_FORMAT_UNKNOWN)
		, usage(D3D11_USAGE_DEFAULT)
	{}

	ushort3 dimensions;
	ushort array_size;
	D3D11_RESOURCE_DIMENSION type;
	DXGI_FORMAT format;
	D3D11_USAGE usage;
};

texture_info get_texture_info(Resource* r);
	
// sets to all pipeline stages
void set_cbs(DeviceContext* dc, uint slot, uint num_buffers, Buffer* const* buffers, uint gpu_stage_flags = ~0u);
void set_srvs(DeviceContext* dc, uint slot, uint num_srvs, ShaderResourceView* const* srvs, uint gpu_stage_flags = ~0u);
void set_samplers(DeviceContext* dc, uint slot, uint num_samplers, const SamplerState* const* samplers, uint gpu_stage_flags = ~0u);

Buffer* make_buffer(const char* name, Device* dev, uint element_stride, uint num_elements, D3D11_USAGE usage, uint bind_flags, uint misc_flags, const void* init_data);

// fills out_srv and out_uav with views on a structured buffer
void make_structured(const char* name, Device* dev, uint struct_stride, uint num_structs, const void* src, uint uav_flags, ShaderResourceView** out_srv, UnorderedAccessView** out_uav);

// Creates a STAGING version of the specified resource and copies the src to it and flushes 
// the immediate context. If do_copy is false then the buffer is created uninitialized.
Resource* make_cpu_copy(Resource* src, bool do_copy = true);

// calls UpdateSubresource for the specified buffer. This won't work for a D3D11_BIND_CONSTANT_BUFFER.
void update_buffer(DeviceContext* dc, Buffer* b, uint byte_offset, uint num_bytes, const void* src);
void update_buffer(DeviceContext* dc, View* v, uint byte_offset, uint num_bytes, const void* src);

// calls UpdateSubresource for the specified buffer.
void update_texture(DeviceContext* dc, bool device_supports_deferred_contexts, Resource* r, uint subresource, const surface::const_mapped_subresource& src, const surface::box& region = surface::box());
inline void update_texture(DeviceContext* dc, bool device_supports_deferred_contexts, Resource* r, uint subresource, const surface::mapped_subresource& src, const surface::box& region = surface::box()) { update_texture(dc, device_supports_deferred_contexts, r, subresource, (const surface::const_mapped_subresource&)src, region); }

inline D3D11_RESOURCE_DIMENSION get_type(Resource* r) { D3D11_RESOURCE_DIMENSION type; r->GetType(&type); return type; }
inline D3D11_RESOURCE_DIMENSION get_type(View* v) { Resource* r = nullptr; v->GetResource(&r); D3D11_RESOURCE_DIMENSION type = get_type(r); r->Release(); return type; }

// clears render targets and unordered access views from dc
void unset_all_draw_targets(DeviceContext* dc);
void unset_all_dispatch_targets(DeviceContext* dc);

// return the size of the specified hlsl shader bytecode
uint bytecode_size(const void* bytecode);

}}}

#endif
