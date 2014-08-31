// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.
// This header compiles in C++ and HLSL
#ifndef oHLSL
	#pragma once
#endif
#ifndef oGfxVertexLayouts_h
#define oGfxVertexLayouts_h

#ifdef oHLSL

#define oVL_POSITION float3 position : POSITION
#define oVL_NORMAL float3 normal : NORMAL
#define oVL_TANGENT float4 tangent : TANGENT
#define oVL_TEXCOORD20 float2 texcoord : TEXCOORD0
#define oVL_TEXCOORD40 float4 texcoord : TEXCOORD0
#define oVL_COLOR float4 color : COLOR

// _____________________________________________________________________________
// Interpolants

struct vsout_superset
{
	float4 SSposition : SV_Position;
	float3 WSposition : POSITION;
	float3 LSposition : POSITION1;
	float3 WSnormal : NORMAL;
	float4 WStangent : TANGENT;
	float4 texcoord : TEXCOORD;
	float4 color : COLOR;
};

#else

#include <oHLSL/oHLSLTypes.h>
#include <oBase/color.h>
#include <oBase/dec3n.h>

#define oVL_POSITION float3 position
#define oVL_NORMAL dec3n normal
#define oVL_TANGENT dec3n tangent
#define oVL_TEXCOORD20 float2 texcoord
#define oVL_TEXCOORD40 float4 texcoord
#define oVL_COLOR color color

namespace ouro { namespace gfx {
#endif

// _____________________________________________________________________________
// Layouts matching gfx::vertex_input values

struct vertex_pos
{
	oVL_POSITION;
};

struct vertex_pos_color
{
	oVL_POSITION;
	oVL_COLOR;
};

struct vertex_pos_nrm
{
	oVL_POSITION;
	oVL_NORMAL;
};

struct vertex_pos_nrm_tan
{
	oVL_POSITION;
	oVL_NORMAL;
	oVL_TANGENT;
};

struct vertex_pos_nrm_tan_uv0
{
	oVL_POSITION;
	oVL_NORMAL;
	oVL_TANGENT;
	oVL_TEXCOORD20;
};

struct vertex_pos_nrm_tan_uvwx0
{
	oVL_POSITION;
	oVL_NORMAL;
	oVL_TANGENT;
	oVL_TEXCOORD40;
};

struct vertex_pos_nrm_tan_uv0_color
{
	oVL_POSITION;
	oVL_NORMAL;
	oVL_TANGENT;
	oVL_TEXCOORD20;
	oVL_COLOR;
};

struct vertex_pos_nrm_tan_uvwx0_color
{
	oVL_POSITION;
	oVL_NORMAL;
	oVL_TANGENT;
	oVL_TEXCOORD40;
	oVL_COLOR;
};

struct vertex_pos_uv0
{
	oVL_POSITION;
	oVL_TEXCOORD20;
};

struct vertex_pos_uvwx0
{
	oVL_POSITION;
	oVL_TEXCOORD40;
};

#ifndef oHLSL
}}
#endif
#endif
