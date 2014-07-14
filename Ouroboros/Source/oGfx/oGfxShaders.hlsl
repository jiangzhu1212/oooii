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

#include <oCompute/oComputeConstants.h>
#include <oCompute/oComputePhong.h>
#include <oCompute/oComputeProcedural.h>
#include <oCompute/oComputeUtil.h>

#include <oGfx/oGfxDrawConstants.h>
#include <oGfx/oGfxLightConstants.h>
#include <oGfx/oGfxMaterialConstants.h>
#include <oGfx/oGfxViewConstants.h>
#include <oGfx/oGfxVertexLayouts.h>

// _____________________________________________________________________________
// Conventions:

// LS: local space
// WS: world space
// VS: view space
// CS: clip space
// NS: ndc (post-clip) space
// SS: screen-space

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

float4 PSBlack() : SV_Target { return oBLACK4; }
float4 PSWhite() : SV_Target { return oWHITE4; }
float4 PSRed() : SV_Target { return oRED4; }
float4 PSGreen() : SV_Target { return oGREEN4; }
float4 PSBlue() : SV_Target { return oBLUE4; }
float4 PSYellow() : SV_Target { return oYELLOW4; }
float4 PSMagenta() : SV_Target { return oMAGENTA4; }
float4 PSCyan() : SV_Target { return oCYAN4; }

// _____________________________________________________________________________
// Simple texture-based shaders

Texture1D Simple1D : register(t0);
Texture1DArray Simple1DArray : register(t0);
Texture2D Simple2D : register(t0);
Texture2DArray Simple2DArray : register(t0);
Texture3D Simple3D : register(t0);
TextureCube SimpleCube : register(t0);
TextureCubeArray SimpleCubeArray : register(t0);

float4 PSTexture1D(float4 SSpostion : SV_Position, float texcoord : TEXCOORD) : SV_Target { return Simple1D.Sample(LinearWrap, texcoord); }
float4 PSTexture1DArray(float4 SSpostion : SV_Position, float2 texcoord : TEXCOORD) : SV_Target { return Simple1DArray.Sample(LinearWrap, texcoord); }
float4 PSTexture2D(float4 SSpostion : SV_Position, float2 texcoord : TEXCOORD) : SV_Target { return Simple2D.Sample(LinearWrap, texcoord); }
float4 PSTexture2DArray(float4 SSpostion : SV_Position, float3 texcoord : TEXCOORD) : SV_Target { return Simple2DArray.Sample(LinearWrap, texcoord); }
float4 PSTexture3D(float4 SSpostion : SV_Position, float3 texcoord : TEXCOORD) : SV_Target { return Simple3D.Sample(LinearWrap, texcoord); }
float4 PSTextureCube(float4 SSpostion : SV_Position, float3 texcoord : TEXCOORD) : SV_Target { return SimpleCube.Sample(LinearWrap, texcoord); }
float4 PSTextureCubeArray(float4 SSpostion : SV_Position, float4 texcoord : TEXCOORD) : SV_Target { return SimpleCubeArray.Sample(LinearWrap, texcoord); }

// _____________________________________________________________________________
// Simple interpolant-based shaders

float4 PSVertexColor(float4 SSposition : SV_Position, float4 color : COLOR) : SV_Target { return color; }

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
	out_SSposition = oGfxLStoSS(LSposition);
	out_texcoord = texcoord;
}

void VSTexture1DArray(float3 LSposition : POSITION, float texcoord : TEXCOORD, out float4 out_SSposition : SV_Position, out float2 out_texcoord : TEXCOORD)
{
	out_SSposition = oGfxLStoSS(LSposition);
	out_texcoord = float2(texcoord, oGfxGetSlice());
}

void VSTexture2D(float3 LSposition : POSITION, float2 texcoord : TEXCOORD, out float4 out_SSposition : SV_Position, out float2 out_texcoord : TEXCOORD)
{
	out_SSposition = oGfxLStoSS(LSposition);
	out_texcoord = texcoord;
}

void VSTexture2DArray(float3 LSposition : POSITION, float2 texcoord : TEXCOORD, out float4 out_SSposition : SV_Position, out float3 out_texcoord : TEXCOORD)
{
	out_SSposition = oGfxLStoSS(LSposition);
	out_texcoord = float3(texcoord, oGfxGetSlice());
}

void VSTexture3D(float3 LSposition : POSITION, float3 texcoord : TEXCOORD, out float4 out_SSposition : SV_Position, out float3 out_texcoord : TEXCOORD)
{
	out_SSposition = oGfxLStoSS(LSposition);
	out_texcoord = texcoord;
}

