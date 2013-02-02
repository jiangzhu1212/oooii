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
// NOTE: This header is compiled both by HLSL and C++
#ifndef oHLSL
	#pragma once
#endif
#ifndef oHLSL_h
#define oHLSL_h

#include <oBasis/oMathShared.h>

// @oooii-tony: HACK I shouldn't have to wrap this, but if I don't, raw voxel 
// splatting doesn't look correct.
#ifndef oHLSL
	#include <oBasis/oHLSLStructs.h>
#endif

#ifndef oCONCAT
	#define oCONCAT(x, y) x##y
#endif

#ifndef oMATRIX_COLUMN_MAJOR
	#define oMATRIX_COLUMN_MAJOR 
#endif

#ifndef oRIGHTHANDED
	#define oRIGHTHANDED
#endif

// This is used below in oConvertShininessToSpecularExponent
#ifndef oMAX_SPECULAR_EXPONENT
	#define oMAX_SPECULAR_EXPONENT 2000.0
#endif

#ifdef oHLSL
// _____________________________________________________________________________
// Types and constants

// Because code can be cross-compiled between C++ and shader languages, there
// can be a bit of necessary patch-up. Also create some defines for common
// values.

#define quatf float4
#define oRGBf float3

#ifdef oRIGHTHANDED
	static const float3 oVECTOR_TOWARDS_SCREEN = float3(0,0,1);
	static const float3 oVECTOR_INTO_SCREEN = float3(0,0,-1);
#else
	static const float3 oVECTOR_TOWARDS_SCREEN = float3(0,0,-1);
	static const float3 oVECTOR_INTO_SCREEN = float3(0,0,1);
#endif

// _____________________________________________________________________________
// Noise

// Generates a clipspace-ish quad based off the SVVertexID semantic
// VertexID Texcoord Position
// 0        0,0      -1,1,0	
// 1        1,0      1,1,0
// 2        0,1      -1,-1,0
// 3        1,1      1,-1,0
void oExtractQuadInfoFromVertexID(in uint _SVVertexID, out float4 _Position, out float2 _Texcoord)
{
	_Texcoord = float2(_SVVertexID & 1, _SVVertexID >> 1);
	_Position = float4(_Texcoord * float2(2.0f, -2.0f) + float2(-1.0f, 1.0f), 0.0f, 1.0f);
}

// _____________________________________________________________________________
// Procedural textures to test out the noise functionality above and to give 
// some options for fallback textures.

// Calculate a marble-like pattern (from NVIDIA, ATI, RenderMan and anyone else
// whose had a procedural marble texture). Use the returned value to interpolate
// between two colors. (Also use it to interpolate between two specular values!)
// Start tweaking from values: _Scale = 5, _Lacunarity = 2, _Gain = 0.5
float oCalcMarbleIntensity(float3 _Coord, float _Scale, float _Lacunarity, float _Gain)
{
	return 0.2 + _Scale * abs(oTurbulence(_Coord, 4, _Lacunarity, _Gain) - 0.5);
}

// Calculate a granite-like pattern (from NVIDIA, ATI, RenderMan and anyone else
// whose had a procedural granite texture). Use the returned value to 
// interpolate between two colors. (Also use it to interpolate between two 
// specular values!) 
// Start tweaking from values: _Scale = 5, _Lacunarity = 2, _Gain = 0.5
float oCalcGraniteIntensity(float3 _Coord, float _Scale, float _Lacunarity, float _Gain)
{
	return abs(0.8 - oTurbulence(_Coord, 4, _Lacunarity, _Gain));
}

// This might've been described as a marble formula, but it appears with 
// oTurbulence to be more of a rust/stain pattern. Interpolate between diffuse
// and rust colors/textures. (Also use it to interpolate between two specular 
// values!)
// Start tweaking from values: _Amplification = 1, _Lacunarity = 2, _Gain = 0.5
float oCalcStainIntensity(float3 _Coord, float _Amplification, float _Lacunarity, float _Gain)
{
	// based on:
	// http://www.sci.utah.edu/~leenak/IndStudy_reportfall/MarbleCode.txt
	return saturate(cos(_Coord.z * 0.1 + _Amplification * oTurbulence(_Coord, 4, _Lacunarity, _Gain)));
}

