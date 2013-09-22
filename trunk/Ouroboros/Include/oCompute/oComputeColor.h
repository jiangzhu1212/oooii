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
// This code contains code that cross-compiles in C++ and HLSL. This contains
// utilities for working with color, or colorizing other types of data.
#ifndef oHLSL
	#pragma once
#endif
#ifndef oComputeColor_h
#define oComputeColor_h

#include <oHLSL/oHLSLMacros.h>
#include <oHLSL/oHLSLMath.h>
#include <oCompute/oComputeUtil.h>

#ifdef oHLSL
	#define oRGBf float3
	#define oRGBAf float4

#else

#include <oBase/color.h>
#include <oBase/operators.h>

#include <oHLSL/oHLSLSwizzlesOn.h>

class oRGBf
{
	// Handle automatic expansion of ouro::color to float3s. Using this type
	// allows for a single header to be defined for usage in both C++ and HLSL. 
	// NOTE: Alpha is quietly dropped/ignored when using this.

public:
	float r, g, b;

	oRGBf() : r(0.0f), g(0.0f), b(0.0f) {}
	oRGBf(float _R, float _G, float _B) : r(_R), g(_G), b(_B) {}
	oRGBf(const float3& _Color) : r(_Color.x), g(_Color.y), b(_Color.z) {}
	oRGBf(const ouro::color& _Color) { _Color.decompose(&r, &g, &b); }
	oRGBf(const oRGBf& _RGB) : r(_RGB.r), g(_RGB.g), b(_RGB.b) {}

	inline operator float3&() { return *(float3*)this; }
	inline operator const float3&() const { return *(float3*)this; }
	inline operator float3() const { return *(float3*)this; }
	inline operator ouro::color() const { return ouro::color(r, g, b, 1.0f); }
	inline const oRGBf& operator=(int _Color) { r = static_cast<float>(_Color); g = static_cast<float>(_Color); b = static_cast<float>(_Color); return *this; }
	inline const oRGBf& operator=(const oRGBf& _Color) { r = _Color.r; g = _Color.g; b = _Color.b; return *this; }
	inline const oRGBf& operator=(const float3& _Color) { r = _Color.x; g = _Color.y; b = _Color.z; return *this; }
	inline const oRGBf& operator=(const ouro::color& _Color) { _Color.decompose(&r, &g, &b); return *this; }

	oRGBf& operator+=(const oRGBf& _That) { *this = saturate((const float3&)*this + (const float3&)_That); return *this; }
	oRGBf& operator-=(const oRGBf& _That) { *this = saturate((const float3&)*this - (const float3&)_That); return *this; }
	oRGBf& operator*=(const oRGBf& _That) { *this = saturate((const float3&)*this * (const float3&)_That); return *this; }
	oRGBf& operator/=(const oRGBf& _That) { *this = saturate((const float3&)*this / (const float3&)_That); return *this; }

	oRGBf& operator+=(const float& _That) { operator+=(oRGBf(_That, _That, _That)); return *this; }
	oRGBf& operator-=(const float& _That) { operator-=(oRGBf(_That, _That, _That)); return *this; }
	oRGBf& operator*=(const float& _That) { operator*=(oRGBf(_That, _That, _That)); return *this; }
	oRGBf& operator/=(const float& _That) { operator/=(oRGBf(_That, _That, _That)); return *this; }

	oOPERATORS_DERIVED_COMMUTATIVE2(oRGBf, float, +) oOPERATORS_DERIVED(oRGBf, -) oOPERATORS_DERIVED(oRGBf, *) oOPERATORS_DERIVED(oRGBf, /)

	inline oRGBf operator-(float x) const { return (const float3&)*this - x; }
	inline oRGBf operator*(float x) const { return (const float3&)*this * x; }
	inline oRGBf operator/(float x) const { return (const float3&)*this / x; }

