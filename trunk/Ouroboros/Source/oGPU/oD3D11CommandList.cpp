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
#include "oD3D11Texture.h"
#include "oD3D11Query.h"
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

ID3D11Resource* get_subresource(resource1* _pResource, int _Subresource, uint* _pD3DSubresourceIndex)
{
	*_pD3DSubresourceIndex = _Subresource;
	switch (_pResource->type())
	{
		case resource_type::texture1: return static_cast<d3d11_texture1*>(_pResource)->pResource;
		oNODEFAULT;
	}
}

static ID3D11ShaderResourceView* get_srv(const resource1* _pResource, int _Miplevel, int _ArrayIndex, int _SRVIndex)
{
	switch (_pResource->type())
	{
		case resource_type::texture1:
		{
			d3d11_texture1* t = (d3d11_texture1*)_pResource;
			d3d11_texture1* t2 = (d3d11_texture1*)t->Texture2.get();
			return _SRVIndex == 0 ? t->SRV : (t2 ? t2->SRV : nullptr);
		}

		oNODEFAULT;
	}
}

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

void d3d11_command_list::begin_query(query* _pQuery)
{
	static_cast<d3d11_query*>(_pQuery)->begin(Context);
}

void d3d11_command_list::end_query(query* _pQuery)
{
	static_cast<d3d11_query*>(_pQuery)->end(Context);
}

void d3d11_command_list::flush()
{
	Context->Flush();
}

void d3d11_command_list::reset()
{
	Context->ClearState();
}

surface::mapped_subresource d3d11_command_list::reserve(resource1* _pResource, int _Subresource)
{
	return dev()->reserve(Context, _pResource, _Subresource);
}

void d3d11_command_list::commit(resource1* _pResource, int _Subresource, const surface::mapped_subresource& _Source, const surface::box& _Subregion)
{
	dev()->commit(Context, _pResource, _Subresource, _Source, _Subregion);
}

void d3d11_command_list::copy(buffer* _pDestination, uint _DestinationOffsetBytes, buffer* _pSource, uint _SourceOffsetBytes, uint _SizeBytes)
{
	uint D3DSubresourceIndex = 0;
	ID3D11Resource* d = get_subresource(_pDestination, 0, &D3DSubresourceIndex);
	ID3D11Resource* s = get_subresource(_pSource, 0, &D3DSubresourceIndex);

	D3D11_BOX CopyBox;
	CopyBox.left = _SourceOffsetBytes;
	CopyBox.top = 0;
	CopyBox.right = _SourceOffsetBytes + _SizeBytes;
	CopyBox.bottom = 1;
	CopyBox.front = 0;
	CopyBox.back = 1;

	Context->CopySubresourceRegion(d, 0, _DestinationOffsetBytes, 0, 0, s, 0, &CopyBox);
}

void d3d11_command_list::copy(resource1* _pDestination, resource1* _pSource)
{
	oCHECK(_pDestination && _pSource && _pDestination->type() == _pSource->type(), "Copy(%s, %s) can only occur between two same-typed objects", _pDestination ? as_string(_pDestination->type()) : "(null)", _pSource ? as_string(_pSource->type()) : "(null)");
	uint D3DSubresourceIndex = 0;
	ID3D11Resource* d = get_subresource(_pDestination, 0, &D3DSubresourceIndex);
	ID3D11Resource* s = get_subresource(_pSource, 0, &D3DSubresourceIndex);
	Context->CopyResource(d, s);
}

void d3d11_command_list::set_samplers(uint _StartSlot, uint _NumStates, const sampler_state::value* _pSamplerState)
{
	dev()->SamplerStates.set(this, _StartSlot, _NumStates, _pSamplerState);
}

void d3d11_command_list::set_shader_resources(uint _StartSlot, uint _NumResources, const resource1* const* _ppResources)
{
	const ID3D11ShaderResourceView* SRVs[D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT];
	uint InternalNumResources = _NumResources;
	bool SetSecondaries = false;

	if (!_NumResources || !_ppResources)
	{
		memset(SRVs, 0, sizeof(ID3D11ShaderResourceView*) * D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT);
		InternalNumResources = D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT;
	}
	else
	{
		oCHECK(oCOUNTOF(SRVs) >= _NumResources, "Too many resources specified");
		for (uint i = 0; i < _NumResources; i++)
			SRVs[i] = _ppResources[i] ? get_srv(_ppResources[i], 0, 0, 0) : nullptr;

		SetSecondaries = true;
	}

	set_srvs(Context, _StartSlot, InternalNumResources, SRVs);

	// Primarily for YUV emulation: bind a secondary buffer (i.e. AY is in the
	// primary, UV is in the secondary) backing from the end of the resource
	// array.
	if (SetSecondaries)
	{
		for (uint i = 0, j = _NumResources-1; i < _NumResources; i++, j--)
			SRVs[j] = _ppResources[i] ? get_srv(_ppResources[i], 0, 0, 1) : nullptr;

		set_srvs(Context, D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT - _NumResources - _StartSlot, _NumResources, SRVs);
	}
}

