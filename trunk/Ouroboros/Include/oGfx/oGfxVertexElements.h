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
// This header is compiled both by HLSL and C++. It describes a oGfx-level 
// policy encapsulation of the layout of vertex buffers that oGfx shaders 
// expect.
#ifndef oHLSL
	#pragma once
#endif
#ifndef oGfxVertexElements_h
#define oGfxVertexElements_h

#ifdef oHLSL

#define oGFX_VE(_Type, _Name, _Semantic) _Type _Name : _Semantic

#else

#define oGFX_VE(_Type, _Name, _Semantic) _Type _Name

#include <oBasis/oGPUConcepts.h>

enum oGFX_VERTEX_ELEMENT_LIST
{
	oGFX_VE_POSITION,
	oGFX_VE_POSITION_INSTANCED,
	oGFX_VE_RIGID,
	oGFX_VE_RIGID_INSTANCED,
	oGFX_VE_LINE,
};

void oGfxGetVertexElements(oGFX_VERTEX_ELEMENT_LIST _GfxVertexElementList, const oGPU_VERTEX_ELEMENT** _ppVertexElements, unsigned int* _pNumVertexElements);

#endif

struct oGFX_POSITION_VERTEX
{
	oGFX_VE(float3, LSPosition, POS0);
};

struct oGFX_POSITION_VERTEX_INSTANCED
{
	oGFX_VE(float3, LSPosition, POS0);

	// Per-instance
	oGFX_VE(float3, Translation, TX);
	oGFX_VE(quatf, Rotation, ROT);
	oGFX_VE(float, Scale, SCAL);
};

struct oGFX_RIGID_VERTEX
{
	oGFX_VE(float3, LSPosition, POS0);
	oGFX_VE(float3, LSNormal, NML0);
	oGFX_VE(float2, Texcoord, TEX0);
	oGFX_VE(float4, LSTangent, TAN0);
};

struct oGFX_RIGID_VERTEX_INSTANCED
{
	// Per-vertex
	oGFX_VE(float3, LSPosition, POS0);
	oGFX_VE(float3, LSNormal, NML0);
	oGFX_VE(float2, Texcoord, TEX0);
	oGFX_VE(float4, LSTangent, TAN0);

	// Per-instance
	oGFX_VE(float3, Translation, TX);
	oGFX_VE(quatf, Rotation, ROT);
	oGFX_VE(float, Scale, SCAL);
	
	#ifdef oHLSL
		uint InstanceID : SV_InstanceID;
	#endif
};

struct oGFX_LINE_VERTEX
{
	oGFX_VE(float3, LSPosition, POS0);

	#ifdef oHLSL
		float4 Color : CLR0;
	#else
		oStd::color Color;
	#endif
};

#endif
