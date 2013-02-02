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
#ifndef oHLSL
	#pragma once
#endif
#ifndef oGPUHLSLCommon_h
#define oGPUHLSLCommon_h

#include <oGPU/oGPUShared.h>
#include <oGPU/oGPUDrawConstants.h>
#include <oGPU/oGPUViewConstants.h>
#include <oGPU/oGPUMaterialConstants.h>

static const float3 oCHALKYBLUE = float3(0.44, 0.57, 0.75);
static const float3 oSMOKEYWHITE = float3(0.88, 0.9, 0.96);

// LS: Local Space
// WS: World Space
// SS: Screen Space
// VS: View Space

struct VSIN
{
	float3 LSPosition : POSITION;
	float3 LSNormal : NORMAL;
	float2 Texcoord : TEXCOORD;
	float4 LSTangent : TANGENT;
};

struct VSININSTANCED
{
	// Per-vertex
	float3 LSPosition : POSITION;
	float3 LSNormal : NORMAL;
	float2 Texcoord : TEXCOORD;
	float4 LSTangent : TANGENT;

	// Per-instance
	float3 Translation : TRANSLATION;
	float4 Rotation : ROTATION;
	uint InstanceID : SV_InstanceID;
};

struct VSOUT
{
	float4 SSPosition : SV_Position;
	float3 WSPosition : POSITION0;
	float3 LSPosition : POSITION1;
	float3 WSNormal : NORMAL;
	float3 WSTangent : TANGENT0;
	float3 WSBitangent : TANGENT1;
	float2 Texcoord : TEXCOORD;
	float4 Color : Color;
};

struct PSOUT
{
	float4 Color;
};

float3 oGetSpelunkerLightPosition(float3 _WSEye)
{
	return normalize(_WSEye + oMul(oGPU_VIEW_CONSTANT_BUFFER.ViewInverse, float4(3,3,-1,1)).xyz); // hard-code a light.
}

#endif