	friend inline oRGBf operator-(float x, const oRGBf& _RGBf) { return oRGBf(x,x,x) - _RGBf; }
	friend inline oRGBf operator*(float x, const oRGBf& _RGBf) { return oRGBf(x,x,x) / _RGBf; }

	inline const float3& as_float3() const { return *(const float3*)this; }
};

class oRGBAf
{
	// Handle automatic expansion of ouro::color to float3s. Using this type 
	// allows for a single header to be defined for usage in both C++ and HLSL. 

public:
	float r, g, b, a;

	oRGBAf() : r(0.0f), g(0.0f), b(0.0f), a(1.0f) {}
	oRGBAf(float _R, float _G, float _B, float _A) : r(_R), g(_G), b(_B), a(_A) {}
	oRGBAf(const float4& _Color) : r(_Color.x), g(_Color.y), b(_Color.z), a(_Color.w) {}
	oRGBAf(const ouro::color& _Color) { _Color.decompose(&r, &g, &b, &a); }
	oRGBAf(const oRGBAf& _RGBA) : r(_RGBA.r), g(_RGBA.g), b(_RGBA.b), a(_RGBA.a) {}

	inline const float3& rgb() const { return *(const float3*)this; }
	inline operator float4&() { return *(float4*)this; }
	inline operator const float4&() const { return *(const float4*)this; }
	inline operator ouro::color() const { return ouro::color(r, g, b, a); }
	inline const oRGBAf& operator=(int _Color) { r = static_cast<float>(_Color); g = static_cast<float>(_Color); b = static_cast<float>(_Color); a = static_cast<float>(_Color); return *this; }
	inline const oRGBAf& operator=(const oRGBAf& _Color) { r = _Color.r; g = _Color.g; b = _Color.b; a = _Color.a; return *this; }
	inline const oRGBAf& operator=(const float4& _Color) { r = _Color.x; g = _Color.y; b = _Color.z; a = _Color.w; return *this; }
	inline const oRGBAf& operator=(const ouro::color& _Color) { _Color.decompose(&r, &g, &b, &a); return *this; }

	oRGBAf& operator+=(const oRGBAf& _That) { *this = saturate((const float4&)*this + (const float4&)_That); return *this; }
	oRGBAf& operator-=(const oRGBAf& _That) { *this = saturate((const float4&)*this - (const float4&)_That); return *this; }
	oRGBAf& operator*=(const oRGBAf& _That) { *this = saturate((const float4&)*this * (const float4&)_That); return *this; }
	oRGBAf& operator/=(const oRGBAf& _That) { *this = saturate((const float4&)*this / (const float4&)_That); return *this; }

	oRGBAf& operator+=(const float& _That) { operator+=(oRGBAf(_That, _That, _That, _That)); return *this; }
	oRGBAf& operator-=(const float& _That) { operator-=(oRGBAf(_That, _That, _That, _That)); return *this; }
	oRGBAf& operator*=(const float& _That) { operator*=(oRGBAf(_That, _That, _That, _That)); return *this; }
	oRGBAf& operator/=(const float& _That) { operator/=(oRGBAf(_That, _That, _That, _That)); return *this; }

	oOPERATORS_DERIVED_COMMUTATIVE2(oRGBAf, float, +) oOPERATORS_DERIVED(oRGBAf, -) oOPERATORS_DERIVED(oRGBAf, *) oOPERATORS_DERIVED(oRGBAf, /)

	inline oRGBAf operator-(float x) const { return (const float4&)*this - x; }
	inline oRGBAf operator*(float x) const { return (const float4&)*this * x; }
	inline oRGBAf operator/(float x) const { return (const float4&)*this / x; }

	friend inline oRGBAf operator-(float x, const oRGBAf& _RGBAf) { return oRGBAf(x,x,x,x) - _RGBAf; }
	friend inline oRGBAf operator*(float x, const oRGBAf& _RGBAf) { return oRGBAf(x,x,x,x) / _RGBAf; }

