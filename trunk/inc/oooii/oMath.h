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
// IMPORTANT NOTES:
// The plane equation used here is Ax + By + Cz + D = 0, so see sdistance() as 
// to the implications. Primarily it means that positive D values are in the 
// direction/on the side of the normal, and negative values are in the opposite
// direction/on the opposite side of the normal.
// Remember when looking at the plane equation itself, a positive D offset is in 
// the opposite direction of the normal, and a negative D is in the direction of 
// the normal. The best example of seeing this is to use oCreateOrthographic to
// create a -1,1,-1,1,0,1 projection (unit/clip space) and then convert that to 
// a frustum with oCalcFrustumPlanesRH(). You'll see that the left clip plane is
// 1,0,0,1 meaning the normal points inward to the right and the offset is away
// from that normal to the left/-1 side. Likewise the right clip plane is 
// -1,0,0,1 meaning the normal points inward to the left, and the offset is once
// again away from that normal to the right/+1 side.

// The float4x4 and float3x3 matrices are OpenGL-style, column-major, right-handed.

#pragma once
#ifndef oMath_h
#define oMath_h

#include <Math.h>
#include <float.h>
#include <half.h>
#include <oooii/oAssert.h>
#include <oooii/oLimits.h>
#include <oooii/oStddef.h>
#include <oooii/oSwizzle.h>

#define oPI (3.14159265358979323846)
#define oPIf (float(oPI))
#define oE (2.71828183)
#define oEf (float(oE))

#ifndef oMATH_USE_FAST_ASINT
	#define oMATH_USE_FAST_ASINT 1
#endif

#ifndef oMATH_USE_FAST_RCP
	#define oMATH_USE_FAST_RCP 1
#endif

#ifndef oMATH_USE_FAST_RSQRT
	#define oMATH_USE_FAST_RSQRT 1
#endif

#ifndef oMATH_USE_FAST_LOG2
	#define oMATH_USE_FAST_LOG2 1
#endif

// _____________________________________________________________________________
// Fast bit-twiddling

#if defined(_WIN32) || defined(_WIN64)

	#ifdef __cplusplus
		extern "C" {
	#endif

	// forward declared to avoid including oWindows.h
	unsigned char _BitScanForward(unsigned long* Index, unsigned long Mask);
	unsigned char _BitScanReverse(unsigned long* Index, unsigned long Mask);

	#pragma intrinsic(_BitScanReverse)
	#pragma intrinsic(_BitScanForward)

	#ifdef _WIN64
		// forward declared to avoid including oWindows.h
		unsigned char _BitScanForward64(unsigned long* Index, unsigned __int64 Mask);
		unsigned char _BitScanReverse64(unsigned long* Index, unsigned __int64 Mask);
		#pragma intrinsic(_BitScanReverse64)
		#pragma intrinsic(_BitScanForward64)
	#endif

	#ifdef __cplusplus
		}
	#endif

	// returns 0 if no bits found
	inline bool oBSR(unsigned int& _Index, unsigned int _Mask) { return !!_BitScanReverse((unsigned long*)&_Index, _Mask); }
	inline bool oBSF(unsigned int& _Index, unsigned int _Mask) { return !!_BitScanForward((unsigned long*)&_Index, _Mask); }
	#ifdef _WIN64
		inline bool oBSR(unsigned int& _Index, unsigned long long _Mask) { return !!_BitScanReverse64((unsigned long*)&_Index, _Mask); }
		inline bool oBSF(unsigned int& _Index, unsigned long long _Mask) { return !!_BitScanForward64((unsigned long*)&_Index, _Mask); }
	#endif
#endif

// _____________________________________________________________________________
// Vector 2D, 3D, and 4D types

// Internal macros used in the macros below
#define oMATH_MEMBER_OP(return_t, param_t, op) inline const return_t& operator op##=(const param_t& a) { *this = *this op a; return *this; }
#define oMATH_ELOP2(op) template<typename T> inline TVECTOR2<T> operator op(const TVECTOR2<T>& a, const TVECTOR2<T>& b) { return TVECTOR2<T>(a.x op b.x, a.y op b.y); } template<typename T> inline TVECTOR2<T> operator op(const TVECTOR2<T>& a, const T& b) { return TVECTOR2<T>(a.x op b, a.y op b); } template<typename T> inline TVECTOR2<T> operator op(const T& a, const TVECTOR2<T>& b) { return TVECTOR2<T>(a op b.x, a op b.y); }
#define oMATH_ELOP3(op) template<typename T> inline TVECTOR3<T> operator op(const TVECTOR3<T>& a, const TVECTOR3<T>& b) { return TVECTOR3<T>(a.x op b.x, a.y op b.y, a.z op b.z); } template<typename T> inline TVECTOR3<T> operator op(const TVECTOR3<T>& a, const T& b) { return TVECTOR3<T>(a.x op b, a.y op b, a.z op b); } template<typename T> inline TVECTOR3<T> operator op(const T& a, const TVECTOR3<T>& b) { return TVECTOR3<T>(a op b.x, a op b.y, a op b.z); }
#define oMATH_ELOP4(op) template<typename T> inline TVECTOR4<T> operator op(const TVECTOR4<T>& a, const TVECTOR4<T>& b) { return TVECTOR4<T>(a.x op b.x, a.y op b.y, a.z op b.z, a.w op b.w); } template<typename T> inline TVECTOR4<T> operator op(const TVECTOR4<T>& a, const T& b) { return TVECTOR4<T>(a.x op b, a.y op b, a.z op b, a.w op b); } template<typename T> inline TVECTOR4<T> operator op(const T& a, const TVECTOR4<T>& b) { return TVECTOR4<T>(a op b.x, a op b.y, a op b.z, a op b.w); }
#define oMATH_ELUFN2(fn) template<typename T> inline TVECTOR2<T> fn(const TVECTOR2<T>& a) { return TVECTOR2<T>(fn(a.x), fn(a.y)); }
#define oMATH_ELUFN3(fn) template<typename T> inline TVECTOR3<T> fn(const TVECTOR3<T>& a) { return TVECTOR3<T>(fn(a.x), fn(a.y), fn(a.z)); }
#define oMATH_ELUFN4(fn) template<typename T> inline TVECTOR4<T> fn(const TVECTOR4<T>& a) { return TVECTOR3<T>(fn(a.x), fn(a.y), fn(a.z), fn(a.w)); }
#define oMATH_ELBFN2(pubfn, implfn) template<typename T> inline TVECTOR2<T> pubfn(const TVECTOR2<T>& a, const TVECTOR2<T>& b) { return TVECTOR2<T>(implfn(a.x, b.x), implfn(a.y, b.y)); }
#define oMATH_ELBFN3(pubfn, implfn) template<typename T> inline TVECTOR3<T> pubfn(const TVECTOR3<T>& a, const TVECTOR3<T>& b) { return TVECTOR3<T>(implfn(a.x, b.x), implfn(a.y, b.y), implfn(a.z, b.z)); }
#define oMATH_ELBFN4(pubfn, implfn) template<typename T> inline TVECTOR4<T> pubfn(const TVECTOR4<T>& a, const TVECTOR4<T>& b) { return TVECTOR3<T>(implfn(a.x, b.x), implfn(a.y, b.y), implfn(a.z, b.z), implfn(a.w, b.w)); }
#define oMATH_EQ2() template<typename T> inline bool operator==(const TVECTOR2<T>& a, const TVECTOR2<T>& b) { return oEqual(a.x, b.x) && oEqual(a.y, b.y); }
#define oMATH_EQ3() template<typename T> inline bool operator==(const TVECTOR3<T>& a, const TVECTOR3<T>& b) { return oEqual(a.x, b.x) && oEqual(a.y, b.y) && oEqual(a.z, b.z); }
#define oMATH_EQ4() template<typename T> inline bool operator==(const TVECTOR4<T>& a, const TVECTOR4<T>& b) { return oEqual(a.x, b.x) && oEqual(a.y, b.y) && oEqual(a.z, b.z) && oEqual(a.w, b.w); }
#define oMATH_NEQ(type) template<typename T> inline bool operator!=(const type& a, const type& b) { return !(a == b); }
#define oMATH_CMP2(fn,cmp) template<typename T> inline bool fn(const TVECTOR2<T>& a, const TVECTOR2<T>& b) { return a.x cmp b.x && a.y cmp b.y; }
#define oMATH_CMP3(fn,cmp) template<typename T> inline bool fn(const TVECTOR3<T>& a, const TVECTOR3<T>& b) { return a.x cmp b.x && a.y cmp b.y && a.z cmp b.z; }
#define oMATH_CMP4(fn,cmp) template<typename T> inline bool fn(const TVECTOR4<T>& a, const TVECTOR4<T>& b) { return a.x cmp b.x && a.y cmp b.y && a.z cmp b.z && a.w cmp b.w; }

