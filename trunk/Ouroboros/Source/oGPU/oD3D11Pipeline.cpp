/**************************************************************************
 * The MIT License                                                        *
 * Copyright (c) 2013 Antony Arciuolo.                                    *
 * arciuolo@gmail.com                                                     *
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

static bool oInitializeInputElementDesc(D3D11_INPUT_ELEMENT_DESC* _pInputElementDescs, size_t _MaxNumInputElementDescs, char* _SemanticBuffer, const oGPU_VERTEX_ELEMENT* _pVertexElements, size_t _NumVertexElements)
{
	oASSERT(_MaxNumInputElementDescs >= _NumVertexElements, "");
	char fcc[5];
	char* s = _SemanticBuffer;
	for (size_t i = 0; i < _NumVertexElements; i++)
	{
		D3D11_INPUT_ELEMENT_DESC& el = _pInputElementDescs[i];
		const oGPU_VERTEX_ELEMENT& e = _pVertexElements[i];

		if (!oGPUParseSemantic(e.Semantic, s, &el.SemanticIndex))
			return oErrorSetLast(std::errc::invalid_argument, "Invalid semantic %s", to_string(fcc, e.Semantic));

		el.SemanticName = s;
		s += strlen(s) + 1;

		el.Format = oDXGIFromSurfaceFormat(e.Format);
		el.InputSlot = e.InputSlot;
		el.AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
		el.InputSlotClass = e.Instanced ? D3D11_INPUT_PER_INSTANCE_DATA : D3D11_INPUT_PER_VERTEX_DATA;
		el.InstanceDataStepRate = e.Instanced ? 1 : 0;
	}

	return true;
}

oDEFINE_GPUDEVICE_CREATE(oD3D11, Pipeline);
oBEGIN_DEFINE_GPUDEVICECHILD_CTOR(oD3D11, Pipeline)
	, InputTopology(static_cast<D3D_PRIMITIVE_TOPOLOGY>(_Desc.InputType))
	, DebugName(_Desc.DebugName)
{
	*_pSuccess = false;
	char SemanticBuffer[D3D11_VS_INPUT_REGISTER_COUNT * 5];
	*SemanticBuffer = 0;
	D3D11_INPUT_ELEMENT_DESC Elements[D3D11_VS_INPUT_REGISTER_COUNT];
	if (_Desc.NumElements)
	{
		oASSERT(_Desc.NumElements > 0, "At least one vertex element must be specified");
		NumElements = _Desc.NumElements;
		pElements = new oGPU_VERTEX_ELEMENT[NumElements];
		memcpy(pElements, _Desc.pElements, sizeof(oGPU_VERTEX_ELEMENT) * NumElements);

		if (!oInitializeInputElementDesc(Elements, oCOUNTOF(Elements), SemanticBuffer, pElements, NumElements))
			return; // pass through error
	}
	else
	{
		NumElements = 0;
		pElements = nullptr;
	}

	// Verify input against shaders
	if ((InputTopology == D3D11_PRIMITIVE_TOPOLOGY_UNDEFINED || InputTopology < D3D11_PRIMITIVE_TOPOLOGY_1_CONTROL_POINT_PATCHLIST) && (_Desc.pHullShader || _Desc.pDomainShader))
	{
		oErrorSetLast(std::errc::invalid_argument, "%s inputs cannot have a hull or domain shader bound", ouro::as_string(_Desc.InputType));
		return;
	}

	switch (InputTopology)
	{
		case D3D11_PRIMITIVE_TOPOLOGY_UNDEFINED:
		case D3D_PRIMITIVE_TOPOLOGY_LINELIST:
		case D3D_PRIMITIVE_TOPOLOGY_LINESTRIP:
		case D3D_PRIMITIVE_TOPOLOGY_LINELIST_ADJ:
		case D3D_PRIMITIVE_TOPOLOGY_LINESTRIP_ADJ:
		{
			// HS/DS handled above in if statement

			if (_Desc.pGeometryShader)
			{
				oErrorSetLast(std::errc::invalid_argument, "%s inputs cannot have a geometry shader bound", ouro::as_string(_Desc.InputType));
				return;
			}

			break;
		}
		
		default:
			break;
	}

	oD3D11DEVICE();
	if (_Desc.NumElements)
	{
		oV(D3DDevice->CreateInputLayout(Elements, NumElements, _Desc.pVertexShader, oD3D11GetHLSLByteCodeSize(_Desc.pVertexShader), &InputLayout));
	}

	if (_Desc.pVertexShader)
	{
		oV(D3DDevice->CreateVertexShader(_Desc.pVertexShader, oD3D11GetHLSLByteCodeSize(_Desc.pVertexShader), 0, &VertexShader));
		oVERIFY(oD3D11SetDebugName(VertexShader, _Desc.DebugName));
	}

	if (_Desc.pHullShader)
	{
		oV(D3DDevice->CreateHullShader(_Desc.pHullShader, oD3D11GetHLSLByteCodeSize(_Desc.pHullShader), 0, &HullShader));
		oVERIFY(oD3D11SetDebugName(HullShader, _Desc.DebugName));
	}

	if (_Desc.pDomainShader)
	{
		oV(D3DDevice->CreateDomainShader(_Desc.pDomainShader, oD3D11GetHLSLByteCodeSize(_Desc.pDomainShader), 0, &DomainShader));
		oVERIFY(oD3D11SetDebugName(DomainShader, _Desc.DebugName));
	}

	if (_Desc.pGeometryShader)
	{
		oV(D3DDevice->CreateGeometryShader(_Desc.pGeometryShader, oD3D11GetHLSLByteCodeSize(_Desc.pGeometryShader), 0, &GeometryShader));
		oVERIFY(oD3D11SetDebugName(GeometryShader, _Desc.DebugName));
	}

	if (_Desc.pPixelShader)
	{
		oV(D3DDevice->CreatePixelShader(_Desc.pPixelShader, oD3D11GetHLSLByteCodeSize(_Desc.pPixelShader), 0, &PixelShader));
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
	_pDesc->InputType = static_cast<oGPU_PRIMITIVE_TYPE>(InputTopology);
}
