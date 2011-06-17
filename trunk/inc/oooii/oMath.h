// $(header)

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
#include <oooii/oAssert.h>
#include <oooii/oBit.h>
#include <oooii/oLimits.h>
#include <oooii/oMathTypes.h>
#include <oooii/oStddef.h>

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

// requires defines
#include <oooii/oMathInternalHLSL.h>

// _____________________________________________________________________________
// Approximate Equality for halfs, floats and doubles

// ulps = "units of last place". Number of float-point steps of error. At various
// sizes, 1 bit of difference in the floating point number might mean large or
// small deltas, so just doing an epsilon difference is not valid across the
// entire spectrum of floating point representations. With ULPS, you specify the
// maximum number of floating-point steps, not absolute (fixed) value that some-
// thing should differ by, so it scales across all of float's range.
#define DEFAULT_ULPS 5

template<typename T> inline bool oEqual(const T& A, const T& B, int maxUlps = DEFAULT_ULPS) { return A == B; }

inline bool oEqual(const double& A, const double& B, int maxUlps)
{
	typedef long long intT;

	/** <citation
		usage="Adaptation" 
		reason="Apparently using eps isn't good enough." 
		author="Bruce Dawson"
		description="http://www.cygnus-software.com/papers/comparingfloats/comparingfloats.htm"
		license="*** Assumed Public Domain ***"
		licenseurl="http://www.cygnus-software.com/papers/comparingfloats/comparingfloats.htm"
		modification="changed assert macro and types to accommodate doubles"
	/>*/
	// $(CitedCodeBegin)
	// Make sure maxUlps is non-negative and small enough that the
	// default NAN won't compare as equal to anything.
	oASSERT(maxUlps > 0 && maxUlps < 4 * 1024 * 1024, "");
	intT aInt = *(intT*)&A;
	// Make aInt lexicographically ordered as a twos-complement int
	if (aInt < 0)
		aInt = 0x8000000000000000 - aInt;
	// Make bInt lexicographically ordered as a twos-complement int
	intT bInt = *(intT*)&B;
	if (bInt < 0)
		bInt = 0x8000000000000000 - bInt;
	intT intDiff = abs(aInt - bInt);
	if (intDiff <= maxUlps)
		return true;
	return false;
	// $(CitedCodeEnd)
}

inline bool oEqual(const float& A, const float& B, int maxUlps)
{
	/** <citation
		usage="Implementation" 
		reason="Apparently using eps isn't good enough." 
		author="Bruce Dawson"
		description="http://www.cygnus-software.com/papers/comparingfloats/comparingfloats.htm"
		license="*** Assumed Public Domain ***"
		licenseurl="http://www.cygnus-software.com/papers/comparingfloats/comparingfloats.htm"
		modification="changed assert macro"
	/>*/
	// $(CitedCodeBegin)
	// Make sure maxUlps is non-negative and small enough that the
	// default NAN won't compare as equal to anything.
	oASSERT(maxUlps > 0 && maxUlps < 4 * 1024 * 1024, "");
	int aInt = *(int*)&A;
	// Make aInt lexicographically ordered as a twos-complement int
	if (aInt < 0)
		aInt = 0x80000000 - aInt;
	// Make bInt lexicographically ordered as a twos-complement int
	int bInt = *(int*)&B;
	if (bInt < 0)
		bInt = 0x80000000 - bInt;
	int intDiff = abs(aInt - bInt);
	if (intDiff <= maxUlps)
		return true;
	return false;
	// $(CitedCodeEnd)
}

inline bool oEqual(const TVEC2<float>& a, const TVEC2<float>& b, int maxUlps = DEFAULT_ULPS) { return oEqual(a.x, b.x, maxUlps) && oEqual(a.y, b.y, maxUlps); }
inline bool oEqual(const TVEC3<float>& a, const TVEC3<float>& b, int maxUlps = DEFAULT_ULPS) { return oEqual(a.x, b.x, maxUlps) && oEqual(a.y, b.y, maxUlps) && oEqual(a.z, b.z, maxUlps); }
inline bool oEqual(const TVEC4<float>& a, const TVEC4<float>& b, int maxUlps = DEFAULT_ULPS) { return oEqual(a.x, b.x, maxUlps) && oEqual(a.y, b.y, maxUlps) && oEqual(a.z, b.z, maxUlps) && oEqual(a.w, b.w, maxUlps); }

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