// Macros to get through the boilerplate for operators, compares, etc.
#define oMATH_MEMBER_OPS(type, scalar_t) oMATH_MEMBER_OP(type, scalar_t, *) oMATH_MEMBER_OP(type, scalar_t, /) oMATH_MEMBER_OP(type, scalar_t, +) oMATH_MEMBER_OP(type, scalar_t, -) oMATH_MEMBER_OP(type, type, *) oMATH_MEMBER_OP(type, type, /) oMATH_MEMBER_OP(type, type, +) oMATH_MEMBER_OP(type, type, -)
#define oMATH_ELOPS(N) oMATH_ELOP##N(*) oMATH_ELOP##N(/) oMATH_ELOP##N(+) oMATH_ELOP##N(-) oMATH_ELOP##N(%)
#define oMATH_ELUFNS(fn) oMATH_ELUFN2(fn) oMATH_ELUFN3(fn) oMATH_ELUFN4(fn)
#define oMATH_ELBFNS(pubfn, implfn) oMATH_ELBFN2(pubfn, implfn) oMATH_ELBFN3(pubfn, implfn) oMATH_ELBFN4(pubfn, implfn)
#define oMATH_CMPS2(type) oMATH_EQ2() oMATH_NEQ(type) oMATH_CMP2(less_than, <) oMATH_CMP2(greater_than, >) oMATH_CMP2(less_than_equal, <=) oMATH_CMP2(greater_than_equal, >=)
#define oMATH_CMPS3(type) oMATH_EQ3() oMATH_NEQ(type) oMATH_CMP3(less_than, <) oMATH_CMP3(greater_than, >) oMATH_CMP3(less_than_equal, <=) oMATH_CMP3(greater_than_equal, >=)
#define oMATH_CMPS4(type) oMATH_EQ4() oMATH_NEQ(type) oMATH_CMP4(less_than, <) oMATH_CMP4(greater_than, >) oMATH_CMP4(less_than_equal, <=) oMATH_CMP4(greater_than_equal, >=)

template<typename T> struct TVECTOR2
{
	T x,y;
	inline TVECTOR2() {}
	inline TVECTOR2(const TVECTOR2& _Vector) : x(_Vector.x), y(_Vector.y) {}
	inline TVECTOR2(T _XY) : x(_XY), y(_XY) {}
	inline TVECTOR2(T _X, T _Y) : x(_X), y(_Y) {}
	const T& operator[](int i) const { return *(&x + i); }
	T& operator[](int i) { return *(&x + i); }
	oMATH_MEMBER_OPS(TVECTOR2<T>, T);
};

template<typename T> struct TVECTOR3
{
	T x,y,z;
	inline TVECTOR3() {};
	inline TVECTOR3(const TVECTOR3& _Vector) : x(_Vector.x), y(_Vector.y), z(_Vector.z) {}
	inline TVECTOR3(T _XYZ) : x(_XYZ), y(_XYZ), z(_XYZ) {}
	inline TVECTOR3(T _X, T _Y, T _Z) : x(_X), y(_Y), z(_Z) {}
	inline TVECTOR3(const TVECTOR2<T>& _XY, T _Z) : x(_XY.x), y(_XY.y), z(_Z) {}
	const T& operator[](int i) const { return *(&x + i); }
	T& operator[](int i) { return *(&x + i); }
	inline const TVECTOR2<T>& XY() const { return *(TVECTOR2<T>*)this; }
	inline const TVECTOR2<T>& YZ() const { return *(TVECTOR2<T>*)&y; }
	oMATH_MEMBER_OPS(TVECTOR3<T>, T);
};

template<typename T> struct TVECTOR4
{
	T x,y,z,w;
	inline TVECTOR4() {};
	inline TVECTOR4(const TVECTOR4& _Vector) : x(_Vector.x), y(_Vector.y), z(_Vector.z), w(_Vector.w) {}
	inline TVECTOR4(T _XYZW) : x(_XYZW), y(_XYZW), z(_XYZW), w(_XYZW) {}
	inline TVECTOR4(const TVECTOR2<T>& _XY, T _Z, T _W) : x(_XY.x), y(_XY.y), z(_Z), w(_Z) {}
	inline TVECTOR4(const TVECTOR3<T>& _XYZ, T _W) : x(_XYZ.x), y(_XYZ.y), z(_XYZ.z), w(_W) {}
	inline TVECTOR4(const TVECTOR2<T>& _XY, const TVECTOR2<T>& _ZW) : x(_XY.x), y(_XY.y), z(_ZW.z), w(_ZW.w) {}
	inline TVECTOR4(T _X, const TVECTOR3<T>& _YZW) : x(_X), y(_YZW.y), z(_YZW.z), w(_YZW.w) {}
	inline TVECTOR4(T _X, T _Y, T _Z, T _W) : x(_X), y(_Y), z(_Z), w(_W) {}
	const T& operator[](int i) const { return *(&x + i); }
	T& operator[](int i) { return *(&x + i); }
	inline const TVECTOR2<T>& XY() const { return *(TVECTOR2<T>*)&x; }
	inline const TVECTOR2<T>& YZ() const { return *(TVECTOR2<T>*)&y; }
	inline const TVECTOR2<T>& ZW() const { return *(TVECTOR2<T>*)&z; }
	inline const TVECTOR3<T>& XYZ() const { return *(TVECTOR3<T>*)&x; }
	inline const TVECTOR3<T>& YZW() const { return *(TVECTOR3<T>*)&y; }
	oMATH_MEMBER_OPS(TVECTOR4<T>, T);
};

template<typename T> struct TQUATERNION
{
	enum SPECIAL { Identity };
	T x,y,z,w;
	inline TQUATERNION() {};
	inline TQUATERNION(const TQUATERNION& _Quaternion) : x(_Quaternion.x), y(_Quaternion.y), z(_Quaternion.z), w(_Quaternion.w) {}
	inline TQUATERNION(SPECIAL _Type) : x(0), y(0), z(0), w(1) {}
	inline TQUATERNION(T _X, T _Y, T _Z, T _W) : x(_X), y(_Y), z(_Z), w(_W) {}
	const T& operator[](int i) const { return *(&x + i); }
	T& operator[](int i) { return *(&x + i); }
};

template<typename T> struct TMATRIX3
{
	// Column-major 3x3 matrix
	enum SPECIAL { Identity };
	TVECTOR3<T> Column0;
	TVECTOR3<T> Column1;
	TVECTOR3<T> Column2;
	TMATRIX3() {}
	TMATRIX3(const TMATRIX3& _Matrix) : Column0(_Matrix.Column0), Column1(_Matrix.Column1), Column2(_Matrix.Column2) {}
	TMATRIX3(const TVECTOR3<T>& _Column0, const TVECTOR3<T>& _Column1, const TVECTOR3<T>& _Column2) : Column0(_Column0), Column1(_Column1), Column2(_Column2) {}
	TMATRIX3(SPECIAL _Type) : Column0(T(1), T(0), T(0)), Column1(T(0), T(1), T(0)), Column2(T(0), T(0), T(1)) {}
	const TVECTOR3<T>& operator[](int i) const { return *(&Column0 + i); }
	TVECTOR3<T>& operator[](int i) { return *(&Column0 + i); }
};

template<typename T> struct TMATRIX4
{
	// Column-major 4x4 matrix
	enum SPECIAL { Identity };
	TVECTOR4<T> Column0;
	TVECTOR4<T> Column1;
	TVECTOR4<T> Column2;
	TVECTOR4<T> Column3;
	TMATRIX4() {}
	TMATRIX4(const TMATRIX4& _Matrix) : Column0(_Matrix.Column0), Column1(_Matrix.Column1), Column2(_Matrix.Column2), Column3(_Matrix.Column3) {}
	TMATRIX4(SPECIAL _Type) : Column0(T(1), T(0), T(0), T(0)), Column1(T(0), T(1), T(0), T(0)), Column2(T(0), T(0), T(1), T(0)), Column3(T(0), T(0), T(0), T(1)) {}
	TMATRIX4(const TVECTOR4<T>& _Column0, const TVECTOR4<T>& _Column1, const TVECTOR4<T>& _Column2, const TVECTOR4<T>& _Column3) : Column0(_Column0), Column1(_Column1), Column2(_Column2), Column3(_Column3) {}
	TMATRIX4(const TMATRIX3<T>& _ScaleRotation, const TVECTOR3<T>& _Translation) : Column0(_ScaleRotation.Column0, 0), Column1(_ScaleRotation.Column1, 0), Column2(_ScaleRotation.Column2, 0), Column3(_Translation, 0) {}
	TMATRIX4(const TQUATERNION<T>& _Rotation, const TVECTOR3<T>& _Translation) { TMATRIX3<T> r = oCreateRotationQ(_Rotation); *this = TMATRIX4(r, _Translation); }
	const TVECTOR4<T>& operator[](int i) const { return *(&Column0 + i); }
	TVECTOR4<T>& operator[](int i) { return *(&Column0 + i); }
};

