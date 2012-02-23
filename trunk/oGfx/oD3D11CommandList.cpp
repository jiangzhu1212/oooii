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
#include "oD3D11InstanceList.h"
#include "oD3D11LineList.h"
//#include "oD3D11Material.h"
#include "oD3D11Mesh.h"
#include "oD3D11Pipeline.h"
//#include "oD3D11Texture.h"
#include <oGfx/oGfxDrawConstants.h>

const oGUID& oGetGUID(threadsafe const oD3D11CommandList* threadsafe const *)
{
	// {2D6106C4-7741-41CD-93DE-2C2A9BCD9163}
	static const oGUID oIID_D3D11CommandList = { 0x2d6106c4, 0x7741, 0x41cd, { 0x93, 0xde, 0x2c, 0x2a, 0x9b, 0xcd, 0x91, 0x63 } };
	return oIID_D3D11CommandList;
}

oDEFINE_GFXDEVICE_CREATE(oD3D11, CommandList);
oBEGIN_DEFINE_GFXDEVICECHILD_CTOR(oD3D11, CommandList)
	, pRenderTarget(nullptr)
	, View(float4x4::Identity)
	, Projection(float4x4::Identity)
	, RSState(oRSFRONTFACE)
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

bool oD3D11CommandList::QueryInterface(const oGUID& _InterfaceID, threadsafe void** _ppInterface) threadsafe
{
	if (MIXINQueryInterface(_InterfaceID, _ppInterface))
		return true;

	else if (_InterfaceID == (const oGUID&)__uuidof(ID3D11DeviceContext))
	{
		Context->AddRef();
		*_ppInterface = Context;
	}

	return !!*_ppInterface;
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
	D3D11_MAPPED_SUBRESOURCE mapped;
	_pDeviceContext->Map(_pViewConstants, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
	oDeferredViewConstants* V = (oDeferredViewConstants*)mapped.pData;
	V->Set(_View, _Projection, oCastAsFloat(_RTDesc.Dimensions), static_cast<uint>(_RenderTargetIndex));
	_pDeviceContext->Unmap(_pViewConstants, 0);
	oD3D11SetConstantBuffers(_pDeviceContext, 0, 1, &_pViewConstants);
}

static void SetDrawConstants(ID3D11DeviceContext* _pDeviceContext, ID3D11Buffer* _pDrawConstants, const float4x4& _World, const float4x4& _View, const float4x4& _Projection, uint _ObjectID, uint _DrawID)
{
	D3D11_MAPPED_SUBRESOURCE mapped;
	_pDeviceContext->Map(_pDrawConstants, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
	oGfxDrawConstants* D = (oGfxDrawConstants*)mapped.pData;
	D->Set(_World, _View, _Projection, _ObjectID, _DrawID);
	_pDeviceContext->Unmap(_pDrawConstants, 0);
	oD3D11SetConstantBuffers(_pDeviceContext, 1, 1, &_pDrawConstants);
}

void oD3D11CommandList::Begin(
	const float4x4& _View
	, const float4x4& _Projection
	, oGfxRenderTarget* _pRenderTarget
	, size_t _RenderTargetIndex
	, size_t _NumViewports
	, const VIEWPORT* _pViewports)
{
	oASSERT(_pRenderTarget, "A render target must be specified");

	// Retain values used until End()
	oDEVICE_LOCK_SUBMIT();
	View = _View;
	Projection = _Projection;
	pRenderTarget = static_cast<oD3D11RenderTarget*>(_pRenderTarget);

	// Set per-commandlist state to begin rendering
	pRenderTarget->Set(Context);
	oGfxRenderTarget::DESC RTDesc;
	_pRenderTarget->GetDesc(&RTDesc);
	SetViewports(Context, RTDesc, _NumViewports, _pViewports);
	SetViewConstants(Context, D3DDevice()->ViewConstants, _View, _Projection, RTDesc, _RenderTargetIndex);
}

void oD3D11CommandList::End()
{
	pRenderTarget = nullptr;
	Context->FinishCommandList(FALSE, &CommandList);

	oDEVICE_UNLOCK_SUBMIT();
}

void oD3D11CommandList::SetPipeline(const oGfxPipeline* _pPipeline)
{
	if (_pPipeline)
	{
		oD3D11Pipeline* p = const_cast<oD3D11Pipeline*>(static_cast<const oD3D11Pipeline*>(_pPipeline));
		Context->IASetInputLayout(p->InputLayout);
		Context->VSSetShader(p->VertexShader, 0, 0);
		Context->HSSetShader(p->HullShader, 0, 0);
		Context->DSSetShader(p->DomainShader, 0, 0);
		Context->GSSetShader(p->GeometryShader, 0, 0);
		Context->PSSetShader(p->PixelShader, 0, 0);
	}

	else
	{
		Context->IASetInputLayout(nullptr);
		Context->VSSetShader(nullptr, nullptr, 0);
		Context->HSSetShader(nullptr, nullptr, 0);
		Context->DSSetShader(nullptr, nullptr, 0);
		Context->GSSetShader(nullptr, nullptr, 0);
		Context->PSSetShader(nullptr, nullptr, 0);
	}
}

void oD3D11CommandList::RSSetState(oRSSTATE _State)
{
	Context->RSSetState(D3DDevice()->RSStates[_State]);
	RSState = _State;
}

void oD3D11CommandList::OMSetState(oOMSTATE _State)
{
	static const FLOAT sBlendFactor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	Context->OMSetBlendState(D3DDevice()->OMStates[_State], sBlendFactor, 0xffffffff);
}

void oD3D11CommandList::DSSetState(oDSSTATE _State)
{
	Context->OMSetDepthStencilState(D3DDevice()->DSStates[_State], 0);
}

void oD3D11CommandList::SASetStates(size_t _StartSlot, size_t _NumStates, const oSASTATE* _pSAStates, const oMBSTATE* _pMBStates)
{
	ID3D11SamplerState* Samplers[D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT];
	oASSERT(oCOUNTOF(Samplers) > _NumStates, "Too many samplers specified");
	for (size_t i = 0; i < _NumStates; i++)
		Samplers[i] = D3DDevice()->SAStates[_pSAStates[i]][_pMBStates[i]];
	
	oD3D11SetSamplers(Context, static_cast<UINT>(_StartSlot), static_cast<UINT>(_NumStates), Samplers);
}
#if 0
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
bool oD3D11CommandList::Map(oGfxResource* _pResource, size_t _SubresourceIndex, MAPPED* _pMapped)
{
	switch (_pResource->GetType())
	{
		case oGfxResource::INSTANCELIST:return static_cast<oD3D11InstanceList*>(_pResource)->Map(Context, _pMapped);
		case oGfxResource::LINELIST:return static_cast<oD3D11LineList*>(_pResource)->Map(Context, _pMapped);
		case oGfxResource::MESH: return static_cast<oD3D11Mesh*>(_pResource)->Map(Context, _SubresourceIndex, _pMapped);
		default:
		{
			oASSERT(0, "stale code");
			//D3D11_MAPPED_SUBRESOURCE mapped;
			//UINT D3D11SubresourceIndex = 0;
			//ID3D11Resource* pD3D11Resource = GetResourceBuffer(_pResource, _SubresourceIndex, &D3D11SubresourceIndex);
			//if (!Context->Map(pD3D11Resource, D3D11SubresourceIndex, D3D11_MAP_WRITE_DISCARD, 0, &mapped))
			//	return oWinSetLastError();

			//_pMapped->pData = mapped.pData;
			//_pMapped->RowPitch = mapped.RowPitch;
			//_pMapped->SlicePitch = mapped.DepthPitch;
			return true;
		}
	}
}

void oD3D11CommandList::Unmap(oGfxResource* _pResource, size_t _SubresourceIndex, size_t _NewCount)
{
	switch (_pResource->GetType())
	{
		case oGfxResource::INSTANCELIST: static_cast<oD3D11InstanceList*>(_pResource)->Unmap(Context, _NewCount == oInvalid ? 0 : oSize32(_NewCount)); return;
		case oGfxResource::LINELIST: static_cast<oD3D11LineList*>(_pResource)->Unmap(Context, _NewCount == oInvalid ? 0 : oSize32(_NewCount)); return;
		case oGfxResource::MESH: static_cast<oD3D11Mesh*>(_pResource)->Unmap(Context, _SubresourceIndex); return;
		default:
		{
			oASSERT(0, "stale code");
			//UINT D3D11SubresourceIndex = 0;
			//ID3D11Resource* pD3D11Resource = GetResourceBuffer(_pResource, _SubresourceIndex, &D3D11SubresourceIndex, _NewCount);
			//Context->Unmap(pD3D11Resource, D3D11SubresourceIndex);
		}
	}
}

void oD3D11CommandList::Clear(CLEAR_TYPE _ClearType)
{
	oASSERT(pRenderTarget, "No oGfxRenderTarget specified for %s %s", typeid(*this).name(), GetName());
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

void oD3D11CommandList::DrawMesh(const float4x4& _Transform, uint _MeshID, const oGfxMesh* _pMesh, size_t _RangeIndex, const oGfxInstanceList* _pInstanceList)
{
	uint DrawID = D3DDevice()->IncrementDrawID();
	SetDrawConstants(Context, D3DDevice()->DrawConstants, _Transform, View, Projection, _MeshID, DrawID);

	const oD3D11Mesh* M = static_cast<const oD3D11Mesh*>(_pMesh);
	oGfxMesh::DESC desc;
	M->GetDesc(&desc);

	uint StartIndex = 0;
	uint NumTriangles = 0;
	uint MinVertex = 0;

	if (_RangeIndex == ~0u)
		NumTriangles = desc.NumIndices / 3;
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

	const ID3D11Buffer* pVertices[D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT];
	UINT Strides[D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT];
	uint nVertexBuffers = 0;
	for (uint i = 0; i < oCOUNTOF(M->Vertices); i++)
	{
		if (M->Vertices[i])
		{
			pVertices[i] = M->Vertices[i];
			Strides[i] = M->VertexStrides[i];
			nVertexBuffers = __max(nVertexBuffers, i+1);
		}
	}

	uint NumInstances = 0;
	if (_pInstanceList)
	{
		oGfxInstanceList::DESC ILDesc;
		_pInstanceList->GetDesc(&ILDesc);
		oASSERT(ILDesc.InputSlot >= nVertexBuffers, "Mesh defines vertices in the instance input slot");
		pVertices[ILDesc.InputSlot] = static_cast<const oD3D11InstanceList*>(_pInstanceList)->Instances;
		Strides[ILDesc.InputSlot] = oGfxCalcInterleavedVertexSize(ILDesc.pElements, ILDesc.NumElements, ILDesc.InputSlot);
		nVertexBuffers = __max(nVertexBuffers, ILDesc.InputSlot+1);
		NumInstances = ILDesc.NumInstances;
	}

	oD3D11Draw(Context
		, RSState >= oRSFRONTPOINTS ? D3D11_PRIMITIVE_TOPOLOGY_POINTLIST : D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST
		, NumTriangles
		, nVertexBuffers
		, pVertices
		, Strides
		, MinVertex
		, 0
		, M->Indices
		, StartIndex
		, NumInstances);
}

void oD3D11CommandList::DrawLines(const float4x4& _Transform, uint _LineListID, const oGfxLineList* _pLineList)
{
	// Set up draw buffer

	uint DrawID = D3DDevice()->IncrementDrawID();
	SetDrawConstants(Context, D3DDevice()->DrawConstants, _Transform, View, Projection, _LineListID, DrawID);

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
		, nullptr);
}
#if 0
void oD3D11CommandList::DrawQuad(float4x4& _Transform, uint _QuadID)
{
	oD3D11DrawSVQuad(Context);
}

#endif