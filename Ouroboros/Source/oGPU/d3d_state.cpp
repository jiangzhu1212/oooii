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
#include "d3d_state.h"
#include "d3d_debug.h"
#include <oBase/macros.h>
#include <oCore/windows/win_util.h>

namespace ouro {
	namespace gpu {
		namespace d3d {

void sampler_state_registry::initialize(Device* dev)
{
	deinitialize();

	static const D3D11_FILTER sFilters[] = 
	{
		D3D11_FILTER_MIN_MAG_MIP_POINT,
		D3D11_FILTER_MIN_MAG_MIP_POINT,
		D3D11_FILTER_MIN_MAG_MIP_LINEAR,
		D3D11_FILTER_MIN_MAG_MIP_LINEAR,
		D3D11_FILTER_COMPARISON_ANISOTROPIC,
		D3D11_FILTER_COMPARISON_ANISOTROPIC,
	};
	static_assert(oCOUNTOF(sFilters) == sampler_state::count, "array mismatch");

	static const D3D11_TEXTURE_ADDRESS_MODE sAddresses[] = 
	{
		D3D11_TEXTURE_ADDRESS_CLAMP,
		D3D11_TEXTURE_ADDRESS_WRAP,
		D3D11_TEXTURE_ADDRESS_CLAMP,
		D3D11_TEXTURE_ADDRESS_WRAP,
		D3D11_TEXTURE_ADDRESS_CLAMP,
		D3D11_TEXTURE_ADDRESS_WRAP,
	};
	static_assert(oCOUNTOF(sAddresses) == sampler_state::count, "array mismatch");

	mstring name;
	D3D11_SAMPLER_DESC desc;

	for (int i = 0; i < sampler_state::count; i++)
	{
		desc.Filter = sFilters[i];
		desc.AddressU = desc.AddressV = desc.AddressW = sAddresses[i];
		desc.MipLODBias = 0.0f;
		desc.MaxAnisotropy = 16;
		desc.ComparisonFunc = D3D11_COMPARISON_NEVER;
		(float4&)desc.BorderColor = float4(0.0f, 0.0f, 0.0f, 0.0f);
		desc.MinLOD = 0.0f;
		desc.MaxLOD = FLT_MAX;
		oV(dev->CreateSamplerState(&desc, &states[i]));
		snprintf(name, "sampler::%s", as_string(sampler_state::value(i)));
		debug_name(states[i], name);
	}
}

void rasterizer_state_registry::initialize(Device* dev)
{
	deinitialize();

	static const D3D11_FILL_MODE sFills[] = 
	{
		D3D11_FILL_SOLID,
		D3D11_FILL_SOLID,
		D3D11_FILL_SOLID,
		D3D11_FILL_WIREFRAME,
		D3D11_FILL_WIREFRAME,
		D3D11_FILL_WIREFRAME,
	};
	static_assert(oCOUNTOF(sFills) == rasterizer_state::count, "array mismatch");

	static const D3D11_CULL_MODE sCulls[] = 
	{
		D3D11_CULL_BACK,
		D3D11_CULL_FRONT,
		D3D11_CULL_NONE,
		D3D11_CULL_BACK,
		D3D11_CULL_FRONT,
		D3D11_CULL_NONE,
	};
	static_assert(oCOUNTOF(sCulls) == rasterizer_state::count, "array mismatch");

	mstring name;
	D3D11_RASTERIZER_DESC desc;
  desc.FrontCounterClockwise = false;
  desc.DepthBias = 0;
  desc.DepthBiasClamp = 0.0f;
  desc.SlopeScaledDepthBias = 0.0f;
  desc.DepthClipEnable = true;
  desc.ScissorEnable = false;
  desc.MultisampleEnable = false;
  desc.AntialiasedLineEnable = false;

	for (int i = 0; i < rasterizer_state::count; i++)
	{
		desc.FillMode = sFills[i];
		desc.CullMode = sCulls[i];
		oV(dev->CreateRasterizerState(&desc, &states[i]));
		snprintf(name, "rasterizer_state::%s", as_string(rasterizer_state::value(i)));
		debug_name(states[i], name);
	}
}

void blend_state_registry::initialize(Device* dev)
{
	deinitialize();

	static const D3D11_RENDER_TARGET_BLEND_DESC sBlends[] =
	{
		{ FALSE, D3D11_BLEND_ONE, D3D11_BLEND_ZERO, D3D11_BLEND_OP_ADD, D3D11_BLEND_ONE, D3D11_BLEND_ZERO, D3D11_BLEND_OP_ADD, D3D11_COLOR_WRITE_ENABLE_ALL },
		{ FALSE, D3D11_BLEND_ONE, D3D11_BLEND_ZERO, D3D11_BLEND_OP_ADD, D3D11_BLEND_ONE, D3D11_BLEND_ZERO, D3D11_BLEND_OP_ADD, D3D11_COLOR_WRITE_ENABLE_ALL },
		{ TRUE, D3D11_BLEND_ONE, D3D11_BLEND_ONE, D3D11_BLEND_OP_ADD, D3D11_BLEND_ONE, D3D11_BLEND_ONE, D3D11_BLEND_OP_ADD, D3D11_COLOR_WRITE_ENABLE_ALL },
		{ TRUE, D3D11_BLEND_SRC_ALPHA, D3D11_BLEND_ONE, D3D11_BLEND_OP_ADD, D3D11_BLEND_ZERO, D3D11_BLEND_ZERO, D3D11_BLEND_OP_ADD, D3D11_COLOR_WRITE_ENABLE_ALL },
		{ TRUE, D3D11_BLEND_DEST_COLOR, D3D11_BLEND_ZERO, D3D11_BLEND_OP_ADD, D3D11_BLEND_ZERO, D3D11_BLEND_ZERO, D3D11_BLEND_OP_ADD, D3D11_COLOR_WRITE_ENABLE_ALL },
		{ TRUE, D3D11_BLEND_INV_DEST_COLOR, D3D11_BLEND_ONE, D3D11_BLEND_OP_ADD, D3D11_BLEND_ZERO, D3D11_BLEND_ZERO, D3D11_BLEND_OP_ADD, D3D11_COLOR_WRITE_ENABLE_ALL },
		{ TRUE, D3D11_BLEND_SRC_ALPHA, D3D11_BLEND_INV_SRC_ALPHA, D3D11_BLEND_OP_ADD, D3D11_BLEND_ZERO, D3D11_BLEND_ZERO, D3D11_BLEND_OP_ADD, D3D11_COLOR_WRITE_ENABLE_ALL },
		{ TRUE, D3D11_BLEND_ONE, D3D11_BLEND_ONE, D3D11_BLEND_OP_MIN, D3D11_BLEND_ONE, D3D11_BLEND_ONE, D3D11_BLEND_OP_MIN, D3D11_COLOR_WRITE_ENABLE_ALL },
		{ TRUE, D3D11_BLEND_ONE, D3D11_BLEND_ONE, D3D11_BLEND_OP_MAX, D3D11_BLEND_ONE, D3D11_BLEND_ONE, D3D11_BLEND_OP_MAX, D3D11_COLOR_WRITE_ENABLE_ALL },
	};
	static_assert(oCOUNTOF(sBlends) == blend_state::count, "array mismatch");

	mstring name;
	D3D11_BLEND_DESC desc;
	desc.AlphaToCoverageEnable = FALSE;
	desc.IndependentBlendEnable = FALSE;

	for (int i = 0; i < blend_state::count; i++)
	{
		for (auto& bs : desc.RenderTarget)
			bs = sBlends[i];
		oV(dev->CreateBlendState(&desc, &states[i]));
		snprintf(name, "blend_state::%s", as_string(blend_state::value(i)));
		debug_name(states[i], name);
	}
}

void depth_stencil_state_registry::initialize(Device* dev)
{
	deinitialize();

	static const D3D11_DEPTH_STENCIL_DESC sDepthStencils[] = 
	{
		{ FALSE, D3D11_DEPTH_WRITE_MASK_ALL, D3D11_COMPARISON_ALWAYS, FALSE, 0xFF, 0XFF, D3D11_STENCIL_OP_KEEP, D3D11_STENCIL_OP_KEEP, D3D11_STENCIL_OP_KEEP, D3D11_COMPARISON_ALWAYS, D3D11_STENCIL_OP_KEEP, D3D11_STENCIL_OP_KEEP, D3D11_STENCIL_OP_KEEP, D3D11_COMPARISON_ALWAYS },
		{ TRUE, D3D11_DEPTH_WRITE_MASK_ALL, D3D11_COMPARISON_LESS_EQUAL, FALSE, 0xFF, 0XFF, D3D11_STENCIL_OP_KEEP, D3D11_STENCIL_OP_KEEP, D3D11_STENCIL_OP_KEEP, D3D11_COMPARISON_ALWAYS, D3D11_STENCIL_OP_KEEP, D3D11_STENCIL_OP_KEEP, D3D11_STENCIL_OP_KEEP, D3D11_COMPARISON_ALWAYS },
		{ TRUE, D3D11_DEPTH_WRITE_MASK_ZERO, D3D11_COMPARISON_LESS_EQUAL, FALSE, 0xFF, 0XFF, D3D11_STENCIL_OP_KEEP, D3D11_STENCIL_OP_KEEP, D3D11_STENCIL_OP_KEEP, D3D11_COMPARISON_ALWAYS, D3D11_STENCIL_OP_KEEP, D3D11_STENCIL_OP_KEEP, D3D11_STENCIL_OP_KEEP, D3D11_COMPARISON_ALWAYS },
	};
	
	mstring name;
	for (int i = 0; i < depth_stencil_state::count; i++)
	{
		oV(dev->CreateDepthStencilState(&sDepthStencils[i], &states[i]));
		snprintf(name, "depth_stencil_state::%s", as_string(depth_stencil_state::value(i)));
		debug_name(states[i], name);
	}
}

		} // namespace d3d
	} // namespace gpu
} // namespace ouro
