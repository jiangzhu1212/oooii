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
// template versions of linear_algebra functions to support the occasional 
// interest in double precision for some tools code.

#include <exception>
#include <oBase/byte.h>
#include <oCompute/oComputeUtil.h>

#define MAT4_IDENTITY TMAT4<T>( \
			TVEC4<T>(T(1), T(0), T(0), T(0)), \
			TVEC4<T>(T(0), T(1), T(0), T(0)), \
			TVEC4<T>(T(0), T(0), T(1), T(0)), \
			TVEC4<T>(T(0), T(0), T(0), T(1)))

namespace ouro {

	float4x4 make_rotation(float _Radians, const float3& _NormalizedRotationAxis);
	double4x4 make_rotation(double _Radians, const double3& _NormalizedRotationAxis);

	namespace template_ {

template<typename T> TMAT4<T> make_rotation(const TVEC3<T>& _NormalizedFrom, const TVEC3<T>& _NormalizedTo)
{
	T a = angle(_NormalizedFrom, _NormalizedTo);

	// Check for identity
	if (equal(a, T(0)))
		return MAT4_IDENTITY;

	// Check for flip
	if (equal(a, T(oPI)))
		return TMAT4<T>(
			TVEC4<T>(T(-1), T(0), T(0), T(0)),
			TVEC4<T>(T(0), T(-1), T(0), T(0)),
			TVEC4<T>(T(0), T(0), T(-1), T(0)),
			TVEC4<T>(T(0), T(0), T(0), T(1)));

	TVEC3<T> NormalizedAxis = normalize(cross(_NormalizedFrom, _NormalizedTo));
	return ouro::make_rotation(a, NormalizedAxis);
}

template<typename T> TMAT4<T> make_translation(const TVEC3<T>& _Translation)
{
	return TMAT4<T>(
		TVEC4<T>(T(1), T(0), T(0), T(0)),
		TVEC4<T>(T(0), T(1), T(0), T(0)),
		TVEC4<T>(T(0), T(0), T(1), T(0)),
		TVEC4<T>(_Translation, T(1)));
}

template<typename T> TMAT4<T> make_scale(const TVEC3<T>& _Scale)
{
	return TMAT4<T>(
		TVEC4<T>(_Scale.x, T(0), T(0), T(0)),
		TVEC4<T>(T(0), _Scale.y, T(0), T(0)),
		TVEC4<T>(T(0), T(0), _Scale.z, T(0)),
		TVEC4<T>(T(0), T(0), T(0), T(1)));
}

template<typename T> TMAT4<T> make_reflection(const TVEC4<T>& _NormalizedReflectionPlane)
{
	const T A = _NormalizedReflectionPlane.x;
	const T B = _NormalizedReflectionPlane.y;
	const T C = _NormalizedReflectionPlane.z;
	const T D = _NormalizedReflectionPlane.w;
	return float4x4(
		TVEC4<T>(T(2) * A * A + T(1), T(2) * B * A,        T(2) * C * A,        T(0)),
		TVEC4<T>(T(2) * A * B,        T(2) * B * B + T(1), T(2) * C * B,        T(0)),
		TVEC4<T>(T(2) * A * C,        T(2) * B * C,        T(2) * C * C + T(1), T(0)),
		TVEC4<T>(T(2) * A * D,        T(2) * B * D,        T(2) * C * D,        T(1)));
}

// creates a matrix that transforms points from ones that lie on the XY plane (+Z up)
// to points that lie on plane p
template<typename T> TMAT4<T> make_plane(const TVEC4<T>& _Plane)
{
	TMAT4<T> m = make_rotation(TVEC3<T>(T(0), T(0), T(1)), _Plane.xyz());

	// Since make_rotation takes no default axis it will flip the world when the 
	// angle between the source and destination is 0. Since the desire is to force 
	// rotation around y negate the y portion of the rotation when the plane's 
	// normal is TVEC3<T>(T(0), T(0), T(-1))
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
template<typename T> TMAT4<T> make_normalization(const TVEC3<T>& _AABoxMin, const TVEC3<T>& _AABoxMax)
{
	const TVEC3<T> Scale = max(abs(_AABoxMax - _AABoxMin));
	// Scaling the bounds can introduce floating point precision issues near the 
	// bounds therefore add epsilon to the translation.
	const TVEC3<T> ScaledTranslation = (-_AABoxMin / Scale) + TVEC3<T>(std::numeric_limits<T>::epsilon());
	return mul(make_scale(rcp(Scale)), make_translation(ScaledTranslation));
}
template<typename T> const TVEC3<T> combine(const TVEC3<T>& a, const TVEC3<T>& b, T aScale, T bScale) { return aScale * a + bScale * b; }

// returns the specified vector with the specified length
template<typename T> TVEC3<T> scale(const TVEC3<T>& a, T newLength)
{
	T oldLength = length(a);
	if (equal(oldLength, T(0.0)))
		return a;
	return a * (newLength / oldLength);
}

template<typename T> bool decompose(const TMAT4<T>& _Matrix, TVEC3<T>* _pScale, T* _pShearXY, T* _pShearXZ, T* _pShearYZ, TVEC3<T>* _pRotation, TVEC3<T>* _pTranslation, TVEC4<T>* _pPerspective)
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
//unmatrix(mat, tran)
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
 	if (locmat[3][3] == 0)
 		return 0;
 	for (i=0; i<4;i++)
 		for (j=0; j<4; j++)
 			locmat[i][j] /= locmat[3][3];
 	/* pmat is used to solve for perspective, but it also provides
 	 * an easy way to test for singularity of the upper 3x3 component.
 	 */
 	pmat = locmat;
 	for (i=0; i<3; i++)
 		pmat[i][3] = 0;
 	pmat[3][3] = 1;

 	if (determinant(pmat) == 0.0)
 		return 0;

 	/* First, isolate perspective.  This is the messiest. */
 	if (locmat[0][3] != 0 || locmat[1][3] != 0 ||
 		locmat[2][3] != 0) {
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
 		invpmat = invert(pmat);
		tinvpmat = transpose(invpmat);
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
 	for (i=0; i<3; i++) {
 		(*_pTranslation)[i] = locmat[3][i];
 		locmat[3][i] = 0;
 	}

 	/* Now get scale and shear. */
 	for (i=0; i<3; i++) {
 		row[i].x = locmat[i][0];
 		row[i].y = locmat[i][1];
 		row[i].z = locmat[i][2];
 	}

 	/* Compute X scale factor and normalize first row. */
 	_pScale->x = length(*(TVEC3<T>*)&row[0]);
 	*(TVEC3<T>*)&row[0] = scale(*(TVEC3<T>*)&row[0], T(1.0));

 	/* Compute XY shear factor and make 2nd row orthogonal to 1st. */
 	*_pShearXY = dot(*(TVEC3<T>*)&row[0], *(TVEC3<T>*)&row[1]);
	*(TVEC3<T>*)&row[1] = combine(*(TVEC3<T>*)&row[1], *(TVEC3<T>*)&row[0], T(1.0), -*_pShearXY);

 	/* Now, compute Y scale and normalize 2nd row. */
 	_pScale->y = length(*(TVEC3<T>*)&row[1]);
 	*(TVEC3<T>*)&row[1] = scale(*(TVEC3<T>*)&row[1], T(1.0));
 	*_pShearXY /= _pScale->y;

 	/* Compute XZ and YZ shears, orthogonalize 3rd row. */
 	*_pShearXZ = dot(*(TVEC3<T>*)&row[0], *(TVEC3<T>*)&row[2]);
	*(TVEC3<T>*)&row[2] = combine(*(TVEC3<T>*)&row[2], *(TVEC3<T>*)&row[0], T(1.0), -*_pShearXZ);
 	*_pShearYZ = dot(*(TVEC3<T>*)&row[1], *(TVEC3<T>*)&row[2]);
 	*(TVEC3<T>*)&row[2] = combine(*(TVEC3<T>*)&row[2], *(TVEC3<T>*)&row[1], T(1.0), -*_pShearYZ);

 	/* Next, get Z scale and normalize 3rd row. */
 	_pScale->z = length(*(TVEC3<T>*)&row[2]);
 	*(TVEC3<T>*)&row[2] = scale(*(TVEC3<T>*)&row[2], T(1.0));
 	*_pShearXZ /= _pScale->z;
 	*_pShearYZ /= _pScale->z;
 
 	/* At this point, the matrix (in rows[]) is orthonormal.
 	 * Check for a coordinate system flip.  If the determinant
 	 * is -1, then negate the matrix and the scaling factors.
 	 */
 	if (dot(*(TVEC3<T>*)&row[0], cross(*(TVEC3<T>*)&row[1], *(TVEC3<T>*)&row[2])) < T(0))
 		for (i = 0; i < 3; i++) {
 			(*_pScale)[i] *= T(-1);
 			row[i].x *= T(-1);
 			row[i].y *= T(-1);
 			row[i].z *= T(-1);
 		}
 
 	/* Now, get the rotations out, as described in the gem. */
 	_pRotation->y = asin(-row[0].z);
 	if (cos(_pRotation->y) != 0) {
 		_pRotation->x = atan2(row[1].z, row[2].z);
 		_pRotation->z = atan2(row[0].y, row[0].x);
 	} else {
 		_pRotation->x = atan2(-row[2].x, row[1].y);
 		_pRotation->z = 0;
 	}
 	/* All done! */
 	return 1;
}

template<typename T> TMAT4<T> 
make_lookat_lh(const TVEC3<T>& _Eye, const TVEC3<T>& _At, const TVEC3<T>& _Up)
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

template<typename T> TMAT4<T> 
make_lookat_rh(const TVEC3<T>& _Eye, const TVEC3<T>& _At, const TVEC3<T>& _Up)
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

template<typename T> TMAT4<T> 
make_orthographic_lh(T _Left, T _Right, T _Bottom, T _Top, T _ZNear = oDEFAULT_NEAR, T _ZFar = oDEFAULT_FAR)
{
	return TMAT4<T>(
		TVEC4<T>(T(2)/(_Right-_Left),           T(0),                          T(0),                  T(0)),
		TVEC4<T>(T(0),                          T(2)/(_Top-_Bottom),           T(0),                  T(0)),
		TVEC4<T>(T(0),                          T(0),                          T(1)/(_ZFar-_ZNear),   T(0)),
		TVEC4<T>((_Left+_Right)/(_Left-_Right), (_Top+_Bottom)/(_Bottom-_Top), _ZNear/(_ZNear-_ZFar), T(1)));
}

template<typename T> TMAT4<T> 
make_orthographic_rh(T _Left, T _Right, T _Bottom, T _Top, T _ZNear = oDEFAULT_NEAR, T _ZFar = oDEFAULT_FAR)
{
	return TMAT4<T>(
		TVEC4<T>(T(2)/(_Right-_Left),           T(0),                          T(0),                  T(0)),
		TVEC4<T>(T(0),                          T(2)/(_Top-_Bottom),           T(0),                  T(0)),
		TVEC4<T>(T(0),                          T(0),                          T(1)/(_ZNear-_ZFar),   T(0)),
		TVEC4<T>((_Left+_Right)/(_Left-_Right), (_Top+_Bottom)/(_Bottom-_Top), _ZNear/(_ZNear-_ZFar), T(1)));
}

// declares and initializes _22 and _32
// Infinite projection matrix
// http://www.google.com/url?sa=t&source=web&cd=2&ved=0CBsQFjAB&url=http%3A%2F%2Fwww.terathon.com%2Fgdc07_lengyel.ppt&rct=j&q=eric%20lengyel%20projection&ei=-NpaTZvWKYLCsAOluNisCg&usg=AFQjCNGkbo93tbmlrXqkbdJg-krdEYNS1A

// http://knol.google.com/k/perspective-transformation#
// A nice reference that has LH and RH perspective matrices right next to each 
// other.

#define Z_PRECISION T(0.0001)
#define INFINITE_PLANE_LH(_ZFar) \
	T _22 = (_ZFar < T(0)) ? (T(1) - Z_PRECISION) : _ZFar / (_ZFar - _ZNear); \
	T _32 = (_ZFar < T(0)) ? _ZNear * (T(2) - Z_PRECISION) : (-_ZNear * _ZFar / (_ZFar - _ZNear))

// For off-center projection _32 needs to be negated when compared to INFINITE_PLANE_LH
#define INFINITE_PLANE_OFFCENTER_LH(_ZFar) \
	T _22 = (_ZFar < T(0)) ? (T(1) - Z_PRECISION) : _ZFar / (_ZFar - _ZNear); \
	T _32 = (_ZFar < T(0)) ? _ZNear * -(T(2) - Z_PRECISION) : (_ZNear * _ZFar / (_ZFar - _ZNear))

#define INFINITE_PLANE_RH(_ZFar) \
	T _22 = (_ZFar < T(0)) ? -(T(1) - Z_PRECISION) : _ZFar / (_ZNear - _ZFar); \
	T _32 = (_ZFar < T(0)) ? _ZNear * -(T(2) - Z_PRECISION) : (_ZNear * _ZFar / (_ZNear - _ZFar))

template<typename T> TMAT4<T> 
make_perspective_lh(const T& _FovYRadians, const T& _AspectRatio, const T& _ZNear, const T& _ZFar)
{
	const T yScale = T(1) / tanf(_FovYRadians / T(2));
	const T xScale = yScale / _AspectRatio;
	INFINITE_PLANE_LH(_ZFar);
	return TMAT4<T>(
		TVEC4<T>(xScale, T(0), T(0), T(0)),
		TVEC4<T>(T(0), yScale, T(0), T(0)),
		TVEC4<T>(T(0), T(0), _22, T(1)),
		TVEC4<T>(T(0), T(0), _32, T(0)));
}

template<typename T> TMAT4<T>
make_perspective_rh(const T& _FovYRadians, const T& _AspectRatio, const T& _ZNear, const T& _ZFar)
{
	const T& yScale = T(1) / tanf(_FovYRadians / T(2));
	const T& xScale = yScale / _AspectRatio;
	INFINITE_PLANE_RH(_ZFar);
	return TMAT4<T>(
		TVEC4<T>(xScale, T(0), T(0), T(0)),
		TVEC4<T>(T(0), yScale, T(0), T(0)),
		TVEC4<T>(T(0), T(0), _22, -T(1)),
		TVEC4<T>(T(0), T(0), _32, T(0)));
}

template<typename T> TMAT4<T> 
make_offcenter_perspective_lh(const T& _Left, const T& _Right, const T& _Bottom, const T& _Top, const T& _ZNear)
{
	INFINITE_PLANE_OFFCENTER_LH(-T(1));
	return TMAT4<T>(
		TVEC4<T>((T(2)*_ZNear)/(_Right-_Left),  T(0),                          T(0), T(0)),
		TVEC4<T>(T(0),                          (T(2)*_ZNear)/(_Top-_Bottom),  T(0), T(0)),
		TVEC4<T>((_Left+_Right)/(_Left-_Right), (_Top+_Bottom)/(_Bottom-_Top), _22,  T(1)),
		TVEC4<T>(T(0),                          T(0),                          _32,  T(0)));
}

// _OutputTransform Scale, Rotation, Translation (SRT) of the output device plane
// (i.e. the screen) in the same space as the eye (usually world space)
template<typename T> TMAT4<T>
make_offcenter_perspective_lh(const TMAT4<T>& _OutputTransform, const TVEC3<T>& _EyePosition, const T& _ZNear)
{
	// Get the position and dimensions of the output
	T ShXY, ShXZ, ShZY;
	TVEC3<T> outputSize, R, outputPosition;
	TVEC4<T> P;
	decompose(_OutputTransform, &outputSize, &ShXY, &ShXZ, &ShZY, &R, &outputPosition, &P);
	TVEC3<T> offset = outputPosition - _EyePosition;

	// Get the basis of the output
	TVEC3<T> outputBasisX = normalize(extract_axis_x(_OutputTransform));
	TVEC3<T> outputBasisY = normalize(extract_axis_y(_OutputTransform));
	TVEC3<T> outputBasisZ = normalize(extract_axis_z(_OutputTransform));

	// Get local offsets from the eye to the output
	T w = dot(outputBasisX, offset);
	T h = dot(outputBasisY, offset);
	T d = dot(outputBasisZ, offset);

	// Incorporate user near plane adjustment
	T zn = max(d + _ZNear, T(0.01));
	T depthScale = zn / d;

	return make_offcenter_perspective_lh(w * depthScale, (w + outputSize.x) * depthScale, h * depthScale, (h + outputSize.y) * depthScale, zn);
}

template<typename T> TMAT4<T> 
make_viewport(const T& _NDCResolutionX, const T& _NDCResolutionY, const T& _NDCRectLeft, const T& _NDCRectBottom, const T& _NDCRectWidth, const T& _NDCRectHeight)
{
	TVEC2<T> dim = TVEC2<T>(_NDCResolutionX, _NDCResolutionY);
	TVEC2<T> NDCMin = (TVEC2<T>(_NDCRectLeft, _NDCRectBottom) / dim) * T(2) - T(1);
	TVEC2<T> NDCMax = (TVEC2<T>(_NDCRectLeft + _NDCRectWidth, _NDCRectBottom + _NDCRectHeight) / dim) * T(2) - T(1);
	TVEC2<T> NDCScale = T(2) / (NDCMax - NDCMin);
	TVEC2<T> NDCTranslate = TVEC2<T>(-T(1), T(1)) - NDCMin * TVEC2<T>(NDCScale.x, -NDCScale.y);
	return TMAT4<T>(
		TVEC4<T>(NDCScale.x,     T(0),           T(0), T(0)),
		TVEC4<T>(T(0),           NDCScale.y,     T(0), T(0)),
		TVEC4<T>(T(0),           T(0),           T(1), T(0)),
		TVEC4<T>(NDCTranslate.x, NDCTranslate.y, T(0), T(1)));
}

// Returns a view matrix that fits the bounding sphere of the specified bounds
// to the specified projection. This preserves the rotation of the view, so this
// will only update the position of the view to be far enough from the center of 
// the bounds so it all fits on screen.
template<typename T> TMAT4<T> 
fit_to_view(const TMAT4<T>& _View, T _FovYRadians, const TVEC3<T>& _BoundCenter
	, T _BoundRadius, T _OffsetMultiplier = T(1))
{
	TMAT4<T> invView = invert(_View);
	T Offset = (_OffsetMultiplier * _BoundRadius) / tan(_FovYRadians / T(2));
	TVEC3<T> P = _BoundCenter - extract_axis_z(invView) * Offset;
	invView.Column3 = TVEC4<T>(P, invView.Column3.w);
	return invert(invView);
}

template<typename T> TMAT4<T> 
fit_to_view(const TMAT4<T>& _View, const TMAT4<T>& _Projection
	, const TVEC3<T>& _BoundCenter, T _BoundRadius, T _OffsetMultiplier = T(1))
{
	T FovYRadians, AspectRatio, ZNear, ZFar;
	extract_perspective_parameters(_Projection, &FovYRadians, &AspectRatio, &ZNear, &ZFar);
	return fit_to_view(_View, FovYRadians, _BoundCenter, _BoundRadius, _OffsetMultiplier);
}

// Get the original axis values from a view matrix
template<typename T> void extract_lookat(const TMAT4<T>& _View
	, TVEC3<T>* _pEye, TVEC3<T>* _pAt, TVEC3<T>* _pUp, TVEC3<T>* _pRight)
{
	*_pEye = invert(_View).Column3.xyz();
	_pRight->x = _View[0][0]; _pUp->x = _View[0][1]; _pAt->x = -_View[0][2];
	_pRight->y = _View[1][0]; _pUp->y = _View[1][1]; _pAt->y = -_View[1][2];
	_pRight->z = _View[2][0]; _pUp->z = _View[2][1]; _pAt->z = -_View[2][2];
}

template<typename T> void extract_near_far_distances(const TMAT4<T>& _Projection
	, T* _pNearDistance, T* _pFarDistance)
{
	T N, F;
	T C = _Projection[2][2];
	T D = _Projection[3][2];

	if (equal(C, T(0)))
		N = T(0);
	else
		N = (-D / C);

	if (oHasPerspective(_Projection))
	{
		if (equal(C, T(1)) || equal(D, T(0)))
			F = T(1) / abs(D);
		else
			F = D / (T(1) - C);
	}
	else
		F = (T(1) + C * N) / C;

	*_pNearDistance = N;
	*_pFarDistance = F;
}

// Extracts the parameters used in make_perspectivec() or make_orthographic()
// to define the specified projection.
template<typename T>
void extract_perspective_parameters(const TMAT4<T>& _Projection
	, T* _pFovYRadians, T* _pAspectRatio, T* _pZNear, T* _pZFar)
{
	*_pFovYRadians = atan(T(1) / _Projection.Column1.y) * T(2);
	*_pAspectRatio = _Projection.Column1.y / _Projection.Column0.x;
	extract_near_far_distances(_Projection, _pZNear, _pZFar);
}

// Returns the area, and fills _pCentroid with the center-most point from a 
// list of 2D vertices.
template<typename T>
T calcuate_area_and_centroid(TVEC2<T>* _pCentroid
	, const TVEC2<T>* _pVertices, size_t _VertexStride, size_t _NumVertices)
{
	// Bashein, Gerard, Detmer, Paul R. "Graphics Gems IV." 
	// ed. Paul S. Heckbert. pg 3-6. Academic Press, San Diego, 1994.

	T area = T(0);
	if (_NumVertices < 3)
		throw std::invalid_argument("must be at least 3 vertices");

	T atmp = T(0), xtmp = T(0), ytmp = T(0);
	const TVEC2<T>* vj = _pVertices;
	const TVEC2<T>* vi = byte_add(_pVertices, _VertexStride, _NumVertices - 1);
	const TVEC2<T>* end = byte_add(vi, _VertexStride, 1);
	while (vj < end)
	{
		T ai = vi->x * vj->y - vj->x * vi->y;
		atmp += ai;
		xtmp += (vj->x * vi->x) * ai;
		ytmp += (vj->y * vi->y) * ai;

		vi = vj;
		vj += _VertexStride;
	}

	area = atmp / T(2);
	if (!equal(atmp, T(0)))
	{
		_pCentroid->x = xtmp / (T(3) * atmp);
		_pCentroid->y = ytmp / (T(3) * atmp);
	}

	return area;
}

template<typename T>
T trilaterate(const TVEC3<T> _ObserversIn[4], const T _DistancesIn[4], TVEC3<T>* _pPosition)
{
	TVEC3<T> observers[4];
	T distances[4];
	for (int i = 0; i < 4; i++)
	{
		observers[i].x = _ObserversIn[i].x;
		observers[i].y = _ObserversIn[i].y;
		observers[i].z = _ObserversIn[i].z;
		distances[i] = _DistancesIn[i];
	}

	T minError = std::numeric_limits<float>::max();
	TVEC3<T> bestEstimate;

	// We try four times because one point is a tie-breaker and we want the best approximation
	for (int i = 0; i < 4; i++)
	{
		// Switch which observer is the tie-breaker (index 3)
		TVEC3<T> oldZero = observers[0];
		T oldZeroDistance = distances[0];
		for (int j = 0; j < 3; j++)
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
		// r2sqrd = (x - x1)^2	+ ysqrd			+ zsqrd
		// r3sqrd = (x - x2)^2	+ (y - y2) ^2 + zsqrd
		TMAT4<T> inverseTransform;
		TVEC3<T> transformedObservers[4];
		{
			// Translate everything such that the first point is at the orgin (collapsing the x y and z components)
			TVEC3<T> translation = -observers[0];
			TMAT4<T> completeTransform = make_translation(translation);

			for (int i = 0; i < 4; i++)
				transformedObservers[i] = observers[i] + translation;

			// Rotate everything such that the second point lies on the X-axis (collapsing the y and z components)
			TMAT4<T> rot =  make_rotation(normalize(transformedObservers[1]), TVEC3<T>(1.0f, 0.0f, 0.0f)); 
			for (int i = 1; i < 4; i++)
				transformedObservers[i] = mul(rot, TVEC4<T>(transformedObservers[i], 1.0f)).xyz();

			// Add the rotation to our transform
			completeTransform = mul(rot, completeTransform);

			// Rotate everything such that the third point lies on the Z-plane (collapsing the z component)
			const TVEC3<T>& poi = transformedObservers[2];
			T rad = acos(normalize(TVEC3<T>(0.0f, poi.y, poi.z)).y);
			rad *= poi.z < 0.0f ? 1.0f : -1.0f;

			rot = ouro::make_rotation(rad, TVEC3<T>(1.0f, 0.0f, 0.0f));
			for (int j = 1; j < 4; j++)
				transformedObservers[j] = mul(rot, TVEC4<T>(transformedObservers[j], 1.0f)).xyz();

			// Add the rotation to our transform
			completeTransform = mul(rot, completeTransform);

			// Invert so that we can later move back to the original space
			inverseTransform = invert(completeTransform);
		}

		// Trilaterate the postion in the transformed space
		TVEC3<T> triPos;
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
			T x = (r1sqrd - r2sqrd + x1sqrd) / (2 * x1);

			// Solve for y
			T y = ((r1sqrd - r3sqrd + x2sqrd + y2sqrd) / (2 * y2)) - ((x2 / y2) * x);

			// Solve positive Z
			T zsqrd = (r1sqrd - (x * x) - (y * y));
			if (zsqrd < 0.0)
				continue;

			// Use the fourth point as a tie-breaker
			T posZ = sqrt(zsqrd);
			triPos = TVEC3<T>(x, y, posZ);
			T posDistToFourth = abs(distance(triPos, transformedObservers[3]) - distances[3]);
			T negDistToFourth = abs(distance(TVEC3<T>(triPos.xy(), -triPos[2]), transformedObservers[3]) - distances[3]);
			if (negDistToFourth < posDistToFourth)
				triPos.z = -triPos.z;

			T error = __min(posDistToFourth, negDistToFourth);
			if (error < minError)
			{
				minError = error;
				bestEstimate = mul(inverseTransform, TVEC4<T>(triPos, 1.0f)).xyz();
			}
		}

	}

	// Return the trilaterated position in the original space
	*_pPosition = bestEstimate;
	return minError;
}

// Helper function for coordinate_transform that determines where a position 
// lies in another coordinate system.
template<typename T> bool position_in_start_coordinates(const TVEC3<T> _FromCoords[4], const TVEC3<T> _ToCoords[4], const TVEC3<T>& _EndPos, TVEC3<T>* _pPosition)
{
	T distances[4];
	for (int i = 0; i < 4; i++)
		distances[i] = distance(_ToCoords[i], _EndPos);
	return trilaterate(_FromCoords, distances, _pPosition) < T(10);
}

template<typename T> bool coordinate_transform(const TVEC3<T> _FromCoords[4], const TVEC3<T> _ToCoords[4], TMAT4<T>* _pMatrix)
{
	TVEC3<T> orgin;
	TVEC3<T> posXAxis;
	TVEC3<T> posYAxis;
	TVEC3<T> posZAxis;

	// Trilaterate the orgin an axis
	if (!position_in_start_coordinates(_FromCoords, _ToCoords, TVEC3<T>(T(0), T(0), T(0)), &orgin)) return false;
	if (!position_in_start_coordinates(_FromCoords, _ToCoords, TVEC3<T>(T(1), T(0), T(0)), &posXAxis)) return false;
	if (!position_in_start_coordinates(_FromCoords, _ToCoords, TVEC3<T>(T(0), T(1), T(0)), &posYAxis)) return false;
	if (!position_in_start_coordinates(_FromCoords, _ToCoords, TVEC3<T>(T(0), T(0), T(1)), &posZAxis)) return false;

	// Normalize axis
	posXAxis = normalize(posXAxis - orgin);
	posYAxis = normalize(posYAxis - orgin);
	posZAxis = normalize(posZAxis - orgin);

	TMAT4<T> transform;
	transform.Column0 = TVEC4<T>(posXAxis, T(0));
	transform.Column1 = TVEC4<T>(posYAxis, T(0));
	transform.Column2 = TVEC4<T>(posZAxis, T(0));
	transform.Column3 = TVEC4<T>(T(0), T(0), T(0), T(1));

	transform = invert(transform);
	transform = mul(transform, make_translation(-orgin));

	*_pMatrix = transform;
	return true;
}

	} // namespace template_
} // namespace ouro