// Generate a lerp value between two colors in a procedural wood pattern.
// (note it is recommended to have two specular colors to go with each diffuse
// color and lerp them by the intensity value
// Good start values: _RingScale = 16 _IrregularityScale = 0.2
float oCalcWoodIntensity(float3 _Coord, float _RingScale, float _IrregularityScale, float _Streakiness)
{
	// Based on NVIDIA Shader Library's wood shader

	float noisy0 = oTurbulence(_Coord, 4, 2.2, 0.5);
	float noisy1 = oTurbulence(_Coord + 1, 4, 2.1, 0.5);
	float noisy2 = oTurbulence(_Coord + 2, 4, 2.345, 0.5) * (1-_Streakiness);

	float3 noiseval = float3(noisy0, noisy1, noisy2);
	float3 Pwood = _Coord + (_IrregularityScale * noiseval);
	float r = _RingScale * length(Pwood.xy);
	r = r + oTurbulence(r.xxx/32.0, 4, 2, 0.5);
	r = frac(r);
	return smoothstep(0.0, 0.8, r) - smoothstep(0.83, 1.0, r);
}

// _____________________________________________________________________________
// Space Tranformation. To abstract row-major v. col-major and simplify 
// quaternion math, present these utility functions.

// Order will always be the same with this function, regardless of matrix
// representation.
float4 oMul(float4x4 m, float4 v)
{
	#ifdef oMATRIX_COLUMN_MAJOR
		return mul(m, v);
	#else
		return mul(v, m);
	#endif
}

float3 oMul(float3x3 m, float3 v)
{
	#ifdef oMATRIX_COLUMN_MAJOR
		return mul(m, v);
	#else
		return mul(v, m);
	#endif
}

// Multiply/combine two quaternions. Careful, remember that quats are not 
// communicative, so order matters. This returns a * b.
float4 oQMul(float4 a, float4 b)
{
	// http://code.google.com/p/kri/wiki/Quaternions
	return float4(cross(a.xyz,b.xyz) + a.xyz*b.w + b.xyz*a.w, a.w*b.w - dot(a.xyz,b.xyz));
}

// Rotate a vector by a quaternion
// q: quaternion to rotate vector by
// v: vector to be rotated
// returns: rotated vector
float3 oQRotate(float4 q, float3 v)
{
	// http://code.google.com/p/kri/wiki/Quaternions
	#if 1
		return v + 2.0*cross(q.xyz, cross(q.xyz,v) + q.w*v);
	#else
		return v*(q.w*q.w - dot(q.xyz,q.xyz)) + 2.0*q.xyz*dot(q.xyz,v) + 2.0*q.w*cross(q.xyz,v);
	#endif
}

// Returns texcoords of screen: [0,0] upper-left, [1,1] lower-right
// _SVPosition is a local-space 3D position multiplied by a WVP matrix, so the
// final projected position that would normally be passed from a vertex shader
// to a pixel shader.
// NOTE: There are 2 flavors because the _SVPosition behaves slightly 
// differently between the result calculated in a vertex shader and what happens
// to it by the time it gets to the pixel shader.
float2 oCalculateScreenSpaceTexcoordVS(float4 _SVPosition)
{
	float2 Texcoord = _SVPosition.xy / _SVPosition.w;
	return Texcoord * float2(0.5, -0.5) + 0.5;
}

float2 oCalculateScreenSpaceTexcoordPS(float4 _SVPosition, float2 _RenderTargetDimensions)
{
	return _SVPosition.xy / _RenderTargetDimensions;
}

// Returns the eye position in whatever space the view matrix is in. REMEMBER
// to pass the INVERSE view matrix.
float3 oGetEyePosition(float4x4 _InverseViewMatrix)
{
	return oMul(_InverseViewMatrix, float4(0.0, 0.0, 0.0, 1.0)).xyz;
}

// When writing a normal to a screen buffer, it's not useful to have normals that
// point away from the screen and thus won't be evaluated, so get that precision
// back by mapping a normal that could point anywhere on a unit sphere into a 
// half-sphere.
float2 oFullToHalfSphere(float3 _Normal)
{
	// From Inferred Lighting, Kicher, Lawrance @ Volition
	// But modified to be left-handed
	return normalize(-_Normal + oVECTOR_TOWARDS_SCREEN).xy;
}

// Given the XY of a normal, recreate Z and remap from a half-sphere to a full-
// sphere normal.
float3 oHalfToFullSphere(float2 Nxy)
{
	// Restores Z value from a normal's XY on a half sphere
	float z = sqrt(1 - dot(Nxy, Nxy));
	return -float3(2 * z * Nxy.x, 2 * z * Nxy.y, (2 * z * z) - 1);
}