template<typename T> inline bool operator==(const TQUATERNION<T>& a, const TQUATERNION<T>& b) { return oEqual(a.x, b.x) && oEqual(a.y, b.y) && oEqual(a.z, b.z) && oEqual(a.w, b.w); }
template<typename T> inline bool operator!=(const TQUATERNION<T>& a, const TQUATERNION<T>& b) { return !(a == b); }

template<typename T> inline TVECTOR2<T> operator-(const TVECTOR2<T>& a) { return TVECTOR2<T>(-a.x, -a.y); }
template<typename T> inline TVECTOR3<T> operator-(const TVECTOR3<T>& a) { return TVECTOR3<T>(-a.x, -a.y, -a.z); }
template<typename T> inline TVECTOR4<T> operator-(const TVECTOR4<T>& a) { return TVECTOR4<T>(-a.x, -a.y, -a.z, -a.w); }
template<typename T> inline TVECTOR3<T> operator*(const TMATRIX3<T>& a, const TVECTOR3<T>& b) { return mul(a, b); }
template<typename T> inline TMATRIX3<T> operator*(const TMATRIX3<T>& a, const TMATRIX3<T>& b) { return mul(a, b); }
template<typename T> inline TVECTOR4<T> operator*(const TMATRIX4<T>& a, const TVECTOR4<T>& b) { return mul(a, b); }
template<typename T> inline TMATRIX4<T> operator*(const TMATRIX4<T>& a, const TMATRIX4<T>& b) { return mul(a, b); }
template<typename T> inline TQUATERNION<T> operator*(const TQUATERNION<T>&a, const TQUATERNION<T>& b) { return mul(a, b); }

template<typename T>
inline TVECTOR2<T> oMin(const TVECTOR2<T>& a, const TVECTOR2<T>& b)
{
	TVECTOR2<T> mn;
	mn.x = __min(a.x, b.x);
	mn.y = __min(a.y, b.y);
	return mn;
}

template<typename T>
inline TVECTOR3<T> oMin(TVECTOR3<T> a, TVECTOR3<T> b)
{
	TVECTOR3<T> mn;
	mn.x = __min(a.x, b.x);
	mn.y = __min(a.y, b.y);
	mn.z = __min(a.z, b.z);
	return mn;
}

template<typename T>
inline TVECTOR2<T> oMax(const TVECTOR2<T>& a, const TVECTOR2<T>& b)
{
	TVECTOR2<T> mx;
	mx.x = __max(a.x, b.x);
	mx.y = __max(a.y, b.y);
	return mx;
}

template<typename T>
inline TVECTOR3<T> oMax(TVECTOR3<T> a, TVECTOR3<T> b)
{
	TVECTOR3<T> mx;
	mx.x = __max(a.x, b.x);
	mx.y = __max(a.y, b.y);
	mx.z = __max(a.z, b.z);
	return mx;
}

inline long long abs(const long long& x) { return _abs64(x); }

// ulps = "units of last place". Number of float-point steps of error. At various
// sizes, 1 bit of difference in the floating point number might mean large or
// small deltas, so just doing an epsilon difference is not valid across the
// entire spectrum of floating point representations. With ULPS, you specify the
// maximum number of floating-point steps, not absolute (fixed) value that some-
// thing should differ by, so it scales across all of float's range.
#define DEFAULT_ULPS 5

template<typename T> inline bool oEqual(const T& A, const T& B, int maxUlps = DEFAULT_ULPS) { return A == B; }

template<> inline bool oEqual(const double& A, const double& B, int maxUlps)
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

template<> inline bool oEqual(const float& A, const float& B, int maxUlps)
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

inline bool oEqual(const TVECTOR2<float>& a, const TVECTOR2<float>& b, int maxUlps = DEFAULT_ULPS) { return oEqual(a.x, b.x, maxUlps) && oEqual(a.y, b.y, maxUlps); }
inline bool oEqual(const TVECTOR3<float>& a, const TVECTOR3<float>& b, int maxUlps = DEFAULT_ULPS) { return oEqual(a.x, b.x, maxUlps) && oEqual(a.y, b.y, maxUlps) && oEqual(a.z, b.z, maxUlps); }
inline bool oEqual(const TVECTOR4<float>& a, const TVECTOR4<float>& b, int maxUlps = DEFAULT_ULPS) { return oEqual(a.x, b.x, maxUlps) && oEqual(a.y, b.y, maxUlps) && oEqual(a.z, b.z, maxUlps) && oEqual(a.w, b.w, maxUlps); }

template<typename T> inline T round(const T& a) { return floor(a + T(0.5)); }
template<typename T> inline T frac(const T& a) { return a - floor(a); }

inline bool isfinite(const double& a) { return !!_finite(a); }
inline bool isfinite(const float& a)
{
	#ifdef _M_X64
		return !!_finitef(a);
	#else
		return isfinite((double)a);
	#endif
}

template<typename T> inline bool isinf(const T& a) { return !isfinite(a); }
inline bool isinf(const double& a) { return !isfinite(a); }
inline bool isnan(const double& a) { return !!_isnan(a); }
inline bool isnan(const float& a)
{
	#ifdef _M_X64
		return !!_isnanf(a);
	#else
		return !!_isnan((double)a);
	#endif
}

// NON-HLSL
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

inline double log2(double a) { static const double sCONV = 1.0/log(2.0); return log(a) * sCONV; }
inline float log2(float val)
{
	#if oMATH_USE_FAST_LOG2
		/** <citation
			usage="Implementation" 
			reason="std libs don't have log2" 
			author="Blaxill"
			description="http://www.devmaster.net/forums/showthread.php?t=12765"
			license="*** Assumed Public Domain ***"
			licenseurl="http://www.devmaster.net/forums/showthread.php?t=12765"
			modification=""
		/>*/
		// $(CitedCodeBegin)
		int * const    exp_ptr = reinterpret_cast <int *>(&val);
		int            x = *exp_ptr;
		const int      log_2 = ((x >> 23) & 255) - 128;
		x &= ~(255 << 23);
		x += 127 << 23;
		*exp_ptr = x;

		val = ((-1.0f/3) * val + 2) * val - 2.0f/3; //(1)

		return (val + log_2);
		// $(CitedCodeEnd)
	#else
		static const double sCONV = 1.0/log(2.0);
		return log(val) * sCONV;
	#endif
}

inline float exp2(float a) { return powf(2.0f, a); }
inline double exp2(double a) { return pow(2.0, a); }
template<typename T> inline T frexp(const T& x, T& exp) { int e; T ret = ::frexp(x, &e); exp = static_cast<T>(e); return ret; }
inline float modf(const float& x, float& ip) { return ::modf(x, &ip); }
inline double modf(const double& x, double& ip) { return ::modf(x, &ip); }
template<typename T> inline TVECTOR2<T> modf(const TVECTOR2<T>& x, TVECTOR2<T>& ip) { return TVECTOR2<T>(modf(x.x, ip.x), modf(x.y, ip.y)); }
template<typename T> inline TVECTOR3<T> modf(const TVECTOR3<T>& x, TVECTOR3<T>& ip) { return TVECTOR3<T>(modf(x.x, ip.x), modf(x.y, ip.y), modf(x.z, ip.z)); }
template<typename T> inline TVECTOR4<T> modf(const TVECTOR4<T>& x, TVECTOR4<T>& ip) { return TVECTOR4<T>(modf(x.x, ip.x), modf(x.y, ip.y), modf(x.z, ip.z), modf(x.w, ip.w)); }
template<typename T> inline T smoothstep(const T& minimum, const T& maximum, const T& x) { return x < minimum ? T(0) : (x > maximum ? T(1) : T(-2) * pow((x – minimum) / (maximum – minimum), T(3)) + T(3) * pow((x – minimum) / (maximum – minimum), T(2))); } // http://http.developer.nvidia.com/CgTutorial/cg_tutorial_chapter05.html
template<typename T> inline T rcp(const T& value) { return T(1) / value; }
template<> inline float rcp(const float& x)
{
	#if oMATH_USE_FAST_RCP
		/** <citation
			usage="Implementation" 
			reason="Because we always need a faster math op." 
			author="Simon Hughes"
			description="http://www.codeproject.com/cpp/floatutils.asp?df=100&forumid=208&exp=0&select=950856#xx950856xx"
			license="CPOL 1.02"
			licenseurl="http://www.codeproject.com/info/cpol10.aspx"
			modification="FP_INV made more C++-like, compiler warning fixes"
		/>*/
		// $(CitedCodeBegin)
		// This is about 2.12 times faster than using 1.0f / n 
			int _i = 2 * 0x3F800000 - *(int*)&x;
			float r = *(float *)&_i;
			return r * (2.0f - x * r);
		// $(CitedCodeEnd)
	#else
		return 1.0f / x;
	#endif
}

