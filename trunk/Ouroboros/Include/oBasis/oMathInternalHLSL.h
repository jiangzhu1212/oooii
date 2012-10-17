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
// The part of the OOOii Math Library that is exactly the same as HLSL Shader
// Model 5.0 for HLSL SM5 types. This simplifies oMath.h to just those functions
// that differ from HLSL SM5 documentation. This header is not intended to be
// included outside oMath.h
//
// NOTE: Because min/max can conflict with C-lib and platform lib (windows.h)
// headers, min/max are defined as oMin/oMax. Because the macros for min/max
// are in math.h, it may not be noticible until using the functions on vectors.
#pragma once
#ifndef oMathInternalHLSL_h
#define oMathInternalHLSL_h

#include <oBasis/oMathTypes.h>
#include <oBasis/oByteSwizzle.h>
#include <math.h>
#include <float.h>

#ifndef oMATH_USE_FAST_ASINT
	#define oMATH_USE_FAST_ASINT 0
#endif

#ifndef oMATH_USE_FAST_RCP
	#define oMATH_USE_FAST_RCP 0
#endif

#ifndef oMATH_USE_FAST_RSQRT
	#define oMATH_USE_FAST_RSQRT 0
#endif

#ifndef oMATH_USE_FAST_LOG2
	#define oMATH_USE_FAST_LOG2 0
#endif

// _____________________________________________________________________________
// Operators

// mul(a,b) means a * b, meaning if you want to scale then translate, it would be result = scale * translate
template<typename T> TMAT3<T> mul(const TMAT3<T>& _Matrix0, const TMAT3<T>& _Matrix1) { return TMAT3<T>((_Matrix1 * _Matrix1.Column0), (_Matrix1 * _Matrix0.Column1), (_Matrix1 * _Matrix0.Column2)); }
template<typename T> TMAT4<T> mul(const TMAT4<T>& _Matrix0, const TMAT4<T>& _Matrix1) { return TMAT4<T>((_Matrix1 * _Matrix0.Column0), (_Matrix1 * _Matrix0.Column1), (_Matrix1 * _Matrix0.Column2), (_Matrix1 * _Matrix0.Column3)); }
template<typename T> TVEC3<T> mul(const TMAT3<T>& _Matrix, const TVEC3<T>& _Vector) { return TVEC3<T>(_Matrix[0].x * _Vector.x + _Matrix[1].x * _Vector.y + _Matrix[2].x * _Vector.z, _Matrix[0].y * _Vector.x + _Matrix[1].y * _Vector.y + _Matrix[2].y * _Vector.z, _Matrix[0].z * _Vector.x + _Matrix[1].z * _Vector.y + _Matrix[2].z * _Vector.z); }
template<typename T> TVEC3<T> mul(const TMAT4<T>& _Matrix, const TVEC3<T>& _Vector) { return mul(_Matrix, TVEC4<T>(_Vector.x,_Vector.y,_Vector.z,1.0f)).xyz(); }
template<typename T> TVEC4<T> mul(const TMAT4<T>& _Matrix, const TVEC4<T>& _Vector) { return TVEC4<T>(((((_Matrix.Column0.x*_Vector.x) + (_Matrix.Column1.x*_Vector.y)) + (_Matrix.Column2.x*_Vector.z)) + (_Matrix.Column3.x*_Vector.w)), ((((_Matrix.Column0.y*_Vector.x) + (_Matrix.Column1.y*_Vector.y)) + (_Matrix.Column2.y*_Vector.z)) + (_Matrix.Column3.y*_Vector.w)), ((((_Matrix.Column0.z*_Vector.x) + (_Matrix.Column1.z*_Vector.y)) + (_Matrix.Column2.z*_Vector.z)) + (_Matrix.Column3.z*_Vector.w)), ((((_Matrix.Column0.w*_Vector.x) + (_Matrix.Column1.w*_Vector.y)) + (_Matrix.Column2.w*_Vector.z)) + (_Matrix.Column3.w*_Vector.w))); }
template<typename T> T mad(const T& mvalue, const T& avalue, const T& bvalue) { return mvalue*avalue + bvalue; }

// _____________________________________________________________________________
// HLSL Matrix operations

