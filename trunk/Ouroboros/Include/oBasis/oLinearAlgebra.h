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
// This header contains utilities for working with 3x3 and 4x4 matrices and 
// quaternions.

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
// This math has been developed in a left-handed system, so there may be 
// assumption of left-handedness in some functions.

#pragma once
#ifndef oMatrix_h
#define oMatrix_h

#include <oHLSL/oHLSLTypes.h>
#include <oCompute/oQuaternion.h>
#include <oCompute/oComputeConstants.h>
#include <oStd/equal.h>
#include <limits>

// _____________________________________________________________________________
// Template identity values

template<typename T> T oIdentity() { return T(0); }

template<> inline float4x4 oIdentity() { return oIDENTITY4x4; }
template<> inline double4x4 oIdentity() { return oIDENTITY4x4D; }
template<> inline quatf oIdentity() { return oIDENTITYQ; }
template<> inline quatd oIdentity() { return oIDENTITYQD; }

// _____________________________________________________________________________
// oStd::equal support

template<> inline bool oStd::equal(const TVEC2<float>& a, const TVEC2<float>& b, unsigned int maxUlps) { return oStd::equal(a.x, b.x, maxUlps) && oStd::equal(a.y, b.y, maxUlps); }
template<> inline bool oStd::equal(const TVEC3<float>& a, const TVEC3<float>& b, unsigned int maxUlps) { return oStd::equal(a.x, b.x, maxUlps) && oStd::equal(a.y, b.y, maxUlps) && oStd::equal(a.z, b.z, maxUlps); }
template<> inline bool oStd::equal(const TVEC4<float>& a, const TVEC4<float>& b, unsigned int maxUlps) { return oStd::equal(a.x, b.x, maxUlps) && oStd::equal(a.y, b.y, maxUlps) && oStd::equal(a.z, b.z, maxUlps) && oStd::equal(a.w, b.w, maxUlps); }
template<> inline bool oStd::equal(const TMAT3<float>& a, const TMAT3<float>& b, unsigned int maxUlps) { return oStd::equal(a.Column0, b.Column0, maxUlps) && oStd::equal(a.Column1, b.Column1, maxUlps) && oStd::equal(a.Column2, b.Column2, maxUlps); }
template<> inline bool oStd::equal(const TMAT4<float>& a, const TMAT4<float>& b, unsigned int maxUlps) { return oStd::equal(a.Column0, b.Column0, maxUlps) && oStd::equal(a.Column1, b.Column1, maxUlps) && oStd::equal(a.Column2, b.Column2, maxUlps) && oStd::equal(a.Column3, b.Column3, maxUlps); }
template<> inline bool oStd::equal(const TVEC2<double>& a, const TVEC2<double>& b, unsigned int maxUlps) { return oStd::equal(a.x, b.x, maxUlps) && oStd::equal(a.y, b.y, maxUlps); }
template<> inline bool oStd::equal(const TVEC3<double>& a, const TVEC3<double>& b, unsigned int maxUlps) { return oStd::equal(a.x, b.x, maxUlps) && oStd::equal(a.y, b.y, maxUlps) && oStd::equal(a.z, b.z, maxUlps); }
template<> inline bool oStd::equal(const TVEC4<double>& a, const TVEC4<double>& b, unsigned int maxUlps) { return oStd::equal(a.x, b.x, maxUlps) && oStd::equal(a.y, b.y, maxUlps) && oStd::equal(a.z, b.z, maxUlps) && oStd::equal(a.w, b.w, maxUlps); }
template<> inline bool oStd::equal(const TMAT3<double>& a, const TMAT3<double>& b, unsigned int maxUlps) { return oStd::equal(a.Column0, b.Column0, maxUlps) && oStd::equal(a.Column1, b.Column1, maxUlps) && oStd::equal(a.Column2, b.Column2, maxUlps); }
template<> inline bool oStd::equal(const TMAT4<double>& a, const TMAT4<double>& b, unsigned int maxUlps) { return oStd::equal(a.Column0, b.Column0, maxUlps) && oStd::equal(a.Column1, b.Column1, maxUlps) && oStd::equal(a.Column2, b.Column2, maxUlps) && oStd::equal(a.Column3, b.Column3, maxUlps); }

// _____________________________________________________________________________
// General transformation functions

template<typename T> TMAT3<T> invert(const TMAT3<T>& _Matrix);
template<typename T> TMAT4<T> invert(const TMAT4<T>& _Matrix);

// _____________________________________________________________________________
// Non-view, non-perspective matrix composition functions

