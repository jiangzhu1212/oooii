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
// This code contains code that cross-compiles in C++ and HLSL, but is beyond
// the vanilla HLSL specification. The goal is to create code that is singular
// in source, but usable in C++ and HLSL, so some coding standards and 
// conventions might be violated to favor HLSL compatibility.
#ifndef oHLSL
	#pragma once
#endif
#ifndef oMathShared_h
#define oMathShared_h

#define HALF_EPSILON	0.00097656f	// Smallest positive e for which
#define HALF_MAX		65504.0f	// Largest positive half

#ifdef oHLSL

	#define FLT_EPSILON     1.192092896e-07F        /* smallest such that 1.0+FLT_EPSILON != 1.0 */
	#define FLT_MAX         3.402823466e+38F        /* max value */

	#define oIN(Type, Param) in Type Param
	#define oOUT(Type, Param) out Type Param

	#define oHLSL_ALLOW_UAV_CONDITION [allow_uav_condition]
	#define oHLSL_LOOP [loop]
	#define oHLSL_UNROLL [unroll]
	#define oHLSL_UNROLL1(x) [unroll(x)]
	#define oHLSL_FASTOPT [fastopt]

#else

	#include <oBasis/oHLSLMath.h> // Add HLSL-spec math that is assumed in HLSL

	#define oIN(Type, Param) const Type& Param
	#define oOUT(Type, Param) Type& Param

	#define oHLSL_ALLOW_UAV_CONDITION
	#define oHLSL_LOOP
	#define oHLSL_UNROLL
	#define oHLSL_UNROLL1(x)
	#define oHLSL_FASTOP

	// Swizzle Compatibility
	// This might be dangerous, so include this header judiciously and use real
	// variable names so that macro substitution doesn't occur, but for now the 
	// benefits of cross-compiling code between C++ and shaders outweighs the 
	// intrusiveness of macro replacements on seemingly trivial strings. Not all
	// combinations are here, but we can add them over time.

	// float2 swizzles
	#define xy xy()
	#define yx yx()

	// float3 swizzles
	#define xyz xyz()
	#define xzy xzy()
	#define yxz yxz()
	#define yzx yzx()
	#define zxy zxy()
	#define zyx zyx()
	#define xy xy()
	#define xz xz()
	#define yx yx()
	#define yz yz()
	#define zx zx()
	#define zy zy()

	// float4 swizzles
	#define xyzw xyzw()
	#define wxyz wxyz()
	#define zwxy zwxy()
	#define yzwx yzwx()
	#define xyz xyz()
	#define xy xy()

#endif

// _____________________________________________________________________________
// Common math constants

#define oPI (3.14159265358979323846)
#define oPIf (float(oPI))
#define oE (2.71828183)
#define oEf (float(oE))

#define oSQRT2 (1.41421356237309504880)
#define oSQRT2f (float(oSQRT2))
#define oHALF_SQRT2 (0.70710678118654752440)
#define oHALF_SQRT2f (float(oHALF_SQRT2))

// sqrt(0.75f) this is the length from the center of a unit cube to one of the corners
#define oSQRT3Quarter (0.86602540378443860)
#define oSQRT3Quarterf (float(oSQRT3Quarter))

#define oDEFAULT_NEAR (10.0f)
#define oDEFAULT_FAR (10000.0f)

// NOTE: Epsilon for small float values is not defined. See "ulps" for more.

// _____________________________________________________________________________
// Identities

static const float3x3 float3x3_identity = float3x3 (
	float3(1.0, 0.0, 0.0),
	float3(0.0, 1.0, 0.0),
	float3(0.0, 0.0, 1.0));

// _____________________________________________________________________________
// Other constants

static const float3 oVECTOR_UP = float3(0.0f,1.0f,0.0f);

static const float2 oZERO2 = float2(0.0f,0.0f);