template<typename T> T determinant(const TMAT3<T>& _Matrix) { return dot(_Matrix.Column2, cross(_Matrix.Column0, _Matrix.Column1)); }
template<typename T> T determinant(const TMAT4<T>& _Matrix);

template<typename T> TMAT3<T> transpose(const TMAT3<T>& _Matrix) { return TMAT3<T>(TVEC3<T>(_Matrix.Column0.x, _Matrix.Column1.x, _Matrix.Column2.x), TVEC3<T>(_Matrix.Column0.y, _Matrix.Column1.y, _Matrix.Column2.y), TVEC3<T>(_Matrix.Column0.z, _Matrix.Column1.z, _Matrix.Column2.z)); }
template<typename T> TMAT4<T> transpose(const TMAT4<T>& _Matrix) { return TMAT4<T>(TVEC4<T>(_Matrix.Column0.x, _Matrix.Column1.x, _Matrix.Column2.x, _Matrix.Column3.x), TVEC4<T>(_Matrix.Column0.y, _Matrix.Column1.y, _Matrix.Column2.y, _Matrix.Column3.y), TVEC4<T>(_Matrix.Column0.z, _Matrix.Column1.z, _Matrix.Column2.z, _Matrix.Column3.z), TVEC4<T>(_Matrix.Column0.w, _Matrix.Column1.w, _Matrix.Column2.w, _Matrix.Column3.w)); }

// _____________________________________________________________________________
// Permutate bit operations to vector types

oMATH_ELUFNS(countbits);
oMATH_ELUFNS(reversebits);
oMATH_ELUFNS(firstbithigh);
oMATH_ELUFNS(firstbitlow);

// _____________________________________________________________________________
// Type conversions

inline int asint(float f)
{
	#if oMATH_USE_FAST_ASINT
		/** <citation
			usage="Implementation" 
			reason="Because we always need a faster math op." 
			author="Jonathon Blow"
			description="http://www.beyond3d.com/content/articles/8/"
			license="MIT-like"
			licenseurl="http://www.gdmag.com/resources/code.htm"
			modification="Renamed, code standard compliance, compiler warning fixes"
		/>
		<license>
			This program is Copyright (c) 2003 Jonathan Blow.  All rights reserved.
			Permission to use, modify, and distribute a modified version of 
			this software for any purpose and without fee is hereby granted, 
			provided that this notice appear in all copies.
			THE MATERIAL EMBODIED ON THIS SOFTWARE IS PROVIDED TO YOU "AS-IS"
			AND WITHOUT WARRANTY OF ANY KIND, EXPRESS, IMPLIED OR OTHERWISE,
			INCLUDING WITHOUT LIMITATION, ANY WARRANTY OF MERCHANTABILITY OR
			FITNESS FOR A PARTICULAR PURPOSE.
		</license>
		*/
		// $(CitedCodeBegin)

		// Blow, Jonathon. "Unified Rendering Level-of-Detail, Part 2." 
		// Game Developer Magazine, April 2003.
		// http://www.gdmag.com/resources/code.htm
		/*
			This file contains the necessary code to do a fast float-to-int
			conversion, where the input is an IEEE-754 32-bit floating-point
			number.  Just call FastInt(f).  The idea here is that C and C++
			have to do some extremely slow things in order to ensure proper
			rounding semantics; if you don't care about your integer result
			precisely conforming to the language spec, then you can use this.
			FastInt(f) is many many many times faster than (int)(f) in almost
			all cases.
		*/
		const unsigned int DOPED_MAGIC_NUMBER_32 = 0x4b3fffff;
		f += (float &)DOPED_MAGIC_NUMBER_32;
		int result = (*(unsigned int*)&f) - DOPED_MAGIC_NUMBER_32;
		return result;
		// $(CitedCodeEnd)
	#else
		return static_cast<int>(f);
	#endif
}

