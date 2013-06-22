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
#include <oBasis/oLinearAlgebra.h>
#include <oCompute/oComputeConstants.h>
#include <oCompute/oComputeUtil.h>

#include <vectormath/scalar/cpp/vectormath_aos.h>
#include <vectormath/scalar/cpp/vectormath_aos_d.h>
using namespace Vectormath::Aos;

float3x3 invert(const float3x3& _Matrix) { Matrix3 m = inverse((const Matrix3&)_Matrix); return (float3x3&)m; }
double3x3 invert(const double3x3& _Matrix) { Matrix3d m = inverse((const Matrix3d&)_Matrix); return (double3x3&)m; }
float4x4 invert(const float4x4& _Matrix) { Matrix4 m = inverse((const Matrix4&)_Matrix); return (float4x4&)m; }
double4x4 invert(const double4x4& _Matrix) { Matrix4d m = inverse((const Matrix4d&)_Matrix); return (double4x4&)m; }

float4x4 oCreateRotation(const float3& _Radians) { Matrix4 m = Matrix4::rotationZYX((const Vector3&)_Radians); return (float4x4&)m; }
double4x4 oCreateRotation(const double3& _Radians) { Matrix4d m = Matrix4d::rotationZYX((const Vector3d&)_Radians); return (double4x4&)m; }

float4x4 oCreateRotation(const float& _Radians, const float3& _NormalizedRotationAxis) { Matrix4 m = Matrix4::rotation(_Radians, (const Vector3&)_NormalizedRotationAxis); return (float4x4&)m; }
double4x4 oCreateRotation(const double& _Radians, const double3& _NormalizedRotationAxis) { Matrix4d m = Matrix4d::rotation(_Radians, (const Vector3d&)_NormalizedRotationAxis); return (double4x4&)m; }

template<typename T> TMAT4<T> oCreateRotation(const TVEC3<T>& _NormalizedSrcVec, const TVEC3<T>& _NormalizedDstVec)
{
	T a = angle(_NormalizedSrcVec, _NormalizedDstVec);

	// Check for identity
	if (oStd::equal(a, T(0)))
		return oIdentity<TMAT4<T>>();

	// Check for flip
	if(oStd::equal(a, T(oPI)))
		return TMAT4<T>(
			TVEC4<T>(T(-1), T(0), T(0), T(0)),
			TVEC4<T>(T(0), T(-1), T(0), T(0)),
			TVEC4<T>(T(0), T(0), T(-1), T(0)),
			TVEC4<T>(T(0), T(0), T(0), T(1)));

	TVEC3<T> NormalizedAxis = normalize(cross(_NormalizedSrcVec, _NormalizedDstVec));
	return oCreateRotation(a, NormalizedAxis);
}
float4x4 oCreateRotation(const float3& _NormalizedSrcVec, const float3& _NormalizedDstVec) { return oCreateRotation<float>(_NormalizedSrcVec, _NormalizedDstVec); }
double4x4 oCreateRotation(const double3& _NormalizedSrcVec, const double3& _NormalizedDstVec) { return oCreateRotation<double>(_NormalizedSrcVec, _NormalizedDstVec); }

float4x4 oCreateRotation(const quatf& _Quaternion) { Matrix4 m = Matrix4::rotation((const Quat&)_Quaternion); return (float4x4&)m; }
double4x4 oCreateRotation(const quatd& _Quaternion) { Matrix4d m = Matrix4d::rotation((const Quatd&)_Quaternion); return (double4x4&)m; }

