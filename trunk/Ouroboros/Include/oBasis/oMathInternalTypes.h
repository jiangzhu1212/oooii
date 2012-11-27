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
// This is an internal header that contains the macros and templates for 
// defining math types such as TVecors and matrices. This is not intended to be 
// used publicly, or even included outside oMath.h. Use the typedefs found in 
// oMath.h for these types. This is separated out from oMath.h to allow for a 
// more readable public oMath.h header.
#pragma once
#ifndef oMathInternalTypes_h
#define oMathInternalTypes_h

#include <oBasis/oEqual.h>
#include <oBasis/oLimits.h>
#include <unordered_map>

// _____________________________________________________________________________
// Internal macros used in the definition of tuples below

#define oMATH_CONCAT(a,b) a##b

#define oMATH_MEMBER_OP(return_t, param_t, op) inline const return_t& operator op##=(const param_t& a) { *this = *this op a; return *this; }
#define oMATH_MEMBER_EQOP(param_t, op) bool operator op(const param_t& a) const { return ::operator op(*this, a); }
#define oMATH_ELOP2(op) template<typename T> oMathNS::TVEC2<T> operator op(const oMathNS::TVEC2<T>& a, const oMathNS::TVEC2<T>& b) { return oMathNS::TVEC2<T>(a.x op b.x, a.y op b.y); } template<typename T> oMathNS::TVEC2<T> operator op(const oMathNS::TVEC2<T>& a, const T& b) { return oMathNS::TVEC2<T>(a.x op b, a.y op b); } template<typename T> oMathNS::TVEC2<T> operator op(const T& a, const oMathNS::TVEC2<T>& b) { return oMathNS::TVEC2<T>(a op b.x, a op b.y); }
#define oMATH_ELOP3(op) template<typename T> oMathNS::TVEC3<T> operator op(const oMathNS::TVEC3<T>& a, const oMathNS::TVEC3<T>& b) { return oMathNS::TVEC3<T>(a.x op b.x, a.y op b.y, a.z op b.z); } template<typename T> oMathNS::TVEC3<T> operator op(const oMathNS::TVEC3<T>& a, const T& b) { return oMathNS::TVEC3<T>(a.x op b, a.y op b, a.z op b); } template<typename T> oMathNS::TVEC3<T> operator op(const T& a, const oMathNS::TVEC3<T>& b) { return oMathNS::TVEC3<T>(a op b.x, a op b.y, a op b.z); }
#define oMATH_ELOP4(op) template<typename T> oMathNS::TVEC4<T> operator op(const oMathNS::TVEC4<T>& a, const oMathNS::TVEC4<T>& b) { return oMathNS::TVEC4<T>(a.x op b.x, a.y op b.y, a.z op b.z, a.w op b.w); } template<typename T> oMathNS::TVEC4<T> operator op(const oMathNS::TVEC4<T>& a, const T& b) { return oMathNS::TVEC4<T>(a.x op b, a.y op b, a.z op b, a.w op b); } template<typename T> oMathNS::TVEC4<T> operator op(const T& a, const oMathNS::TVEC4<T>& b) { return oMathNS::TVEC4<T>(a op b.x, a op b.y, a op b.z, a op b.w); }
#define oMATH_ELOPT2(op, param_t) template<typename T> oMathNS::TVEC2<T> operator op(const oMathNS::TVEC2<T>& a, param_t b) { return oMathNS::TVEC2<T>(a.x op b, a.y op b); } 
#define oMATH_ELOPT3(op, param_t) template<typename T> oMathNS::TVEC3<T> operator op(const oMathNS::TVEC3<T>& a, param_t b) { return oMathNS::TVEC3<T>(a.x op b, a.y op b, a.z op b); } 
#define oMATH_ELOPT4(op, param_t) template<typename T> oMathNS::TVEC4<T> operator op(const oMathNS::TVEC4<T>& a, param_t b) { return oMathNS::TVEC4<T>(a.x op b, a.y op b, a.z op b, a.w op b); } 
#define oMATH_ELUFN2(fn) template<typename T> oMathNS::TVEC2<T> fn(const oMathNS::TVEC2<T>& a) { return oMathNS::TVEC2<T>(fn(a.x), fn(a.y)); }
#define oMATH_ELUFN3(fn) template<typename T> oMathNS::TVEC3<T> fn(const oMathNS::TVEC3<T>& a) { return oMathNS::TVEC3<T>(fn(a.x), fn(a.y), fn(a.z)); }
#define oMATH_ELUFN4(fn) template<typename T> oMathNS::TVEC4<T> fn(const oMathNS::TVEC4<T>& a) { return oMathNS::TVEC3<T>(fn(a.x), fn(a.y), fn(a.z), fn(a.w)); }
#define oMATH_ELBFN2(pubfn, implfn) template<typename T> oMathNS::TVEC2<T> pubfn(const oMathNS::TVEC2<T>& a, const oMathNS::TVEC2<T>& b) { return oMathNS::TVEC2<T>(implfn(a.x, b.x), implfn(a.y, b.y)); }
#define oMATH_ELBFN3(pubfn, implfn) template<typename T> oMathNS::TVEC3<T> pubfn(const oMathNS::TVEC3<T>& a, const oMathNS::TVEC3<T>& b) { return oMathNS::TVEC3<T>(implfn(a.x, b.x), implfn(a.y, b.y), implfn(a.z, b.z)); }
#define oMATH_ELBFN4(pubfn, implfn) template<typename T> oMathNS::TVEC4<T> pubfn(const oMathNS::TVEC4<T>& a, const oMathNS::TVEC4<T>& b) { return oMathNS::TVEC4<T>(implfn(a.x, b.x), implfn(a.y, b.y), implfn(a.z, b.z), implfn(a.w, b.w)); }
#define oMATH_EQ2() template<typename T> bool operator==(const oMathNS::TVEC2<T>& a, const oMathNS::TVEC2<T>& b) { return oEqual(a.x, b.x) && oEqual(a.y, b.y); }
#define oMATH_EQ3() template<typename T> bool operator==(const oMathNS::TVEC3<T>& a, const oMathNS::TVEC3<T>& b) { return oEqual(a.x, b.x) && oEqual(a.y, b.y) && oEqual(a.z, b.z); }
#define oMATH_EQ4() template<typename T> bool operator==(const oMathNS::TVEC4<T>& a, const oMathNS::TVEC4<T>& b) { return oEqual(a.x, b.x) && oEqual(a.y, b.y) && oEqual(a.z, b.z) && oEqual(a.w, b.w); }
#define oMATH_NEQ(type) template<typename T> bool operator!=(const type& a, const type& b) { return !(a == b); }
#define oMATH_CMP2(fn,cmp) template<typename T> bool fn(const oMathNS::TVEC2<T>& a, const oMathNS::TVEC2<T>& b) { return a.x cmp b.x && a.y cmp b.y; }
#define oMATH_CMP3(fn,cmp) template<typename T> bool fn(const oMathNS::TVEC3<T>& a, const oMathNS::TVEC3<T>& b) { return a.x cmp b.x && a.y cmp b.y && a.z cmp b.z; }
#define oMATH_CMP4(fn,cmp) template<typename T> bool fn(const oMathNS::TVEC4<T>& a, const oMathNS::TVEC4<T>& b) { return a.x cmp b.x && a.y cmp b.y && a.z cmp b.z && a.w cmp b.w; }
#define oMATH_ANYCMP2(fn,cmp) template<typename T> bool fn(const oMathNS::TVEC2<T>& a, const oMathNS::TVEC2<T>& b) { return a.x cmp b.x || a.y cmp b.y; }
#define oMATH_ANYCMP3(fn,cmp) template<typename T> bool fn(const oMathNS::TVEC3<T>& a, const oMathNS::TVEC3<T>& b) { return a.x cmp b.x || a.y cmp b.y || a.z cmp b.z; }
#define oMATH_ANYCMP4(fn,cmp) template<typename T> bool fn(const oMathNS::TVEC4<T>& a, const oMathNS::TVEC4<T>& b) { return a.x cmp b.x || a.y cmp b.y || a.z cmp b.z || a.w cmp b.w; }
// Macros to get through the boilerplate for operators, compares, etc.
#define oMATH_MEMBER_OPS(type, scalar_t) oMATH_MEMBER_EQOP(type, ==) oMATH_MEMBER_EQOP(type, !=) oMATH_MEMBER_OP(type, scalar_t, *) oMATH_MEMBER_OP(type, scalar_t, /) oMATH_MEMBER_OP(type, scalar_t, +) oMATH_MEMBER_OP(type, scalar_t, -) oMATH_MEMBER_OP(type, scalar_t, &) oMATH_MEMBER_OP(type, scalar_t, |) oMATH_MEMBER_OP(type, scalar_t, ^) oMATH_MEMBER_OP(type, scalar_t, <<) oMATH_MEMBER_OP(type, scalar_t, >>) oMATH_MEMBER_OP(type, type, *) oMATH_MEMBER_OP(type, type, /) oMATH_MEMBER_OP(type, type, +) oMATH_MEMBER_OP(type, type, -) oMATH_MEMBER_OP(type, type, &) oMATH_MEMBER_OP(type, type, |) oMATH_MEMBER_OP(type, type, ^) oMATH_MEMBER_OP(type, type, <<) oMATH_MEMBER_OP(type, type, >>) oMATH_VBRACKET_OP(scalar_t)
#define oMATH_ELOPS(N) oMATH_ELOP##N(*) oMATH_ELOP##N(/) oMATH_ELOP##N(+) oMATH_ELOP##N(-) oMATH_ELOP##N(%) oMATH_ELOPT##N(<<, int) oMATH_ELOPT##N(>>, int) oMATH_ELOPT##N(&, int) oMATH_ELOPT##N(|, int) oMATH_ELOPT##N(^, int)
#define oMATH_ELUFNS(fn) oMATH_ELUFN2(fn) oMATH_ELUFN3(fn) oMATH_ELUFN4(fn)
#define oMATH_ELBFNS(pubfn, implfn) oMATH_ELBFN2(pubfn, implfn) oMATH_ELBFN3(pubfn, implfn) oMATH_ELBFN4(pubfn, implfn)
// NOTE: This does not define operators because of the ambiguity between any and all, but rather the named functions in the macros (i.e. any_greater_than)
#define oMATH_CMP(num) oMATH_NEQ(oMATH_CONCAT(oMathNS::TVEC, num)<T>) oMATH_CONCAT(oMATH_EQ, num)() \
	oMATH_CONCAT(oMATH_CMP, num)(less_than, <) oMATH_CONCAT(oMATH_CMP, num)(greater_than, >)  oMATH_CONCAT(oMATH_CMP, num)(less_than_equal, <=)  oMATH_CONCAT(oMATH_CMP, num)(greater_than_equal, >=) \
	oMATH_CONCAT(oMATH_ANYCMP, num)(any_less_than, <) oMATH_CONCAT(oMATH_ANYCMP, num)(any_greater_than, >)  oMATH_CONCAT(oMATH_ANYCMP, num)(any_less_than_equal, <=) oMATH_CONCAT(oMATH_ANYCMP, num)(any_greater_than_equal, >=) oMATH_CONCAT(oMATH_ANYCMP, num)(any_equal, ==)