inline double asdouble(unsigned int lowbits, unsigned int highbits) { oByteSwizzle64 s; s.AsUnsignedInt[0] = lowbits; s.AsUnsignedInt[1] = highbits; return s.AsDouble; }
inline double2 asdouble(const uint2& lowbits, const uint2& highbits) { oByteSwizzle64 s[2]; s[0].AsUnsignedInt[0] = lowbits.x; s[0].AsUnsignedInt[1] = highbits.x; s[1].AsUnsignedInt[0] = lowbits.y; s[1].AsUnsignedInt[1] = highbits.y; return double2(s[0].AsDouble, s[1].AsDouble); }
template<typename T> double asdouble(const T& value) { return *(double*)&value; }
template<typename T> double2 asdouble(const TVEC2<T>& value) { return double2(asdouble(value.x), asdouble(value.y)); }
template<typename T> double3 asdouble(const TVEC3<T>& value) { return double3(asdouble(value.x), asdouble(value.y), asdouble(value.z)); }
template<typename T> double4 asdouble(const TVEC4<T>& value) { return double4(asdouble(value.x), asdouble(value.y), asdouble(value.z), asdouble(value.w)); }
template<typename T> double4x4 asdouble(const TMAT4<T>& value) { return double4x4(asdouble(value.Column0), asdouble(value.Column1), asdouble(value.Column2), asdouble(value.Column3)); }
inline float2 asfloat(const double& value) { oByteSwizzle64 s; s.AsDouble = value; return float2(s.AsFloat[0], s.AsFloat[1]); }
inline float4 asfloat(const double2& value) { oByteSwizzle64 s[2]; s[0].AsDouble = value.x; s[1].AsDouble = value.y; return float4(s[0].AsFloat[0], s[0].AsFloat[1], s[1].AsFloat[0], s[1].AsFloat[1]); }
inline float2 asfloat(const llong& value) { oByteSwizzle64 s; s.AsLongLong = value; return float2(s.AsFloat[0], s.AsFloat[1]); }
inline float4 asfloat(const llong2& value) { oByteSwizzle64 s[2]; s[0].AsLongLong = value.x; s[1].AsLongLong = value.y; return float4(s[0].AsFloat[0], s[0].AsFloat[1], s[1].AsFloat[0], s[1].AsFloat[1]); }
template<typename T> float asfloat(const T& value) { return *(float*)&value; }
template<typename T> float2 asfloat(const TVEC2<T>& value) { return float2(asfloat(value.x), asfloat(value.y)); }
template<typename T> float3 asfloat(const TVEC3<T>& value) { return float3(asfloat(value.x), asfloat(value.y), asfloat(value.z)); }
template<typename T> float4 asfloat(const TVEC4<T>& value) { return float4(asfloat(value.x), asfloat(value.y), asfloat(value.z), asfloat(value.w)); }
template<typename T> float4x4 asfloat(const TMAT4<T>& value) { return float4x4(asfloat(value.Column0), asfloat(value.Column1), asfloat(value.Column2), asfloat(value.Column3)); }
inline int2 asint(double value) { oByteSwizzle64 s; s.AsDouble = value; return int2(s.AsInt[0], s.AsInt[1]); }
inline int4 asint(double2 value) { oByteSwizzle64 s[2]; s[0].AsDouble = value.x; s[1].AsDouble = value.y; return int4(s[0].AsInt[0], s[0].AsInt[1], s[1].AsInt[0], s[1].AsInt[1]); }
inline int2 asint(long long value) { oByteSwizzle64 s; s.AsLongLong = value; return int2(s.AsInt[0], s.AsInt[1]); }
inline int4 asint(const llong2& value) { oByteSwizzle64 s[2]; s[0].AsLongLong = value.x; s[1].AsLongLong = value.y; return int4(s[0].AsInt[0], s[0].AsInt[1], s[1].AsInt[0], s[1].AsInt[1]); }
template<typename T> int asint(const T& value) { return *(int*)&value; }
template<typename T> int2 asint(const TVEC2<T>& value) { return int2(asint(value.x), asint(value.y)); }
template<typename T> int3 asint(const TVEC3<T>& value) { return int3(asint(value.x), asint(value.y), asint(value.z)); }
template<typename T> int4 asint(const TVEC4<T>& value) { return int4(asint(value.x), asint(value.y), asint(value.z), asint(value.w)); }

