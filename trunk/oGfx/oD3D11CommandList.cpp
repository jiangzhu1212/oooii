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
#include "oD3D11CommandList.h"
//#include "oD3D11InstanceList.h"
//#include "oD3D11LineList.h"
//#include "oD3D11Material.h"
#include "oD3D11Mesh.h"
#include "oD3D11Pipeline.h"
//#include "oD3D11Texture.h"
#include <oGfx/oGfxDrawConstants.h>

oDEFINE_GFXDEVICE_CREATE(oD3D11, CommandList);
oBEGIN_DEFINE_GFXDEVICECHILD_CTOR(oD3D11, CommandList)
{
	*_pSuccess = false;
	oD3D11DEVICE();

	HRESULT hr = D3DDevice->CreateDeferredContext(0, &Context);
	if (FAILED(hr))
	{
		char err[128];
		sprintf_s(err, "Failed to create oGfxDeviceContext %u: ", _Desc.DrawOrder);
		oWinSetLastError(hr, err);
		return;
	}

	oDEVICE_REGISTER_THIS();

	*_pSuccess = true;
}

oD3D11CommandList::~oD3D11CommandList()
{
	oDEVICE_UNREGISTER_THIS();
}

static void SetViewports(ID3D11DeviceContext* _pDeviceContext, const oGfxRenderTarget::DESC& _RTDesc, size_t _NumViewports, const oGfxCommandList::VIEWPORT* _pViewports)
{
	D3D11_VIEWPORT Viewports[D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE];
	if (_NumViewports && _pViewports)
	{
		for (size_t i = 0; i < _NumViewports; i++)
		{
			memcpy(&Viewports[i], &_pViewports[i], sizeof(oGfxCommandList::VIEWPORT));
			Viewports[i].MinDepth = 0.0f;
			Viewports[i].MaxDepth = 1.0f;
		}
	}

	else
	{
		_NumViewports = 1;
		Viewports[0].TopLeftX = 0.0f;
		Viewports[0].TopLeftY = 0.0f;
		Viewports[0].Width = static_cast<float>(_RTDesc.Dimensions.x);
		Viewports[0].Height = static_cast<float>(_RTDesc.Dimensions.y);
		Viewports[0].MinDepth = 0.0f;
		Viewports[0].MaxDepth = 1.0f;
	}

	_pDeviceContext->RSSetViewports(static_cast<uint>(_NumViewports), Viewports);
}


static void SetViewConstants(ID3D11DeviceContext* _pDeviceContext, ID3D11Buffer* _pViewConstants, const float4x4& _View, const float4x4& _Projection, const oGfxRenderTarget::DESC& _RTDesc, size_t _RenderTargetIndex)
{
	//D3D11_MAPPED_SUBRESOURCE mapped;
	//_pDeviceContext->Map(_pViewConstants, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
	//oDeferredViewConstants* V = (oDeferredViewConstants*)mapped.pData;
	//V->Set(_View, _Projection, asfloat(uint2(_RTDesc.Width, _RTDesc.Height)), static_cast<uint>(_RenderTargetIndex));
	//_pDeviceContext->Unmap(_pViewConstants, 0);
	//oD3D11SetConstantBuffers(_pDeviceContext, 0, 1, &_pViewConstants);
}

static void SetPipeline(ID3D11DeviceContext* _pDeviceContext, const oGfxPipeline* _pPipeline)
{
	oASSERT(_pPipeline, "A pipline must be specified");
	oD3D11Pipeline* p = const_cast<oD3D11Pipeline*>(static_cast<const oD3D11Pipeline*>(_pPipeline));
	_pDeviceContext->IASetInputLayout(p->InputLayout);
	_pDeviceContext->VSSetShader(p->VertexShader, 0, 0);
	_pDeviceContext->HSSetShader(p->HullShader, 0, 0);
	_pDeviceContext->DSSetShader(p->DomainShader, 0, 0);
	_pDeviceContext->GSSetShader(p->GeometryShader, 0, 0);
	_pDeviceContext->PSSetShader(p->PixelShader, 0, 0);
}

