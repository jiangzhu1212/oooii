// $(header)
#pragma once
#ifndef oD3D11CommandList_h
#define oD3D11CommandList_h

#include <oGfx/oGfx.h>
#include "oGfxCommon.h"
#include "oD3D11RenderTarget.h"
#include <oooii/oD3D11.h>

oDECLARE_GFXDEVICECHILD_IMPLEMENTATION(oD3D11, CommandList)
{
	oDEFINE_GFXDEVICECHILD_INTERFACE();
	oDECLARE_GFXDEVICECHILD_CTOR(oD3D11, CommandList);
	~oD3D11CommandList();

	void GetDesc(DESC* _pDesc) const threadsafe override;

	void Begin(
		const float4x4& View
		, const float4x4& Projection
		, const oGfxPipeline* _pPipeline
		, const oGfxRenderTarget2* _pRenderTarget
		, size_t _NumViewports
		, const VIEWPORT* _pViewports) override;

	void End() override;

	void RSSetState(oRSSTATE _State) override;
	void OMSetState(oOMSTATE _State) override;
	void DSSetState(oDSSTATE _State) override;
	void SASetStates(size_t _StartSlot, size_t _NumStates, const oSASTATE* _pSAStates, const oMBSTATE* _pMBStates) override;
	void SetTextures(size_t _StartSlot, size_t _NumTextures, const oGfxTexture* const* _ppTextures) override;
	void SetMaterials(size_t _StartSlot, size_t _NumMaterials, const oGfxMaterial* const* _ppMaterials) override;
	void Map(oGfxResource* _pResource, size_t _SubresourceIndex, MAPPING* _pMapping) override;
	void Unmap(oGfxResource* _pResource, size_t _SubresourceIndex) override;
	void Clear(CLEAR_TYPE _ClearType) override;
	void Draw(float4x4& _Transform, uint _MeshID, const oGfxMesh* _pMesh, size_t _RangeIndex) override;
	void Draw(uint _LineListID, const oGfxLineList* _pLineList) override;
	void DrawQuad(float4x4& _Transform, uint _MeshID) override;

	oRef<ID3D11DeviceContext> Context;
	oRef<ID3D11CommandList> CommandList;
	DESC Desc;

	oD3D11RenderTarget2* pRenderTarget;
	oD3D11BlendState* pOMState;
	oD3D11RasterizerState* pRSState;
	oD3D11DepthStencilState* pDSState;
	oD3D11SamplerState* pSAState;
};

#endif
