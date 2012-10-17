/**************************************************************************
 * The MIT License                                                        *
 * Copyright (c) 2011 Antony Arciuolo & Kevin Myers                       *
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
#ifndef oGPUHLSL_h
#define oGPUHLSL_h

#include <oGPU/oGPUDrawConstants.h>
#include <oGPU/oGPUViewConstants.h>
#include <oGPU/oGPUMaterialConstants.h>

// LS: Local Space
// WS: World Space
// SS: Screen Space
// VS: View Space

struct VSIN
{
	float3 LSPosition : POSITION;
	float3 LSNormal : NORMAL;
	float2 Texcoord : TEXCOORD;
};

struct VSININSTANCED
{
	// Per-vertex
	float3 LSPosition : POSITION;
	float3 LSNormal : NORMAL;
	float2 Texcoord : TEXCOORD;

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
	float2 Texcoord : TEXCOORD;
	float4 Color : Color;
};

struct PSOUT
{
	float4 Color;
};

VSOUT CommonVS(VSIN In)
{
	VSOUT Out = (VSOUT)0;
	Out.SSPosition = oGPULStoSS(In.LSPosition);
	Out.WSPosition = oGPULStoWS(In.LSPosition);
	Out.LSPosition = In.LSPosition;
	Out.WSNormal = oGPUNormalLStoWS(In.LSNormal);
	Out.Texcoord = In.Texcoord;
	Out.Color = oWHITE4;
	return Out;
}

#endif