void VSTextureCube(float3 LSposition : POSITION, float3 texcoord : TEXCOORD, out float4 out_SSposition : SV_Position, out float3 out_texcoord : TEXCOORD)
{
	out_SSposition = oGfxLStoSS(LSposition);
	out_texcoord = texcoord;
}

void VSTextureCubeArray(float3 LSposition : POSITION, float3 texcoord : TEXCOORD, out float4 out_SSposition : SV_Position, out float4 out_texcoord : TEXCOORD)
{
	out_SSposition = oGfxLStoSS(LSposition);
	out_texcoord = float4(texcoord, oGfxGetSlice());
}

struct oGFX_INSTANCE
{
	float3 Translation;
	quatf Rotation;
};

#define MAX_NUM_INSTANCES (64)

cbuffer cb_GfxInstances { oGFX_INSTANCE GfxInstances[MAX_NUM_INSTANCES]; }

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

// _____________________________________________________________________________
// Vertex Shaders

oGFX_VS_OUT_UNLIT VSLines(vertex_pos_color In)
{
	oGFX_VS_OUT_UNLIT Out = (oGFX_VS_OUT_UNLIT)0;
	Out.SSPosition = oGfxLStoSS(In.position);
	Out.Color = In.color;
	return Out;
}

vertex_pos_nrm_tan_uv0 VSPassThrough(vertex_pos_nrm_tan_uv0 In)
{
	return In;
}

float4 VSPosition(float3 _LSPosition : POSITION) : SV_Position
{
	return oGfxLStoSS(_LSPosition);
}

oGFX_VS_OUT_LIT VSRigid(vertex_pos_nrm_tan_uv0 In)
{
	oGFX_VS_OUT_LIT Out = (oGFX_VS_OUT_LIT)0;
	Out.SSPosition = oGfxLStoSS(In.position);
	Out.WSPosition = oGfxLStoWS(In.position);
	Out.LSPosition = In.position;
	Out.VSDepth = oGfxLStoVS(In.position).z;
	oGfxRotateBasisLStoWS(In.normal, In.tangent, Out.WSNormal, Out.WSTangent, Out.WSBitangent);
	Out.VSNormal = oGfxRotateLStoVS(In.normal);
	Out.Texcoord = In.texcoord;
	return Out;
}
	
oGFX_VS_OUT_LIT VSRigidInstanced(vertex_pos_nrm_tan_uv0 In, uint _InstanceID : SV_InstanceID)
{
	oGFX_INSTANCE Instance = GfxInstances[_InstanceID];
	oGFX_VS_OUT_LIT Out = (oGFX_VS_OUT_LIT)0;
	Out.SSPosition = oGfxWStoSS(Out.WSPosition);
	Out.WSPosition = qmul(Instance.Rotation, In.position) + Instance.Translation;
	Out.LSPosition = In.position;
	Out.VSDepth = oGfxWStoVS(Out.WSPosition).z;
	oQRotateTangentBasisVectors(Instance.Rotation, In.normal, In.tangent, Out.WSNormal, Out.WSTangent, Out.WSBitangent);
	Out.VSNormal = oGfxRotateWStoVS(qmul(Instance.Rotation, In.normal));
	Out.Texcoord = In.texcoord;
	return Out;
}

void VSShadow(in float3 LSPosition : POSITION, out float4 _Position : SV_Position, out float _VSDepth : VIEWSPACEZ)
{
	_Position = oGfxLStoSS(LSPosition);
	_VSDepth = oGfxNormalizeDepth(oGfxLStoVS(LSPosition).z);
}

// _____________________________________________________________________________
// Hull Shaders

// _____________________________________________________________________________
// Domain Shaders
	
// _____________________________________________________________________________
// Geometry Shaders

[maxvertexcount(2)]
void GSVertexNormals(point vertex_pos_nrm_tan_uv0 In[1], inout LineStream<oGFX_VS_OUT_UNLIT> _Out)
{
	oGSExpandVertexVector(In[0].position, In[0].normal, oGREEN4, _Out);
}

[maxvertexcount(2)]
void GSVertexTangents(point vertex_pos_nrm_tan_uv0 In[1], inout LineStream<oGFX_VS_OUT_UNLIT> _Out)
{
	oGSExpandVertexVector(In[0].position, In[0].tangent.xyz, oRED4, _Out);
}

// _____________________________________________________________________________
// Pixel Shaders

float4 PSColor(float4 _Position : SV_Position, float4 _Color : CLR0) : SV_Target
{
	return _Color;
}

static const float3 oCHALKYBLUE = float3(0.44, 0.57, 0.75);
static const float3 oSMOKEYWHITE = float3(0.88, 0.9, 0.96);