template<typename T> inline T rsqrt(T x) { return T(1) / sqrt(x); }
template<> inline float rsqrt(float x)
{
	#if oMATH_USE_FAST_RSQRT
		/** <citation
			usage="Implementation" 
			reason="Because we always need a faster math op." 
			author="?"
			description="http://www.beyond3d.com/content/articles/8/"
			license="*** Assumed Public Domain ***"
			licenseurl="http://www.beyond3d.com/content/articles/8/"
			modification="code standard compliance, fix compiler warnings"
		/>*/
		// $(CitedCodeBegin)
		float xhalf = 0.5f*x;
		int i = *(int*)&x;
		i = 0x5f3759df - (i>>1);
		x = *(float*)&i;
		x = x*(1.5f - xhalf*x*x);
		return x;
		// $(CitedCodeEnd)
	#else
		return 1.0f / (float)sqrt(x);
	#endif
}

inline unsigned int countbits(unsigned long n)
{
	/** <citation
		usage="Implementation" 
		reason="std libs don't have bit functions" 
		author="Gurmeet Singh Manku"
		description="http://www.codeproject.com/cpp/floatutils.asp?df=100&forumid=208&exp=0&select=950856#xx950856xx"
		license="*** Assumed Public Domain ***"
		licenseurl="http://www.codeproject.com/cpp/floatutils.asp?df=100&forumid=208&exp=0&select=950856#xx950856xx"
		modification=""
	/>*/
	// $(CitedCodeBegin)
	// MIT HAKMEM Count
	/* works for 32-bit numbers only    */
	/* fix last line for 64-bit numbers */
	register unsigned int tmp;
	tmp = n - ((n >> 1) & 033333333333) - ((n >> 2) & 011111111111);
	return ((tmp + (tmp >> 3)) & 030707070707) % 63;
	// $(CitedCodeEnd)
}

inline unsigned int countbits(unsigned long long i) 
{
	/** <citation
		usage="Implementation" 
		reason="std libs don't have bit functions" 
		author="Maciej H"
		description="http://stackoverflow.com/questions/2709430/count-number-of-bits-in-a-64-bit-long-big-integer"
		license="*** Assumed Public Domain ***"
		licenseurl="http://stackoverflow.com/questions/2709430/count-number-of-bits-in-a-64-bit-long-big-integer"
		modification=""
	/>*/
	// $(CitedCodeBegin)
	// Hamming weight
	i = i - ((i >> 1) & 0x5555555555555555); 
	i = (i & 0x3333333333333333) + ((i >> 2) & 0x3333333333333333); 
	return (((i + (i >> 4)) & 0xF0F0F0F0F0F0F0F) * 0x101010101010101) >> 56; 
	// $(CitedCodeEnd)
}

inline unsigned int reversebits(unsigned int v)
{
	/** <citation
		usage="Implementation" 
		reason="std libs don't have bit functions"
		author="Edwin Freed"
		description="http://graphics.stanford.edu/~seander/bithacks.html#BitReverseObvious"
		license="*** Assumed Public Domain ***"
		licenseurl="http://graphics.stanford.edu/~seander/bithacks.html#BitReverseObvious"
		modification=""
	/>*/
	// $(CitedCodeBegin)
	//unsigned int v; // 32-bit word to reverse bit order
	// swap odd and even bits
	v = ((v >> 1) & 0x55555555) | ((v & 0x55555555) << 1);
	// swap consecutive pairs
	v = ((v >> 2) & 0x33333333) | ((v & 0x33333333) << 2);
	// swap nibbles ... 
	v = ((v >> 4) & 0x0F0F0F0F) | ((v & 0x0F0F0F0F) << 4);
	// swap bytes
	v = ((v >> 8) & 0x00FF00FF) | ((v & 0x00FF00FF) << 8);
	// swap 2-byte long pairs
	v = (v >> 16            ) | (v               << 16);
	return v;
	// $(CitedCodeEnd)
}

inline int reversebits(int v) { unsigned int r = reversebits(*(unsigned int*)&v); return *(int*)&r; }
inline int firstbithigh(unsigned int value) { unsigned int index; return oBSR(index, value) ? index : -1; }
inline int firstbithigh(int value) { return firstbithigh(*(unsigned int*)&value); }
inline int firstbitlow(unsigned int value) { unsigned int index; return oBSF(index, value) ? index : -1; }
inline int firstbitlow(int value) { return firstbitlow(*(unsigned int*)&value); }

#ifdef _M_X64
	inline int firstbithigh(unsigned long long value) { unsigned int index; return oBSR(index, value) ? index : -1; }
	inline int firstbithigh(long long value) { return firstbithigh(*(unsigned long long*)&value); }
	inline int firstbitlow(unsigned long long value) { unsigned int index; return oBSF(index, value) ? index : -1; }
	inline int firstbitlow(long long value) { return firstbitlow(*(unsigned long long*)&value); }
#endif

oMATH_ELOPS(2) oMATH_ELOPS(3) oMATH_ELOPS(4)
oMATH_CMPS2(TVECTOR2<T>) oMATH_CMPS3(TVECTOR3<T>) oMATH_CMPS4(TVECTOR4<T>)

// Float
oMATH_ELUFNS(abs);
oMATH_ELUFNS(ceil);
oMATH_ELUFNS(floor);
oMATH_ELUFNS(frac);
oMATH_ELUFNS(round);
template<typename T> inline T truc(const T& x) { return floor(x); }
template<typename T> inline bool isfinite(const TVECTOR2<T>& a) { return isfinite(a.x) && isfinite(a.y); }
template<typename T> inline bool isfinite(const TVECTOR3<T>& a) { return isfinite(a.x) && isfinite(a.y) && isfinite(a.z); }
template<typename T> inline bool isfinite(const TVECTOR4<T>& a) { return isfinite(a.x) && isfinite(a.y) && isfinite(a.z) && isfinite(a.w); }
template<typename T> inline bool isinf(const TVECTOR2<T>& a) { return isinf(a.x) && isinf(a.y); }
template<typename T> inline bool isinf(const TVECTOR3<T>& a) { return isinf(a.x) && isinf(a.y) && isinf(a.z); }
template<typename T> inline bool isinf(const TVECTOR4<T>& a) { return isinf(a.x) && isinf(a.y) && isinf(a.z) && isinf(a.w); }
template<typename T> inline bool isnan(const TVECTOR2<T>& a) { return isnan(a.x) && isnan(a.y); }
template<typename T> inline bool isnan(const TVECTOR3<T>& a) { return isnan(a.x) && isnan(a.y) && isnan(a.z); }
template<typename T> inline bool isnan(const TVECTOR4<T>& a) { return isnan(a.x) && isnan(a.y) && isnan(a.z) && isnan(a.w); }
template<typename T> inline bool all(const TVECTOR2<T>& a) { return !oEqual(a.x, T(0)) && !oEqual(a.y, T(0)); }
template<typename T> inline bool all(const TVECTOR3<T>& a) { return !oEqual(a.x, T(0)) && !oEqual(a.y, T(0)) && !oEqual(a.z, T(0)); }
template<typename T> inline bool all(const TVECTOR4<T>& a) { return !oEqual(a.x, T(0)) && !oEqual(a.y, T(0)) && !oEqual(a.z, T(0)) && !oEqual(a.w, T(0)); }
template<typename T> inline bool any(const T& a) { return a != T(0); }
template<typename T> inline T clamp(const T& x, const T& minimum, const T& maximum) { return __max(__min(x, maximum), minimum); }
template<typename T> inline T saturate(const T& x) { return clamp<T>(x, T(0.0), T(1.0)); }
template<typename T> inline T sign(const T& x) { return x < 0 ? T(-1) : (x == 0 ? T(0) : T(1)); }
template<typename T> inline T step(const T& y, const T& x) { return (x >= y) ? T(1) : T(0); } 
oMATH_ELBFNS(hlslmax, __max); // not named max to avoid name and macro collision
oMATH_ELBFNS(hlslmin, __min); // not named max to avoid name and macro collision

