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
#include <oGPU/state.h>
#include <oCore/windows/win_util.h>
#include "d3d_debug.h"
#include "d3d_util.h"

#define STR_SUPPORT(_T, _NumTs) \
	oDEFINE_TO_STRING(_T) \
	oDEFINE_FROM_STRING(_T, _NumTs)

using namespace ouro::gpu::d3d;

namespace ouro {

const char* as_string(const gpu::sampler_state::value& _Value)
{
	switch (_Value)
	{
		case gpu::sampler_state::point_clamp: return "point_clamp"; 
		case gpu::sampler_state::point_wrap: return "point_wrap";
		case gpu::sampler_state::linear_clamp: return "linear_clamp";
		case gpu::sampler_state::linear_wrap: return "linear_wrap";
		case gpu::sampler_state::aniso_clamp: return "aniso_clamp";
		case gpu::sampler_state::aniso_wrap: return "aniso_wrap";
		default: break;
	}
	return "?";
}
STR_SUPPORT(gpu::sampler_state::value, gpu::sampler_state::count);

const char* as_string(const gpu::rasterizer_state::value& _Value)
{
	switch (_Value)
	{
		case gpu::rasterizer_state::front_face: return "front_face";
		case gpu::rasterizer_state::back_face: return "back_face"; 
		case gpu::rasterizer_state::two_sided: return "two_sided";
		case gpu::rasterizer_state::front_wireframe: return "front_wireframe";
		case gpu::rasterizer_state::back_wireframe: return "back_wireframe"; 
		case gpu::rasterizer_state::two_sided_wireframe: return "two_sided_wireframe";
		default: break;
	}
	return "?";
}
STR_SUPPORT(gpu::rasterizer_state::value, gpu::rasterizer_state::count);

const char* as_string(const gpu::depth_stencil_state::value& _Value)
{
	switch (_Value)
	{
		case gpu::depth_stencil_state::none: return "none";
		case gpu::depth_stencil_state::test_and_write: return "test_and_write";
		case gpu::depth_stencil_state::test: return "test";
		default: break;
	}
	return "?";
}
STR_SUPPORT(gpu::depth_stencil_state::value, gpu::depth_stencil_state::count);

const char* as_string(const gpu::blend_state::value& _Value)
{
	switch (_Value)
	{
		case gpu::blend_state::opaque: return "opaque";
		case gpu::blend_state::alpha_test: return "alpha_test";
		case gpu::blend_state::accumulate: return "accumulate";
		case gpu::blend_state::additive: return "additive";
		case gpu::blend_state::multiply: return "multiply";
		case gpu::blend_state::screen: return "screen";
		case gpu::blend_state::translucent: return "translucent";
		case gpu::blend_state::min_: return "min";
		case gpu::blend_state::max_: return "max";
		default: break;
	}
	return "?";
}
STR_SUPPORT(gpu::blend_state::value, gpu::blend_state::count);

namespace gpu {

Device* get_device(device* dev);
DeviceContext* get_dc(command_list* cl);

void sampler_states::initialize(device* dev)
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
	CD3D11_SAMPLER_DESC desc(D3D11_DEFAULT);

	Device* D3DDevice = get_device(dev);
	for (int i = 0; i < states.size(); i++)
	{
		desc.Filter = sFilters[i];
		desc.AddressU = desc.AddressV = desc.AddressW = sAddresses[i];
		oV(D3DDevice->CreateSamplerState(&desc, (SamplerState**)&states[i]));
		snprintf(name, "sampler::%s", as_string(sampler_state::value(i)));
		debug_name((SamplerState*)states[i], name);
	}
}

void sampler_states::deinitialize()
{
	for (auto p : states)
		if (p)
			((DeviceChild*)p)->Release();
	states.fill(nullptr);
}

void sampler_states::set(command_list* cl, uint slot, uint num_samplers, const sampler_state::value* samplers)
{
	SamplerState* States[max_num_samplers];
	for (uint i = 0; i < num_samplers; i++)
		States[i] = (SamplerState*)states[samplers[i]];
	set_samplers(get_dc(cl), slot, num_samplers, States);
}

void rasterizer_states::initialize(device* dev)
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
  desc.DepthBias = D3D11_DEFAULT_DEPTH_BIAS;
  desc.DepthBiasClamp = D3D11_DEFAULT_DEPTH_BIAS_CLAMP;
  desc.SlopeScaledDepthBias = D3D11_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
  desc.DepthClipEnable = true;
  desc.ScissorEnable = false;
  desc.MultisampleEnable = false;
  desc.AntialiasedLineEnable = false;