template<typename T> inline T fresnel(const TVEC3<T>& i, const TVEC3<T>& n) { return 0.02f+0.97f*pow((1-max(dot(i, n))),5); } // http://habibs.wordpress.com/alternative-solutions/
template<typename T> inline T angle(const TVEC3<T>& a, const TVEC3<T>& b) { return acos(dot(a, b) / (length(a) * length(b))); }
template<typename T> inline TVEC4<T> oNormalizePlane(const TVEC4<T>& _Plane) { T invLength = rsqrt(dot(_Plane.XYZ(), _Plane.XYZ())); return _Plane * invLength; }

// Signed distance from a plane in Ax + By + Cz + D = 0 format (ABC = normalized normal, D = offset)
// This assumes the plane is normalized.
// >0 means on the same side as the normal
// <0 means on the opposite side as the normal
// 0 means on the plane
template<typename T> inline T sdistance(const TVEC4<T>& _Plane, const TVEC3<T>& _Point) { return dot(_Plane.XYZ(), _Point) - _Plane.w; }
template<typename T> inline T distance(const TVEC4<T>& _Plane, const TVEC3<T>& _Point) { return abs(sdistance(_Plane, _Point)); }

// _____________________________________________________________________________
// Quaternion functions

template<typename T> inline TQUAT<T> mul(const TQUAT<T>& _Quaternion0, const TQUAT<T>& _Quaternion1) { return TQUAT<T>(((((_Quaternion0.w * _Quaternion1.x) + (_Quaternion0.x * _Quaternion1.w)) + (_Quaternion0.y * _Quaternion1.z)) - (_Quaternion0.z * _Quaternion1.y)), ((((_Quaternion0.w * _Quaternion1.y) + (_Quaternion0.y * _Quaternion1.w)) + (_Quaternion0.z * _Quaternion1.x)) - (_Quaternion0.x * _Quaternion1.z)), ((((_Quaternion0.w * _Quaternion1.z) + (_Quaternion0.z * _Quaternion1.w)) + (_Quaternion0.x * _Quaternion1.y)) - (_Quaternion0.y * _Quaternion1.x)), ((((_Quaternion0.w * _Quaternion1.w) - (_Quaternion0.x * _Quaternion1.x)) - (_Quaternion0.y * _Quaternion1.y)) - (_Quaternion0.z * _Quaternion1.z))); }
template<typename T> TQUAT<T> oSlerp(const TQUAT<T>& a, const TQUAT<T>& b, T s);

// Rotates the specified vector by the specified normalized quaternion
template<typename T> inline TVEC3<T> oRotateVector(const TQUAT<T>& _Rotation, const TVEC3<T>& _Vector)
{
	// http://code.google.com/p/kri/wiki/Quaternions
	return _Vector + T(2.0) * cross(_Rotation.XYZ(), cross(_Rotation.XYZ(), _Vector) + _Rotation.w * _Vector);
}

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
template<typename T> TVEC3<T> oExtractTranslation(const TMAT4<T>& _Matrix) { T xy, xz, zy; TVEC3<T> s, r, t; TVEC4<T> p; oDecompose(_Matrix, &s, &xy, &xz, &zy, &r, &t, &p); return t; }

template<typename T> void oExtractAxes(const TMAT4<T>& _Matrix, TVEC3<T>* _pXAxis, TVEC3<T>* _pYAxis, TVEC3<T>* _pZAxis);

// returns true of there is a projection/perspective or false if orthographic
template<typename T> bool oHasPerspective(const TMAT4<T>& _Matrix);

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
template<typename T> TMAT4<T> oCreateRotation(const TVEC3<T>& _CurrenTVEC, const TVEC3<T>& _DesiredVector, const TVEC3<T>& _DefaultRotationAxis);
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
// (phyisical glass of the screen). By specifying a near plane that is closer to
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
// http://msdn.microsoft.com/en-us/library/ee415205(VS.85).aspx
inline float4x4 oFlipViewHandedness(const float4x4& _View) { float4x4 m = _View; m.Column2 = -m.Column2; return m; }

// Get the original axis values from a view matrix
void oExtractLookAt(const float4x4& _View, float3* _pEye, float3* _pAt, float3* _pUp, float3* _pRight);

// Get the world space eye position from a view matrix
template<typename T> inline TVEC3<T> oExtractEye(const TMAT4<T>& _View) { return invert(_View).Column3.XYZ(); }

