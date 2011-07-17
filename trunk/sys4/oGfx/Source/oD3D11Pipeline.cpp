// $(header)
#include "oD3D11Pipeline.h"
#include "oD3D11Device.h"
#include <oooii/oSurface.h>

static void oInitializeInputElementDesc(D3D11_INPUT_ELEMENT_DESC* _pInputElementDescs, size_t _MaxNumInputElementDescs, const oGfxPipeline::VERTEX_ATTRIBUTE* _pVertexAttributes, size_t _NumVertexAttributes)
{
	oASSERT(_MaxNumInputElementDescs >= _NumVertexAttributes, "");
	for (size_t i = 0; i < _NumVertexAttributes; i++)
	{
		D3D11_INPUT_ELEMENT_DESC& el = _pInputElementDescs[i];
		const oGfxPipeline::VERTEX_ATTRIBUTE& a = _pVertexAttributes[i];

		el.SemanticName = a.Name;
		el.SemanticIndex = a.Index;
		el.Format = oSurface::GetPlatformFormat<DXGI_FORMAT>(a.Format);
		el.InputSlot = a.InputSlot;
		el.AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
		el.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
		el.InstanceDataStepRate = 0;
	}
}

oDEFINE_GFXDEVICE_CREATE(oD3D11, Pipeline);
oBEGIN_DEFINE_GFXDEVICECHILD_CTOR(oD3D11, Pipeline)
{
	oASSERT(_Desc.NumAttributes > 0, "At least one vertex attribute must be specified");
	NumAttributes = _Desc.NumAttributes;
	pAttributes = new VERTEX_ATTRIBUTE[NumAttributes];
	memcpy(pAttributes, _Desc.pAttributes, sizeof(VERTEX_ATTRIBUTE) * NumAttributes);

	D3D11_INPUT_ELEMENT_DESC Elements[32];
	oInitializeInputElementDesc(Elements, oCOUNTOF(Elements), pAttributes, NumAttributes);

	oD3D11DEVICE();
	oV(D3DDevice->CreateInputLayout(Elements, NumAttributes, _Desc.pVSByteCode, oD3D11GetEncodedByteCodeSize(_Desc.pVSByteCode), &InputLayout));

	CREATE_SHADER(VS, VertexShader);

	if (_Desc.pVSByteCode)
		oV(D3DDevice->CreateVertexShader(_Desc.pVSByteCode, oD3D11GetEncodedByteCodeSize(_Desc.pVSByteCode), 0, &VertexShader));
	if (_Desc.pHSByteCode)
		oV(D3DDevice->CreateHullShader(_Desc.pHSByteCode, oD3D11GetEncodedByteCodeSize(_Desc.pHSByteCode), 0, &HullShader));
	if (_Desc.pDSByteCode)
		oV(D3DDevice->CreateDomainShader(_Desc.pDSByteCode, oD3D11GetEncodedByteCodeSize(_Desc.pDSByteCode), 0, &DomainShader));
	if (_Desc.pGSByteCode)
		oV(D3DDevice->CreateGeometryShader(_Desc.pGSByteCode, oD3D11GetEncodedByteCodeSize(_Desc.pGSByteCode), 0, &GeometryShader));
	if (_Desc.pPSByteCode)
		oV(D3DDevice->CreatePixelShader(_Desc.pPSByteCode, oD3D11GetEncodedByteCodeSize(_Desc.pPSByteCode), 0, &PixelShader));
}

oD3D11Pipeline::~oD3D11Pipeline()
{
	if (pAttributes)
		delete [] pAttributes;
}

void oD3D11Pipeline::GetDesc(DESC* _pDesc) const threadsafe
{
	memset(_pDesc, 0, sizeof(DESC));
	_pDesc->pAttributes = pAttributes;
	_pDesc->NumAttributes = NumAttributes;
}
