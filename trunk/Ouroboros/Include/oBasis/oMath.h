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

// OOOii Math Library. This math library attempts to conform to HLSL (SM5) as 
// closely as possible. To reduce typing, templates and macros are used 
// extensively, but the code should still be simpler than the alternative.
// There are additional APIs as well that extend beyond HLSL as well, but try to 
// keep the spirit of hlsl in mind.
//
//
// === PLANES ===
//
// The plane equation used here is Ax + By + Cz + D = 0, so see sdistance() as 
// to the implications. Primarily it means that positive D values are in the 
// direction/on the side of the normal, and negative values are in the opposite
// direction/on the opposite side of the normal. The best example of seeing this 
// is to use oCreateOrthographic to create a -1,1,-1,1,0,1 projection (unit/clip 
// space) and then convert that to a frustum with oCalcFrustumPlanes(). You'll 
// see that the left clip plane is 1,0,0,-1 meaning the normal points inward to 
// the right and the offset is away from that normal to the left/-1 side. 
// Likewise the right clip plane is -1,0,0,-1 meaning the normal points inward 
// to the left, and the offset is once again away from that normal to the 
// right/+1 side.
//
//
// === MATRICES ===
// 
// Matrices are column-major - don't be fooled because matrices are a list of 
// column vectors, so in the debugger it seems to be row-major, but it is 
// column-major.
//
// Matrix multiplication is done in-order from left to right. This means if you 
// want to assemble a typical transform in SRT order it would look like this: 
// float4x4 transform = ScaleMatrix * RotationMatrix * TransformMatrix
// OR
// float4x4 WorldViewProjection = WorldMatrix * ViewMatrix * ProjectionMatrix
//
// All multiplication against vectors occur in matrix * vector order. 
// Unimplemented operator* for vector * matrix enforce this.
//
// Both left-handed and right-handed functions are provided where appropriate.

#pragma once
#ifndef oMath_h
#define oMath_h

#include <Math.h>
#include <float.h>
#include <oBasis/oAssert.h>
#include <oBasis/oBit.h>
#include <oBasis/oEqual.h>
#include <oBasis/oLimits.h>
#include <oBasis/oMathTypes.h>
#include <oBasis/oMathInternalHLSL.h>

// _____________________________________________________________________________
// Common math constants

#define oPI (3.14159265358979323846)
#define oPIf (float(oPI))
#define oE (2.71828183)
#define oEf (float(oE))

#define oDEFAULT_NEAR (10.0f)
#define oDEFAULT_FAR (10000.0f)

// NOTE: Epsilon for small float values is not defined. See documentation of ulps
// below.

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
// Geometry

template<typename T> inline T fresnel(const TVEC3<T>& i, const TVEC3<T>& n) { return 0.02f+0.97f*pow((1-oMax(dot(i, n))),5); } // http://habibs.wordpress.com/alternative-solutions/
template<typename T> inline T angle(const TVEC3<T>& a, const TVEC3<T>& b) { return acos(dot(a, b) / (length(a) * length(b))); }
template<typename T> inline TVEC4<T> oNormalizePlane(const TVEC4<T>& _Plane) { T invLength = rsqrt(dot(_Plane.xyz(), _Plane.xyz())); return _Plane * invLength; }

// Signed distance from a plane in Ax + By + Cz + Dw = 0 format (ABC = normalized normal, D = offset)
// This assumes the plane is normalized.
// >0 means on the same side as the normal
// <0 means on the opposite side as the normal
// 0 means on the plane
template<typename T> inline T sdistance(const TVEC4<T>& _Plane, const TVEC3<T>& _Point) { return dot(_Plane.xyz(), _Point) + _Plane.w; }
template<typename T> inline T distance(const TVEC4<T>& _Plane, const TVEC3<T>& _Point) { return abs(sdistance(_Plane, _Point));  }

// _____________________________________________________________________________
// Quaternion functions

