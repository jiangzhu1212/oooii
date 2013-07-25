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
// This header is designed to cross-compile in both C++ and HLSL. This defines
// a frustum using planes. Containment is based on planes that point inward, so
// the positive direction is towards the inside of the frustum.

#ifndef oHLSL
	#pragma once
#endif
#ifndef oCompute_frustum_h
#define oCompute_frustum_h

#include <oCompute/oComputeUtil.h>
#include <oCompute/oPlane.h>
#include <oHLSL/oHLSLMath.h>
#include <oHLSL/oHLSLTypes.h>

#ifdef oHLSL

#else

enum oFRUSTUM_CORNER
{
	oFRUSTUM_LEFT_TOP_NEAR,
	oFRUSTUM_LEFT_TOP_FAR,
	oFRUSTUM_LEFT_BOTTOM_NEAR,
	oFRUSTUM_LEFT_BOTTOM_FAR,
	oFRUSTUM_RIGHT_TOP_NEAR,
	oFRUSTUM_RIGHT_TOP_FAR,
	oFRUSTUM_RIGHT_BOTTOM_NEAR,
	oFRUSTUM_RIGHT_BOTTOM_FAR,
	oFRUSTUM_CORNER_COUNT,
};

enum oFRUSTUM_PLANE
{
	oFRUSTUM_LEFT,
	oFRUSTUM_RIGHT,
	oFRUSTUM_TOP,
	oFRUSTUM_BOTTOM,
	oFRUSTUM_NEAR,
	oFRUSTUM_FAR,
	oFRUSTUM_PLANE_COUNT,
};

// Wrap in a namespace so that NoStepInto can be used for VS2010+.
namespace oCompute {
	template<typename T> struct frustum
	{
		typedef T element_type;
		typedef plane<T> plane_type;
		typedef TMAT4<T> matrix_type;

		// This order must match oFRUSTUM_CORNER order
		plane_type Left;
		plane_type Right;
		plane_type Top;
		plane_type Bottom;
		plane_type Near;
		plane_type Far;

		frustum() {}

		// This frustum will be in whatever space the matrix was in minus one,
		// meaning in the space conversion of:
		// Model -> World -> View -> Projection that a projection matrix returned 
		// by oCreatePerspective?H() will be in view space. A View * Projection 
		// will be in world space, and a WVP matrix will be in model space.
		frustum(const matrix_type& _Projection) { oExtractFrustumPlanes(&Left, _Projection, true); }
		const frustum<T>& operator=(const matrix_type& _Projection) { oExtractFrustumPlanes(&Left, _Projection, true); return *this; }
		inline const plane_type& GetPlane(oFRUSTUM_CORNER _Corner) const { return (&Left)[_Index]; }
	};
} // namespace oCompute

typedef oCompute::frustum<float> oFrustumf; //typedef TFRUSTUM<double> oFrustumd; // @oooii-tony: Need an oIntersects for double