inline void asuint(double value, unsigned int& a, unsigned int& b) { oByteSwizzle64 s; s.AsDouble = value; a = s.AsUnsignedInt[0]; b = s.AsUnsignedInt[1]; }
template<typename T> uint asuint(const T& value) { return *(uint*)&value; }
template<typename T> uint2 asuint(const TVEC2<T>& value) { return uint2(asuint(value.x), asuint(value.y)); }
template<typename T> uint3 asuint(const TVEC3<T>& value) { return uint3(asuint(value.x), asuint(value.y), asuint(value.z)); }
template<typename T> uint4 asuint(const TVEC4<T>& value) { return uint4(asuint(value.x), asuint(value.y), asuint(value.z), asuint(value.w)); }

#ifdef _HALF_H_
	inline float f16tof32(unsigned int value) { half h; h.setBits(static_cast<unsigned short>(value)); return static_cast<float>(h); }
	template<typename T> float2 f16tof32(const float2& value) { return float2(f16tof32(value.x), f16tof32(value.y)); }
	template<typename T> float3 f16tof32(const float3& value) { return float2(f16tof32(value.x), f16tof32(value.y), f16tof32(value.z)); }
	template<typename T> float4 f16tof32(const float4& value) { return float2(f16tof32(value.x), f16tof32(value.y), f16tof32(value.z), f16tof32(value.w)); }
	inline unsigned int f32tof16(float value) { return half(value).bits(); }
	template<typename T> uint2 f32tof16(const float2& value) { return uint2(f32tof16(value.x), f32tof16(value.y)); }
	template<typename T> uint3 f32tof16(const float3& value) { return uint3(f32tof16(value.x), f32tof16(value.y), f32tof16(value.z)); }
	template<typename T> uint4 f32tof16(const float4& value) { return uint4(f32tof16(value.x), f32tof16(value.y), f32tof16(value.z), f32tof16(value.w)); }
#endif

// _____________________________________________________________________________
// Introspection for halfs, floats and doubles

template<typename T> T sign(const T& x) { return x < 0 ? T(-1) : (x == 0 ? T(0) : T(1)); }

inline bool isfinite(const float& a)
{
	#ifdef _M_X64
		return !!_finitef(a);
	#else
		return isfinite(a);
	#endif
}

inline bool isnan(const float& a)
{
	#ifdef _M_X64
		return !!_isnanf(a);
	#else
		return !!_isnan((double)a);
	#endif
}

inline bool isfinite(const double& a) { return !!_finite(a); }
template<typename T> bool isinf(const T& a) { return !isfinite(a); }
inline bool isinf(const double& a) { return !isfinite(a); }
inline bool isnan(const double& a) { return !!_isnan(a); }
template<typename T> bool isfinite(const TVEC2<T>& a) { return isfinite(a.x) && isfinite(a.y); }
template<typename T> bool isfinite(const TVEC3<T>& a) { return isfinite(a.x) && isfinite(a.y) && isfinite(a.z); }
template<typename T> bool isfinite(const TVEC4<T>& a) { return isfinite(a.x) && isfinite(a.y) && isfinite(a.z) && isfinite(a.w); }
template<typename T> bool isinf(const TVEC2<T>& a) { return isinf(a.x) || isinf(a.y); }
template<typename T> bool isinf(const TVEC3<T>& a) { return isinf(a.x) || isinf(a.y) || isinf(a.z); }
template<typename T> bool isinf(const TVEC4<T>& a) { return isinf(a.x) || isinf(a.y) || isinf(a.z) || isinf(a.w); }
template<typename T> bool isnan(const TVEC2<T>& a) { return isnan(a.x) || isnan(a.y); }
template<typename T> bool isnan(const TVEC3<T>& a) { return isnan(a.x) || isnan(a.y) || isnan(a.z); }
template<typename T> bool isnan(const TVEC4<T>& a) { return isnan(a.x) || isnan(a.y) || isnan(a.z) || isnan(a.w); }
template<typename T> bool isdenorm(const TVEC2<T>& a) { return isdenorm(a.x) || isdenorm(a.y); }
template<typename T> bool isdenorm(const TVEC3<T>& a) { return isdenorm(a.x) || isdenorm(a.y) || isdenorm(a.z); }
template<typename T> bool isdenorm(const TVEC4<T>& a) { return isdenorm(a.x) || isdenorm(a.y) || isdenorm(a.z) || isdenorm(a.w); }

// _____________________________________________________________________________
// Component selection for halfs, floats and doubles

