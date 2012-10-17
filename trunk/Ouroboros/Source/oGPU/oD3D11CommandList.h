/**************************************************************************
 * The MIT License                                                        *
 * Copyright (c) 2011 Antony Arciuolo & Kevin Myers                       *
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
#include <oPlatform/Windows/oD3D11.h>

ID3D11Resource* oD3D11GetSubresource(oGPUResource* _pResource, int _Subresource, int* _pD3DSubresourceIndex);

oDECLARE_GPUDEVICECHILD_IMPLEMENTATION(oD3D11, CommandList)
{
	oDEFINE_CONST_GETDESC_INTERFACE(Desc, threadsafe);
	oDEFINE_GPUDEVICECHILD_INTERFACE_EXPLICIT_QI();
	oDECLARE_GPUDEVICECHILD_CTOR(oD3D11, CommandList);
	~oD3D11CommandList();

	void Begin() override;
	void End() override;
	void Reserve(oGPUResource* _pResource, int _Subresource, oSURFACE_MAPPED_SUBRESOURCE* _pMappedSubresource) override;
	void Commit(oGPUResource* _pResource, int _Subresource, oSURFACE_MAPPED_SUBRESOURCE& _Source, const oRECT& _Subregion = oRECT()) override;
	void Copy(oGPUResource* _pDestination, oGPUResource* _pSource) override;
	void SetSamplers(int _StartSlot, int _NumStates, const oGPU_SAMPLER_STATE* _pSamplerState) override;
	void SetTextures(int _StartSlot, int _NumTextures, const oGPUTexture* const* _ppTextures) override;
	void SetConstants(int _StartSlot, int _NumConstants, const oGPUBuffer* const* _ppConstants) override;

	void SetRenderTarget(oGPURenderTarget* _pRenderTarget, int _NumViewports = 0, const oAABoxf* _pViewports = nullptr) override;
	void SetRenderTargetAndUnorderedTextures(oGPURenderTarget* _pRenderTarget, int _NumViewports, const oAABoxf* _pViewports, int _NumUnorderedTextures, oGPUTexture** _ppUnorderedTextures) override;
	void SetPipeline(const oGPUPipeline* _pPipeline) override;
	void SetSurfaceState(oGPU_SURFACE_STATE _State) override;
	void SetBlendState(oGPU_BLEND_STATE _State) override;
	void SetDepthStencilState(oGPU_DEPTH_STENCIL_STATE _State) override;
	void Clear(oGPURenderTarget* _pRenderTarget, oGPU_CLEAR _Clear) override;
	void Draw(const oGPUMesh* _pMesh, int _RangeIndex, const oGPUInstanceList* _pInstanceList = nullptr) override;
	void Draw(const oGPULineList* _pLineList) override;
	void DrawSVQuad(uint _NumInstances = 1) override;


	void ClearI(oGPUResource* _pUnorderedResource, const uint _Values[4]) override;
	void ClearF(oGPUResource* _pUnorderedResource, const float _Values[4]) override;
	void Dispatch(oGPUComputeShader* _pComputeShader, const int3& _ThreadGroupCount, int _NumUnorderedResources, const oGPUResource* const* _ppUnorderedResources) override;
	void Dispatch(oGPUComputeShader* _pComputeShader, oGPUBuffer* _pThreadGroupCountBuffer, int _AlignedByteOffsetToThreadGroupCount, int _NumUnorderedResources, const oGPUResource* const* _ppUnorderedResources) override;

	oRef<ID3D11DeviceContext> Context;
	oRef<ID3D11CommandList> CommandList;
	DESC Desc;

	// Shortcut to a bunch of typecasting
	// This thread_cast is safe because oD3D11CommandList is single-threaded
	// and most access is to get at common/safe resources
	inline oD3D11Device* D3DDevice() { return thread_cast<oD3D11Device*>(static_cast<threadsafe oD3D11Device*>(Device.c_ptr())); }
};

#endif
