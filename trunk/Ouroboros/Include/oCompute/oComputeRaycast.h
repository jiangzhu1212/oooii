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
// This code contains code that cross-compiles in C++ and HLSL. This contains
// utility code for voxel-based raycasting.
#ifndef oHLSL
	#pragma once
#endif
#ifndef oComputeRaycast_h
#define oComputeRaycast_h

#include <oCompute/oComputeColor.h>

#include <oHLSL/oHLSLMacros.h>
#include <oHLSL/oHLSLMath.h>
#include <oCompute/oQuaternion.h>
#include <oHLSL/oHLSLSwizzlesOn.h>

// _____________________________________________________________________________
// Matrix code

// @oooii-kevin: FIXME, this is borrowed from bullet, promote this throughout the rest
// or our code
inline float3x3 make_rotationHLSL( float radians, const float3 unitVec)
{
	float x, y, z, s, c, oneMinusC, x_y, y_z, z_x;
	s = sin( radians );
	c = cos( radians );
	x = unitVec.x;
	y = unitVec.y;
	z = unitVec.z;
	x_y = ( x * y );
	y_z = ( y * z );
	z_x = ( z * x );
	oneMinusC = ( 1.0f - c );
	return float3x3(
		float3( ( ( ( x * x ) * oneMinusC ) + c ), ( ( x_y * oneMinusC ) + ( z * s ) ), ( ( z_x * oneMinusC ) - ( y * s ) )),
		float3( ( ( x_y * oneMinusC ) - ( z * s ) ), ( ( ( y * y ) * oneMinusC ) + c ), ( ( y_z * oneMinusC ) + ( x * s ) )),
		float3( ( ( z_x * oneMinusC ) + ( y * s ) ), ( ( y_z * oneMinusC ) - ( x * s ) ), ( ( ( z * z ) * oneMinusC ) + c ))
		);
}

inline float3x3 make_rotationHLSL(const float3 _NormalizedSrcVec, const float3 _NormalizedDstVec)
{
	float a = angle(_NormalizedSrcVec, _NormalizedDstVec);

	// Check for identity
	if (0.0f == a)
		return oIDENTITY3x3;

	// Check for flip
	if(oPI == a)
		return float3x3(-oXAXIS3, -oYAXIS3, -oZAXIS3);

	float3 NormalizedAxis = normalize(cross(_NormalizedSrcVec, _NormalizedDstVec));
	return make_rotationHLSL(a, NormalizedAxis);
}

// _____________________________________________________________________________
// Voxel Grid style raycasting support

// 3DDA (3D Digital Differential Analyzer) as explained here
// http://www.cse.yorku.ca/~amana/research/grid.pdf
// This ensures all voxels are 6-connected which is required when raycasting
// thin voxel shells. 3DDA Rays can only be cast recursively hence the 
// 3DDARayContext. To jump large volumes, Bresenham rays can be used.

struct o3DDARay
{
	float3 Delta;
	int3 Step;
};

inline o3DDARay oDeltaRayCreate(oIN(float3, _Ray))
{
	o3DDARay DDARay;

	float3 AbsRay = _Ray;
	DDARay.Step = 1;

	if(_Ray.x < 0.0f)
	{
		DDARay.Step.x *= -1;
		AbsRay.x *= -1.0f;
	}
	if(_Ray.y < 0.0f)
	{
		DDARay.Step.y *= -1;
		AbsRay.y *= -1.0f;
	}
	if(_Ray.z < 0.0f)
	{
		DDARay.Step.z *= -1;
		AbsRay.z *= -1.0f;
	}

	// 1.0f / abs( _Ray ) which is how far you have to step along the ray to step 1 unit in each axis
	DDARay.Delta.x = AbsRay.x > FLT_EPSILON ? rcp(AbsRay.x) : FLT_MAX; 
	DDARay.Delta.y = AbsRay.y > FLT_EPSILON ? rcp(AbsRay.y) : FLT_MAX;
	DDARay.Delta.z = AbsRay.z > FLT_EPSILON ? rcp(AbsRay.z) : FLT_MAX;

	return DDARay;
}

struct o3DDARayContext
{
#ifndef oHLSL
	bool operator==(const o3DDARayContext& _rhs) const { return all(_rhs.CurrentPosition == CurrentPosition); }
#endif
	float3 DeltaSum;
	int3 CurrentPosition;
};

inline o3DDARayContext o3DDARayContextCreate(const o3DDARay _Ray, const int3 _StartPosition = 0)
{
	o3DDARayContext Context;

	// This is an extension to the above sourced algorithm that ensures we start with the major axis
	// and don't walk down degenerate axis.
	Context.DeltaSum = _Ray.Delta;
	Context.CurrentPosition = _StartPosition;
	return Context;
}

inline o3DDARayContext o3DDARayCast(const o3DDARay _DeltaRay, o3DDARayContext _Context)
{
	if(_Context.DeltaSum.x < _Context.DeltaSum.y)
	{
		if(_Context.DeltaSum.x < _Context.DeltaSum.z)
		{
			_Context.CurrentPosition.x += _DeltaRay.Step.x;
			_Context.DeltaSum.x += _DeltaRay.Delta.x;
		}
		else
		{
			_Context.CurrentPosition.z += _DeltaRay.Step.z;
			_Context.DeltaSum.z += _DeltaRay.Delta.z;
		}
	}
	else
	{
		if(_Context.DeltaSum.y < _Context.DeltaSum.z)
		{
			_Context.CurrentPosition.y += _DeltaRay.Step.y;
			_Context.DeltaSum.y += _DeltaRay.Delta.y;
		}
		else
		{
			_Context.CurrentPosition.z += _DeltaRay.Step.z;
			_Context.DeltaSum.z += _DeltaRay.Delta.z;
		}
	}
	return _Context;
}

