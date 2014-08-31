// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.
// A bounding sphere. This header cross-compiles in C++ and HLSL.
#ifndef oHLSL
	#pragma once
#endif
#ifndef oBase_sphere_h
#define oBase_sphere_h

#include <oHLSL/oHLSLMacros.h>
#include <oHLSL/oHLSLMath.h>
#include <oHLSL/oHLSLTypes.h>

#ifdef oHLSL

#define spheref float4
#define sphered double4

#else
namespace ouro {

template<typename T> struct sphere : public TVEC4<T>
{
	typedef T element_type;
	typedef TVEC3<element_type> vector_type;
	typedef TVEC4<element_type> base_type;
	sphere() {}
	sphere(const sphere& _That) { operator=(_That); }
	sphere(const base_type& _That) { operator=(_That); }
	sphere(const vector_type& _Position, element_type _Radius) : base_type(_Position, _Radius) {}
	element_type radius() const { return w; }
	const sphere& operator=(const sphere& _That) { return operator=(*(base_type*)&_That); }
	const sphere& operator=(const base_type& _That) { *(base_type*)this = _That; return *this; }
	operator base_type() { return *this; }
};

template<typename T> inline bool equal(const sphere<T>& a, const sphere<T>& b, unsigned int maxUlps) { return equal(a.x, b.x, maxUlps) && equal(a.y, b.y, maxUlps) && equal(a.z, b.z, maxUlps) && equal(a.w, b.w, maxUlps); }

}

typedef ouro::sphere<float> spheref; typedef ouro::sphere<double> sphered;

#endif
#endif