#define oMATH_VBRACKET_OP(return_t) const return_t& operator[](size_t i) const { return *(&x + i); } return_t& operator[](size_t i) { return *(&x + i); }
#define oMATH_MBRACKET_OP(return_t) const return_t& operator[](size_t i) const { return *(&Column0 + i); } return_t& operator[](size_t i) { return *(&Column0 + i); }

#define oMATH_SW2__(a,b) inline const TVEC2<T> a##b() const { return TVEC2<T>(a,b); }
#define oMATH_SWIZZLE2(type) oMATH_SW2__(x,x) oMATH_SW2__(x,y) oMATH_SW2__(y,x) oMATH_SW2__(y,y)

#define oMATH_SW3__(a,b,c) inline const TVEC3<T> a##b##c() const { return TVEC3<T>(a,b,c); }
#define oMATH_SWIZZLE3(type) oMATH_SWIZZLE2(type) oMATH_SW2__(y,z) oMATH_SW2__(z,y) oMATH_SW2__(x,z) oMATH_SW2__(z,x) oMATH_SW2__(z,z) \
	oMATH_SW3__(x,x,x) oMATH_SW3__(x,x,y) oMATH_SW3__(x,x,z) oMATH_SW3__(x,y,x) oMATH_SW3__(x,y,y) oMATH_SW3__(x,y,z) oMATH_SW3__(x,z,x) \
	oMATH_SW3__(x,z,y) oMATH_SW3__(x,z,z) oMATH_SW3__(y,x,x) oMATH_SW3__(y,x,y) oMATH_SW3__(y,x,z) oMATH_SW3__(y,y,x) oMATH_SW3__(y,y,y) \
	oMATH_SW3__(y,y,z) oMATH_SW3__(y,z,x) oMATH_SW3__(y,z,y) oMATH_SW3__(y,z,z) oMATH_SW3__(z,x,x) oMATH_SW3__(z,x,y) oMATH_SW3__(z,x,z) \
	oMATH_SW3__(z,y,x) oMATH_SW3__(z,y,y) oMATH_SW3__(z,y,z) oMATH_SW3__(z,z,x) oMATH_SW3__(z,z,y) oMATH_SW3__(z,z,z)
	