float4 PSGrid(oGFX_VS_OUT_LIT In) : SV_Target
{
	float GridFactor = oCalcFadingGridShadeIntensity2D(In.LSPosition.xy, oCalcMipSelection(In.LSPosition.xy), 0.2, 10, 0.05);
	float3 Color = lerp(oSMOKEYWHITE, oCHALKYBLUE, GridFactor);
	return float4(Color, 1);
}

// The Hero Shader is a reimplementation of the material interface described by
// Valve for DOTA2 here:
// http://media.steampowered.com/apps/dota2/workshop/Dota2ShaderMaskGuide.pdf

//cbuffer
//{
static const float GfxMaterialConstants_IoR = IoRAir;
static const float GfxMaterialConstants_FresnelFalloff = 1.0;
static const float GfxMaterialConstants_BumpScale = 1.0;
//};

oGFX_GBUFFER_FRAGMENT PSHero(oGFX_VS_OUT_LIT In)
{
	oGfxMaterialConstants M = oGfxGetMaterial(In.Texcoord);

	// Calculate relevant values in world-space 

	float3 WSEyePosition = oGfxGetEyePosition();
	float3 WSEyeVector = normalize(WSEyePosition - In.WSPosition);

	// Just grab the first light for now
	oGfxLight Light = oGfxGetLight(0);
	float3 WSLightVector = normalize(Light.WSPosition - In.WSPosition);

	// @tony: It'd be good to add a detail normal map as well, but will that
	// need a separate mask from the color detail map? Maybe it's just part of a 
	// detail material.
  float3 WSUnnormalizedNormal = oDecodeTSNormal(
		In.WSTangent
		, In.WSBitangent
		, In.WSNormal
		, M.SampledTSNormal.xyz
		, GfxMaterialConstants_BumpScale);

	// Do phong-style lighting
	float3 WSNormal;
	float4 Lit = oLitHalfLambertToksvig(WSUnnormalizedNormal, WSLightVector, WSEyeVector, M.SpecularExponent, WSNormal);

	// Decode lit and material mask values into more familiar coefficients
	// NOTE: DetailMap, Rimlight, Fresnel color not yet implmented
	const float Ke = M.SelfIlluminationIntensity;
	const float Kd = (1 - M.MetalnessIntensity) * Lit.y;
	const float Ks = M.SpecularIntensity * Lit.z;
	const float Kf = M.DiffuseFresnelIntensity * oFresnel(
		WSEyeVector
		, WSNormal
		, IoRAir
		, GfxMaterialConstants_IoR
		, GfxMaterialConstants_FresnelFalloff);

	const float3 Ce = M.Color;
	const float3 Cd = M.Color;
	const float3 Cs = lerp(oWHITE3, Cd, M.SpecularTintIntensity);
	
	// Valve has this from a 3D texture (how is it 3D? there's 2D texcoords AFAICT 
	// and only one intensity value). This probably will come from engine values,
	// such as a realtime reflection map, so stub this for now.
	const float3 Cf = oWHITE3;

	// Assign output
	oGFX_GBUFFER_FRAGMENT Out = (oGFX_GBUFFER_FRAGMENT)0;
	Out.VSNormalXY = oFullToHalfSphere(normalize(In.VSNormal));
	Out.LinearDepth = In.VSDepth;

	// @tony: this is not proving to be all that useful...
	Out.LinearDepth_DDX_DDY = float2(abs(100* max(ddx(In.Texcoord))), abs(100 * max(ddy(In.Texcoord))));
	Out.Mask0 = float4(0,0,0,0); // outline mask is off by default

	// ambient component ignored/emulated by emissive
	Out.Color = float4(Ke*Ce + Kd*Cd + Ks*Cs + Kf*Cf, M.Opacity);
	return Out;
}

struct VSOUT
{
	float4 Color : SV_Target;
};

oGFX_GBUFFER_FRAGMENT PSMaterial(oGFX_VS_OUT_LIT In)
{
	oGFX_GBUFFER_FRAGMENT Out = (oGFX_GBUFFER_FRAGMENT)0;	

	float4 Diffuse = oGfxColorSample(In.Texcoord);

	float3 E = oGfxGetEyePosition();
	float3 EyeVector = normalize(E - In.WSPosition);
	oGfxLight Light = oGfxGetLight(0);
	float3 L = normalize(Light.WSPosition - In.WSPosition);
	float3 HalfVector = normalize(EyeVector + L);

  float3 WSUnnormalizedNormal = oDecodeTSNormal(In.WSTangent, In.WSBitangent, In.WSNormal
		, oGfxNormalSample(In.Texcoord).xyz
		, 1.0f); // FIXME: Figure out what mask channel this comes from
		
	
	oRGBf rgb = oPhongShade(normalize(WSUnnormalizedNormal)
									, L
									, EyeVector
									, 1
									, oBLACK3
									// Figure out where these other parameters come from in the mask texture
									, oBLACK3
									, Diffuse.rgb
									, oBLACK3
									, oZERO3
									, oZERO3
									, oWHITE3
									, 1
									, 0.1f);

	Out.Color = float4(rgb, 1);
	Out.VSNormalXY = oFullToHalfSphere(normalize(In.VSNormal));
	Out.LinearDepth = oGfxNormalizeDepth(In.VSDepth);
	Out.Mask0 = float4(0,0,0,0);
	return Out;
}