// Rotation is done around the Z axis, then Y, then X.
// Rotation is clockwise when the axis is a vector pointing at the viewer/
// observer. So to rotate a Y-up vector (0,1,0) to become a +X vector (pointing
// to the right in either left- or right-handed systems), you would create a 
// rotation axis of (0,0,-1) and rotate 90 degrees. To rotate (0,1,0) to become 
// -X, either change the rotation axis to be (0,0,1), OR negate the rotation of 
// 90 degrees.
template<typename T> TMAT4<T> oCreateRotation(const TVEC3<T>& _Radians);
template<typename T> TMAT4<T> oCreateRotation(const T& _Radians, const TVEC3<T>& _NormalizedRotationAxis);
template<typename T> TMAT4<T> oCreateRotation(const TVEC3<T>& _NormalizedSrcVec, const TVEC3<T>& _NormalizedDstVec);
template<typename T> TMAT4<T> oCreateRotation(const oCompute::quaternion<T>& _Quaternion);
template<typename T> oCompute::quaternion<T> oCreateRotationQ(const TVEC3<T>& _Radians);
template<typename T> oCompute::quaternion<T> oCreateRotationQ(T _Radians, const TVEC3<T>& _NormalizedRotationAxis);
// There are infinite/undefined solutions when the vectors point in opposite directions
template<typename T> oCompute::quaternion<T> oCreateRotationQ(const TVEC3<T>& _CurrentVector, const TVEC3<T>& _DesiredVector);
template<typename T> oCompute::quaternion<T> oCreateRotationQ(const TMAT4<T>& _Matrix);

template<typename T> TMAT4<T> oCreateTranslation(const TVEC3<T>& _Translation)
{
	return TMAT4<T>(
		TVEC4<T>(T(1), T(0), T(0), T(0)),
		TVEC4<T>(T(0), T(1), T(0), T(0)),
		TVEC4<T>(T(0), T(0), T(1), T(0)),
		TVEC4<T>(_Translation, T(1)));
}

template<typename T> TMAT4<T> oCreateScale(const TVEC3<T>& _Scale)
{
	return TMAT4<T>(
		TVEC4<T>(_Scale.x, T(0), T(0), T(0)),
		TVEC4<T>(T(0), _Scale.y, T(0), T(0)),
		TVEC4<T>(T(0), T(0), _Scale.z, T(0)),
		TVEC4<T>(T(0), T(0), T(0), T(1)));
}

template<typename T> TMAT4<T> oCreateScale(const T& _Scale) { return oCreateScale(TVEC3<T>(_Scale)); }
template<typename T> TMAT4<T> oCreateMatrix(const oCompute::quaternion<T>& _Rotation, const TVEC3<T>& _Translation) { return TMAT4<T>((TMAT3<T>)oCreateRotation(_Rotation), _Translation); }

template<typename T> TMAT4<T> oCreateReflection(const float4& _NormalizedReflectionPlane)
{
	const T A = _NormalizedReflectionPlane.x;
	const T B = _NormalizedReflectionPlane.y;
	const T C = _NormalizedReflectionPlane.z;
	const T D = _NormalizedReflectionPlane.w;
	return float4x4(
		TVEC4<T>(T(2) * A * A + T(1), T(2) * B * A,        T(2) * C * A, T(0)),
		TVEC4<T>(T(2) * A * B,        T(2) * B * B + T(1), T(2) * C * B, T(0)),
		TVEC4<T>(T(2) * A * C,        T(2) * B * C,        T(2) * C * C + T(1), T(0)),
		TVEC4<T>(T(2) * A * D,        T(2) * B * D,        T(2) * C * D, T(1)));
}

// creates a matrix that transforms points from ones that lie on the XY plane (+Z up)
// to points that lie on plane p
template<typename T> TMAT4<T> oCalcPlaneMatrix(const TVEC4<T>& _Plane)
{
	TMAT4<T> m = oCreateRotation(TVEC3<T>(T(0), T(0), T(1)), _Plane.xyz());

	// Since oCreateRotation takes no default axis it will flip the world
	// when the angle between the source and destination is 0. Since the desire is 
	// to force rotation around y, negate the y portion of the rotation when the 
	// plane's normal is TVEC3<T>(T(0), T(0), T(-1))
	if (_Plane.z == T(-1))
		m.Column1.y = -m.Column1.y;

	TVEC3<T> offset(T(0), T(0), _Plane.w);
	offset = ((TMAT3<T>)m) * offset;
	m.Column3.x = offset.x;
	m.Column3.y = offset.y;
	m.Column3.z = offset.z;
	return m;
}

// Compute the scale and translate matrix that converts the specified bound into
// a unit AABox with a min at (0,0,0) and max at (1,1,1).
template<typename T> TMAT4<T> oCreateNormalizationMatrix(const TVEC3<T>& _AABoxMin, const TVEC3<T>& _AABoxMax)
{
	const TVEC3<T> Scale = max(abs(_AABoxMax - _AABoxMin));
	// Scaling the bounds can introduce floating point precision issues near the 
	// bounds therefore add epsilon to the translation.
	const TVEC3<T> ScaledTranslation = (-_AABoxMin / Scale) + TVEC3<T>(std::numeric_limits<T>::epsilon());
	return mul(oCreateScale(rcp(Scale)), oCreateTranslation(ScaledTranslation));
}

