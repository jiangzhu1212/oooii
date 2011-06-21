// $(header)
#include "oD3D11CommandList.h"
#include "oD3D11Device.h"
#include "oD3D11LineList.h"
#include "oD3D11Material.h"
#include "oD3D11Mesh.h"
#include "oD3D11Texture.h"

oDEFINE_GFXDEVICE_CREATE(oD3D11, CommandList);
oBEGIN_DEFINE_GFXDEVICECHILD_CTOR(oD3D11, CommandList)
{
	*_pSuccess = false;
	oD3D11DEVICE();

	// Cache pointers to often-used state locally
	oD3D11Device* pDevice = thread_cast<oD3D11Device*>(static_cast<threadsafe oD3D11Device*>(Device.c_ptr())); // Safe because we're read-only accessing pointer values
	pOMState = &pDevice->OMState;
	pRSState = &pDevice->RSState;
	pDSState = &pDevice->DSState;
	pSAState = &pDevice->SAState;

	HRESULT hr = D3DDevice->CreateDeferredContext(0, &Context);
	if (FAILED(hr))
	{
		char err[128];
		sprintf_s(err, "Failed to create oGfxDeviceContext %u: ", _Desc.DrawOrder);
		oSetLastErrorNative(hr, err);
		return;
	}

	static_cast<threadsafe oD3D11Device*>(Device.c_ptr())->Insert(this);

	*_pSuccess = true;
}

oD3D11CommandList::~oD3D11CommandList()
{
	static_cast<threadsafe oD3D11Device*>(Device.c_ptr())->Remove(this);
}

void oD3D11CommandList::Begin(
	const float4x4& View
	, const float4x4& Projection
	, const oGfxPipeline* _pPipeline
	, const oGfxRenderTarget2* _pRenderTarget
	, size_t _NumViewports
	, const VIEWPORT* _pViewports)
{
}

void oD3D11CommandList::End()
{
	Context->FinishCommandList(FALSE, &CommandList);
}

void oD3D11CommandList::RSSetState(oRSSTATE _State)
{
	// @oooii-tony: TODO: Move oD3D11BlendState's enum to be oRS_STATE
	oASSERT(0, "Enums are not current compatible");
	//oSTATICASSERT(oOM_COUNT == oD3D11BlendState::NUM_STATES);
	pOMState->SetState(Context, (oD3D11BlendState::STATE)_State);
}

void oD3D11CommandList::OMSetState(oOMSTATE _State)
{
	pOMState->SetState(Context, (oD3D11BlendState::STATE)_State);
}

void oD3D11CommandList::DSSetState(oDSSTATE _State)
{
	pDSState->SetState(Context, (oD3D11DepthStencilState::STATE)_State);
}

void oD3D11CommandList::SASetStates(size_t _StartSlot, size_t _NumStates, const oSASTATE* _pSAStates, const oMBSTATE* _pMBStates)
{
	pSAState->SetState(Context, _StartSlot, reinterpret_cast<const oD3D11SamplerState::SAMPLER_STATE*>(_pSAStates), reinterpret_cast<const oD3D11SamplerState::MIP_BIAS_LEVEL*>(_pMBStates), _NumStates);
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

static ID3D11Resource* GetResourceBuffer(oGfxResource* _pResource, size_t _SubresourceIndex)
{
	switch (_pResource->GetType())
	{
		case oGfxResource::TEXTURE: return static_cast<oD3D11Texture*>(_pResource)->Texture.c_ptr();
		case oGfxResource::MATERIAL: return static_cast<oD3D11Material*>(_pResource)->Constants.c_ptr();
		case oGfxResource::LINELIST: return static_cast<oD3D11LineList*>(_pResource)->Lines.c_ptr();
		case oGfxResource::MESH:
		{
			switch (_SubresourceIndex)
			{
				case oGfxMesh::RANGES: oASSERT(0, "Ranges are not an ID3D11Buffer");
				case oGfxMesh::INDICES: return static_cast<oD3D11Mesh*>(_pResource)->Indices.c_ptr();
				case oGfxMesh::VERTICES: return static_cast<oD3D11Mesh*>(_pResource)->Vertices.c_ptr();
				case oGfxMesh::SKINNING: return static_cast<oD3D11Mesh*>(_pResource)->Skinning.c_ptr();
				default: oASSUME(0);
			}
		}

		default: oASSUME(0);
	}
}

void oD3D11CommandList::Map(oGfxResource* _pResource, size_t _SubresourceIndex, MAPPING* _pMapping)
{
	D3D11_MAPPED_SUBRESOURCE mapped;
	oV(Context->Map(GetResourceBuffer(_pResource, _SubresourceIndex), static_cast<UINT>(_SubresourceIndex), D3D11_MAP_WRITE_DISCARD, 0, &mapped));
	_pMapping->pData = mapped.pData;
	_pMapping->RowPitch = mapped.RowPitch;
	_pMapping->SlicePitch = mapped.DepthPitch;
}

void oD3D11CommandList::Unmap(oGfxResource* _pResource, size_t _SubresourceIndex)
{
	Context->Unmap(GetResourceBuffer(_pResource, _SubresourceIndex), static_cast<UINT>(_SubresourceIndex));
}

void oD3D11CommandList::Clear(CLEAR_TYPE _ClearType)
{
	oASSERT(pRenderTarget, "No oGfxRenderTarget specified for %s %s", typeid(*this), GetName());
	oD3D11RenderTarget2* pRT = static_cast<oD3D11RenderTarget2*>(pRenderTarget);

	if (_ClearType >= COLOR)
	{
		for (uint i = 0; i < pRT->Desc.ArraySize; i++)
		{
			FLOAT c[4];
			oDecomposeColor(pRT->Desc.ClearDesc.ClearColor[i], c);
			Context->ClearRenderTargetView(pRT->RTVs[i], c);
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

	if (pRT->DSV && _ClearType != COLOR)
		Context->ClearDepthStencilView(pRT->DSV, sClearFlags[_ClearType], pRT->Desc.ClearDesc.DepthClearValue, pRT->Desc.ClearDesc.StencilClearValue);
}

void oD3D11CommandList::Draw(float4x4& _Transform, uint _MeshID, const oGfxMesh* _pMesh, size_t _RangeIndex)
{
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
	oASSERT(M->Vertices, "No geometry vertices specified");

	oD3D_BUFFER_TOPOLOGY VertTopo;
	oD3D11GetBufferDescription(M->Vertices, &VertTopo);

	const ID3D11Buffer* pVertices = M->Vertices;
	UINT VertexStride = VertTopo.ElementStride;

	D3D11_PRIMITIVE_TOPOLOGY topology;
	Context->IAGetPrimitiveTopology(&topology);

	oD3D11Draw(Context
		, oD3D11GetNumElements(topology, NumTriangles)
		, 1
		, &pVertices
		, &VertexStride
		, MinVertex
		, 0
		, M->Indices.c_ptr()
		, true
		, StartIndex);
}

void oD3D11CommandList::Draw(uint _LineListID, const oGfxLineList* _pLineList)
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

void oD3D11CommandList::DrawQuad(float4x4& _Transform, uint _MeshID)
{
	oD3D11DrawSVQuad(Context);
}
