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
#include <oGPU/shaders.h>

struct oGpuTrivialLineInterpolant
{
	float4 SSposition : SV_Position;
	float4 color : COLOR;
};

struct oGpuTrivialVertexVectorsInput
{
	float3 LSposition : POSITION;
	float3 LSnormal : NORMAL;
	float4 LStangent : TANGENT;
};

// _____________________________________________________________________________
// oGPU samplers

SamplerState PointClamp : register(s0);
SamplerState PointWrap : register(s1);
SamplerState LinearClamp : register(s2);
SamplerState LinearWrap : register(s3);
SamplerState AnisoClamp : register(s4);
SamplerState AnisoWrap : register(s5);

// _____________________________________________________________________________
// Solid color shaders

float4 PSBlack() : SV_Target { return float4(0,0,0,1); }
float4 PSWhite() : SV_Target { return float4(1,1,1,1); }
float4 PSRed() : SV_Target { return float4(1,0,0,1); }
float4 PSGreen() : SV_Target { return float4(0,1,0,1); }
float4 PSBlue() : SV_Target { return float4(0,0,1,1); }
float4 PSYellow() : SV_Target { return float4(1,1,0,1); }
float4 PSMagenta() : SV_Target { return float4(1,0,1,1); }
float4 PSCyan() : SV_Target { return float4(0,1,1,1); }

// _____________________________________________________________________________
// Simple interpolant pixel shaders

Texture1D Simple1D : register(t0);
Texture1DArray Simple1DArray : register(t0);
Texture2D Simple2D : register(t0);
Texture2DArray Simple2DArray : register(t0);
Texture3D Simple3D : register(t0);
TextureCube SimpleCube : register(t0);
TextureCubeArray SimpleCubeArray : register(t0);

float4 PSTexture1D(float4 SSposition : SV_Position, float texcoord : TEXCOORD) : SV_Target { return Simple1D.Sample(LinearWrap, texcoord); }
float4 PSTexture1DArray(float4 SSposition : SV_Position, float2 texcoord : TEXCOORD) : SV_Target { return Simple1DArray.Sample(LinearWrap, texcoord); }
float4 PSTexture2D(float4 SSposition : SV_Position, float2 texcoord : TEXCOORD) : SV_Target { return Simple2D.Sample(LinearWrap, texcoord); }
float4 PSTexture2DArray(float4 SSposition : SV_Position, float3 texcoord : TEXCOORD) : SV_Target { return Simple2DArray.Sample(LinearWrap, texcoord); }
float4 PSTexture3D(float4 SSposition : SV_Position, float3 texcoord : TEXCOORD) : SV_Target { return Simple3D.Sample(LinearWrap, texcoord); }
float4 PSTextureCube(float4 SSposition : SV_Position, float3 texcoord : TEXCOORD) : SV_Target { return SimpleCube.Sample(LinearWrap, texcoord); }
float4 PSTextureCubeArray(float4 SSposition : SV_Position, float4 texcoord : TEXCOORD) : SV_Target { return SimpleCubeArray.Sample(LinearWrap, texcoord); }
float4 PSVertexColor(float4 SSposition : SV_Position, float4 color : COLOR) : SV_Target { return color; }
float4 PSTexcoord(float4 SSposition : SV_Position, float2 texcoord : TEXCOORD) : SV_Target { return float4(texcoord, 0, 1); }

// _____________________________________________________________________________
// Trivial vertex shaders

float4 VSPassThroughPos(float3 LSposition : POSITION) : SV_Position
{
	return float4(LSposition, 1);
}

void VSPassThroughPosColor(float3 LSposition : POSITION, float4 color : COLOR, out float4 out_SSposition : SV_Position, out float4 out_color : COLOR)
{
	out_SSposition = float4(LSposition, 1);
	out_color = color;
}

void VSPassThroughPosUv(float3 LSposition : POSITION, float2 texcoord : TEXCOORD, out float4 out_SSposition : SV_Position, out float2 out_texcoord : TEXCOORD)
{
	out_SSposition = float4(LSposition, 1);
	out_texcoord = texcoord;
}

void VSPassThroughPosUvw(float3 LSposition : POSITION, float3 texcoord : TEXCOORD, out float4 out_SSposition : SV_Position, out float3 out_texcoord : TEXCOORD)
{
	out_SSposition = float4(LSposition, 1);
	out_texcoord = texcoord;
}

void VSTexture1D(float3 LSposition : POSITION, float texcoord : TEXCOORD, out float4 out_SSposition : SV_Position, out float out_texcoord : TEXCOORD)
{
	out_SSposition = oGpuLStoSS(LSposition);
	out_texcoord = texcoord;
}

void VSTexture1DArray(float3 LSposition : POSITION, float texcoord : TEXCOORD, out float4 out_SSposition : SV_Position, out float2 out_texcoord : TEXCOORD)
{
	out_SSposition = oGpuLStoSS(LSposition);
	out_texcoord = float2(texcoord, oGpuGetSlice());
}

