// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.
// This code contains code that cross-compiles in C++ and HLSL. This contains
// utilities for working with color, or colorizing other types of data.
// This allows the same type to be used in shared headers and more encasulated
// and automated conversion between color and floating-point color values.
#ifndef oHLSL
	#pragma once
#endif
#ifndef oBase_rgb_h
#define oBase_rgb_h

#include <oHLSL/oHLSLMacros.h>
#include <oHLSL/oHLSLMath.h>

#ifdef oHLSL
	#define rgbf float3
	#define rgbaf float4
#else

#include <oBase/color.h>
#include <oBase/operators.h>
#include <oHLSL/oHLSLMath.h>
#include <oHLSL/oHLSLTypes.h>
#include <oHLSL/oHLSLSwizzlesOn.h>

//namespace ouro {

class rgbf
{
	// Handle automatic expansion of color to float3s. Using this type
	// allows for a single header to be defined for usage in both C++ and HLSL. 
	// NOTE: Alpha is quietly dropped/ignored when using this.

public:
	float r, g, b;

	rgbf() : r(0.0f), g(0.0f), b(0.0f) {}
	rgbf(float _R, float _G, float _B) : r(_R), g(_G), b(_B) {}
	rgbf(const float3& _Color) : r(_Color.x), g(_Color.y), b(_Color.z) {}
	rgbf(const ouro::color& _Color) { _Color.decompose(&r, &g, &b); }
	rgbf(const rgbf& _RGB) : r(_RGB.r), g(_RGB.g), b(_RGB.b) {}

	inline operator float3&() { return *(float3*)this; }
	inline operator const float3&() const { return *(float3*)this; }
	inline operator float3() const { return *(float3*)this; }
	inline operator ouro::color() const { return ouro::color(r, g, b, 1.0f); }
	inline const rgbf& operator=(int _Color) { r = static_cast<float>(_Color); g = static_cast<float>(_Color); b = static_cast<float>(_Color); return *this; }
	inline const rgbf& operator=(const rgbf& _Color) { r = _Color.r; g = _Color.g; b = _Color.b; return *this; }
	inline const rgbf& operator=(const float3& _Color) { r = _Color.x; g = _Color.y; b = _Color.z; return *this; }
	inline const rgbf& operator=(const ouro::color& _Color) { _Color.decompose(&r, &g, &b); return *this; }

	rgbf& operator+=(const rgbf& _That) { *this = saturate((const float3&)*this + (const float3&)_That); return *this; }
	rgbf& operator-=(const rgbf& _That) { *this = saturate((const float3&)*this - (const float3&)_That); return *this; }
	rgbf& operator*=(const rgbf& _That) { *this = saturate((const float3&)*this * (const float3&)_That); return *this; }
	rgbf& operator/=(const rgbf& _That) { *this = saturate((const float3&)*this / (const float3&)_That); return *this; }

	rgbf& operator+=(const float& _That) { operator+=(rgbf(_That, _That, _That)); return *this; }
	rgbf& operator-=(const float& _That) { operator-=(rgbf(_That, _That, _That)); return *this; }
	rgbf& operator*=(const float& _That) { operator*=(rgbf(_That, _That, _That)); return *this; }
	rgbf& operator/=(const float& _That) { operator/=(rgbf(_That, _That, _That)); return *this; }

	oOPERATORS_DERIVED_COMMUTATIVE2(rgbf, float, +) oOPERATORS_DERIVED(rgbf, -) oOPERATORS_DERIVED(rgbf, *) oOPERATORS_DERIVED(rgbf, /)

	inline rgbf operator-(float x) const { return (const float3&)*this - x; }
	inline rgbf operator*(float x) const { return (const float3&)*this * x; }
	inline rgbf operator/(float x) const { return (const float3&)*this / x; }

	friend inline rgbf operator-(float x, const rgbf& _RGBf) { return rgbf(x,x,x) - _RGBf; }
	friend inline rgbf operator*(float x, const rgbf& _RGBf) { return rgbf(x,x,x) / _RGBf; }

	inline const float3& as_float3() const { return *(const float3*)this; }
};

