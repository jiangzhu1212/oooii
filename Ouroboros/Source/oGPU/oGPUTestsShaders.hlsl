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

#include <oGPUTestHLSL.h>

static int TESTGPUBufferAppendIndices[20] = 
{ 5, 6, 7, 18764, 2452, 2423, 52354, 344, -1542, 3434, 53, -4535, 3535, 88884747, 534535, 88474, -445, 4428855, -1235, 4661};

void VSTestBuffer(in uint ID : SV_VertexID, out float4 Position : SV_Position, out int Index : TESTBUFFERINDEX)
{
	Position = float4(0.0f, 0.0f, 0.0f, 1.0f);
	Index = TESTGPUBufferAppendIndices[ID];
}

AppendStructuredBuffer<int> Output : register( u0 );

void PSTestBuffer(in float4 Position : SV_Position, in int Index : TESTBUFFERINDEX)
{
	Output.Append(Index);
}

float4 PSTestColor(VSOUT In) : SV_Target
{
	return In.Color;
}

VSOUT VSTestPassThroughColor(float3 LSPosition : POSITION, float4 Color : COLOR)
{
	VSOUT Out = (VSOUT)0;
	Out.SSPosition = float4(LSPosition,1);
	Out.Color = Color;
	return Out;
}

VSOUT VSTestPassThrough(float3 LSPosition : POSITION)
{
	VSOUT Out = (VSOUT)0;
	Out.SSPosition = float4(LSPosition,1);
	return Out;
}
	
VSOUT VSTestTexture1D(float3 LSPosition : POSITION, float Texcoord : TEXCOORD)
{
	return CommonVS(LSPosition, float3(Texcoord, 0, 0));
}

SamplerState Bilin : register(s0);
Texture1D Diffuse1D : register(t0);

float4 PSTestTexture1D(VSOUT In) : SV_Target
{
	return Diffuse1D.Sample(Bilin, In.Texcoord.x);
}

VSOUT VSTestTexture2D(float3 LSPosition : POSITION, float2 Texcoord : TEXCOORD)
{
	return CommonVS(LSPosition, float3(Texcoord, 0));
}

Texture2D Diffuse2D : register(t0);

float4 PSTestTexture2D(VSOUT In) : SV_Target
{
	return Diffuse2D.Sample(Bilin, In.Texcoord.xy);
}

VSOUT VSTestTexture3D(float3 LSPosition : POSITION, float3 Texcoord : TEXCOORD)
{
	return CommonVS(LSPosition, Texcoord);
}

Texture3D Diffuse3D : register(t0);

float4 PSTestTexture3D(VSOUT In) : SV_Target
{
	return Diffuse3D.Sample(Bilin, In.Texcoord);
}

VSOUT VSTestTextureCube(float3 LSPosition : POSITION, float3 Texcoord : TEXCOORD)
{
	return CommonVS(LSPosition, LSPosition);
}

TextureCube DiffuseCube : register(t0);

float4 PSTestTextureCube(VSOUT In) : SV_Target
{
	return DiffuseCube.Sample(Bilin, In.Texcoord);
}

VSOUT VSTestWhiteInstanced(float3 LSPosition : POSITION, uint Index : SV_InstanceID)
{
	return CommonVS(LSPosition, Index);
}

VSOUT VSTestWhite(float3 LSPosition : POSITION)
{
	return CommonVS(LSPosition, oZERO3);
}

float4 PSTestWhite(VSOUT In) : SV_Target
{
	return oWHITE4;
}
