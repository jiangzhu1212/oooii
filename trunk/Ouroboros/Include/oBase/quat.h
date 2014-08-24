/**************************************************************************
 * The MIT License                                                        *
 * Copyright (c) 2014 Antony Arciuolo.                                    *
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
// A quaterion. This header cross-compiles in C++ and HLSL.
#ifndef oHLSL
	#pragma once
#endif
#ifndef oBase_quat_h
#define oBase_quat_h

#include <oHLSL/oHLSLMacros.h>
#include <oHLSL/oHLSLMath.h>
#include <oHLSL/oHLSLTypes.h>

#ifdef oHLSL
	#define quatf float4
	#define quatd double4
#else

// Wrap in a namespace so that NoStepInto can be used for VS2010+.
namespace ouro {

template<typename T> struct quat
{
	typedef T element_type;
	T x,y,z,w;
	inline quat() {};
	explicit inline quat(const TVEC4<T>& _quat) : x(_quat.x), y(_quat.y), z(_quat.z), w(_quat.w) {}
	inline quat(const quat& _quat) : x(_quat.x), y(_quat.y), z(_quat.z), w(_quat.w) {}
	inline quat(T _X, T _Y, T _Z, T _W) : x(_X), y(_Y), z(_Z), w(_W) {}
	inline quat(const TVEC3<T>& _XYZ, T _W) : x(_XYZ.x), y(_XYZ.y), z(_XYZ.z), w(_W) {}
	oHLSL_VBRACKET_OP(T);
	inline const TVEC3<T>& xyz() const { return *(TVEC3<T>*)&x; }
	inline const TVEC4<T>& xyzw() const { return *(TVEC4<T>*)&x; }
	inline const quat<T>& operator*=(T _Scalar) { *this = *this * _Scalar; return *this; }
	inline const quat<T>& operator+=(const quat<T> _quat) { *this = *this + _quat; return *this; }
	inline const quat<T>& operator-=(const quat<T> _quat) { *this = *this - _quat; return *this; }
	inline quat<T> operator-() const { return quat<T>(-x, -y, -z, -w); }
	inline quat<T> operator*(T _Scalar) const { return quat<T>(x * _Scalar, y * _Scalar, z * _Scalar, w * _Scalar); }
};

}

typedef ouro::quat<float> quatf; typedef ouro::quat<double> quatd;

oHLSL_ELOPT4_VECTOR(ouro::quat, +) oHLSL_ELOPT4_VECTOR(ouro::quat, -) oHLSL_ELOPT4_SCALAR(ouro::quat, *) oHLSL_ELOPT4_SCALAR(ouro::quat, /) oHLSL_ELOPT4_SCALAR(ouro::quat, +) oHLSL_ELOPT4_SCALAR(ouro::quat, -)

static const quatd identity_quatd = quatd(0.0, 0.0, 0.0, 1.0);

#endif

#include <oHLSL/oHLSLSwizzlesOn.h> // must be below xyz() and xyzw() definitions for quat

static const quatf identity_quatf = quatf(0.0f, 0.0f, 0.0f, 1.0f);

// Multiply/combine two quats. Careful, remember that quats are not 
// communicative, so order matters. This returns a * b.
inline quatf qmul(oIN(quatf, a), oIN(quatf, b))
{
	// http://code.google.com/p/kri/wiki/quats
	return quatf(cross(a.xyz,b.xyz) + a.xyz*b.w + b.xyz*a.w, a.w*b.w - dot(a.xyz,b.xyz));
}

// Return the specified vector rotated by the specified quat
inline float3 qmul(oIN(quatf, q), oIN(float3, v))
{
	// http://code.google.com/p/kri/wiki/quats
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
