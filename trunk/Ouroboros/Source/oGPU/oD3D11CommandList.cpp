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
#include "oD3D11CommandList.h"
#include "oD3D11Pipeline.h"
#include "dxgi_util.h"
#include "d3d_types.h"

namespace ouro {
	namespace gpu {

d3d::DeviceContext* get_device_context(command_list* _pCommandList) { return ((d3d11::d3d11_command_list*)_pCommandList)->Context; }
d3d::DeviceContext* get_dc(command_list* _pCommandList) { return get_device_context(_pCommandList); }
d3d::ComputeShader* get_noop_cs(command_list* _pCommandList) { return ((d3d11::d3d11_command_list*)_pCommandList)->get_noop_cs(); }

	} // namespace gpu
} // namespace ouro

oGPU_NAMESPACE_BEGIN

oDEFINE_DEVICE_MAKE(command_list)
oDEVICE_CHILD_CTOR(command_list)
	, Info(_Info)
	, PrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_UNDEFINED)
	, WeakDevice(_Device.get())
{
	oD3D11_DEVICE();

	if (Info.draw_order == command_list_info::immediate)
		D3DDevice->GetImmediateContext(&Context);
	else
	{
		HRESULT hr = D3DDevice->CreateDeferredContext(0, &Context);
		if (FAILED(hr))
		{
			mstring err;

			UINT DeviceCreationFlags = D3DDevice->GetCreationFlags();

			if (DeviceCreationFlags & D3D11_CREATE_DEVICE_SINGLETHREADED)
				snprintf(err, "command_lists cannot be created on a device created as single-threaded.");
			else
				snprintf(err, "Failed to create command_list %u: ", Info.draw_order);

			throw windows::error(hr, err);
		}
		dev()->insert(this);
	}
}

d3d11_command_list::d3d11_command_list(std::shared_ptr<device>& _Device, ID3D11DeviceContext* _pDeviceContext, short _DrawOrder)
	: device_child_mixin<command_list_info, command_list, d3d11_command_list>(_Device, "immediate")
	, Context(_pDeviceContext)
	, PrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_UNDEFINED)
	, WeakDevice(_Device.get())
{
	Device = nullptr; // don't have a circular reference
	Info.draw_order = _DrawOrder;
}

d3d11_command_list::~d3d11_command_list()
{
	if (Info.draw_order != invalid)
		dev()->remove(this);
}

command_list_info d3d11_command_list::get_info() const
{
	return Info;
}

void d3d11_command_list::begin()
{
	if (Info.draw_order != invalid) // ignore for immediate
		dev()->block_submission();
}

void d3d11_command_list::end()
{
	if (Info.draw_order != invalid) // ignore for immediate
	{
		Context->FinishCommandList(FALSE, &CommandList);
		dev()->unblock_submission();
	}
}

void d3d11_command_list::flush()
{
	Context->Flush();
}

void d3d11_command_list::reset()
{
	Context->ClearState();
}

void d3d11_command_list::set_pipeline(const pipeline1* _pPipeline)
{
	if (_pPipeline)
	{
		d3d11_pipeline1* p = (d3d11_pipeline1*)_pPipeline;
		PrimitiveTopology = p->InputTopology;
		Context->IASetPrimitiveTopology(p->InputTopology);
		Context->IASetInputLayout((d3d::InputLayout*)p->VertexLayout.get());
		
		p->VertexShader.set(this);
		p->HullShader.set(this);
		p->DomainShader.set(this);
		p->GeometryShader.set(this);
		p->PixelShader.set(this);
	}

	else
	{
		PrimitiveTopology = D3D_PRIMITIVE_TOPOLOGY_UNDEFINED;
		Context->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_UNDEFINED);
		Context->IASetInputLayout(nullptr);
		Context->VSSetShader(nullptr, nullptr, 0);
		Context->HSSetShader(nullptr, nullptr, 0);
		Context->DSSetShader(nullptr, nullptr, 0);
		Context->GSSetShader(nullptr, nullptr, 0);
		Context->PSSetShader(nullptr, nullptr, 0);
	}
}

oGPU_NAMESPACE_END