void oD3D11CommandList::Begin(
	const float4x4& _View
	, const float4x4& _Projection
	, const oGfxPipeline* _pPipeline
	, oGfxRenderTarget* _pRenderTarget
	, size_t _RenderTargetIndex
	, size_t _NumViewports
	, const VIEWPORT* _pViewports)
{
	oASSERT(_pRenderTarget, "A render target must be specified");

	oDEVICE_LOCK_SUBMIT();

	oGfxRenderTarget::DESC RTDesc;
	_pRenderTarget->GetDesc(&RTDesc);
	pRenderTarget = static_cast<oD3D11RenderTarget*>(_pRenderTarget);
	Context->OMSetRenderTargets(RTDesc.MRTCount, (ID3D11RenderTargetView* const*)pRenderTarget->RTVs, (ID3D11DepthStencilView*)pRenderTarget->DSV.c_ptr());

	SetViewports(Context, RTDesc, _NumViewports, _pViewports);
	SetViewConstants(Context, D3DDevice()->ViewConstants, _View, _Projection, RTDesc, _RenderTargetIndex);
	SetPipeline(Context, _pPipeline);
}

void oD3D11CommandList::End()
{
	pRenderTarget = nullptr;
	Context->FinishCommandList(FALSE, &CommandList);

	oDEVICE_UNLOCK_SUBMIT();
}
#if 0
void oD3D11CommandList::RSSetState(oRSSTATE _State)
{
	// @oooii-tony: TODO: Move oD3D11BlendState's enum to be oRS_STATE
	oASSERT(0, "Enums are not current compatible");
	//oSTATICASSERT(oOM_COUNT == oD3D11BlendState::NUM_STATES);
	D3DDevice()->OMState.SetState(Context, (oD3D11BlendState::STATE)_State);
}

void oD3D11CommandList::OMSetState(oOMSTATE _State)
{
	D3DDevice()->OMState.SetState(Context, (oD3D11BlendState::STATE)_State);
}

void oD3D11CommandList::DSSetState(oDSSTATE _State)
{
	D3DDevice()->DSState.SetState(Context, (oD3D11DepthStencilState::STATE)_State);
}

void oD3D11CommandList::SASetStates(size_t _StartSlot, size_t _NumStates, const oSASTATE* _pSAStates, const oMBSTATE* _pMBStates)
{
	D3DDevice()->SAState.SetState(Context, _StartSlot, reinterpret_cast<const oD3D11SamplerState::SAMPLER_STATE*>(_pSAStates), reinterpret_cast<const oD3D11SamplerState::MIP_BIAS_LEVEL*>(_pMBStates), _NumStates);
}

void oD3D11CommandList::SetTextures(size_t _StartSlot, size_t _NumTextures, const oGfxTexture* const* _ppTextures)
{
	const ID3D11ShaderResourceView* SRVs[D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT];
	for (size_t i = 0; i < _NumTextures; i++)
		SRVs[i] = const_cast<ID3D11ShaderResourceView*>(static_cast<const oD3D11Texture*>(_ppTextures[i])->SRV.c_ptr());

	oD3D11SetShaderResourceViews(Context, _StartSlot, _NumTextures, SRVs);
}