// Given the rotation of a normal from oVECTOR_UP, create a half-
// sphere encoded version fit for use in deferred rendering
float2 oEncodeQuaternionNormal(float4 _NormalRotationQuaternion, bool _IsFrontFace)
{
	float3 up = _IsFrontFace ? oVECTOR_UP : -oVECTOR_UP;
	return oFullToHalfSphere(oQRotate(normalize(_NormalRotationQuaternion), up));
}

// Returns a normal as encoded by oEncodeQuaternionNormal()
float3 oDecodeQuaternionNormal(float2 _EncodedQuaternionNormal)
{
	return oHalfToFullSphere(_EncodedQuaternionNormal);
}

// _____________________________________________________________________________
// Color space tranformation

// Shininess is a float value on [0,1] that describes
// a value between minimum (0) specular and a maximum 
// (system-defined) specular exponent value.
uint oEncodeShininess(float _Shininess)
{
	return _Shininess * 255.0;
}

float oDecodeShininess(uint _EncodedShininess)
{
	return _EncodedShininess / 255.0f;
}

float oConvertShininessToSpecularExponent(float _Shininess)
{
	return _Shininess * oMAX_SPECULAR_EXPONENT;
}

// _____________________________________________________________________________
// Texture Sampling

// Returns 3 values; x contains the integer high mip, y the next-lower level 
// mip, and z contains the trilinear filter value
float3 oCalcMipSelection(float2 _Texcoord)
{
	float2 delta = float2(length(ddx(_Texcoord)), length(ddy(_Texcoord)));
	float mip;
	float trilinInterp = modf(-log2(max(delta.x, delta.y)), mip);
	return float3(mip, mip+1, trilinInterp);
}

// Samples a displacement maps texel's 4 neighbors to generate a normalized 
// normal across the surface
float3 oCrossSampleNormal(Texture2D _Texture, SamplerState _Sampler, float2 _Texcoord, float _NormalScale)
{
	float w, h;
	_Texture.GetDimensions(w, h);
	float2 texelSize = 1.0 / float2(w, h);

	float T = _Texture.Sample(_Sampler, _Texcoord + float2(0, 1) * texelSize).x;
	float L = _Texture.Sample(_Sampler, _Texcoord + float2(-1, 0) * texelSize).x;
	float R = _Texture.Sample(_Sampler, _Texcoord + float2(1, 0) * texelSize).x;
	float B = _Texture.Sample(_Sampler, _Texcoord + float2(0, -1) * texelSize).x;
	return normalize(float3(R-L, 2 * _NormalScale, B-T));
}

// Samples a displacement maps texel's 8 neighbors to generate a normalized 
// normal across the surface
float3 oSobelSampleNormal(Texture2D _Texture, SamplerState _Sampler, float2 _Texcoord, float _NormalScale)
{
	float w, h;
	_Texture.GetDimensions(w, h);
	float2 texelSize = 1.0 / float2(w, h);

	float tl = _Texture.Sample(_Sampler, _Texcoord + float2(-1, 1) * texelSize).x;
	float T = _Texture.Sample(_Sampler, _Texcoord + float2(0, 1) * texelSize).x;
	float tr = _Texture.Sample(_Sampler, _Texcoord + float2(1, 1) * texelSize).x;
	float L = _Texture.Sample(_Sampler, _Texcoord + float2(-1, 0) * texelSize).x;
	float R = _Texture.Sample(_Sampler, _Texcoord + float2(1, 0) * texelSize).x;
	float bl = _Texture.Sample(_Sampler, _Texcoord + float2(-1, -1) * texelSize).x;
	float B = _Texture.Sample(_Sampler, _Texcoord + float2(0, -1) * texelSize).x;
	float br = _Texture.Sample(_Sampler, _Texcoord + float2(1, -1) * texelSize).x;

	return normalize(float3(
		tr + 2*R + br - tl - 2*L - bl, 
		1 / _NormalScale,
		bl + 2*B + br - tl - 2*T - tr));
}