#if 0
static void set_viewports(ID3D11DeviceContext* _pDeviceContext, const int2& _TargetDimensions, uint _NumViewports, const aaboxf* _pViewports)
{
	D3D11_VIEWPORT Viewports[D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE];
	if (_NumViewports && _pViewports)
	{
		for (uint i = 0; i < _NumViewports; i++)
			Viewports[i] = to_viewport(_pViewports[i]);
	}

	else
	{
		_NumViewports = 1;
		Viewports[0] = to_viewport(_TargetDimensions);
	}

	_pDeviceContext->RSSetViewports(_NumViewports, Viewports);
}

void d3d11_command_list::set_render_target_and_unordered_resources(render_target* _pRenderTarget, uint _NumViewports, const aaboxf* _pViewports, bool _SetForDispatch, uint _UnorderedResourcesStartSlot, uint _NumUnorderedResources, resource1** _ppUnorderedResources, uint* _pInitialCounts)
{
	oCHECK(!_SetForDispatch || !_pRenderTarget, "If _SetForDispatch is true, then _pRenderTarget must be nullptr");

	d3d11_render_target* RT = static_cast<d3d11_render_target*>(_pRenderTarget);

	uint StartSlot = _UnorderedResourcesStartSlot;
	if (StartSlot == invalid)
		StartSlot = RT ? RT->Info.num_mrts : 0;

	ID3D11UnorderedAccessView* UAVs[max_num_unordered_buffers];
	get_uavs(UAVs, _NumUnorderedResources, _ppUnorderedResources, 0, 0, !_SetForDispatch);

	uint NumUAVs = _NumUnorderedResources;
	if (_NumUnorderedResources == invalid)
		NumUAVs = max_num_unordered_buffers - StartSlot;

	uint* pInitialCounts = _pInitialCounts;
	if (!pInitialCounts)
		pInitialCounts = dev()->NoopUAVInitialCounts;

	if (_SetForDispatch)
	{
		// buffers can't be bound in both places at once, so ensure that state
		Context->OMSetRenderTargetsAndUnorderedAccessViews(0, nullptr, nullptr, 0, max_num_unordered_buffers, dev()->NullUAVs, dev()->NoopUAVInitialCounts);
		Context->CSSetUnorderedAccessViews(StartSlot, NumUAVs, UAVs, pInitialCounts);
	}

	else
	{
		// buffers can't be bound in both places at once, so ensure that state
		Context->CSSetUnorderedAccessViews(0, max_num_unordered_buffers, dev()->NullUAVs, dev()->NoopUAVInitialCounts);

		if (RT)
		{
			Context->OMSetRenderTargetsAndUnorderedAccessViews(RT->Info.num_mrts, (ID3D11RenderTargetView* const*)RT->RTVs.data(), RT->DSV, StartSlot, NumUAVs, UAVs, pInitialCounts);
			render_target_info i = _pRenderTarget->get_info();
			set_viewports(Context, i.dimensions.xy(), _NumViewports, _pViewports);
		}

		else
		{
			oASSERT(NumUAVs >= 0 && NumUAVs <= max_num_unordered_buffers, "Invalid _NumUnorderedResources");
			Context->OMSetRenderTargetsAndUnorderedAccessViews(0, nullptr, nullptr, StartSlot, NumUAVs, UAVs, pInitialCounts);
			set_viewports(Context, 1, _NumViewports, _pViewports);
		}	
	}
}
#endif

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

void d3d11_command_list::set_rasterizer_state(const rasterizer_state::value& _State)
{
	dev()->RasterizerStates.set(this, _State);
}

void d3d11_command_list::set_blend_state(const blend_state::value& _State)
{
	dev()->BlendStates.set(this, _State);
}

void d3d11_command_list::set_depth_stencil_state(const depth_stencil_state::value& _State)
{
	dev()->DepthStencilStates.set(this, _State);
}

oGPU_NAMESPACE_END