// Fills the specified array with planes that point inward in the following 
// order: left, right, top, bottom, near, far. The planes will be in whatever 
// space the matrix was in minus one. This means in the space conversion of:
// Model -> World -> View -> Projection that a projection matrix returned by
// oCreatePerspective?H() will be in view space. A View * Projection will be
// in world space, and a WVP matrix will be in model space.
template<typename T> void oExtractFrustumPlanes(TVEC4<T> _Planes[6], const TMAT4<T>& _Projection, bool _Normalize);

// Calculates the distance from the eye to the near and far planes of the 
// visible frustum
template<typename T> inline void oCalculateNearFarPlanesDistance(const TMAT4<T>& _View, const TMAT4<T>& _Projection, T* _pNear, T* _pFar)
{
	// Store the 1/far plane distance so we can calculate view-space depth,
	// then store it on [0,1] for maximum precision.
	TFRUSTUM<T> WorldSpaceFrustum(_View * _Projection);
	const TVEC3<T> eye = oExtractEye(_View);
	*_pNear = distance(WorldSpaceFrustum.Near, eye);
	*_pFar = distance(WorldSpaceFrustum.Far, eye);
}

// This results in a normalized vector that points into the screen as is 
// needed for arcball-style calculation. An arcball is a sphere around a 
// focus point that is used to rotate an eye point around that focus while 
// maintaining the specified radius.
float3 oScreenToVector(const float2& _ScreenPoint, const float2& _ViewportDimensions, float _Radius);

// _____________________________________________________________________________
// Vertex-based mesh utility functions

void oTransformPoints(const float4x4& _Matrix, float3* oRESTRICT _pDestination, unsigned int _DestinationStride, const float3* oRESTRICT _pSource, unsigned int _SourceStride, unsigned int _NumPoints);
void oTransformVectors(const float4x4& _Matrix, float3* oRESTRICT _pDestination, unsigned int _DestinationStride, const float3* oRESTRICT _pSource, unsigned int _SourceStride, unsigned int _NumVectors);

// Removes indices for degenerate triangles. After calling this function, use
// oPruneUnindexedVertices() to clean up extra vertices.
// _pPositions: list of XYZ positions indexed by the index array
// _NumberOfVertices: The number of vertices in the _pPositions array
// _pIndices: array of triangles (every 3 specifies a triangle)
// _NumberOfIndices: The number of indices in the _pIndices array
// _pNewNumIndices: The new number of indices as a result of removed degenerages
template<typename T> bool oRemoveDegenerates(const TVEC3<T>* _pPositions, size_t _NumberOfPositions, unsigned int* _pIndices, size_t _NumberOfIndices, size_t* _pNewNumIndices);

// Calculates the face normals from the following inputs:
// _pFaceNormals: output, array to fill with normals. This should be at least as
//                large as the number of faces in the specified mesh (_NumberOfIndices/3)
// _pIndices: array of triangles (every 3 specifies a triangle)
// _NumberOfIndices: The number of indices in the _pIndices array
// _pPositions: list of XYZ positions for the mesh that are indexed by the index 
//              array
// _NumberOfPositions: The number of vertices in the _pPositions array
// _CCW: If true, triangles are assumed to have their front-face be specified by
//       the counter-clockwise order of vertices in a triangle. This affects 
//       which way a normal points.
//
// This can return EINVAL if a parameters isn't something that can be used.
template<typename T> bool oCalculateFaceNormals(TVEC3<T>* _pFaceNormals, const unsigned int* _pIndices, size_t _NumberOfIndices, const TVEC3<T>* _pPositions, size_t _NumberOfPositions, bool _CCW = false);

// Calculates the vertex normals by averaging face normals from the following 
// inputs:
// _pVertexNormals: output, array to fill with normals. This should be at least 
//                  as large as the number of vertices in the specified mesh
//                  (_NumberOfVertices).
// _pIndices: array of triangles (every 3 specifies a triangle)
// _NumberOfIndices: The number of indices in the _pIndices array
// _pPositions: list of XYZ positions for the mesh that are indexed by the index 
//              array
// _NumberOfPositions: The number of vertices in the _pPositions array
// _CCW: If true, triangles are assumed to have their front-face be specified by
//       the counter-clockwise order of vertices in a triangle. This affects 
//       which way a normal points.
// _OverwriteAll: Overwrites any pre-existing data in the array. If this is 
// false, any zero-length vector will be overwritten. Any length-having vector
// will not be touched.
// This can return EINVAL if a parameters isn't something that can be used.
template<typename T> bool oCalculateVertexNormals(TVEC3<T>* _pVertexNormals, const unsigned int* _pIndices, size_t _NumberOfIndices, const TVEC3<T>* _pPositions, size_t _NumberOfPositions, bool _CCW = false, bool _OverwriteAll = true);

