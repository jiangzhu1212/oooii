// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.
#include <oGPU/shaders.h>

typedef unsigned char BYTE;

#include <VSPassThroughPos.h>
#include <VSPassThroughPosColor.h>
#include <VSPassThroughPosUv.h>
#include <VSPassThroughPosUvw.h>

#include <VSTexture1D.h>
#include <VSTexture1DArray.h>
#include <VSTexture2D.h>
#include <VSTexture2DArray.h>
#include <VSTexture3D.h>
#include <VSTextureCube.h>
#include <VSTextureCubeArray.h>
#include <VSTrivialPos.h>
#include <VSTrivialPosColor.h>
#include <VSTrivialPosColorInstanced.h>
#include <VSFullscreenTri.h>
#include <VSGpuTestBuffer.h>

#include <GSVertexNormals.h>
#include <GSVertexTangents.h>

#include <PSBlack.h>
#include <PSWhite.h>
#include <PSRed.h>
#include <PSGreen.h>
#include <PSBlue.h>
#include <PSYellow.h>
#include <PSMagenta.h>
#include <PSCyan.h>
#include <PSVertexColor.h>
#include <PSTexcoord.h>

#include <PSTexture1D.h>
#include <PSTexture1DArray.h>
#include <PSTexture2D.h>
#include <PSTexture2DArray.h>
#include <PSTexture3D.h>
#include <PSTextureCube.h>
#include <PSTextureCubeArray.h>
#include <PSGpuTestBuffer.h>

#include <CSNoop.h>

static_assert((sizeof(oGpuTrivialDrawConstants) % 16) == 0, "shader struct must be 16-byte aligned");
static_assert((sizeof(oGpuTrivialInstanceConstants) % 16) == 0, "shader struct must be 16-byte aligned");

struct SHADER
{
	const char* name;
	const void* byte_code;
};

#define oSH(x) { #x, x },
#define oBYTE_CODE(type) const void* byte_code(const type::value& shader) { static_assert(oCOUNTOF(s_##type) == type::count, "array mismatch"); return s_##type[shader].byte_code; }
#define oAS_STRING(type) const char* as_string(const gpu::intrinsic::type::value& shader) { return gpu::intrinsic::s_##type[shader].name; }

using namespace ouro::mesh;

namespace ouro { namespace gpu { namespace intrinsic {

mesh::element_array elements(const vertex_layout::value& input)
{
	element_array e;
	switch (input)
	{
		case vertex_layout::none:
			break;
		case vertex_layout::pos:
			e[0] = element(surface::semantic::vertex_position, 0, surface::format::r32g32b32_float, 0);
			break;
		case vertex_layout::pos_color:
			e[0] = element(surface::semantic::vertex_position, 0, surface::format::r32g32b32_float, 0);
			e[1] = element(surface::semantic::vertex_color, 0, surface::format::b8g8r8a8_unorm, 0);
			break;
		case vertex_layout::pos_uv:
			e[0] = element(surface::semantic::vertex_position, 0, surface::format::r32g32b32_float, 0);
			e[1] = element(surface::semantic::vertex_texcoord, 0, surface::format::r32g32_float, 0);
			break;
		case vertex_layout::pos_uvw:
			e[0] = element(surface::semantic::vertex_position, 0, surface::format::r32g32b32_float, 0);
			e[1] = element(surface::semantic::vertex_texcoord, 0, surface::format::r32g32b32_float, 0);
			break;
		default:
			break;
	}

	return e;
}

const void* vs_byte_code(const vertex_layout::value& input)
{
	static const vertex_shader::value sVS[] =
	{
		vertex_shader::fullscreen_tri,
		vertex_shader::pass_through_pos,
		vertex_shader::pass_through_pos_color,
		vertex_shader::pass_through_pos_uv,
		vertex_shader::pass_through_pos_uvw,
	};
	static_assert(oCOUNTOF(sVS) == vertex_layout::count, "array mismatch");
	return byte_code(sVS[input]);
}

static const SHADER s_vertex_shader[] = 
{
	oSH(VSPassThroughPos)
	oSH(VSPassThroughPosColor)
	oSH(VSPassThroughPosUv)
	oSH(VSPassThroughPosUvw)
	oSH(VSTexture1D)
	oSH(VSTexture1DArray)
	oSH(VSTexture2D)
	oSH(VSTexture2DArray)
	oSH(VSTexture3D)
	oSH(VSTextureCube)
	oSH(VSTextureCubeArray)
	oSH(VSTrivialPos)
	oSH(VSTrivialPosColor)
	oSH(VSTrivialPosColorInstanced)
	oSH(VSFullScreenTri)
	oSH(VSGpuTestBuffer)
};

static const SHADER s_hull_shader[] = 
{
	oSH(nullptr)
};

static const SHADER s_domain_shader[] = 
{
	oSH(nullptr)
};

static const SHADER s_geometry_shader[] = 
{
	oSH(GSVertexNormals)
	oSH(GSVertexTangents)
};

static const SHADER s_pixel_shader[] = 
{
	oSH(PSBlack)
	oSH(PSWhite)
	oSH(PSRed)
	oSH(PSGreen)
	oSH(PSBlue)
	oSH(PSYellow)
	oSH(PSMagenta)
	oSH(PSCyan)
	oSH(PSVertexColor)
	oSH(PSTexcoord)

	oSH(PSTexture1D)
	oSH(PSTexture1DArray)
	oSH(PSTexture2D)
	oSH(PSTexture2DArray)
	oSH(PSTexture3D)
	oSH(PSTextureCube)
	oSH(PSTextureCubeArray)

	oSH(PSGpuTestBuffer)
};

static const SHADER s_compute_shader[] = 
{
	oSH(CSNoop)
};

oBYTE_CODE(vertex_shader)
oBYTE_CODE(hull_shader)
oBYTE_CODE(domain_shader)
oBYTE_CODE(geometry_shader)
oBYTE_CODE(pixel_shader)
oBYTE_CODE(compute_shader)

	}}

const char* as_string(const gpu::intrinsic::vertex_layout::value& input)
{
	static const char* sNames[] = 
	{
		"none",
		"pos",
		"pos_color",
		"pos_uv",
		"pos_uvw",
	};
	static_assert(oCOUNTOF(sNames) == gpu::intrinsic::vertex_layout::count, "array mismatch");

	return sNames[input];
}

oAS_STRING(vertex_shader)
oAS_STRING(hull_shader)
oAS_STRING(domain_shader)
oAS_STRING(geometry_shader)
oAS_STRING(pixel_shader)
oAS_STRING(compute_shader)

} // namespace ouro