void VSTexture2D(float3 LSposition : POSITION, float2 texcoord : TEXCOORD, out float4 out_SSposition : SV_Position, out float2 out_texcoord : TEXCOORD)
{
	out_SSposition = oGpuLStoSS(LSposition);
	out_texcoord = texcoord;
}

void VSTexture2DArray(float3 LSposition : POSITION, float2 texcoord : TEXCOORD, out float4 out_SSposition : SV_Position, out float3 out_texcoord : TEXCOORD)
{
	out_SSposition = oGpuLStoSS(LSposition);
	out_texcoord = float3(texcoord, oGpuGetSlice());
}

void VSTexture3D(float3 LSposition : POSITION, float3 texcoord : TEXCOORD, out float4 out_SSposition : SV_Position, out float3 out_texcoord : TEXCOORD)
{
	out_SSposition = oGpuLStoSS(LSposition);
	out_texcoord = texcoord;
}

void VSTextureCube(float3 LSposition : POSITION, out float4 out_SSposition : SV_Position, out float3 out_texcoord : TEXCOORD)
{
	out_SSposition = oGpuLStoSS(LSposition);
	out_texcoord = LSposition; // use local-space position and allow for cube mapping to be applied since texcoords are so often in 2D and not appropriate for cubemaps
}

void VSTextureCubeArray(float3 LSposition : POSITION, float3 texcoord : TEXCOORD, out float4 out_SSposition : SV_Position, out float4 out_texcoord : TEXCOORD)
{
	out_SSposition = oGpuLStoSS(LSposition);
	out_texcoord = float4(LSposition, oGpuGetSlice()); // use local-space position and allow for cube mapping to be applied since texcoords are so often in 2D and not appropriate for cubemaps
}

float4 VSTrivialPos(float3 LSposition : POSITION) : SV_Position
{
	return oGpuLStoSS(LSposition);
}

void VSTrivialPosColor(float3 LSposition : POSITION, out float4 out_SSposition : SV_Position, out float4 out_color : COLOR)
{
	out_SSposition = oGpuLStoSS(LSposition);
	out_color = oGpuGetColor();
}

void VSTrivialPosColorInstanced(float3 LSposition : POSITION, uint instance : SV_InstanceID, out float4 out_SSposition : SV_Position, out float4 out_color : COLOR)
{
	out_SSposition = oGpuLStoSS(LSposition, instance);
	out_color = oGpuGetColor(instance);
}

oGpuTrivialVertexVectorsInput VSPassThroughVertexVectors(oGpuTrivialVertexVectorsInput In)
{
	return In;
}

void VSFullScreenTri(uint id : SV_VertexID, out float4 out_SSposition : SV_Position, out float2 out_texcoord : TEXCOORD)
{
	out_texcoord = float2((id << 1) & 2, id & 2); 
	out_SSposition = float4(out_texcoord * float2(2, -2) + float2(-1, 1), 0, 1);
}

// _____________________________________________________________________________
// Geometry Shaders

// Tangents, Bitangents and Normals vectors are common per-vertex data. This 
// defines a geometry shader that will expand a vertex position into a line 
// representing such a vector.
void oGSExpandVertexVector(float3 LSposition, float3 LSvector, float4 color, inout LineStream<oGpuTrivialLineInterpolant> out_line)
{
	static const float3 kVectorScale = 0.025;
	oGpuTrivialLineInterpolant interp = (oGpuTrivialLineInterpolant)0;
	interp.color = color;
	interp.SSposition = oGpuLStoSS(LSposition);
	out_line.Append(interp);
	interp.SSposition = oGpuLStoSS(LSvector * kVectorScale + LSposition);
	out_line.Append(interp);
}

[maxvertexcount(2)]
void GSVertexNormals(point oGpuTrivialVertexVectorsInput In[1], inout LineStream<oGpuTrivialLineInterpolant> out_line)
{
	oGSExpandVertexVector(In[0].LSposition, In[0].LSnormal, float4(0,1,0,1), out_line);
}

[maxvertexcount(2)]
void GSVertexTangents(point oGpuTrivialVertexVectorsInput In[1], inout LineStream<oGpuTrivialLineInterpolant> out_line)
{
	oGSExpandVertexVector(In[0].LSposition, In[0].LStangent.xyz, float4(1,0,0,1), out_line);
}

// _____________________________________________________________________________
// Test shaders

static int TESTBufferAppendIndices[20] = 
{ 5, 6, 7, 18764, 2452, 2423, 52354, 344, -1542, 3434, 53, -4535, 3535, 88884747, 534535, 88474, -445, 4428855, -1235, 4661};

void VSGpuTestBuffer(in uint id : SV_VertexID, out float4 out_SSposition : SV_Position, out int out_index : TESTBUFFERINDEX)
{
	out_SSposition = float4(0, 0, 0, 1);
	out_index = TESTBufferAppendIndices[id];
}

AppendStructuredBuffer<int> TESTGPUBufferOutput : register(u0);

void PSGpuTestBuffer(in float4 Position : SV_Position, in int Index : TESTBUFFERINDEX)
{
	TESTGPUBufferOutput.Append(Index);
}
// _____________________________________________________________________________
// Compute Shaders

[numthreads(1, 1, 1)] void CSNoop()
{
}

