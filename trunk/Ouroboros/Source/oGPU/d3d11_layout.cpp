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
#include "d3d11_layout.h"
#include <oGPU/oGPU.h>
#include "oCore/Windows/win_util.h"
#include "d3d11_util.h"
#include "d3d_compile.h"

#define oD3D_EL(_Semantic, _SemanticIndex, _Format, _InputSlot) { _Semantic, _SemanticIndex, _Format, _InputSlot, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
#define oD3D_NULL oD3D_EL(0, 0, DXGI_FORMAT_UNKNOWN, 0)
#define oD3D_POSITION32(_SemanticIndex, _InputSlot) oD3D_EL("POSITION", _SemanticIndex, DXGI_FORMAT_R32G32B32_FLOAT, _InputSlot)
#define oD3D_POSITION16(_SemanticIndex, _InputSlot) oD3D_EL("POSITION", _SemanticIndex, DXGI_FORMAT_R16G16B16A16_UNORM, _InputSlot)
#define oD3D_NORMAL32(_SemanticIndex, _InputSlot) oD3D_EL("NORMAL", _SemanticIndex, DXGI_FORMAT_R32G32B32_FLOAT, _InputSlot)
#define oD3D_NORMAL10(_SemanticIndex, _InputSlot) oD3D_EL("NORMAL", _SemanticIndex, DXGI_FORMAT_R11G11B10_FLOAT, _InputSlot)
#define oD3D_TANGENT32(_SemanticIndex, _InputSlot) oD3D_EL("TANGENT", _SemanticIndex, DXGI_FORMAT_R32G32B32A32_FLOAT, _InputSlot)
#define oD3D_TANGENT10(_SemanticIndex, _InputSlot) oD3D_EL("TANGENT", _SemanticIndex, DXGI_FORMAT_R10G10B10A2_UNORM, _InputSlot)
#define oD3D_UV32(_SemanticIndex, _InputSlot) oD3D_EL("TEXCOORD", _SemanticIndex, DXGI_FORMAT_R32G32_FLOAT, _InputSlot) 
#define oD3D_UVW32(_SemanticIndex, _InputSlot) oD3D_EL("TEXCOORD", _SemanticIndex, DXGI_FORMAT_R32G32B32_FLOAT, _InputSlot) 
#define oD3D_UVWX32(_SemanticIndex, _InputSlot) oD3D_EL("TEXCOORD", _SemanticIndex, DXGI_FORMAT_R32G32B32A32_FLOAT, _InputSlot) 
#define oD3D_UV16(_SemanticIndex, _InputSlot) oD3D_EL("TEXCOORD", _SemanticIndex, DXGI_FORMAT_R16G16_FLOAT, _InputSlot) 
#define oD3D_UVWX16(_SemanticIndex, _InputSlot) oD3D_EL("TEXCOORD", _SemanticIndex, DXGI_FORMAT_R16G16B16A16_FLOAT, _InputSlot) 
#define oD3D_COLOR32(_SemanticIndex, _InputSlot) oD3D_EL("COLOR", _SemanticIndex, DXGI_FORMAT_R32G32B32A32_FLOAT, _InputSlot) 
#define oD3D_COLOR8(_SemanticIndex, _InputSlot) oD3D_EL("COLOR", _SemanticIndex, DXGI_FORMAT_R8G8B8A8_UNORM, _InputSlot) 

