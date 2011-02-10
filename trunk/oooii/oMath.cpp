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
#include "pch.h"
#include <oooii/oMath.h>
#include <oooii/oAssert.h>
#include <oooii/oByte.h>
#include <oooii/oErrno.h>
#include <oooii/oSingleton.h>
#include <oooii/oString.h>
#include "perlin.h"
#include <vectormath/scalar/cpp/vectormath_aos.h>
#include <vectormath/scalar/cpp/vectormath_aos_d.h>

using namespace Vectormath::Aos;

struct oPerlinContext : oSingleton<oPerlinContext>
{
	oPerlinContext()
		: P(4,4,1,94)
	{}

	Perlin P;
};

float noise(float x) { return oPerlinContext::Singleton()->P.Get(x); }
float noise(const TVECTOR2<float>& x) { return oPerlinContext::Singleton()->P.Get(x.x, x.y); }
float noise(const TVECTOR3<float>& x) { return oPerlinContext::Singleton()->P.Get(x.x, x.y, x.z); }
float noise(const TVECTOR4<float>& x)  { return oPerlinContext::Singleton()->P.Get(x.x, x.y, x.z); } // @oooii-tony: not yet implemented.

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

float4x4 oCreateRotation(const float3& _Radians)
{
	Matrix4 m = Matrix4::rotationZYX((const Vector3&)_Radians);;
	return (float4x4&)m;
}

float4x4 oCreateRotation(float _Radians, const float3& _NormalizedRotationAxis)
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
	Matrix4 m = Matrix4::translation((const Vector3&)_Translation);
	return (float4x4&)m;
}

float4x4 oCreateScale(const float3& _Scale)
{
	Matrix4 m = Matrix4::scale((const Vector3&)_Scale);
	return (float4x4&)m;
}

float4x4 oCreateLookAt(const float3& _Eye, const float3& _At, const float3& _Up)
{
	Matrix4 m = Matrix4::lookAt((const Point3&)_Eye, (const Point3&)_At, (const Vector3&)_Up);
	return (float4x4&)m;
}

float4x4 oCreatePerspective(float _FovY, float _AspectRatio, float _ZNear, float _ZFar)
{
	Matrix4 m = Matrix4::perspective(_FovY, _AspectRatio, _ZNear, _ZFar);
	return (float4x4&)m;
}

