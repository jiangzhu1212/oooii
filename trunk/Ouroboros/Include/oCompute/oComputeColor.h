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

#include <oStd/color.h>
#include <oStd/operators.h>

class oRGBf
{
	// Handle automatic expansion of oStd::color to float3s. Using this type
	// allows for a single header to be defined for usage in both C++ and HLSL. 
	// NOTE: Alpha is quietly dropped/ignored when using this.

public:
	oRGBf() : Color(0.0f, 0.0f, 0.0f) {}
	oRGBf(float _R, float _G, float _B) : Color(_R, _G, _B) {}
	oRGBf(const float3& _Color) : Color(_Color) {}
	oRGBf(const oStd::color& _Color) { _Color.decompose(&Color.x, &Color.y, &Color.z); }
	oRGBf(const oRGBf& _RGB) : Color(_RGB.Color) {}

	inline operator float3&() { return Color; }
	inline operator const float3&() const { return Color; }
	inline operator oStd::color() const { return oStd::color(Color.x, Color.y, Color.z, 1.0f); }
	inline const oRGBf& operator=(int _Color) { Color = (float)_Color; return *this; }
	inline const oRGBf& operator=(const oRGBf& _Color) { Color = _Color.Color; return *this; }
	inline const oRGBf& operator=(const float3& _Color) { Color = _Color; return *this; }
	inline const oRGBf& operator=(const oStd::color& _Color) { _Color.decompose(&Color.x, &Color.y, &Color.z); return *this; }

	oRGBf& operator+=(const oRGBf& _That) { Color = saturate(Color + (const float3&)_That); return *this; }
	oRGBf& operator-=(const oRGBf& _That) { Color = saturate(Color - (const float3&)_That); return *this; }
	oRGBf& operator*=(const oRGBf& _That) { Color = saturate(Color * (const float3&)_That); return *this; }
	oRGBf& operator/=(const oRGBf& _That) { Color = saturate(Color / (const float3&)_That); return *this; }

	oRGBf& operator+=(const float& _That) { operator+=(oRGBf(_That, _That, _That)); return *this; }
	oRGBf& operator-=(const float& _That) { operator-=(oRGBf(_That, _That, _That)); return *this; }
	oRGBf& operator*=(const float& _That) { operator*=(oRGBf(_That, _That, _That)); return *this; }
	oRGBf& operator/=(const float& _That) { operator/=(oRGBf(_That, _That, _That)); return *this; }

	oOPERATORS_DERIVED_COMMUTATIVE2(oRGBf, float, +) oOPERATORS_DERIVED(oRGBf, -) oOPERATORS_DERIVED(oRGBf, *) oOPERATORS_DERIVED(oRGBf, /)

	inline oRGBf operator-(float x) const { return Color - x; }
	inline oRGBf operator*(float x) const { return Color * x; }
	inline oRGBf operator/(float x) const { return Color / x; }

	friend inline oRGBf operator-(float x, const oRGBf& _RGBf) { return oRGBf(x,x,x) - _RGBf; }
	friend inline oRGBf operator*(float x, const oRGBf& _RGBf) { return oRGBf(x,x,x) / _RGBf; }

protected:
	float3 Color;
};

class oRGBAf
{
	// Handle automatic expansion of oStd::color to float3s. Using this type 
	// allows for a single header to be defined for usage in both C++ and HLSL. 

public:
	oRGBAf() : Color(0.0f, 0.0f, 0.0f, 1.0f) {}
	oRGBAf(float _R, float _G, float _B, float _A) : Color(_R, _G, _B, _A) {}
	oRGBAf(const float4& _Color) : Color(_Color) {}
	oRGBAf(const oStd::color& _Color) { _Color.decompose(&Color.x, &Color.y, &Color.z, &Color.w); }
	oRGBAf(const oRGBAf& _RGBA) : Color(_RGBA.Color) {}

	inline const float3& rgb() const { return *(const float3*)&Color; }
	inline operator float4&() { return Color; }
	inline operator const float4&() const { return Color; }
	inline operator oStd::color() const { return oStd::color(Color.x, Color.y, Color.z, 1.0f); }
	inline const oRGBAf& operator=(int _Color) { Color = (float)_Color; return *this; }
	inline const oRGBAf& operator=(const oRGBAf& _Color) { Color = _Color.Color; return *this; }
	inline const oRGBAf& operator=(const float4& _Color) { Color = _Color; return *this; }
	inline const oRGBAf& operator=(const oStd::color& _Color) { _Color.decompose(&Color.x, &Color.y, &Color.z, &Color.w); return *this; }

	oRGBAf& operator+=(const oRGBAf& _That) { Color = saturate(Color + (const float4&)_That); return *this; }
	oRGBAf& operator-=(const oRGBAf& _That) { Color = saturate(Color - (const float4&)_That); return *this; }
	oRGBAf& operator*=(const oRGBAf& _That) { Color = saturate(Color * (const float4&)_That); return *this; }
	oRGBAf& operator/=(const oRGBAf& _That) { Color = saturate(Color / (const float4&)_That); return *this; }

	oRGBAf& operator+=(const float& _That) { operator+=(oRGBAf(_That, _That, _That, _That)); return *this; }
	oRGBAf& operator-=(const float& _That) { operator-=(oRGBAf(_That, _That, _That, _That)); return *this; }
	oRGBAf& operator*=(const float& _That) { operator*=(oRGBAf(_That, _That, _That, _That)); return *this; }
	oRGBAf& operator/=(const float& _That) { operator/=(oRGBAf(_That, _That, _That, _That)); return *this; }

	oOPERATORS_DERIVED_COMMUTATIVE2(oRGBAf, float, +) oOPERATORS_DERIVED(oRGBAf, -) oOPERATORS_DERIVED(oRGBAf, *) oOPERATORS_DERIVED(oRGBAf, /)

	inline oRGBAf operator-(float x) const { return Color - x; }
	inline oRGBAf operator*(float x) const { return Color * x; }
	inline oRGBAf operator/(float x) const { return Color / x; }

	friend inline oRGBAf operator-(float x, const oRGBAf& _RGBAf) { return oRGBAf(x,x,x,x) - _RGBAf; }
	friend inline oRGBAf operator*(float x, const oRGBAf& _RGBAf) { return oRGBAf(x,x,x,x) / _RGBAf; }

protected:
	float4 Color;
};

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

// Convert from HSV (HSL) color space to RGB
inline oRGBf oHSVtoRGB(oIN(float3, HSV))
{
	// http://chilliant.blogspot.com/2010/11/rgbhsv-in-hlsl.html
	float R = abs(HSV.x * 6.0f - 3.0f) - 1.0f;
	float G = 2 - abs(HSV.x * 6.0f - 2.0f);
	float B = 2 - abs(HSV.x * 6.0f - 4.0f);
	return ((saturate(float3(R,G,B)) - 1.0f) * HSV.y + 1.0f) * HSV.z;
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

#endif
