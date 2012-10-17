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
#include <oBasis/oMath.h>
#include <oBasis/oAssert.h>
#include <oBasis/oAtof.h>
#include <oBasis/oByte.h>
#include <oBasis/oError.h>
#include <oBasis/oLimits.h>
#include <oBasis/oMemory.h>
#include <oBasis/oString.h>
#include <vectormath/scalar/cpp/vectormath_aos.h>
#include <vectormath/scalar/cpp/vectormath_aos_d.h>
#include <cstring>
#include "perlin.h"

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

// @oooii-tony: Trade-off. Though we have a process-wide singleton, this entire
// module is platform-independent but for using oProcessSingleton. To reduce
// dependencies, the choice was made to duplicate these contents across any DLLs
// This is mathematically fine because the Perlin noise is singularly 
// initialized, so all instances of the Singleton will evaluate to the same 
// outputs given the same inputs. The penalty however is a few kilobytes of code
// space, which irks me as a console programmer from the GBA/PS1 days, but I
// guess I should embrace the 16 GB of memory on my PC for now.
struct oPerlinContext
{
	static oPerlinContext* Singleton() { static oPerlinContext PCtx; return &PCtx; } // @oooii-tony: Do not convert to oSingleton without a code review/approval

	oPerlinContext()
		: P(4,4,1,94)
	{}

	Perlin P;
};

float noise(float x) { return oPerlinContext::Singleton()->P.Get(x); }
float noise(const TVEC2<float>& x) { return oPerlinContext::Singleton()->P.Get(x.x, x.y); }
float noise(const TVEC3<float>& x) { return oPerlinContext::Singleton()->P.Get(x.x, x.y, x.z); }
float noise(const TVEC4<float>& x)  { return oPerlinContext::Singleton()->P.Get(x.x, x.y, x.z); } // @oooii-tony: not yet implemented for 4th parameter

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

quatf slerp(const quatf& a, const quatf& b, float s)
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

