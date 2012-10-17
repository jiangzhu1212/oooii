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
#include <oPlatform/Windows/oDXGI.h>

static bool oInitializeInputElementDesc(D3D11_INPUT_ELEMENT_DESC* _pInputElementDescs, size_t _MaxNumInputElementDescs, const oGPU_VERTEX_ELEMENT* _pVertexElements, size_t _NumVertexElements)
{
	oASSERT(_MaxNumInputElementDescs >= _NumVertexElements, "");
	for (size_t i = 0; i < _NumVertexElements; i++)
	{
		D3D11_INPUT_ELEMENT_DESC& el = _pInputElementDescs[i];
		const oGPU_VERTEX_ELEMENT& e = _pVertexElements[i];

		el.SemanticName = oD3D11AsSemantic(e.Semantic);

		if (!oStrncmp(el.SemanticName, "Unrecog", 7))
		{
			char fcc[5];
			return oErrorSetLast(oERROR_INVALID_PARAMETER, "Bad semantic fourcc: %s on %d%s element", oToString(fcc, e.Semantic), i, oOrdinal(oInt(i)));
		}

		el.SemanticIndex = oGPUGetSemanticIndex(e.Semantic);
		el.Format = oDXGIFromSurfaceFormat(e.Format);
		el.InputSlot = e.InputSlot;
		el.AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
		el.InputSlotClass = e.Instanced ? D3D11_INPUT_PER_INSTANCE_DATA : D3D11_INPUT_PER_VERTEX_DATA;
		el.InstanceDataStepRate = e.Instanced ? 1 : 0;
	}

	return true;
}

const oGUID& oGetGUID(threadsafe const oD3D11Pipeline* threadsafe const *)
{
	// {772E2A04-4C2D-447A-8DA8-91F258EFA68C}
	static const oGUID oIID_D3D11Pipeline = { 0x772e2a04, 0x4c2d, 0x447a, { 0x8d, 0xa8, 0x91, 0xf2, 0x58, 0xef, 0xa6, 0x8c } };
	return oIID_D3D11Pipeline;
}

oDEFINE_GPUDEVICE_CREATE(oD3D11, Pipeline);
oBEGIN_DEFINE_GPUDEVICECHILD_CTOR(oD3D11, Pipeline)
	, DebugName(_Desc.DebugName)
{
	*_pSuccess = false;

	D3D11_INPUT_ELEMENT_DESC Elements[D3D11_VS_INPUT_REGISTER_COUNT];
	if (_Desc.NumElements)
	{
		oASSERT(_Desc.NumElements > 0, "At least one vertex element must be specified");
		NumElements = _Desc.NumElements;
		pElements = new oGPU_VERTEX_ELEMENT[NumElements];
		memcpy(pElements, _Desc.pElements, sizeof(oGPU_VERTEX_ELEMENT) * NumElements);

		if (!oInitializeInputElementDesc(Elements, oCOUNTOF(Elements), pElements, NumElements))
			return; // pass through error
	}
	else
	{
		NumElements = 0;
		pElements = nullptr;
	}

	oD3D11DEVICE();
	if (_Desc.NumElements)
	{
		oV(D3DDevice->CreateInputLayout(Elements, NumElements, _Desc.pVertexShader, oHLSLGetByteCodeSize(_Desc.pVertexShader), &InputLayout));
	}

	if (_Desc.pVertexShader)
	{
		oV(D3DDevice->CreateVertexShader(_Desc.pVertexShader, oHLSLGetByteCodeSize(_Desc.pVertexShader), 0, &VertexShader));
		oVERIFY(oD3D11SetDebugName(VertexShader, _Desc.DebugName));
	}

	if (_Desc.pHullShader)
	{
		oV(D3DDevice->CreateHullShader(_Desc.pHullShader, oHLSLGetByteCodeSize(_Desc.pHullShader), 0, &HullShader));
		oVERIFY(oD3D11SetDebugName(HullShader, _Desc.DebugName));
	}

	if (_Desc.pDomainShader)
	{
		oV(D3DDevice->CreateDomainShader(_Desc.pDomainShader, oHLSLGetByteCodeSize(_Desc.pDomainShader), 0, &DomainShader));
		oVERIFY(oD3D11SetDebugName(DomainShader, _Desc.DebugName));
	}

	if (_Desc.pGeometryShader)
	{
		oV(D3DDevice->CreateGeometryShader(_Desc.pGeometryShader, oHLSLGetByteCodeSize(_Desc.pGeometryShader), 0, &GeometryShader));
		oVERIFY(oD3D11SetDebugName(GeometryShader, _Desc.DebugName));
	}

	if (_Desc.pPixelShader)
	{
		oV(D3DDevice->CreatePixelShader(_Desc.pPixelShader, oHLSLGetByteCodeSize(_Desc.pPixelShader), 0, &PixelShader));
		oVERIFY(oD3D11SetDebugName(PixelShader, _Desc.DebugName));
	}

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
	_pDesc->DebugName = DebugName;
	_pDesc->pElements = pElements;
	_pDesc->NumElements = NumElements;
}
