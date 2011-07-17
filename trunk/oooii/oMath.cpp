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
#include <oooii/oMath.h>
#include <oooii/oAssert.h>
#include <oooii/oByte.h>
#include <oooii/oErrno.h>
#include <oooii/oSingleton.h>
#include <oooii/oSTL.h>
#include <oooii/oString.h>
#include "perlin.h"
#include <vectormath/scalar/cpp/vectormath_aos.h>
#include <vectormath/scalar/cpp/vectormath_aos_d.h>
#include <oooii/oThreading.h>

using namespace Vectormath::Aos;

const quatf quatf::Identity(0.0f, 0.0f, 0.0f, 1.0f);
const quatd quatd::Identity(0.0, 0.0, 0.0, 1.0);

const float3x3 float3x3::Identity(float3(1.0f, 0.0f, 0.0f),
                                  float3(0.0f, 1.0f, 0.0f),
                                  float3(0.0f, 0.0f, 1.0f));

const double3x3 double3x3::Identity(double3(1.0, 0.0, 0.0),
                                    double3(0.0, 1.0, 0.0),
                                    double3(0.0, 0.0, 1.0));

const float4x4 float4x4::Identity(float4(1.0f, 0.0f, 0.0f, 0.0f),
                                  float4(0.0f, 1.0f, 0.0f, 0.0f),
                                  float4(0.0f, 0.0f, 1.0f, 0.0f),
                                  float4(0.0f, 0.0f, 0.0f, 1.0f));

const double4x4 double4x4::Identity(double4(1.0, 0.0, 0.0, 0.0),
                                    double4(0.0, 1.0, 0.0, 0.0),
                                    double4(0.0, 0.0, 1.0, 0.0),
                                    double4(0.0, 0.0, 0.0, 1.0));

struct oPerlinContext : oProcessSingleton<oPerlinContext>
{
	oPerlinContext()
		: P(4,4,1,94)
	{}

	Perlin P;
};

float noise(float x) { return oPerlinContext::Singleton()->P.Get(x); }
float noise(const TVEC2<float>& x) { return oPerlinContext::Singleton()->P.Get(x.x, x.y); }
float noise(const TVEC3<float>& x) { return oPerlinContext::Singleton()->P.Get(x.x, x.y, x.z); }
float noise(const TVEC4<float>& x)  { return oPerlinContext::Singleton()->P.Get(x.x, x.y, x.z); } // @oooii-tony: not yet implemented.

float determinant(const float4x4& _Matrix)
{
	return determinant((const Matrix4&)_Matrix);
}

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

quatf oSlerp(const quatf& a, const quatf& b, float s)
{
	Quat q = slerp(s, (const Quat&)a, (const Quat&)b);
	return (quatf&)q;
}

// @oooii-tony: Should this be exposed? I've not seen this function around, but
// it seems to be a precursor to ROP-style blend operations.
template<typename T> const TVEC3<T> combine(const TVEC3<T>& a, const TVEC3<T>& b, T aScale, T bScale) { return aScale * a + bScale * b; }