// @oooii-tony: todo: Add the rest of the swizzle3 with W permutations and swizzle4 permutations
#define oMATH_SWIZZLE4(type) oMATH_SWIZZLE3(type) \
	oMATH_SW2__(z,w) oMATH_SW2__(w,z) oMATH_SW2__(x,w) oMATH_SW2__(w,x) oMATH_SW2__(y,w) oMATH_SW2__(w,y) oMATH_SW2__(w,w) \
	oMATH_SW3__(w,w,w) oMATH_SW3__(y,z,w) \
	inline const TVEC4<T>& xyzw() const { return *(TVEC4<T>*)this; }

// Wrap root types in a namespace so that NoStepInto regex's can be used to 
// avoid stepping into trivial ctors and operators. Use a very short name 
// because we've run into name truncation warnings especially when using 
// std::bind.
namespace oMathNS {

template<typename T> struct TVEC2
{
	union { T x; T r; };
	union { T y; T g; };
	inline TVEC2() {}
	inline TVEC2(const TVEC2& _TVector) : x(_TVector.x), y(_TVector.y) {}
	template<typename U> TVEC2(const TVEC2<U>& _UVector) : x(static_cast<T>(_UVector.x)), y(static_cast<T>(_UVector.y)) {}
	template<typename U> explicit TVEC2(U _XY) { x = y = static_cast<T>(_XY); }
	inline TVEC2(T _XY) : x(_XY), y(_XY) {}
	inline TVEC2(T _X, T _Y) : x(_X), y(_Y) {}
	oMATH_SWIZZLE2(T);
	oMATH_MEMBER_OPS(TVEC2<T>, T);
};

template<typename T> struct TVEC3
{
	union { T x; T r; };
	union { T y; T g; };
	union { T z; T b; };
	inline TVEC3() {};
	inline TVEC3(const TVEC3& _TVector) : x(_TVector.x), y(_TVector.y), z(_TVector.z) {}
	template<typename U> TVEC3(const TVEC3<U>& _UVector) : x(static_cast<T>(_UVector.x)), y(static_cast<T>(_UVector.y)), z(static_cast<T>(_UVector.z)) {}
	template<typename U> explicit TVEC3(U _XYZ) { x = y = z = static_cast<T>(_XYZ); }
	inline TVEC3(T _XYZ) : x(_XYZ), y(_XYZ), z(_XYZ) {}
	inline TVEC3(T _X, T _Y, T _Z) : x(_X), y(_Y), z(_Z) {}
	inline TVEC3(const TVEC2<T>& _XY, T _Z) : x(_XY.x), y(_XY.y), z(_Z) {}
	oMATH_SWIZZLE3(T);
	oMATH_MEMBER_OPS(TVEC3<T>, T);
};

template<typename T> struct TVEC4
{
	union { T x; T r; };
	union { T y; T g; };
	union { T z; T b; };
	union { T w; T a; };
	inline TVEC4() {};
	inline TVEC4(const TVEC4& _TVector) : x(_TVector.x), y(_TVector.y), z(_TVector.z), w(_TVector.w) {}
	template<typename U> TVEC4(const TVEC4<U>& _UVector) : x(static_cast<T>(_UVector.x)), y(static_cast<T>(_UVector.y)), z(static_cast<T>(_UVector.z)), w(static_cast<T>(_UVector.w)) {}
	template<typename U> explicit TVEC4(U _XYZW) { x = y = z = w = static_cast<T>(_XYZW); }
	inline TVEC4(T _XYZW) : x(_XYZW), y(_XYZW), z(_XYZW), w(_XYZW) {}
	inline TVEC4(const TVEC2<T>& _XY, T _Z, T _W) : x(_XY.x), y(_XY.y), z(_Z), w(_Z) {}
	inline TVEC4(const TVEC3<T>& _XYZ, T _W) : x(_XYZ.x), y(_XYZ.y), z(_XYZ.z), w(_W) {}
	inline TVEC4(const TVEC2<T>& _XY, const TVEC2<T>& _ZW) : x(_XY.x), y(_XY.y), z(_ZW.z), w(_ZW.w) {}
	inline TVEC4(T _X, const TVEC3<T>& _YZW) : x(_X), y(_YZW.y), z(_YZW.z), w(_YZW.w) {}
	inline TVEC4(T _X, T _Y, T _Z, T _W) : x(_X), y(_Y), z(_Z), w(_W) {}
	oMATH_SWIZZLE4(T);
	inline TVEC3<T>& xyz() { return *(TVEC3<T>*)&x; }
	oMATH_MEMBER_OPS(TVEC4<T>, T);
};

template<typename T> struct TQUAT
{
	T x,y,z,w;
	inline TQUAT() {};
	inline TQUAT(const TQUAT& _Quaternion) : x(_Quaternion.x), y(_Quaternion.y), z(_Quaternion.z), w(_Quaternion.w) {}
	inline TQUAT(T _X, T _Y, T _Z, T _W) : x(_X), y(_Y), z(_Z), w(_W) {}
	oMATH_VBRACKET_OP(T);
	inline const TVEC3<T>& xyz() const { return *(TVEC3<T>*)&x; }
	inline const TVEC4<T>& xyzw() const { return *(TVEC4<T>*)&x; }
	static const TQUAT<T> Identity;
};

template<typename T> struct TMAT3
{
	// Column-major 3x3 matrix
	TVEC3<T> Column0;
	TVEC3<T> Column1;
	TVEC3<T> Column2;
	TMAT3() {}
	TMAT3(const TMAT3& _Matrix) : Column0(_Matrix.Column0), Column1(_Matrix.Column1), Column2(_Matrix.Column2) {}
	TMAT3(const TVEC3<T>& _Column0, const TVEC3<T>& _Column1, const TVEC3<T>& _Column2) : Column0(_Column0), Column1(_Column1), Column2(_Column2) {}
	oMATH_MBRACKET_OP(TVEC3<T>);
	static const TMAT3<T> Identity;
};

template<typename T> struct TMAT4
{
	// Column-major 4x4 matrix
	TVEC4<T> Column0;
	TVEC4<T> Column1;
	TVEC4<T> Column2;
	TVEC4<T> Column3;
	TMAT4() {}
	TMAT4(const TMAT4& _Matrix) : Column0(_Matrix.Column0), Column1(_Matrix.Column1), Column2(_Matrix.Column2), Column3(_Matrix.Column3) {}
	TMAT4(const TVEC4<T>& _Column0, const TVEC4<T>& _Column1, const TVEC4<T>& _Column2, const TVEC4<T>& _Column3) : Column0(_Column0), Column1(_Column1), Column2(_Column2), Column3(_Column3) {}
	TMAT4(const TMAT3<T>& _ScaleRotation, const TVEC3<T>& _Translation) : Column0(_ScaleRotation.Column0, 0), Column1(_ScaleRotation.Column1, 0), Column2(_ScaleRotation.Column2, 0), Column3(_Translation, 1.0f) {}
	TMAT4(const TQUAT<T>& _Rotation, const TVEC3<T>& _Translation) { TMAT4<T> r = oCreateRotation(_Rotation); *this = TMAT4(r.GetUpper3x3(), _Translation); }
	TMAT3<T> GetUpper3x3() const { return TMAT3<T>(Column0.xyz(), Column1.xyz(), Column2.xyz()); }
	oMATH_MBRACKET_OP(TVEC4<T>);
	static const TMAT4<T> Identity;
};

template<typename T, typename TVec> struct TAABOX
{
	TAABOX() { Clear(); }
	TAABOX(const TAABOX<T,TVec>& _Box) : Min(_Box.Min), Max(_Box.Max) {}
	TAABOX(const TVec& _Min, const TVec& _Max) : Min(_Min), Max(_Max) {}
	inline const TAABOX<T,TVec>& operator=(const TVec& _Box) { Min = box.Min; Max = box.Max; return *this; }
	inline const TVec& GetMin() const { return Min; }
	inline const TVec& GetMax() const { return Max; }
	inline void SetMin(const TVec& _Min) { Min = _Min; }
	inline void SetMax(const TVec& _Max) { Max = _Max; }
	inline bool IsEmpty() const { return any_less_than(Max, Min); } 
	inline bool IsZero() const { return Max==Min; }
	inline void Clear() { Min = TVec(oNumericLimits<T>::GetMax()); Max = TVec(oNumericLimits<T>::GetSignedMin()); }
	inline TVec GetDimensions() const { return abs(Max - Min); }
	inline TVec GetCenter() const { return Min + GetDimensions() / T(2.0); }
	inline T GetBoundingRadius() const { return length(Max - Min) / T(2.0); }

	inline void GetVertices(TVec vertices[8]) const 
  { 
    vertices[0] = TVec(Min.x, Min.y, Min.z);
    vertices[1] = TVec(Max.x, Min.y, Min.z);
    vertices[2] = TVec(Min.x, Max.y, Min.z);
    vertices[3] = TVec(Max.x, Max.y, Min.z);

    vertices[4] = TVec(Min.x, Min.y, Max.z);
    vertices[5] = TVec(Max.x, Min.y, Max.z);
    vertices[6] = TVec(Min.x, Max.y, Max.z);
    vertices[7] = TVec(Max.x, Max.y, Max.z);
  }

protected:
	TVec Min;
	TVec Max;
};

template<typename T> struct TPLANE : public TVEC4<T>
{
	TPLANE<T>() {}
	TPLANE<T>(const TVEC3<T>& _Normal, const T& _Offset) : TVEC4<T>(normalize(_Normal), _Offset) {}
	TPLANE<T>(const T& _NormalX, const T& _NormalY, const T& _NormalZ, const T& _Offset) : TVEC4<T>(normalize(TVEC3<T>(_NormalX, _NormalY, _NormalZ)), _Offset) {}
	TPLANE<T>(const TVEC3<T>& _Normal, const TVEC3<T> _Point) { xyz() = normalize(_Normal); w = dot(n, _Point); }

	operator TVEC4<T>() { return *this; }
};

template<typename T> struct TFRUSTUM
{
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

  // This order must be maintained
	TVEC4<T> Left;
	TVEC4<T> Right;
	TVEC4<T> Top;
	TVEC4<T> Bottom;
	TVEC4<T> Near;
	TVEC4<T> Far;

	TFRUSTUM() {}

	// This frustum will be in whatever space the matrix was in minus one. This 
	// means in the space conversion of:
	// Model -> World -> View -> Projection that a projection matrix returned by
	// oCreatePerspective?H() will be in view space. A View * Projection will be
	// in world space, and a WVP matrix will be in model space.
	TFRUSTUM(const TMAT4<T>& _Projection) { oExtractFrustumPlanes(&Left, _Projection, true); }
	const TFRUSTUM<T>& operator=(const TMAT4<T>& _Projection) { oExtractFrustumPlanes(&Left, _Projection, true); return *this; }
	inline const TVEC4<T>& GetPlane(size_t _Index) const { return (&Left)[_Index]; }
};

template<typename T> struct TSPHERE : public TVEC4<T>
{
	TSPHERE<T>() {}
	TSPHERE<T>(const TSPHERE<T>& _Sphere) { *this = _Sphere; }
	TSPHERE<T>(const TVEC3<T>& _Position, T _Radius) : TVEC4<T>(_Position, _Radius) {}
	T GetRadius() const { return w; }
	const TVEC3<T>& GetPosition() const { return *(TVEC3<T>*)this; }
};

} // namespace oMathNS