// Trig
oMATH_ELUFNS(acos);
oMATH_ELUFNS(asin);
oMATH_ELUFNS(atan);
oMATH_ELUFNS(cos);
oMATH_ELUFNS(cosh);
oMATH_ELUFNS(sin);
oMATH_ELUFNS(sinh);
oMATH_ELUFNS(tan);
oMATH_ELUFNS(tanh);
oMATH_ELBFNS(atan2, atan2);
template<typename T> inline void sincos(const T& angleInRadians, T& outSin, T& outCos) { outSin = sin(angleInRadians); outCos = cos(angleInRadians); }

// Advanced
oMATH_ELUFNS(log);
oMATH_ELUFNS(log10);
oMATH_ELUFNS(log2);
oMATH_ELUFNS(exp);
oMATH_ELUFNS(exp2);
oMATH_ELBFNS(ldexp, ldexp); // return x * 2^(exp) float
oMATH_ELBFNS(frexp, frexp);
oMATH_ELBFNS(fmod, fmod);
oMATH_ELUFNS(pow);
oMATH_ELUFNS(rcp);
oMATH_ELUFNS(sqrt);
oMATH_ELUFNS(step);
oMATH_ELUFNS(smoothstep);
template<typename T> inline T mad(const T& mvalue, const T& avalue, const T& bvalue) { return mvalue*avalue + bvalue; }
float noise(float x);
float noise(const TVECTOR2<float>& x);
float noise(const TVECTOR3<float>& x);
// float noise(const TVECTOR4<float>& x); // @oooii-tony: not yet implemented

// 3D
template<typename T> inline TVECTOR3<T> cross(const TVECTOR3<T>& a, const TVECTOR3<T>& b) { return TVECTOR3<T>(a.y*b.z - a.z*b.y, a.z*b.x - a.x*b.z, a.x*b.y - a.y*b.x); }
template<typename T> inline T dot(const TVECTOR2<T>& a, const TVECTOR2<T>& b) { return a.x*b.x + a.y*b.y; }
template<typename T> inline T dot(const TVECTOR3<T>& a, const TVECTOR3<T>& b) { return a.x*b.x + a.y*b.y + a.z*b.z; }
template<typename T> inline T dot(const TVECTOR4<T>& a, const TVECTOR4<T>& b) { return a.x*b.x + a.y*b.y + a.z*b.z + a.w*b.w; }
template<typename T> inline T length(const TVECTOR3<T>& a) { return sqrt(dot(a, a)); }
template<typename T> inline const T lerp(const T& a, const T& b, const T& s) { return a + s * (b-a); }
template<typename T> inline const TVECTOR2<T> lerp(const TVECTOR2<T>& a, const TVECTOR2<T>& b, const T& s) { return a + s * (b-a); }
template<typename T> inline const TVECTOR3<T> lerp(const TVECTOR3<T>& a, const TVECTOR3<T>& b, const T& s) { return a + s * (b-a); }
template<typename T> inline const TVECTOR4<T> lerp(const TVECTOR4<T>& a, const TVECTOR4<T>& b, const T& s) { return a + s * (b-a); }
template<typename T> inline TVECTOR4<T> lit(const T& n_dot_l, const T& n_dot_h, const T& m) { TVECTOR4<T>(T(1), (n_dot_l < 0) ? 0 : n_dot_l, (n_dot_l < 0) || (n_dot_h < 0) ? 0 : (n_dot_h * m), T(1)); }
template<typename T> inline T faceforward(const T& n, const T& i, const T& ng) { return -n * sign(dot(i, ng)); }
template<typename T> inline T normalize(const T& x) { return x / length(x); }
template<typename T> inline T radians(T degrees) { return degrees * T(oPI) / T(180.0); }
template<typename T> inline T degrees(T radians) { return radians * T(180.0) / T(oPI); }
template<typename T> inline T distance(const TVECTOR3<T>& a, const TVECTOR3<T>& b) { return length(a-b); }
template<typename T> inline T reflect(const T& i, const T& n) { return i - 2 * n * dot(i,n); }
template<typename T> inline T refract(const TVECTOR3<T>& i, const TVECTOR3<T>& n, const T& r) { T c1 = dot(i,n); T c2 = T(1) - r*r * (T(1) - c1*c1); return (c2 < T(0)) ? TVECTOR3<T>(0) : r*i + (sqrt(c2) - r*c1) * n; } // http://www.physicsforums.com/archive/index.php/t-187091.html

// _____________________________________________________________________________
// Unimplemented
//D3DCOLORtoUBYTE4
//dst

// _____________________________________________________________________________
// Partial derivatives. These are declared, but unimplemented 
// because this mere header does not provide enough infrastructure 
// to produce derivatives.
template<typename T> T ddx(const T& x);
template<typename T> T ddx_coarse(const T& x);
template<typename T> T ddx_fine(const T& x);
template<typename T> T ddy(const T& x);
template<typename T> T ddy_coarse(const T& x);
template<typename T> T ddy_fine(const T& x);
template<typename T> inline T fwidth(const T& x) { return abs(ddx(x)) + abs(ddy(x)); }

// Typedefs
typedef TVECTOR2<short> short2;
typedef TVECTOR3<short> short3;
typedef TVECTOR4<short> short4;

typedef unsigned short ushort;
typedef TVECTOR2<ushort> ushort2;
typedef TVECTOR3<ushort> ushort3;
typedef TVECTOR4<ushort> ushort4;

typedef TVECTOR2<int> int2;
typedef TVECTOR3<int> int3;
typedef TVECTOR4<int> int4;

typedef unsigned int uint;
typedef TVECTOR2<unsigned int> uint2;
typedef TVECTOR3<unsigned int> uint3;
typedef TVECTOR4<unsigned int> uint4;

typedef TVECTOR2<long long> longlong2;
typedef TVECTOR3<long long> longlong3;
typedef TVECTOR4<long long> longlong4;

typedef TVECTOR2<float> float2;
typedef TVECTOR3<float> float3;
typedef TVECTOR4<float> float4;

typedef TVECTOR2<double> double2;
typedef TVECTOR3<double> double3;
typedef TVECTOR4<double> double4;

typedef TMATRIX3<float> float3x3;
typedef TMATRIX4<float> float4x4;

typedef TMATRIX3<double> double3x3;
typedef TMATRIX4<double> double4x4;

typedef TQUATERNION<float> quatf;
typedef TQUATERNION<double> quatd;

// Type conversions
inline double asdouble(unsigned int lowbits, unsigned int highbits) { oByteSwizzle64 s; s.AsUnsignedInt[0] = lowbits; s.AsUnsignedInt[1] = highbits; return s.AsDouble; }
inline double2 asdouble(const uint2& lowbits, const uint2& highbits) { oByteSwizzle64 s[2]; s[0].AsUnsignedInt[0] = lowbits.x; s[0].AsUnsignedInt[1] = highbits.x; s[1].AsUnsignedInt[0] = lowbits.y; s[1].AsUnsignedInt[1] = highbits.y; return double2(s[0].AsDouble, s[1].AsDouble); }
inline float2 asfloat(const double& value) { oByteSwizzle64 s; s.AsDouble = value; return float2(s.AsFloat[0], s.AsFloat[1]); }
inline float4 asfloat(const double2& value) { oByteSwizzle64 s[2]; s[0].AsDouble = value.x; s[1].AsDouble = value.y; return float4(s[0].AsFloat[0], s[0].AsFloat[1], s[1].AsFloat[0], s[1].AsFloat[1]); }
inline float2 asfloat(const long long& value) { oByteSwizzle64 s; s.AsLongLong = value; return float2(s.AsFloat[0], s.AsFloat[1]); }
inline float4 asfloat(const longlong2& value) { oByteSwizzle64 s[2]; s[0].AsLongLong = value.x; s[1].AsLongLong = value.y; return float4(s[0].AsFloat[0], s[0].AsFloat[1], s[1].AsFloat[0], s[1].AsFloat[1]); }
template<typename T> inline float asfloat(const T& value) { return static_cast<float>(value); }
inline int2 asint(double value) { oByteSwizzle64 s; s.AsDouble = value; return int2(s.AsInt[0], s.AsInt[1]); }
inline int4 asint(double2 value) { oByteSwizzle64 s[2]; s[0].AsDouble = value.x; s[1].AsDouble = value.y; return int4(s[0].AsInt[0], s[0].AsInt[1], s[1].AsInt[0], s[1].AsInt[1]); }
inline int2 asint(long long value) { oByteSwizzle64 s; s.AsLongLong = value; return int2(s.AsInt[0], s.AsInt[1]); }
inline int4 asint(const longlong2& value) { oByteSwizzle64 s[2]; s[0].AsLongLong = value.x; s[1].AsLongLong = value.y; return int4(s[0].AsInt[0], s[0].AsInt[1], s[1].AsInt[0], s[1].AsInt[1]); }
template<typename T> inline int asint(T value) { return static_cast<int>(value); }
template<> inline int asint(float f)
{
	#if oMATH_USE_FAST_ASINT
		/** <citation
			usage="Implementation" 
			reason="Because we always need a faster math op." 
			author="Jonathon Blow"
			description="http://www.beyond3d.com/content/articles/8/"
			license="MIT-like"
			licenseurl="http://www.gdmag.com/resources/code.htm"
			modification="Renamed, code standard compliance, compiler warning fixes"
		/>
		<license>
			This program is Copyright (c) 2003 Jonathan Blow.  All rights reserved.
			Permission to use, modify, and distribute a modified version of 
			this software for any purpose and without fee is hereby granted, 
			provided that this notice appear in all copies.
			THE MATERIAL EMBODIED ON THIS SOFTWARE IS PROVIDED TO YOU "AS-IS"
			AND WITHOUT WARRANTY OF ANY KIND, EXPRESS, IMPLIED OR OTHERWISE,
			INCLUDING WITHOUT LIMITATION, ANY WARRANTY OF MERCHANTABILITY OR
			FITNESS FOR A PARTICULAR PURPOSE.
		</license>
		*/
		// $(CitedCodeBegin)

		// Blow, Jonathon. "Unified Rendering Level-of-Detail, Part 2." 
		// Game Developer Magazine, April 2003.
		// http://www.gdmag.com/resources/code.htm
		/*
			This file contains the necessary code to do a fast float-to-int
			conversion, where the input is an IEEE-754 32-bit floating-point
			number.  Just call FastInt(f).  The idea here is that C and C++
			have to do some extremely slow things in order to ensure proper
			rounding semantics; if you don't care about your integer result
			precisely conforming to the language spec, then you can use this.
			FastInt(f) is many many many times faster than (int)(f) in almost
			all cases.
		*/
		const unsigned int DOPED_MAGIC_NUMBER_32 = 0x4b3fffff;
		f += (float &)DOPED_MAGIC_NUMBER_32;
		int result = (*(unsigned int*)&f) - DOPED_MAGIC_NUMBER_32;
		return result;
		// $(CitedCodeEnd)
	#else
		return static_cast<int>(value);
	#endif
}

