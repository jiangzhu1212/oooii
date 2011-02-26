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
// NOTE: This is a header of utility functions for HLSL (not C++)
#ifndef oHLSL_h
#define oHLSL_h

#ifndef oMATRIX_COLUMN_MAJOR
	#define oMATRIX_COLUMN_MAJOR 
#endif

#ifndef oRIGHTHANDED
	#define oRIGHTHANDED
#endif

#ifdef oRIGHTHANDED
	static const float3 oVECTOR_TOWARDS_SCREEN = float3(0,0,1);
	static const float3 oVECTOR_INTO_SCREEN = float3(0,0,-1);
#else
	static const float3 oVECTOR_TOWARDS_SCREEN = float3(0,0,-1);
	static const float3 oVECTOR_INTO_SCREEN = float3(0,0,1);
#endif
static const float3 oVECTOR_UP = float3(0,1,0);

static const float4 oBLACK = float4(0,0,0,1);
static const float4 oWHITE = float4(1,1,1,1);
static const float4 oRED = float4(1,0,0,1);
static const float4 oGREEN = float4(0,0,1,1);
static const float4 oBLUE = float4(0,0,1,1);
static const float4 oYELLOW = float4(1,1,0,1);
static const float4 oMAGENTA = float4(1,0,1,1);
static const float4 oCYAN = float4(0,1,1,1);

// To encapsulate matrix library math differences and enforce one ordering of 
// multiplication, use oMul instead of mul
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

// Returns texcoords where the lower left of the render target is 0,0 and the 
// upper right is 1,1 given the SV_POSITION (projected position) value and the
// dimensions of the render target itself.
float2 oGetScreenSpaceTexcoord(float4 _SVPosition, float2 _RenderTargetDimensions)
{
	return float2(_SVPosition.x / _RenderTargetDimensions.x, _SVPosition.y / -_RenderTargetDimensions.y);
}

// Same as above, but 0,0 is in upper left, 1,1 in lower right
float2 oGetScreenSpaceTexcoordInvertedY(float4 _SVPosition, float2 _RenderTargetDimensions)
{
	return _SVPosition.xy / _RenderTargetDimensions;
}

// Returns the eye position in whatever space the view matrix is in.
float3 oGetEyePosition(float4x4 _ViewMatrix)
{
#ifdef oMATRIX_COLUMN_MAJOR
	return -float3(_ViewMatrix[0].w, _ViewMatrix[1].w, _ViewMatrix[2].w );
#else
	return -_ViewMatrix[3].xyz;
#endif
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

// Convert from HSV (HSL) color space to RGB
float3 oHSVtoRGB(float3 HSV)
{
	// http://chilliant.blogspot.com/2010/11/rgbhsv-in-hlsl.html
	float R = abs(HSV.x * 6 - 3) - 1;
	float G = 2 - abs(HSV.x * 6 - 2);
	float B = 2 - abs(HSV.x * 6 - 4);
	return ((saturate(float3(R,G,B)) - 1) * HSV.y + 1) * HSV.z;
}

float oRGBtoLuminance(float3 color)
{
	// from http://en.wikipedia.org/wiki/Luminance_(relative)
	// "For RGB color spaces that use the ITU-R BT.709 primaries 
	// (or sRGB, which defines the same primaries), relative 
	// luminance can be calculated from linear RGB components:"
	color *= float3(0.2126, 0.7152, 0.0722);
	return color.r + color.g + color.b;
}

// A simple LCG rand() function, unshifted/masked
uint oUnmaskedRand(uint Seed)
{
	return 1103515245 * Seed + 12345;
}

// Given an integer ID [0,255], return a color that ensures IDs near each other 
// (i.e. 13,14,15) have significantly different colors.
float3 oIDtoColor8Bit(uint ID8Bit)
{
	uint H = oUnmaskedRand(ID8Bit+1); // +1 ensures a non-black color for 0
	uint S = oUnmaskedRand(H);
	uint V = oUnmaskedRand(S);

	float3 HSV = ((uint3(H,S,V) >> 16) & 0xff) / 255.0;
	return oHSVtoRGB(HSV);
}

// Given an integer ID [0,65535], return a color that ensures IDs near each other 
// (i.e. 13,14,15) have significantly different colors.
float3 oIDtoColor16Bit(uint ID16Bit)
{
	uint H = oUnmaskedRand(ID16Bit+1); // +1 ensures a non-black color for 0
	uint S = oUnmaskedRand(H);
	uint V = oUnmaskedRand(S);

	float3 HSV = ((uint3(H,S,V) >> 16) & 0xffff) / 65535.0;
	return oHSVtoRGB(HSV);
}

float oCalcAttenuation(float ConstantFalloff, float LinearFalloff, float QuadraticFalloff, float LightDistance)
{
	return saturate(1 / (ConstantFalloff + LinearFalloff*LightDistance + QuadraticFalloff*LightDistance*LightDistance));
}

float oCalcLinearFogRatio(float _EyeDistance, float _FogStart, float _FogDistance)
{
	return saturate((_EyeDistance - _FogStart) / _FogDistance);
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

// Returns a color resulting from the input parameters consistent with the Phong
// shading model.
float4 oPhongShade(float3 _SurfaceNormal // assumed to be normalized, pointing out from the surface
									, float3 _LightVector // assumed to be normalized, pointing from surface to light
									, float3 _EyeVector // assumed to be normalized, pointing from surface to eye
									, float _Attenuation // result of oCalcAttenuation
									, float4 _Ka // ambient color
									, float4 _Ke // emissive color
									, float4 _Kd // diffuse color
									, float4 _Ks // specular color
									, float _Kh // specular exponent
									, float4 _Kt // transmissive color
									, float4 _Kr // reflective color
									, float3 _Kl // light color
									, float _Ksh // shadow term (1 = unshadowed, 0 = shadowed)
									)
{
	// NOTE: At the moment I assume passing constants for any of these terms will 
	// cause the compiler to optimize those constants out. If not, decorate this 
	// with #ifdefs accordingly.

	float NdotL = dot(_SurfaceNormal, _LightVector);
	float NdotH = dot(_SurfaceNormal, normalize(_LightVector + _EyeVector));
	float4 Lit = lit(NdotL, NdotH, _Kh);
	float diffuseCoeff = Lit.y;
	float ambientCoeff = 1-diffuseCoeff; // so that in-shadow bump mapping isn't completely lost
	float specularCoeff = Lit.z;
	float attenuationCoeff = _Ksh * _Attenuation;

	float3 rgb = ambientCoeff * _Ka.rgb * _Kd.rgb
		+ _Ke.rgb
		+ attenuationCoeff * _Kl.rgb * (diffuseCoeff * _Kd.rgb + specularCoeff * _Ks.rgb);

	// @oooii-tony: TODO: add transmissive and reflective

	float alpha = _Ka.a * _Ke.a * _Kd.a * _Ks.a * _Kr.a;
	return float4(rgb, alpha);
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

#endif
