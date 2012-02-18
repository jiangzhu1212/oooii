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
#pragma once
#ifndef oD3D11CommandList_h
#define oD3D11CommandList_h

#include <oGfx/oGfx.h>
#include "oGfxCommon.h"
#include "oD3D11RenderTarget.h"
#include "oD3D11Device.h"
#include <oPlatform/oD3D11.h>

oDECLARE_GFXDEVICECHILD_IMPLEMENTATION(oD3D11, CommandList)
{
	oDEFINE_CONST_GETDESC_INTERFACE(Desc, threadsafe);
	oDEFINE_GFXDEVICECHILD_INTERFACE();
	oDECLARE_GFXDEVICECHILD_CTOR(oD3D11, CommandList);
	~oD3D11CommandList();

	void Begin(
		const float4x4& _View
		, const float4x4& _Projection
		, const oGfxPipeline* _pPipeline
		, oGfxRenderTarget* _pRenderTarget
		, size_t _RenderTargetIndex
		, size_t _NumViewports
		, const VIEWPORT* _pViewports) override;

	void End() override;

	//void RSSetState(oRSSTATE _State) override;
	//void OMSetState(oOMSTATE _State) override;
	//void DSSetState(oDSSTATE _State) override;
	//void SASetStates(size_t _StartSlot, size_t _NumStates, const oSASTATE* _pSAStates, const oMBSTATE* _pMBStates) override;
	//void SetTextures(size_t _StartSlot, size_t _NumTextures, const oGfxTexture* const* _ppTextures) override;
	//void SetMaterials(size_t _StartSlot, size_t _NumMaterials, const oGfxMaterial* const* _ppMaterials) override;
	//void Map(oGfxResource* _pResource, size_t _SubresourceIndex, MAPPING* _pMapping) override;
	//void Unmap(oGfxResource* _pResource, size_t _SubresourceIndex, size_t _NewCount = 1) override;
	void Clear(CLEAR_TYPE _ClearType) override;
	//void DrawMesh(float4x4& _Transform, uint _MeshID, const oGfxMesh* _pMesh, size_t _RangeIndex, const oGfxInstanceList* _pInstanceList = nullptr) override;
	//void DrawLines(uint _LineListID, const oGfxLineList* _pLineList) override;
	//void DrawQuad(float4x4& _Transform, uint _MeshID) override;

	oRef<ID3D11DeviceContext> Context;
	oRef<ID3D11CommandList> CommandList;
	DESC Desc;

	oD3D11RenderTarget* pRenderTarget;

	// Shortcut to a bunch of typecasting
	// This thread_cast is safe because oD3D11CommandList is single-threaded
	// and most access is to get at common/safe resources
	inline oD3D11Device* D3DDevice() { return thread_cast<oD3D11Device*>(static_cast<threadsafe oD3D11Device*>(Device.c_ptr())); }
};

#endif