// _____________________________________________________________________________
// Non-view, non-perspective matrix decomposition functions

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

// Output values are not normalized.
template<typename T> TVEC3<T> oExtractAxisX(const TMAT4<T>& _Matrix) { return _Matrix.Column0.xyz(); }
template<typename T> TVEC3<T> oExtractAxisY(const TMAT4<T>& _Matrix) { return _Matrix.Column1.xyz(); }
template<typename T> TVEC3<T> oExtractAxisZ(const TMAT4<T>& _Matrix) { return _Matrix.Column2.xyz(); }

template<typename T> TVEC3<T> oExtractAxisX(const TMAT3<T>& _Matrix) { return _Matrix.Column0; }
template<typename T> TVEC3<T> oExtractAxisY(const TMAT3<T>& _Matrix) { return _Matrix.Column1; }
template<typename T> TVEC3<T> oExtractAxisZ(const TMAT3<T>& _Matrix) { return _Matrix.Column2; }

// _____________________________________________________________________________
// View/Projection matrix composition functions

template<typename T> TMAT4<T> oCreateLookAtLH(const TVEC3<T>& _Eye, const TVEC3<T>& _At, const TVEC3<T>& _Up)
{
	TVEC3<T> z = normalize(_At - _Eye);
	TVEC3<T> x = normalize(cross(_Up, z));
	TVEC3<T> y = cross(z, x);
	return TMAT4<T>(
		TVEC4<T>(x.x,          y.x,            z.x,           T(0)),
		TVEC4<T>(x.y,          y.y,            z.y,           T(0)),
		TVEC4<T>(x.z,          y.z,            z.z,           T(0)),
		TVEC4<T>(-dot(x, _Eye), -dot(y, _Eye), -dot(z, _Eye), T(1)));
}

template<typename T> TMAT4<T> oCreateLookAtRH(const TVEC3<T>& _Eye, const TVEC3<T>& _At, const TVEC3<T>& _Up)
{
	TVEC3<T> z = normalize(_Eye - _At);
	TVEC3<T> x = normalize(cross(_Up, z));
	TVEC3<T> y = cross(z, x);
	return TMAT4<T>(
		TVEC4<T>(x.x,          y.x,            z.x,           T(0)),
		TVEC4<T>(x.y,          y.y,            z.y,           T(0)),
		TVEC4<T>(x.z,          y.z,            z.z,           T(0)),
		TVEC4<T>(-dot(x, _Eye), -dot(y, _Eye), -dot(z, _Eye), T(1)));
}

template<typename T> TMAT4<T> oCreateOrthographicLH(T _Left, T _Right, T _Bottom, T _Top, T _ZNear = oDEFAULT_NEAR, T _ZFar = oDEFAULT_FAR)
{
	return TMAT4<T>(
		TVEC4<T>(T(2)/(_Right-_Left),           T(0),                          T(0),                  T(0)),
		TVEC4<T>(T(0),                          T(2)/(_Top-_Bottom),           T(0),                  T(0)),
		TVEC4<T>(T(0),                          T(0),                          T(1)/(_ZFar-_ZNear),   T(0)),
		TVEC4<T>((_Left+_Right)/(_Left-_Right), (_Top+_Bottom)/(_Bottom-_Top), _ZNear/(_ZNear-_ZFar), T(1)));
}

template<typename T> TMAT4<T> oCreateOrthographicRH(T _Left, T _Right, T _Bottom, T _Top, T _ZNear = oDEFAULT_NEAR, T _ZFar = oDEFAULT_FAR)
{
	return TMAT4<T>(
		TVEC4<T>(T(2)/(_Right-_Left),           T(0),                          T(0),                  T(0)),
		TVEC4<T>(T(0),                          T(2)/(_Top-_Bottom),           T(0),                  T(0)),
		TVEC4<T>(T(0),                          T(0),                          T(1)/(_ZNear-_ZFar),   T(0)),
		TVEC4<T>((_Left+_Right)/(_Left-_Right), (_Top+_Bottom)/(_Bottom-_Top), _ZNear/(_ZNear-_ZFar), T(1)));
}