float3 oSobelSampleNormalATI(Texture2D _Texture, SamplerState _Sampler, float2 _Texcoord, float4 _Lightness, float2 _TextureDimensions)
{
	// From RenderMonkey code, which is the GPU version of ATI's

// The Sobel filter extracts the first order derivates of the image,
// that is, the slope. The slope in X and Y directon allows us to
// given a heightmap evaluate the normal for each pixel. This is
// the same this as ATI's NormalMapGenerator application does,
// except this is in hardware.
//
// These are the filter kernels:
//
//  SobelX       SobelY
//  1  0 -1      1  2  1
//  2  0 -2      0  0  0
//  1  0 -1     -1 -2 -1

	float2 off = 1.0 / _TextureDimensions;

	// Take all neighbor samples
	float4 s00 = _Texture.Sample(_Sampler, _Texcoord + -off);
	float4 s01 = _Texture.Sample(_Sampler, _Texcoord + float2(0, -off.y));
	float4 s02 = _Texture.Sample(_Sampler, _Texcoord + float2(off.x, -off.y));

	float4 s10 = _Texture.Sample(_Sampler, _Texcoord + float2(-off.x, 0));
	float4 s12 = _Texture.Sample(_Sampler, _Texcoord + float2(off.x, 0));

	float4 s20 = _Texture.Sample(_Sampler, _Texcoord + float2(-off.x, off.y));
	float4 s21 = _Texture.Sample(_Sampler, _Texcoord + float2(0, off.y));
	float4 s22 = _Texture.Sample(_Sampler, _Texcoord + off);

	// Slope in X direction
	float4 sobelX = s00 + 2 * s10 + s20 - s02 - 2 * s12 - s22;
	// Slope in Y direction
	float4 sobelY = s00 + 2 * s01 + s02 - s20 - 2 * s21 - s22;

	// Weight the slope in all channels, we use grayscale as height
	float sx = dot(sobelX, _Lightness);
	float sy = dot(sobelY, _Lightness);

	// Compose the normal
	float3 normal = normalize(float3(sx, sy, 1));

	// Pack [-1, 1] into [0, 1]
	return normal * 0.5 + 0.5;
}

// Returns RGB as sampled from YUV sources in NV12 format (1 1-channel and 1 2-
// channel textures)
float3 oYUVSampleNV12(Texture2D _Y, Texture2D _UV, SamplerState _Sampler, float2 _Texcoord)
{
	float y = _Y.Sample(_Sampler, _Texcoord).x;
	float2 uv = _UV.Sample(_Sampler, _Texcoord).xy;
	return oYUVToRGB(float3(y, uv.x, uv.y));
}

float4 oYUVSampleNV12A(Texture2D _YA, Texture2D _UV, SamplerState _Sampler, float2 _Texcoord)
{
	float2 ya = _YA.Sample(_Sampler, _Texcoord).xy;
	float2 uv = _UV.Sample(_Sampler, _Texcoord).xy;
	return float4(oYUVToRGB(float3(ya.x, uv.x, uv.y)), ya.y);
}

// _____________________________________________________________________________
// Lighting

// Handedness is stored in w component of _LSTangent. This should be called out 
// of the vertex shader and the out values passed through to the pixel shader.
// Read more:
// http://www.terathon.com/code/tangent.html.
void oCalcWSTangentBasisVectors(in float4x4 _WorldInverseTranspose, in float3 _LSNormal, in float4 _LSTangent, out float3 _OutWSNormal, out float3 _OutWSTangent, out float3 _OutWSBitangent)
{
	_OutWSNormal = oMul(_WorldInverseTranspose, float4(_LSNormal, 1)).xyz;
	_OutWSTangent = oMul(_WorldInverseTranspose, float4(_LSTangent.xyz, 1)).xyz;
	_OutWSBitangent = cross(_OutWSNormal, _OutWSTangent) * _LSTangent.w;
}

// Returns an unnormalized normal in world space. If the return value is to be
// passed onto a Toksvig-scaled lighting model, then the input vertex vectors do 
// not need to be normalized.
float3 oDecodeTSNormal(
	float3 _WSVertexTangent
	, float3 _WSVertexBitangent
	, float3 _WSVertexNormal
	, float3 _TSNormalMapColor
	, float _BumpScale)
{
	const float3 TSNormal = _TSNormalMapColor*2-1;
	return TSNormal.x*_BumpScale*_WSVertexTangent + TSNormal.y*_BumpScale*_WSVertexBitangent + TSNormal.z*_WSVertexNormal;
}

// Classic OpenGL style attenuation
float oCalcAttenuation(float _ConstantFalloff, float _LinearFalloff, float _QuadraticFalloff, float _LightDistance)
{
	return saturate(1 / (_ConstantFalloff + _LinearFalloff*_LightDistance + _QuadraticFalloff*_LightDistance*_LightDistance));
}