	inline const float4& as_float4() const { return *(const float4*)this; }
};

inline float min(oIN(oRGBf, _RGB)) { return min(min(_RGB.r, _RGB.g), _RGB.b); }
inline float max(oIN(oRGBf, _RGB)) { return max(max(_RGB.r, _RGB.g), _RGB.b); }
inline float min(oIN(oRGBAf, _RGBA)) { return min(min(min(_RGBA.r, _RGBA.g), _RGBA.b), _RGBA.a); }
inline float max(oIN(oRGBAf, _RGBA)) { return max(max(max(_RGBA.r, _RGBA.g), _RGBA.b), _RGBA.a); }

inline float dot(oIN(oRGBf, _RGB), oIN(float3, _X)) { return dot(_RGB.as_float3(), _X); }
inline float dot(oIN(oRGBAf, _RGBA), oIN(float4, _X)) { return dot(_RGBA.as_float4(), _X); }

#endif

// Converts a 3D normalized vector into an RGB color
// (typically for encoding a normal)
inline oRGBf oColorizeVector(oIN(float3, _NormalizedVector))
{
	return _NormalizedVector * float3(0.5f, 0.5f, -0.5f) + 0.5f;
}

// Converts a normalized vector stored as RGB color
// back to a vector
inline float3 oDecolorizeVector(oIN(oRGBf, _RGBVector))
{
	return _RGBVector * float3(2.0f, 2.0f, -2.0f) - 1.0f;
}

// Convert from HSV color space to RGB
inline oRGBf oHSVtoRGB(oIN(float3, _HSV))
{
	// http://chilliant.blogspot.com/2010/11/rgbhsv-in-hlsl.html
	float R = abs(_HSV.x * 6.0f - 3.0f) - 1.0f;
	float G = 2.0f - abs(_HSV.x * 6.0f - 2.0f);
	float B = 2.0f - abs(_HSV.x * 6.0f - 4.0f);
	return ((saturate(float3(R,G,B)) - 1.0f) * _HSV.y + 1.0f) * _HSV.z;
}

// Convert from RGB color space to HSV
inline float3 oRGBtoHSV(oIN(oRGBf, _RGB))
{
	// http://stackoverflow.com/questions/4728581/hsl-image-adjustements-on-gpu
	float H = 0.0f;
	float S = 0.0f;
	float V = max(_RGB);
	float m = min(_RGB);
	float chroma = V - m;
	S = chroma / V;
	float3 delta = (V - _RGB) / chroma;
	delta -= delta.zxy;
	delta += float3(2.0f, 4.0f, 0.0f);
	float3 choose = float3(
		(_RGB.g == V && _RGB.b != V) ? 1.0f : 0.0f,
		(_RGB.b == V && _RGB.r != V) ? 1.0f : 0.0f,
		(_RGB.r == V && _RGB.g != V) ? 1.0f : 0.0f);
	H = dot(delta, choose);
	H = frac(H / 6.0f);
	return float3(chroma == 0.0f ? 0.0f : H, chroma == 0.0f ? 0.0f : S, V);
}

inline float oRGBtoLuminance(oIN(float3, _Color))
{
	// from http://en.wikipedia.org/wiki/Luminance_(relative)
	// "For RGB color spaces that use the ITU-R BT.709 primaries 
	// (or sRGB, which defines the same primaries), relative 
	// luminance can be calculated from linear RGB components:"
	float3 c = _Color * float3(0.2126f, 0.7152f, 0.0722f);
	return c.x + c.y + c.z;
}

// This isn't quite a color, but since normals are often stored as RGB values
// in render targets, etc., place it here to draw contrast with the RGB version
// that uses a perceptual weighting. For normals, all axes are equal, so the 
// weighting is uniform.
inline float oNormalToLuminance(oIN(float3, _Normal))
{
	return dot(_Normal, float3(0.33333f, 0.33333f, 0.33333f));
}