// Creates a perspective projection matrix. If _ZFar is less than 0, then an 
// infinitely far plane is used. Remember precision issues with regard to near
// and far plane. Because precision on most hardware is distributed 
// logarithmically so that more precision is near to the view, you can gain more
// precision across the entire scene by moving out the near plane slightly 
// relative to bringing in the far plane by a lot.
template<typename T> TMAT4<T> oCreatePerspectiveLH(T _FovYRadians, T _AspectRatio, T _ZNear = oDEFAULT_NEAR, T _ZFar = oDEFAULT_FAR);
template<typename T> TMAT4<T> oCreatePerspectiveRH(T _FovYRadians, T _AspectRatio, T _ZNear = oDEFAULT_NEAR, T _ZFar = oDEFAULT_FAR);

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
template<typename T> TMAT4<T> oCreateOffCenterPerspectiveLH(const TMAT4<T>& _OutputTransform, const TVEC3<T>& _EyePosition, T _ZNear = oDEFAULT_NEAR);

// Given some information about the render target in NDC space, create a scale
// and bias transform to create a viewport. Pre-multiply this by the projection
// matrix to get a sub-projection matrix just for the specified viewport.
template<typename T> TMAT4<T> oCreateViewport(T _NDCResolutionX, T _NDCResolutionY, T _NDCRectLeft, T _NDCRectBottom, T _NDCRectWidth, T _NDCRectHeight);

// Converts a right-handed view matrix to left-handed, and vice-versa
// http://msdn.microsoft.com/en-us/library/bb204853(v=vs.85).aspx
template<typename T> TMAT4<T> oFlipViewHandedness(const TMAT4<T>& _View) { TMAT4<T> m = _View; m.Column2 = -m.Column2; return m; }

// Returns a view matrix that fits the bounding sphere of the specified bounds
// to the specified projection. This preserves the rotation of the view, so this
// will only update the position of the view to be far enough from the center of 
// the bounds so it all fits on screen.
template<typename T> TMAT4<T> oFitToView(const TMAT4<T>& _View, T _FovYRadians, const TVEC3<T>& _BoundCenter, T _BoundRadius, T _OffsetMultiplier = T(1))
{
	TMAT4<T> invView = invert(_View);
	T Offset = (_OffsetMultiplier * _BoundRadius) / tan(_FovYRadians / T(2));
	TVEC3<T> P = _BoundCenter - oExtractAxisZ(invView) * Offset;
	invView.Column3 = TVEC4<T>(P, invView.Column3.w);
	return invert(invView);
}

template<typename T> TMAT4<T> oFitToView(const TMAT4<T>& _View, const TMAT4<T>& _Projection, const TVEC3<T>& _BoundCenter, T _BoundRadius, T _OffsetMultiplier = T(1))
{
	T FovYRadians, AspectRatio, ZNear, ZFar;
	oExtractPerspectiveParameters(_Projection, &FovYRadians, &AspectRatio, &ZNear, &ZFar);
	return oFitToView(_View, FovYRadians, _BoundCenter, _BoundRadius, _OffsetMultiplier);
}

// _____________________________________________________________________________
// View/Projection matrix decomposition functions

// Get the original axis values from a view matrix
template<typename T> void oExtractLookAt(const TMAT4<T>& _View, TVEC3<T>* _pEye, TVEC3<T>* _pAt, TVEC3<T>* _pUp, TVEC3<T>* _pRight)
{
	*_pEye = invert(_View).Column3.xyz();
	_pRight->x = _View[0][0]; _pUp->x = _View[0][1]; _pAt->x = -_View[0][2];
	_pRight->y = _View[1][0]; _pUp->y = _View[1][1]; _pAt->y = -_View[1][2];
	_pRight->z = _View[2][0]; _pUp->z = _View[2][1]; _pAt->z = -_View[2][2];
}

// Get the world space eye position from a view matrix
template<typename T> TVEC3<T> oExtractEye(const TMAT4<T>& _View) { return invert(_View).Column3.xyz(); }

template<typename T> void oExtractNearFarPlanesDistance(const TMAT4<T>& _Projection, T* _pNearDistance, T* _pFarDistance)
{
	T N, F;
	T C = _Projection[2][2];
	T D = _Projection[3][2];

	if (oStd::equal(C, T(0)))
		N = T(0);
	else
		N = (-D / C);

	if (oHasPerspective(_Projection))
	{
		if (oStd::equal(C, T(1)) || oStd::equal(D, T(0)))
			F = T(1) / abs(D);
		else
			F = D / (T(1) - C);
	}
	else
		F = (T(1) + C * N) / C;

	*_pNearDistance = N;
	*_pFarDistance = F;
}

// Extracts the parameters used in oCreatePerspective() or oCreateOrthographic()
// to define the specified projection.
template<typename T> void oExtractPerspectiveParameters(const TMAT4<T>& _Projection, T* _pFovYRadians, T* _pAspectRatio, T* _pZNear, T* _pZFar)
{
	*_pFovYRadians = atan(T(1) / _Projection.Column1.y) * T(2);
	*_pAspectRatio = _Projection.Column1.y / _Projection.Column0.x;
	oCalcNearFarPlanesDistance(_Projection, _pZNear, _pZFar);
}

#endif