template<typename T> TQUAT<T> mul(const TQUAT<T>& a, const TQUAT<T>& b) { return TQUAT<T>(((((a.w * b.x) + (a.x * b.w)) + (a.y * b.z)) - (a.z * b.y)), ((((a.w * b.y) + (a.y * b.w)) + (a.z * b.x)) - (a.x * b.z)), ((((a.w * b.z) + (a.z * b.w)) + (a.x * b.y)) - (a.y * b.x)), ((((a.w * b.w) - (a.x * b.x)) - (a.y * b.y)) - (a.z * b.z))); }
template<typename T> TVEC3<T> mul(const TQUAT<T>& q, const TVEC3<T>& v) { return v + T(2.0) * cross(q.xyz(), cross(q.xyz(), v) + q.w * v); } // http://code.google.com/p/kri/wiki/Quaternions
template<typename T> TQUAT<T> normalize(const TQUAT<T>& a) { TVEC4<T> v = normalize(a.xyzw()); return *(TQUAT<T>*)&v; }
template<typename T> TQUAT<T> conjugate(const TQUAT<T>& a) { return TQUAT<T>(-a.x, -a.y, -a.z, a.w); }
template<typename T> TQUAT<T> invert(const TQUAT<T>& a) { TVEC4<T> v = conjugate(a).xyzw() * (1 / dot(a.xyzw(), a.xyzw())); return *(TQUAT<T>*)&v; }
template<typename T> TQUAT<T> slerp(const TQUAT<T>& a, const TQUAT<T>& b, T s);

// _____________________________________________________________________________
// Matrix functions

template<typename T> TMAT3<T> invert(const TMAT3<T>& _Matrix);
template<typename T> TMAT4<T> invert(const TMAT4<T>& _Matrix);

// Decomposes the specified matrix into its components as if they were applied 
// in the following order:
// ScaleXYZ ShearXY ShearXY ShearZY RotationXYZ TranslationXYZ ProjectionXYZW
// Returns true if successful, or false if the specified matrix is singular.
// NOTE: This does not support negative scale well. Rotations might appear 
// rotated by 180 degrees, and the resulting scale can be the wrong sign.
template<typename T> bool oDecompose(const TMAT4<T>& _Matrix, TVEC3<T>* _pScale, T* _pShearXY, T* _pShearXZ, T* _pShearZY, TVEC3<T>* _pRotation, TVEC3<T>* _pTranslation, TVEC4<T>* _pPerspective);

// Extract components from a matrix. Currently this uses decompose and just hides
// a bunch of the extra typing that's required.
template<typename T> TVEC3<T> oExtractScale(const TMAT4<T>& _Matrix) { T xy, xz, zy; TVEC3<T> s, r, t; TVEC4<T> p; oDecompose(_Matrix, &s, &xy, &xz, &zy, &r, &t, &p); return s; }
template<typename T> TVEC3<T> oExtractRotation(const TMAT4<T>& _Matrix) { T xy, xz, zy; TVEC3<T> s, r, t; TVEC4<T> p; oDecompose(_Matrix, &s, &xy, &xz, &zy, &r, &t, &p); return r; }
template<typename T> TVEC3<T> oExtractTranslation(const TMAT4<T>& _Matrix) { return _Matrix.Column3.xyz(); }

template<typename T> void oExtractAxes(const TMAT4<T>& _Matrix, TVEC3<T>* _pXAxis, TVEC3<T>* _pYAxis, TVEC3<T>* _pZAxis);

// returns true of there is a projection/perspective or false if orthographic
template<typename T> bool oHasPerspective(const TMAT4<T>& _Matrix);

// _____________________________________________________________________________
// Volume functions

template<typename T> void oExtractAABoxCorners(TVEC3<T> _Corners[8], const TAABOX<T, TVEC3<T> >& _AABox)
{
	const TVEC3<T>& Min = _AABox.GetMin();
	const TVEC3<T>& Max = _AABox.GetMax();
	_Corners[0] = Min;
	_Corners[1] = TVEC3<T>(Max.x, Min.y, Min.z);
	_Corners[2] = TVEC3<T>(Min.x, Max.y, Min.z);
	_Corners[3] = TVEC3<T>(Max.x, Max.y, Min.z);
	_Corners[4] = TVEC3<T>(Min.x, Min.y, Max.z);
	_Corners[5] = TVEC3<T>(Max.x, Min.y, Max.z);
	_Corners[6] = TVEC3<T>(Min.x, Max.y, Max.z);
	_Corners[7] = Max;
}

template<typename T, typename TVec> void oExtendBy(TAABOX<T, TVec>& _AABox, const TVec& _Point) { _AABox.SetMin(oMin(_AABox.GetMin(), _Point)); _AABox.SetMax(oMax(_AABox.GetMax(), _Point)); }
template<typename T, typename TVec> void oExtendBy(TAABOX<T, TVec>& _AABox1, const TAABOX<T, TVec>& _AABox2) { oExtendBy(_AABox1, _AABox2.GetMin()); oExtendBy(_AABox1, _AABox2.GetMax()); }
template<typename T, typename TVec> void oTranslate(TAABOX<T, TVec>& _AABox, const TVec& _Translation) { _AABox.SetMin(_AABox.GetMin() + _Translation); _AABox.SetMax(_AABox.GetMax() + _Translation); }