float4x4 oCreateRotation(const float3& _CurrentVector, const float3& _DesiredVector, const float3& _DefaultRotationAxis)
{
	float3 x;

	float a = angle(_CurrentVector, _DesiredVector);
	if (oEqual(a, 0.0f))
		return float4x4(float4x4::Identity);
	
	if (_CurrentVector == _DesiredVector)
		return float4x4(float4x4::Identity);

	else if (-_CurrentVector == _DesiredVector)
		x = _DefaultRotationAxis;

	else
	{
		x = cross(_CurrentVector, _DesiredVector);
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

quatf oCreateRotationQ(const float3& _CurrentVector, const float3& _DesiredVector)
{
	Quat q = Quat::rotation((const Vector3&)_CurrentVector, (const Vector3&)_DesiredVector);
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
	*_pXAxis = _Matrix.Column0.xyz();
	*_pYAxis = _Matrix.Column1.xyz();
	*_pZAxis = _Matrix.Column2.xyz();
}

void oExtractAxes(const double4x4& _Matrix, double3* _pXAxis, double3* _pYAxis, double3* _pZAxis)
{
	*_pXAxis = _Matrix.Column0.xyz();
	*_pYAxis = _Matrix.Column1.xyz();
	*_pZAxis = _Matrix.Column2.xyz();
}

double4x4 oCreateRotation(const double3& _Radians)
{
	Matrix4d m = Matrix4d::rotationZYX((const Vector3d&)_Radians);
	return (double4x4&)m;
}

double4x4 oCreateRotation(const double &_Radians, const double3& _NormalizedRotationAxis)
{
	Matrix4d m = Matrix4d::rotation(_Radians, (const Vector3d&)_NormalizedRotationAxis);
	return (double4x4&)m;
}

double4x4 oCreateRotation(const double3& _CurrentVector, const double3& _DesiredVector, const double3& _DefaultRotationAxis)
{
	double3 x;

	double a = angle(_CurrentVector, _DesiredVector);
	if (oEqual(a, 0.0))
		return double4x4(double4x4::Identity);
	
	if (_CurrentVector == _DesiredVector)
		return double4x4(double4x4::Identity);

	else if (-_CurrentVector == _DesiredVector)
		x = _DefaultRotationAxis;

	else
	{
		x = cross(_CurrentVector, _DesiredVector);
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

quatd oCreateRotationQ(const double3& _CurrentVector, const double3& _DesiredVector)
{
	Quatd q = Quatd::rotation((const Vector3d&)_CurrentVector, (const Vector3d&)_DesiredVector);
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
		float4(2.0f * A * A + 1.0f, 2.0f * B * A,        2.0f * C * A, 0.0f),
		float4(2.0f * A * B,        2.0f * B * B + 1.0f, 2.0f * C * B, 0.0f),
		float4(2.0f * A * C,        2.0f * B * C,        2.0f * C * C + 1.0f, 0.0f),
		float4(2.0f * A * D,        2.0f * B * D,        2.0f * C * D, 1.0f));
}

void oExtractLookAt(const float4x4& _View, float3* _pEye, float3* _pAt, float3* _pUp, float3* _pRight)
{
	*_pEye = invert(_View).Column3.xyz();
	_pRight->x = _View[0][0]; _pUp->x = _View[0][1]; _pAt->x = -_View[0][2];
	_pRight->y = _View[1][0]; _pUp->y = _View[1][1]; _pAt->y = -_View[1][2];
	_pRight->z = _View[2][0]; _pUp->z = _View[2][1]; _pAt->z = -_View[2][2];
}

void oCalcPlaneMatrix(const float4& _Plane, float4x4* _pMatrix)
{
	*_pMatrix = oCreateRotation(float3(0.0f, 0.0f, 1.0f), _Plane.xyz(), float3(0.0f, 1.0f, 0.0f));
	float3 offset(0.0f, 0.0f, _Plane.w);  // @oooii@doug need to verify sign change
	offset = _pMatrix->GetUpper3x3() * offset;
	_pMatrix->Column3.x = offset.x;
	_pMatrix->Column3.y = offset.y;
	_pMatrix->Column3.z = offset.z;
}

const char* oAsString(oFrustumf::CORNER _Corner)
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
		oNODEFAULT;
	}
}

enum PLANE_TYPES
{
	LEFT_PLANE,
	RIGHT_PLANE,
	TOP_PLANE,
	BOTTOM_PLANE,
	NEAR_PLANE,
	FAR_PLANE
};

template<typename T> void oExtractFrustumPlanesT(TVEC4<T> _Planes[6], const TMAT4<T>& _Projection, bool _Normalize)
{
	/** <citation
		usage="Adaptation" 
		reason="Simple straightforward paper with code to do this important conversion." 
		author="Gil Gribb & Klaus Hartmann"
		description="http://www2.ravensoft.com/users/ggribb/plane%20extraction.pdf"
		description2="http://crazyjoke.free.fr/doc/3D/plane%20extraction.pdf"
    license="*** Assumed Public Domain ***"
		licenseurl="http://www2.ravensoft.com/users/ggribb/plane%20extraction.pdf"
		modification="negate the w value so that -offsets are opposite the normal and add support for orthographic projections"
	/>*/

	// $(CitedCodeBegin)

	// Left clipping plane
	_Planes[LEFT_PLANE].x = _Projection[0][3] + _Projection[0][0];
	_Planes[LEFT_PLANE].y = _Projection[1][3] + _Projection[1][0];
	_Planes[LEFT_PLANE].z = _Projection[2][3] + _Projection[2][0];
	_Planes[LEFT_PLANE].w = _Projection[3][3] + _Projection[3][0];

	// Right clipping plane
	_Planes[RIGHT_PLANE].x = _Projection[0][3] - _Projection[0][0];
	_Planes[RIGHT_PLANE].y = _Projection[1][3] - _Projection[1][0];
	_Planes[RIGHT_PLANE].z = _Projection[2][3] - _Projection[2][0];
	_Planes[RIGHT_PLANE].w = _Projection[3][3] - _Projection[3][0];

	// Top clipping plane
	_Planes[TOP_PLANE].x = _Projection[0][3] - _Projection[0][1];
	_Planes[TOP_PLANE].y = _Projection[1][3] - _Projection[1][1];
	_Planes[TOP_PLANE].z = _Projection[2][3] - _Projection[2][1];
	_Planes[TOP_PLANE].w = _Projection[3][3] - _Projection[3][1];

	// Bottom clipping plane
	_Planes[BOTTOM_PLANE].x = _Projection[0][3] + _Projection[0][1];
	_Planes[BOTTOM_PLANE].y = _Projection[1][3] + _Projection[1][1];
	_Planes[BOTTOM_PLANE].z = _Projection[2][3] + _Projection[2][1];
	_Planes[BOTTOM_PLANE].w = _Projection[3][3] + _Projection[3][1];

	// Near clipping plane
	if (oHasPerspective(_Projection))
	{
		_Planes[NEAR_PLANE].x = _Projection[0][2];
		_Planes[NEAR_PLANE].y = _Projection[1][2];
		_Planes[NEAR_PLANE].z = _Projection[2][2];
		_Planes[NEAR_PLANE].w = _Projection[3][2];
	}

	else
	{
		_Planes[NEAR_PLANE].x = _Projection[0][3] + _Projection[0][2];
		_Planes[NEAR_PLANE].y = _Projection[1][3] + _Projection[1][2];
		_Planes[NEAR_PLANE].z = _Projection[2][3] + _Projection[2][2];
		_Planes[NEAR_PLANE].w = _Projection[3][3] + _Projection[3][2];
	}

	// Far clipping plane
	_Planes[FAR_PLANE].x = _Projection[0][3] - _Projection[0][2];
	_Planes[FAR_PLANE].y = _Projection[1][3] - _Projection[1][2];
	_Planes[FAR_PLANE].z = _Projection[2][3] - _Projection[2][2];
	_Planes[FAR_PLANE].w = _Projection[3][3] - _Projection[3][2];

  if (_Normalize)
    for (int i = 0; i < 6; i++)
      _Planes[i] = oNormalizePlane(_Planes[i]);

	// $(CitedCodeEnd)
}

void oExtractFrustumPlanes(float4 _Planes[6], const float4x4& _Projection, bool _Normalize)
{
	oExtractFrustumPlanesT(_Planes, _Projection, _Normalize);
}

template<typename T> bool oExtractFrustumCornersT(TVEC3<T> _Corners[8], const TFRUSTUM<T>& _Frustum)
{
	// @oooii-tony: TODO implement oIntersects for double
	bool isect = oIntersects(&_Corners[TFRUSTUM<T>::LEFT_TOP_NEAR], _Frustum.Left, _Frustum.Top, _Frustum.Near);
	isect = isect && oIntersects(&_Corners[TFRUSTUM<T>::LEFT_TOP_FAR], _Frustum.Left, _Frustum.Top, _Frustum.Far);
	isect = isect && oIntersects(&_Corners[TFRUSTUM<T>::LEFT_BOTTOM_NEAR], _Frustum.Left, _Frustum.Bottom, _Frustum.Near);
	isect = isect && oIntersects(&_Corners[TFRUSTUM<T>::LEFT_BOTTOM_FAR], _Frustum.Left, _Frustum.Bottom, _Frustum.Far);
	isect = isect && oIntersects(&_Corners[TFRUSTUM<T>::RIGHT_TOP_NEAR], _Frustum.Right, _Frustum.Top, _Frustum.Near);
	isect = isect && oIntersects(&_Corners[TFRUSTUM<T>::RIGHT_TOP_FAR], _Frustum.Right, _Frustum.Top, _Frustum.Far);
	isect = isect && oIntersects(&_Corners[TFRUSTUM<T>::RIGHT_BOTTOM_NEAR], _Frustum.Right, _Frustum.Bottom, _Frustum.Near);
	isect = isect && oIntersects(&_Corners[TFRUSTUM<T>::RIGHT_BOTTOM_FAR], _Frustum.Right, _Frustum.Bottom, _Frustum.Far);
	return isect;
}

bool oExtractFrustumCorners(float3 _Corners[8], const oFrustumf& _Frustum)
{
	return oExtractFrustumCornersT(_Corners, _Frustum);
}

template<typename T> void oCalculateNearInverseFarPlanesDistance(const TMAT4<T>& _Projection, T* _pNearDistance, T* _pInverseFarDistance)
{
  T C = _Projection[2][2];
  T D = _Projection[3][2];

  if (oEqual(C, T(0)))
    *_pNearDistance = T(0);
  else
    *_pNearDistance = (-D / C);

  if (oHasPerspective(_Projection))
  {
	  if (oEqual(C, T(1)) || oEqual(D, T(0)))
		  *_pInverseFarDistance = fabs(D);  
	  else
		  *_pInverseFarDistance = (T(1) - C) / D;
  }
  else
  {
	  *_pInverseFarDistance = C / (T(1) + C * (*_pNearDistance));
  }
}

void oCalculateNearInverseFarPlanesDistance(const float4x4& _Projection, float* _pNearDistance, float* _pInverseFarDistance)
{
	return oCalculateNearInverseFarPlanesDistance<float>(_Projection, _pNearDistance, _pInverseFarDistance);
}

void oCalculateNearInverseFarPlanesDistance(const double4x4& _Projection, double* _pNearDistance, double* _pInverseFarDistance)
{
	return oCalculateNearInverseFarPlanesDistance<double>(_Projection, _pNearDistance, _pInverseFarDistance);
}

template<typename T> void oExtractPerspectiveParameters(const TMAT4<T>& _Projection, T* _pFovYRadians, T* _pAspectRatio, T* _pZNear, T* _pZFar)
{
	*_pFovYRadians = atan(T(1.0) / _Projection.Column1.y) * T(2.0);
	*_pAspectRatio = _Projection.Column1.y / _Projection.Column0.x;
	oCalculateNearInverseFarPlanesDistance(_Projection, _pZNear, _pZFar);
	*_pZFar = 1.0f / *_pZFar;
}

void oExtractPerspectiveParameters(const float4x4& _Projection, float* _pFovYRadians, float* _pAspectRatio, float* _pZNear, float* _pZFar)
{
	oExtractPerspectiveParameters<float>(_Projection, _pFovYRadians, _pAspectRatio, _pZNear, _pZFar);
}

void oExtractPerspectiveParameters(const double4x4& _Projection, double* _pFovYRadians, double* _pAspectRatio, double* _pZNear, double* _pZFar)
{
	oExtractPerspectiveParameters<double>(_Projection, _pFovYRadians, _pAspectRatio, _pZNear, _pZFar);
}

template<typename T> static TMAT4<T> oFitToViewT(const TMAT4<T>& _View, const TMAT4<T>& _Projection, const TSPHERE<T>& _Bounds, T _OffsetMultiplier)
{
	TMAT4<T> invView = invert(_View);
	T FovYRadians, AspectRatio, ZNear, ZFar;
	oExtractPerspectiveParameters(_Projection, &FovYRadians, &AspectRatio, &ZNear, &ZFar);
	T Offset = -_OffsetMultiplier * _Bounds.GetRadius() / tan(FovYRadians / T(2.0));
	TVEC3<T> P = _Bounds.GetPosition() + mul(invView.GetUpper3x3(), TVEC3<T>(T(0.0), T(0.0), Offset));
	invView.Column3 = TVEC4<T>(P, invView.Column3.w);
	return invert(invView);
}

float4x4 oFitToView(const float4x4& _View, const float4x4& _Projection, const oSpheref& _Bounds, float _OffsetMultiplier = 1.0f)
{
	return oFitToViewT<float>(_View, _Projection, _Bounds, _OffsetMultiplier);
}

double4x4 oFitToView(const double4x4& _View, const double4x4& _Projection, const oSphered& _Bounds, double _OffsetMultiplier = 1.0)
{
	return oFitToViewT<double>(_View, _Projection, _Bounds, _OffsetMultiplier);
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

  // http://paulbourke.net/geometry/3planes/

  // check that there is a valid cross product
	float3 P1X2 = cross(_Plane1.xyz(), _Plane2.xyz());
	if (oEqual(lengthSquared(P1X2), 0.0f)) 
		return false;

	float3 P2X0 = cross(_Plane2.xyz(), _Plane0.xyz());
	if (oEqual(lengthSquared(P2X0), 0.0f)) 
		return false;

	float3 P0X1 = cross(_Plane0.xyz(), _Plane1.xyz());
	if (oEqual(lengthSquared(P0X1), 0.0f)) 
		return false;

	*_pIntersection = (-_Plane0.w * P1X2 - _Plane1.w * P2X0 - _Plane2.w * P0X1) / determinant(float3x3(_Plane0.xyz(), _Plane1.xyz(), _Plane2.xyz()));
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
		float denom = dot(diff, _Plane.xyz());

		if (fabs(denom) < oNumericLimits<float>::GetEpsilon())
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
	NP_ASSIGN_COMPONENT(_Box, _Plane.xyz(), x, _N, _P);
	NP_ASSIGN_COMPONENT(_Box, _Plane.xyz(), y, _N, _P);
	NP_ASSIGN_COMPONENT(_Box, _Plane.xyz(), z, _N, _P);

	#undef NP_ASSIGN_COMPONENT
}

template<typename T> static oCONTAINMENT oContainsT(const TVEC4<T>* _pPlanes, size_t _NumPlanes, const TAABOX<T, TVEC3<T>>& _Box)
{
	/** <citation
		usage="Paper" 
		reason="A reasonably efficient non-SIMD frust cull" 
		author="Daniel Sýkora & Josef Jelínek"
		description="http://www.cescg.org/CESCG-2002/DSykoraJJelinek/index.html"
	/>*/

#if 0
	TVEC3<T> n, p;
	oCONTAINMENT result = oWHOLLY_CONTAINED;
	for (size_t i = 0; i < _NumPlanes; i++)
	{
		const TVEC4<T>& plane = _pPlanes[i];
		lookupNP(n, p, _Box, plane);

		if (sdistance(plane, p) < 0.0f)
			return oNOT_CONTAINED;

		if (sdistance(plane, n) < 0.0f)
			result = oPARTIALLY_CONTAINED;
	}

	return result;
#endif
 
  TVEC3<T> vCorner[8];
	int totalInFront = 0;

	// get the corners of the box into the vCorner array
	_Box.GetVertices(vCorner);

	// test all 8 corners against the 6 sides 
	// if all points are behind 1 specific plane, we are out
	// if we are in with all points, then we are fully in
	for(unsigned int p = 0; p < _NumPlanes; ++p) 
  {
		int behindPlaneInCount = 0;
		int allInFrontOfPlane = 1;

    const TVEC4<T> & plane = _pPlanes[p];

		for(int i = 0; i < 8; ++i) 
    {
      TVEC3<T> & pt = vCorner[i];

			// test this point against the planes
      if (sdistance(plane, pt) < 0.0f)
      {
			  allInFrontOfPlane = 0;
				behindPlaneInCount++;
			}
		}

		// were all the points behind plane p?
		//if (behindPlaneInCount == 8)
		//	return oNOT_CONTAINED;

		// check if they were all on the right side of the plane
		totalInFront += allInFrontOfPlane;
	}

	// so if iTotalIn is 6, then all are inside the view
	if (totalInFront == 6)
		return oWHOLLY_CONTAINED;

	return oPARTIALLY_CONTAINED;
}


oCONTAINMENT oContains(const oRECT& _Rect0, const oRECT& _Rect1)
{
	if (_Rect0.GetMin().x > _Rect1.GetMax().x
		|| _Rect0.GetMax().x < _Rect1.GetMin().x
		|| _Rect0.GetMin().y > _Rect1.GetMax().y
		|| _Rect0.GetMax().y < _Rect1.GetMin().y)
		return oNOT_CONTAINED;

	if (greater_than_equal(_Rect1.GetMin(), _Rect0.GetMin()) && less_than_equal(_Rect1.GetMax(), _Rect0.GetMax()))
		return oWHOLLY_CONTAINED;

	return oPARTIALLY_CONTAINED;
}

oCONTAINMENT oContains(const oAABoxf& _Box0, const oAABoxf& _Box1)
{
	if (_Box0.GetMin().x > _Box1.GetMax().x
		|| _Box0.GetMax().x < _Box1.GetMin().x
		|| _Box0.GetMin().y > _Box1.GetMax().y
		|| _Box0.GetMax().y < _Box1.GetMin().y
		|| _Box0.GetMin().z > _Box1.GetMax().z
		|| _Box0.GetMax().z < _Box1.GetMax().z)
		return oNOT_CONTAINED;

	if (greater_than_equal(_Box1.GetMin(), _Box0.GetMin()) && less_than_equal(_Box1.GetMax(), _Box0.GetMax()))
		return oWHOLLY_CONTAINED;

	return oPARTIALLY_CONTAINED;
}

oCONTAINMENT oContains(float3 _Point, const oAABoxf& _Box)
{
	float3 distance = _Point - _Box.GetCenter();
	bool bInX = abs(distance.x) < _Box.GetDimensions().x / 2.f;
	bool bInY = abs(distance.y) < _Box.GetDimensions().y / 2.f;
	bool bInZ = abs(distance.z) < _Box.GetDimensions().z / 2.f;

	return (bInX && bInY && bInZ) ? oWHOLLY_CONTAINED : oNOT_CONTAINED;
}

oCONTAINMENT oContains(const oFrustumf& _Frustum, const oAABoxf& _Box)
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

template<typename T> static oCONTAINMENT oContainsT(const TVEC4<T>* _pPlanes, size_t _NumPlanes, const TVEC4<T>& _Sphere)
{
	oCONTAINMENT result = oWHOLLY_CONTAINED;
	for (size_t i = 0; i < _NumPlanes; i++)
	{
		float sdist = sdistance(_pPlanes[i], _Sphere.xyz());

		if (sdist < -_Sphere.w)
			return oNOT_CONTAINED;

		if (sdist < _Sphere.w)
			result = oPARTIALLY_CONTAINED;
	}

	return result;
}

oCONTAINMENT oContains(const oFrustumf& _Frustum, const float4& _Sphere)
{
	return oContainsT(&_Frustum.Left, 6, _Sphere);
}

template<typename T> oCONTAINMENT oContainsT(const TSPHERE<T>& _Sphere, const TAABOX<T,TVEC3<T>>& _Box)
{
	/** <citation
		usage="Paper" 
		reason="Need sphere box collision and this article describes it succinctly" 
		author="James Arvo"
		description="Graphics Gems, Academic Press, 1990"
		license=""
		licenseurl=""
		modification=""
	/>*/

	const float radiusSq = _Sphere.GetRadius() * _Sphere.GetRadius();

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

	if (dmin > radiusSq)
		return oNOT_CONTAINED;

	const float minSq = dot(_Box.GetMin(), _Box.GetMin());
	const float maxSq = dot(_Box.GetMax(), _Box.GetMax());

	if (minSq <= radiusSq && maxSq <= radiusSq)
		return oWHOLLY_CONTAINED;

	return oPARTIALLY_CONTAINED;
}

int oContains(const oSpheref& _Sphere, const oAABoxf& _Box)
{
	return oContainsT(_Sphere, _Box);
}

// sscanf doesn't seem to support the f suffix ("1.0f") so use atof

bool oFromString(float2* _pValue, const char* _StrSource)
{
	if (!_pValue || !_StrSource) return false;
	_StrSource += strcspn(_StrSource, "0123456789+-.");
	if (!*_StrSource) return false;

	const char* end = 0;
	end = _StrSource + strcspn(_StrSource, " ");
	if (!*end) return false;

	if (!oAtof(_StrSource, &_pValue->x)) return false;
	_StrSource = end + 1;
	if (!oAtof(_StrSource, &_pValue->y)) return false;
	return true;
}

bool oFromString(float3* _pValue, const char* _StrSource)
{
	if (!_pValue || !_StrSource) return false;
	_StrSource += strcspn(_StrSource, "0123456789+-.");
	if (!*_StrSource) return false;

	const char* end = 0;
	end = _StrSource + strcspn(_StrSource, " ");
	if (!*end) return false;
	if (!oAtof(_StrSource, &_pValue->x)) return false;
	_StrSource = end + 1;
	end += 1 + strcspn(_StrSource, " ");
	if (!oAtof(_StrSource, &_pValue->y)) return false;
	_StrSource = end + 1;
	if (!oAtof(_StrSource, &_pValue->z)) return false;
	return true;
}

bool oFromString(float4* _pValue, const char* _StrSource)
{
	if (!_pValue || !_StrSource) return false;
	_StrSource += strcspn(_StrSource, "0123456789+-.");
	if (!*_StrSource) return false;

	const char* end = 0;
	end = _StrSource + strcspn(_StrSource, " ");
	if (!*end) return false;
	if (!oAtof(_StrSource, &_pValue->x)) return false;
	_StrSource = end + 1;
	end += 1 + strcspn(_StrSource, " ");
	if (!oAtof(_StrSource, &_pValue->y)) return false;
	_StrSource = end + 1;
	end += 1 + strcspn(_StrSource, " ");
	if (!oAtof(_StrSource, &_pValue->z)) return false;
	_StrSource = end + 1;
	if (!oAtof(_StrSource, &_pValue->w)) return false;
	return true;
}

bool oFromString(quatf* _pValue, const char* _StrSource)
{
	float4* pTmp = (float4*)_pValue;
	return oFromString(pTmp, _StrSource);
}

bool oFromString(float4x4* _pValue, const char* _StrSource)
{
	_StrSource += strcspn(_StrSource, "0123456789+-.");
	if (!*_StrSource) return false;

	// Read in-order, then transpose
	const char* end = 0;
	float* f = (float*)_pValue;
	for (size_t i = 0; i < 16; i++)
	{
		end = _StrSource + strcspn(_StrSource, " ");
		if (!oAtof(_StrSource, &f[i])) return false;
		_StrSource = end + 1;
	}

	transpose(*_pValue);
	return true;
}

#include <stdio.h>

#define VALIDATE_AND_MOVE_TO_INT() do \
{	if (!_pValue || !_StrSource) return false; \
	_StrSource += strcspn(_StrSource, "0123456789+-"); \
	if (!*_StrSource) return false; \
} while (false)

#define VALIDATE_AND_MOVE_TO_UINT() do \
{	if (!_pValue || !_StrSource) return false; \
	_StrSource += strcspn(_StrSource, "0123456789+"); \
	if (!*_StrSource) return false; \
} while (false)

bool oFromString(int2* _pValue, const char* _StrSource)
{
	VALIDATE_AND_MOVE_TO_INT();
	return 2 == sscanf_s(_StrSource, "%d %d", &_pValue->x, &_pValue->y);
}

bool oFromString(int3* _pValue, const char* _StrSource)
{
	VALIDATE_AND_MOVE_TO_INT();
	return 3 == sscanf_s(_StrSource, "%d %d %d", &_pValue->x, &_pValue->y, &_pValue->z);
}

bool oFromString(int4* _pValue, const char* _StrSource)
{
	VALIDATE_AND_MOVE_TO_INT();
	return 4 == sscanf_s(_StrSource, "%d %d %d %d", &_pValue->x, &_pValue->y, &_pValue->z, &_pValue->w);
}

bool oFromString(uint2* _pValue, const char* _StrSource)
{
	VALIDATE_AND_MOVE_TO_UINT();
	return 2 == sscanf_s(_StrSource, "%u %u", &_pValue->x, &_pValue->y);
}

bool oFromString(uint3* _pValue, const char* _StrSource)
{
	VALIDATE_AND_MOVE_TO_UINT();
	return 3 == sscanf_s(_StrSource, "%u %u %u", &_pValue->x, &_pValue->y, &_pValue->z);
}

bool oFromString(uint4* _pValue, const char* _StrSource)
{
	VALIDATE_AND_MOVE_TO_INT();
	return 4 == sscanf_s(_StrSource, "%u %u %u %u", &_pValue->x, &_pValue->y, &_pValue->z, &_pValue->w);
}

bool oFromString(oRECT* _pValue, const char* _StrSource)
{
	if (!_pValue) return false;
	int4 temp;
	if (!oFromString(&temp, _StrSource))
		return false;
	_pValue->SetMin(int2(temp.x, temp.y));
	_pValue->SetMax(int2(temp.z, temp.w));
	return true;
}

bool oFromString(oRGBf* _pValue, const char* _StrSource)
{
	oColor c;
	// Valid forms are: 0xAABBGGRR, R G B [0,1], and a std::color

	if (*_StrSource == '0' && tolower(*_StrSource) == 'x')
	{
		unsigned int i;
		if (oFromString(&i, _StrSource))
			*_pValue = *(oColor*)&i;
	}

	else if (oFromString(&c, _StrSource))
		*_pValue = c;

	else if (!oFromString((float3*)_pValue, _StrSource))
		return false;

	return true;
}

char* oToString(char* _StrDestination, size_t _SizeofStrDestination, const float2& _Value) { return -1 != oPrintf(_StrDestination, _SizeofStrDestination, "%f %f", _Value.x, _Value.y) ? _StrDestination : nullptr; }
char* oToString(char* _StrDestination, size_t _SizeofStrDestination, const float3& _Value) { return -1 != oPrintf(_StrDestination, _SizeofStrDestination, "%f %f %f", _Value.x, _Value.y, _Value.z) ? _StrDestination : nullptr; }
char* oToString(char* _StrDestination, size_t _SizeofStrDestination, const float4& _Value) { return -1 != oPrintf(_StrDestination, _SizeofStrDestination, "%f %f %f %f", _Value.x, _Value.y, _Value.z, _Value.w) ? _StrDestination : nullptr; }
char* oToString(char* _StrDestination, size_t _SizeofStrDestination, const quatf& _Value) { return -1 != oPrintf(_StrDestination, _SizeofStrDestination, "%f %f %f %f", _Value.x, _Value.y, _Value.z, _Value.w) ? _StrDestination : nullptr; }
char* oToString(char* _StrDestination, size_t _SizeofStrDestination, const double2& _Value) { return -1 != oPrintf(_StrDestination, _SizeofStrDestination, "%f %f", _Value.x, _Value.y) ? _StrDestination : nullptr; }
char* oToString(char* _StrDestination, size_t _SizeofStrDestination, const double3& _Value) { return -1 != oPrintf(_StrDestination, _SizeofStrDestination, "%f %f %f", _Value.x, _Value.y, _Value.z) ? _StrDestination : nullptr; }
char* oToString(char* _StrDestination, size_t _SizeofStrDestination, const double4& _Value) { return -1 != oPrintf(_StrDestination, _SizeofStrDestination, "%f %f %f %f", _Value.x, _Value.y, _Value.z, _Value.w) ? _StrDestination : nullptr; }
char* oToString(char* _StrDestination, size_t _SizeofStrDestination, const quatd& _Value) { return -1 != oPrintf(_StrDestination, _SizeofStrDestination, "%f %f %f %f", _Value.x, _Value.y, _Value.z, _Value.w) ? _StrDestination : nullptr; }
char* oToString(char* _StrDestination, size_t _SizeofStrDestination, const int2& _Value) { return -1 != oPrintf(_StrDestination, _SizeofStrDestination, "%d %d", _Value.x, _Value.y) ? _StrDestination : nullptr; }
char* oToString(char* _StrDestination, size_t _SizeofStrDestination, const int3& _Value) { return -1 != oPrintf(_StrDestination, _SizeofStrDestination, "%d %d %d", _Value.x, _Value.y, _Value.z) ? _StrDestination : nullptr; }
char* oToString(char* _StrDestination, size_t _SizeofStrDestination, const int4& _Value) { return -1 != oPrintf(_StrDestination, _SizeofStrDestination, "%d %d %d %d", _Value.x, _Value.y, _Value.z, _Value.w) ? _StrDestination : nullptr; }
char* oToString(char* _StrDestination, size_t _SizeofStrDestination, const uint2& _Value) { return -1 != oPrintf(_StrDestination, _SizeofStrDestination, "%u %u", _Value.x, _Value.y) ? _StrDestination : nullptr; }
char* oToString(char* _StrDestination, size_t _SizeofStrDestination, const uint3& _Value) { return -1 != oPrintf(_StrDestination, _SizeofStrDestination, "%u %u %u", _Value.x, _Value.y, _Value.z) ? _StrDestination : nullptr; }
char* oToString(char* _StrDestination, size_t _SizeofStrDestination, const uint4& _Value) { return -1 != oPrintf(_StrDestination, _SizeofStrDestination, "%u %u %u %u", _Value.x, _Value.y, _Value.z, _Value.w) ? _StrDestination : nullptr; }
char* oToString(char* _StrDestination, size_t _SizeofStrDestination, const oRECT& _Value) { return -1 != oPrintf(_StrDestination, _SizeofStrDestination, "%u %u %u %u", _Value.GetMin().x, _Value.GetMin().y, _Value.GetMax().x, _Value.GetMax().y) ? _StrDestination : nullptr; }

char* oToString(char* _StrDestination, size_t _SizeofStrDestination, const oRGBf& _Value) 
{
	oColor c = oColorComposeRGB<oRGBf>(_Value);
	if (!oToString(_StrDestination, _SizeofStrDestination, c))
		if (!oToString(_StrDestination, _SizeofStrDestination, (const float3&)_Value))
			return nullptr;

	return _StrDestination;
}

template<typename T> char* oToStringT(char* _StrDestination, size_t _SizeofStrDestination, const TMAT4<T>& _Value)
{
	return -1 != oPrintf(_StrDestination, _SizeofStrDestination, "%f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f"
		, _Value.Column0.x, _Value.Column1.x, _Value.Column2.x, _Value.Column3.x
		, _Value.Column0.y, _Value.Column1.y, _Value.Column2.y, _Value.Column3.y
		, _Value.Column0.z, _Value.Column1.z, _Value.Column2.z, _Value.Column3.z
		, _Value.Column0.w, _Value.Column1.w, _Value.Column2.w, _Value.Column3.w) ? _StrDestination : nullptr;
}

char* oToString(char* _StrDestination, size_t _SizeofStrDestination, const float4x4& _Value) { return oToStringT<float>(_StrDestination, _SizeofStrDestination, _Value); }
char* oToString(char* _StrDestination, size_t _SizeofStrDestination, const double4x4& _Value) { return oToStringT<double>(_StrDestination, _SizeofStrDestination, _Value); }

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
				transformedObservers[i] = mul( rot, T_VEC4( transformedObservers[i], 1.0f ) ).xyz();

			// Add the rotation to our transform
			completeTransform = mul( rot, completeTransform );

			// Rotate everything such that the third point lies on the Z-plane (collapsing the z component )
			const T_VEC3& poi = transformedObservers[2];
			T rad = acos( normalize( T_VEC3( 0.0f, poi.y, poi.z) ).y );
			rad *= poi.z < 0.0f ? 1.0f : -1.0f;

			rot = oCreateRotation( rad, T_VEC3( 1.0f, 0.0f, 0.0f ) );
			for( int j = 1; j < 4; ++ j )
				transformedObservers[j] = mul( rot, T_VEC4( transformedObservers[j], 1.0f ) ).xyz();

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
			T negDistToFourth = abs( distance( T_VEC3( triPos.xy(), -triPos[2] ), transformedObservers[3] ) - distances[3]  );
			if( negDistToFourth < posDistToFourth )
				triPos.z = -triPos.z;

			T error = __min( posDistToFourth, negDistToFourth );
			if( error < minError )
			{
				minError = error;
				bestEstimate = mul( inverseTransform, T_VEC4( triPos, 1.0f ) ).xyz();
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

/** <citation
	usage="Implementation" 
	reason="9/7 CDF Wavlet transform"
	author="Gregoire Pau"
	description="http://www.embl.de/~gpau/misc/dwt97.c"
	license="*** Assumed Public Domain ***"
	licenseurl="http://www.embl.de/~gpau/misc/dwt97.c"
	modification="switched to floating point, renamed variables, removed malloc"
/>*/

// $(CitedCodeBegin)

void oCDF97Fwd(float* _pValues, size_t _NumValues)
{
	float a;
	size_t i;

	// Predict 1
	a=-1.586134342f;
	for (i=1;i<_NumValues-2;i+=2) {
		_pValues[i]+=a*(_pValues[i-1]+_pValues[i+1]);
	} 
	_pValues[_NumValues-1]+=2*a*_pValues[_NumValues-2];

	// Update 1
	a=-0.05298011854f;
	for (i=2;i<_NumValues;i+=2) {
		_pValues[i]+=a*(_pValues[i-1]+_pValues[i+1]);
	}
	_pValues[0]+=2.0f*a*_pValues[1];

	// Predict 2
	a=0.8829110762f;
	for (i=1;i<_NumValues-2;i+=2) {
		_pValues[i]+=a*(_pValues[i-1]+_pValues[i+1]);
	}
	_pValues[_NumValues-1]+=2*a*_pValues[_NumValues-2];

	// Update 2
	a=0.4435068522f;
	for (i=2;i<_NumValues;i+=2) {
		_pValues[i]+=a*(_pValues[i-1]+_pValues[i+1]);
	}
	_pValues[0]+=2*a*_pValues[1];

	// Scale
	a=1.0f/1.149604398f;
	for (i=0;i<_NumValues;i++) {
		if (i%2) _pValues[i]*=a;
		else _pValues[i]/=a;
	}

	// Pack
	float* TempBank = (float*)oStackAlloc(sizeof(float) * _NumValues );

	for (size_t i = 0; i < _NumValues; i++) 
	{
		if (i%2==0) 
			TempBank[i/2]= _pValues[i];
		else 
			TempBank[_NumValues/2+i/2] = _pValues[i];
	}

	for (size_t i = 0; i < _NumValues; i++) 
		_pValues[i]=TempBank[i];
}

void oCDF97Inv(float* _pValues, size_t _NumValues) 
{
	float a;
	size_t i;

	// Unpack
	float* TempBank = (float*)oStackAlloc(sizeof(float) * _NumValues );
	for (i=0;i<_NumValues/2;i++) {
		TempBank[i*2]=_pValues[i];
		TempBank[i*2+1]=_pValues[i+_NumValues/2];
	}
	for (i=0;i<_NumValues;i++) _pValues[i]=TempBank[i];

	// Undo scale
	a=1.149604398f;
	for (i=0;i<_NumValues;i++) {
		if (i%2) _pValues[i]*=a;
		else _pValues[i]/=a;
	}

	// Undo update 2
	a=-0.4435068522f;
	for (i=2;i<_NumValues;i+=2) {
		_pValues[i]+=a*(_pValues[i-1]+_pValues[i+1]);
	}
	_pValues[0]+=2*a*_pValues[1];

	// Undo predict 2
	a=-0.8829110762f;
	for (i=1;i<_NumValues-2;i+=2) {
		_pValues[i]+=a*(_pValues[i-1]+_pValues[i+1]);
	}
	_pValues[_NumValues-1]+=2*a*_pValues[_NumValues-2];

	// Undo update 1
	a=0.05298011854f;
	for (i=2;i<_NumValues;i+=2) {
		_pValues[i]+=a*(_pValues[i-1]+_pValues[i+1]);
	}
	_pValues[0]+=2*a*_pValues[1];

	// Undo predict 1
	a=1.586134342f;
	for (i=1;i<_NumValues-2;i+=2) {
		_pValues[i]+=a*(_pValues[i-1]+_pValues[i+1]);
	} 
	_pValues[_NumValues-1]+=2*a*_pValues[_NumValues-2];
}

// $(CitedCodeEnd)