void oD3D11CommandList::SetMaterials(size_t _StartSlot, size_t _NumMaterials, const oGfxMaterial* const* _ppMaterials)
{
	const ID3D11Buffer* CBs[D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT];
	for (size_t i = 0; i < _NumMaterials; i++)
		CBs[i] = const_cast<ID3D11Buffer*>(static_cast<const oD3D11Material*>(_ppMaterials[i])->Constants.c_ptr());

	oD3D11SetConstantBuffers(Context, _StartSlot, _NumMaterials, CBs);
}
#endif
static ID3D11Resource* GetResourceBuffer(oGfxResource* _pResource, size_t _SubresourceIndex, UINT* _pD3DSubresourceIndex, size_t _NewCount = oInvalid)
{
	switch (_pResource->GetType())
	{
		//case oGfxResource::INSTANCELIST:
		//{
		//	oD3D11InstanceList* il = static_cast<oD3D11InstanceList*>(_pResource);
		//	if (_NewCount != oInvalid)
		//	{
		//		threadsafe oD3D11InstanceList::DESC* pDesc = il->GetDirectDesc();
		//		oSWAP(&pDesc->NumInstances, oSize32(_NewCount));
		//	}
		//	return il->Instances.c_ptr();
		//}

		//case oGfxResource::LINELIST:
		//{
		//	oD3D11LineList* ll = static_cast<oD3D11LineList*>(_pResource);
		//	if (_NewCount != oINVALID)
		//	{
		//		threadsafe oD3D11LineList::DESC* pDesc = ll->GetDirectDesc();
		//		oSWAP(&pDesc->NumLines, oSize32(_NewCount));
		//	}
		//	return ll->Lines.c_ptr();
		//}

		//case oGfxResource::TEXTURE: return static_cast<oD3D11Texture*>(_pResource)->Texture.c_ptr();
		//case oGfxResource::MATERIAL: return static_cast<oD3D11Material*>(_pResource)->Constants.c_ptr();
		case oGfxResource::MESH:
		{
			switch (_SubresourceIndex)
			{
				case oGfxMesh::RANGES: oASSERT(0, "Ranges are not an ID3D11Buffer");
				case oGfxMesh::INDICES: *_pD3DSubresourceIndex = 0; return static_cast<oD3D11Mesh*>(_pResource)->Indices.c_ptr();
				case oGfxMesh::VERTICES0: *_pD3DSubresourceIndex = 0; return static_cast<oD3D11Mesh*>(_pResource)->Vertices[0].c_ptr();
				case oGfxMesh::VERTICES1: *_pD3DSubresourceIndex = 0; return static_cast<oD3D11Mesh*>(_pResource)->Vertices[1].c_ptr();
				case oGfxMesh::VERTICES2: *_pD3DSubresourceIndex = 0; return static_cast<oD3D11Mesh*>(_pResource)->Vertices[2].c_ptr();
				oNODEFAULT;
			}
		}
		oNODEFAULT;
	}
}

void oD3D11CommandList::Map(oGfxResource* _pResource, size_t _SubresourceIndex, MAPPED* _pMapped)
{
	if (_pResource->GetType() == oGfxResource::MESH && _SubresourceIndex == oGfxMesh::RANGES)
	{
		_pMapped->pData = static_cast<oD3D11Mesh*>(_pResource)->LockRanges();
		_pMapped->RowPitch = sizeof(oGfxMesh::RANGE);
		_pMapped->SlicePitch = 0;
	}

	else
	{
		D3D11_MAPPED_SUBRESOURCE mapped;
		UINT D3D11SubresourceIndex = 0;
		ID3D11Resource* pD3D11Resource = GetResourceBuffer(_pResource, _SubresourceIndex, &D3D11SubresourceIndex);
		oV(Context->Map(pD3D11Resource, D3D11SubresourceIndex, D3D11_MAP_WRITE_DISCARD, 0, &mapped));
		_pMapped->pData = mapped.pData;
		_pMapped->RowPitch = mapped.RowPitch;
		_pMapped->SlicePitch = mapped.DepthPitch;
	}
}

void oD3D11CommandList::Unmap(oGfxResource* _pResource, size_t _SubresourceIndex, size_t _NewCount)
{
	if (_pResource->GetType() == oGfxResource::MESH && _SubresourceIndex == oGfxMesh::RANGES)
		static_cast<oD3D11Mesh*>(_pResource)->UnlockRanges();
	else
	{
		UINT D3D11SubresourceIndex = 0;
		ID3D11Resource* pD3D11Resource = GetResourceBuffer(_pResource, _SubresourceIndex, &D3D11SubresourceIndex);
		Context->Unmap(pD3D11Resource, D3D11SubresourceIndex);
	}
}

