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
#include "oGfxCommon.h"
#include <oCompute/oComputePhong.h>

// The Hero Shader is a reimplementation of the material interface described by
// Valve for DOTA2 here:
// http://media.steampowered.com/apps/dota2/workshop/Dota2ShaderMaskGuide.pdf

//cbuffer
//{
static const float GfxMaterialConstants_IoR = IoRAir;
static const float GfxMaterialConstants_FresnelFalloff = 1.0;
static const float GfxMaterialConstants_BumpScale = 1.0;
//};

oGFX_GBUFFER_FRAGMENT main(oGFX_VS_OUT_LIT In)
{
	oGfxMaterialConstants M = oGfxGetMaterial(In.Texcoord);

	// Calculate relevant values in world-space 

	float3 WSEyePosition = oGfxGetEyePosition();
	float3 WSEyeVector = normalize(WSEyePosition - In.WSPosition);

	// Just grab the first light for now
	oGfxLight Light = oGfxGetLight(0);
	float3 WSLightVector = normalize(Light.WSPosition - In.WSPosition);

	// @oooii-tony: It'd be good to add a detail normal map as well, but will that
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

	// @oooii-tony: this is not proving to be all that useful...
	Out.LinearDepth_DDX_DDY = float2(abs(100* max(ddx(In.Texcoord))), abs(100 * max(ddy(In.Texcoord))));
	Out.Mask0 = float4(0,0,0,0); // outline mask is off by default

	// ambient component ignored/emulated by emissive
	Out.Color = float4(Ke*Ce + Kd*Cd + Ks*Cs + Kf*Cf, M.Opacity);
	return Out;
}
