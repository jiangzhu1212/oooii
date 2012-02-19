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
#include "oD3D11Pipeline.h"
#include "oD3D11Device.h"
#include <oPlatform/oDXGI.h>

static void oInitializeInputElementDesc(D3D11_INPUT_ELEMENT_DESC* _pInputElementDescs, size_t _MaxNumInputElementDescs, const oIAELEMENT* _pVertexElements, size_t _NumVertexElements)
{
	oASSERT(_MaxNumInputElementDescs >= _NumVertexElements, "");
	for (size_t i = 0; i < _NumVertexElements; i++)
	{
		D3D11_INPUT_ELEMENT_DESC& el = _pInputElementDescs[i];
		const oIAELEMENT& e = _pVertexElements[i];

		el.SemanticName = e.Name;
		el.SemanticIndex = e.Index;
		el.Format = oDXGIFromSurfaceFormat(e.Format);
		el.InputSlot = e.InputSlot;
		el.AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
		el.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
		el.InstanceDataStepRate = 0;
	}
}

oDEFINE_GFXDEVICE_CREATE(oD3D11, Pipeline);
oBEGIN_DEFINE_GFXDEVICECHILD_CTOR(oD3D11, Pipeline)
{
	*_pSuccess = false;

	oASSERT(_Desc.NumElements > 0, "At least one vertex element must be specified");
	NumElements = _Desc.NumElements;
	pElements = new oIAELEMENT[NumElements];
	memcpy(pElements, _Desc.pElements, sizeof(oIAELEMENT) * NumElements);

	D3D11_INPUT_ELEMENT_DESC Elements[32];
	oInitializeInputElementDesc(Elements, oCOUNTOF(Elements), pElements, NumElements);

	oD3D11DEVICE();
	oV(D3DDevice->CreateInputLayout(Elements, NumElements, _Desc.pVSByteCode, oD3D11GetEncodedByteCodeSize(_Desc.pVSByteCode), &InputLayout));

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

	*_pSuccess = true;
}

oD3D11Pipeline::~oD3D11Pipeline()
{
	if (pElements)
		delete [] pElements;
}

void oD3D11Pipeline::GetDesc(DESC* _pDesc) const threadsafe
{
	memset(_pDesc, 0, sizeof(DESC));
	_pDesc->pElements = pElements;
	_pDesc->NumElements = NumElements;
}