float4x4 oCreateOrthographic(float _Left, float _Right, float _Bottom, float _Top, float _ZNear, float _ZFar)
{
	Matrix4 m = Matrix4::orthographic(_Left, _Right, _Bottom, _Top, _ZNear, _ZFar);
	return (float4x4&)m;
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

double4x4 oCreateRotation(const double3& _Radians)
{
	Matrix4d m = Matrix4d::rotationZYX((const Vector3d&)_Radians);;
	return (double4x4&)m;
}

double4x4 oCreateRotation(double _Radians, const double3& _NormalizedRotationAxis)
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

double4x4 oCreateLookAt(const double3& _Eye, const double3& _At, const double3& _Up)
{
	Matrix4d m = Matrix4d::lookAt((const Point3d&)_Eye, (const Point3d&)_At, (const Vector3d&)_Up);
	return (double4x4&)m;
}

double4x4 oCreatePerspective(double _FovY, double _AspectRatio, double _ZNear, double _ZFar)
{
	Matrix4d m = Matrix4d::perspective(_FovY, _AspectRatio, _ZNear, _ZFar);
	return (double4x4&)m;
}

double4x4 oCreateOrthographic(double _Left, double _Right, double _Bottom, double _Top, double _ZNear, double _ZFar)
{
	Matrix4d m = Matrix4d::orthographic(_Left, _Right, _Bottom, _Top, _ZNear, _ZFar);
	return (double4x4&)m;
}

float4x4 oAsWV(const float4x4& _World, const float4x4& _View)
{
	// @oooii-tony: Since our matrices are right-handed, concat backwards to be left-handed/d3d compatible
	return _View * _World;
}

float4x4 oAsVP(const float4x4& _View, const float4x4& _Projection)
{
	// @oooii-tony: Since our matrices are right-handed, concat backwards to be left-handed/d3d compatible
	return _Projection * _View;
}

float4x4 oAsWVP(const float4x4& _World, const float4x4& _View, const float4x4& _Projection)
{
	// @oooii-tony: Since our matrices are right-handed, concat backwards to be left-handed/d3d compatible
	return _Projection * _View * _World;
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

	float2 scr = (_ScreenPoint - (_ViewportDimensions / 2.0f)) / (_Radius * (_ViewportDimensions-1.0f) / 2.0f);

	float z = 0.0f;
	float mag = dot(scr, scr); // length squared

	if (mag > 1.0f)
	{
		float scale = 1.0f / sqrt(mag);
		scr *= scale;
	}

	else
		z = sqrt(1.0f - mag);

	return float3(-scr.x, scr.y, -z);
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

template<typename T> static void lookupNP(TVECTOR3<T>& _N, TVECTOR3<T>& _P, const TAABOX<T, TVECTOR3<T>>& _Box, const TVECTOR4<T>& _Plane)
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
template<typename T> static int oContains(const TVECTOR4<T>* _pPlanes, size_t _NumPlanes, const TAABOX<T, TVECTOR3<T>>& _Box)
{
	/** <citation
		usage="Paper" 
		reason="A reasonably efficient non-SIMD frust cull" 
		author="Daniel Sýkora & Josef Jelínek"
		description="http://www.cescg.org/CESCG-2002/DSykoraJJelinek/index.html"
	/>*/

	TVECTOR3<T> n, p;
	int result = 1;
	for (size_t i = 0; i < _NumPlanes; i++)
	{
		const TVECTOR4<T>& plane = _pPlanes[i];
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
	return oContains(&_Frustum.Left, 6, _Box);
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

errno_t oToString(char* _StrDestination, size_t _SizeofStrDestination, const float2& _Value) { return sprintf_s(_StrDestination, _SizeofStrDestination, "%f %f", _Value.x, _Value.y); }
errno_t oToString(char* _StrDestination, size_t _SizeofStrDestination, const float3& _Value) { return sprintf_s(_StrDestination, _SizeofStrDestination, "%f %f %f", _Value.x, _Value.y, _Value.z); }
errno_t oToString(char* _StrDestination, size_t _SizeofStrDestination, const float4& _Value) { return sprintf_s(_StrDestination, _SizeofStrDestination, "%f %f %f %f", _Value.x, _Value.y, _Value.z, _Value.w); }
errno_t oToString(char* _StrDestination, size_t _SizeofStrDestination, const quatf& _Value) { return sprintf_s(_StrDestination, _SizeofStrDestination, "%f %f %f %f", _Value.x, _Value.y, _Value.z, _Value.w); }

template<typename T>
T oTrilaterateBase( const TVECTOR3<T> observersIn[4], const T distancesIn[4], TVECTOR3<T>* position )
{
	oASSERT( position, "oTrilaterate return value is NULL! " );

	typedef TVECTOR3<T> T_VEC3;
	typedef TVECTOR4<T> T_VEC4;
	typedef TMATRIX4<T> T_MAT4;

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
template<typename T> bool positionInStartCoordSystem( const TVECTOR3<T> startCoords[4], const TVECTOR3<T>  endCoords[4], TVECTOR3<T> endPos, TVECTOR3<T>* position )
{
	T distances[4];
	for( int i = 0; i < 4; ++i )
		distances[i] = distance( endCoords[i], endPos );

	return oTrilaterate( startCoords, distances, position ) < 10.0f;
}
template<typename T> bool oCoordinateTransformBase( const TVECTOR3<T> startCoords[4], const TVECTOR3<T> endCoords[4], TMATRIX4<T> *matrix )
{
	oASSERT( matrix, "oCoordinateTransform return value is NULL! " );
	TVECTOR3<T> orgin;
	TVECTOR3<T> posXAxis;
	TVECTOR3<T> posYAxis;
	TVECTOR3<T> posZAxis;

	// Trilaterate the orgin an axis
	if( !positionInStartCoordSystem( startCoords, endCoords, TVECTOR3<T>( 0.0, 0.0, 0.0 ), &orgin ) )
		return false;

	if( !positionInStartCoordSystem( startCoords, endCoords, TVECTOR3<T>( 1.0, 0.0, 0.0 ), &posXAxis ) )
		return false;

	if( !positionInStartCoordSystem( startCoords, endCoords, TVECTOR3<T>( 0.0, 1.0, 0.0 ), &posYAxis ) )
		return false;

	if( !positionInStartCoordSystem( startCoords, endCoords, TVECTOR3<T>( 0.0, 0.0, 1.0 ), &posZAxis ) )
		return false;

	// Normalize axis
	posXAxis = normalize( posXAxis - orgin );
	posYAxis = normalize( posYAxis - orgin );
	posZAxis = normalize( posZAxis - orgin );

	
	TMATRIX4<T> transform = TMATRIX4<T>::Identity;
	transform.Column0 =  TVECTOR4<T>( posXAxis, 0.0 );  
	transform.Column1 = TVECTOR4<T>( posYAxis, 0.0 );
	transform.Column2 = TVECTOR4<T>( posZAxis, 0.0 );

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
	typedef TVECTOR2<T> T_VEC;

	T_VEC Dimensions;
	_SrcRect.GetDimensions( &Dimensions.x, &Dimensions.y );

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