oMATH_ELUFNS(abs);
oMATH_ELUFNS(ceil);
oMATH_ELUFNS(floor);
oMATH_ELUFNS(frac);
oMATH_ELUFNS(round);
#if (_MSC_VER < 1600)
inline long long abs(const long long& x) { return _abs64(x); }
#endif
template<typename T> T round(const T& a) { return floor(a + T(0.5)); }
template<typename T> T frac(const T& a) { return a - floor(a); }
template<typename T> T truc(const T& x) { return floor(x); }
template<typename T> bool all(const TVEC2<T>& a) { return !oEqual(a.x, T(0)) && !oEqual(a.y, T(0)); }
template<typename T> bool all(const TVEC3<T>& a) { return !oEqual(a.x, T(0)) && !oEqual(a.y, T(0)) && !oEqual(a.z, T(0)); }
template<typename T> bool all(const TVEC4<T>& a) { return !oEqual(a.x, T(0)) && !oEqual(a.y, T(0)) && !oEqual(a.z, T(0)) && !oEqual(a.w, T(0)); }
template<typename T> bool any(const T& a) { return a != T(0); }

// _____________________________________________________________________________
// Trigonometry

oMATH_ELUFNS(cos); oMATH_ELUFNS(acos); oMATH_ELUFNS(cosh);
oMATH_ELUFNS(sin); oMATH_ELUFNS(asin); oMATH_ELUFNS(sinh);
oMATH_ELUFNS(tan); oMATH_ELUFNS(atan); oMATH_ELUFNS(tanh);
oMATH_ELBFNS(atan2, atan2);
template<typename T> void sincos(const T& angleInRadians, T& outSin, T& outCos) { outSin = sin(angleInRadians); outCos = cos(angleInRadians); }

// _____________________________________________________________________________
// Geometry

oMATH_ELUFNS(radians);
oMATH_ELUFNS(degrees);
template<typename T> TVEC3<T> cross(const TVEC3<T>& a, const TVEC3<T>& b) { return TVEC3<T>(a.y*b.z - a.z*b.y, a.z*b.x - a.x*b.z, a.x*b.y - a.y*b.x); }
template<typename T> T dot(const TVEC2<T>& a, const TVEC2<T>& b) { return a.x*b.x + a.y*b.y; }
template<typename T> T dot(const TVEC3<T>& a, const TVEC3<T>& b) { return a.x*b.x + a.y*b.y + a.z*b.z; }
template<typename T> T dot(const TVEC4<T>& a, const TVEC4<T>& b) { return a.x*b.x + a.y*b.y + a.z*b.z + a.w*b.w; }
template<typename T> T length(const TVEC2<T>& a) { return sqrt(dot(a, a)); }
template<typename T> T length(const TVEC3<T>& a) { return sqrt(dot(a, a)); }
template<typename T> T length(const TVEC4<T>& a) { return sqrt(dot(a, a)); }
template<typename T> const T lerp(const T& a, const T& b, const T& s) { return a + s * (b-a); }
template<typename T> const TVEC2<T> lerp(const TVEC2<T>& a, const TVEC2<T>& b, const T& s) { return a + s * (b-a); }
template<typename T> const TVEC3<T> lerp(const TVEC3<T>& a, const TVEC3<T>& b, const T& s) { return a + s * (b-a); }
template<typename T> const TVEC4<T> lerp(const TVEC4<T>& a, const TVEC4<T>& b, const T& s) { return a + s * (b-a); }
template<typename T> TVEC4<T> lit(const T& n_dot_l, const T& n_dot_h, const T& m) { TVEC4<T>(T(1), (n_dot_l < 0) ? 0 : n_dot_l, (n_dot_l < 0) || (n_dot_h < 0) ? 0 : (n_dot_h * m), T(1)); }
template<typename T> T faceforward(const T& n, const T& i, const T& ng) { return -n * sign(dot(i, ng)); }
template<typename T> T normalize(const T& x) { return x / length(x); }
template<typename T> T radians(T degrees) { return degrees * T(3.14159265358979323846) / T(180.0); }
template<typename T> T degrees(T radians) { return radians * T(180.0) / T(3.14159265358979323846); }
template<typename T> T distance(const TVEC2<T>& a, const TVEC2<T>& b) { return length(a-b); }
template<typename T> T distance(const TVEC3<T>& a, const TVEC3<T>& b) { return length(a-b); }
template<typename T> T reflect(const T& i, const T& n) { return i - 2 * n * dot(i,n); }
template<typename T> T refract(const TVEC3<T>& i, const TVEC3<T>& n, const T& r) { T c1 = dot(i,n); T c2 = T(1) - r*r * (T(1) - c1*c1); return (c2 < T(0)) ? TVEC3<T>(0) : r*i + (sqrt(c2) - r*c1) * n; } // http://www.physicsforums.com/archive/index.php/t-187091.html