class rgbaf
{
	// Handle automatic expansion of color to float3s. Using this type 
	// allows for a single header to be defined for usage in both C++ and HLSL. 

public:
	float r, g, b, a;

	rgbaf() : r(0.0f), g(0.0f), b(0.0f), a(1.0f) {}
	rgbaf(float _R, float _G, float _B, float _A) : r(_R), g(_G), b(_B), a(_A) {}
	rgbaf(const float4& _Color) : r(_Color.x), g(_Color.y), b(_Color.z), a(_Color.w) {}
	rgbaf(const ouro::color& _Color) { _Color.decompose(&r, &g, &b, &a); }
	rgbaf(const rgbaf& _RGBA) : r(_RGBA.r), g(_RGBA.g), b(_RGBA.b), a(_RGBA.a) {}

	inline const float3& rgb() const { return *(const float3*)this; }
	inline operator float4&() { return *(float4*)this; }
	inline operator const float4&() const { return *(const float4*)this; }
	inline operator ouro::color() const { return ouro::color(r, g, b, a); }
	inline const rgbaf& operator=(int _Color) { r = static_cast<float>(_Color); g = static_cast<float>(_Color); b = static_cast<float>(_Color); a = static_cast<float>(_Color); return *this; }
	inline const rgbaf& operator=(const rgbaf& _Color) { r = _Color.r; g = _Color.g; b = _Color.b; a = _Color.a; return *this; }
	inline const rgbaf& operator=(const float4& _Color) { r = _Color.x; g = _Color.y; b = _Color.z; a = _Color.w; return *this; }
	inline const rgbaf& operator=(const ouro::color& _Color) { _Color.decompose(&r, &g, &b, &a); return *this; }

	rgbaf& operator+=(const rgbaf& _That) { *this = saturate((const float4&)*this + (const float4&)_That); return *this; }
	rgbaf& operator-=(const rgbaf& _That) { *this = saturate((const float4&)*this - (const float4&)_That); return *this; }
	rgbaf& operator*=(const rgbaf& _That) { *this = saturate((const float4&)*this * (const float4&)_That); return *this; }
	rgbaf& operator/=(const rgbaf& _That) { *this = saturate((const float4&)*this / (const float4&)_That); return *this; }

	rgbaf& operator+=(const float& _That) { operator+=(rgbaf(_That, _That, _That, _That)); return *this; }
	rgbaf& operator-=(const float& _That) { operator-=(rgbaf(_That, _That, _That, _That)); return *this; }
	rgbaf& operator*=(const float& _That) { operator*=(rgbaf(_That, _That, _That, _That)); return *this; }
	rgbaf& operator/=(const float& _That) { operator/=(rgbaf(_That, _That, _That, _That)); return *this; }

	oOPERATORS_DERIVED_COMMUTATIVE2(rgbaf, float, +) oOPERATORS_DERIVED(rgbaf, -) oOPERATORS_DERIVED(rgbaf, *) oOPERATORS_DERIVED(rgbaf, /)

	inline rgbaf operator-(float x) const { return (const float4&)*this - x; }
	inline rgbaf operator*(float x) const { return (const float4&)*this * x; }
	inline rgbaf operator/(float x) const { return (const float4&)*this / x; }

	friend inline rgbaf operator-(float x, const rgbaf& _RGBAf) { return rgbaf(x,x,x,x) - _RGBAf; }
	friend inline rgbaf operator*(float x, const rgbaf& _RGBAf) { return rgbaf(x,x,x,x) / _RGBAf; }

	inline const float4& as_float4() const { return *(const float4*)this; }
};

inline float min(oIN(rgbf, _RGB)) { return min(min(_RGB.r, _RGB.g), _RGB.b); }
inline float max(oIN(rgbf, _RGB)) { return max(max(_RGB.r, _RGB.g), _RGB.b); }
inline float min(oIN(rgbaf, _RGBA)) { return min(min(min(_RGBA.r, _RGBA.g), _RGBA.b), _RGBA.a); }
inline float max(oIN(rgbaf, _RGBA)) { return max(max(max(_RGBA.r, _RGBA.g), _RGBA.b), _RGBA.a); }

