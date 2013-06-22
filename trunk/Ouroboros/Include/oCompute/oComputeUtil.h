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
// more primitive math manipulation functions.
#ifndef oHLSL
	#pragma once
#endif
#ifndef oComputeUtil_h
#define oComputeUtil_h

#include <oHLSL/oHLSLMath.h>
#include <oHLSL/oHLSLMacros.h>
#include <oCompute/oQuaternion.h>
#include <oHLSL/oHLSLSwizzlesOn.h>

#ifndef oCONCAT
	#define oCONCAT(x, y) x##y
#endif

// _____________________________________________________________________________
// Space Transformation. To abstract row-major v. col-major.

#ifndef oMATRIX_COLUMN_MAJOR
	#define oMATRIX_COLUMN_MAJOR 
#endif

// Order will always be the same with this function, regardless of matrix
// representation. Prefer oMul to mul
inline float4 oMul(oIN(float4x4, m), oIN(float4, v))
{
	#ifdef oMATRIX_COLUMN_MAJOR
		return mul(m, v);
	#else
		return mul(v, m);
	#endif
}

inline float3 oMul(oIN(float3x3, m), oIN(float3, v))
{
	#ifdef oMATRIX_COLUMN_MAJOR
		return mul(m, v);
	#else
		return mul(v, m);
	#endif
}

// _____________________________________________________________________________
// oSwap in a compatible way (no templates in hlsl)

#define oMATH_DEFINE_OSWAP(_Type) inline void oSwap(oINOUT(_Type, A), oINOUT(_Type, B)) { _Type C = A; A = B; B = C; }

oMATH_DEFINE_OSWAP(float)
oMATH_DEFINE_OSWAP(float2)
oMATH_DEFINE_OSWAP(float3)
oMATH_DEFINE_OSWAP(float4)

#ifndef oHLSL
	oMATH_DEFINE_OSWAP(quatf)
#endif

oMATH_DEFINE_OSWAP(int)
oMATH_DEFINE_OSWAP(int2)
oMATH_DEFINE_OSWAP(int3)
oMATH_DEFINE_OSWAP(int4)
oMATH_DEFINE_OSWAP(uint)
oMATH_DEFINE_OSWAP(uint2)
oMATH_DEFINE_OSWAP(uint3)
oMATH_DEFINE_OSWAP(uint4)

// _____________________________________________________________________________
// Component ordering operators

inline float min(oIN(float2, a)) { return min(a.x, a.y); }
inline float min(oIN(float3, a)) { return min(min(a.x, a.y), a.z); }
inline float min(oIN(float4, a)) { return min(min(min(a.x, a.y), a.z), a.w); }
inline int min(oIN(int2, a)) { return min(a.x, a.y); }
inline int min(oIN(int3, a)) { return min(min(a.x, a.y), a.z); }
inline int min(oIN(int4, a)) { return min(min(min(a.x, a.y), a.z), a.w); }
inline uint min(oIN(uint2, a)) { return min(a.x, a.y); }
inline uint min(oIN(uint3, a)) { return min(min(a.x, a.y), a.z); }
inline uint min(oIN(uint4, a)) { return min(min(min(a.x, a.y), a.z), a.w); }

// Returns the largest value in the specified vector
inline float max(oIN(float2, a)) { return max(a.x, a.y); }
inline float max(oIN(float3, a)) { return max(max(a.x, a.y), a.z); }
inline float max(oIN(float4, a)) { return max(max(max(a.x, a.y), a.z), a.w); }
inline int max(oIN(int2, a)) { return max(a.x, a.y); }
inline int max(oIN(int3, a)) { return max(max(a.x, a.y), a.z); }
inline int max(oIN(int4, a)) { return max(max(max(a.x, a.y), a.z), a.w); }
inline uint max(oIN(uint2, a)) { return max(a.x, a.y); }
inline uint max(oIN(uint3, a)) { return max(max(a.x, a.y), a.z); }
inline uint max(oIN(uint4, a)) { return max(max(max(a.x, a.y), a.z), a.w); }

// _____________________________________________________________________________
// Geometry / Linear Algebra

// Returns the angle between the two specified vectors (vectors are not assumed
// to be normalized).
inline float angle(oIN(float3, a), oIN(float3, b)) { return acos(dot(a, b) / (length(a) * length(b))); }

// Returns just the "rotation" part of matrix. Careful, if there's scale, then
// this will loose shear parameters.
inline float3x3 oGetUpper3x3(oIN(float4x4, m))
{
	#ifdef oHLSL
		#pragma warning(disable:3206)
	#endif
	return (float3x3)m;
	#ifdef oHLSL
		#pragma warning(default:3206)
	#endif
}

