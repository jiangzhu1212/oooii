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
// structs that are described by vertex_layout. This header compiles in C++ and HLSL
#ifndef oHLSL
	#pragma once
#endif
#ifndef oGPU_vertex_layouts_h
#define oGPU_vertex_layouts_h

#ifdef oHLSL
#define oVL_POSITION float3 position : POSITION
#define oVL_NORMAL float3 normal : NORMAL
#define oVL_TANGENT float4 tangent : TANGENT
#define oVL_TEXCOORD20 float2 texcoord : TEXCOORD0
#define oVL_TEXCOORD40 float4 texcoord : TEXCOORD0
#define oVL_COLOR float4 color : COLOR
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

namespace ouro {
	namespace gpu {
#endif

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
	} // namespace gpu
} // namespace ouro
#endif
#endif