// Useful for min/max concepts where a must always be greater than b, so the
// rest of a calling function can execute normally, and if the values need to 
// be swapped, use this.
// template<typename T> void swap(T& a, T& b) { T c = a; a = b; b = c; }
template<typename T> void swap_if_greater(oMathNS::TVEC2<T>& a, oMathNS::TVEC2<T>& b) { if (a.x > b.x) std::swap(a.x, b.x); if (a.y > b.y) std::swap(a.y, b.y); }
template<typename T> void swap_if_greater(oMathNS::TVEC3<T>& a, oMathNS::TVEC3<T>& b) { if (a.x > b.x) std::swap(a.x, b.x); if (a.y > b.y) std::swap(a.y, b.y); if (a.z > b.z) std::swap(a.z, b.z); }
template<typename T> void swap_if_greater(oMathNS::TVEC4<T>& a, oMathNS::TVEC4<T>& b) { if (a.x > b.x) std::swap(a.x, b.x); if (a.y > b.y) std::swap(a.y, b.y); if (a.z > b.z) std::swap(a.z, b.z); if (a.w > b.w) std::swap(a.w, b.w); }

template<typename T> bool operator==(const oMathNS::TQUAT<T>& a, const oMathNS::TQUAT<T>& b) { return oEqual(a.x, b.x) && oEqual(a.y, b.y) && oEqual(a.z, b.z) && oEqual(a.w, b.w); }
template<typename T> bool operator!=(const oMathNS::TQUAT<T>& a, const oMathNS::TQUAT<T>& b) { return !(a == b); }