// _____________________________________________________________________________
// NON-HLSL Matrix creation

// Rotation is done around the Z axis, then Y, then X.
// Rotation is clockwise when the axis is a vector pointing at the viewer/
// observer. So to rotate a Y-up vector (0,1,0) to become a +X vector (pointing
// to the right in either left- or right-handed systems), you would create a 
// rotation axis of (0,0,-1) and rotate 90 degrees. To rotate (0,1,0) to become 
// -X, either change the rotation axis to be (0,0,1), OR negate the rotation of 
// 90 degrees.
template<typename T> TMAT4<T> oCreateRotation(const TVEC3<T>& _Radians);
template<typename T> TMAT4<T> oCreateRotation(const T& _Radians, const TVEC3<T>& _NormalizedRotationAxis);
template<typename T> TMAT4<T> oCreateRotation(const TVEC3<T>& _CurrentVector, const TVEC3<T>& _DesiredVector, const TVEC3<T>& _DefaultRotationAxis);
template<typename T> TMAT4<T> oCreateRotation(const TQUAT<T>& _Quaternion);
template<typename T> TQUAT<T> oCreateRotationQ(const TVEC3<T>& _Radians);
template<typename T> TQUAT<T> oCreateRotationQ(T _Radians, const TVEC3<T>& _NormalizedRotationAxis);
// There are infinite/undefined solutions when the vectors point in opposite directions
template<typename T> TQUAT<T> oCreateRotationQ(const TVEC3<T>& _CurrentVector, const TVEC3<T>& _DesiredVector);
template<typename T> TQUAT<T> oCreateRotationQ(const TMAT4<T>& _Matrix);
template<typename T> TMAT4<T> oCreateTranslation(const TVEC3<T>& _Translation);
template<typename T> TMAT4<T> oCreateScale(const TVEC3<T>& _Scale);
template<typename T> TMAT4<T> oCreateScale(const T& _Scale) { return oCreateScale(TVEC3<T>(_Scale)); }

template<typename T> TMAT4<T> oCreateLookAtLH(const TVEC3<T>& _Eye, const TVEC3<T>& _At, const TVEC3<T>& _Up);
template<typename T> TMAT4<T> oCreateLookAtRH(const TVEC3<T>& _Eye, const TVEC3<T>& _At, const TVEC3<T>& _Up);

// Creates a perspective projection matrix. If _ZFar is less than 0, then an 
// infinitely far plane is used. Remember precision issues with regard to near
// and far plane. Because precision on most hardware is distributed 
// logarithmically so that more precision is near to the view, you can gain more
// precision across the entire scene by moving out the near plane slightly 
// relative to bringing in the far plane by a lot.
template<typename T> TMAT4<T> oCreatePerspectiveLH(T _FovYRadians, T _AspectRatio, T _ZNear = oDEFAULT_NEAR, T _ZFar = oDEFAULT_FAR);
template<typename T> TMAT4<T> oCreatePerspectiveRH(T _FovYRadians, T _AspectRatio, T _ZNear = oDEFAULT_NEAR, T _ZFar = oDEFAULT_FAR);

template<typename T> TMAT4<T> oCreateOrthographicLH(T _Left, T _Right, T _Bottom, T _Top, T _ZNear = oDEFAULT_NEAR, T _ZFar = oDEFAULT_FAR);
template<typename T> TMAT4<T> oCreateOrthographicRH(T _Left, T _Right, T _Bottom, T _Top, T _ZNear = oDEFAULT_NEAR, T _ZFar = oDEFAULT_FAR);

// Given the corners of the display medium (relative to the origin where the eye
// is) create an off-axis (off-center) projection matrix.
template<typename T> TMAT4<T> oCreateOffCenterPerspectiveLH(T _Left, T _Right, T _Bottom, T _ZNear = oDEFAULT_NEAR, T _ZFar = oDEFAULT_FAR);