// Input vectors are assumed to be normalized. Once NdotL is valid for lighting
// (i.e. this can be skipped if not) then this further culls against the cone
// of a spot light. The scalar returned should be multiplied against all 
// lighting values affected by the spotlight, (i.e. diffuse and specular, etc.)
float oCalcSpotlightAttenuation(float3 _SpotlightVector, float3 _LightVector, float _CosCutoffAngle, float _SpotExponent)
{
	float s = dot(_SpotlightVector, _LightVector);
	return (s > _CosCutoffAngle) ? pow(s, _SpotExponent) : 0;
}

// Attenuates quadratically to a specific bounds of a light. This is useful for
// screen-space lighting where it is desirable to minimize the number of pixels
// touched, so the light's effect is 0 at _LightRadius.
float oCalculateBoundQuadraticAttenuation(float _LightDistanceFromSurface, float _LightRadius, float _Cutoff)
{
	// Based on: http://imdoingitwrong.wordpress.com/2011/01/31/light-attenuation/
	float denom = (_LightDistanceFromSurface / _LightRadius) + 1;
	float attenuation = 1 / (denom * denom);
	return max((attenuation - _Cutoff) / (1 - _Cutoff), 0);
}

// This is a companion function to oCalculateBoundQuadraticAttenuation and
// can be used to determine the radius of a screen-space circle inscribed in
// a quad that will optimally fit exactly the falloff of a point light.
float oCalculateMaximumDistanceForPointLight(float _LightRadius, float _Cutoff)
{
	return _LightRadius * (sqrt(1 / _Cutoff) - 1);
}

float oCalcLinearFogRatio(float _EyeDistance, float _FogStart, float _FogDistance)
{
	return saturate((_EyeDistance - _FogStart) / _FogDistance);
}

// Returns NdotL in x and NdotH in y (not clamped in any way). All vectors are 
// assumed to be normalized.
float2 oLambert(float3 _SurfaceNormal, float3 _LightVector, float3 _EyeVector)
{
	return float2(dot(_SurfaceNormal, _LightVector), dot(_SurfaceNormal, normalize(_LightVector + _EyeVector)));
}

// This equation is from the NVIDIA shader library "lambskin". This is a super-
// trivial poor approximation for sub-surface scatter to achieve a skin-like 
// look. It's included here to be used as a placeholder during renderer bringup.
float oLambertSSS(float _NdotL, float _Rolloff)
{
	return max(0, smoothstep(-_Rolloff, 1, _NdotL) - smoothstep(0, 1, _NdotL));
}

// Toksvig specular scales brightness based on the mipmapping of normal maps. As 
// normal maps mip, they approach a planar mirror, so specular can get 
// disproportionately brighter at distances. The _UnnormalizedNdotNormalizedH is 
// a decoded normal, unnormalized. This means normalization of vertex normal/
// tangent/bitangent can be skipped and used unnormalized to sample a tangent-
// space normal map, which too can remain unnormalized. Use that dotted against 
// a normalized half vector (Hn = dot(normalize(ViewDir), normalize(LightDir))). 
// The second parameter is the length of the unnormalized sampled and 
// transformed normal. Read more:
// http://www.nvidia.com/object/mipmapping_normal_maps.html
// ftp://download.nvidia.com/developer/presentations/GDC_2004/RenderingTechniquesNVIDIA.pdf
// http://developer.download.nvidia.com/shaderlibrary/packages/Toksvig_NormalMaps.zip
float oCalcToksvigSpecular(float _UnnormalizedNdotNormalizedH, float _UnnormalizedNormalLength, float _SpecularExponent)
{
	float toksvig = _UnnormalizedNormalLength/(_UnnormalizedNormalLength+_SpecularExponent*(1-_UnnormalizedNormalLength));
	float AdjustedSpecularExponent = toksvig * _SpecularExponent;
	return (1.0+AdjustedSpecularExponent)/(1.0+_SpecularExponent)* pow(max(0,_UnnormalizedNdotNormalizedH/_UnnormalizedNormalLength), AdjustedSpecularExponent);
}

// Returns the results of HLSL's lit() with the specified parameters. All 
// vectors are assumed to be normalized.
float4 oLit(float3 _SurfaceNormal, float3 _LightVector, float3 _EyeVector, float _SpecularExponent)
{
	float2 L = oLambert(_SurfaceNormal, _LightVector, _EyeVector);
	return lit(L.x, L.y, _SpecularExponent);
}

