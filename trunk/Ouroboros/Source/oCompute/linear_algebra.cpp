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
#include <oCompute/linear_algebra.h>

#include <vectormath/scalar/cpp/vectormath_aos.h>
#include <vectormath/scalar/cpp/vectormath_aos_d.h>
using namespace Vectormath::Aos;

#include "linear_algebra.inl"

namespace ouro {

float3x3 invert(const float3x3& _Matrix)
{
	Matrix3 m = inverse((const Matrix3&)_Matrix);
	return (float3x3&)m;
}

float4x4 invert(const float4x4& _Matrix)
{
	Matrix4 m = inverse((const Matrix4&)_Matrix);
	return (float4x4&)m;
}

float4x4 make_rotation(const float3& _Radians)
{
	Matrix4 m = Matrix4::rotationZYX((const Vector3&)_Radians);
	return (float4x4&)m;
}

float4x4 make_rotation(float _Radians, const float3& _NormalizedRotationAxis)
{
	Matrix4 m = Matrix4::rotation(_Radians, (const Vector3&)_NormalizedRotationAxis);
	return (float4x4&)m;
}

float4x4 make_rotation(const float3& _NormalizedFrom, const float3& _NormalizedTo)
{
	return template_::make_rotation<float>(_NormalizedFrom, _NormalizedTo);
}

float4x4 make_rotation(const quatf& _Quaternion)
{
	Matrix4 m = Matrix4::rotation((const Quat&)_Quaternion);
	return (float4x4&)m;
}

quatf make_quaternion(const float3& _Radians)
{
	Quat q = Quat::Quat(Matrix3::rotationZYX((const Vector3&)_Radians));
	return (quatf&)q;
}

quatf make_quaternion(float _Radians, const float3& _NormalizedRotationAxis)
{
	Quat q = Quat::rotation(_Radians, (const Vector3&)_NormalizedRotationAxis);
	return (quatf&)q;
}

quatf make_quaternion(const float3& _CurrentVector, const float3& _DesiredVector)
{
	Quat q = Quat::rotation((const Vector3&)_CurrentVector, (const Vector3&)_DesiredVector);
	return (quatf&)q;
}

quatf make_quaternion(const float4x4& _Matrix)
{
	Quat q = Quat(((const Matrix4&)_Matrix).getUpper3x3());
	return (quatf&)q;
}

float4x4 make_translation(const float3& _Translation)
{
	return template_::make_translation<float>(_Translation);
}

float4x4 make_scale(const float3& _Scale)
{
	return template_::make_scale<float>(_Scale);
}

float4x4 make_matrix(const float3x3& _Rotation, const float3& _Translation)
{
	return float4x4(_Rotation, _Translation);
}

float4x4 make_reflection(const float4& _NormalizedReflectionPlane)
{
	return template_::make_reflection<float>(_NormalizedReflectionPlane);
}

float4x4 make_plane(const float4& _Plane)
{
	return template_::make_plane<float>(_Plane);
}

float4x4 make_normalization(const float3& _AABoxMin, const float3& _AABoxMax)
{
	return template_::make_normalization(_AABoxMin, _AABoxMax);
}

bool decompose(const float4x4& _Matrix, float3* _pScale
	, float* _pShearXY, float* _pShearXZ, float* _pShearZY
	, float3* _pRotation, float3* _pTranslation, float4* _pPerspective)
{
	return template_::decompose(_Matrix, _pScale, _pShearXY, _pShearXZ, _pShearZY
		, _pRotation, _pTranslation, _pPerspective);
}

float4x4 make_lookat_lh(const float3& _Eye, const float3& _At, const float3& _Up)
{
	return template_::make_lookat_lh<float>(_Eye, _At, _Up);
}

float4x4 make_lookat_rh(const float3& _Eye, const float3& _At, const float3& _Up)
{
	return template_::make_lookat_rh<float>(_Eye, _At, _Up);
}

float4x4 make_orthographic_lh(float _Left, float _Right, float _Bottom, float _Top, float _ZNear, float _ZFar)
{
	return template_::make_orthographic_lh<float>(_Left, _Right, _Bottom, _Top, _ZNear, _ZFar);
}

float4x4 make_orthographic_rh(float _Left, float _Right, float _Bottom, float _Top, float _ZNear, float _ZFar)
{
	return template_::make_orthographic_rh<float>(_Left, _Right, _Bottom, _Top, _ZNear, _ZFar);
}

float4x4 make_perspective_lh(float _FovYRadians, float _AspectRatio, float _ZNear, float _ZFar)
{
	return template_::make_perspective_lh<float>(_FovYRadians, _AspectRatio, _ZNear, _ZFar);
}

float4x4 make_perspective_rh(float _FovYRadians, float _AspectRatio, float _ZNear, float _ZFar)
{
	return template_::make_perspective_rh<float>(_FovYRadians, _AspectRatio, _ZNear, _ZFar);
}

float4x4 make_offcenter_perspective_lh(float _Left, float _Right, float _Bottom, float _ZNear, float _ZFar)
{
	return template_::make_offcenter_perspective_lh<float>(_Left, _Right, _Bottom, _ZNear, _ZFar);
}

float4x4 make_offcenter_perspective_lh(const float4x4& _OutputTransform
	, const float3& _EyePosition, float _ZNear)
{
	return template_::make_offcenter_perspective_lh<float>(_OutputTransform, _EyePosition, _ZNear);
}

float4x4 make_viewport(float _NDCResolutionX, float _NDCResolutionY, float _NDCRectLeft, float _NDCRectBottom, float _NDCRectWidth, float _NDCRectHeight)
{
	return template_::make_viewport<float>(_NDCResolutionX, _NDCResolutionY, _NDCRectLeft, _NDCRectBottom, _NDCRectWidth, _NDCRectHeight);
}

float4x4 fit_to_view(const float4x4& _View, float _FovYRadians
	, const float3& _BoundCenter, float _BoundRadius, float _OffsetMultiplier)
{
	return template_::fit_to_view<float>(_View, _FovYRadians, _BoundCenter, _BoundRadius, _OffsetMultiplier);
}

float4x4 fit_to_view(const float4x4& _View, const float4x4& _Projection
	, const float3& _BoundCenter, float _BoundRadius, float _OffsetMultiplier)
{
	return template_::fit_to_view(_View, _Projection, _BoundCenter, _BoundRadius, _OffsetMultiplier);
}

void extract_lookat(const float4x4& _View, float3* _pEye, float3* _pAt
	, float3* _pUp, float3* _pRight)
{
	*_pEye = invert(_View).Column3.xyz();
	_pRight->x = _View[0][0]; _pUp->x = _View[0][1]; _pAt->x = -_View[0][2];
	_pRight->y = _View[1][0]; _pUp->y = _View[1][1]; _pAt->y = -_View[1][2];
	_pRight->z = _View[2][0]; _pUp->z = _View[2][1]; _pAt->z = -_View[2][2];
}

void extract_near_far_distances(const float4x4& _Projection
	, float* _pNearDistance, float* _pFarDistance)
{
	return template_::extract_near_far_distances<float>(_Projection, _pNearDistance, _pFarDistance);
}

void extract_perspective_parameters(const float4x4& _Projection
	, float* _pFovYRadians, float* _pAspectRatio, float* _pZNear, float* _pZFar)
{
	return template_::extract_perspective_parameters<float>(_Projection, _pFovYRadians, _pAspectRatio, _pZNear, _pZFar);
}

float trilaterate(const float3 _ObserversIn[4], const float _DistancesIn[4], float3* _pPosition)
{
	return template_::trilaterate<float>(_ObserversIn, _DistancesIn, _pPosition);
}

bool coordinate_transform(const float3 _FromCoords[4], const float3 _ToCoords[4], float4x4* _pMatrix)
{
	return template_::coordinate_transform<float>(_FromCoords, _ToCoords, _pMatrix);
}

} // namespace ouro
