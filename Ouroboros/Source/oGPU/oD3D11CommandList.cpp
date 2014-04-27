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
#include "oD3D11Buffer.h"
#include "oD3D11CommandList.h"
#include "oD3D11ComputeShader.h"
#include "oD3D11Pipeline.h"
#include "oD3D11Texture.h"
#include "oD3D11Query.h"
#include "dxgi_util.h"
#include "d3d_types.h"

namespace ouro {
	namespace gpu {

d3d::DeviceContext* get_device_context(device_context* _pDeviceContext) { return ((d3d11::d3d11_command_list*)_pDeviceContext)->Context; }

	} // namespace gpu
} // namespace ouro

oGPU_NAMESPACE_BEGIN

ID3D11Resource* get_subresource(resource* _pResource, int _Subresource, uint* _pD3DSubresourceIndex)
{
	*_pD3DSubresourceIndex = _Subresource;
	switch (_pResource->type())
	{
		case resource_type::buffer: return static_cast<d3d11_buffer*>(_pResource)->Buffer;
		case resource_type::texture: return static_cast<d3d11_texture*>(_pResource)->pResource;
		oNODEFAULT;
	}
}

static ID3D11UnorderedAccessView* get_uav(const resource* _pResource, int _Miplevel, int _Slice, bool _UseAppendIfAvailable = false, bool _CheckHasCounter = false)
{
	switch (_pResource->type())
	{
		case resource_type::buffer:
		{
			auto b = static_cast<const d3d11_buffer*>(_pResource);
			oCHECK(!_CheckHasCounter || b->has_counter(), "Buffer does not have a counter");
			return b->choose_uav(_UseAppendIfAvailable);
		}

		case resource_type::texture: return ((d3d11_texture*)_pResource)->UAV;
		oNODEFAULT;
	}
}

static void get_uavs(ID3D11UnorderedAccessView* (&_ppUAVs)[max_num_unordered_buffers], int _NumUnorderedResources, resource** _ppUnorderedResources, int _Miplevel, int _Slice, bool _UseAppendIfAvailable = false, bool _AssertHaveCounters = false)
{
	if (!_ppUnorderedResources || _NumUnorderedResources == invalid || _NumUnorderedResources == 0)
		memset(_ppUAVs, 0, sizeof(_ppUAVs));
	else
	{
		for (int i = 0; i < _NumUnorderedResources; i++)
		{
			if (_ppUnorderedResources[i])
			{
				_ppUAVs[i] = get_uav(_ppUnorderedResources[i], 0, 0, _UseAppendIfAvailable, _AssertHaveCounters);
				oCHECK(_ppUAVs[i], "The specified resource %p %s does not have an unordered resource view", _ppUnorderedResources[i], _ppUnorderedResources[i]->name()); 
			}
			else 
				_ppUAVs[i] = nullptr;
		}
	}
}