// returns the specified vector with the specified length
template<typename T> TVEC3<T> oScale(const TVEC3<T>& a, T newLength)
{
	T oldLength = length(a);
	if (oEqual(oldLength, T(0.0)))
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

bool oDecompose(const float4x4& _Matrix, float3* _pScale, float* _pShearXY, float* _pShearXZ, float* _pShearZY, float3* _pRotation, float3* _pTranslation, float4* _pPerspective)
{
	return oDecomposeT(_Matrix, _pScale, _pShearXY, _pShearXZ, _pShearZY, _pRotation, _pTranslation, _pPerspective);
}

bool oDecompose(const double4x4& _Matrix, double3* _pScale, double* _pShearXY, double* _pShearXZ, double* _pShearZY, double3* _pRotation, double3* _pTranslation, double4* _pPerspective)
{
	return oDecomposeT(_Matrix, _pScale, _pShearXY, _pShearXZ, _pShearZY, _pRotation, _pTranslation, _pPerspective);
}

template<typename T> bool oHasPerspectiveT(const TMAT4<T>& _Matrix)
{
	// Taken from the above decompose() function.
	return (_Matrix[0][3] != 0 || _Matrix[1][3] != 0 || _Matrix[2][3] != 0);
}

bool oHasPerspective(const float4x4& _Matrix)
{
	return oHasPerspectiveT(_Matrix);
}

bool oHasPerspective(const double4x4& _Matrix)
{
	return oHasPerspectiveT(_Matrix);
}

float4x4 oCreateRotation(const float3& _Radians)
{
	Matrix4 m = Matrix4::rotationZYX((const Vector3&)_Radians);
	return (float4x4&)m;
}

float4x4 oCreateRotation(const float& _Radians, const float3& _NormalizedRotationAxis)
{
	Matrix4 m = Matrix4::rotation(_Radians, (const Vector3&)_NormalizedRotationAxis);
	return (float4x4&)m;
}

float4x4 oCreateRotation(const float3& _CurrenTVEC, const float3& _DesiredVector, const float3& _DefaultRotationAxis)
{
	float3 x;

	float a = angle(_CurrenTVEC, _DesiredVector);
	if (oEqual(a, 0.0f))
		return float4x4(float4x4::Identity);
	
	if (_CurrenTVEC == _DesiredVector)
		return float4x4(float4x4::Identity);

	else if (-_CurrenTVEC == _DesiredVector)
		x = _DefaultRotationAxis;

	else
	{
		x = cross(_CurrenTVEC, _DesiredVector);
		if (x == float3(0.0f, 0.0f, 0.0f))
			x = _DefaultRotationAxis;
		else
			x = normalize(x);
	}

	return oCreateRotation(a, x);
}

float4x4 oCreateRotation(const quatf& _Quaternion)
{
	Matrix4 m = Matrix4::rotation((const Quat&)_Quaternion);
	return (float4x4&)m;
}

quatf oCreateRotationQ(const float3& _Radians)
{
	Quat q = Quat::Quat(Matrix3::rotationZYX((const Vector3&)_Radians));
	return (quatf&)q;
}

quatf oCreateRotationQ(float _Radians, const float3& _NormalizedRotationAxis)
{
	Quat q = Quat::rotation(_Radians, (const Vector3&)_NormalizedRotationAxis);
	return (quatf&)q;
}

quatf oCreateRotationQ(const float3& _CurrenTVEC, const float3& _DesiredVector)
{
	Quat q = Quat::rotation((const Vector3&)_CurrenTVEC, (const Vector3&)_DesiredVector);
	return (quatf&)q;
}

quatf oCreateRotationQ(const float4x4& _Matrix)
{
	Quat q = Quat(((const Matrix4&)_Matrix).getUpper3x3());
	return (quatf&)q;
}

float4x4 oCreateTranslation(const float3& _Translation)
{
	return float4x4(
		float4(1.0f, 0.0f, 0.0f, 0.0f),
		float4(0.0f, 1.0f, 0.0f, 0.0f),
		float4(0.0f, 0.0f, 1.0f, 0.0f),
		float4(_Translation, 1.0f));
}

float4x4 oCreateScale(const float3& _Scale)
{
	return float4x4(
		float4(_Scale.x, 0.0f, 0.0f, 0.0f),
		float4(0.0f, _Scale.y, 0.0f, 0.0f),
		float4(0.0f, 0.0f, _Scale.z, 0.0f),
		float4(0.0f, 0.0f, 0.0f, 1.0f));
}

float4x4 oCreateLookAtLH(const float3& _Eye, const float3& _At, const float3& _Up)
{
	float3 z = normalize(_At - _Eye);
	float3 x = normalize(cross(_Up, z));
	float3 y = cross(z, x);

	return float4x4(
		float4(x.x,          y.x,            z.x,           0.0f),
		float4(x.y,          y.y,            z.y,           0.0f),
		float4(x.z,          y.z,            z.z,           0.0f),
		float4(-dot(x, _Eye), -dot(y, _Eye), -dot(z, _Eye), 1.0f));
}

float4x4 oCreateLookAtRH(const float3& _Eye, const float3& _At, const float3& _Up)
{
	float3 z = normalize(_Eye - _At);
	float3 x = normalize(cross(_Up, z));
	float3 y = cross(z, x);

	return float4x4(
		float4(x.x,          y.x,            z.x,           0.0f),
		float4(x.y,          y.y,            z.y,           0.0f),
		float4(x.z,          y.z,            z.z,           0.0f),
		float4(-dot(x, _Eye), -dot(y, _Eye), -dot(z, _Eye), 1.0f));
}

float4x4 oCreateOrthographicLH(float _Left, float _Right, float _Bottom, float _Top, float _ZNear, float _ZFar)
{
	return float4x4(
		float4(2.0f/(_Right-_Left),           0.0f,                          0.0f,                  0.0f),
		float4(0.0f,                          2.0f/(_Top-_Bottom),           0.0f,                  0.0f),
		float4(0.0f,                          0.0f,                          1.0f/(_ZFar-_ZNear),   0.0f),
		float4((_Left+_Right)/(_Left-_Right), (_Top+_Bottom)/(_Bottom-_Top), _ZNear/(_ZNear-_ZFar), 1.0f));
}

float4x4 oCreateOrthographicRH(float _Left, float _Right, float _Bottom, float _Top, float _ZNear, float _ZFar)
{
	return float4x4(
		float4(2.0f/(_Right-_Left),           0.0f,                          0.0f,                  0.0f),
		float4(0.0f,                          2.0f/(_Top-_Bottom),           0.0f,                  0.0f),
		float4(0.0f,                          0.0f,                          1.0f/(_ZNear-_ZFar),   0.0f),
		float4((_Left+_Right)/(_Left-_Right), (_Top+_Bottom)/(_Bottom-_Top), _ZNear/(_ZNear-_ZFar), 1.0f));
}

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
	INFINITE_PLANE_LH(-1.0f);

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
	// @oooii-tony: This is a blind port of code we used in UE3 to do off-axis 
	// projection. UE3 looks down +X (yarly!), so this tries to look down -Z, so
	// there still might be some negation or re-axis-izing that needs to be done.

	oWARN_ONCE("Math not yet confirmed for oCreateOffCenterPerspective(), be careful!");

	// Get the position and dimensions of the output
	float ShXY, ShXZ, ShZY;
	float3 outputSize, R, outputPosition;
	float4 P;
	oDecompose(_OutputTransform, &outputSize, &ShXY, &ShXZ, &ShZY, &R, &outputPosition, &P);
	float3 offset = outputPosition - _EyePosition;

	// Get the basis of the output
	float3 outputBasisX, outputBasisY, outputBasisZ;
	oExtractAxes(_OutputTransform, &outputBasisX, &outputBasisY, &outputBasisZ);

	// Get local offsets from the eye to the output
	float w = dot(outputBasisX, offset);
	float h = dot(outputBasisY, offset);
	float d = dot(outputBasisZ, offset);

	// Incorporate user near plane adjustment
	float zn = __max(d + _ZNear, 3.0f);
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

double determinant(const double4x4& _Matrix)
{
	return determinant((const Matrix4d&)_Matrix);
}

double3x3 invert(const double3x3& _Matrix)
{
	Matrix3d m = inverse((const Matrix3d&)_Matrix);
	return (double3x3&)m;
}

double4x4 invert(const double4x4& _Matrix)
{
	Matrix4d m = inverse((const Matrix4d&)_Matrix);
	return (double4x4&)m;
}

quatd slerp(const quatd& a, const quatd& b, double s)
{
	Quatd q = slerp(s, (const Quatd&)a, (const Quatd&)b);
	return (quatd&)q;
}

void oExtractAxes(const float4x4& _Matrix, float3* _pXAxis, float3* _pYAxis, float3* _pZAxis)
{
	*_pXAxis = _Matrix.Column0.XYZ();
	*_pYAxis = _Matrix.Column1.XYZ();
	*_pZAxis = _Matrix.Column2.XYZ();
}

void oExtractAxes(const double4x4& _Matrix, double3* _pXAxis, double3* _pYAxis, double3* _pZAxis)
{
	*_pXAxis = _Matrix.Column0.XYZ();
	*_pYAxis = _Matrix.Column1.XYZ();
	*_pZAxis = _Matrix.Column2.XYZ();
}

double4x4 oCreateRotation(const double3& _Radians)
{
	Matrix4d m = Matrix4d::rotationZYX((const Vector3d&)_Radians);;
	return (double4x4&)m;
}

double4x4 oCreateRotation(const double &_Radians, const double3& _NormalizedRotationAxis)
{
	Matrix4d m = Matrix4d::rotation(_Radians, (const Vector3d&)_NormalizedRotationAxis);
	return (double4x4&)m;
}

double4x4 oCreateRotation(const double3& _CurrenTVEC, const double3& _DesiredVector, const double3& _DefaultRotationAxis)
{
	double3 x;

	double a = angle(_CurrenTVEC, _DesiredVector);
	if (oEqual(a, 0.0))
		return double4x4(double4x4::Identity);
	
	if (_CurrenTVEC == _DesiredVector)
		return double4x4(double4x4::Identity);

	else if (-_CurrenTVEC == _DesiredVector)
		x = _DefaultRotationAxis;

	else
	{
		x = cross(_CurrenTVEC, _DesiredVector);
		if (x == double3(0.0, 0.0, 0.0))
			x = _DefaultRotationAxis;
		else
			x = normalize(x);
	}

	return oCreateRotation(a, x);
}

double4x4 oCreateRotation(const quatd& _Quaternion)
{
	Matrix4d m = Matrix4d::rotation((const Quatd&)_Quaternion);
	return (double4x4&)m;
}

quatd oCreateRotationQ(const double3& _Radians)
{
	Quatd q = Quatd::Quatd(Matrix3d::rotationZYX((const Vector3d&)_Radians));
	return (quatd&)q;
}

quatd oCreateRotationQ(double _Radians, const double3& _NormalizedRotationAxis)
{
	Quatd q = Quatd::rotation(_Radians, (const Vector3d&)_NormalizedRotationAxis);
	return (quatd&)q;
}

quatd oCreateRotationQ(const double3& _CurrenTVEC, const double3& _DesiredVector)
{
	Quatd q = Quatd::rotation((const Vector3d&)_CurrenTVEC, (const Vector3d&)_DesiredVector);
	return (quatd&)q;
}

quatd oCreateRotationQ(const double4x4& _Matrix)
{
	Quatd q = Quatd(((const Matrix4d&)_Matrix).getUpper3x3());
	return (quatd&)q;
}

double4x4 oCreateTranslation(const double3& _Translation)
{
	Matrix4d m = Matrix4d::translation((const Vector3d&)_Translation);
	return (double4x4&)m;
}

double4x4 oCreateScale(const double3& _Scale)
{
	Matrix4d m = Matrix4d::scale((const Vector3d&)_Scale);
	return (double4x4&)m;
}

float4x4 oAsReflection(const float4& _ReflectionPlane)
{
	float4 p = oNormalizePlane(_ReflectionPlane);
	const float A = p.x;
	const float B = p.y;
	const float C = p.z;
	const float D = p.w;
	return float4x4(
		float4(-2.0f * A * A + 1.0f, -2.0f * B * A, -2.0f * C * A, 0.0f),
		float4(-2.0f * A * B, -2.0f * B * B + 1.0f, -2.0f * C * B, 0.0f),
		float4(-2.0f * A * C, -2.0f * B * C, -2.0f * C * C + 1.0f, 0.0f),
		float4(-2.0f * A * D, -2.0f * B * D, -2.0f * C * D, 1.0f));
}

void oExtractLookAt(const float4x4& _View, float3* _pEye, float3* _pAt, float3* _pUp, float3* _pRight)
{
	*_pEye = invert(_View).Column3.XYZ();
	_pRight->x = _View[0][0]; _pUp->x = _View[0][1]; _pAt->x = -_View[0][2];
	_pRight->y = _View[1][0]; _pUp->y = _View[1][1]; _pAt->y = -_View[1][2];
	_pRight->z = _View[2][0]; _pUp->z = _View[2][1]; _pAt->z = -_View[2][2];
}

float3 oScreenToVector(const float2& _ScreenPoint, const float2& _ViewportDimensions, float _Radius)
{
	// Based on DXUTcamera.cpp

	// @oooii-eric: the -1 caused trouble with manipulators, it assumed coords were in pixels. caller of this function needs to worry about the 
	//	off by one problem instead. i.e. this code now assumes _ScreePoint can equal _ViewportDimensions, instead of maxing out at 1 less than that.
	float2 scr = (_ScreenPoint - (_ViewportDimensions / 2.0f)) / (_Radius * (_ViewportDimensions/*-1.0f*/) / 2.0f);
	scr.x = -scr.x;
	float z = 0.0f;
	float mag = dot(scr, scr); // length squared

	if (mag > 1.0f)
	{
		float scale = 1.0f / sqrt(mag);
		scr *= scale;
	}

	else
		z = sqrt(1.0f - mag);

	return float3(scr.x, scr.y, z);
}

void oCalcPlaneMatrix(const float4& _Plane, float4x4* _pMatrix)
{
	*_pMatrix = oCreateRotation(float3(0.0f, 0.0f, 1.0f), _Plane.XYZ(), float3(0.0f, 1.0f, 0.0f));
	float3 offset(0.0f, 0.0f, _Plane.w);
	oTransformVectors(*_pMatrix, &offset, sizeof(float3), &offset, sizeof(float3), 1);
	_pMatrix->Column3.x = offset.x;
	_pMatrix->Column3.y = offset.y;
	_pMatrix->Column3.z = offset.z;
}

const char* oAsString(const oFrustumf::CORNER& _Corner)
{
	switch (_Corner)
	{
		case oFrustumf::LEFT_TOP_NEAR: return "LEFT_TOP_NEAR";
		case oFrustumf::LEFT_TOP_FAR: return "LEFT_TOP_FAR";
		case oFrustumf::LEFT_BOTTOM_NEAR: return "LEFT_BOTTOM_NEAR";
		case oFrustumf::LEFT_BOTTOM_FAR: return "LEFT_BOTTOM_FAR";
		case oFrustumf::RIGHT_TOP_NEAR: return "RIGHT_TOP_NEAR";
		case oFrustumf::RIGHT_TOP_FAR: return "RIGHT_TOP_FAR";
		case oFrustumf::RIGHT_BOTTOM_NEAR: return "RIGHT_BOTTOM_NEAR";
		case oFrustumf::RIGHT_BOTTOM_FAR: return "RIGHT_BOTTOM_FAR";
		default: oASSUME(0);
	}
}

template<typename T> void oExtractFrustumPlanesT(TVEC4<T> _Planes[6], const TMAT4<T>& _Projection, bool _Normalize)
{
	/** <citation
		usage="Adaptation" 
		reason="Simple straightforward paper with code to do this important conversion." 
		author="Gil Gribb & Klaus Hartmann"
		description="http://www2.ravensoft.com/users/ggribb/plane%20extraction.pdf"
		license="*** Assumed Public Domain ***"
		licenseurl="http://www2.ravensoft.com/users/ggribb/plane%20extraction.pdf"
		modification="negate the w value so that -offsets are opposite the normal and add support for orthographic projections"
	/>*/

	// $(CitedCodeBegin)

	// Left clipping plane
	_Planes[0].x = _Projection.Column0.w + _Projection.Column0.x;
	_Planes[0].y = _Projection.Column1.w + _Projection.Column1.x;
	_Planes[0].z = _Projection.Column2.w + _Projection.Column2.x;
	_Planes[0].w = -(_Projection.Column3.w + _Projection.Column3.x);

	// Right clipping plane
	_Planes[1].x = _Projection.Column0.w - _Projection.Column0.x;
	_Planes[1].y = _Projection.Column1.w - _Projection.Column1.x;
	_Planes[1].z = _Projection.Column2.w - _Projection.Column2.x;
	_Planes[1].w = -(_Projection.Column3.w - _Projection.Column3.x);

	// Top clipping plane
	_Planes[2].x = _Projection.Column0.w - _Projection.Column0.y;
	_Planes[2].y = _Projection.Column1.w - _Projection.Column1.y;
	_Planes[2].z = _Projection.Column2.w - _Projection.Column2.y;
	_Planes[2].w = -(_Projection.Column3.w - _Projection.Column3.y);

	// Bottom clipping plane
	_Planes[3].x = _Projection.Column0.w + _Projection.Column0.y;
	_Planes[3].y = _Projection.Column1.w + _Projection.Column1.y;
	_Planes[3].z = _Projection.Column2.w + _Projection.Column2.y;
	_Planes[3].w = -(_Projection.Column3.w + _Projection.Column3.y);

	// Near clipping plane
	if (oHasPerspective(_Projection))
	{
		_Planes[4].x = _Projection.Column0.z;
		_Planes[4].y = _Projection.Column1.z;
		_Planes[4].z = _Projection.Column2.z;
		_Planes[4].w = -_Projection.Column3.z;
	}

	else
	{
		_Planes[4].x = _Projection.Column0.w + _Projection.Column0.z;
		_Planes[4].y = _Projection.Column1.w + _Projection.Column1.z;
		_Planes[4].z = _Projection.Column2.w + _Projection.Column2.z;
		_Planes[4].w = -(_Projection.Column3.w + _Projection.Column3.z);
	}

	// Far clipping plane
	_Planes[5].x = _Projection.Column0.w - _Projection.Column0.z;
	_Planes[5].y = _Projection.Column1.w - _Projection.Column1.z;
	_Planes[5].z = _Projection.Column2.w - _Projection.Column2.z;
	_Planes[5].w = -(_Projection.Column3.w - _Projection.Column3.z);

	if (_Normalize)
		for (size_t i = 0; i < 6; i++)
			_Planes[i] = oNormalizePlane(_Planes[i]);

	// $(CitedCodeEnd)
}

void oExtractFrustumPlanes(float4 _Planes[6], const float4x4& _Projection, bool _Normalize)
{
	oExtractFrustumPlanesT(_Planes, _Projection, _Normalize);
}

void oTransformPoints(const float4x4& _Matrix, float3* oRESTRICT _pDestination, unsigned int _DestinationStride, const float3* oRESTRICT _pSource, unsigned int _SourceStride, unsigned int _NumPoints)
{
	for (unsigned int i = 0; i < _NumPoints; i++)
	{
		*_pDestination = mul(_Matrix, float4(*_pSource, 1.0f)).XYZ();
		_pDestination = oByteAdd(_pDestination, _DestinationStride);
		_pSource = oByteAdd(_pSource, _SourceStride);
	}
}

void oTransformVectors(const float4x4& _Matrix, float3* oRESTRICT _pDestination, unsigned int _DestinationStride, const float3* oRESTRICT _pSource, unsigned int _SourceStride, unsigned int _NumVectors)
{
	float3x3 m_(_Matrix[0].XYZ(), _Matrix[1].XYZ(), _Matrix[2].XYZ());

	for (unsigned int i = 0; i < _NumVectors; i++)
	{
		*_pDestination = m_ * (*_pSource);
		_pDestination = oByteAdd(_pDestination, _DestinationStride);
		_pSource = oByteAdd(_pSource, _SourceStride);
	}
}

template<typename T> bool oRemoveDegeneratesT(const TVEC3<T>* _pPositions, size_t _NumberOfPositions, unsigned int* _pIndices, size_t _NumberOfIndices, size_t* _pNewNumIndices)
{
	if ((_NumberOfIndices % 3) != 0)
	{
		oSetLastError(EINVAL, "_NumberOfIndices must be a multiple of 3");
		return false;
	}

	for (size_t i = 0; i < _NumberOfIndices / 3; i++)
	{
		size_t I = i * 3;
		size_t J = i * 3 + 1;
		size_t K = i * 3 + 2;

		if (_pIndices[I] >= _NumberOfPositions || _pIndices[J] >= _NumberOfPositions || _pIndices[K] >= _NumberOfPositions)
		{
			oSetLastError(EINVAL, "an index value indexes outside the range of vertices specified");
			return false;
		}

		const TVEC3<T>& a = _pPositions[_pIndices[I]];
		const TVEC3<T>& b = _pPositions[_pIndices[J]];
		const TVEC3<T>& c = _pPositions[_pIndices[K]];

		if (oEqual(cross(a - b, a - c), 0.0f))
		{
			_pIndices[I] = oINVALID;
			_pIndices[J] = oINVALID;
			_pIndices[K] = oINVALID;
		}
	}

	*_pNewNumIndices = _NumberOfIndices;
	for (size_t i = 0; i < *_pNewNumIndices; i++)
	{
		if (_pIndices[i] == oINVALID)
		{
			memcpy(&_pIndices[i], &_pIndices[i+1], sizeof(unsigned int) * (_NumberOfIndices - i - 1));
			i--;
			(*_pNewNumIndices)--;
		}
	}

	return true;
}

bool oRemoveDegenerates(const float3* _pVertices, size_t _NumberOfVertices, unsigned int* _pIndices, size_t _NumberOfIndices, size_t* _pNewNumIndices)
{
	return oRemoveDegeneratesT(_pVertices, _NumberOfVertices, _pIndices, _NumberOfIndices, _pNewNumIndices);
}

template<typename T> static void CalculateFace(size_t index, TVEC3<T>* _pFaceNormals, const unsigned int* _pIndices, size_t _NumberOfIndices, const TVEC3<T>* _pPositions, size_t _NumberOfPositions, T CCWMultiplier, bool* pSuccess)
{
	size_t I = index * 3;
	size_t J = index * 3 + 1;
	size_t K = index * 3 + 2;

	if (_pIndices[I] >= _NumberOfPositions || _pIndices[J] >= _NumberOfPositions || _pIndices[K] >= _NumberOfPositions)
	{
		*pSuccess = false;
		return;
	}

	const TVEC3<T>& a = _pPositions[_pIndices[I]];
	const TVEC3<T>& b = _pPositions[_pIndices[J]];
	const TVEC3<T>& c = _pPositions[_pIndices[K]];

	// gracefully put in a zero vector for degenerate faces
	float3 cr = cross(a - b, a - c);
	_pFaceNormals[index] = oEqual(cr, 0.0f) ? float3(0.0f, 0.0f, 0.0f) : CCWMultiplier * normalize(cr);
}

template<typename T> bool oCalculateFaceNormalsT(TVEC3<T>* _pFaceNormals, const unsigned int* _pIndices, size_t _NumberOfIndices, const TVEC3<T>* _pPositions, size_t _NumberOfPositions, bool _CCW)
{
	if ((_NumberOfIndices % 3) != 0)
	{
		oSetLastError(EINVAL, "_NumberOfIndices must be a multiple of 3");
		return false;
	}

	bool success = true;
	const T s = _CCW ? T(-1.0) : T(1.0);
	oParallelFor( oBIND( &CalculateFace<T>, oBIND1, _pFaceNormals, _pIndices, _NumberOfIndices, _pPositions, _NumberOfPositions, s, &success ), 0, _NumberOfIndices / 3 );
	if( !success )
		oSetLastError(EINVAL, "an index value indexes outside the range of vertices specified");

	return success;
}

bool oCalculateFaceNormals(float3* _pFaceNormals, const unsigned int* _pIndices, size_t _NumberOfIndices, const float3* _pPositions, size_t _NumberOfPositions, bool _CCW)
{
	return oCalculateFaceNormalsT(_pFaceNormals, _pIndices, _NumberOfIndices, _pPositions, _NumberOfPositions, _CCW);
}

template<typename T> bool oCalculateVertexNormalsT(TVEC3<T>* _pVertexNormals, const unsigned int* _pIndices, size_t _NumberOfIndices, const TVEC3<T>* _pPositions, size_t _NumberOfVertices, bool _CCW, bool _OverwriteAll)
{											
	std::vector<TVEC3<T>> faceNormals(_NumberOfIndices / 3, TVEC3<T>(T(0.0), T(0.0), T(0.0)));

	if (!oCalculateFaceNormals(oGetData(faceNormals), _pIndices, _NumberOfIndices, _pPositions, _NumberOfVertices, _CCW))
		return false;

	const size_t nFaces = _NumberOfIndices / 3;

	// for each vertex, store a list of the faces to which it contributes
	std::vector<std::vector<size_t> > trianglesUsedByVertex(_NumberOfVertices);

	for (size_t i = 0; i < nFaces; i++)
	{
		oPushBackUnique(trianglesUsedByVertex[_pIndices[i*3]], i);
		oPushBackUnique(trianglesUsedByVertex[_pIndices[i*3+1]], i);
		oPushBackUnique(trianglesUsedByVertex[_pIndices[i*3+2]], i);
	}

	// Now go through the list and average the normals
	if (_OverwriteAll)
	{
		for (size_t i = 0; i < _NumberOfVertices; i++)
		{
			TVEC3<T> N(T(0.0), T(0.0), T(0.0));
			oFOREACH(size_t faceIndex, trianglesUsedByVertex[i])
				if (!oEqual(dot(faceNormals[faceIndex], faceNormals[faceIndex]), 0.0f))
					N += faceNormals[faceIndex];
		
				_pVertexNormals[i] = normalize(N);
		}
	}

	else
	{
		for (size_t i = 0; i < _NumberOfVertices; i++)
		{
			// If there is length on the data already, leave it alone
			if (oEqual(_pVertexNormals[i], 0.0f))
				continue;

			TVEC3<T> N(T(0.0), T(0.0), T(0.0));
			oFOREACH(size_t faceIndex, trianglesUsedByVertex[i])
				if (!oEqual(dot(faceNormals[faceIndex], faceNormals[faceIndex]), 0.0f))
					N += faceNormals[faceIndex];

			_pVertexNormals[i] = normalize(N);
		}
	}

	return true;
}

bool oCalculateVertexNormals(float3* _pVertexNormals, const unsigned int* _pIndices, size_t _NumberOfIndices, const float3* _pPositions, size_t _NumberOfVertices, bool _CCW, bool _OverwriteAll)
{
	return oCalculateVertexNormalsT(_pVertexNormals, _pIndices, _NumberOfIndices, _pPositions, _NumberOfVertices, _CCW, _OverwriteAll);
}

template<typename T> void oCalculateTangentsT(TVEC4<T>* _pTangents
 , const unsigned int* _pIndices
 , size_t _NumberOfIndices
 , const TVEC3<T>* _pPositions
 , const TVEC3<T>* _pNormals
 , const TVEC2<T>* _pTexcoords
 , size_t _NumberOfVertices)
{
	/** <citation
		usage="Implementation" 
		reason="tangents can be derived, and this is how to do it" 
		author="Eric Lengyel"
		description="http://www.terathon.com/code/tangent.html"
		license="*** Assumed Public Domain ***"
		licenseurl="http://www.terathon.com/code/tangent.html"
		modification="Changes types to oMath types"
	/>*/

	// $(CitedCodeBegin)

	std::vector<TVEC3<T> > tan1(_NumberOfVertices, TVEC3<T>(T(0), T(0), T(0)));
	std::vector<TVEC3<T> > tan2(_NumberOfVertices, TVEC3<T>(T(0), T(0), T(0)));

	const size_t count = _NumberOfIndices / 3;
	for (unsigned int i = 0; i < count; i++)
	{
		const unsigned int a = _pIndices[3*i];
		const unsigned int b = _pIndices[3*i+1];
		const unsigned int c = _pIndices[3*i+2];

		const TVEC3<T>& Pa = _pPositions[a];
		const TVEC3<T>& Pb = _pPositions[b];
		const TVEC3<T>& Pc = _pPositions[c];

		const T x1 = Pb.x - Pa.x;
		const T x2 = Pc.x - Pa.x;
		const T y1 = Pb.y - Pa.y;
		const T y2 = Pc.y - Pa.y;
		const T z1 = Pb.z - Pa.z;
		const T z2 = Pc.z - Pa.z;
        
		const TVEC2<T>& TCa = _pTexcoords[a];
		const TVEC2<T>& TCb = _pTexcoords[b];
		const TVEC2<T>& TCc = _pTexcoords[c];

		const T s1 = TCb.x - TCa.x;
		const T s2 = TCc.x - TCa.x;
		const T t1 = TCb.y - TCa.y;
		const T t2 = TCc.y - TCa.y;

		T r = T(1.0) / (s1 * t2 - s2 * t1);
		TVEC3<T> s((t2 * x1 - t1 * x2) * r, (t2 * y1 - t1 * y2) * r, (t2 * z1 - t1 * z2) * r);
		TVEC3<T> t((s1 * x2 - s2 * x1) * r, (s1 * y2 - s2 * y1) * r, (s1 * z2 - s2 * z1) * r);

		tan1[a] += s;
		tan1[b] += s;
		tan1[c] += s;

		tan2[a] += t;
		tan2[b] += t;
		tan2[c] += t;
	}

	for (unsigned int i = 0; i < _NumberOfVertices; i++)
	{
		// Gram-Schmidt orthogonalize + handedness
		const TVEC3<T>& n = _pNormals[i];
		const TVEC3<T>& t = tan1[i];
		_pTangents[i] = TVEC4<T>(normalize(t - n * dot(n, t)), (dot(static_cast<float3>(cross(n, t)), tan2[i]) < T(0)) ? T(-1.0) : T(1.0));
	}

	// $(CitedCodeEnd)
}

void oCalculateTangents(float4* _pTangents
											 , const unsigned int* _pIndices
											 , size_t _NumberOfIndices
											 , const float3* _pPositions
											 , const float3* _pNormals
											 , const float2* _pTexcoords
											 , size_t _NumberOfVertices)
{
	oCalculateTangentsT(_pTangents, _pIndices, _NumberOfIndices, _pPositions, _pNormals, _pTexcoords, _NumberOfVertices);
}

namespace TerathonEdges {

	/** <citation
		usage="Implementation" 
		reason="tangents can be derived, and this is how to do it" 
		author="Eric Lengyel"
		description="http://www.terathon.com/code/edges.html"
		license="*** Assumed Public Domain ***"
		licenseurl="http://www.terathon.com/code/edges.html"
		modification="Minor changes to not limit algo to 65536 indices and some fixes to get it compiling"
	/>*/

// $(CitedCodeBegin)

// Building an Edge List for an Arbitrary Mesh
// The following code builds a list of edges for an arbitrary triangle 
// mesh and has O(n) running time in the number of triangles n in the 
// pGeometry-> The edgeArray parameter must point to a previously allocated 
// array of Edge structures large enough to hold all of the mesh's 
// edges, which in the worst possible case is 3 times the number of 
// triangles in the pGeometry->

// An edge list is useful for many geometric algorithms in computer 
// graphics. In particular, an edge list is necessary for stencil 
// shadows.

struct Edge
{
    unsigned int      vertexIndex[2]; 
    unsigned int      faceIndex[2];
};


struct Triangle
{
    unsigned int      index[3];
};


long BuildEdges(long vertexCount, long triangleCount,
                const Triangle *triangleArray, Edge *edgeArray)
{
    long maxEdgeCount = triangleCount * 3;
    unsigned int *firstEdge = new unsigned int[vertexCount + maxEdgeCount];
    unsigned int *nextEdge = firstEdge + vertexCount;
    
    for (long a = 0; a < vertexCount; a++) firstEdge[a] = 0xFFFFFFFF;
    
    // First pass over all triangles. This finds all the edges satisfying the
    // condition that the first vertex index is less than the second vertex index
    // when the direction from the first vertex to the second vertex represents
    // a counterclockwise winding around the triangle to which the edge belongs.
    // For each edge found, the edge index is stored in a linked list of edges
    // belonging to the lower-numbered vertex index i. This allows us to quickly
    // find an edge in the second pass whose higher-numbered vertex index is i.
    
    long edgeCount = 0;
    const Triangle *triangle = triangleArray;
    for (long a = 0; a < triangleCount; a++)
    {
        long i1 = triangle->index[2];
        for (long b = 0; b < 3; b++)
        {
            long i2 = triangle->index[b];
            if (i1 < i2)
            {
                Edge *edge = &edgeArray[edgeCount];
                
                edge->vertexIndex[0] = (unsigned int) i1;
                edge->vertexIndex[1] = (unsigned int) i2;
                edge->faceIndex[0] = (unsigned int) a;
                edge->faceIndex[1] = (unsigned int) a;
                
                long edgeIndex = firstEdge[i1];
                if (edgeIndex == 0xFFFFFFFF)
                {
                    firstEdge[i1] = edgeCount;
                }
                else
                {
                    for (;;)
                    {
                        long index = nextEdge[edgeIndex];
                        if (index == 0xFFFFFFFF)
                        {
                            nextEdge[edgeIndex] = edgeCount;
                            break;
                        }
                        
                        edgeIndex = index;
                    }
                }
                
                nextEdge[edgeCount] = 0xFFFFFFFF;
                edgeCount++;
            }
            
            i1 = i2;
        }
        
        triangle++;
    }
    
    // Second pass over all triangles. This finds all the edges satisfying the
    // condition that the first vertex index is greater than the second vertex index
    // when the direction from the first vertex to the second vertex represents
    // a counterclockwise winding around the triangle to which the edge belongs.
    // For each of these edges, the same edge should have already been found in
    // the first pass for a different triangle. So we search the list of edges
    // for the higher-numbered vertex index for the matching edge and fill in the
    // second triangle index. The maximum number of comparisons in this search for
    // any vertex is the number of edges having that vertex as an endpoint.
    
    triangle = triangleArray;
    for (long a = 0; a < triangleCount; a++)
    {
        long i1 = triangle->index[2];
        for (long b = 0; b < 3; b++)
        {
            long i2 = triangle->index[b];
            if (i1 > i2)
            {
                for (long edgeIndex = firstEdge[i2]; edgeIndex != 0xFFFFFFFF;
                        edgeIndex = nextEdge[edgeIndex])
                {
                    Edge *edge = &edgeArray[edgeIndex];
                    if ((edge->vertexIndex[1] == (unsigned int)i1) &&
                            (edge->faceIndex[0] == edge->faceIndex[1]))
                    {
                        edge->faceIndex[1] = (unsigned int) a;
                        break;
                    }
                }
            }
            
            i1 = i2;
        }
        
        triangle++;
    }
    
    delete[] firstEdge;
    return (edgeCount);
}

// $(CitedCodeEnd)

} // namespace TerathonEdges

void oCalculateEdges(size_t _NumberOfVertices, const unsigned int* _pIndices, size_t _NumberOfIndices, unsigned int** _ppEdges, size_t* _pNumberOfEdges)
{
	const size_t numTriangles = _NumberOfIndices / 3;
	oASSERT((size_t)((long)_NumberOfVertices) == _NumberOfVertices, "");
	oASSERT((size_t)((long)numTriangles) == numTriangles, "");

	TerathonEdges::Edge* edgeArray = new TerathonEdges::Edge[3 * numTriangles];

	size_t numEdges = static_cast<size_t>(TerathonEdges::BuildEdges(static_cast<long>(_NumberOfVertices), static_cast<long>(numTriangles), (const TerathonEdges::Triangle *)_pIndices, edgeArray));

	// @oooii-tony: Should the allocator be exposed?
	*_ppEdges = new unsigned int[numEdges * 2];

	for (size_t i = 0; i < numEdges; i++)
	{
		(*_ppEdges)[i*2] = edgeArray[i].vertexIndex[0];
		(*_ppEdges)[i*2+1] = edgeArray[i].vertexIndex[1];
	}

	*_pNumberOfEdges = numEdges;

	delete [] edgeArray;
}

void oFreeEdgeList(unsigned int* _pEdges)
{
	delete [] _pEdges;
}

static void PruneIndices(const std::vector<bool>& _Refed, unsigned int* _pIndices, size_t _NumberOfIndices)
{
	std::vector<unsigned int> sub(_Refed.size(), 0);

	for (unsigned int i = 0; i < _Refed.size(); i++)
		if (!_Refed[i])
			for (unsigned int j = i; j < sub.size(); j++)
				(sub[j])++;

	for (size_t i = 0; i < _NumberOfIndices; i++)
		_pIndices[i] -= sub[_pIndices[i]];
}

template<typename T> static size_t PruneStream(const std::vector<bool>& _Refed, T* _pStream, size_t _NumberOfElements)
{
	if (!_pStream)
		return 0;

	std::vector<bool>::const_iterator itRefed = _Refed.begin();
	T* r = _pStream, *w = _pStream;
	while (itRefed != _Refed.end())
	{
		if (*itRefed++)
			*w++ = *r++;
		else
			++r;
	}

	return _NumberOfElements - (r - w);
}

template<typename T> void oPruneUnindexedVerticesT(unsigned int* _pIndices
																									, size_t _NumberOfIndices
																									, TVEC3<T>* _pPositions
																									, TVEC3<T>* _pNormals
																									, TVEC4<T>* _pTangents
																									, TVEC2<T>* _pTexcoords0
																									, TVEC2<T>* _pTexcoords1
																									, unsigned int* _pColors
																									, size_t _NumberOfVertices
																									, size_t *_pNewNumVertices)
{
	std::vector<bool> refed;
	refed.assign(_NumberOfVertices, false);
	for (size_t i = 0; i < _NumberOfIndices; i++)
		refed[_pIndices[i]] = true;
	size_t newNumVertices = PruneStream(refed, _pPositions, _NumberOfVertices);
	PruneStream(refed, _pNormals, _NumberOfVertices);
	PruneStream(refed, _pTangents, _NumberOfVertices);
	PruneStream(refed, _pTexcoords0, _NumberOfVertices);
	PruneStream(refed, _pTexcoords1, _NumberOfVertices);
	PruneStream(refed, _pColors, _NumberOfVertices);
	PruneIndices(refed, _pIndices, _NumberOfIndices);
	*_pNewNumVertices = newNumVertices;
}

void oPruneUnindexedVertices(unsigned int* _pIndices
														 , size_t _NumberOfIndices
														 , float3* _pPositions
														 , float3* _pNormals
														 , float4* _pTangents
														 , float2* _pTexcoords0
														 , float2* _pTexcoords1
														 , unsigned int* _pColors
														 , size_t _NumberOfVertices
														 , size_t *_pNewNumVertices)
{
	oPruneUnindexedVerticesT(_pIndices, _NumberOfIndices, _pPositions, _pNormals, _pTangents, _pTexcoords0, _pTexcoords1, _pColors, _NumberOfVertices, _pNewNumVertices);
}

template<typename T> inline void oCalculateMinMaxPointsT(const TVEC3<T>* oRESTRICT _pPoints, size_t _NumberOfPoints, TVEC3<T>* oRESTRICT _pMinPoint, TVEC3<T>* oRESTRICT _pMaxPoint)
{
	*_pMinPoint = TVEC3<T>(oNumericLimits<T>::GetMax());
	*_pMaxPoint = TVEC3<T>(oNumericLimits<T>::GetSignedMin());

	for (size_t i = 0; i < _NumberOfPoints; i++)
	{
		*_pMinPoint = oMin(*_pMinPoint, _pPoints[i]);
		*_pMaxPoint = oMax(*_pMaxPoint, _pPoints[i]);
	}
}

void oCalculateMinMaxPoints(const float3* oRESTRICT _pPoints, size_t _NumberOfPoints, float3* oRESTRICT _pMinPoint, float3* oRESTRICT _pMaxPoint)
{
	oCalculateMinMaxPointsT(_pPoints, _NumberOfPoints, _pMinPoint, _pMaxPoint);
}

void oCalculateMinMaxPoints(const double3* oRESTRICT _pPoints, size_t _NumberOfPoints, double3* oRESTRICT _pMinPoint, double3* oRESTRICT _pMaxPoint)
{
	oCalculateMinMaxPointsT(_pPoints, _NumberOfPoints, _pMinPoint, _pMaxPoint);
}

bool oCalculateAreaAndCentriod(float* _pArea, float2* _pCentroid, const float2* _pVertices, size_t _VertexStride, size_t _NumVertices)
{
	// Bashein, Gerard, Detmer, Paul R. "Graphics Gems IV." 
	// ed. Paul S. Heckbert. pg 3-6. Academic Press, San Diego, 1994.

	*_pArea = float(0);
	if (_NumVertices < 3)
		return false;

	float atmp = float(0), xtmp = float(0), ytmp = float(0);
	const float2* vj = _pVertices;
	const float2* vi = oByteAdd(_pVertices, _VertexStride, _NumVertices-1);
	const float2* end = oByteAdd(vi, _VertexStride, 1);
	while (vj < end)
	{
		float ai = vi->x * vj->y - vj->x * vi->y;
		atmp += ai;
		xtmp += (vj->x * vi->x) * ai;
		ytmp += (vj->y * vi->y) * ai;

		vi = vj;
		vj += _VertexStride;
	}

	*_pArea = atmp / 2.0f;
	if (!oEqual(atmp, 0.0f))
	{
		_pCentroid->x = xtmp / (3.0f * atmp);
		_pCentroid->y = ytmp / (3.0f * atmp);
	}

	return true;
}

inline float lengthSquared(const float3& x) { return dot(x,x); }

bool oIntersects(float3* _pIntersection, const float4& _Plane0, const float4& _Plane1, const float4& _Plane2)
{
	// Goldman, Ronald. Intersection of Three Planes. In A. Glassner,
	// ed., Graphics Gems pg 305. Academic Press, Boston, 1991.

	// intersection = (P0.V0)(V1XV2) + (P1.V1)(V2XV0) + (P2.V2)(V0XV1)/Det(V0,V1,V2)
	// Vk is the plane unit normal, Pk is a point on the plane
	// Note that P0 dot V0 is the same as d in abcd form.

	float3 _1X2 = cross(_Plane1.XYZ(), _Plane2.XYZ());
	if (oEqual(lengthSquared(_1X2), 0.0f)) 
		return false;

	float3 _2X0 = cross(_Plane2.XYZ(), _Plane0.XYZ());
	if (oEqual(lengthSquared(_2X0), 0.0f)) 
		return false;

	float3 _0X1 = cross(_Plane0.XYZ(), _Plane1.XYZ());
	if (oEqual(lengthSquared(_0X1), 0.0f)) 
		return false;

	*_pIntersection = (_Plane0.w * _1X2 + _Plane1.w * _2X0 + _Plane2.w * _0X1) / determinant(float3x3(_Plane0.XYZ(), _Plane1.XYZ(), _Plane2.XYZ()));
	return true;
}

bool oIntersects(float3* _pIntersection, const float4& _Plane, const float3& _Point0, const float3& _Point1)
{
	bool intersects = true;
	float d0 = distance(_Plane, _Point0);
	float d1 = distance(_Plane, _Point1);
	bool in0 = 0.0f > d0;
	bool in1 = 0.0f > d1;

	if ((in0 && in1) || (!in0 && !in1)) // totally in or totally out
		intersects = false;
	else // partial
	{
		// the intersection point is along p0,p1, so p(t) = p0 + t(p1 - p0)
		// the intersection point is on the plane, so (p(t) - C) . N = 0
		// with above distance function, C is 0,0,0 and the offset along 
		// the normal is considered. so (pt - c) . N is distance(pt)

		// (p0 + t ( p1 - p0 ) - c) . n = 0
		// p0 . n + t (p1 - p0) . n - c . n = 0
		// t (p1 - p0) . n = c . n - p0 . n
		// ((c - p0) . n) / ((p1 - p0) . n)) 
		//  ^^^^^^^ (-(p0 -c)) . n: this is why -distance

		float3 diff = _Point1 - _Point0;
		float denom = dot(diff, _Plane.XYZ());

		if (fabs(denom) < std::numeric_limits<float>::epsilon())
			return false;

		float t = -distance(_Plane, _Point0) / denom;
		*_pIntersection = _Point0 + t * _Point1;
		intersects = true;
	}

	return intersects;
}

bool oIntersects(const float3& _SphereCenter0, float _Radius0, const float3& _SphereCenter1, float _Radius1)
{
	const float distance2 = dot(_SphereCenter0, _SphereCenter1); // length squared
	float minDistance2 = _Radius0 + _Radius1;
	minDistance2 *= minDistance2;
	return distance2 < minDistance2;
}

template<typename T> static void lookupNP(TVEC3<T>& _N, TVEC3<T>& _P, const TAABOX<T, TVEC3<T>>& _Box, const TVEC4<T>& _Plane)
{
	/** <citation
		usage="Paper" 
		reason="This is part of a reasonably efficient non-SIMD frust cull" 
		author="Daniel Sýkora & Josef Jelínek"
		description="http://www.cescg.org/CESCG-2002/DSykoraJJelinek/index.html"
	/>*/

	#define NP_ASSIGN_COMPONENT(box, planeNormal, component, N, P) \
		if ((planeNormal).component < 0.0f) \
		{	(P).component = box.GetMin().component; \
			(N).component = box.GetMax().component; \
		} else \
		{	(P).component = box.GetMax().component; \
			(N).component = box.GetMin().component; \
		}

	// This is derived from Table 1
	NP_ASSIGN_COMPONENT(_Box, _Plane.XYZ(), x, _N, _P);
	NP_ASSIGN_COMPONENT(_Box, _Plane.XYZ(), y, _N, _P);
	NP_ASSIGN_COMPONENT(_Box, _Plane.XYZ(), z, _N, _P);

	#undef NP_ASSIGN_COMPONENT
}

// Returns -1 if partially in, 0 if totally out, 1 if wholly in
template<typename T> static int oContainsT(const TVEC4<T>* _pPlanes, size_t _NumPlanes, const TAABOX<T, TVEC3<T>>& _Box)
{
	/** <citation
		usage="Paper" 
		reason="A reasonably efficient non-SIMD frust cull" 
		author="Daniel Sýkora & Josef Jelínek"
		description="http://www.cescg.org/CESCG-2002/DSykoraJJelinek/index.html"
	/>*/

	TVEC3<T> n, p;
	int result = 1;
	for (size_t i = 0; i < _NumPlanes; i++)
	{
		const TVEC4<T>& plane = _pPlanes[i];
		lookupNP(n, p, _Box, plane);

		if (sdistance(plane, p) < 0.0f)
			return 0;

		if (sdistance(plane, n) < 0.0f)
			result = -1;
	}

	return result;
}

int oContains(const oFrustumf& _Frustum, const oAABoxf& _Box)
{
	// @oooii-tony: A reasonable optimization might be to set 6 to 5, thus ignoring
	// far plane clipping. When do we limit view distances these days?
	return oContainsT(&_Frustum.Left, 6, _Box);
}

template<typename T> void oFrustCullT(const TFRUSTUM<T>* oRESTRICT _pFrustra, size_t _NumberOfFrusta, const TAABOX<T,TVEC3<T> >* oRESTRICT _pVolumes, size_t _NumberOfVolumes, size_t* _pResults, size_t _MaxNumberOfVolumes, size_t* _pNumResults)
{
	if (!_NumberOfFrusta || !_NumberOfVolumes)
		return;

	oASSERT(_NumberOfVolumes <= _MaxNumberOfVolumes, "");
	for (size_t i = 0; i < _NumberOfFrusta; i++)
	{
		size_t* pFrustumResults = _pResults + (i * _MaxNumberOfVolumes);
		size_t& nResults = _pNumResults[i];
		nResults = 0;

		for (size_t j = 0; j < _NumberOfVolumes; j++)
		{
			if (oContainsT(&_pFrustra[i].Left, 6, _pVolumes[j]))
			{
				pFrustumResults[nResults++] = j;
			}
		}
	}
}

void oFrustCull(const oFrustumf* oRESTRICT _pFrustra, size_t _NumberOfFrusta, const oAABoxf* oRESTRICT _pVolumes, size_t _NumberOfVolumes, size_t* _pResults, size_t _MaxNumberOfVolumes, size_t* _pNumResults)
{
	oFrustCullT(_pFrustra, _NumberOfFrusta, _pVolumes, _NumberOfVolumes, _pResults, _MaxNumberOfVolumes, _pNumResults);
}

template<typename T> static int oContainsT(const TVEC4<T>* _pPlanes, size_t _NumPlanes, const TVEC4<T>& _Sphere)
{
	int result = 1;
	for (size_t i = 0; i < _NumPlanes; i++)
	{
		float sdist = sdistance(_pPlanes[i], _Sphere.XYZ());

		if (sdist < -_Sphere.w)
			return 0;

		if (sdist < _Sphere.w)
			result = -1;
	}

	return result;
}

int oContains(const oFrustumf& _Frustum, const float4& _Sphere)
{
	return oContainsT(&_Frustum.Left, 6, _Sphere);
}

template<typename T> int oContainsT(const TSPHERE<T>& _Sphere, const TAABOX<T,TVEC3<T>>& _Box)
{
	/** <citation
		usage="Paper" 
		reason="Need sphere box collision and this article describes it succinctly" 
		author="James Arvo"
		description=""
		license=""
		licenseurl=""
		modification=""
	/>*/

	T dmin = 0;
	for (size_t i = 0; i < 3; i++)
	{
		if (_Sphere[i] < _Box.GetMin()[i])
		{
			float diff = _Sphere[i] - _Box.GetMin()[i];
			dmin += diff * diff;
		}
		
		else if (_Sphere[i] > _Box.GetMax()[i])
		{
			float diff = _Sphere[i] - _Box.GetMax()[i];
			dmin += diff * diff;
		}
	}

	return dmin <= (_Sphere.w * _Sphere.w) ? 1 : 0;
}

int oContains(const oSpheref& _Sphere, const oAABoxf& _Box)
{
	return oContainsT(_Sphere, _Box);
}

// sscanf doesn't seem to support the f suffix ("1.0f") so use atof

errno_t oFromString(float2* _pValue, const char* _StrSource)
{
	if (!_pValue || !_StrSource) return EINVAL;
	const char* end = 0;
	end = _StrSource + strcspn(_StrSource, " ");
	if (!*end) return EINVAL;
	_pValue->x = static_cast<float>(atof(_StrSource));
	_StrSource = end + 1;
	_pValue->y = static_cast<float>(atof(_StrSource));
	return 0;
}

errno_t oFromString(float3* _pValue, const char* _StrSource)
{
	if (!_pValue || !_StrSource) return EINVAL;
	const char* end = 0;
	end = _StrSource + strcspn(_StrSource, " ");
	if (!*end) return EINVAL;
	_pValue->x = static_cast<float>(atof(_StrSource));
	_StrSource = end + 1;
	end += 1 + strcspn(_StrSource, " ");
	_pValue->y = static_cast<float>(atof(_StrSource));
	_StrSource = end + 1;
	_pValue->z = static_cast<float>(atof(_StrSource));
	return 0;
}

errno_t oFromString(float4* _pValue, const char* _StrSource)
{
	if (!_pValue || !_StrSource) return EINVAL;
	const char* end = 0;
	end = _StrSource + strcspn(_StrSource, " ");
	if (!*end) return EINVAL;
	_pValue->x = static_cast<float>(atof(_StrSource));
	_StrSource = end + 1;
	end += 1 + strcspn(_StrSource, " ");
	_pValue->y = static_cast<float>(atof(_StrSource));
	_StrSource = end + 1;
	end += 1 + strcspn(_StrSource, " ");
	_pValue->z = static_cast<float>(atof(_StrSource));
	_StrSource = end + 1;
	_pValue->w = static_cast<float>(atof(_StrSource));
	return 0;
}

errno_t oFromString(quatf* _pValue, const char* _StrSource)
{
	float4* pTmp = (float4*)_pValue;
	return oFromString(pTmp, _StrSource);
}

errno_t oFromString(float4x4* _pValue, const char* _StrSource)
{
	// Read in-order, then transpose
	const char* end = 0;
	float* f = (float*)_pValue;
	for (size_t i = 0; i < 16; i++)
	{
		end = _StrSource + strcspn(_StrSource, " ");
		f[i] = static_cast<float>(atof(_StrSource));
		_StrSource += strcspn(_StrSource, " ");
	}

	transpose(*_pValue);
	return 0;
}

errno_t oFromString(int2* _pValue, const char* _StrSource)
{
	if (!_pValue || !_StrSource) return EINVAL;
	const char* end = 0;
	end = _StrSource + strcspn(_StrSource, " ");
	if (!*end) return EINVAL;
	_pValue->x = static_cast<int>(atoi(_StrSource));
	_StrSource = end + 1;
	end += 1 + strcspn(_StrSource, " ");
	_pValue->y = static_cast<int>(atoi(_StrSource));
	return 0;
}

errno_t oFromString(int3* _pValue, const char* _StrSource)
{
	if (!_pValue || !_StrSource) return EINVAL;
	const char* end = 0;
	end = _StrSource + strcspn(_StrSource, " ");
	if (!*end) return EINVAL;
	_pValue->x = static_cast<int>(atoi(_StrSource));
	_StrSource = end + 1;
	end += 1 + strcspn(_StrSource, " ");
	_pValue->y = static_cast<int>(atoi(_StrSource));
	_StrSource = end + 1;
	end += 1 + strcspn(_StrSource, " ");
	_pValue->z = static_cast<int>(atoi(_StrSource));
	return 0;
}

errno_t oFromString(int4* _pValue, const char* _StrSource)
{
	if (!_pValue || !_StrSource) return EINVAL;
	const char* end = 0;
	end = _StrSource + strcspn(_StrSource, " ");
	if (!*end) return EINVAL;
	_pValue->x = static_cast<int>(atoi(_StrSource));
	_StrSource = end + 1;
	end += 1 + strcspn(_StrSource, " ");
	_pValue->y = static_cast<int>(atoi(_StrSource));
	_StrSource = end + 1;
	end += 1 + strcspn(_StrSource, " ");
	_pValue->z = static_cast<int>(atoi(_StrSource));
	_StrSource = end + 1;
	_pValue->w = static_cast<int>(atoi(_StrSource));
	return 0;
}

errno_t oFromString(uint2* _pValue, const char* _StrSource)
{
	if (!_pValue || !_StrSource) return EINVAL;
	const char* end = 0;
	end = _StrSource + strcspn(_StrSource, " ");
	if (!*end) return EINVAL;
	_pValue->x = static_cast<uint>(atoi(_StrSource));
	_StrSource = end + 1;
	end += 1 + strcspn(_StrSource, " ");
	_pValue->y = static_cast<uint>(atoi(_StrSource));
	return 0;
}

errno_t oFromString(uint3* _pValue, const char* _StrSource)
{
	if (!_pValue || !_StrSource) return EINVAL;
	const char* end = 0;
	end = _StrSource + strcspn(_StrSource, " ");
	if (!*end) return EINVAL;
	_pValue->x = static_cast<uint>(atoi(_StrSource));
	_StrSource = end + 1;
	end += 1 + strcspn(_StrSource, " ");
	_pValue->y = static_cast<uint>(atoi(_StrSource));
	_StrSource = end + 1;
	end += 1 + strcspn(_StrSource, " ");
	_pValue->z = static_cast<uint>(atoi(_StrSource));
	return 0;
}

errno_t oFromString(uint4* _pValue, const char* _StrSource)
{
	if (!_pValue || !_StrSource) return EINVAL;
	const char* end = 0;
	end = _StrSource + strcspn(_StrSource, " ");
	if (!*end) return EINVAL;
	_pValue->x = static_cast<uint>(atoi(_StrSource));
	_StrSource = end + 1;
	end += 1 + strcspn(_StrSource, " ");
	_pValue->y = static_cast<uint>(atoi(_StrSource));
	_StrSource = end + 1;
	end += 1 + strcspn(_StrSource, " ");
	_pValue->z = static_cast<uint>(atoi(_StrSource));
	_StrSource = end + 1;
	_pValue->w = static_cast<uint>(atoi(_StrSource));
	return 0;
}

errno_t oToString(char* _StrDestination, size_t _SizeofStrDestination, const float2& _Value) { return sprintf_s(_StrDestination, _SizeofStrDestination, "%f %f", _Value.x, _Value.y); }
errno_t oToString(char* _StrDestination, size_t _SizeofStrDestination, const float3& _Value) { return sprintf_s(_StrDestination, _SizeofStrDestination, "%f %f %f", _Value.x, _Value.y, _Value.z); }
errno_t oToString(char* _StrDestination, size_t _SizeofStrDestination, const float4& _Value) { return sprintf_s(_StrDestination, _SizeofStrDestination, "%f %f %f %f", _Value.x, _Value.y, _Value.z, _Value.w); }
errno_t oToString(char* _StrDestination, size_t _SizeofStrDestination, const quatf& _Value) { return sprintf_s(_StrDestination, _SizeofStrDestination, "%f %f %f %f", _Value.x, _Value.y, _Value.z, _Value.w); }
errno_t oToString(char* _StrDestination, size_t _SizeofStrDestination, const int2& _Value) { return sprintf_s(_StrDestination, _SizeofStrDestination, "%d %d", _Value.x, _Value.y); }
errno_t oToString(char* _StrDestination, size_t _SizeofStrDestination, const int3& _Value) { return sprintf_s(_StrDestination, _SizeofStrDestination, "%d %d %d", _Value.x, _Value.y, _Value.z); }
errno_t oToString(char* _StrDestination, size_t _SizeofStrDestination, const int4& _Value) { return sprintf_s(_StrDestination, _SizeofStrDestination, "%d %d %d %d", _Value.x, _Value.y, _Value.z, _Value.w); }
errno_t oToString(char* _StrDestination, size_t _SizeofStrDestination, const uint2& _Value) { return sprintf_s(_StrDestination, _SizeofStrDestination, "%u %u", _Value.x, _Value.y); }
errno_t oToString(char* _StrDestination, size_t _SizeofStrDestination, const uint3& _Value) { return sprintf_s(_StrDestination, _SizeofStrDestination, "%u %u %u", _Value.x, _Value.y, _Value.z); }
errno_t oToString(char* _StrDestination, size_t _SizeofStrDestination, const uint4& _Value) { return sprintf_s(_StrDestination, _SizeofStrDestination, "%u %u %u %u", _Value.x, _Value.y, _Value.z, _Value.w); }

errno_t oToString(char* _StrDestination, size_t _SizeofStrDestination, const float4x4& _Value)
{
	return sprintf_s(_StrDestination, _SizeofStrDestination, "%f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f"
		, _Value.Column0.x, _Value.Column1.x, _Value.Column2.x, _Value.Column3.x
		, _Value.Column0.y, _Value.Column1.y, _Value.Column2.y, _Value.Column3.y
		, _Value.Column0.z, _Value.Column1.z, _Value.Column2.z, _Value.Column3.z
		, _Value.Column0.w, _Value.Column1.w, _Value.Column2.w, _Value.Column3.w);
}

template<typename T>
T oTrilaterateBase( const TVEC3<T> observersIn[4], const T distancesIn[4], TVEC3<T>* position )
{
	oASSERT( position, "oTrilaterate return value is NULL! " );

	typedef TVEC3<T> T_VEC3;
	typedef TVEC4<T> T_VEC4;
	typedef TMAT4<T> T_MAT4;

	T_VEC3 observers[4];
	T distances[4];
	for( int i = 0; i < 4; ++i )
	{
		observers[i].x = observersIn[i].x;
		observers[i].y = observersIn[i].y;
		observers[i].z = observersIn[i].z;
		distances[i] = distancesIn[i];
	}

	T minError = oNumericLimits<float>::GetMax();
	T_VEC3 bestEstimate;

	// We try four times because one point is a tie-breaker and we want the best approximation
	for( int i = 0; i < 4; ++i )
	{
		// Switch which observer is the tie-breaker (index 3)
		T_VEC3 oldZero = observers[0];
		T oldZeroDistance = distances[0];
		for( int j = 0; j < 3; ++j )
		{
			observers[j] = observers[j+1];
			distances[j] = distances[j+1];
		}
		observers[3] = oldZero;
		distances[3] = oldZeroDistance;

		// Translate and rotate all observers such that the 
		// first three observers lie on the z=0 plane this
		// simplifies the system of equations necessary to perform
		// trilateration to the following: 
		// r1sqrd = xsqrd			+ ysqrd			+ zsqrd
		// r2sqrd = ( x - x1 )^2	+ ysqrd			+ zsqrd
		// r3sqrd = ( x - x2 )^2	+ ( y - y2 ) ^2 + zsqrd
		T_MAT4 inverseTransform;
		T_VEC3 transformedObservers[4];
		{
			// Translate everything such that the first point is at the orgin (collapsing the x y and z components)
			T_VEC3 translation = -observers[0];
			T_MAT4 completeTransform = oCreateTranslation( translation );

			for( int i = 0; i < 4; ++i )
				transformedObservers[i] = observers[i] + translation;

			// Rotate everything such that the second point lies on the X-axis (collapsing the y and z components)
			T_MAT4 rot =  oCreateRotation( normalize( transformedObservers[1] ), T_VEC3( 1.0f, 0.0f, 0.0f ), T_VEC3( 0.0f, 0.0f, 1.0f ) ); 
			for( int i = 1; i < 4; ++ i )
				transformedObservers[i] = mul( rot, T_VEC4( transformedObservers[i], 1.0f ) ).XYZ();

			// Add the rotation to our transform
			completeTransform = mul( rot, completeTransform );

			// Rotate everything such that the third point lies on the Z-plane (collapsing the z component )
			const T_VEC3& poi = transformedObservers[2];
			T rad = acos( normalize( T_VEC3( 0.0f, poi.y, poi.z) ).y );
			rad *= poi.z < 0.0f ? 1.0f : -1.0f;

			rot = oCreateRotation( rad, T_VEC3( 1.0f, 0.0f, 0.0f ) );
			for( int j = 1; j < 4; ++ j )
				transformedObservers[j] = mul( rot, T_VEC4( transformedObservers[j], 1.0f ) ).XYZ();

			// Add the rotation to our transform
			completeTransform = mul( rot, completeTransform );

			// Invert so that we can later move back to the original space
			inverseTransform = invert( completeTransform );

			//oASSERT( transformedObservers[1].y < 1.0f && transformedObservers[1].z < 1.0f && transformedObservers[2].z < 1.0f, "Failed to transform to z == 0" );
		}

		// Trilaterate the postion in the transformed space
		T_VEC3 triPos;
		{
			const T x1 = transformedObservers[1][0];
			const T x1sqrd = x1 * x1;
			const T x2 = transformedObservers[2][0];
			const T x2sqrd = x2 * x2;
			const T y2 = transformedObservers[2][1];
			const T y2sqrd = y2 * y2;

			const T r1sqrd = distances[0] * distances[0];
			const T r2sqrd = distances[1] * distances[1];
			const T r3sqrd = distances[2] * distances[2];

			// Solve for x
			T x = ( r1sqrd - r2sqrd + x1sqrd ) / ( 2 * x1 );

			// Solve for y
			T y = ( ( r1sqrd - r3sqrd + x2sqrd + y2sqrd ) / ( 2 * y2 ) ) - ( ( x2 / y2 ) * x );

			// Solve positive Z
			T zsqrd = ( r1sqrd - ( x * x ) - ( y * y ) );
			if( zsqrd < 0.0 )
				continue;

			// Use the fourth point as a tie-breaker
			T posZ = sqrt( zsqrd );
			triPos = T_VEC3( x, y, posZ );
			T posDistToFourth = abs( distance( triPos, transformedObservers[3] ) - distances[3] );
			T negDistToFourth = abs( distance( T_VEC3( triPos.XY(), -triPos[2] ), transformedObservers[3] ) - distances[3]  );
			if( negDistToFourth < posDistToFourth )
				triPos.z = -triPos.z;

			T error = __min( posDistToFourth, negDistToFourth );
			if( error < minError )
			{
				minError = error;
				bestEstimate = mul( inverseTransform, T_VEC4( triPos, 1.0f ) ).XYZ();;
			}
		}

	}

	// Return the trilaterated position in the original space
	*position = bestEstimate;
	return minError;
}
float oTrilaterate( const float3 observersIn[4], const float distancesIn[4], float3* position )
{
	return oTrilaterateBase( observersIn, distancesIn, position );
}
double oTrilaterate( const double3 observersIn[4], const double distancesIn[4], double3* position )
{
	return oTrilaterateBase( observersIn, distancesIn, position );
}

// Helper function for oCoordinateTransform that determines where a position lies in another coordinate system
template<typename T> bool positionInStartCoordSystem( const TVEC3<T> startCoords[4], const TVEC3<T>  endCoords[4], TVEC3<T> endPos, TVEC3<T>* position )
{
	T distances[4];
	for( int i = 0; i < 4; ++i )
		distances[i] = distance( endCoords[i], endPos );

	return oTrilaterate( startCoords, distances, position ) < 10.0f;
}
template<typename T> bool oCoordinateTransformBase( const TVEC3<T> startCoords[4], const TVEC3<T> endCoords[4], TMAT4<T> *matrix )
{
	oASSERT( matrix, "oCoordinateTransform return value is NULL! " );
	TVEC3<T> orgin;
	TVEC3<T> posXAxis;
	TVEC3<T> posYAxis;
	TVEC3<T> posZAxis;

	// Trilaterate the orgin an axis
	if( !positionInStartCoordSystem( startCoords, endCoords, TVEC3<T>( 0.0, 0.0, 0.0 ), &orgin ) )
		return false;

	if( !positionInStartCoordSystem( startCoords, endCoords, TVEC3<T>( 1.0, 0.0, 0.0 ), &posXAxis ) )
		return false;

	if( !positionInStartCoordSystem( startCoords, endCoords, TVEC3<T>( 0.0, 1.0, 0.0 ), &posYAxis ) )
		return false;

	if( !positionInStartCoordSystem( startCoords, endCoords, TVEC3<T>( 0.0, 0.0, 1.0 ), &posZAxis ) )
		return false;

	// Normalize axis
	posXAxis = normalize( posXAxis - orgin );
	posYAxis = normalize( posYAxis - orgin );
	posZAxis = normalize( posZAxis - orgin );

	
	TMAT4<T> transform = TMAT4<T>::Identity;
	transform.Column0 =  TVEC4<T>( posXAxis, 0.0 );  
	transform.Column1 = TVEC4<T>( posYAxis, 0.0 );
	transform.Column2 = TVEC4<T>( posZAxis, 0.0 );

	transform = invert( transform );
	transform = mul( transform, oCreateTranslation( -orgin ) );

	*matrix = transform;
	return true;
}

bool oCoordinateTransform( const float3 startCoords[4], const float3 endCoords[4], float4x4 *matrix )
{
	return oCoordinateTransformBase( startCoords, endCoords, matrix );
}
bool oCoordinateTransform( const double3 startCoords[4], const double3 endCoords[4], double4x4 *matrix )
{
	return oCoordinateTransformBase( startCoords, endCoords, matrix );
}

unsigned int SplitRect( const oRECT& _SrcRect, const unsigned int _MaxNumSplits, const float* _pOrderedSplitRatio, const unsigned int _XMultiple, const unsigned int _YMultiple, oRECT* _pSplitResults )
{
	typedef int T;
	typedef TVEC2<T> T_VEC;

	T_VEC Dimensions = _SrcRect.GetDimensions();

	// Split along the larger axis
	bool SplitHorizontally = Dimensions.x > Dimensions.y;
	T SplitMultiple = SplitHorizontally ? _XMultiple : _YMultiple;
	T SplitCount = SplitHorizontally ? ( ( Dimensions.x ) / _XMultiple ) : ( ( Dimensions.y ) / _YMultiple );

	T HorizSplit = 0;
	T VeritcalSplit = 0;
	unsigned int i = 0;
	for(;i < _MaxNumSplits; ++i )
	{
		T SplitAmount		= SplitMultiple * static_cast<T>( _pOrderedSplitRatio[i] * SplitCount );
		T HorizAmount		= SplitHorizontally ? SplitAmount : Dimensions.x;
		T VerticalAmount	= SplitHorizontally ? Dimensions.y : SplitAmount;

		oRECT& rect = _pSplitResults[i];
		rect.SetMin( T_VEC( HorizSplit, VeritcalSplit ) );
		rect.SetMax( T_VEC( HorizSplit + HorizAmount, VeritcalSplit + VerticalAmount ) );
		HorizSplit += SplitHorizontally ? ( HorizAmount ) : 0;
		VeritcalSplit += SplitHorizontally ? 0 : ( VerticalAmount  );

		T_VEC RMax = rect.GetMax();
		if( RMax.x > Dimensions.x || RMax.y > Dimensions.y )
		{
			T_VEC minDim = oMin( Dimensions, RMax );
			rect.SetMax( oMin( Dimensions, RMax ) );
			return i + 1;
		}
	}

	// Give any remainder to the last split
	_pSplitResults[i-1].SetMax( Dimensions );

	return _MaxNumSplits;
}