// Create an off-axis (off-center) projection matrix properly sized to the 
// specified output medium (i.e. the screen if it were transformed in world 
// space)
// _OutputTransform: Scale, Rotation, Translation (SRT) of the output device 
// plane (i.e. the screen) in the same space as the eye (usually world space)
// _EyePosition: the location of the eye in the same space as _OutputTransform
// _ZNear: A near plane that is different than the plane of the output 
// (physical glass of the screen). By specifying a near plane that is closer to
// the eye than the glass plane, an effect of "popping out" 3D can be achieved.
template<typename T> TMAT4<T> oCreateOffCenterPerspectiveLH(const TMAT4<T>& _OutputTransform, const TVEC3<T>& _EyePosition, T _ZNear = 10.0f);

// Given some information about the render target in NDC space, create a scale
// and bias transform to create a viewport. Pre-multiply this by the projection
// matrix to get a sub-projection matrix just for the specified viewport.
template<typename T> TMAT4<T> oCreateViewport(T _NDCResolutionX, T _NDCResolutionY, T _NDCRectLeft, T _NDCRectBottom, T _NDCRectWidth, T _NDCRectHeight);

float4x4 oAsReflection(const float4& _ReflectionPlane);

// creates a matrix that transforms points from ones that lie on the XY plane (+Z up)
// to points that lie on plane p
void oCalcPlaneMatrix(const float4& _Plane, float4x4* _pMatrix);

// _____________________________________________________________________________
// NON-HLSL View and Projection utility functions

// Converts a right-handed view matrix to left-handed, and vice-versa
// http://msdn.microsoft.com/en-us/library/bb204853(v=vs.85).aspx
inline float4x4 oFlipViewHandedness(const float4x4& _View) { float4x4 m = _View; m.Column2 = -m.Column2; return m; }

// Get the original axis values from a view matrix
void oExtractLookAt(const float4x4& _View, float3* _pEye, float3* _pAt, float3* _pUp, float3* _pRight);

// Get the world space eye position from a view matrix
template<typename T> TVEC3<T> oExtractEye(const TMAT4<T>& _View) { return invert(_View).Column3.xyz(); }

// Fills the specified array with planes that point inward in the following 
// order: left, right, top, bottom, near, far. The planes will be in whatever 
// space the matrix was in minus one. This means in the space conversion of:
// Model -> World -> View -> Projection that a projection matrix returned by
// oCreatePerspective?H() will be in view space. A View * Projection will be
// in world space, and a WVP matrix will be in model space.
template<typename T> void oExtractFrustumPlanes(TVEC4<T> _Planes[6], const TMAT4<T>& _Projection, bool _Normalize);

// Fills the specified array with points that represent the 8 corners of the
// frustum. Index into the array using TFRUSTUM::CORNER. Returns true if values 
// are valid or false if planes don't meet in 8 corners.
template<typename T> bool oExtractFrustumCorners(TVEC3<T> _Corners[8], const TFRUSTUM<T>& _Frustum);

// Returns the near plane and 1/farplane to the far plane of the visible frustum.
template<typename T> void oCalculateNearInverseFarPlanesDistance(const TMAT4<T>& _Projection, T* _pNearDistance, T* _pInverseFarDistance);

// Extracts the parameters used in oCreatePerspective() or oCreateOrthographic()
// to define the specified projection.
template<typename T> void oExtractPerspectiveParameters(const TMAT4<T>& _Projection, T* _pFovYRadians, T* _pAspectRatio, T* _pZNear, T* _pZFar);

// Returns a view matrix that fits the bounding sphere of the specified bounds
// to the specified projection. This preserves the rotation of the view, so this
// will only update the position of the view to be far enough from the center of 
// the bounds so it all fits on screen.
template<typename T> TMAT4<T> oFitToView(const TMAT4<T>& _View, const TMAT4<T>& _Projection, const TSPHERE<T>& _Bounds, T _OffsetMultiplier = T(1.0));
template<typename T> TMAT4<T> oFitToView(const TMAT4<T>& _View, const TMAT4<T>& _Projection, const TAABOX<T, TVEC3<T>>& _Bounds, T _OffsetMultiplier = T(1.0)) { return oFitToView(_View, _Projection, TSPHERE<T>(_Bounds.GetCenter(), _Bounds.GetBoundingRadius()), _OffsetMultiplier); }

// _____________________________________________________________________________
// Containment/intersection/base collision

template<typename T> TAABOX<T, TVEC3<T>> oGetBoundingAABox(const TSPHERE<T>& _Sphere) { return TAABOX<T, TVEC3<T>>(TVEC3<T>(_Sphere.GetPosition() - _Sphere.GetRadius()), TVEC3<T>(_Sphere.GetPosition() + _Sphere.GetRadius())); }