static const float3 oZERO3 = float3(0.0f,0.0f,0.0f);
static const float3 oBLACK3 = float3(0.0f,0.0f,0.0f);
static const float3 oWHITE3 = float3(1.0f,1.0f,1.0f);
static const float3 oRED3 = float3(1.0f,0.0f,0.0f);
static const float3 oGREEN3 = float3(0.0f,1.0f,0.0f);
static const float3 oBLUE3 = float3(0.0f,0.0f,1.0f);
static const float3 oYELLOW3 = float3(1.0f,1.0f,0.0f);
static const float3 oMAGENTA3 = float3(1.0f,0.0f,1.0f);
static const float3 oCYAN3 = float3(0.0f,1.0f,1.0f);

static const float4 oZERO4 = float4(0.0f,0.0f,0.0f,0.0f);
static const float4 oBLACK4 = float4(0.0f,0.0f,0.0f,1.0f);
static const float4 oWHITE4 = float4(1.0f,1.0f,1.0f,1.0f);
static const float4 oRED4 = float4(1.0f,0.0f,0.0f,1.0f);
static const float4 oGREEN4 = float4(0.0f,1.0f,0.0f,1.0f);
static const float4 oBLUE4 = float4(0.0f,0.0f,1.0f,1.0f);
static const float4 oYELLOW4 = float4(1.0f,1.0f,0.0f,1.0f);
static const float4 oMAGENTA4 = float4(1.0f,0.0f,1.0f,1.0f);
static const float4 oCYAN4 = float4(0.0f,1.0f,1.0f,1.0f);

static const float3 oCOLORS3[] = { oBLACK3, oBLUE3, oGREEN3, oCYAN3, oRED3, oMAGENTA3, oYELLOW3, oWHITE3, };
static const float4 oCOLORS4[] = { oBLACK4, oBLUE4, oGREEN4, oCYAN4, oRED4, oMAGENTA4, oYELLOW4, oWHITE4, };

// Three unit length vectors that approximate a hemisphere
// with a normal of float3( 0.0f, 0.0f, 1.0f )
static const float3 oHEMISPHERE3[3] = 
{
	float3( 0.0f,   oSQRT3Quarterf       , 0.5f ),
	float3(-0.75f, -oSQRT3Quarterf * 0.5f, 0.5f ),
	float3( 0.75f, -oSQRT3Quarterf * 0.5f, 0.5f ),
};

static const float3 oHEMISPHERE4[4] = 
{
	float3( 0.0f,				oSQRT3Quarterf, 0.5f ),
	float3( -oSQRT3Quarterf,	0.0f       ,	0.5f ),
	float3( 0.0f,				-oSQRT3Quarterf,0.5f ),
	float3( oSQRT3Quarterf,		0.0f       ,	0.5f ),
};

// _____________________________________________________________________________
// Utility code

// Due to lack of templates
#ifdef oHLSL
	#define oDEFINE_OSWAP(T) inline void oSwap(inout T A, inout T B) { T AOrig = A; A = B; B = AOrig; }
#else
	#define oDEFINE_OSWAP(T) inline void oSwap(T& A, T& B) { T AOrig = A; A = B; B = AOrig; }
#endif

oDEFINE_OSWAP(float);
oDEFINE_OSWAP(float2);
oDEFINE_OSWAP(float3);
oDEFINE_OSWAP(float4);

oDEFINE_OSWAP(int);
oDEFINE_OSWAP(int2);
oDEFINE_OSWAP(int3);
oDEFINE_OSWAP(int4);

// _____________________________________________________________________________
// Component ordering operators

inline float min(float2 a) { return min(a.x, a.y); }
inline float min(float3 a) { return min(min(a.x, a.y), a.z); }
inline float min(float4 a) { return min(min(min(a.x, a.y), a.z), a.w); }
inline int min(int2 a) { return min(a.x, a.y); }
inline int min(int3 a) { return min(min(a.x, a.y), a.z); }
inline int min(int4 a) { return min(min(min(a.x, a.y), a.z), a.w); }
inline uint min(uint2 a) { return min(a.x, a.y); }
inline uint min(uint3 a) { return min(min(a.x, a.y), a.z); }
inline uint min(uint4 a) { return min(min(min(a.x, a.y), a.z), a.w); }