// Calculates the tangent space vector and its handedness from the following
// inputs:
// _pTangents: output, array to fill with tangents. This should be at least 
//             as large as the number of certices in the specified mesh 
//             (_NumberOfVertices)
// _pIndices: array of triangles (every 3 specifies a triangle)
// _NumberOfIndices: The number of indices in the _pIndices array
// _pPositions: list of XYZ positions for the mesh that are indexed by the index 
//              array
// _pNormals: list of normalized normals for the mesh that are indexed by the 
//            index array
// _pTexcoords: list of texture coordinates for the mesh that are indexed by the 
//              index array
// _NumberOfVertices: The number of vertices in the _pPositions, _pNormals, and 
//                    _pTexCoords arrays
void oCalculateTangents(float4* _pTangents, const unsigned int* _pIndices, size_t _NumberOfIndices, const float3* _pPositions, const float3* _pNormals, const float2* _pTexcoords, size_t _NumberOfVertices);

// Allocates and fills an edge list for the mesh described by the specified 
// indices:
// _NumberOfVertices: The number of vertices the index array indexes
// _pIndices: array of triangles (every 3 specifies a triangle)
// _NumberOfIndices: The number of indices in the _pIndices array
// _ppEdges: a pointer to receive an allocation and be filled with index pairs 
//           describing an edge. Use oFreeEdgeList() to free memory the edge 
//           list allocation. So every two uints in *_ppEdges represents an edge.
// _pNumberOfEdges: a pointer to receive the number of edge pairs returned
void oCalculateEdges(size_t _NumberOfVertices, const unsigned int* _pIndices, size_t _NumberOfIndices, unsigned int** _ppEdges, size_t* _pNumberOfEdges);
void oFreeEdgeList(unsigned int* _pEdges);

// If some process modified indices, go through and compact the specified vertex
// streams. 
// _pIndices: array of triangles (every 3 specifies a triangle)
// _NumberOfIndices: The number of indices in the _pIndices array
// pointers to attributes: any can be NULL if not available
// _NumberOfVertices: The number of vertices the index array indexes
// _pNewNumVertices: receives the new count of each of the specified streams of 
// data
//
// All attribute streams will be modified and _pIndices values will be updated
// to reflect new vertices.
template<typename T> void oPruneUnindexedVertices(unsigned int* _pIndices, size_t _NumberOfIndices, TVEC3<T>* _pPositions, TVEC3<T>* _pNormals, TVEC4<T>* _pTangents, TVEC2<T>* _pTexcoords0, TVEC2<T>* _pTexcoords1, unsigned int* _pColors, size_t _NumberOfVertices, size_t *_pNewNumVertices);

// Given an array of points, compute the minimize and maximum axis-aligned 
// corners of the set. (Useful for calculating corners of an axis-aligned 
// bounding box.
template<typename T> void oCalculateMinMaxPoints(const TVEC3<T>* oRESTRICT _pPoints, size_t _NumberOfPoints, TVEC3<T>* oRESTRICT _pMinPoint, TVEC3<T>* oRESTRICT _pMaxPoint);

// _____________________________________________________________________________
// Containment/intersection/base collision

bool oIntersects(float3* _pIntersection, const float4& _Plane0, const float4& _Plane1, const float4& _Plane2);

// Calculate the point on this plane where the line segment
// described by p0 and p1 intersects.
bool oIntersects(float3* _pIntersection, const float4& _Plane, const float3& _Point0, const float3& _Point1);

inline bool oContains(const oRECT& _rect, int2 _point) {return _point.x >= _rect.GetMin().x && _point.x <= _rect.GetMax().x && _point.y >= _rect.GetMin().y && _point.y <= _rect.GetMax().y;}

// Returns -1 if the frustum partially contains the box, 0 if not at all contained,
// and 1 if the box is wholly inside the frustum.
template<typename T> int oContains(const TFRUSTUM<T>& _Frustum, const TAABOX<T,TVEC3<T>>& _Box);
template<typename T> int oContains(const TFRUSTUM<T>& _Frustum, const TVEC4<T>& _Sphere);
template<typename T> int oContains(const TSPHERE<T>& _Sphere, const TAABOX<T,TVEC3<T>>& _Box);

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

#endif
