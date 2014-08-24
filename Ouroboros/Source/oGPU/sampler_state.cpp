/**************************************************************************
 * The MIT License                                                        *
 * Copyright (c) 2014 Antony Arciuolo.                                    *
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
#include <oGPU/sampler_state.h>
#include <oCore/windows/win_util.h>
#include "d3d_debug.h"
#include "d3d_util.h"

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
oDEFINE_TO_FROM_STRING(gpu::sampler_state::value, gpu::sampler_state::count);

namespace gpu {

Device* get_device(device& dev);
DeviceContext* get_dc(command_list& cl);

void sampler_state::initialize(const char* name, device& dev)
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
	static_assert(oCOUNTOF(sFilters) == count, "array mismatch");

	static const D3D11_TEXTURE_ADDRESS_MODE sAddresses[] = 
	{
		D3D11_TEXTURE_ADDRESS_CLAMP,
		D3D11_TEXTURE_ADDRESS_WRAP,
		D3D11_TEXTURE_ADDRESS_CLAMP,
		D3D11_TEXTURE_ADDRESS_WRAP,
		D3D11_TEXTURE_ADDRESS_CLAMP,
		D3D11_TEXTURE_ADDRESS_WRAP,
	};
	static_assert(oCOUNTOF(sAddresses) == count, "array mismatch");

	mstring n;
	CD3D11_SAMPLER_DESC desc(D3D11_DEFAULT);

	Device* D3DDevice = get_device(dev);
	for (size_t i = 0; i < states.size(); i++)
	{
		desc.Filter = sFilters[i];
		desc.AddressU = desc.AddressV = desc.AddressW = sAddresses[i];
		oV(D3DDevice->CreateSamplerState(&desc, (SamplerState**)&states[i]));
		snprintf(n, "%s::%s", name, as_string(value(i)));
		debug_name((SamplerState*)states[i], n);
	}
}

void sampler_state::deinitialize()
{
	for (auto& p : states)
	{
		oSAFE_RELEASEV(p);
	}
}

void sampler_state::set(command_list& cl, uint slot, uint num_samplers, const sampler_state::value* samplers)
{
	SamplerState* States[max_num_samplers];
	for (uint i = 0; i < num_samplers; i++)
		States[i] = (SamplerState*)states[samplers[i]];
	set_samplers(get_dc(cl), slot, num_samplers, States);
}

}}
