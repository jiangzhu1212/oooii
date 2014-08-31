// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.
#include <oGPU/depth_stencil_state.h>
#include <oCore/windows/win_util.h>
#include "d3d_debug.h"
#include "d3d_util.h"

using namespace ouro::gpu::d3d;

namespace ouro {

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
oDEFINE_TO_FROM_STRING(gpu::depth_stencil_state::value);

namespace gpu {

Device* get_device(device& dev);
DeviceContext* get_dc(command_list& cl);

void depth_stencil_state::initialize(const char* name, device& dev)
{
	deinitialize();

	static const D3D11_DEPTH_STENCIL_DESC sDepthStencils[] = 
	{
		{ FALSE, D3D11_DEPTH_WRITE_MASK_ALL, D3D11_COMPARISON_ALWAYS, FALSE, 0xFF, 0XFF, D3D11_STENCIL_OP_KEEP, D3D11_STENCIL_OP_KEEP, D3D11_STENCIL_OP_KEEP, D3D11_COMPARISON_ALWAYS, D3D11_STENCIL_OP_KEEP, D3D11_STENCIL_OP_KEEP, D3D11_STENCIL_OP_KEEP, D3D11_COMPARISON_ALWAYS },
		{ TRUE, D3D11_DEPTH_WRITE_MASK_ALL, D3D11_COMPARISON_LESS_EQUAL, FALSE, 0xFF, 0XFF, D3D11_STENCIL_OP_KEEP, D3D11_STENCIL_OP_KEEP, D3D11_STENCIL_OP_KEEP, D3D11_COMPARISON_ALWAYS, D3D11_STENCIL_OP_KEEP, D3D11_STENCIL_OP_KEEP, D3D11_STENCIL_OP_KEEP, D3D11_COMPARISON_ALWAYS },
		{ TRUE, D3D11_DEPTH_WRITE_MASK_ZERO, D3D11_COMPARISON_LESS_EQUAL, FALSE, 0xFF, 0XFF, D3D11_STENCIL_OP_KEEP, D3D11_STENCIL_OP_KEEP, D3D11_STENCIL_OP_KEEP, D3D11_COMPARISON_ALWAYS, D3D11_STENCIL_OP_KEEP, D3D11_STENCIL_OP_KEEP, D3D11_STENCIL_OP_KEEP, D3D11_COMPARISON_ALWAYS },
	};
	
	mstring n;
	Device* D3DDevice = get_device(dev);
	for (int i = 0; i < count; i++)
	{
		oV(D3DDevice->CreateDepthStencilState(&sDepthStencils[i], (DepthStencilState**)&states[i]));
		snprintf(n, "depth_stencil_state::%s", name, as_string(value(i)));
		debug_name((DepthStencilState*)states[i], n);
	}
}

void depth_stencil_state::deinitialize()
{
	for (auto& p : states)
	{
		oSAFE_RELEASEV(p);
	}
}

void depth_stencil_state::set(command_list& cl, const value& state, uint stencil_ref)
{
	get_dc(cl)->OMSetDepthStencilState((DepthStencilState*)states[state], stencil_ref);
}

}}