// Returns true of there is a projection/perspective or false if orthographic
inline bool oHasPerspective(oIN(float4x4, _Matrix))
{
	return (_Matrix[0][3] != 0.0f || _Matrix[1][3] != 0.0f || _Matrix[2][3] != 0.0f);
}

// _____________________________________________________________________________
// Simple Random Numbers

// A simple LCG rand() function, unshifted/masked
inline uint oRandUnmasked(uint Seed)
{
	return 1103515245 * Seed + 12345;
}

// Another simple shader-ready RNG based on a 2D coord.
inline float oRand(float2 _Coord)
{
	static const float GelfondsConstant = 23.1406926327792690f; // e ^ pi
	static const float GelfondSchneiderConstant = 2.6651441426902251f; // 2 ^ sqrt(2)
	return frac(cos(fmod(123456789.0f, 1e-7f + 256.0f * dot(_Coord, float2(GelfondsConstant, GelfondSchneiderConstant)))));
}

// _____________________________________________________________________________
// Normalmap and tangent basis utilities

// Returns an unnormalized normal in world space. If the return value is to be
// passed onto a Toksvig-scaled lighting model, then the input vertex vectors do 
// not need to be normalized.
inline float3 oDecodeTSNormal(oIN(float3, _WSVertexTangentVector)
	, oIN(float3, _WSVertexBitangentVector)
	, oIN(float3, _WSVertexNormalVector)
	, oIN(float3, _TSNormalMapColor)
	, float _BumpScale)
{
	const float3 TSNormal = _TSNormalMapColor*2.0f - 1.0f;
	return TSNormal.x*_BumpScale*_WSVertexTangentVector + TSNormal.y*_BumpScale*_WSVertexBitangentVector + TSNormal.z*_WSVertexNormalVector;
}

// Handedness is stored in w component of _InTangent. This should be called out 
// of the vertex shader and the out values passed through to the pixel shader.
// For world-space, pass the world matrix. For view space, pass the WorldView
// matrix. Read more: http://www.terathon.com/code/tangent.html.
inline void oTransformTangentBasisVectors(oIN(float4x4, _Matrix)
	, oIN(float3, _InNormalVector)
	, oIN(float4, _InTangentVectorAndFacing)
	, oOUT(float3, _OutNormalVector)
	, oOUT(float3, _OutTangentVector)
	, oOUT(float3, _OutBitangentVector))
{
	float3x3 R = oGetUpper3x3(_Matrix);
	_OutNormalVector = mul(R, _InNormalVector);
	_OutTangentVector = mul(R, _InTangentVectorAndFacing.xyz);
	_OutBitangentVector = cross(_OutNormalVector, _OutTangentVector) * _InTangentVectorAndFacing.w;
}

inline void oQRotateTangentBasisVectors(oIN(quatf, _Quaternion)
	, oIN(float3, _InNormalVector)
	, oIN(float4, _InTangentVectorAndFacing)
	, oOUT(float3, _OutNormalVector), oOUT(float3, _OutTangentVector), oOUT(float3, _OutBitangentVector))
{
	_OutNormalVector = qmul(_Quaternion, _InNormalVector);
	_OutTangentVector = qmul(_Quaternion, _InTangentVectorAndFacing.xyz);
	_OutBitangentVector = cross(_OutNormalVector, _OutTangentVector) * _InTangentVectorAndFacing.w;
}

// _____________________________________________________________________________
// Misc

// Creates a cube in a geometry shader (useful for voxel visualization. To use,
// generate indices in a for (uint i = 0; i < 14; i++) loop (14 iterations).
inline float3 oGSCubeCalcVertexPosition(uint _Index, oIN(float3, _Offset), oIN(float3, _Scale))
{
	static const float3 oGSCubeStripCW[] = 
	{
		float3(-0.5f,0.5f,-0.5f), // left top front
		float3(0.5f,0.5f,-0.5f), // right top front
		float3(-0.5f,-0.5f,-0.5f), // left bottom front
		float3(0.5f,-0.5f,-0.5f), // right bottom front
		float3(0.5f,-0.5f,0.5f), // right bottom back
		float3(0.5f,0.5f,-0.5f), // right top front
		float3(0.5f,0.5f,0.5f), // right top back
		float3(-0.5f,0.5f,-0.5f), // left top front
		float3(-0.5f,0.5f,0.5f), // left top back
		float3(-0.5f,-0.5f,-0.5f), // left bottom front 7
		float3(-0.5f,-0.5f,0.5f), // left bottom back
		float3(0.5f,-0.5f,0.5f), // right bottom back 5
		float3(-0.5f,0.5f,0.5f), // left top back
		float3(0.5f,0.5f,0.5f), // right top back
	};

	return _Offset + oGSCubeStripCW[_Index] * _Scale;
}

#include <oHLSL/oHLSLSwizzlesOff.h>
#endif
