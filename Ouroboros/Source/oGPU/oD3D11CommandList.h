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

ID3D11Resource* oD3D11GetSubresource(oGPUResource* _pResource, int _Subresource, int* _pD3DSubresourceIndex);

// {2D6106C4-7741-41CD-93DE-2C2A9BCD9163}
oDECLARE_GPUDEVICECHILD_IMPLEMENTATION(oD3D11, CommandList, 0x2d6106c4, 0x7741, 0x41cd, 0x93, 0xde, 0x2c, 0x2a, 0x9b, 0xcd, 0x91, 0x63)
{
	oDEFINE_CONST_GETDESC_INTERFACE(Desc, threadsafe);
	oDEFINE_GPUDEVICECHILD_INTERFACE_EXPLICIT_QI();
	oDECLARE_GPUDEVICECHILD_CTOR(oD3D11, CommandList);
	~oD3D11CommandList();

	void Begin() override;
	void End() override;
	void BeginQuery(oGPUQuery* _pQuery) override;
	void EndQuery(oGPUQuery* _pQuery) override;
	void Flush() override;
	void Reset() override;
	void Reserve(oGPUResource* _pResource, int _Subresource, ouro::surface::mapped_subresource* _pMappedSubresource) override;
	void Commit(oGPUResource* _pResource, int _Subresource, const ouro::surface::mapped_subresource& _Source, const ouro::surface::box& _Subregion = ouro::surface::box()) override;
	void Copy(oGPUResource* _pDestination, oGPUResource* _pSource) override;
	void Copy(oGPUBuffer* _pDestination, int _DestinationOffsetBytes, oGPUBuffer* _pSource, int _SourceOffsetBytes, int _SizeBytes) override;
	void CopyCounter(oGPUBuffer* _pDestination, uint _DestinationAlignedOffset, oGPUBuffer* _pUnorderedSource) override;
	void SetCounters(int _NumUnorderedResources, oGPUResource** _ppUnorderedResources, uint* _pValues) override;
	void SetSamplers(int _StartSlot, int _NumStates, const ouro::gpu::sampler_type::value* _pSamplerState) override;
	void SetShaderResources(int _StartSlot, int _NumResources, const oGPUResource* const* _ppResources) override;
	void SetBuffers(int _StartSlot, int _NumBuffers, const oGPUBuffer* const* _ppConstants) override;

	void SetRenderTargetAndUnorderedResources(oGPURenderTarget* _pRenderTarget, int _NumViewports, const ouro::mesh::boundf* _pViewports, bool _SetForDispatch, int _UnorderedResourcesStartSlot, int _NumUnorderedResources, oGPUResource** _ppUnorderedResources, uint* _pInitialCounts = nullptr) override;
	void SetPipeline(const oGPUPipeline* _pPipeline) override;
	void SetSurfaceState(ouro::gpu::surface_state::value _State) override;
	void SetBlendState(ouro::gpu::blend_state::value _State) override;
	void SetDepthStencilState(ouro::gpu::depth_stencil_state::value _State) override;
	void Clear(oGPURenderTarget* _pRenderTarget, ouro::gpu::clear_type::value _Clear) override;
	void Draw(const oGPUBuffer* _pIndices, int _StartSlot, int _NumVertexBuffers, const oGPUBuffer* const* _ppVertexBuffers, uint _StartPrimitive, uint _NumPrimitives, uint _StartInstance = ouro::invalid, uint _NumInstances = ouro::invalid) override;
	void Draw(oGPUBuffer* _pDrawArgs, int _AlignedByteOffsetForArgs) override;
	void DrawSVQuad(uint _NumInstances = 1) override;
	bool GenerateMips(oGPURenderTarget* _pRenderTarget) override;
	void ClearI(oGPUResource* _pUnorderedResource, const uint4 _Values) override;
	void ClearF(oGPUResource* _pUnorderedResource, const float4 _Values) override;

	void Dispatch(oGPUComputeShader* _pComputeShader, const int3& _ThreadGroupCount) override;
	void Dispatch(oGPUComputeShader* _pComputeShader, oGPUBuffer* _pThreadGroupCountBuffer, int _AlignedByteOffsetToThreadGroupCount) override;

	ouro::intrusive_ptr<ID3D11DeviceContext> Context;
	ouro::intrusive_ptr<ID3D11CommandList> CommandList;
	DESC Desc;
	D3D_PRIMITIVE_TOPOLOGY PrimitiveTopology;

	// Shortcut to a bunch of typecasting
	// This thread_cast is safe because oD3D11CommandList is single-threaded
	// and most access is to get at common/safe resources
	inline oD3D11Device* D3DDevice() { return thread_cast<oD3D11Device*>(static_cast<threadsafe oD3D11Device*>(Device.c_ptr())); }

private:
	void CSSetState(oGPUComputeShader* _pComputeShader, int _NumUnorderedResources, const oGPUResource* const* _ppUnorderedResources, uint* _pInitialCounts);
	void SetVertexBuffers(const oGPUBuffer* _pIndexBuffer, int _StartSlot, int _NumVertexBuffers, const oGPUBuffer* const* _ppVertexBuffers, uint _StartVertex = 0);
};

#endif
