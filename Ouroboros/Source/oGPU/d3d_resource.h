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
#ifndef oGPU_d3d_resource_h
#define oGPU_d3d_resource_h

#include <oGPU/resource.h>
#include "d3d_types.h"
#include <d3d11.h>

namespace ouro {
	namespace gpu {
		namespace d3d {

enum oD3D_VIEW_DIMENSION
{
	oD3D_VIEW_DIMENSION_UNKNOWN,
	oD3D_VIEW_DIMENSION_SHADER_RESOURCE,
	oD3D_VIEW_DIMENSION_RENDER_TARGET,
	oD3D_VIEW_DIMENSION_DEPTH_STENCIL,
	oD3D_VIEW_DIMENSION_UNORDERED_ACCESS,
	oD3D_VIEW_DIMENSION_CPU_ACCESS,
};

struct oD3D_VIEW_DESC
{
	oD3D_VIEW_DESC() : Type(oD3D_VIEW_DIMENSION_UNKNOWN) {}
	oD3D_VIEW_DIMENSION Type;
	union
	{
		D3D11_UNORDERED_ACCESS_VIEW_DESC UAV;
		D3D11_DEPTH_STENCIL_VIEW_DESC DSV;
		D3D11_RENDER_TARGET_VIEW_DESC RTV;
		D3D11_SHADER_RESOURCE_VIEW_DESC SRV;
		D3D11_SHADER_RESOURCE_VIEW_DESC CAV;
	};
};

struct oD3D_RESOURCE_DESC
{
	oD3D_RESOURCE_DESC() : Type(D3D11_RESOURCE_DIMENSION_UNKNOWN) {}
	D3D11_RESOURCE_DIMENSION Type;
	union
	{
		D3D11_BUFFER_DESC BufferDesc;
		D3D11_TEXTURE1D_DESC Texture1DDesc;
		D3D11_TEXTURE2D_DESC Texture2DDesc;
		D3D11_TEXTURE3D_DESC Texture3DDesc;
	};
};

struct oD3D_VIEW_ONLY_DESC
{
	// this contains information that conceptually would seem like it is from a resource
	// but is only available on the view

	oD3D_VIEW_ONLY_DESC() : struct_byte_size(0), uav_flags(0), format(surface::unknown), is_array(false) {}

	uint struct_byte_size;
	uint uav_flags;
	surface::format format;
	bool is_array;
};

oD3D_VIEW_DIMENSION get_type(View* v);
oD3D_VIEW_DESC get_desc(View* v);
oD3D_RESOURCE_DESC get_desc(Resource* r);
oD3D_VIEW_DESC to_view_desc(const resource_info& info);

// not all info is retained by the view desc itself, so returns what is available
// because these parameters are not determinable from a resource alone.
oD3D_VIEW_ONLY_DESC from_view_desc(const oD3D_VIEW_DESC& desc);

oD3D_RESOURCE_DESC to_resource_desc(const resource_info& info);
resource_info from_resource_desc(const oD3D_RESOURCE_DESC& desc, const oD3D_VIEW_ONLY_DESC& view_only_desc, D3D11_USAGE* out_usage = nullptr);

intrusive_ptr<Resource> make_resource(Device* dev, const oD3D_RESOURCE_DESC& desc, const char* debug_name = "", const D3D11_SUBRESOURCE_DATA* init_data = nullptr);
intrusive_ptr<View> make_view(Resource* r, const oD3D_VIEW_DESC& desc, const char* debug_name = "");

resource_info get_info(View* v);

// Copies the contents of the specified resource to the mapped subresource which 
// is assumed to be properly allocated to receive the contents. If flip_vertical 
// is true then the bitmap data will be copied such that the destination will be 
// upside-down relative to the source.
void copy(Resource* r, uint subresource, surface::mapped_subresource* dst_subresource, bool flip_vertically = false);

// Creates a CPU-readable copy of the specified resource immediately with a flush.
intrusive_ptr<Resource> make_cpu_copy(Resource* r);

// Creates a copy of the specified UAV that clears any raw/append/counter flags
intrusive_ptr<UnorderedAccessView> make_unflagged_copy(UnorderedAccessView* v);

// Copies the specified texture to a more image-like and workable format
std::shared_ptr<surface::buffer> make_snapshot(Texture2D* t);

// Uses oTRACE to display the fields of the specified desc.
void trace_desc(const D3D11_TEXTURE2D_DESC& desc, const char* prefix);

		} // namespace d3d
	} // namespace gpu
} // namespace ouro

#endif