inline bool oContains(const oRECT& _rect, int2 _point) 
{ 
  return _point.x >= _rect.GetMin().x && _point.x <= _rect.GetMax().x && _point.y >= _rect.GetMin().y && _point.y <= _rect.GetMax().y;
}

enum oCONTAINMENT
{
	oNOT_CONTAINED,
	oPARTIALLY_CONTAINED,
	oWHOLLY_CONTAINED,
};

template<typename T> oCONTAINMENT oContains(const TFRUSTUM<T>& _Frustum, const TAABOX<T,TVEC3<T>>& _Box);
template<typename T> oCONTAINMENT oContains(const TFRUSTUM<T>& _Frustum, const TVEC4<T>& _Sphere);
template<typename T> oCONTAINMENT oContains(const TSPHERE<T>& _Sphere, const TAABOX<T,TVEC3<T>>& _Box);
template<typename T, typename TVec> oCONTAINMENT oContains(const TAABOX<T, TVec>& _Box0, const TAABOX<T, TVec>& _Box1);
template<typename T> oCONTAINMENT oContains(float3 _Point, const TAABOX<T,TVEC3<T>>& _Box);

template<typename T> bool oIntersects(const TFRUSTUM<T>& _Frustum, const TAABOX<T,TVEC3<T>>& _Box) { return oNOT_CONTAINED != oContains(_Frustum, _Box); }
template<typename T> bool oIntersects(const TFRUSTUM<T>& _Frustum, const TVEC4<T>& _Sphere) 
{ 
  return oNOT_CONTAINED != oContains(_Frustum, _Sphere); 
}
template<typename T> bool oIntersects(const TSPHERE<T>& _Sphere, const TAABOX<T,TVEC3<T>>& _Box) { return oNOT_CONTAINED != oContains(_Sphere, _Box); }
template<typename T, typename TVec> bool oIntersects(const TAABOX<T, TVec>& _Box0, const TAABOX<T, TVec>& _Box1) { return oNOT_CONTAINED != oContains(_Box0, _Box1); }

bool oIntersects(float3* _pIntersection, const float4& _Plane0, const float4& _Plane1, const float4& _Plane2);

// Calculate the point on this plane where the line segment
// described by p0 and p1 intersects.
bool oIntersects(float3* _pIntersection, const float4& _Plane, const float3& _Point0, const float3& _Point1);

// @oooii-tony: This is only implemented and used for 2D rectangles, so move
// this inside oMath.cpp so that it's more hidden that it's not fully implemented.
// Returns a rectangle that is the clipped overlap of this with other
template<typename T, typename TVec> TAABOX<T, TVec> oClip(const TAABOX<T, TVec>& _BoxA, const TAABOX<T, TVec>& _BoxB)
{
	TAABOX<T, TVec> ClippedRect;
	T ClipTop;
	T ClipLeft;
	T ClipWidth;
	T ClipHeight;
	{
		bool BIsLeft = _BoxB.GetMin().x < _BoxA.GetMin().x;
		const TAABOX<T, TVec>& LRect = BIsLeft ? _BoxB : _BoxA;
		const TAABOX<T, TVec>& RRect = BIsLeft ? _BoxA : _BoxB;

		ClipLeft = __max( LRect.GetMin().x, RRect.GetMin().x );
		ClipWidth = __min( LRect.GetMax().x - ClipLeft, RRect.GetMax().x - ClipLeft );
		if( ClipWidth < 0 )
			return ClippedRect;
	}
	{
		bool BIsTop = _BoxB.GetMin().y < _BoxA.GetMin().y;
		const TAABOX<T, TVec>& TRect = BIsTop ? _BoxB : _BoxA;
		const TAABOX<T, TVec>& BRect = BIsTop ? _BoxA : _BoxB;

		ClipTop = __max(TRect.GetMin().y, BRect.GetMin().y);
		ClipHeight = __min(TRect.GetMax().y - ClipTop, BRect.GetMax().y - ClipTop);
		if( ClipHeight < 0 )
			return ClippedRect;
	}

	ClippedRect.SetMin(TVec(ClipLeft, ClipTop));
	ClippedRect.SetMax(TVec(ClipLeft + ClipWidth, ClipTop + ClipHeight));

	return ClippedRect;
}

// _____________________________________________________________________________
// NON-HLSL Miscellaneous

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

#endif