quatf oCreateRotationQ(const float3& _Radians) { Quat q = Quat::Quat(Matrix3::rotationZYX((const Vector3&)_Radians)); return (quatf&)q; }
quatd oCreateRotationQ(const double3& _Radians) { Quatd q = Quatd::Quatd(Matrix3d::rotationZYX((const Vector3d&)_Radians)); return (quatd&)q; }
quatf oCreateRotationQ(float _Radians, const float3& _NormalizedRotationAxis) { Quat q = Quat::rotation(_Radians, (const Vector3&)_NormalizedRotationAxis); return (quatf&)q; }
quatd oCreateRotationQ(double _Radians, const double3& _NormalizedRotationAxis) { Quatd q = Quatd::rotation(_Radians, (const Vector3d&)_NormalizedRotationAxis); return (quatd&)q; }
quatf oCreateRotationQ(const float3& _CurrentVector, const float3& _DesiredVector) { Quat q = Quat::rotation((const Vector3&)_CurrentVector, (const Vector3&)_DesiredVector); return (quatf&)q; }
quatd oCreateRotationQ(const double3& _CurrentVector, const double3& _DesiredVector) { Quatd q = Quatd::rotation((const Vector3d&)_CurrentVector, (const Vector3d&)_DesiredVector); return (quatd&)q; }
quatf oCreateRotationQ(const float4x4& _Matrix) { Quat q = Quat(((const Matrix4&)_Matrix).getUpper3x3()); return (quatf&)q; }
quatd oCreateRotationQ(const double4x4& _Matrix) { Quatd q = Quatd(((const Matrix4d&)_Matrix).getUpper3x3()); return (quatd&)q; }

// @oooii-tony: Should this be exposed? I've not seen this function around, but
// it seems to be a precursor to ROP-style blend operations.
template<typename T> const TVEC3<T> combine(const TVEC3<T>& a, const TVEC3<T>& b, T aScale, T bScale) { return aScale * a + bScale * b; }

// returns the specified vector with the specified length
template<typename T> TVEC3<T> oScale(const TVEC3<T>& a, T newLength)
{
	T oldLength = length(a);
	if (oStd::equal(oldLength, T(0.0)))
		return a;
	return a * (newLength / oldLength);
}