// _____________________________________________________________________________
// Algebra

oMATH_ELUFNS(log); oMATH_ELUFNS(log2); oMATH_ELUFNS(log10);
oMATH_ELUFNS(exp); oMATH_ELUFNS(exp2);
oMATH_ELUFNS(pow); oMATH_ELUFNS(sqrt);
oMATH_ELBFNS(ldexp, ldexp); // return x * 2^(exp) float
oMATH_ELBFNS(frexp, frexp);
oMATH_ELBFNS(fmod, fmod);
oMATH_ELUFNS(rcp);

inline double log2(double a) { static const double sCONV = 1.0/log(2.0); return log(a) * sCONV; }
inline float exp2(float a) { return powf(2.0f, a); }
inline double exp2(double a) { return pow(2.0, a); }
template<typename T> T frexp(const T& x, T& exp) { int e; T ret = ::frexp(x, &e); exp = static_cast<T>(e); return ret; }
inline float modf(const float& x, float& ip) { return ::modf(x, &ip); }
inline double modf(const double& x, double& ip) { return ::modf(x, &ip); }
template<typename T> TVEC2<T> modf(const TVEC2<T>& x, TVEC2<T>& ip) { return TVEC2<T>(modf(x.x, ip.x), modf(x.y, ip.y)); }
template<typename T> TVEC3<T> modf(const TVEC3<T>& x, TVEC3<T>& ip) { return TVEC3<T>(modf(x.x, ip.x), modf(x.y, ip.y), modf(x.z, ip.z)); }
template<typename T> TVEC4<T> modf(const TVEC4<T>& x, TVEC4<T>& ip) { return TVEC4<T>(modf(x.x, ip.x), modf(x.y, ip.y), modf(x.z, ip.z), modf(x.w, ip.w)); }
template<typename T> T rsqrt(T x) { return T(1) / sqrt(x); }
template<typename T> T rcp(const T& value) { return T(1) / value; }

inline float log2(float val)
{
	#if oMATH_USE_FAST_LOG2
		/** <citation
			usage="Implementation" 
			reason="std libs don't have log2" 
			author="Blaxill"
			description="http://www.devmaster.net/forums/showthread.php?t=12765"
			license="*** Assumed Public Domain ***"
			licenseurl="http://www.devmaster.net/forums/showthread.php?t=12765"
			modification=""
		/>*/
		// $(CitedCodeBegin)
		int * const    exp_ptr = reinterpret_cast <int *>(&val);
		int            x = *exp_ptr;
		const int      log_2 = ((x >> 23) & 255) - 128;
		x &= ~(255 << 23);
		x += 127 << 23;
		*exp_ptr = x;

		val = ((-1.0f/3) * val + 2) * val - 2.0f/3; //(1)

		return (val + log_2);
		// $(CitedCodeEnd)
	#else
		static const double sCONV = 1.0/log(2.0);
		return static_cast<float>(log(val) * sCONV);
	#endif
}

template<> inline float rcp(const float& x)
{
	#if oMATH_USE_FAST_RCP
		/** <citation
			usage="Implementation" 
			reason="Because we always need a faster math op." 
			author="Simon Hughes"
			description="http://www.codeproject.com/cpp/floatutils.asp?df=100&forumid=208&exp=0&select=950856#xx950856xx"
			license="CPOL 1.02"
			licenseurl="http://www.codeproject.com/info/cpol10.aspx"
			modification="FP_INV made more C++-like, compiler warning fixes"
		/>*/
		// $(CitedCodeBegin)
		// This is about 2.12 times faster than using 1.0f / n 
			int _i = 2 * 0x3F800000 - *(int*)&x;
			float r = *(float *)&_i;
			return r * (2.0f - x * r);
		// $(CitedCodeEnd)
	#else
		return 1.0f / x;
	#endif
}

