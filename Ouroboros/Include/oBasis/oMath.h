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

// Ouroboros Math Library. This math library attempts to conform to HLSL (SM5) 
// as closely as possible. To reduce typing, templates and macros are used 
// extensively, but the code should still be simpler than the alternative.
// There are additional APIs as well that extend beyond HLSL as well, but try to 
// keep the spirit of hlsl in mind.
//
//
#pragma once
#ifndef oMath_h
#define oMath_h

#include <Math.h>
#include <oHLSL/oHLSLMath.h>
#include <oCompute/oComputeConstants.h>
#include <oBase/types.h>
#include <oBase/aabox.h>
#include <oBase/plane.h>
#include <oBase/quat.h>
#include <oBase/sphere.h>
#include <oCompute/oFrustum.h>
#include <oCompute/linear_algebra.h>
#include <limits>

#include <oHLSL/oHLSLIntrinsics.h> // for _BitScanReverse

// _____________________________________________________________________________
// Denormalized float functions

inline bool isdenorm(const float& a)
{
	int x = *(int*)&a;
	int mantissa = x & 0x007fffff;
	int exponent = x & 0x7f800000;
	return mantissa && !exponent;
}

template<typename T> inline T zerodenorm(const T& a)
{
	// @tony: This constant probably isn't correct for doubles, but doing 
	// the template thing means it works for vector types.
	const T ANTI_DENORM(1e-18f);
	// http://citeseerx.ist.psu.edu/viewdoc/download?doi=10.1.1.20.1348&rep=rep1&type=pdf
	T tmp = a + ANTI_DENORM;
	tmp -= ANTI_DENORM;
	return tmp;
}

// _____________________________________________________________________________
// Miscellaneous utility functions

inline int pow(int a, int e) { int v = 1; for (int i = 0; i < e; i++) v *= a; return v; }
inline unsigned int pow(unsigned int a, unsigned int e) { unsigned int v = 1; for (unsigned int i = 0; i < e; i++) v *= a; return v; }

// Returns the next power of 2 from the current value.
// So if _Value is 512 this will return 1024
inline unsigned long oNextPow2(unsigned long _Value)
{
	unsigned long pow2;
	_BitScanReverse(&pow2, _Value);
	++pow2;
	return 1 << pow2;
}

// asfloat() does a reinterpret_cast (analyzes bits separate from value). 
// oCastAsFloat does a static_cast.
template<typename T> float oCastAsFloat(const T& value) { return static_cast<float>(value); }
template<typename T> float2 oCastAsFloat(const TVEC2<T>& value) { return float2(oCastAsFloat(value.x), oCastAsFloat(value.y)); }
template<typename T> float3 oCastAsFloat(const TVEC3<T>& value) { return float3(oCastAsFloat(value.x), oCastAsFloat(value.y), oCastAsFloat(value.z)); }
template<typename T> float4 oCastAsFloat(const TVEC4<T>& value) { return float4(oCastAsFloat(value.x), oCastAsFloat(value.y), oCastAsFloat(value.z), oCastAsFloat(value.w)); }
template<typename T> float4x4 oCastAsFloat(const TMAT4<T>& value) { return float4x4(oCastAsFloat(value.Column0), oCastAsFloat(value.Column1), oCastAsFloat(value.Column2), oCastAsFloat(value.Column3)); }

template<typename T> int oCastAsInt(const T& value) { return static_cast<int>(value); }
template<typename T> int2 oCastAsInt(const TVEC2<T>& value) { return int2(oCastAsInt(value.x), oCastAsInt(value.y)); }
template<typename T> int3 oCastAsInt(const TVEC3<T>& value) { return int3(oCastAsInt(value.x), oCastAsInt(value.y), oCastAsInt(value.z)); }
template<typename T> int4 oCastAsInt(const TVEC4<T>& value) { return int4(oCastAsInt(value.x), oCastAsInt(value.y), oCastAsInt(value.z), oCastAsInt(value.w)); }