template<typename T> bool oDecomposeT(const TMAT4<T>& _Matrix, TVEC3<T>* _pScale, T* _pShearXY, T* _pShearXZ, T* _pShearYZ, TVEC3<T>* _pRotation, TVEC3<T>* _pTranslation, TVEC4<T>* _pPerspective)
{
	/** <citation
		usage="Adaptation" 
		reason="Lots of math, code already written" 
		author="Spencer W. Thomas"
		description="http://tog.acm.org/resources/GraphicsGems/gemsii/unmatrix.c"
		license="*** Assumed Public Domain ***"
		licenseurl="http://tog.acm.org/resources/GraphicsGems/gemsii/unmatrix.c"
		modification="templatized, changed matrix calls"
	/>*/

	// Code obtained from Graphics Gems 2 source code unmatrix.c

//int
//unmatrix( mat, tran )
//Matrix4 *mat;
//double tran[16];
//{
 	register int i, j;
 	TMAT4<T> locmat;
 	TMAT4<T> pmat, invpmat, tinvpmat;
 	/* Vector4 type and functions need to be added to the common set. */
 	TVEC4<T> prhs, psol;
 	TVEC3<T> row[3], pdum3;

 	locmat = _Matrix;
 	/* Normalize the matrix. */
 	if ( locmat[3][3] == 0 )
 		return 0;
 	for ( i=0; i<4;i++ )
 		for ( j=0; j<4; j++ )
 			locmat[i][j] /= locmat[3][3];
 	/* pmat is used to solve for perspective, but it also provides
 	 * an easy way to test for singularity of the upper 3x3 component.
 	 */
 	pmat = locmat;
 	for ( i=0; i<3; i++ )
 		pmat[i][3] = 0;
 	pmat[3][3] = 1;

 	if ( determinant(pmat) == 0.0 )
 		return 0;

 	/* First, isolate perspective.  This is the messiest. */
 	if ( locmat[0][3] != 0 || locmat[1][3] != 0 ||
 		locmat[2][3] != 0 ) {
 		/* prhs is the right hand side of the equation. */
 		prhs.x = locmat[0][3];
 		prhs.y = locmat[1][3];
 		prhs.z = locmat[2][3];
 		prhs.w = locmat[3][3];

 		/* Solve the equation by inverting pmat and multiplying
 		 * prhs by the inverse.  (This is the easiest way, not
 		 * necessarily the best.)
 		 * inverse function (and det4x4, above) from the Matrix
 		 * Inversion gem in the first volume.
 		 */
 		invpmat = invert( pmat );
		tinvpmat = transpose( invpmat );
		psol = tinvpmat * prhs;
 
 		/* Stuff the answer away. */
		
 		_pPerspective->x = psol.x;
 		_pPerspective->y = psol.y;
 		_pPerspective->z = psol.z;
 		_pPerspective->w = psol.w;
 		/* Clear the perspective partition. */
 		locmat[0][3] = locmat[1][3] =
 			locmat[2][3] = 0;
 		locmat[3][3] = 1;
 	} else		/* No perspective. */
 		_pPerspective->x = _pPerspective->y = _pPerspective->z =
 			_pPerspective->w = 0;

 	/* Next take care of translation (easy). */
 	for ( i=0; i<3; i++ ) {
 		(*_pTranslation)[i] = locmat[3][i];
 		locmat[3][i] = 0;
 	}

 	/* Now get scale and shear. */
 	for ( i=0; i<3; i++ ) {
 		row[i].x = locmat[i][0];
 		row[i].y = locmat[i][1];
 		row[i].z = locmat[i][2];
 	}

 	/* Compute X scale factor and normalize first row. */
 	_pScale->x = length(*(TVEC3<T>*)&row[0]);
 	*(TVEC3<T>*)&row[0] = oScale(*(TVEC3<T>*)&row[0], T(1.0));

 	/* Compute XY shear factor and make 2nd row orthogonal to 1st. */
 	*_pShearXY = dot(*(TVEC3<T>*)&row[0], *(TVEC3<T>*)&row[1]);
	*(TVEC3<T>*)&row[1] = combine(*(TVEC3<T>*)&row[1], *(TVEC3<T>*)&row[0], T(1.0), -*_pShearXY);

 	/* Now, compute Y scale and normalize 2nd row. */
 	_pScale->y = length(*(TVEC3<T>*)&row[1]);
 	*(TVEC3<T>*)&row[1] = oScale(*(TVEC3<T>*)&row[1], T(1.0));
 	*_pShearXY /= _pScale->y;

 	/* Compute XZ and YZ shears, orthogonalize 3rd row. */
 	*_pShearXZ = dot(*(TVEC3<T>*)&row[0], *(TVEC3<T>*)&row[2]);
	*(TVEC3<T>*)&row[2] = combine(*(TVEC3<T>*)&row[2], *(TVEC3<T>*)&row[0], T(1.0), -*_pShearXZ);
 	*_pShearYZ = dot(*(TVEC3<T>*)&row[1], *(TVEC3<T>*)&row[2]);
 	*(TVEC3<T>*)&row[2] = combine(*(TVEC3<T>*)&row[2], *(TVEC3<T>*)&row[1], T(1.0), -*_pShearYZ);

 	/* Next, get Z scale and normalize 3rd row. */
 	_pScale->z = length(*(TVEC3<T>*)&row[2]);
 	*(TVEC3<T>*)&row[2] = oScale(*(TVEC3<T>*)&row[2], T(1.0));
 	*_pShearXZ /= _pScale->z;
 	*_pShearYZ /= _pScale->z;
 
 	/* At this point, the matrix (in rows[]) is orthonormal.
 	 * Check for a coordinate system flip.  If the determinant
 	 * is -1, then negate the matrix and the scaling factors.
 	 */
 	if ( dot( *(TVEC3<T>*)&row[0], cross(*(TVEC3<T>*)&row[1], *(TVEC3<T>*)&row[2]) ) < T(0) )
 		for ( i = 0; i < 3; i++ ) {
 			(*_pScale)[i] *= T(-1);
 			row[i].x *= T(-1);
 			row[i].y *= T(-1);
 			row[i].z *= T(-1);
 		}
 
 	/* Now, get the rotations out, as described in the gem. */
 	_pRotation->y = asin(-row[0].z);
 	if ( cos(_pRotation->y) != 0 ) {
 		_pRotation->x = atan2(row[1].z, row[2].z);
 		_pRotation->z = atan2(row[0].y, row[0].x);
 	} else {
 		_pRotation->x = atan2(-row[2].x, row[1].y);
 		_pRotation->z = 0;
 	}
 	/* All done! */
 	return 1;
}
bool oDecompose(const float4x4& _Matrix, float3* _pScale, float* _pShearXY, float* _pShearXZ, float* _pShearZY, float3* _pRotation, float3* _pTranslation, float4* _pPerspective) { return oDecomposeT(_Matrix, _pScale, _pShearXY, _pShearXZ, _pShearZY, _pRotation, _pTranslation, _pPerspective); }
bool oDecompose(const double4x4& _Matrix, double3* _pScale, double* _pShearXY, double* _pShearXZ, double* _pShearZY, double3* _pRotation, double3* _pTranslation, double4* _pPerspective) { return oDecomposeT(_Matrix, _pScale, _pShearXY, _pShearXZ, _pShearZY, _pRotation, _pTranslation, _pPerspective); }