inline void asuint(double value, unsigned int& a, unsigned int& b) { oByteSwizzle64 s; s.AsDouble = value; a = s.AsUnsignedInt[0]; b = s.AsUnsignedInt[1]; }
oMATH_ELUFNS(countbits);
oMATH_ELUFNS(reversebits);
oMATH_ELUFNS(firstbithigh);
oMATH_ELUFNS(firstbitlow);

#ifdef _HALF_H_
	inline float f16tof32(unsigned int value) { half h; h.setBits(static_cast<unsigned short>(value)); return static_cast<float>(h); }
	template<typename T> inline float2 f16tof32(const float2& value) { return float2(f16tof32(value.x), f16tof32(value.y)); }
	template<typename T> inline float3 f16tof32(const float3& value) { return float2(f16tof32(value.x), f16tof32(value.y), f16tof32(value.z)); }
	template<typename T> inline float4 f16tof32(const float4& value) { return float2(f16tof32(value.x), f16tof32(value.y), f16tof32(value.z), f16tof32(value.w)); }
	inline unsigned int f32tof16(float value) { return half(value).bits(); }
	template<typename T> inline uint2 f32tof16(const float2& value) { return uint2(f32tof16(value.x), f32tof16(value.y)); }
	template<typename T> inline uint3 f32tof16(const float3& value) { return uint3(f32tof16(value.x), f32tof16(value.y), f32tof16(value.z)); }
	template<typename T> inline uint4 f32tof16(const float4& value) { return uint4(f32tof16(value.x), f32tof16(value.y), f32tof16(value.z), f32tof16(value.w)); }
#endif

// _____________________________________________________________________________
// Matrix ops

template<typename T> inline T determinant(const TMATRIX3<T>& _Matrix) { return dot(_Matrix.Column2, cross(_Matrix.Column0, _Matrix.Column1)); }
template<typename T> T determinant(const TMATRIX4<T>& _Matrix);

template<typename T> inline TVECTOR3<T> mul(const TMATRIX3<T>& _Matrix, const TVECTOR3<T>& _Vector) { return TVECTOR3<T>(_Matrix[0].x * _Vector.x + _Matrix[1].x * _Vector.y + _Matrix[2].x * _Vector.z, _Matrix[0].y * _Vector.x + _Matrix[1].y * _Vector.y + _Matrix[2].y * _Vector.z, _Matrix[0].z * _Vector.x + _Matrix[1].z * _Vector.y + _Matrix[2].z * _Vector.z); }
template<typename T> inline TMATRIX3<T> mul(const TMATRIX3<T>& _Matrix0, const TMATRIX3<T>& _Matrix1) { return TMATRIX3<T>((_Matrix0 * _Matrix1.Column0), (_Matrix0 * _Matrix1.Column1), (_Matrix0 * _Matrix1.Column2)); }
template<typename T> inline TVECTOR4<T> mul(const TMATRIX4<T>& _Matrix, const TVECTOR4<T>& _Vector) { return TVECTOR4<T>(((((_Matrix.Column0.x*_Vector.x) + (_Matrix.Column1.x*_Vector.y)) + (_Matrix.Column2.x*_Vector.z)) + (_Matrix.Column3.x*_Vector.w)), ((((_Matrix.Column0.y*_Vector.x) + (_Matrix.Column1.y*_Vector.y)) + (_Matrix.Column2.y*_Vector.z)) + (_Matrix.Column3.y*_Vector.w)), ((((_Matrix.Column0.z*_Vector.x) + (_Matrix.Column1.z*_Vector.y)) + (_Matrix.Column2.z*_Vector.z)) + (_Matrix.Column3.z*_Vector.w)), ((((_Matrix.Column0.w*_Vector.x) + (_Matrix.Column1.w*_Vector.y)) + (_Matrix.Column2.w*_Vector.z)) + (_Matrix.Column3.w*_Vector.w))); }
template<typename T> inline TMATRIX4<T> mul(const TMATRIX4<T>& _Matrix0, const TMATRIX4<T>& _Matrix1) { return TMATRIX4<T>((_Matrix0 * _Matrix1.Column0), (_Matrix0 * _Matrix1.Column1), (_Matrix0 * _Matrix1.Column2), (_Matrix0 * _Matrix1.Column3)); }
template<typename T> inline TQUATERNION<T> mul(const TQUATERNION<T>& _Quaternion0, const TQUATERNION<T>& _Quaternion1) { return TQUATERNION<T>(((((_Quaternion0.w * _Quaternion1.x) + (_Quaternion0.x * _Quaternion1.w)) + (_Quaternion0.y * _Quaternion1.z)) - (_Quaternion0.z * _Quaternion1.y)), ((((_Quaternion0.w * _Quaternion1.y) + (_Quaternion0.y * _Quaternion1.w)) + (_Quaternion0.z * _Quaternion1.x)) - (_Quaternion0.x * _Quaternion1.z)), ((((_Quaternion0.w * _Quaternion1.z) + (_Quaternion0.z * _Quaternion1.w)) + (_Quaternion0.x * _Quaternion1.y)) - (_Quaternion0.y * _Quaternion1.x)), ((((_Quaternion0.w * _Quaternion1.w) - (_Quaternion0.x * _Quaternion1.x)) - (_Quaternion0.y * _Quaternion1.y)) - (_Quaternion0.z * _Quaternion1.z))); }
template<typename T> inline TMATRIX3<T> transpose(const TMATRIX3<T>& _Matrix) { return TMATRIX3<T>(TVECTOR3<T>(_Matrix.Column0.x, _Matrix.Column1.x, _Matrix.Column2.x), TVECTOR3<T>(_Matrix.Column0.y, _Matrix.Column1.y, _Matrix.Column2.y), TVECTOR3<T>(_Matrix.Column0.z, _Matrix.Column1.z, _Matrix.Column2.z)); }
template<typename T> inline TMATRIX4<T> transpose(const TMATRIX4<T>& _Matrix) { return TMATRIX4<T>(TVECTOR4<T>(_Matrix.Column0.x, _Matrix.Column1.x, _Matrix.Column2.x, _Matrix.Column3.x), TVECTOR4<T>(_Matrix.Column0.y, _Matrix.Column1.y, _Matrix.Column2.y, _Matrix.Column3.y), TVECTOR4<T>(_Matrix.Column0.z, _Matrix.Column1.z, _Matrix.Column2.z, _Matrix.Column3.z), TVECTOR4<T>(_Matrix.Column0.w, _Matrix.Column1.w, _Matrix.Column2.w, _Matrix.Column3.w)); }