inline oRGBf oYUVToRGB(oIN(float3, _YUV))
{
	// Using the float version of ITU-R BT.601 that jpeg uses. This is similar to 
	// the integer version, except this uses the full 0 - 255 range.
	static const float3 oITU_R_BT_601_Offset = float3(0.0f, -128.0f, -128.0f) / 255.0f;
	static const float3 oITU_R_BT_601_RFactor = float3(1.0f, 0.0f, 1.402f);
	static const float3 oITU_R_BT_601_GFactor = float3(1.0f, -0.34414f, -0.71414f);
	static const float3 oITU_R_BT_601_BFactor = float3(1.0f, 1.772f, 0.0f);
	float3 yuv = _YUV + oITU_R_BT_601_Offset;
	return saturate(float3(dot(yuv, oITU_R_BT_601_RFactor), dot(yuv, oITU_R_BT_601_GFactor), dot(yuv, oITU_R_BT_601_BFactor)));
}

inline float3 oRGBToYUV(oIN(oRGBf, _RGB))
{
	// Using the float version of ITU-R BT.601 that jpeg uses. This is similar to 
	// the integer version, except this uses the full 0 - 255 range.
	static const float3 oITU_R_BT_601_OffsetYUV = float3(0.0f, 128.0f, 128.0f) / 255.0f;
	static const float3 oITU_R_BT_601_YFactor = float3(0.299f, 0.587f, 0.114f);
	static const float3 oITU_R_BT_601_UFactor = float3(-0.1687f, -0.3313f, 0.5f);
	static const float3 oITU_R_BT_601_VFactor = float3(0.5f, -0.4187f, -0.0813f);
	return saturate(float3(dot(_RGB, oITU_R_BT_601_YFactor), dot(_RGB, oITU_R_BT_601_UFactor), dot(_RGB, oITU_R_BT_601_VFactor)) + oITU_R_BT_601_OffsetYUV);
}

// Converts a color value to a float4 WITHOUT NORMALIZING. This remains [0,255].
// This assumes ABGR ordering to map to float4's rgba mapping.
inline oRGBAf oABGRToFloat4(uint _ABGR)
{
	return oRGBAf(
		float(_ABGR & 0xff),
		float((_ABGR >> 8u) & 0xff),
		float((_ABGR >> 16u) & 0xff),
		float((_ABGR >> 24u) & 0xff));
}

// Converts a float4 color value to uint ABGR WITHOUT NORMALIZING. This does not
// multiply into [0,255] because it is assumed to already have values on [0,255].
// This will map float4's rgba to ABGR.
inline uint oFloat4ToABGR(oIN(float4, _RGBA))
{
	return ((uint(_RGBA.w) & 0xff) << 24u)
		| ((uint(_RGBA.z) & 0xff) << 16u)
		| ((uint(_RGBA.y) & 0xff) << 8u)
		| (uint(_RGBA.x) & 0xff);
}

// Given an integer ID [0,255], return a color that ensures IDs near each other 
// (i.e. 13,14,15) have significantly different colors.
inline oRGBf oIDtoColor8Bit(uint ID8Bit)
{
	uint R = oRandUnmasked(ID8Bit);
	uint G = oRandUnmasked(R);
	uint B = oRandUnmasked(G);
	return oRGBf(float3(uint3(R,G,B) & 0xff) / 255.0f);
}

// Given an integer ID [0,65535], return a color that ensures IDs near each other 
// (i.e. 13,14,15) have significantly different colors.
inline oRGBf oIDtoColor16Bit(uint ID16Bit)
{
	uint R = oRandUnmasked(ID16Bit);
	uint G = oRandUnmasked(R);
	uint B = oRandUnmasked(G);
	return oRGBf(float3(uint3(R,G,B) & 0xffff) / 65535.0f);
}


#include <oHLSL/oHLSLSwizzlesOff.h>
#endif
