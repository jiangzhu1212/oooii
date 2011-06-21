// $(header)
#include "oD3D11Pipeline.h"
#include "oD3D11Device.h"
#include <oooii/oSurface.h>

oDEFINE_GFXDEVICE_CREATE(oD3D11, Pipeline);
oBEGIN_DEFINE_GFXDEVICECHILD_CTOR(oD3D11, Pipeline)
{
	oD3D11DEVICE();
	if (_Desc.pInputLayout && _Desc.InputLayoutByteWidth && _Desc.pVSByteCode)
		oV(D3DDevice->CreateInputLayout(static_cast<const D3D11_INPUT_ELEMENT_DESC*>(_Desc.pInputLayout), static_cast<UINT>(_Desc.InputLayoutByteWidth / sizeof(D3D11_INPUT_ELEMENT_DESC)), _Desc.pVSByteCode, oD3D11GetEncodedByteCodeSize(_Desc.pVSByteCode), &InputLayout));
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