static ID3D11ShaderResourceView* get_srv(const resource* _pResource, int _Miplevel, int _ArrayIndex, int _SRVIndex)
{
	switch (_pResource->type())
	{
		case resource_type::buffer: return ((d3d11_buffer*)_pResource)->SRV;
		case resource_type::texture:
		{
			d3d11_texture* t = (d3d11_texture*)_pResource;
			d3d11_texture* t2 = (d3d11_texture*)t->Texture2.get();
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

surface::mapped_subresource d3d11_command_list::reserve(resource* _pResource, int _Subresource)
{
	return dev()->reserve(Context, _pResource, _Subresource);
}

void d3d11_command_list::commit(resource* _pResource, int _Subresource, const surface::mapped_subresource& _Source, const surface::box& _Subregion)
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

void d3d11_command_list::copy(resource* _pDestination, resource* _pSource)
{
	oCHECK(_pDestination && _pSource && _pDestination->type() == _pSource->type(), "Copy(%s, %s) can only occur between two same-typed objects", _pDestination ? as_string(_pDestination->type()) : "(null)", _pSource ? as_string(_pSource->type()) : "(null)");
	uint D3DSubresourceIndex = 0;
	ID3D11Resource* d = get_subresource(_pDestination, 0, &D3DSubresourceIndex);
	ID3D11Resource* s = get_subresource(_pSource, 0, &D3DSubresourceIndex);
	Context->CopyResource(d, s);
}

void d3d11_command_list::copy_counter(buffer* _pDestination, uint _DestinationAlignedOffset, buffer* _pUnorderedSource)
{
	buffer_info i = static_cast<buffer*>(_pUnorderedSource)->get_info();
	oCHECK(i.type == buffer_type::unordered_structured_append || i.type == buffer_type::unordered_structured_counter, "Source must be an unordered structured buffer with APPEND or COUNTER modifiers");
	oCHECK(byte_aligned(_DestinationAlignedOffset, sizeof(uint)), "_DestinationAlignedOffset must be sizeof(uint)-aligned");
	Context->CopyStructureCount(static_cast<d3d11_buffer*>(_pDestination)->Buffer, _DestinationAlignedOffset, get_uav(_pUnorderedSource, 0, 0, true));
}

void d3d11_command_list::set_counters(uint _NumUnorderedResources, resource** _ppUnorderedResources, uint* _pValues)
{
	ID3D11UnorderedAccessView* UAVs[max_num_unordered_buffers];
	get_uavs(UAVs, _NumUnorderedResources, _ppUnorderedResources, 0, 0, true);

	// Executing a noop only seems to apply the initial counts if done through
	// OMSetRenderTargetsAndUnorderedAccessViews, so set things up that way with
	// a false setting here, and then flush it with a dispatch of a noop.
	Context->CSSetUnorderedAccessViews(0, max_num_unordered_buffers, dev()->NullUAVs, dev()->NoopUAVInitialCounts); // clear any conflicting binding
	Context->OMSetRenderTargetsAndUnorderedAccessViews(0, nullptr, nullptr, 0, _NumUnorderedResources, UAVs, _pValues); // set up binding
	Context->CSSetShader(dev()->NoopCS, nullptr, 0);
	Context->Dispatch(1, 1, 1);
}

void d3d11_command_list::set_samplers(uint _StartSlot, uint _NumStates, const sampler_type::value* _pSamplerState)
{
	ID3D11SamplerState* Samplers[D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT];
	oCHECK(oCOUNTOF(Samplers) >= _NumStates, "Too many samplers specified");
	for (uint i = 0; i < _NumStates; i++)
		Samplers[i] = dev()->SamplerStates[_pSamplerState[i]];
	
	d3d11::set_samplers(Context, as_uint(_StartSlot), as_uint(_NumStates), Samplers);
}

void d3d11_command_list::set_shader_resources(uint _StartSlot, uint _NumResources, const resource* const* _ppResources)
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

void d3d11_command_list::set_buffers(uint _StartSlot, uint _NumBuffers, const buffer* const* _ppBuffers)
{
	const ID3D11Buffer* CBs[D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT];
	oASSERT(oCOUNTOF(CBs) > _NumBuffers, "Too many buffers specified");

	if (_ppBuffers)
		for (uint i = 0; i < _NumBuffers; i++)
			CBs[i] = _ppBuffers[i] ? ((d3d11_buffer*)_ppBuffers[i])->Buffer.c_ptr() : nullptr;
	else
		for (uint i = 0; i < _NumBuffers; i++)
			CBs[i] = nullptr;

	set_constant_buffers(Context, _StartSlot, _NumBuffers, CBs);
}

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

void d3d11_command_list::set_render_target_and_unordered_resources(render_target* _pRenderTarget, uint _NumViewports, const aaboxf* _pViewports, bool _SetForDispatch, uint _UnorderedResourcesStartSlot, uint _NumUnorderedResources, resource** _ppUnorderedResources, uint* _pInitialCounts)
{
	oCHECK(!_SetForDispatch || !_pRenderTarget, "If _SetForDispatch is true, then _pRenderTarget must be nullptr");

	d3d11_render_target* RT = static_cast<d3d11_render_target*>(_pRenderTarget);

	uint StartSlot = _UnorderedResourcesStartSlot;
	if (StartSlot == invalid)
		StartSlot = RT ? RT->Info.mrt_count : 0;

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
			Context->OMSetRenderTargetsAndUnorderedAccessViews(RT->Info.mrt_count, (ID3D11RenderTargetView* const*)RT->RTVs.data(), RT->DSV, StartSlot, NumUAVs, UAVs, pInitialCounts);
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

void d3d11_command_list::set_pipeline(const pipeline1* _pPipeline)
{
	if (_pPipeline)
	{
		d3d11_pipeline1* p = (d3d11_pipeline1*)_pPipeline;
		PrimitiveTopology = p->InputTopology;
		Context->IASetPrimitiveTopology(p->InputTopology);
		Context->IASetInputLayout(p->InputLayout);
		Context->VSSetShader(p->VertexShader, 0, 0);
		Context->HSSetShader(p->HullShader, 0, 0);
		Context->DSSetShader(p->DomainShader, 0, 0);
		Context->GSSetShader(p->GeometryShader, 0, 0);
		Context->PSSetShader(p->PixelShader, 0, 0);
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

void d3d11_command_list::set_surface_state(const surface_state::value& _State)
{
	Context->RSSetState(dev()->SurfaceStates[_State]);
}

void d3d11_command_list::set_blend_state(const blend_state::value& _State)
{
	static const FLOAT sBlendFactor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	Context->OMSetBlendState(dev()->BlendStates[_State], sBlendFactor, 0xffffffff);
}

void d3d11_command_list::set_depth_stencil_state(const depth_stencil_state::value& _State)
{
	Context->OMSetDepthStencilState(dev()->DepthStencilStates[_State], 0);
}

void d3d11_command_list::clear(render_target* _pRenderTarget, const clear_type::value& _Clear)
{
	d3d11_render_target* RT = static_cast<d3d11_render_target*>(_pRenderTarget);

	if (_Clear >= clear_type::color)
	{
		float c[4];
		for (int i = 0; i < RT->Info.mrt_count; i++)
		{
			RT->Info.clear.clear_color[i].decompose(&c[0], &c[1], &c[2], &c[3]);
			Context->ClearRenderTargetView(RT->RTVs[i], c);
		}
	}

	static const UINT sClearFlags[] = 
	{
		D3D11_CLEAR_DEPTH,
		D3D11_CLEAR_STENCIL,
		D3D11_CLEAR_DEPTH|D3D11_CLEAR_STENCIL,
		0,
		D3D11_CLEAR_DEPTH,
		D3D11_CLEAR_STENCIL,
		D3D11_CLEAR_DEPTH|D3D11_CLEAR_STENCIL,
	};
	static_assert(oCOUNTOF(sClearFlags) == clear_type::count, "# clears mismatch");

	if (RT->DSV && _Clear != clear_type::color)
		Context->ClearDepthStencilView(RT->DSV, sClearFlags[_Clear], RT->Info.clear.depth_clear_value, RT->Info.clear.stencil_clear_value);
}

void d3d11_command_list::set_vertex_buffers(const buffer* _pIndexBuffer, uint _StartSlot, uint _NumVertexBuffers, const buffer* const* _ppVertexBuffers, uint _StartVertex)
{
	ID3D11Buffer* b = nullptr;
	DXGI_FORMAT Format = DXGI_FORMAT_R32_UINT;
	const uint _StartIndex = 0; // expose this as a parameter?
	// this can be set to non-zero, but it seems redundant in DrawIndexed and 
	// DrawIndexedInstanced, so keep it hidden for now.
	uint StartByteOffset = 0;
	if (_pIndexBuffer)
	{
		b = ((d3d11_buffer*)_pIndexBuffer)->Buffer;
		buffer_info i = _pIndexBuffer->get_info();
		Format = dxgi::from_surface_format(i.format);
		StartByteOffset = i.struct_byte_size * _StartIndex;
	}
	Context->IASetIndexBuffer(b, Format, StartByteOffset);

	uint Strides[D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT];
	uint ByteOffsets[D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT];
	const ID3D11Buffer* pVertices[D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT];

	if (_ppVertexBuffers)
	{
		for (uint i = 0; i < _NumVertexBuffers; i++)
		{
			Strides[i] = 0;
			ByteOffsets[i] = 0;

			buffer_info bi;
			if (_ppVertexBuffers[i])
			{
				bi = _ppVertexBuffers[i]->get_info();
				Strides[i] = bi.struct_byte_size;
				ByteOffsets[i] = _StartVertex * bi.struct_byte_size;
				pVertices[i] = static_cast<const d3d11_buffer*>(_ppVertexBuffers[i])->Buffer;
			}
			else
				pVertices[i] = nullptr;
		}
	}

	else
		memset(pVertices, 0 , sizeof(ID3D11Buffer*) * _NumVertexBuffers);	

	Context->IASetVertexBuffers(_StartSlot, _NumVertexBuffers, const_cast<ID3D11Buffer* const*>(pVertices), Strides, ByteOffsets);
}

void d3d11_command_list::draw(const buffer* _pIndices, uint _StartSlot, uint _NumVertexBuffers, const buffer* const* _ppVertexBuffers, uint _StartPrimitive, uint _NumPrimitives, uint _StartInstance, uint _NumInstances)
{
	const uint _StartVertex = 0;
	set_vertex_buffers(_pIndices, _StartSlot, _NumVertexBuffers, _ppVertexBuffers, _StartVertex);

	#ifdef _DEBUG
	{
		intrusive_ptr<ID3D11InputLayout> InputLayout = 0;
		Context->IAGetInputLayout(&InputLayout);
		oCHECK(!_ppVertexBuffers || InputLayout, "No InputLayout specified");
	}
	#endif

	const uint NumElements = num_elements(PrimitiveTopology, _NumPrimitives);
	const uint StartElement = num_elements(PrimitiveTopology, _StartPrimitive);

	if (!!_pIndices)
	{
		if (_NumInstances != invalid)
			Context->DrawIndexedInstanced(NumElements, _NumInstances, StartElement, 0, _StartInstance);
		else
			Context->DrawIndexed(NumElements, StartElement, 0);
	}

	else
	{
		if (_NumInstances != invalid)
			Context->DrawInstanced(NumElements, _NumInstances, StartElement, _StartInstance);
		else
			Context->Draw(NumElements, StartElement);
	}
}

void d3d11_command_list::draw(buffer* _pDrawArgs, uint _AlignedByteOffsetForArgs)
{
	Context->DrawInstancedIndirect(static_cast<d3d11_buffer*>(_pDrawArgs)->Buffer, _AlignedByteOffsetForArgs);
}

void d3d11_command_list::generate_mips(render_target* _pRenderTarget)
{
	render_target_info i = _pRenderTarget->get_info();
	if (!is_mipped(i.type))
		oTHROW_INVARG("Cannot generate mips if the type doesn't contain oGPU_TRAIT_TEXTURE_MIPS");
	d3d11_render_target* RT = static_cast<d3d11_render_target*>(_pRenderTarget);
	Context->GenerateMips(static_cast<d3d11_texture*>(RT->Textures[0].get())->SRV);
}

void d3d11_command_list::cleari(resource* _pUnorderedResource, const uint4& _Values)
{
	ID3D11UnorderedAccessView* UAV = get_uav(_pUnorderedResource, 0, 0, false);
	oCHECK(UAV, "The specified resource %p %s does not an unordered resource view", _pUnorderedResource, _pUnorderedResource->name());
	Context->ClearUnorderedAccessViewUint(UAV, (const uint*)&_Values);
}

void d3d11_command_list::clearf(resource* _pUnorderedResource, const float4& _Values)
{
	ID3D11UnorderedAccessView* UAV = get_uav(_pUnorderedResource, 0, 0, false);
	oCHECK(UAV, "The specified resource %p %s does not an unordered resource view", _pUnorderedResource, _pUnorderedResource->name());
	Context->ClearUnorderedAccessViewFloat(UAV, (const float*)&_Values);
}

void d3d11_command_list::dispatch(compute_kernel* _pComputeShader, const int3& _ThreadGroupCount)
{
	oCHECK(all(_ThreadGroupCount <= int3(D3D11_CS_DISPATCH_MAX_THREAD_GROUPS_PER_DIMENSION)), "_ThreadGroupCount cannot have a dimension greater than %u", D3D11_CS_DISPATCH_MAX_THREAD_GROUPS_PER_DIMENSION);
	Context->CSSetShader(static_cast<d3d11_compute_kernel*>(_pComputeShader)->ComputeShader, nullptr, 0);
	Context->Dispatch(_ThreadGroupCount.x, _ThreadGroupCount.y, _ThreadGroupCount.z);
}

void d3d11_command_list::dispatch(compute_kernel* _pComputeShader, buffer* _pThreadGroupCountBuffer, uint _AlignedByteOffsetToThreadGroupCount)
{
	#ifdef _DEBUG
		buffer_info i = _pThreadGroupCountBuffer->get_info();
		oCHECK(i.type == buffer_type::unordered_raw, "Parameters for dispatch must come from an oGPUBuffer of type buffer_type::unordered_raw");
		// Found this out the hard way... if a UAV is bound as a target, then it 
		// won't be flushed such that values are ready for this indirect dispatch.
		// D3D is quiet about it, so do the check here...
		ID3D11Buffer* pBuffer = static_cast<d3d11_buffer*>(_pThreadGroupCountBuffer)->Buffer;
		check_bound_rts_and_uavs(Context, 1, &pBuffer);
		check_bound_cs_uavs(Context, 1, &pBuffer);
	#endif
	Context->CSSetShader(static_cast<d3d11_compute_kernel*>(_pComputeShader)->ComputeShader, nullptr, 0);
	Context->DispatchIndirect(static_cast<d3d11_buffer*>(_pThreadGroupCountBuffer)->Buffer, _AlignedByteOffsetToThreadGroupCount);
}

oGPU_NAMESPACE_END