// Returns the largest value in the specified vector
inline float max(float2 a) { return max(a.x, a.y); }
inline float max(float3 a) { return max(max(a.x, a.y), a.z); }
inline float max(float4 a) { return max(max(max(a.x, a.y), a.z), a.w); }
inline int max(int2 a) { return max(a.x, a.y); }
inline int max(int3 a) { return max(max(a.x, a.y), a.z); }
inline int max(int4 a) { return max(max(max(a.x, a.y), a.z), a.w); }
inline uint max(uint2 a) { return max(a.x, a.y); }
inline uint max(uint3 a) { return max(max(a.x, a.y), a.z); }
inline uint max(uint4 a) { return max(max(max(a.x, a.y), a.z), a.w); }

// _____________________________________________________________________________
// Geometry
inline float angle(const float3 a, const float3 b) { return acos(dot(a, b) / (length(a) * length(b))); }

// _____________________________________________________________________________
// Matrix code

// @oooii-kevin: FIXME, this is borrowed from bullet, promote this throughout the rest
// or our code
inline float3x3 oCreateRotationHLSL( float radians, const float3 unitVec)
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


inline float3x3 oCreateRotationHLSL(const float3 _NormalizedSrcVec, const float3 _NormalizedDstVec)
{
	float a = angle(_NormalizedSrcVec, _NormalizedDstVec);

	// Check for identity
	if (0.0f == a)
		return float3x3_identity;

	// Check for flip
	if(oPI == a)
		return float3x3 (
		float3(-1.0f, 0.0f, 0.0f),
		float3(0.0f, -1.0f, 0.0f),
		float3(0.0f, 0.0f, -1.0f));


	float3 NormalizedAxis = normalize(cross(_NormalizedSrcVec, _NormalizedDstVec));
	return oCreateRotationHLSL(a, NormalizedAxis);
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

inline o3DDARay oDeltaRayCreate(float3 _Ray)
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
	bool operator == (const o3DDARayContext& _rhs) const { return _rhs.CurrentPosition == CurrentPosition; }
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
inline float3 oBresenhamRay(float3 _Ray)
{
	float3 FAbsRay = abs(_Ray);
	return _Ray / max(FAbsRay);
}

// Because this uses and returns an int it guarantees the value generated
// is a voxel that would be generated by o3DDARayCast.  A BresenHam ray
// can then be used to jump through a volume and continue stepping with 
// 3DDA raycasts
inline o3DDARayContext o3DDARayCastBresenham(const o3DDARay _DeltaRay, const float3 _BresenhamRay, int _Delta)
{
	float3 FRay = floor(_BresenhamRay * (float)_Delta);
	o3DDARayContext DeltaContext;
	DeltaContext.CurrentPosition = FRay;
	DeltaContext.DeltaSum = (abs(FRay) + 1.0f) * _DeltaRay.Delta;
	return DeltaContext;
}

// Used to identify an axis throughout shader code
typedef int oAxis;
#define oAXIS_X 0
#define oAXIS_Y 1
#define oAXIS_Z 2

inline oAxis oFindDominantAxis(float3 _Ray)
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
// Morton number support

// We can store 2^10 values for each component of XYZ in a 32-bit Morton number.
// We assume that because the Morton number exists, it's a valid path into an
// Octree (if it was culled by the Octree's root bounds, the number would not be 
// up for consideration in whatever algorithm being implemented). Thus we can 
// store one more level than 2^10. Another way to look at this is that 2^11 is
// 1024 and there can be 1024 values in 10 bits.
static const uint  oMORTON_OCTREE_MAX_TARGET_DEPTH = 11;
static const int   oMORTON_OCTREE_SCALAR = (1 << (oMORTON_OCTREE_MAX_TARGET_DEPTH - 1)) - 1;
static const float oMORTON_OCTREE_SCALAR_FLOAT = (float)oMORTON_OCTREE_SCALAR;

// 32-bit Morton numbers can only use 30-bits for XYZ, 10 each channel, so 
// define a mask used to get rid of any non-Morton usage of the upper two bits.
#define oMORTON_MASK 0x3fffffff

inline uint spreadbits2(uint _x)
{
	/** <citation
		usage="Implementation" 
		reason="std libs don't have bit functions" 
		author="mweerden"
		description="http://stackoverflow.com/questions/1024754/how-to-compute-a-3d-morton-number-interleave-the-bits-of-3-ints?answertab=votes#tab-top"
		license="*** Assumed Public Domain ***"
		licenseurl="http://stackoverflow.com/questions/1024754/how-to-compute-a-3d-morton-number-interleave-the-bits-of-3-ints?answertab=votes#tab-top"
		modification="Isolated bit spreading"
	/>*/
	// $(CitedCodeBegin)
	/* Takes the input value and spreads
	   the bits out such that 2 0s are
	   between each of the original bits.
	   Since this is a uint the input number
	   must be 10-bit or less
	*/
	_x = (_x | (_x << 16)) & 0x030000FF;
	_x = (_x | (_x <<  8)) & 0x0300F00F;
	_x = (_x | (_x <<  4)) & 0x030C30C3;
	_x = (_x | (_x <<  2)) & 0x09249249;
	return _x;
	// $(CitedCodeEnd)
}

// Morton numbers
// http://en.wikipedia.org/wiki/Morton_number_(number_theory)
// Morton numbers take n-dimensional integer coordinates to a single dimension 
// by interleaving the bits of each coordinate. The resulting number then 
// contains a series of n-length bit sequences that describe progressive 
// navigation of quadtrees/octrees and so on.

inline uint oMorton3D(uint3 _Pos)
{
	return (spreadbits2(_Pos.x) << oAXIS_X) | (spreadbits2(_Pos.y) << oAXIS_Y) | (spreadbits2(_Pos.z) << oAXIS_Z);
}

//    +---------+ Returns [0,7] as to what local octant the specified point is 
//   / 1  / 0  /| in. Use oAXIS_X/oAXIS_Y/oAXIS_Z to check specific dimensions 
//  /----/----/ | axis.
//  +---+---+/|0|
//  | 5 | 4 | |/| _DepthIndex=0 for the root, which is not represented in the
//  +---+---+/|2| Morton number because it always evaluates to 0 (the root) or 
//  | 7 | 6 | |/  the number is not in the octree (outside the root bounds). So
//  +---+---+/    for the 1st subdivision (8 octants as pictured) where Depth=1,
//                This would get the highest 3 bits of the 30-bit Morton number.
inline uint oMorton3DToOctree(uint _Morton3D, uint _DepthIndex)
{
	return (_Morton3D >> (30 - _DepthIndex * 3)) & 0x7;
}

// Given a Morton number of arbitrary precision (i.e. one directly calculated
// from any (normalized) float position) truncate values beyond the _DepthIndex 
// such that values at a higher precision evaluate to the same value. Using 
// oMorton3DToOctree with values larger than the one specified here will yield
// invalid information.
inline uint oMortonQuantize(uint _Morton3D, uint _DepthIndex)
{
	// Provides 1 to _DepthIndex inclusively. See above docs describing that 0
	// is assumed.
	return oMORTON_MASK - ((1 << (3*(oMORTON_OCTREE_MAX_TARGET_DEPTH-1-_DepthIndex))) - 1);
}

inline uint oMorton3DEncodeNormalizedPosition(float3 _NormalizedPosition)
{
	// oMorton3D encodes the 3 integers at 10bit precision so we take the span of 
	// [0,1] to integer span [0,1023] (10-bit). This Morton encoded number can 
	// then be used for octree traversal at several levels.
	float3 FDiscretizedPos = oMORTON_OCTREE_SCALAR_FLOAT * _NormalizedPosition;
	uint3 DiscretizedPos = uint3((uint)FDiscretizedPos.x, (uint)FDiscretizedPos.y, (uint)FDiscretizedPos.z);
	return oMorton3D(DiscretizedPos);
}

inline float3 oMorton3DDecodeNormalizedPosition(uint _Morton3D, uint _TargetDepth = oMORTON_OCTREE_MAX_TARGET_DEPTH)
{
	float3 Position = 0.0f;
	float Offset = 0.5f;
	for(uint i = 1; i < _TargetDepth; ++i)
	{  
		uint OctreeSelect = oMorton3DToOctree(_Morton3D, i);
		Position.x += (OctreeSelect & (0x1 << oAXIS_X)) ? Offset : 0.0f;
		Position.y += (OctreeSelect & (0x1 << oAXIS_Y)) ? Offset : 0.0f;
		Position.z += (OctreeSelect & (0x1 << oAXIS_Z)) ? Offset : 0.0f;
		Offset *= 0.5f;
	}

	Position += Offset;

	return Position;
}

// _____________________________________________________________________________
// Random Numbers and Noise

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

/** <citation
	usage="Implementation" 
	reason="Because other noise functions have artifacts"
	author="Stefan Gustavson"
	description="http://www.google.com/url?sa=t&rct=j&q=&esrc=s&source=web&cd=1&cad=rja&ved=0CCIQFjAA&url=http%3A%2F%2Fwww.itn.liu.se%2F~stegu%2Fsimplexnoise%2Fsimplexnoise.pdf&ei=nzZRUMe-JKOtigLDpYBQ&usg=AFQjCNEVzOM03haFrTgLrjJp-jPkQyTOKA"
	license="*** Assumed Public Domain ***"
	licenseurl="http://www.google.com/url?sa=t&rct=j&q=&esrc=s&source=web&cd=1&cad=rja&ved=0CCIQFjAA&url=http%3A%2F%2Fwww.itn.liu.se%2F~stegu%2Fsimplexnoise%2Fsimplexnoise.pdf&ei=nzZRUMe-JKOtigLDpYBQ&usg=AFQjCNEVzOM03haFrTgLrjJp-jPkQyTOKA"
	modification="leverage shader per-component vector ops"
/>*/

static const float3 oSimplexNoise_grad3[12] =
{
	float3(1,1,0),float3(-1,1,0),float3(1,-1,0),float3(-1,-1,0),
	float3(1,0,1),float3(-1,0,1),float3(1,0,-1),float3(-1,0,-1),
	float3(0,1,1),float3(0,-1,1),float3(0,1,-1),float3(0,-1,-1)
};

static const int oSimplexNoise_PermTable[512] = {151,160,137,91,90,15,
	131,13,201,95,96,53,194,233,7,225,140,36,103,30,69,142,8,99,37,240,21,10,23,
	190, 6,148,247,120,234,75,0,26,197,62,94,252,219,203,117,35,11,32,57,177,33,
	88,237,149,56,87,174,20,125,136,171,168, 68,175,74,165,71,134,139,48,27,166,
	77,146,158,231,83,111,229,122,60,211,133,230,220,105,92,41,55,46,245,40,244,
	102,143,54, 65,25,63,161, 1,216,80,73,209,76,132,187,208, 89,18,169,200,196,
	135,130,116,188,159,86,164,100,109,198,173,186, 3,64,52,217,226,250,124,123,
	5,202,38,147,118,126,255,82,85,212,207,206,59,227,47,16,58,17,182,189,28,42,
	223,183,170,213,119,248,152, 2,44,154,163, 70,221,153,101,155,167, 43,172,9,
	129,22,39,253, 19,98,108,110,79,113,224,232,178,185, 112,104,218,246,97,228,
	251,34,242,193,238,210,144,12,191,179,162,241, 81,51,145,235,249,14,239,107,
	49,192,214, 31,181,199,106,157,184, 84,204,176,115,121,50,45,127, 4,150,254,
	138,236,205,93,222,114,67,29,24,72,243,141,128,195,78,66,215,61,156,180,

	151,160,137,91,90,15,
	131,13,201,95,96,53,194,233,7,225,140,36,103,30,69,142,8,99,37,240,21,10,23,
	190, 6,148,247,120,234,75,0,26,197,62,94,252,219,203,117,35,11,32,57,177,33,
	88,237,149,56,87,174,20,125,136,171,168, 68,175,74,165,71,134,139,48,27,166,
	77,146,158,231,83,111,229,122,60,211,133,230,220,105,92,41,55,46,245,40,244,
	102,143,54, 65,25,63,161, 1,216,80,73,209,76,132,187,208, 89,18,169,200,196,
	135,130,116,188,159,86,164,100,109,198,173,186, 3,64,52,217,226,250,124,123,
	5,202,38,147,118,126,255,82,85,212,207,206,59,227,47,16,58,17,182,189,28,42,
	223,183,170,213,119,248,152, 2,44,154,163, 70,221,153,101,155,167, 43,172,9,
	129,22,39,253, 19,98,108,110,79,113,224,232,178,185, 112,104,218,246,97,228,
	251,34,242,193,238,210,144,12,191,179,162,241, 81,51,145,235,249,14,239,107,
	49,192,214, 31,181,199,106,157,184, 84,204,176,115,121,50,45,127, 4,150,254,
	138,236,205,93,222,114,67,29,24,72,243,141,128,195,78,66,215,61,156,180
};

inline int oSimplexNoise_Perm(int i)
{
	return oSimplexNoise_PermTable[i & 255];
}

// Returns simplex noise (Ken Perlin's extended noise) for 3 dimensions on [-1,1]
inline float oSimplexNoise(float3 v)
{
	float n0, n1, n2, n3; // Noise contributions from the four corners
	// Skew the input space to determine which simplex cell we're in
	const float F3 = 1.0f/3.0f;
	float s = (v.x+v.y+v.z)*F3; // Very nice and simple skew factor for 3D
	int3 ijk = floor(v+s);
	const float G3 = 1.0f/6.0f; // Very nice and simple unskew factor, too
	float t = (ijk.x+ijk.y+ijk.z)*G3;
	float3 V0 = float3(float3(ijk)-t); // Unskew the cell origin back to (x,y,z) space
	float3 v0 = float3(v-V0); // The x,y,z distances from the cell origin
	// For the 3D case, the simplex shape is a slightly irregular tetrahedron.
	// Determine which simplex we are in.
	uint3 ijk1; // Offsets for second corner of simplex in (i,j,k) coords
	uint3 ijk2; // Offsets for third corner of simplex in (i,j,k) coords
	if (v0.x >= v0.y)
	{
		if (v0.y >= v0.z) { ijk1 = uint3(1,0,0); ijk2 = uint3(1,1,0); } // X Y Z order
		else if (v0.x >= v0.z) { ijk1 = uint3(1,0,0); ijk2 = uint3(1,0,1); } // X Z Y order
		else { ijk1 = uint3(0,0,1); ijk2 = uint3(1,0,1); } // Z X Y order
	}

	else // v0.x < v0.y
	{ 
		if (v0.y < v0.z) { ijk1 = uint3(0,0,1); ijk2 = uint3(0,1,1); } // Z Y X order
		else if (v0.x < v0.z) { ijk1 = uint3(0,1,0); ijk2 = uint3(0,1,1); } // Y Z X order
		else { ijk1 = uint3(0,1,0); ijk2 = uint3(1,1,0); } // Y X Z order
	}

	// A step of (1,0,0) in (i,j,k) means a step of (1-c,-c,-c) in (x,y,z),
	// a step of (0,1,0) in (i,j,k) means a step of (-c,1-c,-c) in (x,y,z), and
	// a step of (0,0,1) in (i,j,k) means a step of (-c,-c,1-c) in (x,y,z), where
	// c = 1/6.
	float3 v1 = v0 - float3(ijk1) + G3; // Offsets for second corner in (x,y,z) coords
	float3 v2 = v0 - float3(ijk2) + 2.0f*G3; // Offsets for third corner in (x,y,z) coords
	float3 v3 = v0 - 1.0f + 3.0f*G3; // Offsets for last corner in (x,y,z) coords

	// Work out the hashed gradient indices of the four simplex corners
	uint3 ijk255 = ijk & 255;

	const uint TWELVE = 12; // for uint to squelch as warning about signed modulus
	uint gi0 = oSimplexNoise_Perm(ijk255.x+oSimplexNoise_Perm(ijk255.y+oSimplexNoise_Perm(ijk255.z))) % TWELVE;
	uint gi1 = oSimplexNoise_Perm(ijk255.x+ijk1.x+oSimplexNoise_Perm(ijk255.y+ijk1.y+oSimplexNoise_Perm(ijk255.z+ijk1.z))) % TWELVE;
	uint gi2 = oSimplexNoise_Perm(ijk255.x+ijk2.x+oSimplexNoise_Perm(ijk255.y+ijk2.y+oSimplexNoise_Perm(ijk255.z+ijk2.z))) % TWELVE;
	uint gi3 = oSimplexNoise_Perm(ijk255.x+1+oSimplexNoise_Perm(ijk255.y+1+oSimplexNoise_Perm(ijk255.z+1))) % TWELVE;

	// Calculate the contribution from the four corners

	float t0 = 0.6f - dot(v0,v0);
	if(t0<0) n0 = 0.0f;
	else {
		t0 *= t0;
		n0 = t0 * t0 * dot(oSimplexNoise_grad3[gi0], v0);
	}
	float t1 = 0.6f - dot(v1,v1);
	if(t1<0) n1 = 0.0f;
	else {
		t1 *= t1;
		n1 = t1 * t1 * dot(oSimplexNoise_grad3[gi1], v1);
	}
	float t2 = 0.6f - dot(v2,v2);
	if(t2<0) n2 = 0.0f;
	else {
		t2 *= t2;
		n2 = t2 * t2 * dot(oSimplexNoise_grad3[gi2], v2);
	}
	float t3 = 0.6f - dot(v3,v3);
	if(t3<0) n3 = 0.0f;
	else {
		t3 *= t3;
		n3 = t3 * t3 * dot(oSimplexNoise_grad3[gi3], v3);
	}
	// Add contributions from each corner to get the final noise value.
	// The result is scaled to stay just inside [-1,1]
	return 32.0f*(n0 + n1 + n2 + n3);
}

// Fractal Brownian motion (layered Perlin noise)
inline float ofBm(float3 _Coord, uint _NumOctaves, float _Lacunarity, float _Gain)
{
	// based on:
	// http://www.scribd.com/doc/39470687/27/fBm-Shader-Code

	float Amp = 1;
	float AmpSum = 1;
	float3 C = _Coord;

	float n = oSimplexNoise(_Coord);
	for (uint i = 1; i < _NumOctaves; i += 1)
	{
		Amp *= _Gain;
		AmpSum += Amp;
		C *= _Lacunarity;
		n += Amp * oSimplexNoise(C);
	}

	return n / AmpSum;
}

// fBM, using abs() of noise() function.
inline float oTurbulence(float3 _Coord, uint _NumOctaves, float _Lacunarity, float _Gain)
{
	// based on:
	// http://www.scribd.com/doc/39470687/27/fBm-Shader-Code

	float Sum = 0;
	float Amp = 1;
	float AmpSum = 0;

	float3 C = _Coord;

	for (uint i = 0; i < _NumOctaves; i++)
	{
		Sum += Amp * (0.5f+0.5f*oSimplexNoise(C));
		AmpSum += Amp;
		Amp *= _Gain;
		C *= _Lacunarity;
	}

	return Sum / AmpSum;
}

// _____________________________________________________________________________
// Color space tranformation

// Converts a 3D normalized vector into an RGB color
// (typically for encoding a normal)
inline float3 oColorizeVector(float3 _NormalizedVector)
{
	return _NormalizedVector * float3(0.5f, 0.5f, -0.5f) + 0.5f;
}

// Converts a normalized vector stored as RGB color
// back to a vector
inline float3 oDecolorizeVector(float3 _RGBVector)
{
	return _RGBVector * float3(2.0f, 2.0f, -2.0f) - 1.0f;
}

// Convert from HSV (HSL) color space to RGB
inline float3 oHSVtoRGB(float3 HSV)
{
	// http://chilliant.blogspot.com/2010/11/rgbhsv-in-hlsl.html
	float R = abs(HSV.x * 6.0f - 3.0f) - 1.0f;
	float G = 2 - abs(HSV.x * 6.0f - 2.0f);
	float B = 2 - abs(HSV.x * 6.0f - 4.0f);
	return ((saturate(float3(R,G,B)) - 1.0f) * HSV.y + 1.0f) * HSV.z;
}

inline float oRGBtoLuminance(float3 color)
{
	// from http://en.wikipedia.org/wiki/Luminance_(relative)
	// "For RGB color spaces that use the ITU-R BT.709 primaries 
	// (or sRGB, which defines the same primaries), relative 
	// luminance can be calculated from linear RGB components:"
	color *= float3(0.2126f, 0.7152f, 0.0722f);
	return color.r + color.g + color.b;
}

inline float3 oYUVToRGB(float3 _YUV)
{
	// Using the float version of ITU-R BT.601 that jpeg uses. This is similar to 
	// the integer version, except this uses the full 0 - 255 range.
	static const float3 oITU_R_BT_601_Offset = float3(0.0f, -128.0f, -128.0f) / 255.0f;
	static const float3 oITU_R_BT_601_RFactor = float3(1.0f, 0.0f, 1.402f);
	static const float3 oITU_R_BT_601_GFactor = float3(1.0f, -0.34414f, -0.71414f);
	static const float3 oITU_R_BT_601_BFactor = float3(1.0f, 1.772f, 0.0f);
	_YUV += oITU_R_BT_601_Offset;
	return saturate(float3(dot(_YUV, oITU_R_BT_601_RFactor), dot(_YUV, oITU_R_BT_601_GFactor), dot(_YUV, oITU_R_BT_601_BFactor)));
}

// Converts a color value to a float4 WITHOUT NORMALIZING. This remains [0,255].
// This assumes ABGR ordering to map to float4's rgba mapping.
inline float4 oABGRToFloat4(uint _ABGR)
{
	return float4(
		float(_ABGR & 0xff),
		float((_ABGR >> 8u) & 0xff),
		float((_ABGR >> 16u) & 0xff),
		float((_ABGR >> 24u) & 0xff));
}

// Converts a float4 color value to uint ABGR WITHOUT NORMALIZING. This does not
// multiply into [0,255] because it is assumed to already have values on [0,255].
// This will map float4's rgba to ABGR.
inline uint oFloat4ToABGR(float4 _RGBA)
{
	return ((uint(_RGBA.a) & 0xff) << 24u)
		| ((uint(_RGBA.b) & 0xff) << 16u)
		| ((uint(_RGBA.g) & 0xff) << 8u)
		| (uint(_RGBA.r) & 0xff);
}

// Given an integer ID [0,255], return a color that ensures IDs near each other 
// (i.e. 13,14,15) have significantly different colors.
inline float3 oIDtoColor8Bit(uint ID8Bit)
{
	uint R = oRandUnmasked(ID8Bit);
	uint G = oRandUnmasked(R);
	uint B = oRandUnmasked(G);
	return float3(uint3(R,G,B) & 0xff) / 255.0f;
}

// Given an integer ID [0,65535], return a color that ensures IDs near each other 
// (i.e. 13,14,15) have significantly different colors.
inline float3 oIDtoColor16Bit(uint ID16Bit)
{
	uint R = oRandUnmasked(ID16Bit);
	uint G = oRandUnmasked(R);
	uint B = oRandUnmasked(G);
	return float3(uint3(R,G,B) & 0xffff) / 65535.0f;
}

// _____________________________________________________________________________
// Misc functions

// For voxel-based irradiance, the scene is positioned on [0,1]
inline bool oVoxelInBounds(float3 _Point)
{
	return _Point.x >= 0.0f && _Point.x <= 1.0f
		&& _Point.y >= 0.0f && _Point.y <= 1.0f
		&& _Point.z >= 0.0f && _Point.z <= 1.0f;
}

inline bool oIVoxelInBounds(int3 _Point)
{
	return _Point.x >= 0 && _Point.x <= oMORTON_OCTREE_SCALAR
		&& _Point.y >= 0 && _Point.y <= oMORTON_OCTREE_SCALAR
		&& _Point.z >= 0 && _Point.z <= oMORTON_OCTREE_SCALAR;
}

// Intersects the ray with the Voxel space which is normalized 0x0x0->1x1x1
inline float3 oVoxelSpaceFindFirstIntersection( float3 vRayO, float3 vRayDir )
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

#endif
