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

#include <oGPU/shader.h>
#include <oCore/windows/win_util.h>
#include "d3d_compile.h"
#include "d3d_debug.h"
#include "d3d_shader.h"
#include <string>

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

using namespace ouro::gpu::d3d;

namespace ouro {
	namespace gpu {

Device* get_device(device* dev);

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

vertex_layout* make_vertex_layout(device* dev, mesh::layout_array& layout, const void* vs_bytecode, const char* name)
{
	D3D11_INPUT_ELEMENT_DESC Elements[max_vertex_elements];
	memset(Elements, 0, sizeof(Elements));
	uint Offset = 0;

	// copy pieces into one array for input layout definition
	for (uint L = 0; L < mesh::usage::count; L++)
	{
		mesh::layout::value Layout = layout[L];
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

	ID3D11InputLayout* input_layout = nullptr;
	oV(get_device(dev)->CreateInputLayout(Elements, Offset, vs_bytecode, bytecode_size(vs_bytecode), &input_layout));
	return (vertex_layout*)input_layout;
}

vertex_shader* make_vertex_shader(device* dev, const void* bytecode, const char* debug_name) { return (vertex_shader*)d3d::make_vertex_shader(get_device(dev), bytecode, debug_name); }
hull_shader* make_hull_shader(device* dev, const void* bytecode, const char* debug_name) { return (hull_shader*)d3d::make_hull_shader(get_device(dev), bytecode, debug_name); }
domain_shader* make_domain_shader(device* dev, const void* bytecode, const char* debug_name) { return (domain_shader*)d3d::make_domain_shader(get_device(dev), bytecode, debug_name); }
geometry_shader* make_geometry_shader(device* dev, const void* bytecode, const char* debug_name) { return (geometry_shader*)d3d::make_geometry_shader(get_device(dev), bytecode, debug_name); }
pixel_shader* make_pixel_shader(device* dev, const void* bytecode, const char* debug_name) { return (pixel_shader*)d3d::make_pixel_shader(get_device(dev), bytecode, debug_name); }
compute_shader* make_compute_shader(device* dev, const void* bytecode, const char* debug_name) { return (compute_shader*)d3d::make_compute_shader(get_device(dev), bytecode, debug_name); }

void unmake_vertex_layout(vertex_layout* layout)
{
	if (layout)
		((DeviceChild*)layout)->Release();
}

void unmake_shader(shader* s)
{
	if (s)
		((DeviceChild*)s)->Release();
}

scoped_allocation compile_shader(const char* _IncludePaths
	, const char* _Defines
	, const char* _ShaderSourcePath
	, const pipeline_stage::value& _Stage
	, const char* _EntryPoint
	, const char* _ShaderSource
	, const allocator& _Allocator)
{
	std::string cmdline;
	cmdline.reserve(2048);
	cmdline = "/O3 /T ";
	cmdline += shader_profile(D3D_FEATURE_LEVEL_11_0, _Stage);
	cmdline += " /E ";
	cmdline += _EntryPoint;

	// Add defines
	cmdline += " /D oHLSL ";
	if (_Defines)
	{
		char* ctx = nullptr;
		std::string defs(_Defines);
		char* def = strtok_r((char*)defs.c_str(), ";", &ctx);
		while (def)
		{
			cmdline += " /D ";
			cmdline += def;
			def = strtok_r(nullptr, ";", &ctx);
		}
	}
	
	// Add includes
	if (_IncludePaths)
	{
		char* ctx = nullptr;
		std::string incs(_IncludePaths);
		char* inc = strtok_r((char*)incs.c_str(), ";", &ctx);
		while (inc)
		{
			cmdline += " /I ";
			cmdline += inc;
			inc = strtok_r(nullptr, ";", &ctx);
		}
	}

	return d3d::compile_shader(cmdline.c_str(), _ShaderSourcePath, _ShaderSource, _Allocator);
}

	} // namespace gpu
} // namespace ouro