	Device* D3DDevice = get_device(dev);
	for (int i = 0; i < rasterizer_state::count; i++)
	{
		desc.FillMode = sFills[i];
		desc.CullMode = sCulls[i];
		oV(D3DDevice->CreateRasterizerState(&desc, (RasterizerState**)&states[i]));
		snprintf(name, "rasterizer_state::%s", as_string(rasterizer_state::value(i)));
		debug_name((RasterizerState*)states[i], name);
	}
}

void rasterizer_states::deinitialize()
{
	for (auto p : states)
		if (p)
			((DeviceChild*)p)->Release();
	states.fill(nullptr);
}

void rasterizer_states::set(command_list* cl, const rasterizer_state::value& state)
{
	get_dc(cl)->RSSetState((RasterizerState*)states[state]);
}

void blend_states::initialize(device* dev)
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

	Device* D3DDevice = get_device(dev);
	for (int i = 0; i < blend_state::count; i++)
	{
		for (auto& bs : desc.RenderTarget)
			bs = sBlends[i];
		oV(D3DDevice->CreateBlendState(&desc, (BlendState**)&states[i]));
		snprintf(name, "blend_state::%s", as_string(blend_state::value(i)));
		debug_name((BlendState*)states[i], name);
	}
}

void blend_states::deinitialize()
{
	for (auto p : states)
		if (p)
			((DeviceChild*)p)->Release();
	states.fill(nullptr);
}

void blend_states::set(command_list* cl, const blend_state::value& state)
{
	static const float default_blend_factor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	get_dc(cl)->OMSetBlendState((BlendState*)states[state], default_blend_factor, 0xffffffff);
}

void blend_states::set(command_list* cl, const blend_state::value& state, const float blend_factor[4], uint sample_mask)
{
	get_dc(cl)->OMSetBlendState((BlendState*)states[state], blend_factor, sample_mask);
}

void depth_stencil_states::initialize(device* dev)
{
	deinitialize();

	static const D3D11_DEPTH_STENCIL_DESC sDepthStencils[] = 
	{
		{ FALSE, D3D11_DEPTH_WRITE_MASK_ALL, D3D11_COMPARISON_ALWAYS, FALSE, 0xFF, 0XFF, D3D11_STENCIL_OP_KEEP, D3D11_STENCIL_OP_KEEP, D3D11_STENCIL_OP_KEEP, D3D11_COMPARISON_ALWAYS, D3D11_STENCIL_OP_KEEP, D3D11_STENCIL_OP_KEEP, D3D11_STENCIL_OP_KEEP, D3D11_COMPARISON_ALWAYS },
		{ TRUE, D3D11_DEPTH_WRITE_MASK_ALL, D3D11_COMPARISON_LESS_EQUAL, FALSE, 0xFF, 0XFF, D3D11_STENCIL_OP_KEEP, D3D11_STENCIL_OP_KEEP, D3D11_STENCIL_OP_KEEP, D3D11_COMPARISON_ALWAYS, D3D11_STENCIL_OP_KEEP, D3D11_STENCIL_OP_KEEP, D3D11_STENCIL_OP_KEEP, D3D11_COMPARISON_ALWAYS },
		{ TRUE, D3D11_DEPTH_WRITE_MASK_ZERO, D3D11_COMPARISON_LESS_EQUAL, FALSE, 0xFF, 0XFF, D3D11_STENCIL_OP_KEEP, D3D11_STENCIL_OP_KEEP, D3D11_STENCIL_OP_KEEP, D3D11_COMPARISON_ALWAYS, D3D11_STENCIL_OP_KEEP, D3D11_STENCIL_OP_KEEP, D3D11_STENCIL_OP_KEEP, D3D11_COMPARISON_ALWAYS },
	};
	
	mstring name;
	Device* D3DDevice = get_device(dev);
	for (int i = 0; i < depth_stencil_state::count; i++)
	{
		oV(D3DDevice->CreateDepthStencilState(&sDepthStencils[i], (DepthStencilState**)&states[i]));
		snprintf(name, "depth_stencil_state::%s", as_string(depth_stencil_state::value(i)));
		debug_name((DepthStencilState*)states[i], name);
	}
}

void depth_stencil_states::deinitialize()
{
	for (auto p : states)
		if (p)
			((DeviceChild*)p)->Release();
	states.fill(nullptr);
}

void depth_stencil_states::set(command_list* cl, const depth_stencil_state::value& state, uint stencil_ref)
{
	get_dc(cl)->OMSetDepthStencilState((DepthStencilState*)states[state], stencil_ref);
}

}}
