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

intrusive_ptr<Texture1D> make_texture_1d(const char* name, Device* dev, surface::format format, uint width, uint array_size, bool mips);
intrusive_ptr<Texture2D> make_texture_2d(const char* name, Device* dev, surface::format format, uint width, uint height, uint array_size, uint bind_flags, bool mips, bool cube = false);
intrusive_ptr<Texture3D> make_texture_3d(const char* name, Device* dev, surface::format format, uint width, uint height, uint depth, bool mips);

// basic 1D 1DARRAY 2D 2DARRAY 3D CUBE CUBEARRAY types
intrusive_ptr<ShaderResourceView> make_srv(Resource* r, DXGI_FORMAT format, uint array_size);
intrusive_ptr<ShaderResourceView> make_srv(Resource* r, surface::format format, uint array_size);

intrusive_ptr<RenderTargetView> make_rtv(Resource* r, uint array_slice = invalid);
intrusive_ptr<DepthStencilView> make_dsv(Resource* r, uint array_slice = invalid);

bool is_array(ShaderResourceView* v);
bool is_array(RenderTargetView* v);
bool is_array(DepthStencilView* v);
bool is_array(UnorderedAccessView* v);

// common struct for information from various texture types (including buffers, Width is full byte width, ArraySize is num elements, so stride is Width / ArraySize)
struct D3D_TEXTURE_DESC
{
	D3D_TEXTURE_DESC()
		: Width(0)
		, Height(0)
		, Depth(0)
		, ArraySize(0)
		, Type(D3D11_RESOURCE_DIMENSION_UNKNOWN)
		, Format(DXGI_FORMAT_UNKNOWN)
		, Usage(D3D11_USAGE_DEFAULT)
		, CPUAccessFlags(0)
		, MiscFlags(0)
		, Mips(false)
	{ SampleDesc.Quality = 0; SampleDesc.Count = 1; }

	uint Width;
	uint Height;
	uint Depth;
	uint ArraySize;
	D3D11_RESOURCE_DIMENSION Type;
	DXGI_FORMAT Format;
	DXGI_SAMPLE_DESC SampleDesc;
	D3D11_USAGE Usage;
	uint BindFlags;
	uint CPUAccessFlags;
	uint MiscFlags;
	bool Mips;

	inline bool is_target() const { return !!(BindFlags & (D3D11_BIND_RENDER_TARGET|D3D11_BIND_DEPTH_STENCIL)); }
	inline bool is_color_target() const { return !!(BindFlags & D3D11_BIND_RENDER_TARGET); }
	inline bool is_depth_target() const { return !!(BindFlags & D3D11_BIND_DEPTH_STENCIL); }
	inline bool is_unordered() const { return !!(BindFlags & D3D11_BIND_UNORDERED_ACCESS); }
	inline bool is_cube() const { return !!(MiscFlags & D3D11_RESOURCE_MISC_TEXTURECUBE); }
};

D3D_TEXTURE_DESC get_texture_desc(Resource* r);

// uses oTRACE to display the fields of d
void trace_texture_desc(const D3D_TEXTURE_DESC& d, const char* prefix = "\t");

// sets to all pipeline stages
void set_cbs(DeviceContext* dc, uint slot, uint num_buffers, Buffer* const* buffers, uint gpu_stage_flags = ~0u);
void set_srvs(DeviceContext* dc, uint slot, uint num_srvs, ShaderResourceView* const* srvs, uint gpu_stage_flags = ~0u);
void set_samplers(DeviceContext* dc, uint slot, uint num_samplers, const SamplerState* const* samplers, uint gpu_stage_flags = ~0u);

intrusive_ptr<Buffer> make_buffer(const char* name, Device* dev, uint element_stride, uint num_elements, D3D11_USAGE usage, uint bind_flags, uint misc_flags, const void* init_data);

// fills out_srv and out_uav with views on a structured buffer
void make_structured(const char* name, Device* dev, uint struct_stride, uint num_structs, const void* src, uint uav_flags, ShaderResourceView** out_srv, UnorderedAccessView** out_uav);

// Creates a STAGING version of the specified resource and copies the src to it and flushes 
// the immediate context. If do_copy is false then the buffer is created uninitialized.
intrusive_ptr<Resource> make_cpu_copy(Resource* src, bool do_copy = true);

// Copies the contents of the specified texture to dst which is assumed to be 
// properly allocated to receive the contents. If flip_vertically is true then
// the bitmap data will be copied such that the destination will be upside-down 
// compared to the source.
void copy(ID3D11Resource* r, uint subresource, surface::mapped_subresource& dst, bool flip_vertically = false);

// blocks until async's data is available and can be read into dst. This must be
// called outside dc's begin/end block.
bool copy_async_data(DeviceContext* dc, Asynchronous* async, void* dst, uint dst_size, bool blocking = true);
template<typename T> bool copy_async_data(DeviceContext* dc, Asynchronous* async, T* data, bool blocking = true) { return copy_async_data(dc, async, data, sizeof(T), blocking); }

// copies the contents of t to a new surface buffer
surface::buffer make_snapshot(Texture2D* t);

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

// Implementations of scanning the specified buffers to see if they're bound to
// certain quiet/noop-y behaviors when using compute. IMHO certain DX warnings
// should be errors and certain silences should make noise. These API are 
// exposed here as much a documentation/callout to the user as they are part of
// implementing a robust checking system in the cross-platform API. Basically
// D3D11 will promote a read-only binding to read-write, but will not all a 
// read-write binding to be replaced/concurrent with a read-only binding.
void check_bound_rts_and_uavs(DeviceContext* dc, uint num_buffers, const Buffer* const* buffers);

// check that none of the specified buffers are bound to CS UAVs.
void check_bound_cs_uavs(DeviceContext* dc, uint num_buffers, const Buffer* const* buffers);

// return the size of the specified hlsl shader bytecode
uint bytecode_size(const void* bytecode);

}}}

#endif
