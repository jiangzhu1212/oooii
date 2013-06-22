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

// OOOii Math Library. This math library attempts to conform to HLSL (SM5) as 
// closely as possible. To reduce typing, templates and macros are used 
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
#include <oBasis/oMathTypes.h>
#include <limits>

#include <oStd/intrinsics.h> // for _BitScanReverse

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
	// @oooii-tony: This constant probably isn't correct for doubles, but doing 
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

inline unsigned char oUNORMAsUBYTE(float x) { return static_cast<unsigned char>(floor(x * 255.0f + 0.5f)); }
inline unsigned short oUNORMAsUSHORT(float x) { return static_cast<unsigned char>(floor(x * 65535.0f + 0.5f)); }
inline float oUBYTEAsUNORM(size_t c) { return (c & 0xff) / 255.0f; }
inline float oUSHORTAsUNORM(size_t c) { return (c & 0xffff) / 65535.0f; }

// _____________________________________________________________________________
// Containment/intersection/base collision

template<typename T> oCompute::aabox<T, TVEC3<T>> oGetBoundingAABox(const oCompute::sphere<T>& _Sphere) { return oCompute::aabox<T, TVEC3<T>>(TVEC3<T>(_Sphere.GetPosition() - _Sphere.radius()), TVEC3<T>(_Sphere.GetPosition() + _Sphere.radius())); }

enum oCONTAINMENT
{
	oNOT_CONTAINED,
	oPARTIALLY_CONTAINED,
	oWHOLLY_CONTAINED,
};

template<typename T> oCONTAINMENT oContains(const oCompute::frustum<T>& _Frustum, const oCompute::aabox<T,TVEC3<T>>& _Box);
template<typename T> oCONTAINMENT oContains(const oCompute::frustum<T>& _Frustum, const TVEC4<T>& _Sphere);
template<typename T> oCONTAINMENT oContains(const oCompute::sphere<T>& _Sphere, const oCompute::aabox<T,TVEC3<T>>& _Box);
template<typename T, typename TVec> oCONTAINMENT oContains(const oCompute::aabox<T, TVec>& _Box0, const oCompute::aabox<T, TVec>& _Box1);
template<typename T> oCONTAINMENT oContains(float3 _Point, const oCompute::aabox<T,TVEC3<T>>& _Box);

template<typename T> bool oIntersects(const oCompute::frustum<T>& _Frustum, const oCompute::aabox<T,TVEC3<T>>& _Box) { return oNOT_CONTAINED != oContains(_Frustum, _Box); }
template<typename T> bool oIntersects(const oCompute::frustum<T>& _Frustum, const TVEC4<T>& _Sphere) 
{ 
  return oNOT_CONTAINED != oContains(_Frustum, _Sphere); 
}
template<typename T> bool oIntersects(const oCompute::sphere<T>& _Sphere, const oCompute::aabox<T,TVEC3<T>>& _Box) { return oNOT_CONTAINED != oContains(_Sphere, _Box); }
template<typename T, typename TVec> bool oIntersects(const oCompute::aabox<T, TVec>& _Box0, const oCompute::aabox<T, TVec>& _Box1) { return oNOT_CONTAINED != oContains(_Box0, _Box1); }

// _____________________________________________________________________________
// Miscellaneous

bool oCalculateAreaAndCentriod(float* _pArea, float2* _pCentroid, const float2* _pVertices, size_t _VertexStride, size_t _NumVertices);

// Determines a location in 3D space based on 4 reference locations and their distances from the location
template<typename T> T oTrilaterate(const TVEC3<T> observers[4], const T distances[4], TVEC3<T>* position);
template<typename T>
inline T oTrilaterate(const TVEC3<T> observers[4], T distance, TVEC3<T>* position)
{
	T distances[4];
	for(int i = 0; i < 4; ++i)
		distances[i] = distance;
	return oTrilaterate(observers, distances, position);	
}
// Computes a matrix to move from one coordinate system to another based on 4 known reference locations in the start and end systems assuming uniform units
template<typename T> bool oCoordinateTransform(const TVEC3<T> startCoords[4], const TVEC3<T> endCoords[4], TMAT4<T> *matrix);

// Computes the gaussian weight of a specific sample in a 1D kernel 
inline float GaussianWeight(float stdDeviation, int sampleIndex)
{
	return (1.0f / (sqrt(2.0f * oPIf) * stdDeviation)) * pow(oEf, -((float)(sampleIndex * sampleIndex) / (2.0f * stdDeviation * stdDeviation)));
}

template<typename T> oRECT oToRect(const T& _Rect);

// Takes a rectangle and breaks it into _MaxNumSplits rectangles
// where each rectangle's area is a % of the source rectangle approximated 
// by its value in _pOrderedSplitRatio.  The sum of the values in _pOrderedSplitRatio
// must be 1.0f and decreasing in size.  SplitRect returns the number of splits
// it could do (which may be less than _MaxNumSplits when the ratios are too small)
unsigned int SplitRect(const oRECT& _SrcRect, const unsigned int _MaxNumSplits, const float* _pOrderedSplitRatio, const unsigned int _XMultiple, const unsigned int _YMultiple, oRECT* _pSplitResults);

// Forward biorthogonal CDF 9/7 wavelet transform, transforms in place
void oCDF97Fwd(float* _pValues, size_t _NumValues);

// Inverse biorthogonal CDF 9/7 wavelet transform, transforms in place
void oCDF97Inv(float* _pValues, size_t _NumValues);

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

template<typename T> T oSinc(T _Value)
{
	if (abs(_Value) > std::numeric_limits<T>::epsilon())
	{
		_Value *= T(oPI);
		return sin(_Value) / _Value;
	} 
	return T(1);
}

// A small class to encapsulate calculating a cumulative moving average
template<typename T>
class oMovingAverage
{
public:
	oMovingAverage()
		: CA(0)
		, Count(0)
	{}

	T Calc(const T& _Value)
	{
		Count += T(1);
		CA = CA + ((_Value - CA) / Count);
		return CA;
	}

private:
	T CA;
	T Count;
};

#endif
