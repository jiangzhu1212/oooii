// $(header)
#include "oD3D11CommandList.h"
#include "oD3D11Device.h"

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
	oASSERT(0, "RSSetState");
}

void oD3D11CommandList::OMSetState(oOMSTATE _State)
{
	oASSERT(0, "OMSetState");
}

void oD3D11CommandList::DSSetState(oDSSTATE _State)
{
	oASSERT(0, "DSSetState");
}

void oD3D11CommandList::SASetStates(size_t _StartSlot, size_t _NumStates, const oSASTATE* _pSAStates, const oMBSTATE* _pMBStates)
{
	oASSERT(0, "SASetStates");
}

void oD3D11CommandList::SetTextures(size_t _StartSlot, size_t _NumTextures, const oGfxTexture* const* _ppTextures)
{
	oASSERT(0, "SetTextures");
}

void oD3D11CommandList::SetMaterials(size_t _StartSlot, size_t _NumMaterials, const oGfxMaterial* const* _ppMaterials)
{
	oASSERT(0, "SetMaterials");
}

void oD3D11CommandList::Map(oGfxResource* _pResource, size_t _SubresourceIndex, MAPPING* _pMapping)
{
	oASSERT(0, "Map");
}

void oD3D11CommandList::Unmap(oGfxResource* _pResource, size_t _SubresourceIndex)
{
	oASSERT(0, "Unmap");
}

void oD3D11CommandList::Clear(CLEAR_TYPE _ClearType)
{
	oASSERT(0, "Clear");
}

void oD3D11CommandList::Draw(float4x4& _Transform, uint _MeshID, const oGfxMesh* _pMesh, size_t _SectionIndex)
{
	oASSERT(0, "Draw");
}

void oD3D11CommandList::DrawQuad(float4x4& _Transform, uint _MeshID)
{
	oASSERT(0, "DrawQuad");
}

void oD3D11CommandList::DrawLine(uint _LineID, const LINE& _Line)	
{
	oASSERT(0, "DrawLine");
}