// NON-HLSL
template<typename T> TMATRIX3<T> invert(const TMATRIX3<T>& _Matrix);
template<typename T> TMATRIX4<T> invert(const TMATRIX4<T>& _Matrix);

template<typename T> TQUATERNION<T> slerp(const TQUATERNION<T>& a, const TQUATERNION<T>& b, T s);

// _____________________________________________________________________________
// NON-HLSL

// Rotation is done around the Z axis, then Y, then X.
template<typename T> TMATRIX4<T> oCreateRotation(const TVECTOR3<T>& _Radians);
template<typename T> TMATRIX4<T> oCreateRotation(T _Radians, const TVECTOR3<T>& _NormalizedRotationAxis);
template<typename T> TMATRIX4<T> oCreateRotation(const TVECTOR3<T>& _CurrentVector, const TVECTOR3<T>& _DesiredVector, const TVECTOR3<T>& _DefaultRotationAxis);
template<typename T> TMATRIX4<T> oCreateRotation(const TQUATERNION<T>& _Quaternion);
template<typename T> TQUATERNION<T> oCreateRotationQ(const TVECTOR3<T>& _Radians);
template<typename T> TQUATERNION<T> oCreateRotationQ(T _Radians, const TVECTOR3<T>& _NormalizedRotationAxis);
// There are infinite/undefined solutions when the vectors point in opposite directions
template<typename T> TQUATERNION<T> oCreateRotationQ(const TVECTOR3<T>& _CurrentVector, const TVECTOR3<T>& _DesiredVector);
template<typename T> TQUATERNION<T> oCreateRotationQ(const TMATRIX4<T>& _Matrix);
template<typename T> TMATRIX4<T> oCreateTranslation(const TVECTOR3<T>& _Translation);
template<typename T> TMATRIX4<T> oCreateScale(const TVECTOR3<T>& _Scale);
template<typename T> TMATRIX4<T> oCreateScale(T _Scale) { return oCreateScale(TVECTOR3<T>(_Scale)); }

template<typename T> TMATRIX4<T> oCreateLookAt(const TVECTOR3<T>& _Eye, const TVECTOR3<T>& _At, const TVECTOR3<T>& _Up);
template<typename T> TMATRIX4<T> oCreatePerspective(T _FovY, T _AspectRatio, T _ZNear, T _ZFar);
template<typename T> TMATRIX4<T> oCreateOrthographic(T _Left, T _Right, T _Bottom, T _Top, T _ZNear, T _ZFar);

// more matrix ops. Right-handed vs. left-handed is a headache, so when doing
// typical graphics operations, use these higher-level functions and let whoever
// is maintaining the math lib figure out what's best.

// Converts a right-handed view matrix, as can be returned from float4x4::lookAt(),
// to a left-handed matrix, such as is fit for DirectX. For more details, see:
// http://msdn.microsoft.com/en-us/library/ee415205(VS.85).aspx
inline float4x4 oAsViewLH(const float4x4& _ViewRH)
{
	float4x4 viewLH(_ViewRH);
	for (int i = 0; i < 4; i++)
		viewLH[2][i] = -viewLH[2][i];
	return viewLH;
}

float4x4 oAsWV(const float4x4& _World, const float4x4& _View);
float4x4 oAsVP(const float4x4& _View, const float4x4& _Projection);
float4x4 oAsWVP(const float4x4& _World, const float4x4& _View, const float4x4& _Projection);

float4x4 oAsReflection(const float4& _ReflectionPlane);
void oExtractLookAt(const float4x4& _View, float3* _pEye, float3* _pAt, float3* _pUp, float3* _pRight);
inline float3 oExtractEye(const float4x4& view) { return -view.Column3.XYZ(); }

// This results in a normalized vector that points into the screen as is 
// needed for arcball-style calculation. An arcball is a sphere around a 
// focus point that is used to rotate an eye point around that focus while 
// maintaining the specified radius.
float3 oScreenToVector(const float2& _ScreenPoint, const float2& _ViewportDimensions, float _Radius);

// creates a matrix that transforms points from ones that lie on the XY plane (+Z up)
// to points that lie on plane p
void oCalcPlaneMatrix(const float4& _Plane, float4x4* _pMatrix);

template<typename T> inline TVECTOR4<T> oNormalizePlane(const TVECTOR4<T>& _Plane) { T magnitude = length(_Plane.XYZ()); return TVECTOR4<T>(_Plane) / magnitude; }

// Fills the specified array with planes that point inward in the following 
// order: left, right, top, bottom, near, far. The planes will be in whatever
// space _Projection is, so using a concatenated matrix will result in planes in
// that transformed space.
template<typename T> void oCalcFrustumPlanesRH(TVECTOR4<T>* _pPlanes, const TMATRIX4<T>& _Projection, bool _Normalize)
{
	/** <citation
		usage="Adaptation" 
		reason="Simple straightforward paper with code to do this important conversion." 
		author="Gil Gribb & Klaus Hartmann"
		description="http://www2.ravensoft.com/users/ggribb/plane%20extraction.pdf"
		license="*** Assumed Public Domain ***"
		licenseurl="http://www2.ravensoft.com/users/ggribb/plane%20extraction.pdf"
		modification=""
	/>*/

	// $(CitedCodeBegin)

	// Left clipping plane
	_pPlanes[0].x = _Projection.Column0.w + _Projection.Column0.x;
	_pPlanes[0].y = _Projection.Column1.w + _Projection.Column1.x;
	_pPlanes[0].z = _Projection.Column2.w + _Projection.Column2.x;
	_pPlanes[0].w = _Projection.Column3.w + _Projection.Column3.x;

	// Right clipping plane
	_pPlanes[1].x = _Projection.Column0.w - _Projection.Column0.x;
	_pPlanes[1].y = _Projection.Column1.w - _Projection.Column1.x;
	_pPlanes[1].z = _Projection.Column2.w - _Projection.Column2.x;
	_pPlanes[1].w = _Projection.Column3.w - _Projection.Column3.x;

	// Top clipping plane
	_pPlanes[2].x = _Projection.Column0.w - _Projection.Column0.y;
	_pPlanes[2].y = _Projection.Column1.w - _Projection.Column1.y;
	_pPlanes[2].z = _Projection.Column2.w - _Projection.Column2.y;
	_pPlanes[2].w = _Projection.Column3.w - _Projection.Column3.y;

	// Bottom clipping plane
	_pPlanes[3].x = _Projection.Column0.w + _Projection.Column0.y;
	_pPlanes[3].y = _Projection.Column1.w + _Projection.Column1.y;
	_pPlanes[3].z = _Projection.Column2.w + _Projection.Column2.y;
	_pPlanes[3].w = _Projection.Column3.w + _Projection.Column3.y;

	// Near clipping plane
	_pPlanes[4].x = _Projection.Column0.w + _Projection.Column0.z;
	_pPlanes[4].y = _Projection.Column1.w + _Projection.Column1.z;
	_pPlanes[4].z = _Projection.Column2.w + _Projection.Column2.z;
	_pPlanes[4].w = _Projection.Column3.w + _Projection.Column3.z;

	// Far clipping plane
	_pPlanes[5].x = _Projection.Column0.w - _Projection.Column0.z;
	_pPlanes[5].y = _Projection.Column1.w - _Projection.Column1.z;
	_pPlanes[5].z = _Projection.Column2.w - _Projection.Column2.z;
	_pPlanes[5].w = _Projection.Column3.w - _Projection.Column3.z;

	if (_Normalize)
		for (size_t i = 0; i < 6; i++)
			_pPlanes[i] = oNormalizePlane(_pPlanes[i]);

	// $(CitedCodeEnd)
}

void oTransformPoints(const float4x4& _Matrix, float3* oRESTRICT _pDestination, unsigned int _DestinationStride, const float3* oRESTRICT _pSource, unsigned int _SourceStride, unsigned int _NumPoints);
void oTransformVectors(const float4x4& _Matrix, float3* oRESTRICT _pDestination, unsigned int _DestinationStride, const float3* oRESTRICT _pSource, unsigned int _SourceStride, unsigned int _NumVectors);

inline unsigned char oUNORMAsUBYTE(float x) { return static_cast<unsigned char>(floor(x * 255.0f + 0.5f)); }
inline unsigned short oUNORMAsUSHORT(float x) { return static_cast<unsigned char>(floor(x * 65535.0f + 0.5f)); }
inline float oUBYTEAsUNORM(size_t c) { return (c & 0xff) / 255.0f; }
inline float oUSHORTAsUNORM(size_t c) { return (c & 0xffff) / 65535.0f; }

