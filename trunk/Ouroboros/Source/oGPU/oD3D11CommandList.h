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
#ifndef oD3D11CommandList_h
#define oD3D11CommandList_h

#include <oGPU/oGPU.h>
#include "oGPUCommon.h"
#include "oD3D11RenderTarget.h"
#include "oD3D11Device.h"
#include <d3d11.h>

oGPU_NAMESPACE_BEGIN

// Abstracts the differences between a texture and a buffer
ID3D11Resource* get_subresource(resource* _pResource, int _Subresource, uint* _pD3DSubresourceIndex);

oDEVICE_CHILD_CLASS(command_list)
{
	oDEVICE_CHILD_DECLARATION(command_list)
	d3d11_command_list(std::shared_ptr<device>& _Device, ID3D11DeviceContext* _pDeviceContext, short _DrawOrder);
	~d3d11_command_list();

	inline d3d11_device* dev() { return static_cast<d3d11_device*>(WeakDevice); }
	inline d3d::ComputeShader* get_noop_cs() { return (d3d::ComputeShader*)dev()->NoopCS.get(); }

	command_list_info get_info() const override;
	void begin() override;
	void end() override;
	void begin_query(query* _pQuery) override;
	void end_query(query* _pQuery) override;
	void flush() override;
	void reset() override;
	surface::mapped_subresource reserve(resource* _pResource, int _Subresource) override;
	void commit(resource* _pResource, int _Subresource, const surface::mapped_subresource& _Source, const surface::box& _Subregion = surface::box()) override;
	void copy(resource* _pDestination, resource* _pSource) override;
	void copy(buffer* _pDestination, uint _DestinationOffsetBytes, buffer* _pSource, uint _SourceOffsetBytes, uint _SizeBytes) override;
	void copy_counter(buffer* _pDestination, uint _DestinationAlignedOffset, buffer* _pUnorderedSource) override;
	void set_counters(uint _NumUnorderedResources, resource** _ppUnorderedResources, uint* _pValues) override;
	void set_samplers(uint _StartSlot, uint _NumStates, const sampler_state::value* _pSamplerState) override;
	void set_shader_resources(uint _StartSlot, uint _NumResources, const resource* const* _ppResources) override;
	void set_buffers(uint _StartSlot, uint _NumBuffers, const buffer* const* _ppBuffers) override;
	void set_render_target_and_unordered_resources(render_target* _pRenderTarget, uint _NumViewports, const aaboxf* _pViewports, bool _SetForDispatch, uint _UnorderedResourcesStartSlot, uint _NumUnorderedResources, resource** _ppUnorderedResources, uint* _pInitialCounts = nullptr) override;

	// _____________________________________________________________________________
	// Rasterization-specific

	void set_pipeline(const pipeline1* _pPipeline) override;
	void set_rasterizer_state(const rasterizer_state::value& _State) override;
	void set_blend_state(const blend_state::value& _State) override;
	void set_depth_stencil_state(const depth_stencil_state::value& _State) override;
	void clear(render_target* _pRenderTarget, const clear_type::value& _Clear) override;
	void draw(
		const buffer* _pIndices
		, uint _StartSlot
		, uint _NumVertexBuffers
		, const buffer* const* _ppVertexBuffers
		, uint _StartPrimitive
		, uint _NumPrimitives
		, uint _StartInstance = invalid
		, uint _NumInstances = invalid
	) override;

	void draw(buffer* _pDrawArgs, uint _AlignedByteOffsetForArgs) override;
	void generate_mips(render_target* _pRenderTarget) override;

	// _____________________________________________________________________________
	// Compute-specific

	void cleari(resource* _pUnorderedResource, const uint4& _Values) override;
	void clearf(resource* _pUnorderedResource, const float4& _Values) override;
	void dispatch(compute_kernel* _pComputeShader, const int3& _ThreadGroupCount) override;
	void dispatch(compute_kernel* _pComputeShader, buffer* _pThreadGroupCountBuffer, uint _AlignedByteOffsetToThreadGroupCount) override;

	device* WeakDevice;
	intrusive_ptr<ID3D11DeviceContext> Context;
	intrusive_ptr<ID3D11CommandList> CommandList;
	command_list_info Info;
	D3D_PRIMITIVE_TOPOLOGY PrimitiveTopology;

	void set_vertex_buffers(const buffer* _pIndexBuffer, uint _StartSlot, uint _NumVertexBuffers, const buffer* const* _ppVertexBuffers, uint _StartVertex = 0);
};

oGPU_NAMESPACE_END

#endif