// Creates a "Bresenham Ray" which is a ray normalized by the length
// of the dominant axis.  This can then be used in place of a normalized
// ray for grid based raycasting which steps along the major axis
inline float3 oBresenhamRay(oIN(float3, _Ray))
{
	float3 FAbsRay = abs(_Ray);
	return _Ray / max(FAbsRay);
}

// Because this uses and returns an int it guarantees the value generated
// is a voxel that would be generated by o3DDARayCast.  A BresenHam ray
// can then be used to jump through a volume and continue stepping with 
// 3DDA raycasts
inline o3DDARayContext o3DDARayCastBresenham(const o3DDARay _DeltaRay, oIN(float3, _BresenhamRay), int _Delta)
{
	float3 FRay = floor(_BresenhamRay * (float)_Delta);
	o3DDARayContext DeltaContext;
	DeltaContext.CurrentPosition = FRay;
	DeltaContext.DeltaSum = (abs(FRay) + 1.0f) * _DeltaRay.Delta;
	return DeltaContext;
}

inline oAxis oFindDominantAxis(oIN(float3, _Ray))
{
	float3 FAbsRay = abs(_Ray);
	float FDominanatAxis = max(FAbsRay);
	if (FDominanatAxis <= FAbsRay.x)
		return oAXIS_X;
	else if (FDominanatAxis <= FAbsRay.y)
		return oAXIS_Y;
	return oAXIS_Z;
}

// Swaps X or Y axis with Z via Fwd and reverses the operation via Rev. This is 
// useful when doing dominant axis projections.
inline float3 oAxisRotFwdX(float3 _Value)
{
	return _Value.yzx;
}

inline float3 oAxisRotRevX(float3 _Value)
{
	return _Value.zxy;
}

inline float3 oAxisRotFwdY(float3 _Value)
{
	return _Value.xzy;
}

inline float3 oAxisRotRevY(float3 _Value)
{
	return _Value.xzy;
}

// _____________________________________________________________________________
// Misc functions

// For voxel-based irradiance, the scene is positioned on [0,1]
inline bool oVoxelInBounds(oIN(float3, _Point))
{
	return _Point.x >= 0.0f && _Point.x <= 1.0f
		&& _Point.y >= 0.0f && _Point.y <= 1.0f
		&& _Point.z >= 0.0f && _Point.z <= 1.0f;
}

inline bool oIVoxelInBounds(oIN(int3, _Point))
{
	return _Point.x >= 0 && _Point.x <= oMORTON_OCTREE_SCALAR
		&& _Point.y >= 0 && _Point.y <= oMORTON_OCTREE_SCALAR
		&& _Point.z >= 0 && _Point.z <= oMORTON_OCTREE_SCALAR;
}

// Intersects the ray with the Voxel space which is normalized 0x0x0->1x1x1
inline float3 oVoxelSpaceFindFirstIntersection(float3 vRayO, float3 vRayDir)
{
	// Intersect the ray with the bounding box
	// ( y - vRayO.y ) / vRayDir.y = t

	float fMaxT = -1;
	float t;
	float3 vRayIntersection;

	// -X plane
	if( vRayDir.x > 0 )
	{
		t = ( 0 - vRayO.x ) / vRayDir.x;
		fMaxT = max( t, fMaxT );
	}

	// +X plane
	if( vRayDir.x < 0 )
	{
		t = ( 1 - vRayO.x ) / vRayDir.x;
		fMaxT = max( t, fMaxT );
	}

	// -Y plane
	if( vRayDir.y > 0 )
	{
		t = ( 0 - vRayO.y ) / vRayDir.y;
		fMaxT = max( t, fMaxT );
	}

	// +Y plane
	if( vRayDir.y < 0 )
	{
		t = ( 1 - vRayO.y ) / vRayDir.y;
		fMaxT = max( t, fMaxT );
	}

	// -Z plane
	if( vRayDir.z > 0 )
	{
		t = ( 0 - vRayO.z ) / vRayDir.z;
		fMaxT = max( t, fMaxT );
	}

	// +Z plane
	if( vRayDir.z < 0 )
	{
		t = ( 1 - vRayO.z ) / vRayDir.z;
		fMaxT = max( t, fMaxT );
	}

	vRayIntersection = vRayO + vRayDir * fMaxT;

	return vRayIntersection;
}

// Returns the size in each major axis of a voxels at the specified depth. See
// from the math that at the root (DepthIndex=0) the size will be 1, which is 
// consistent with a normalized space such as is assumed in oVoxelInBounds().
inline float oVoxelCalcSize(uint _DepthIndex)
{
	return rcp(float(1 << _DepthIndex));
}

// Returns the resolution each dimension will have at a given Octree Depth
inline uint oVoxelCalcResolution(uint _OctreeDepth)
{
	return 1 << (_OctreeDepth - 1);
}

#include <oHLSL/oHLSLSwizzlesOff.h>
#endif