uint PSObjectID(float4 Position : SV_Position) : SV_Target
{
	return oGfxGetObjectID();
}

void PSShadow(in float4 _Position : SV_Position, in float _VSDepth : VIEWSPACEZ, out float _Depth : SV_Depth) 
{ 
	_Depth = _VSDepth;
}

float4 PSTexcoord(oGFX_VS_OUT_LIT In) : SV_Target
{
	return float4(In.Texcoord.xy, 0, 1);
}

Texture2D ColorTexture : register(t0);
SamplerState Sampler : register(s0);

float4 PSTextureColor(oGFX_VS_OUT_LIT In) : SV_Target
{
	return ColorTexture.Sample(Sampler, In.Texcoord);
}

Texture2D DiffuseTexture : register(t0);

oGFX_GBUFFER_FRAGMENT PSVertexPhong(oGFX_VS_OUT_LIT In)
{
	float4 Diffuse = DiffuseTexture.Sample(Sampler, In.Texcoord);
	
	float3 E = oGfxGetEyePosition();
	float3 ViewVector = normalize(E - In.WSPosition);
	oGfxLight Light = oGfxGetLight(0);
	float3 L = normalize(Light.WSPosition - In.WSPosition);

	oGFX_GBUFFER_FRAGMENT Out = (oGFX_GBUFFER_FRAGMENT)0;	
	oRGBf rgb = oPhongShade(normalize(In.WSNormal)
									, L
									, ViewVector
									, 1
									, oBLACK3
									 // Figure out where these other parameters come from in the mask texture
									, oBLACK3
									, Diffuse.rgb
									, oBLACK3
									, oZERO3
									, oZERO3
									, oWHITE3
									, 1
									, 0.1f);

	Out.Color = float4(rgb, 1);
	return Out;
}

float4 PSVSDepth(oGFX_VS_OUT_LIT In) : SV_Target
{
	return float4(oGfxNormalizeDepth(In.VSDepth).xxx, 1);
}

float4 PSWSPixelNormal(oGFX_VS_OUT_LIT In) : SV_Target
{
  float3 WSNormal = normalize(oDecodeTSNormal(
		In.WSTangent
		, In.WSBitangent
		, In.WSNormal
		, oGfxNormalSample(In.Texcoord).rgb
		, 1.0));
	return float4(colorize_vector(WSNormal), 1);
}

float4 PSWSVertexNormal(oGFX_VS_OUT_LIT In) : SV_Target
{
	return float4(colorize_vector(normalize(In.WSNormal)), 1);
}











// _____________________________________________________________________________
// Shaders used in simple unit tests

static int TESTBufferAppendIndices[20] = 
{ 5, 6, 7, 18764, 2452, 2423, 52354, 344, -1542, 3434, 53, -4535, 3535, 88884747, 534535, 88474, -445, 4428855, -1235, 4661};

void VSTestBuffer(in uint id : SV_VertexID, out float4 out_SSposition : SV_Position, out int out_index : TESTBUFFERINDEX)
{
	out_SSposition = float4(0.0f, 0.0f, 0.0f, 1.0f);
	out_index = TESTBufferAppendIndices[id];
}

AppendStructuredBuffer<int> TESTGPUBufferOutput : register(u0);

void PSTestBuffer(in float4 Position : SV_Position, in int Index : TESTBUFFERINDEX)
{
	TESTGPUBufferOutput.Append(Index);
}

void VSTestTransform(float3 LSposition : POSITION, out float4 out_SSposition : SV_Position)
{
	out_SSposition = oGfxLStoSS(LSposition);
}

void VSTestInstanced(float3 LSposition : POSITION, uint instance_index : SV_InstanceID, out float4 out_SSposition : SV_Position, out float4 out_color : COLOR)
{
	oGfxTestInstance Inst = GPUTestInstances[instance_index];
	out_SSposition = oGfxLStoSS(qmul(Inst.Rotation, LSposition) + Inst.Translation);
	out_color = oGfxGetColor();
}