// Returns results similar to HLSL's lit(), but uses Toksvig's math to calculate
// a more appealing specular value based on mip'ing normal maps that approach
// perfect mirrors for higher mip levels. Use this instead of oLit when using
// normal maps with a mip chain.
float4 oLitToksvig(float3 _UnnormalizedSurfaceNormal, float3 _NormalizedLightVector, float3 _NormalizedEyeVector, float _SpecularExponent)
{
	float NLen = length(_UnnormalizedSurfaceNormal);
	float NDotL = dot(_UnnormalizedSurfaceNormal / NLen, _NormalizedLightVector);
	float Kd = max(0, NDotL);
	float3 HalfVector = normalize(_NormalizedLightVector + _NormalizedEyeVector);
	float NdotH = dot(_UnnormalizedSurfaceNormal, HalfVector);
	float Ks = (NDotL < 0) || (NdotH < 0) ? 0 : oCalcToksvigSpecular(NdotH, NLen, _SpecularExponent);
	return float4(1, Kd, Ks, 1);
}

// A variation on oLit that uses a softening of the NdotL ratio to give what 
// some might consider a more appealing falloff.
float4 oLitHalfLambert(float3 _SurfaceNormal, float3 _LightVector, float3 _EyeVector, float _SpecularExponent)
{
	float2 L = oLambert(_SurfaceNormal, _LightVector, _EyeVector);
	float Kd = max(L.x*0.5+0.5, 0);
	float Ks = pow(saturate(L.y), _SpecularExponent);
	return float4(1, Kd, Ks, 1);
}

float3 oPhongShade(float4 _Lit // results of lit (or oLit variants)
	, float _Attenuation // result of oCalcAttenuation
	, float3 _Ka // ambient color
	, float3 _Ke // emissive color
	, float3 _Kd // diffuse color
	, float3 _Ks // specular color
	, float3 _Kt // transmissive color
	, float3 _Kr // reflective color
	, float3 _Kl // light color
	, float _Ksh // shadow term (1 = unshadowed, 0 = shadowed)
	)
{
	float diffuseCoeff = _Lit.y;
	float ambientCoeff = _Lit.x * (1-diffuseCoeff); // so that in-shadow bump mapping isn't completely lost
	float specularCoeff = _Lit.z;
	float attenuationCoeff = _Ksh * _Attenuation;

	float3 rgb = ambientCoeff * _Ka * _Kd + _Ke + attenuationCoeff * _Kl * (diffuseCoeff * _Kd + specularCoeff * _Ks);

	// @oooii-tony: TODO: add transmissive and reflective

	return rgb;
}

// Returns a color resulting from the input parameters consistent with the Phong
// shading model.
float3 oPhongShade(float3 _SurfaceNormal // assumed to be normalized, pointing out from the surface
	, float3 _LightVector // assumed to be normalized, pointing from surface to light
	, float3 _EyeVector // assumed to be normalized, pointing from surface to eye
	, float _Attenuation // result of oCalcAttenuation
	, float3 _Ka // ambient color
	, float3 _Ke // emissive color
	, float3 _Kd // diffuse color
	, float3 _Ks // specular color
	, float _Kh // specular exponent
	, float3 _Kt // transmissive color
	, float3 _Kr // reflective color
	, float3 _Kl // light color
	, float _Ksh // shadow term (1 = unshadowed, 0 = shadowed)
	)
{
	float4 Lit = oLit(_SurfaceNormal, _LightVector, _EyeVector, _Kh);
	return oPhongShade(Lit, _Attenuation, _Ka, _Ke, _Kd, _Ks, _Kt, _Kr, _Kl, _Ksh);
}

// Returns a color resulting from the input parameters consistent with the Phong
// shading model.
float3 oPhongShadeToksvig(float3 _UnnormalizedSurfaceNormal // assumed to be sampled from a normal map with mips
	, float3 _LightVector // assumed to be normalized, pointing from surface to light
	, float3 _EyeVector // assumed to be normalized, pointing from surface to eye
	, float _Attenuation // result of oCalcAttenuation
	, float3 _Ka // ambient color
	, float3 _Ke // emissive color
	, float3 _Kd // diffuse color
	, float3 _Ks // specular color
	, float _Kh // specular exponent
	, float3 _Kt // transmissive color
	, float3 _Kr // reflective color
	, float3 _Kl // light color
	, float _Ksh // shadow term (1 = unshadowed, 0 = shadowed)
	)
{
	float4 Lit = oLitToksvig(_UnnormalizedSurfaceNormal, _LightVector, _EyeVector, _Kh);
	return oPhongShade(Lit, _Attenuation, _Ka, _Ke, _Kd, _Ks, _Kt, _Kr, _Kl, _Ksh);
}

