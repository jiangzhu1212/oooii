// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.
#include <oGPU/blend_state.h>
#include <oCore/windows/win_util.h>
#include "d3d_debug.h"
#include "d3d_util.h"

using namespace ouro::gpu::d3d;

namespace ouro {

const char* as_string(const gpu::blend_state::value& state)
{
	switch (state)
	{
		case gpu::blend_state::opaque: return "opaque";
		case gpu::blend_state::alpha_test: return "alpha_test";
		case gpu::blend_state::accumulate: return "accumulate";
		case gpu::blend_state::additive: return "additive";
		case gpu::blend_state::multiply: return "multiply";
		case gpu::blend_state::screen: return "screen";
		case gpu::blend_state::translucent: return "translucent";
		case gpu::blend_state::minimum: return "minimum";
		case gpu::blend_state::maximum: return "maximum";
		default: break;
	}
	return "?";
}
oDEFINE_TO_FROM_STRING(gpu::blend_state::value);

namespace gpu {

Device* get_device(device& dev);
DeviceContext* get_dc(command_list& cl);

void blend_state::initialize(const char* name, device& dev)
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
	static_assert(oCOUNTOF(sBlends) == count, "array mismatch");

	mstring n;
	D3D11_BLEND_DESC desc;
	desc.AlphaToCoverageEnable = FALSE;
	desc.IndependentBlendEnable = FALSE;

	Device* D3DDevice = get_device(dev);
	for (int i = 0; i < count; i++)
	{
		for (auto& bs : desc.RenderTarget)
			bs = sBlends[i];
		oV(D3DDevice->CreateBlendState(&desc, (BlendState**)&states[i]));
		snprintf(n, "%s::%s", name, as_string(blend_state::value(i)));
		debug_name((BlendState*)states[i], n);
	}
}

void blend_state::deinitialize()
{
	for (auto& p : states)
	{
		oSAFE_RELEASEV(p);
	}
}

void blend_state::set(command_list& cl, const blend_state::value& state)
{
	static const float default_blend_factor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	get_dc(cl)->OMSetBlendState((BlendState*)states[state], default_blend_factor, 0xffffffff);
}

void blend_state::set(command_list& cl, const blend_state::value& state, const float blend_factor[4], uint sample_mask)
{
	get_dc(cl)->OMSetBlendState((BlendState*)states[state], blend_factor, sample_mask);
}

}}
