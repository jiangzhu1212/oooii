// $(header)
// NOTE: This header is compiled both by HLSL and C++
#ifndef oHLSL
	#pragma once
#endif
#ifndef oHLSL_h
#define oHLSL_h

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

#ifndef oHLSL

	#include <oooii/oColor.h>
	#include <oooii/oMath.h>

	struct oHLSLColor
	{
		// Handle automatic expansion of oColor to float3s. Using this
		// type allows for a single header to be defined for usage in
		// both C++ and HLSL.

		oHLSLColor() : Color(0.0f, 0.0f, 0.0f) {}
		oHLSLColor(const float3& _Color) : Color(_Color) {}
		oHLSLColor(const oColor& _Color) { float a; oDecomposeColor(_Color, &Color.x, &Color.y, &Color.z, &a); }

		inline operator float3&() { return Color; }
		inline operator const float3&() const { return Color; }
		inline operator oColor() const { return oComposeColor(Color.x, Color.y, Color.z, 1.0f); }
		inline const oHLSLColor& operator=(const oHLSLColor& _Color) { Color = _Color.Color; return *this; }
		inline const oHLSLColor& operator=(const float3& _Color) { Color = _Color; return *this; }
		inline const oHLSLColor& operator=(const oColor& _Color) { float a; oDecomposeColor(_Color, &Color.x, &Color.y, &Color.z, &a); return *this; }
	protected:
		float3 Color;
	};

	#define oHLSL_REQUIRED_STRUCT_ALIGNMENT 16
	#define oHLSLCheckSize(_Struct) oSTATICASSERT(sizeof(_Struct) % oHLSL_REQUIRED_STRUCT_ALIGNMENT) == 0);

#else

#define quatf float4
#define oHLSLColor float3

#ifdef oRIGHTHANDED
	static const float3 oVECTOR_TOWARDS_SCREEN = float3(0,0,1);
	static const float3 oVECTOR_INTO_SCREEN = float3(0,0,-1);
#else
	static const float3 oVECTOR_TOWARDS_SCREEN = float3(0,0,-1);
	static const float3 oVECTOR_INTO_SCREEN = float3(0,0,1);
#endif
static const float3 oVECTOR_UP = float3(0,1,0);

static const float4 oZERO = float4(0,0,0,0);
static const float4 oBLACK = float4(0,0,0,1);
static const float4 oWHITE = float4(1,1,1,1);
static const float4 oRED = float4(1,0,0,1);
static const float4 oGREEN = float4(0,1,0,1);
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

// Returns the eye position in whatever space the view matrix is in.
float3 oGetEyePosition(float4x4 _ViewMatrix)
{
	#ifdef oMATRIX_COLUMN_MAJOR
		return -float3(_ViewMatrix[0].w, _ViewMatrix[1].w, _ViewMatrix[2].w);
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

// Converts a 3D normalized vector into an RGB color
// (typically for encoding a normal)
float3 oColorizeVector(float3 _NormalizedVector)
{
	return _NormalizedVector * float3(0.5, 0.5, -0.5) + 0.5;
}

// Converts a normalized vector stored as RGB color
// back to a vector
float3 oDecolorizeVector(float3 _RGBVector)
{
	return _RGBVector * float3(2.0, 2.0, -2.0) - 1;
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
	uint R = oUnmaskedRand(ID8Bit);
	uint G = oUnmaskedRand(R);
	uint B = oUnmaskedRand(G);
	return (uint3(R,G,B) & 0xff) / 255.0;
}

// Given an integer ID [0,65535], return a color that ensures IDs near each other 
// (i.e. 13,14,15) have significantly different colors.
float3 oIDtoColor16Bit(uint ID16Bit)
{
	uint R = oUnmaskedRand(ID16Bit);
	uint G = oUnmaskedRand(R);
	uint B = oUnmaskedRand(G);
	return (uint3(R,G,B) & 0xffff) / 65535.0;
}

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

// Classic OpenGL style attenuation
float oCalculateAttenuation(float ConstantFalloff, float LinearFalloff, float QuadraticFalloff, float LightDistance)
{
	return saturate(1 / (ConstantFalloff + LinearFalloff*LightDistance + QuadraticFalloff*LightDistance*LightDistance));
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

// Returns the results of HLSL's lit() with the specified parameters. All 
// vectors are assumed to be normalized.
float4 oLit(float3 _SurfaceNormal, float3 _LightVector, float3 _EyeVector, float _SpecularExponent)
{
	float NdotL = dot(_SurfaceNormal, _LightVector);
	float NdotH = dot(_SurfaceNormal, normalize(_LightVector + _EyeVector));
	return lit(NdotL, NdotH, _SpecularExponent);
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

	float4 Lit = oLit(_SurfaceNormal, _LightVector, _EyeVector, _Kh);
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

#endif
#endif