inline float dot(oIN(rgbf, _RGB), oIN(float3, _X)) { return dot(_RGB.as_float3(), _X); }
inline float dot(oIN(rgbaf, _RGBA), oIN(float4, _X)) { return dot(_RGBA.as_float4(), _X); }

#endif

inline float srgbtolin(float x)
{
	return (x <= 0.04045f) ? (x / 12.42f) : pow((x + 0.055f) / 1.055f, 2.4f);
}

inline float lintosrgb(float x)
{
	return (x <= 0.0031308f) ? (x * 12.92f) : (1.055f * pow(x, 1.0f / 2.4f) - 0.055f);
}

inline float lintosrgbfast(float x)
{
	return x < 0.0031308f ? 12.92f * x : 1.13005f * sqrt(abs(x - 0.00228f)) - 0.13448f * x + 0.005719f;
}

inline rgbf srgbtolin(oIN(rgbf, _sRGBf))
{
	rgbf c;
	c.r = srgbtolin(_sRGBf.r);
	c.g = srgbtolin(_sRGBf.g);
	c.b = srgbtolin(_sRGBf.b);
	return c;
}

inline rgbf lintosrgb(oIN(rgbf, _lRGBf))
{
	rgbf c;
	c.r = lintosrgb(_lRGBf.r);
	c.g = lintosrgb(_lRGBf.g);
	c.b = lintosrgb(_lRGBf.b);
	return c;
}

// Converts a 3D normalized vector into an RGB color (typically for encoding a normal)
inline rgbf colorize_vector(oIN(float3, _NormalizedVector))
{
	return _NormalizedVector * 0.5f + 0.5f;
}

// Converts a normalized vector stored as RGB color back to a vector
inline float3 decolorize_vector(oIN(rgbf, _VectorAsColor))
{
	return _VectorAsColor * float3(2.0f, 2.0f, -2.0f) - 1.0f;
}

// Convert from HSV color space to RGB
inline rgbf hsvtorgb(oIN(float3, _HSV))
{
	// http://chilliant.blogspot.com/2010/11/rgbhsv-in-hlsl.html
	float R = abs(_HSV.x * 6.0f - 3.0f) - 1.0f;
	float G = 2.0f - abs(_HSV.x * 6.0f - 2.0f);
	float B = 2.0f - abs(_HSV.x * 6.0f - 4.0f);
	return ((saturate(float3(R,G,B)) - 1.0f) * _HSV.y + 1.0f) * _HSV.z;
}

// Convert from RGB color space to HSV
inline float3 rgbtohsv(oIN(rgbf, _RGB))
{
	// http://stackoverflow.com/questions/4728581/hsl-image-adjustements-on-gpu
	float H = 0.0f;
	float S = 0.0f;
	float V = max(_RGB.r, max(_RGB.g, _RGB.b));
	float m = min(_RGB.r, min(_RGB.g, _RGB.b));
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

inline float rgbtolum(oIN(float3, _Color))
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
inline float normal_to_luminance(oIN(float3, _Normal))
{
	return dot(_Normal, float3(0.33333f, 0.33333f, 0.33333f));
}

inline rgbf yuvtorgb(oIN(float3, _YUV))
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

inline float3 rgbtoyuv(oIN(rgbf, _RGB))
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
inline rgbaf abgrtofloat4(uint _ABGR)
{
	return rgbaf(
		float(_ABGR & 0xff),
		float((_ABGR >> 8u) & 0xff),
		float((_ABGR >> 16u) & 0xff),
		float((_ABGR >> 24u) & 0xff));
}

// Converts a float4 color value to uint ABGR WITHOUT NORMALIZING. This does not
// multiply into [0,255] because it is assumed to already have values on [0,255].
// This will map float4's rgba to ABGR.
inline uint float4toabgr(oIN(float4, _RGBA))
{
	return ((uint(_RGBA.w) & 0xff) << 24u)
		| ((uint(_RGBA.z) & 0xff) << 16u)
		| ((uint(_RGBA.y) & 0xff) << 8u)
		| (uint(_RGBA.x) & 0xff);
}

#ifndef oHLSL
//	}
#endif

#include <oHLSL/oHLSLSwizzlesOff.h>
#endif
