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
#include <oGPU/sampler.h>
#include "oCore/Windows/win_util.h"
#include "d3d11_util.h"
#include "d3d_debug.h"
#include "d3d_types.h"

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

oDEFINE_TO_STRING(gpu::sampler_state::value);
oDEFINE_FROM_STRING(gpu::sampler_state::value, gpu::sampler_state::count);

	namespace gpu {

Device* get_device(device* _pDevice);
DeviceContext* get_device_context(command_list* _pCommandList);

sampler make_sampler_state(device* _pDevice, const sampler_state::value& _Sampler)
{
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

	D3D11_SAMPLER_DESC desc;
	memset(&desc, 0, sizeof(desc));
	desc.Filter = sFilters[_Sampler];
	desc.AddressU = desc.AddressV = desc.AddressW = sAddresses[_Sampler];
	desc.MipLODBias = 0.0f;
	desc.MaxLOD = FLT_MAX; // documented default
	desc.MaxAnisotropy = 16; // documented default
	desc.ComparisonFunc = D3D11_COMPARISON_NEVER;  // documented default
	mstring StateName;
	snprintf(StateName, "sampler::%s", as_string(_Sampler));
	Device* device = get_device(_pDevice);
	SamplerState* s = nullptr;
	oV(device->CreateSamplerState(&desc, &s));
	debug_name(s, StateName);
	return (sampler)s;
}

void unmake_vertex_layout(sampler _SamplerState)
{
	if (_SamplerState)
		((DeviceChild*)_SamplerState)->Release();
}

void bind_samplers(command_list* _pCommandList, uint _StartSlot, const sampler* _pSamplerStates, uint _NumSamplerStates)
{
	DeviceContext* dc = get_device_context(_pCommandList);
	dc->VSSetSamplers(_StartSlot, _NumSamplerStates, (ID3D11SamplerState* const*)_pSamplerStates);
	dc->HSSetSamplers(_StartSlot, _NumSamplerStates, (ID3D11SamplerState* const*)_pSamplerStates);
	dc->DSSetSamplers(_StartSlot, _NumSamplerStates, (ID3D11SamplerState* const*)_pSamplerStates);
	dc->GSSetSamplers(_StartSlot, _NumSamplerStates, (ID3D11SamplerState* const*)_pSamplerStates);
	dc->PSSetSamplers(_StartSlot, _NumSamplerStates, (ID3D11SamplerState* const*)_pSamplerStates);
}

	} // namespace gpu
} // namespace ouro