template<typename T> oMathNS::TVEC2<T> operator-(const oMathNS::TVEC2<T>& a) { return oMathNS::TVEC2<T>(-a.x, -a.y); }
template<typename T> oMathNS::TVEC3<T> operator-(const oMathNS::TVEC3<T>& a) { return oMathNS::TVEC3<T>(-a.x, -a.y, -a.z); }
template<typename T> oMathNS::TVEC4<T> operator-(const oMathNS::TVEC4<T>& a) { return oMathNS::TVEC4<T>(-a.x, -a.y, -a.z, -a.w); }
template<typename T> oMathNS::TVEC3<T> operator*(const oMathNS::TMAT3<T>& a, const oMathNS::TVEC3<T>& b) { return mul(a, b); }
template<typename T> oMathNS::TMAT3<T> operator*(const oMathNS::TMAT3<T>& a, const oMathNS::TMAT3<T>& b) { return mul(a, b); }
template<typename T> oMathNS::TVEC3<T> operator*(const oMathNS::TMAT4<T>& a, const oMathNS::TVEC3<T>& b) { return mul(a, b); }
template<typename T> oMathNS::TVEC4<T> operator*(const oMathNS::TMAT4<T>& a, const oMathNS::TVEC4<T>& b) { return mul(a, b); }
template<typename T> oMathNS::TMAT4<T> operator*(const oMathNS::TMAT4<T>& a, const oMathNS::TMAT4<T>& b) { return mul(a, b); }
template<typename T> oMathNS::TQUAT<T> operator*(const oMathNS::TQUAT<T>&a, const oMathNS::TQUAT<T>& b) { return mul(a, b); }