// declares and initializes _22 and _32
// Infinite projection matrix
// http://www.google.com/url?sa=t&source=web&cd=2&ved=0CBsQFjAB&url=http%3A%2F%2Fwww.terathon.com%2Fgdc07_lengyel.ppt&rct=j&q=eric%20lengyel%20projection&ei=-NpaTZvWKYLCsAOluNisCg&usg=AFQjCNGkbo93tbmlrXqkbdJg-krdEYNS1A

// http://knol.google.com/k/perspective-transformation#
// A nice reference that has LH and RH perspective matrices right next to each 
// other.

static const float Z_PRECISION = 0.0001f;
#define INFINITE_PLANE_LH(_ZFar) \
	float _22 = (_ZFar < 0.0f) ? (1.0f - Z_PRECISION) : _ZFar / (_ZFar - _ZNear); \
	float _32 = (_ZFar < 0.0f) ? _ZNear * (2.0f - Z_PRECISION) : (-_ZNear * _ZFar / (_ZFar - _ZNear))

// For off-center projection _32 needs to be negated when compared to INFINITE_PLANE_LH
#define INFINITE_PLANE_OFFCENTER_LH(_ZFar) \
	float _22 = (_ZFar < 0.0f) ? (1.0f - Z_PRECISION) : _ZFar / (_ZFar - _ZNear); \
	float _32 = (_ZFar < 0.0f) ? _ZNear * -(2.0f - Z_PRECISION) : (_ZNear * _ZFar / (_ZFar - _ZNear))

#define INFINITE_PLANE_RH(_ZFar) \
	float _22 = (_ZFar < 0.0f) ? -(1.0f - Z_PRECISION) : _ZFar / (_ZNear - _ZFar); \
	float _32 = (_ZFar < 0.0f) ? _ZNear * -(2.0f - Z_PRECISION) : (_ZNear * _ZFar / (_ZNear - _ZFar))

float4x4 oCreatePerspectiveLH(float _FovYRadians, float _AspectRatio, float _ZNear, float _ZFar)
{
	float yScale = 1.0f / tanf(_FovYRadians / 2.0f);
	float xScale = yScale / _AspectRatio;

	INFINITE_PLANE_LH(_ZFar);

	return float4x4(
		float4(xScale, 0.0f, 0.0f, 0.0f),
		float4(0.0f, yScale, 0.0f, 0.0f),
		float4(0.0f, 0.0f, _22, 1.0f),
		float4(0.0f, 0.0f, _32, 0.0f));
}