template<> inline float rsqrt(float x)
{
	#if oMATH_USE_FAST_RSQRT
		/** <citation
			usage="Implementation" 
			reason="Because we always need a faster math op." 
			author="?"
			description="http://www.beyond3d.com/content/articles/8/"
			license="*** Assumed Public Domain ***"
			licenseurl="http://www.beyond3d.com/content/articles/8/"
			modification="code standard compliance, fix compiler warnings"
		/>*/
		// $(CitedCodeBegin)
		float xhalf = 0.5f*x;
		int i = *(int*)&x;
		i = 0x5f3759df - (i>>1);
		x = *(float*)&i;
		x = x*(1.5f - xhalf*x*x);
		return x;
		// $(CitedCodeEnd)
	#else
		return 1.0f / (float)sqrt(x);
	#endif
}

// _____________________________________________________________________________
// Range/clamp functions

oMATH_ELBFNS(oMax, __max); // not named max to avoid stdc macro collision
oMATH_ELBFNS(oMin, __min); // not named min to avoid stdc macro collision
oMATH_ELBFNS(step, step);
template<typename T> T oMax(const T& x, const T& y) { return (x > y) ? x : y; }
template<typename T> T oMin(const T& x, const T& y) { return (x < y) ? x : y; }
template<typename T> T step(const T& y, const T& x) { return (x >= y) ? T(1) : T(0); } 
template<typename T> TVEC2<T> smoothstep(const TVEC2<T>& minimum, const TVEC2<T>& maximum, const TVEC2<T>& x) { return TVEC2<T>(smoothstep(minimum.x, maximum.x, x.x), smoothstep(minimum.y, maximum.y, x.y)); }
template<typename T> TVEC3<T> smoothstep(const TVEC3<T>& minimum, const TVEC3<T>& maximum, const TVEC3<T>& x) { return TVEC3<T>(smoothstep(minimum.x, maximum.x, x.x), smoothstep(minimum.y, maximum.y, x.y), smoothstep(minimum.z, maximum.z, x.z)); }
template<typename T> TVEC4<T> smoothstep(const TVEC4<T>& minimum, const TVEC4<T>& maximum, const TVEC4<T>& x) { return TVEC4<T>(smoothstep(minimum.x, maximum.x, x.x), smoothstep(minimum.y, maximum.y, x.y), smoothstep(minimum.z, maximum.z, x.z), smoothstep(minimum.w, maximum.w, x.w)); }
template<typename T> T smoothstep(const T& minimum, const T& maximum, const T& x) { return x < minimum ? T(0) : (x > maximum ? T(1) : T(-2) * pow((x – minimum) / (maximum – minimum), T(3)) + T(3) * pow((x – minimum) / (maximum – minimum), T(2))); } // http://http.developer.nvidia.com/CgTutorial/cg_tutorial_chapter05.html
template<typename T> T clamp(const T& x, const T& minimum, const T& maximum) { return oMax(oMin(x, maximum), minimum); }
template<typename T, typename vectorT> vectorT clamp(const vectorT& x, const T& minimum, const T& maximum) { return clamp<vectorT>(x, vectorT(minimum), vectorT(maximum)); }
template<typename T> T saturate(const T& x) { return clamp<T>(x, T(0.0), T(1.0)); }

// _____________________________________________________________________________
// Randomization

template<typename T, typename TVec> T noise(const TVec& x);

// _____________________________________________________________________________
// Unimplemented HLSL functions declared here for completeness

// Partial derivatives. It would require a lot of non-math infrastructure to 
// support these functions, so they are unimplemented.
template<typename T> T ddx(const T& x);
template<typename T> T ddx_coarse(const T& x);
template<typename T> T ddx_fine(const T& x);
template<typename T> T ddy(const T& x);
template<typename T> T ddy_coarse(const T& x);
template<typename T> T ddy_fine(const T& x);
template<typename T> T fwidth(const T& x) { return abs(ddx(x)) + abs(ddy(x)); }
//D3DCOLORtoUBYTE4
//dst

#endif
