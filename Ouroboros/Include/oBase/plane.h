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
// A plane defined as Ax + By + Cz + D = 0. Primarily it means positive D 
// values are in the direction/on the side of the normal and negative 
// values are in the opposite direction/on the opposite side of the normal.
// This header cross-compiles for C++ and HLSL.
#ifndef oHLSL
	#pragma once
#endif
#ifndef oBase_plane_h
#define oBase_plane_h

#include <oHLSL/oHLSLMacros.h>
#include <oHLSL/oHLSLMath.h>
#include <oHLSL/oHLSLTypes.h>

#ifdef oHLSL

#define planef float4
#define planed double4

#else

#include <oBase/equal.h>

// Wrap in a namespace so that NoStepInto can be used for VS2010+.
namespace ouro {

template<typename T> struct plane : public TVEC4<T>
{
	typedef T element_type;
	typedef TVEC3<element_type> vector_type;
	typedef TVEC4<element_type> base_type;
	plane() {}
	plane(const plane& _That) { operator=(_That); }
	plane(const base_type& _That) { operator=(_That); }
	plane(const vector_type& _Normal, const element_type& _Offset) : base_type(normalize(_Normal), _Offset) {}
	plane(const element_type& _NormalX, const element_type& _NormalY, const element_type& _NormalZ, const element_type& _Offset) : base_type(normalize(vector_type(_NormalX, _NormalY, _NormalZ)), _Offset) {}
	plane(const vector_type& _Normal, const vector_type& _Point) { xyz() = normalize(_Normal); w = dot(n, _Point); }
	const plane& operator=(const plane& _That) { return operator=(*(base_type*)&_That); }
	const plane& operator=(const base_type& _That) { *(base_type*)this = _That; return *this; }
	operator base_type() { return *this; }
};

template<typename T> inline bool equal(const plane<T>& a, const plane<T>& b, unsigned int maxUlps) { return equal(a.x, b.x, maxUlps) && equal(a.y, b.y, maxUlps) && equal(a.z, b.z, maxUlps) && equal(a.w, b.w, maxUlps); }

}

typedef ouro::plane<float> planef; typedef ouro::plane<double> planed;

#endif

#include <oHLSL/oHLSLSwizzlesOn.h>

inline planef normalize_plane(oIN(planef, _Plane)) { float invLength = rsqrt(dot(_Plane.xyz, _Plane.xyz)); return _Plane * invLength; }
inline planed normalize_plane(oIN(planed, _Plane)) { double invLength = rsqrt(dot(_Plane.xyz, _Plane.xyz)); return _Plane * invLength; }

// Signed distance from a plane in Ax + By + Cz + Dw = 0 format (ABC = normalized normal, D = offset)
// This assumes the plane is normalized.
// >0 means on the same side as the normal
// <0 means on the opposite side as the normal
// 0 means on the plane
inline float sdistance(oIN(planef, _Plane), oIN(float3, _Point)) { return dot(_Plane.xyz, _Point) + _Plane.w; }
inline double sdistance(oIN(planed, _Plane), oIN(double3, _Point)) { return dot(_Plane.xyz, _Point) + _Plane.w; }
inline float distance(oIN(planef, _Plane), oIN(float3, _Point)) { return abs(sdistance(_Plane, _Point));  }
inline double distance(oIN(planed, _Plane), oIN(double3, _Point)) { return abs(sdistance(_Plane, _Point));  }

inline bool in_front_of(oIN(planef, _Plane), oIN(float3, _Point)) { return sdistance(_Plane, _Point) > 0.0f; }

inline bool intersects(oIN(planef, _Plane0), oIN(planef, _Plane1), oIN(planef, _Plane2), oOUT(float3, _Intersection))
{
	// Goldman, Ronald. Intersection of Three Planes. In A. Glassner,
	// ed., Graphics Gems pg 305. Academic Press, Boston, 1991.

	// intersection = (P0.V0)(V1XV2) + (P1.V1)(V2XV0) + (P2.V2)(V0XV1)/Det(V0,V1,V2)
	// Vk is the plane unit normal, Pk is a point on the plane
	// Note that P0 dot V0 is the same as d in abcd form.

	// http://paulbourke.net/geometry/3planes/

	// check that there is a valid cross product
	float3 P1X2 = cross(_Plane1.xyz, _Plane2.xyz);
	if (ouro::equal(dot(P1X2,P1X2), 0.0f)) 
		return false;

	float3 P2X0 = cross(_Plane2.xyz, _Plane0.xyz);
	if (ouro::equal(dot(P2X0,P2X0), 0.0f)) 
		return false;

	float3 P0X1 = cross(_Plane0.xyz, _Plane1.xyz);
	if (ouro::equal(dot(P0X1,P0X1), 0.0f)) 
		return false;

	_Intersection = (-_Plane0.w * P1X2 - _Plane1.w * P2X0 - _Plane2.w * P0X1) / determinant(float3x3(_Plane0.xyz, _Plane1.xyz, _Plane2.xyz));
	return true;
}

// Calculate the point on this plane where the line segment described by p0 and 
// p1 intersects.
inline bool intersects(oIN(float4, _Plane), oIN(float3, _Point0), oIN(float3, _Point1), oOUT(float3, _Intersection))
{
	bool intersects = true;
	float d0 = distance(_Plane, _Point0);
	float d1 = distance(_Plane, _Point1);
	bool in0 = 0.0f > d0;
	bool in1 = 0.0f > d1;

	if ((in0 && in1) || (!in0 && !in1)) // totally in or totally out
		intersects = false;
	else // partial
	{
		// the intersection point is along p0,p1, so p(t) = p0 + t(p1 - p0)
		// the intersection point is on the plane, so (p(t) - C) . N = 0
		// with above distance function, C is 0,0,0 and the offset along 
		// the normal is considered. so (pt - c) . N is distance(pt)

		// (p0 + t ( p1 - p0 ) - c) . n = 0
		// p0 . n + t (p1 - p0) . n - c . n = 0
		// t (p1 - p0) . n = c . n - p0 . n
		// ((c - p0) . n) / ((p1 - p0) . n)) 
		//  ^^^^^^^ (-(p0 -c)) . n: this is why -distance

		float3 diff = _Point1 - _Point0;
		float denom = dot(diff, _Plane.xyz);

		if (abs(denom) < FLT_EPSILON)
			return false;

		float t = -distance(_Plane, _Point0) / denom;
		_Intersection = _Point0 + t * _Point1;
		intersects = true;
	}

	return intersects;
}

#include <oHLSL/oHLSLSwizzlesOff.h>

#endif