template<typename T> uint oCastAsUint(const T& value) { return static_cast<uint>(value); }
template<typename T> uint2 oCastAsUint(const TVEC2<T>& value) { return uint2(oCastAsUint(value.x), oCastAsUint(value.y)); }
template<typename T> uint3 oCastAsUint(const TVEC3<T>& value) { return uint3(oCastAsUint(value.x), oCastAsUint(value.y), oCastAsUint(value.z)); }
template<typename T> uint4 oCastAsUint(const TVEC4<T>& value) { return uint4(oCastAsUint(value.x), oCastAsUint(value.y), oCastAsUint(value.z), oCastAsUint(value.w)); }

// _____________________________________________________________________________
// Containment/intersection/base collision

template<typename T> ouro::aabox<T, TVEC3<T>> oGetBoundingAABox(const ouro::sphere<T>& _Sphere) { return ouro::aabox<T, TVEC3<T>>(TVEC3<T>(_Sphere.GetPosition() - _Sphere.radius()), TVEC3<T>(_Sphere.GetPosition() + _Sphere.radius())); }

enum oCONTAINMENT
{
	oNOT_CONTAINED,
	oPARTIALLY_CONTAINED,
	oWHOLLY_CONTAINED,
};

template<typename T> oCONTAINMENT oContains(const ouro::frustum<T>& _Frustum, const ouro::aabox<T,TVEC3<T>>& _Box);
template<typename T> oCONTAINMENT oContains(const ouro::frustum<T>& _Frustum, const TVEC4<T>& _Sphere);
template<typename T> oCONTAINMENT oContains(const ouro::sphere<T>& _Sphere, const ouro::aabox<T,TVEC3<T>>& _Box);

template<typename T> bool oIntersects(const ouro::frustum<T>& _Frustum, const ouro::aabox<T,TVEC3<T>>& _Box) { return oNOT_CONTAINED != oContains(_Frustum, _Box); }
template<typename T> bool oIntersects(const ouro::frustum<T>& _Frustum, const TVEC4<T>& _Sphere) 
{ 
  return oNOT_CONTAINED != oContains(_Frustum, _Sphere); 
}
template<typename T> bool oIntersects(const ouro::sphere<T>& _Sphere, const ouro::aabox<T,TVEC3<T>>& _Box) { return oNOT_CONTAINED != oContains(_Sphere, _Box); }

// _____________________________________________________________________________
// Miscellaneous

template<typename T> ouro::rect oToRect(const T& _Rect);

// Takes a rectangle and breaks it into _MaxNumSplits rectangles
// where each rectangle's area is a % of the source rectangle approximated 
// by its value in _pOrderedSplitRatio.  The sum of the values in _pOrderedSplitRatio
// must be 1.0f and decreasing in size.  SplitRect returns the number of splits
// it could do (which may be less than _MaxNumSplits when the ratios are too small)
unsigned int SplitRect(const ouro::rect& _SrcRect, const unsigned int _MaxNumSplits, const float* _pOrderedSplitRatio, const unsigned int _XMultiple, const unsigned int _YMultiple, ouro::rect* _pSplitResults);

// _____________________________________________________________________________
// Basic Octree-related calculations. The depth index of the root is 0, there 
// are 8 octants at depth index 1.

template<typename T> T oOctreeCalcNumNodesAtLevel(T _DepthIndex) { return pow(T(8), _DepthIndex); }

// Returns the number of octants that tessellate the root volume at the 
// specified level.
template<typename T> T oOctreeCalcSubdivisionAtLevel(T _DepthIndex) { return pow(T(2), _DepthIndex); }

// This takes the depth of the tree, not a particular depth index. The depth of 
// an octree that has only the root node is 1.
template<typename T> T oOctreeCalcTotalNumNodes(T _Depth)
{
	T n = 0;
	for (T i = 0; i < _Depth; i++)
		n += oOctreeCalcNumNodesAtLevel(i);
	return n;
}

#endif