float4x4 oCreatePerspectiveRH(float _FovYRadians, float _AspectRatio, float _ZNear, float _ZFar)
{
	float yScale = 1.0f / tanf(_FovYRadians / 2.0f);
	float xScale = yScale / _AspectRatio;

	INFINITE_PLANE_RH(_ZFar);

	return float4x4(
		float4(xScale, 0.0f, 0.0f, 0.0f),
		float4(0.0f, yScale, 0.0f, 0.0f),
		float4(0.0f, 0.0f, _22, -1.0f),
		float4(0.0f, 0.0f, _32, 0.0f));
}

float4x4 oCreateOffCenterPerspectiveLH(float _Left, float _Right, float _Bottom, float _Top, float _ZNear)
{
	INFINITE_PLANE_OFFCENTER_LH(-1.0f);

	return float4x4(
		float4((2*_ZNear)/(_Right-_Left),     0.0f,                          0.0f, 0.0f),
		float4(0.0f,                          (2.0f*_ZNear)/(_Top-_Bottom),  0.0f, 0.0f),
		float4((_Left+_Right)/(_Left-_Right), (_Top+_Bottom)/(_Bottom-_Top), _22,  1.0f),
		float4(0.0f,                          0.0f,                          _32,  0.0f));
}

// _OutputTransform Scale, Rotation, Translation (SRT) of the output device plane
// (i.e. the screen) in the same space as the eye (usually world space)
float4x4 oCreateOffCenterPerspectiveLH(const float4x4& _OutputTransform, const float3& _EyePosition, float _ZNear)
{
	// Get the position and dimensions of the output
	float ShXY, ShXZ, ShZY;
	float3 outputSize, R, outputPosition;
	float4 P;
	oDecompose(_OutputTransform, &outputSize, &ShXY, &ShXZ, &ShZY, &R, &outputPosition, &P);
	float3 offset = outputPosition - _EyePosition;

	// Get the basis of the output
	float3 outputBasisX = normalize(oExtractAxisX(_OutputTransform));
	float3 outputBasisY = normalize(oExtractAxisY(_OutputTransform));
	float3 outputBasisZ = normalize(oExtractAxisZ(_OutputTransform));

	// Get local offsets from the eye to the output
	float w = dot(outputBasisX, offset);
	float h = dot(outputBasisY, offset);
	float d = dot(outputBasisZ, offset);

	// Incorporate user near plane adjustment
	float zn = __max(d + _ZNear, 0.01f);
	float depthScale = zn / d;

	return oCreateOffCenterPerspectiveLH(w * depthScale, (w + outputSize.x) * depthScale, h * depthScale, (h + outputSize.y) * depthScale, zn);
}

float4x4 oCreateViewport(float _NDCResolutionX, float _NDCResolutionY, float _NDCRectLeft, float _NDCRectBottom, float _NDCRectWidth, float _NDCRectHeight)
{
	float2 dim = float2(_NDCResolutionX, _NDCResolutionY);
	float2 NDCMin = (float2(_NDCRectLeft, _NDCRectBottom) / dim) * 2.0f - 1.0f;
	float2 NDCMax = (float2(_NDCRectLeft + _NDCRectWidth, _NDCRectBottom + _NDCRectHeight) / dim) * 2.0f - 1.0f;
	float2 NDCScale = 2.0f / (NDCMax - NDCMin);
	float2 NDCTranslate = float2(-1.0f, 1.0f) - NDCMin * float2(NDCScale.x, -NDCScale.y);
	return float4x4(
		float4(NDCScale.x,     0.0f,           0.0f, 0.0f),
		float4(0.0f,           NDCScale.y,     0.0f, 0.0f),
		float4(0.0f,           0.0f,           1.0f, 0.0f),
		float4(NDCTranslate.x, NDCTranslate.y, 0.0f, 1.0f));
}