// Returns a color resulting from the input parameters consistent with the Phong
// shading model.
float3 oPhongShadeHalfLambert(float3 _SurfaceNormal // assumed to be normalized, pointing out from the surface
	, float3 _LightVector // assumed to be normalized, pointing from surface to light
	, float3 _EyeVector // assumed to be normalized, pointing from surface to eye
	, float _Attenuation // result of oCalcAttenuation
	, float3 _Ka // ambient color
	, float3 _Ke // emissive color
	, float3 _Kd // diffuse color
	, float3 _Ks // specular color
	, float _Kh // specular exponent
	, float3 _Kt // transmissive color
	, float3 _Kr // reflective color
	, float3 _Kl // light color
	, float _Ksh // shadow term (1 = unshadowed, 0 = shadowed)
	)
{
	float4 Lit = oLitHalfLambert(_SurfaceNormal, _LightVector, _EyeVector, _Kh);
	return oPhongShade(Lit, _Attenuation, _Ka, _Ke, _Kd, _Ks, _Kt, _Kr, _Kl, _Ksh);
}

// A naive global illumination approximation
float4 oHemisphericShade(float3 _SurfaceNormal // assumed to be normalized, pointing out from the surface
	, float3 _SkyVector // assumed to be normalized, vector pointing at the sky color (and away from the ground color)
	, float4 _HemisphericSkyColor // color of the hemisphere in the direction pointed to by _SkyVector
	, float4 _HemisphericGroundColor // color of the hemisphere in the direction pointed away from by _SkyVector
	)
{
	return lerp(_HemisphericGroundColor, _HemisphericSkyColor, dot(_SurfaceNormal, _SkyVector) * 0.5f + 0.5f);
}

// Returns a multiplier on where a uniform grid exists. Good starting values for 
// parameters are: _GridRepeat = 3, _Thickness = 1.2
float oCalcGridShadeIntensity2D(float2 _Coord, float _GridRepeat, float _Thickness)
{
	// thickness-derived values that can be constants
	float mi = max(0.0, _Thickness - 1.0);
	float miInv = max(0.0, 1.0 - _Thickness);
	float ma = max(1.0, _Thickness);

	float2 f  = abs(frac(_Coord * _GridRepeat) - 0.5);
	float2 df = fwidth(_Coord * _GridRepeat);
	float2 g = clamp((f - df * mi) / (0.0001 + df * (ma - mi)), miInv, 1.0);
	return 1 - (g.x * g.y);
}

float oCalcGridShadeIntensity3D(float3 _Coord, float _GridRepeat, float _Thickness)
{
	// thickness-derived values that can be constants
	float mi = max(0.0, _Thickness - 1.0);
	float miInv = max(0.0, 1.0 - _Thickness);
	float ma = max(1.0, _Thickness);

	float3 f  = abs(frac(_Coord * _GridRepeat) - 0.5);
	float3 df = fwidth(_Coord * _GridRepeat);
	float3 g = clamp((f - df * mi) / (0.0001 + df * (ma - mi)), miInv, 1.0);
	return g.x * g.y * g.z;
}

// Returns a multiplier on where a uniform grid exists. This tries to maintain
// a uniform density of grid sizes by calculating mip levels for a surface and
// using that to create another level of gridding (quad-tree style). A trilinear
// calculation is used to fade LODs of the grid in and out. Good starting values
// for parameters are: _GridRepeat = 0.2, _Intensity = 10, _Thickness = 0.05
float oCalcFadingGridShadeIntensity2D(float2 _Texcoord, float _GridRepeat, float _Intensity, float _Thickness)
{
	// Based on article at:
	// http://www.gamedev.net/topic/618788-efficient-drawing-of-2d-gridlines-with-shaders/

	float3 mipSel = oCalcMipSelection(_Texcoord);
	float2 TexcoordThisMip = _Texcoord * pow(2, mipSel.x);
	float2 TexcoordNextMip = _Texcoord * pow(2, mipSel.y);

	float2 HighlightThisMip = saturate(cos(TexcoordThisMip * _GridRepeat) - 1 + _Thickness) * _Intensity;
	float2 HighlightNextMip = saturate(cos(TexcoordNextMip * _GridRepeat) - 1 + _Thickness) * _Intensity;

	// lerp between the intensities
	return saturate(lerp(max(HighlightThisMip.x, HighlightThisMip.y), max(HighlightNextMip.x, HighlightNextMip.y), mipSel.z));
}

