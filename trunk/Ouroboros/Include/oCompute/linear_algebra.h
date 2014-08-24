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
// This header contains utilities for working with 3x3 and 4x4 matrices and 
// quaternions.
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
// Unimplemented operator* for vector * matrix enforces this.
//
// Both left-handed and right-handed functions are provided where appropriate.
// This math has been developed in a left-handed system, so there may be 
// assumption of left-handedness in some functions.

#ifndef ouro_linear_algebra_h
#define ouro_linear_algebra_h

#ifdef oHLSL
#error this header cannot compile in HLSL
#endif

#include <oCompute/oComputeConstants.h>
#include <oCompute/oComputeUtil.h>
#include <oMemory/equal.h>
#include <oBase/types.h>

namespace ouro {

// _____________________________________________________________________________
// General transformation functions

float4x4 invert(const float4x4& _Matrix);
float3x3 invert(const float3x3& _Matrix);

// _____________________________________________________________________________
// Non-view, non-perspective matrix composition functions

// Rotation is done around the Z axis, then Y, then X.
// Rotation is clockwise when the axis is a vector pointing at the viewer/
// observer. So to rotate a Y-up vector (0,1,0) to become a +X vector (pointing
// to the right in either left- or right-handed systems), you would create a 
// rotation axis of (0,0,-1) and rotate 90 degrees. To rotate (0,1,0) to become 
// -X, either change the rotation axis to be (0,0,1), OR negate the rotation of 
// 90 degrees.

float4x4 make_rotation(const float3& _Radians);
float4x4 make_rotation(float _Radians, const float3& _NormalizedRotationAxis);
float4x4 make_rotation(const float3& _NormalizedFrom, const float3& _NormalizedTo);
float4x4 make_rotation(const quatf& _Quaternion);
quatf make_quaternion(const float3& _Radians);
quatf make_quaternion(float _Radians, const float3& _NormalizedRotationAxis);
quatf make_quaternion(const float4x4& _Matrix);

// There are infinite/undefined solutions when the vectors point in opposite directions
quatf make_quaternion(const float3& _NormalizedFrom, const float3& _NormalizedTo);

float4x4 make_translation(const float3& _Translation);
float4x4 make_scale(const float3& _Scale);
inline float4x4 make_scale(float _Scale) { return make_scale(float3(_Scale)); }

float4x4 make_matrix(const float3x3& _Rotation, const float3& _Translation);
inline float4x4 make_matrix(const quatf& _Rotation, const float3& _Translation) { return float4x4((float3x3)make_rotation(_Rotation), _Translation); }

float4x4 make_reflection(const float4& _NormalizedReflectionPlane);

// creates a matrix that transforms points from ones that lie on the XY plane (+Z up)
// to points that lie on plane p
float4x4 make_plane(const float4& _Plane);

// Compute the scale and translate matrix that converts the specified bound into
// a unit AABox with a min at (0,0,0) and max at (1,1,1).
float4x4 make_normalization(const float3& _AABoxMin, const float3& _AABoxMax);

// _____________________________________________________________________________
// Non-view, non-perspective matrix decomposition functions

// Decomposes the specified matrix into its components as if they were applied 
// in the following order:
// ScaleXYZ ShearXY ShearXY ShearZY RotationXYZ TranslationXYZ ProjectionXYZW
// Returns true if successful, or false if the specified matrix is singular.
// NOTE: This does not support negative scale well. Rotations might appear 
// rotated by 180 degrees, and the resulting scale can be the wrong sign.
bool decompose(const float4x4& _Matrix, float3* _pScale
	, float* _pShearXY, float* _pShearXZ, float* _pShearZY
	, float3* _pRotation, float3* _pTranslation, float4* _pPerspective);

// Extract components from a matrix. Currently this uses decompose and just hides
// a bunch of the extra typing that's required.
inline float3 extract_scale(const float4x4& _Matrix)
{
	float xy, xz, zy; float3 s, r, t; float4 p;
	decompose(_Matrix, &s, &xy, &xz, &zy, &r, &t, &p);
	return s;
}

inline float3 extract_rotation(const float4x4& _Matrix)
{
	float xy, xz, zy; float3 s, r, t; float4 p;
	decompose(_Matrix, &s, &xy, &xz, &zy, &r, &t, &p);
	return r;
}

inline float3 extract_translation(const float4x4& _Matrix)
{
	return _Matrix.Column3.xyz();
}

// Output axis values are not normalized.
inline float3 extract_axis_x(const float4x4& _Matrix) { return _Matrix.Column0.xyz(); }
inline float3 extract_axis_y(const float4x4& _Matrix) { return _Matrix.Column1.xyz(); }
inline float3 extract_axis_z(const float4x4& _Matrix) { return _Matrix.Column2.xyz(); }

inline float3 extract_axis_x(const float3x3& _Matrix) { return _Matrix.Column0; }
inline float3 extract_axis_y(const float3x3& _Matrix) { return _Matrix.Column1; }
inline float3 extract_axis_z(const float3x3& _Matrix) { return _Matrix.Column2; }

// _____________________________________________________________________________
// View/Projection matrix composition functions

float4x4 make_lookat_lh(const float3& _Eye, const float3& _At, const float3& _Up);
float4x4 make_lookat_rh(const float3& _Eye, const float3& _At, const float3& _Up);
float4x4 make_orthographic_lh(float _Left, float _Right, float _Bottom, float _Top, float _ZNear = oDEFAULT_NEAR, float _ZFar = oDEFAULT_FAR);
float4x4 make_orthographic_rh(float _Left, float _Right, float _Bottom, float _Top, float _ZNear = oDEFAULT_NEAR, float _ZFar = oDEFAULT_FAR);

// Creates a perspective projection matrix. If _ZFar is less than 0, then an 
// infinitely far plane is used. Remember precision issues with regard to near
// and far plane. Because precision on most hardware is distributed 
// logarithmically so that more precision is near to the view, you can gain more
// precision across the entire scene by moving out the near plane slightly 
// relative to bringing in the far plane by a lot.
float4x4 make_perspective_lh(float _FovYRadians, float _AspectRatio, float _ZNear = oDEFAULT_NEAR, float _ZFar = oDEFAULT_FAR);
float4x4 make_perspective_rh(float _FovYRadians, float _AspectRatio, float _ZNear = oDEFAULT_NEAR, float _ZFar = oDEFAULT_FAR);

// Given the corners of the display medium (relative to the origin where the eye
// is) create an off-axis (off-center) projection matrix.
float4x4 make_offcenter_perspective_lh(float _Left, float _Right, float _Bottom, float _ZNear = oDEFAULT_NEAR, float _ZFar = oDEFAULT_FAR);

// Create an off-axis (off-center) projection matrix properly sized to the 
// specified output medium (i.e. the screen if it were transformed in world 
// space)
// _OutputTransform: Scale, Rotation, Translation (SRT) of the output device 
// plane (i.e. the screen) in the same space as the eye (usually world space)
// _EyePosition: the location of the eye in the same space as _OutputTransform
// _ZNear: A near plane that is different than the plane of the output 
// (physical glass of the screen). By specifying a near plane that is closer to
// the eye than the glass plane, an effect of "popping out" 3D can be achieved.
float4x4 make_offcenter_perspective_lh(const float4x4& _OutputTransform
	, const float3& _EyePosition, float _ZNear = oDEFAULT_NEAR);

// Given some information about the render target in NDC space, create a scale
// and bias transform to create a viewport. Pre-multiply this by the projection
// matrix to get a sub-projection matrix just for the specified viewport.
float4x4 make_viewport(float _NDCResolutionX, float _NDCResolutionY, float _NDCRectLeft, float _NDCRectBottom, float _NDCRectWidth, float _NDCRectHeight);

// Converts a right-handed view matrix to left-handed, and vice-versa
// http://msdn.microsoft.com/en-us/library/bb204853(v=vs.85).aspx
inline float4x4 flip_view_handedness(const float4x4& _View)
{
	float4x4 m = _View;
	m.Column2 = -m.Column2;
	return m;
}

// Returns a view matrix that fits the bounding sphere of the specified bounds
// to the specified projection. This preserves the rotation of the view, so this
// will only update the position of the view to be far enough from the center of 
// the bounds so it all fits on screen.
float4x4 fit_to_view(const float4x4& _View, float _FovYRadians
	, const float3& _BoundCenter, float _BoundRadius, float _OffsetMultiplier = 1.0f);

float4x4 fit_to_view(const float4x4& _View, const float4x4& _Projection
	, const float3& _BoundCenter, float _BoundRadius, float _OffsetMultiplier = 1.0f);

// _____________________________________________________________________________
// View/Projection matrix decomposition functions

// Get the original axis values from a view matrix
void extract_lookat(const float4x4& _View, float3* _pEye, float3* _pAt
	, float3* _pUp, float3* _pRight);

// Get the world space eye position from a view matrix
inline float3 extract_eye(const float4x4& _View)
{
	return invert(_View).Column3.xyz();
}

void extract_near_far_distances(const float4x4& _Projection
, float* _pNearDistance, float* _pFarDistance);

// Extracts the parameters used in make_perspectivec() or make_orthographic()
// to define the specified projection.
void extract_perspective_parameters(const float4x4& _Projection
	, float* _pFovYRadians, float* _pAspectRatio, float* _pZNear, float* _pZFar);

// _____________________________________________________________________________
// Misc

float calcuate_area_and_centroid(float2* _pCentroid
, const float2* _pVertices, size_t _VertexStride, size_t _NumVertices);

// Computes the gaussian weight of a specific sample in a 1D kernel 
inline float gaussian_weight(float stdDeviation, int sampleIndex)
{
	return (1.0f / (sqrt(2.0f * oPIf) * stdDeviation)) * pow(oEf, -((float)(sampleIndex * sampleIndex) / (2.0f * stdDeviation * stdDeviation)));
}

// Determines a location in 3D space based on 4 reference locations and their 
// distances from the location.
float trilaterate(const float3 _ObserversIn[4], const float _DistancesIn[4], float3* _pPosition);

inline float trilaterate(const float3 _Observers[4], float _Distance, float3* _pPosition)
{
	float distances[4];
	for(int i = 0; i < 4; i++)
		distances[i] = _Distance;
	return trilaterate(_Observers, distances, _pPosition);
}

} // namespace ouro

#endif