// Fills the specified array with planes that point inward in the following 
// order: left, right, top, bottom, near, far. The planes will be in whatever 
// space the matrix was in minus one. This means in the space conversion of:
// Model -> World -> View -> Projection that a projection matrix returned by
// oCreatePerspective?H() will be in view space. A View * Projection will be
// in world space, and a WVP matrix will be in model space.
template<typename T> void oExtractFrustumPlanes(oCompute::plane<T> _Planes[oFRUSTUM_PLANE_COUNT], const TMAT4<T>& _Projection, bool _Normalize)
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
	_Planes[oFRUSTUM_LEFT].x = _Projection[0][3] + _Projection[0][0];
	_Planes[oFRUSTUM_LEFT].y = _Projection[1][3] + _Projection[1][0];
	_Planes[oFRUSTUM_LEFT].z = _Projection[2][3] + _Projection[2][0];
	_Planes[oFRUSTUM_LEFT].w = _Projection[3][3] + _Projection[3][0];

	// Right clipping plane
	_Planes[oFRUSTUM_RIGHT].x = _Projection[0][3] - _Projection[0][0];
	_Planes[oFRUSTUM_RIGHT].y = _Projection[1][3] - _Projection[1][0];
	_Planes[oFRUSTUM_RIGHT].z = _Projection[2][3] - _Projection[2][0];
	_Planes[oFRUSTUM_RIGHT].w = _Projection[3][3] - _Projection[3][0];

	// Top clipping plane
	_Planes[oFRUSTUM_TOP].x = _Projection[0][3] - _Projection[0][1];
	_Planes[oFRUSTUM_TOP].y = _Projection[1][3] - _Projection[1][1];
	_Planes[oFRUSTUM_TOP].z = _Projection[2][3] - _Projection[2][1];
	_Planes[oFRUSTUM_TOP].w = _Projection[3][3] - _Projection[3][1];

	// Bottom clipping plane
	_Planes[oFRUSTUM_BOTTOM].x = _Projection[0][3] + _Projection[0][1];
	_Planes[oFRUSTUM_BOTTOM].y = _Projection[1][3] + _Projection[1][1];
	_Planes[oFRUSTUM_BOTTOM].z = _Projection[2][3] + _Projection[2][1];
	_Planes[oFRUSTUM_BOTTOM].w = _Projection[3][3] + _Projection[3][1];

	// Near clipping plane
	if (oHasPerspective(_Projection))
	{
		_Planes[oFRUSTUM_NEAR].x = _Projection[0][2];
		_Planes[oFRUSTUM_NEAR].y = _Projection[1][2];
		_Planes[oFRUSTUM_NEAR].z = _Projection[2][2];
		_Planes[oFRUSTUM_NEAR].w = _Projection[3][2];
	}

	else
	{
		_Planes[oFRUSTUM_NEAR].x = _Projection[0][3] + _Projection[0][2];
		_Planes[oFRUSTUM_NEAR].y = _Projection[1][3] + _Projection[1][2];
		_Planes[oFRUSTUM_NEAR].z = _Projection[2][3] + _Projection[2][2];
		_Planes[oFRUSTUM_NEAR].w = _Projection[3][3] + _Projection[3][2];
	}

	// Far clipping plane
	_Planes[oFRUSTUM_FAR].x = _Projection[0][3] - _Projection[0][2];
	_Planes[oFRUSTUM_FAR].y = _Projection[1][3] - _Projection[1][2];
	_Planes[oFRUSTUM_FAR].z = _Projection[2][3] - _Projection[2][2];
	_Planes[oFRUSTUM_FAR].w = _Projection[3][3] - _Projection[3][2];

  if (_Normalize)
    for (int i = 0; i < oFRUSTUM_PLANE_COUNT; i++)
      _Planes[i] = oNormalizePlane(_Planes[i]);

	// $(CitedCodeEnd)
}

// Fills the specified array with points that represent the 8 corners of the
// frustum. Index into the array using TFRUSTUM::CORNER. Returns true if values 
// are valid or false if planes don't meet in 8 corners.
template<typename T> bool oExtractFrustumCorners(const oCompute::plane<T> _Planes[oFRUSTUM_PLANE_COUNT], TVEC3<T> _Corners[oFRUSTUM_CORNER_COUNT])
{
	// @oooii-tony: TODO implement oIntersects for double
	bool isect = oIntersects(_Planes[oFRUSTUM_LEFT], _Planes[oFRUSTUM_TOP], _Planes[oFRUSTUM_NEAR], _Corners[oFRUSTUM_LEFT_TOP_NEAR]);
	isect = isect && oIntersects(_Planes[oFRUSTUM_LEFT], _Planes[oFRUSTUM_TOP], _Planes[oFRUSTUM_FAR], _Corners[oFRUSTUM_LEFT_TOP_FAR]);
	isect = isect && oIntersects(_Planes[oFRUSTUM_LEFT], _Planes[oFRUSTUM_BOTTOM], _Planes[oFRUSTUM_NEAR], _Corners[oFRUSTUM_LEFT_BOTTOM_NEAR]);
	isect = isect && oIntersects(_Planes[oFRUSTUM_LEFT], _Planes[oFRUSTUM_BOTTOM], _Planes[oFRUSTUM_FAR], _Corners[oFRUSTUM_LEFT_BOTTOM_FAR]);
	isect = isect && oIntersects(_Planes[oFRUSTUM_RIGHT], _Planes[oFRUSTUM_TOP], _Planes[oFRUSTUM_NEAR], _Corners[oFRUSTUM_RIGHT_TOP_NEAR]);
	isect = isect && oIntersects(_Planes[oFRUSTUM_RIGHT], _Planes[oFRUSTUM_TOP], _Planes[oFRUSTUM_FAR], _Corners[oFRUSTUM_RIGHT_TOP_FAR]);
	isect = isect && oIntersects(_Planes[oFRUSTUM_RIGHT], _Planes[oFRUSTUM_BOTTOM], _Planes[oFRUSTUM_NEAR], _Corners[oFRUSTUM_RIGHT_BOTTOM_NEAR]);
	isect = isect && oIntersects(_Planes[oFRUSTUM_RIGHT], _Planes[oFRUSTUM_BOTTOM], _Planes[oFRUSTUM_FAR], _Corners[oFRUSTUM_RIGHT_BOTTOM_FAR]);
	return isect;
}

template<typename T> bool oExtractFrustumCorners(const oCompute::frustum<T>& _Frustum, TVEC3<T> _Corners[oFRUSTUM_CORNER_COUNT])
{
	return oExtractFrustumCorners((const oCompute::plane<T>*)&_Frustum, _Corners);
}

#endif
#endif