template<typename T> inline T fresnel(const TVECTOR3<T>& i, const TVECTOR3<T>& n) { return 0.02f+0.97f*pow((1-max(dot(i, n))),5); } // http://habibs.wordpress.com/alternative-solutions/
template<typename T> inline T angle(const TVECTOR3<T>& a, const TVECTOR3<T>& b) { return acos(dot(a, b) / (length(a) * length(b))); }

// Signed distance from a plane in Ax + By + Cz + D = 0 format (ABC = normalized normal, D = offset)
// This assumes the plane is normalized.
// >0 means on the same side as the normal
// <0 means on the opposite side as the normal
// 0 means on the plane
template<typename T> inline T sdistance(const TVECTOR4<T>& _Plane, const TVECTOR3<T>& _Point) { return dot(_Plane.XYZ(), _Point) + _Plane.w; }
template<typename T> inline T distance(const TVECTOR4<T>& _Plane, const TVECTOR3<T>& _Point) { return abs(sdistance(_Plane, _Point)); }

inline float4 oCreatePlane(const float3& _PlaneNormal, const float3& _Point) { float3 n = normalize(_PlaneNormal); return float4(n, dot(n, _Point)); }
inline float4 oNormalizePlane(const float4& _Plane) { float invLength = rsqrt(dot(_Plane.XYZ(), _Plane.XYZ())); return _Plane * invLength; }

bool oCalculateAreaAndCentriod(float* _pArea, float2* _pCentroid, const float2* _pVertices, size_t _VertexStride, size_t _NumVertices);

// TODO: Add calc the line of intersection
// bool intersects(float3& linePt, float3& lineDir, const planef& plane1) const
bool oIntersects(float3* _pIntersection, const float4& _Plane0, const float4& _Plane1, const float4& _Plane2);

// Calculate the point on this plane where the line segment
// described by p0 and p1 intersects.
bool oIntersects(float3* _pIntersection, const float4& _Plane, const float3& _Point0, const float3& _Point1);

// Determines a location in 3D space based on 4 reference locations and their distances from the location
template<typename T> T oTrilaterate(const TVECTOR3<T> observers[4], const T distances[4], TVECTOR3<T>* position);
template<typename T>
inline T oTrilaterate(const TVECTOR3<T> observers[4], T distance, TVECTOR3<T>* position)
{
	T distances[4];
	for(int i = 0; i < 4; ++i)
		distances[i] = distance;
	return oTrilaterate(observers, distances, position);	
}
// Computes a matrix to move from one coordinate system to another based on 4 known reference locations in the start and end systems assuming uniform units
template<typename T> bool oCoordinateTransform(const TVECTOR3<T> startCoords[4], const TVECTOR3<T> endCoords[4], TMATRIX4<T> *matrix);

// Computes the gaussian weight of a specific sample in a 1D kernel 
inline float GaussianWeight(float stdDeviation, int sampleIndex)
{
	return (1.0f / (sqrt(2.0f * oPIf) * stdDeviation)) * pow(oEf, -((float)(sampleIndex * sampleIndex) / (2.0f * stdDeviation * stdDeviation)));
}

// _____________________________________________________________________________
// More complex math structures

template<typename T, typename TVec> class TAABOX
{
public:
	TAABOX() : Min(oNumericLimits<T>::GetMax()), Max(oNumericLimits<T>::GetMin()){}
	TAABOX(const TAABOX<T,TVec>& box) : Min(box.Min), Max(box.Max) {}
	TAABOX(const TVec& _Min, const TVec& _Max) : Min(_Min), Max(_Max) {}
	inline const TAABOX<T,TVec>& operator=(const TVec& _Box) { Min = box.Min; Max = box.Max; return *this; }
	inline const TVec& GetMin() const { return Min; }
	inline const TVec& GetMax() const { return Max; }
	inline void SetMin(const TVec& _Min) { Min = _Min; }
	inline void SetMax(const TVec& _Max) { Max = _Max; }
	inline bool IsEmpty() const { return less_than_equal(Max, Min); } 
	inline void Empty() { Min = TVec(oNumericLimits<T>::GetMax()); Max = TVec(oNumericLimits<T>::GetMin()); }
	inline TVec GetCenter() const { return Min + (Max - Min) / T(2.0f); }
	inline T GetBoundingRadius() const { return length(Max-Min) / T(2.0f); }
	inline void GetDimensions(T* _pWidth, T* _pHeight) const { *_pWidth = Max.x - Min.x; *_pHeight = Max.y - Min.y; }
	inline void GetDimensions(T* _pWidth, T* _pHeight, T* _pDepth) const { *_pWidth = Max.x - Min.x; *_pHeight = Max.y - Min.y; *_pDepth = Max.z - Min.z; }
	inline void Transform(const TMATRIX4<T>& _Matrix) { Min = _Matrix * Min; Max = _Matrix * Max; }
	inline void ExtendBy(const TVec& _Point) { Min = oMin(Min, _Point); Max = oMax(Max, _Point); }
	inline void ExtendBy(const TAABOX<T,TVec>& _Box) { ExtendBy(_Box.Min); ExtendBy(_Box.Max); }
protected:
	TVec Min;
	TVec Max;
};

typedef TAABOX<float, TVECTOR3<float>> oAABoxf;
typedef TAABOX<double, TVECTOR3<float>> oAABoxd;
typedef TAABOX<int, TVECTOR2<int>> oRECT;

// Takes a rectangle and breaks it into _MaxNumSplits rectangles
// where each rectangle's area is a % of the source rectangle approximated 
// by its value in _pOrderedSplitRatio.  The sum of the values in _pOrderedSplitRatio
// must be 1.0f and decreasing in size.  SplitRect returns the number of splits
// it could do (which may be less than _MaxNumSplits when the ratios are too small)
unsigned int SplitRect(const oRECT& _SrcRect, const unsigned int _MaxNumSplits, const float* _pOrderedSplitRatio, const unsigned int _XMultiple, const unsigned int _YMultiple, oRECT* _pSplitResults);

template<typename T> class TFRUSTUM
{
public:
	enum CORNER
	{
		LEFT_TOP_NEAR,
		LEFT_TOP_FAR,
		LEFT_BOTTOM_NEAR,
		LEFT_BOTTOM_FAR,
		RIGHT_TOP_NEAR,
		RIGHT_TOP_FAR,
		RIGHT_BOTTOM_NEAR,
		RIGHT_BOTTOM_FAR,
	};

	TVECTOR4<T> Left;
	TVECTOR4<T> Right;
	TVECTOR4<T> Top;
	TVECTOR4<T> Bottom;
	TVECTOR4<T> Near;
	TVECTOR4<T> Far;

	TFRUSTUM() {}

	// This frustum will be in whatever space the matrix was in... so if solely a 
	// projection matrix, then the frustum will be in projection space. If a world-
	// view-projection matrix is specified, then the frustum will be in world-view
	// space.
	TFRUSTUM(const TMATRIX4<T>& _Projection) { oCalcFrustumPlanesRH(&Left, _Projection, true); }
	const TFRUSTUM<T>& operator=(const TMATRIX4<T>& _Projection) { oCalcFrustumPlanesRH(&Left, _Projection, true); return *this; }

	inline const TVECTOR4<T>& GetPlane(unsigned int _Index) const { return (&Near)[_Index]; }

	// Returns true if values are valid or false if planes don't meet in 8 corners.
	bool CalcCorners(TVECTOR3<T>* _pCorners) const
	{
		// @oooii-tony: TODO implement oIntersects for double
		bool isect = oIntersects(_pCorners[LEFT_TOP_NEAR], Left, Top, Near);
		isect = isect && oIntersects(_pCorners[LEFT_TOP_FAR], Left, Top, Far);
		isect = isect && oIntersects(_pCorners[LEFT_BOTTOM_NEAR], Left, Bottom, Near);
		isect = isect && oIntersects(_pCorners[LEFT_BOTTOM_FAR], Left, Bottom, Far);
		isect = isect && oIntersects(_pCorners[RIGHT_TOP_NEAR], Right, Top, Near);
		isect = isect && oIntersects(_pCorners[RIGHT_TOP_FAR], Right, Top, Far);
		isect = isect && oIntersects(_pCorners[RIGHT_BOTTOM_NEAR], Right, Bottom, Near);
		isect = isect && oIntersects(_pCorners[RIGHT_BOTTOM_FAR], Right, Bottom, Far);
		return isect;
	}
};

typedef TFRUSTUM<float> oFrustumf;
//typedef TFRUSTUM<double> oFrustumd; // @oooii-tony: Need an oIntersects for double

// Returns -1 if the frustum partially contains the box, 0 if not at all contained,
// and 1 if the box is wholly inside the frustum.
template<typename T> int oContains(const TFRUSTUM<T>& _Frustum, const TAABOX<T,TVECTOR3<T>>& _Box);

#endif