namespace ouro {
	namespace gpu {
		namespace d3d11 {

const char* get_semantic(mesh::semantic::value& _Semantic)
{
	const char* sSemantics[] =
	{
		"POSITION",
		"NORMAL",
		"TANGENT",
		"TEXCOORD",
		"COLOR",
	};
	static_assert(oCOUNTOF(sSemantics) == mesh::semantic::count, "array mismatch");

	return sSemantics[_Semantic];
}

static const D3D11_INPUT_ELEMENT_DESC None[] = { oD3D_NULL };
static const D3D11_INPUT_ELEMENT_DESC Pos[] = { oD3D_POSITION32(0,0) };
static const D3D11_INPUT_ELEMENT_DESC Color[] = { oD3D_COLOR8(0,0) };
static const D3D11_INPUT_ELEMENT_DESC PosColor[] = { oD3D_POSITION32(0,0), oD3D_COLOR8(0,0) };
static const D3D11_INPUT_ELEMENT_DESC PosNrm[] = { oD3D_POSITION32(0,0), oD3D_NORMAL10(0,0) };
static const D3D11_INPUT_ELEMENT_DESC PosNrmTan[] = { oD3D_POSITION32(0,0), oD3D_NORMAL10(0,0), oD3D_TANGENT10(0,0) };
static const D3D11_INPUT_ELEMENT_DESC PosNrmTanUv0[] = { oD3D_POSITION32(0,0), oD3D_NORMAL10(0,0), oD3D_TANGENT10(0,0), oD3D_UV16(0,0) };
static const D3D11_INPUT_ELEMENT_DESC PosNrmTanUvwx0[] = { oD3D_POSITION32(0,0), oD3D_NORMAL10(0,0), oD3D_TANGENT10(0,0), oD3D_UVWX16(0,0) };
static const D3D11_INPUT_ELEMENT_DESC PosUv0[] = { oD3D_POSITION32(0,0), oD3D_UV16(0,0) };
static const D3D11_INPUT_ELEMENT_DESC PosUvwx0[] = { oD3D_POSITION32(0,0), oD3D_UVWX16(0,0) };
static const D3D11_INPUT_ELEMENT_DESC Uv0[] = { oD3D_UV16(0,0) };
static const D3D11_INPUT_ELEMENT_DESC Uvwx0[] = { oD3D_UVWX16(0,0) };
static const D3D11_INPUT_ELEMENT_DESC Uv0Color[] = { oD3D_UV16(0,0), oD3D_COLOR8(0,0) };
static const D3D11_INPUT_ELEMENT_DESC Uvwx0Color[] = { oD3D_UVWX16(0,0), oD3D_COLOR8(0,0) };

static const D3D11_INPUT_ELEMENT_DESC* sInputElements[] = 
{
	None,
	Pos,
	Color,
	PosColor,
	PosNrm,
	PosNrmTan,
	PosNrmTanUv0,
	PosNrmTanUvwx0,
	PosUv0,
	PosUvwx0,
	Uv0,
	Uvwx0,
	Uv0Color,
	Uvwx0Color,
};
static_assert(oCOUNTOF(sInputElements) == mesh::layout::count, "array mismatch");

static uchar sInputElementCounts[] = { 0, 1, 1, 2, 2, 3, 4, 4, 2, 2, 1, 1, 2, 2, };
static_assert(oCOUNTOF(sInputElementCounts) == mesh::layout::count, "array mismatch");

intrusive_ptr<ID3D11InputLayout> make_input_layout(ID3D11Device* _pDevice, const void* _pVertexShaderByteCode, const mesh::layout_array& _VertexLayouts)
{
	D3D11_INPUT_ELEMENT_DESC Elements[16];
	memset(Elements, 0, sizeof(Elements));
	uint Offset = 0;

	// copy pieces into one array for input layout definition
	for (uint L = 0; L < mesh::usage::count; L++)
	{
		mesh::layout::value Layout = _VertexLayouts[L];
		if (Layout == mesh::layout::none)
			continue;

		uint nElements = sInputElementCounts[Layout];
		oCHECK((Offset + nElements) < oCOUNTOF(Elements), "too many vertex buffers");
		memcpy(Elements, sInputElements[Layout], sizeof(D3D11_INPUT_ELEMENT_DESC) * nElements);
		
		// patch input slot
		for (uint i = 0; i < nElements; i++)
			Elements[Offset+i].InputSlot = L;
		
		Offset += nElements;
	}

	intrusive_ptr<ID3D11InputLayout> input_layout;
	oV(_pDevice->CreateInputLayout(Elements, Offset, _pVertexShaderByteCode, d3d::byte_code_size(_pVertexShaderByteCode), &input_layout));
	return input_layout;
}

		} // namespace d3d11
	} // namespace gpu
} // namespace ouro