void oD3D11CommandList::Clear(CLEAR_TYPE _ClearType)
{
	oASSERT(pRenderTarget, "No oGfxRenderTarget specified for %s %s", typeid(*this), GetName());
	oD3D11RenderTarget* pRT = static_cast<oD3D11RenderTarget*>(pRenderTarget);

	if (_ClearType >= COLOR)
	{
		FLOAT c[4];
		for (int i = 0; i < pRenderTarget->Desc.ArraySize; i++)
		{
			oColorDecompose(pRenderTarget->Desc.ClearDesc.ClearColor[i], c);
			Context->ClearRenderTargetView(pRenderTarget->RTVs[i], c);
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

	if (pRenderTarget->DSV && _ClearType != COLOR)
		Context->ClearDepthStencilView(pRenderTarget->DSV, sClearFlags[_ClearType], pRenderTarget->Desc.ClearDesc.DepthClearValue, pRenderTarget->Desc.ClearDesc.StencilClearValue);
}
#if 0
void oD3D11CommandList::DrawMesh(float4x4& _Transform, uint _MeshID, const oGfxMesh* _pMesh, size_t _RangeIndex, const oGfxInstanceList* _pInstanceList)
{
	oASSERT(!_pInstanceList, "Instanced drawing not yet implemented");

	const oD3D11Mesh* M = static_cast<const oD3D11Mesh*>(_pMesh);

	uint StartIndex = 0;
	uint NumTriangles = 0;
	uint MinVertex = 0;

	if (_RangeIndex == ~0u)
	{
		oD3D_BUFFER_TOPOLOGY t;
		oD3D11GetBufferDescription(M->Indices, &t);
		NumTriangles = t.ElementCount / 3;
	}

	else
	{
		oASSERT(_RangeIndex < M->Ranges.size(), "");
		const oGfxMesh::RANGE& r = M->Ranges[_RangeIndex];
		StartIndex = r.StartTriangle * 3;
		NumTriangles = r.NumTriangles;
		MinVertex = r.MinVertex;
	}

	#ifdef _DEBUG
	{
		oRef<ID3D11InputLayout> InputLayout = 0;
		Context->IAGetInputLayout(&InputLayout);
		oASSERT(InputLayout, "No InputLayout specified");
	}
	#endif
	oASSERT(M->Vertices[0], "No geometry vertices specified");

	oGfxMesh::DESC desc;
	M->GetDesc(&desc);

	const ID3D11Buffer* pVertices[D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT];
	UINT Strides[D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT];
	size_t nVertexBuffers = 0;
	for (size_t i = 0; i < oCOUNTOF(M->Vertices); i++)
	{
		if (M->Vertices)
		{
			pVertices[nVertexBuffers] = M->Vertices[i];
			Strides[nVertexBuffers] = desc.VertexByteSize[i];
			nVertexBuffers++;
		}
	}
	
	D3D11_PRIMITIVE_TOPOLOGY topology;
	Context->IAGetPrimitiveTopology(&topology);

	oD3D11Draw(Context
		, oD3D11GetNumElements(topology, NumTriangles)
		, nVertexBuffers
		, pVertices
		, Strides
		, MinVertex
		, 0
		, M->Indices.c_ptr()
		, true
		, StartIndex);
}

void oD3D11CommandList::DrawLines(uint _LineListID, const oGfxLineList* _pLineList)
{
	// Set up draw buffer

	struct REAL_LINE_VERTEX
	{
		float3 Position;
		oColor Color;
	};

	oGfxLineList::DESC d;
	_pLineList->GetDesc(&d);

	const ID3D11Buffer* pLines = static_cast<const oD3D11LineList*>(_pLineList)->Lines;
	UINT VertexStride = sizeof(REAL_LINE_VERTEX);
	
	oD3D11Draw(Context
		, D3D11_PRIMITIVE_TOPOLOGY_LINELIST
		, d.NumLines
		, 1
		, &pLines
		, &VertexStride
		, 0
		, 0
		, 0
		, false
		, 0);
}

void oD3D11CommandList::DrawQuad(float4x4& _Transform, uint _QuadID)
{
	oD3D11DrawSVQuad(Context);
}

#endif