float oCalcFadingGridShadeIntensity3D(float3 _Coord, float _GridRepeat, float _Intensity, float _Thickness)
{
	// Based on article at:
	// http://www.gamedev.net/topic/618788-efficient-drawing-of-2d-gridlines-with-shaders/

	float3 mipSel = oCalcMipSelection(_Coord.xy); // @oooii-tony: This might need to be transformed into view-space

	float3 TexcoordThisMip = _Coord * pow(2, mipSel.x);
	float3 TexcoordNextMip = _Coord * pow(2, mipSel.y);

	float3 HighlightThisMip = saturate(cos(TexcoordThisMip * _GridRepeat) - 1 + _Thickness) * _Intensity;
	float3 HighlightNextMip = saturate(cos(TexcoordNextMip * _GridRepeat) - 1 + _Thickness) * _Intensity;

	// lerp between the intensities
	return saturate(lerp(max(max(HighlightThisMip.x, HighlightThisMip.y), HighlightThisMip.z), max(max(HighlightNextMip.x, HighlightNextMip.y), HighlightNextMip.z), mipSel.z));
}

// Returns a multiplier between 0 and 1 in a checkerboard pattern. Good starting 
// values: _Scale = 0.5, _Balance = 0.5, _AAFilterWidth = 1
float oCalcCheckerIntensity1D(float _Coord, float _Scale, float _Balance, float _AAFilterWidth)
{
	// Based on NVIDIA's checker3d_math.fx shader
	// http://developer.download.nvidia.com/shaderlibrary/webpages/hlsl_shaders.html

	float edge = _Scale * _Balance;
	float op = _AAFilterWidth / _Scale;

	float width = fwidth(_Coord);
	float w = width*op;
	float x0 = _Coord / _Scale - (w/2.0);
	float x1 = x0 + w;
	float nedge = edge/_Scale;
	float i0 = (1.0-nedge)*floor(x0) + max(0.0, frac(x0)-nedge);
	float i1 = (1.0-nedge)*floor(x1) + max(0.0, frac(x1)-nedge);
	return saturate((i1 - i0)/ (0.00001 + w));
}

float oCalcCheckerIntensity2D(float2 _Space, float _Scale, float _Balance, float _AAFilterWidth)
{
	float edge = _Scale*_Balance;
	float op = _AAFilterWidth/_Scale;

	float2 check = float2(
		oCalcCheckerIntensity1D(_Space.x, _Scale, _Balance, _AAFilterWidth),
		oCalcCheckerIntensity1D(_Space.y, _Scale, _Balance, _AAFilterWidth));

	return abs(check.y - check.x);
}

float oCalcCheckerIntensity3D(float3 _Space, float _Scale, float _Balance, float _AAFilterWidth)
{
	float edge = _Scale*_Balance;
	float op = _AAFilterWidth/_Scale;

	float3 check = float3(
		oCalcCheckerIntensity1D(_Space.x, _Scale, _Balance, _AAFilterWidth),
		oCalcCheckerIntensity1D(_Space.y, _Scale, _Balance, _AAFilterWidth),
		oCalcCheckerIntensity1D(_Space.z, _Scale, _Balance, _AAFilterWidth));

	return abs(check.z - abs(check.y - check.x));
}

// _____________________________________________________________________________
// Cube GS code

// Creates a cube in a geometry shader (useful for voxel visualization. To use,
// generate indices in a for (uint i = 0; i < 14; i++) loop (14 iterations).
float3 oGSCubeCalcVertexPosition(uint _Index, float3 _Offset, float3 _Scale)
{
	static const float3 oGSCubeStripCW[] = 
	{
		float3(-0.5f,0.5f,-0.5f), // left top front
		float3(0.5f,0.5f,-0.5f), // right top front
		float3(-0.5f,-0.5f,-0.5f), // left bottom front
		float3(0.5f,-0.5f,-0.5f), // right bottom front
		float3(0.5f,-0.5f,0.5f), //right bottom back
		float3(0.5f,0.5f,-0.5f), //right top front
		float3(0.5f,0.5f,0.5f), //right top back
		float3(-0.5f,0.5f,-0.5f), //left top front
		float3(-0.5f,0.5f,0.5f), //left top back
		float3(-0.5f,-0.5f,-0.5f), //left bottom front 7
		float3(-0.5f,-0.5f,0.5f), //left bottom back
		float3(0.5f,-0.5f,0.5f), //right bottom back 5
		float3(-0.5f,0.5f,0.5f), //left top back
		float3(0.5f,0.5f,0.5f), //right top back
	};

	return _Offset + oGSCubeStripCW[_Index] * _Scale;
}

#endif
#endif