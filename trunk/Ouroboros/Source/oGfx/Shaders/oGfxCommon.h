/**************************************************************************
 * The MIT License                                                        *
 * Copyright (c) 2013 OOOii.                                              *
 * antony.arciuolo@oooii.com                                              *
 * kevin.myers@oooii.com                                                  *
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
// This header is compiled both by HLSL and C++. It describes code shared 
// between oGfx pipelines.
#ifndef oHLSL
	#pragma once
#endif
#ifndef oGfxCommon_h
#define oGfxCommon_h

#include <oGfx/oGfxDrawConstants.h>
#include <oGfx/oGfxLightConstants.h>
#include <oGfx/oGfxMaterialConstants.h>
#include <oGfx/oGfxVertexElements.h>
#include <oGfx/oGfxViewConstants.h>

struct oGFX_VS_OUT_UNLIT
{
	float4 SSPosition : SV_Position;
	float4 Color : CLR0;
};

struct oGFX_VS_OUT_LIT
{
	float4 SSPosition : SV_Position;
	float3 WSPosition : POS0;
	float3 LSPosition : POS1;
	float VSDepth : VSDEPTH;
	float3 WSNormal : NML0;
	float3 VSNormal : NML1;
	float3 WSTangent : TAN0;
	float3 WSBitangent : TAN1;
	float2 Texcoord : TEX0;
};

struct oGFX_GBUFFER_FRAGMENT
{
	float4 Color : SV_Target0;
	float2 VSNormalXY : SV_Target1;
	float LinearDepth : SV_Target2;
	float2 LinearDepth_DDX_DDY : SV_Target3;
	float4 Mask0 : SV_Target4; // R=Outline strength, GBA=Unused
};

// Tangents, Bitangents and Normals vectors are common per-vertex data. This 
// defines a geometry shader that will expand a vertex position into a line 
// representing such a vector.
void oGSExpandVertexVector(float3 _LSPosition, float3 _LSVector, float4 _Color, inout LineStream<oGFX_VS_OUT_UNLIT> _Out)
{
	static const float3 oGFX_VERTEX_VECTOR_SCALE = 0.025;

	oGFX_VS_OUT_UNLIT P;
	P.Color = _Color;
	P.SSPosition = oGfxLStoSS(_LSPosition);
	_Out.Append(P);
	P.SSPosition = oGfxLStoSS(_LSPosition + (_LSVector * oGFX_VERTEX_VECTOR_SCALE * GfxDrawConstants.Scale));
	_Out.Append(P);
}

#endif
