// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.
// Very simple/basic shaders for unit tests and trivial bringup.
#ifndef oHLSL
	#pragma once
#endif
#ifndef oGPU_shaders_h
#define oGPU_shaders_h

#ifndef oHLSL
#include <oBase/colors.h>
#include <oHLSL/oHLSLTypes.h>
#include <oMesh/mesh.h>
#endif
#include <oBase/quat.h>
#include <oBase/rgb.h>

// _____________________________________________________________________________
// Shaders

#ifndef oHLSL
namespace ouro { namespace gpu { namespace intrinsic {

namespace vertex_layout
{	enum value : uchar {

	none,
	
	pos,
	pos_color,
	pos_uv,
	pos_uvw,

	count,

};}

namespace vertex_shader
{	enum value : uchar {

	// passes POSITION unmodified
	pass_through_pos,
	pass_through_pos_color,
	pass_through_pos_uv,
	pass_through_pos_uvw,
	
	// simple transform, uses t0 to sample the related type of texture
	texture1d,
	texture1darray,
	texture2d,
	texture2darray,
	texture3d,
	texturecube,
	texturecubearray,

	// simple transform
	trivial_pos,
	trivial_pos_color,
	trivial_pos_color_instanced,

	// texcoords [0,0] at top left and [1,1] at bottom right. This clips 
	// to a fullscreen quad but is more efficient because it avoids the 
	// diagonal seam between two triangles where shaders need to be 
	// evaluated twice. Set vertex_layout to none to avoid driver noise.
	fullscreen_tri,

	test_buffer,

	count,

};}

namespace hull_shader
{	enum value : uchar {

	null,
	count,

};}

namespace domain_shader
{	enum value : uchar {

	null,
	count,

};}

namespace geometry_shader
{	enum value : uchar {

	vertex_normals,
	vertex_tangents,

	count,

};}

namespace pixel_shader
{	enum value : uchar {

	// solid colors
	black,
	white,
	red,
	green,
	blue,
	yellow,
	magenta,
	cyan,
	vertex_color,
	texcoord,

	// simple texture
	texture1d,
	texture1darray,
	texture2d,
	texture2darray,
	texture3d,
	texturecube,
	texturecubearray,
	
	test_buffer,

	count,

};}

namespace compute_shader
{	enum value : uchar {

	noop,

	count,

};}

// _____________________________________________________________________________
// Accessors

// returns the elements of input
mesh::element_array elements(const vertex_layout::value& input);

// returns the vertex shader byte code with the same input
// signature as input.
const void* vs_byte_code(const vertex_layout::value& input);

// returns the buffer of bytecode compiled during executable compilation time
// (not runtime compilation)
const void* byte_code(const vertex_shader::value& shader);
const void* byte_code(const hull_shader::value& shader);
const void* byte_code(const domain_shader::value& shader);
const void* byte_code(const geometry_shader::value& shader);
const void* byte_code(const pixel_shader::value& shader);
const void* byte_code(const compute_shader::value& shader);

}}}
#endif

// _____________________________________________________________________________
// Shader constant buffer structs and registers

#define oGPU_TRIVIAL_DRAW_CONSTANTS_SLOT 0
#define oGPU_TRIVIAL_INSTANCE_CONSTANTS_SLOT 1
#define oGPU_TRIVIAL_NUM_INSTANCES 64

struct oGpuTrivialDrawConstants
{
	#ifndef oHLSL
	oGpuTrivialDrawConstants(const float4x4& world, const float4x4& view, const float4x4& projection, uint _slice = 0, const rgbaf& _color = rgbaf(1.0f, 1.0f,1.0f,1.0f))
		: wvp(world * view * projection)
		, slice(_slice)
		, padA(0)
		, padB(0)
		, padC(0)
		, color(_color)
	{}
	oGpuTrivialDrawConstants()
		: wvp(float4(1.0f,0.0f,0.0f,0.0f), float4(0.0f,1.0f,0.0f,0.0f), float4(0.0f,0.0f,1.0f,0.0f), float4(0.0f,0.0f,0.0f,1.0f))
		, slice(0)
		, padA(0)
		, padB(0)
		, padC(0)
		, color(ouro::white)
	{}
	#endif

	// when using instances use an identity world matrix - wvp is
	// still used for view and projection
	float4x4 wvp;

	uint slice;
	uint padA;
	uint padB;
	uint padC;

	rgbaf color;
};

struct oGpuTrivialInstanceConstants
{
	#ifndef oHLSL
	oGpuTrivialInstanceConstants()
		: translation(0,0,0)
		, slice(0)
		, rotation(0,0,0,1)
		, color(ouro::black)
	{}
	#endif

	float3 translation;
	uint slice;
	quatf rotation;
	rgbaf color;
};

#ifdef oHLSL
cbuffer cbuffer_oGpuTrivialDrawConstants : register(b0) { oGpuTrivialDrawConstants GpuTrivialDrawConstants; }
cbuffer cbuffer_oGpuTrivialTestInstances : register(b1) { oGpuTrivialInstanceConstants GpuTrivialInstanceConstants[oGPU_TRIVIAL_NUM_INSTANCES]; }

float4 oGpuLStoSS(float3 LSposition)
{
	return mul(GpuTrivialDrawConstants.wvp, float4(LSposition, 1));
}

float4 oGpuLStoSS(float3 LSposition, uint instance)
{
	oGpuTrivialInstanceConstants Inst = GpuTrivialInstanceConstants[instance];
	return oGpuLStoSS(qmul(Inst.rotation, LSposition) + Inst.translation);
}

float oGpuGetSlice()
{
	return GpuTrivialDrawConstants.slice;
}

float oGpuGetSlice(uint instance)
{
	oGpuTrivialInstanceConstants Inst = GpuTrivialInstanceConstants[instance];
	return Inst.slice;
}

float4 oGpuGetColor()
{
	return GpuTrivialDrawConstants.color;
}

float4 oGpuGetColor(uint instance)
{
	oGpuTrivialInstanceConstants Inst = GpuTrivialInstanceConstants[instance];
	return Inst.color;
}
#endif
#endif
