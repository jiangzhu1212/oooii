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
// This header is designed to cross-compile in both C++ and HLSL. This defines
// a quaternion (float4) and some HLSL-style utility functions.
#ifndef oHLSL
	#pragma once
#endif
#ifndef oQuaternion_h
#define oQuaternion_h

#include <oHLSL/oHLSLMacros.h>
#include <oHLSL/oHLSLMath.h>
#include <oHLSL/oHLSLTypes.h>

#ifdef oHLSL
	#define quatf float4
	#define quatd double4
#else

// Wrap in a namespace so that NoStepInto can be used for VS2010+.
namespace ouro {
	template<typename T> struct quaternion
	{
		typedef T element_type;
		T x,y,z,w;
		inline quaternion() {};
		explicit inline quaternion(const TVEC4<T>& _Quaternion) : x(_Quaternion.x), y(_Quaternion.y), z(_Quaternion.z), w(_Quaternion.w) {}
		inline quaternion(const quaternion& _Quaternion) : x(_Quaternion.x), y(_Quaternion.y), z(_Quaternion.z), w(_Quaternion.w) {}
		inline quaternion(T _X, T _Y, T _Z, T _W) : x(_X), y(_Y), z(_Z), w(_W) {}
		inline quaternion(const TVEC3<T>& _XYZ, T _W) : x(_XYZ.x), y(_XYZ.y), z(_XYZ.z), w(_W) {}
		oHLSL_VBRACKET_OP(T);
		inline const TVEC3<T>& xyz() const { return *(TVEC3<T>*)&x; }
		inline const TVEC4<T>& xyzw() const { return *(TVEC4<T>*)&x; }
		inline const quaternion<T>& operator*=(T _Scalar) { *this = *this * _Scalar; return *this; }
		inline const quaternion<T>& operator+=(const quaternion<T> _Quaternion) { *this = *this + _Quaternion; return *this; }
		inline const quaternion<T>& operator-=(const quaternion<T> _Quaternion) { *this = *this - _Quaternion; return *this; }
		inline quaternion<T> operator-() const { return quaternion<T>(-x, -y, -z, -w); }
		inline quaternion<T> operator*(T _Scalar) const { return quaternion<T>(x * _Scalar, y * _Scalar, z * _Scalar, w * _Scalar); }
	};
} // namespace ouro

typedef ouro::quaternion<float> quatf; typedef ouro::quaternion<double> quatd;
oHLSL_ELOPT4_VECTOR(ouro::quaternion, +) oHLSL_ELOPT4_VECTOR(ouro::quaternion, -) oHLSL_ELOPT4_SCALAR(ouro::quaternion, *) oHLSL_ELOPT4_SCALAR(ouro::quaternion, /) oHLSL_ELOPT4_SCALAR(ouro::quaternion, +) oHLSL_ELOPT4_SCALAR(ouro::quaternion, -)

#endif
#include <oHLSL/oHLSLSwizzlesOn.h> // must be below xyz() and xyzw() definitions for quaternion

// Multiply/combine two quaternions. Careful, remember that quats are not 
// communicative, so order matters. This returns a * b.
inline quatf qmul(oIN(quatf, a), oIN(quatf, b))
{
	// http://code.google.com/p/kri/wiki/Quaternions
	return quatf(cross(a.xyz,b.xyz) + a.xyz*b.w + b.xyz*a.w, a.w*b.w - dot(a.xyz,b.xyz));
}

// Return the specified vector rotated by the specified quaternion
inline float3 qmul(oIN(quatf, q), oIN(float3, v))
{
	// http://code.google.com/p/kri/wiki/Quaternions
	#if 1
		return v + 2.0f*cross(q.xyz, cross(q.xyz,v) + q.w*v);
	#else
		return v*(q.w*q.w - dot(q.xyz,q.xyz)) + 2.0f*q.xyz*dot(q.xyz,v) + 2.0f*q.w*cross(q.xyz,v);
	#endif
}

inline quatf normalize(oIN(quatf, a)) { float4 v = normalize(a.xyzw); return quatf(v); }
inline quatf conjugate(oIN(quatf, a)) { return quatf(-a.x, -a.y, -a.z, a.w); }
inline quatf invert(oIN(quatf, a)) { float4 v = conjugate(a).xyzw * (1.0f / dot(a.xyzw, a.xyzw)); return quatf(v); }

// a / b are expected to be normalized (unit length)
inline quatf slerp(oIN(quatf, a), oIN(quatf, b), float s)
{
	/** <citation
		usage="Implementation" 
		reason="Bullet is well-proven, why reinvent? Also expose this to HLSL for usage there"
		author="Erwin Coumans (attributed)"
		description="http://continuousphysics.com/Bullet/"
		license="zlib"
		licenseurl="http://opensource.org/licenses/Zlib"
		modification="Made HLSL-compatible"
	/>*/
	// $(CitedCodeBegin)
	#define _VECTORMATH_SLERP_TOL 0.999f
	quatf start;
	float recipSinAngle, scale0, scale1, cosAngle, angle;
	cosAngle = dot( a.xyzw, b.xyzw );
	if ( cosAngle < 0.0f ) {
		cosAngle = -cosAngle;
		start = ( -a );
	} else {
		start = a;
	}
	if ( cosAngle < _VECTORMATH_SLERP_TOL ) {
		angle = acos( cosAngle );
		recipSinAngle = ( 1.0f / sin( angle ) );
		scale0 = ( sin( ( ( 1.0f - s ) * angle ) ) * recipSinAngle );
		scale1 = ( sin( ( s * angle ) ) * recipSinAngle );
	} else {
		scale0 = ( 1.0f - s );
		scale1 = s;
	}
	return ( ( start * scale0 ) + ( b * scale1 ) );
	// $(CitedCodeEnd)
}

#include <oHLSL/oHLSLSwizzlesOff.h>
#endif