// The namespace is really only to enable NoStepInto settings in MSVC, so don't
// pollute the rest of the headers with the requirement of namespace specification.
using namespace oMathNS;

oMATH_ELOPS(2) oMATH_ELOPS(3) oMATH_ELOPS(4)
oMATH_CMP(2) oMATH_CMP(3) oMATH_CMP(4)

inline bool oEqual(const oMathNS::TVEC2<float>& a, const oMathNS::TVEC2<float>& b, int maxUlps = oDEFAULT_ULPS) { return oEqual(a.x, b.x, maxUlps) && oEqual(a.y, b.y, maxUlps); }
inline bool oEqual(const oMathNS::TVEC3<float>& a, const oMathNS::TVEC3<float>& b, int maxUlps = oDEFAULT_ULPS) { return oEqual(a.x, b.x, maxUlps) && oEqual(a.y, b.y, maxUlps) && oEqual(a.z, b.z, maxUlps); }
inline bool oEqual(const oMathNS::TVEC4<float>& a, const oMathNS::TVEC4<float>& b, int maxUlps = oDEFAULT_ULPS) { return oEqual(a.x, b.x, maxUlps) && oEqual(a.y, b.y, maxUlps) && oEqual(a.z, b.z, maxUlps) && oEqual(a.w, b.w, maxUlps); }
inline bool oEqual(const oMathNS::TQUAT<float>& a, const oMathNS::TQUAT<float>& b, int maxUlps = oDEFAULT_ULPS) { return oEqual(a.x, b.x, maxUlps) && oEqual(a.y, b.y, maxUlps) && oEqual(a.z, b.z, maxUlps) && oEqual(a.w, b.w, maxUlps); }
inline bool oEqual(const oMathNS::TMAT3<float>& a, const oMathNS::TMAT3<float>& b, int maxUlps = oDEFAULT_ULPS) { return oEqual(a.Column0, b.Column0, maxUlps) && oEqual(a.Column1, b.Column1, maxUlps) && oEqual(a.Column2, b.Column2, maxUlps); }
inline bool oEqual(const oMathNS::TMAT4<float>& a, const oMathNS::TMAT4<float>& b, int maxUlps = oDEFAULT_ULPS) { return oEqual(a.Column0, b.Column0, maxUlps) && oEqual(a.Column1, b.Column1, maxUlps) && oEqual(a.Column2, b.Column2, maxUlps) && oEqual(a.Column3, b.Column3, maxUlps); }
inline bool oEqual(const oMathNS::TVEC2<double>& a, const oMathNS::TVEC2<double>& b, int maxUlps = oDEFAULT_ULPS) { return oEqual(a.x, b.x, maxUlps) && oEqual(a.y, b.y, maxUlps); }
inline bool oEqual(const oMathNS::TVEC3<double>& a, const oMathNS::TVEC3<double>& b, int maxUlps = oDEFAULT_ULPS) { return oEqual(a.x, b.x, maxUlps) && oEqual(a.y, b.y, maxUlps) && oEqual(a.z, b.z, maxUlps); }
inline bool oEqual(const oMathNS::TVEC4<double>& a, const oMathNS::TVEC4<double>& b, int maxUlps = oDEFAULT_ULPS) { return oEqual(a.x, b.x, maxUlps) && oEqual(a.y, b.y, maxUlps) && oEqual(a.z, b.z, maxUlps) && oEqual(a.w, b.w, maxUlps); }
inline bool oEqual(const oMathNS::TQUAT<double>& a, const oMathNS::TQUAT<double>& b, int maxUlps = oDEFAULT_ULPS) { return oEqual(a.x, b.x, maxUlps) && oEqual(a.y, b.y, maxUlps) && oEqual(a.z, b.z, maxUlps) && oEqual(a.w, b.w, maxUlps); }
inline bool oEqual(const oMathNS::TMAT3<double>& a, const oMathNS::TMAT3<double>& b, int maxUlps = oDEFAULT_ULPS) { return oEqual(a.Column0, b.Column0, maxUlps) && oEqual(a.Column1, b.Column1, maxUlps) && oEqual(a.Column2, b.Column2, maxUlps); }
inline bool oEqual(const oMathNS::TMAT4<double>& a, const oMathNS::TMAT4<double>& b, int maxUlps = oDEFAULT_ULPS) { return oEqual(a.Column0, b.Column0, maxUlps) && oEqual(a.Column1, b.Column1, maxUlps) && oEqual(a.Column2, b.Column2, maxUlps) && oEqual(a.Column3, b.Column3, maxUlps); }

//support for use as keys in unordered_map, could probably use a better hash
namespace std {
	template <typename T>
	struct hash<TVEC2<T>>
	{
		size_t operator()(const TVEC2<T>& _Vec ) const
		{
			return hash<T>()(_Vec.x) ^ hash<T>()(_Vec.y);
		}
	};
}

namespace std {
	template <typename T>
	struct hash<TVEC3<T>>
	{
		size_t operator()(const TVEC3<T>& _Vec ) const
		{
			return hash<T>()(_Vec.x) ^ hash<T>()(_Vec.y) ^ hash<T>()(_Vec.z);
		}
	};
}

namespace std {
	template <typename T>
	struct hash<TVEC4<T>>
	{
		size_t operator()(const TVEC4<T>& _Vec ) const
		{
			return hash<T>()(_Vec.x) ^ hash<T>()(_Vec.y) ^ hash<T>()(_Vec.z) ^ hash<T>()(_Vec.w);
		}
	};
}

#endif
