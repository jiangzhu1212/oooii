// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.
// This header is designed to cross-compile in both C++ and HLSL. This defines
// the internal parts for defining HLSL simple intrinsic types. The intent is to
// hide this from the main self-documenting header of oHLSLTypes.h so prefer 
// looking there for usage.
#ifndef oHLSL
	#pragma once
#endif
#ifndef oHLSLTypesInternal_h
#define oHLSLTypesInternal_h

// _____________________________________________________________________________
// Internal macros used in the definition of tuples below

#define oHLSL_CONCAT(a,b) a##b

#define oHLSL_MEMBER_OP(return_t, param_t, op) inline const return_t& operator op##=(const param_t& a) { *this = *this op a; return *this; }
#define oHLSL_ELOPT4_SCALAR(type, op) template<typename T> type<T> operator op(const type<T>& a, const T& b) { return type<T>(a.x op b, a.y op b, a.z op b, a.w op b); } template<typename T> type<T> operator op(const T& a, const type<T>& b) { return type<T>(a op b.x, a op b.y, a op b.z, a op b.w); }
#define oHLSL_ELOPT4_VECTOR(type, op) template<typename T> type<T> operator op(const type<T>& a, const type<T>& b) { return type<T>(a.x op b.x, a.y op b.y, a.z op b.z, a.w op b.w); }
#define oHLSL_ELOP2(op) template<typename T> oHLSL::TVEC2<T> operator op(const oHLSL::TVEC2<T>& a, const oHLSL::TVEC2<T>& b) { return oHLSL::TVEC2<T>(a.x op b.x, a.y op b.y); } template<typename T> oHLSL::TVEC2<T> operator op(const oHLSL::TVEC2<T>& a, const T& b) { return oHLSL::TVEC2<T>(a.x op b, a.y op b); } template<typename T> oHLSL::TVEC2<T> operator op(const T& a, const oHLSL::TVEC2<T>& b) { return oHLSL::TVEC2<T>(a op b.x, a op b.y); }
#define oHLSL_ELOP3(op) template<typename T> oHLSL::TVEC3<T> operator op(const oHLSL::TVEC3<T>& a, const oHLSL::TVEC3<T>& b) { return oHLSL::TVEC3<T>(a.x op b.x, a.y op b.y, a.z op b.z); } template<typename T> oHLSL::TVEC3<T> operator op(const oHLSL::TVEC3<T>& a, const T& b) { return oHLSL::TVEC3<T>(a.x op b, a.y op b, a.z op b); } template<typename T> oHLSL::TVEC3<T> operator op(const T& a, const oHLSL::TVEC3<T>& b) { return oHLSL::TVEC3<T>(a op b.x, a op b.y, a op b.z); }
#define oHLSL_ELOP4(op) oHLSL_ELOPT4_VECTOR(oHLSL::TVEC4, op) oHLSL_ELOPT4_SCALAR(oHLSL::TVEC4, op)
#define oHLSL_ELOPT2(op, param_t) template<typename T> oHLSL::TVEC2<T> operator op(const oHLSL::TVEC2<T>& a, param_t b) { return oHLSL::TVEC2<T>(a.x op b, a.y op b); } 
#define oHLSL_ELOPT3(op, param_t) template<typename T> oHLSL::TVEC3<T> operator op(const oHLSL::TVEC3<T>& a, param_t b) { return oHLSL::TVEC3<T>(a.x op b, a.y op b, a.z op b); } 
#define oHLSL_ELOPT4(op, param_t) template<typename T> oHLSL::TVEC4<T> operator op(const oHLSL::TVEC4<T>& a, param_t b) { return oHLSL::TVEC4<T>(a.x op b, a.y op b, a.z op b, a.w op b); } 
#define oHLSL_MELOPT2(ret, op) TVEC2<ret> operator op(const TVEC2<T>& _That) const { return TVEC2<ret>(x op _That.x, y op _That.y); }
#define oHLSL_MELOPT3(ret, op) TVEC3<ret> operator op(const TVEC3<T>& _That) const { return TVEC3<ret>(x op _That.x, y op _That.y, z op _That.z); }
#define oHLSL_MELOPT4(ret, op) TVEC4<ret> operator op(const TVEC4<T>& _That) const { return TVEC4<ret>(x op _That.x, y op _That.y, z op _That.z, w op _That.w); }
#define oHLSL_ELUFN2(fn) template<typename T> oHLSL::TVEC2<T> fn(const oHLSL::TVEC2<T>& a) { return oHLSL::TVEC2<T>(fn(a.x), fn(a.y)); }
#define oHLSL_ELUFN3(fn) template<typename T> oHLSL::TVEC3<T> fn(const oHLSL::TVEC3<T>& a) { return oHLSL::TVEC3<T>(fn(a.x), fn(a.y), fn(a.z)); }
#define oHLSL_ELUFN4(fn) template<typename T> oHLSL::TVEC4<T> fn(const oHLSL::TVEC4<T>& a) { return oHLSL::TVEC4<T>(fn(a.x), fn(a.y), fn(a.z), fn(a.w)); }
#define oHLSL_ELBFN2(pubfn, implfn) template<typename T> oHLSL::TVEC2<T> pubfn(const oHLSL::TVEC2<T>& a, const oHLSL::TVEC2<T>& b) { return oHLSL::TVEC2<T>(implfn(a.x, b.x), implfn(a.y, b.y)); }
#define oHLSL_ELBFN3(pubfn, implfn) template<typename T> oHLSL::TVEC3<T> pubfn(const oHLSL::TVEC3<T>& a, const oHLSL::TVEC3<T>& b) { return oHLSL::TVEC3<T>(implfn(a.x, b.x), implfn(a.y, b.y), implfn(a.z, b.z)); }
#define oHLSL_ELBFN4(pubfn, implfn) template<typename T> oHLSL::TVEC4<T> pubfn(const oHLSL::TVEC4<T>& a, const oHLSL::TVEC4<T>& b) { return oHLSL::TVEC4<T>(implfn(a.x, b.x), implfn(a.y, b.y), implfn(a.z, b.z), implfn(a.w, b.w)); }
#define oHLSL_CMP2(cmp) template<typename T> oHLSL::TVEC2<bool> operator##cmp(const oHLSL::TVEC2<T>& a, const oHLSL::TVEC2<T>& b) { return oHLSL::TVEC2<bool>(a.x cmp b.x, a.y cmp b.y); }
#define oHLSL_CMP3(cmp) template<typename T> oHLSL::TVEC3<bool> operator##cmp(const oHLSL::TVEC3<T>& a, const oHLSL::TVEC3<T>& b) { return oHLSL::TVEC3<bool>(a.x cmp b.x, a.y cmp b.y, a.z cmp b.z); }
#define oHLSL_CMP4(cmp) template<typename T> oHLSL::TVEC4<bool> operator##cmp(const oHLSL::TVEC4<T>& a, const oHLSL::TVEC4<T>& b) { return oHLSL::TVEC4<bool>(a.x cmp b.x, a.y cmp b.y, a.z cmp b.z, a.w cmp b.w); }
// Macros to get through the boilerplate for operators, compares, etc.
#define oHLSL_MEMBER_OPS(type, scalar_t) oHLSL_MEMBER_OP(type, scalar_t, *) oHLSL_MEMBER_OP(type, scalar_t, /) oHLSL_MEMBER_OP(type, scalar_t, +) oHLSL_MEMBER_OP(type, scalar_t, -) oHLSL_MEMBER_OP(type, scalar_t, &) oHLSL_MEMBER_OP(type, scalar_t, |) oHLSL_MEMBER_OP(type, scalar_t, ^) oHLSL_MEMBER_OP(type, scalar_t, <<) oHLSL_MEMBER_OP(type, scalar_t, >>) oHLSL_MEMBER_OP(type, type, *) oHLSL_MEMBER_OP(type, type, /) oHLSL_MEMBER_OP(type, type, +) oHLSL_MEMBER_OP(type, type, -) oHLSL_MEMBER_OP(type, type, &) oHLSL_MEMBER_OP(type, type, |) oHLSL_MEMBER_OP(type, type, ^) oHLSL_MEMBER_OP(type, type, <<) oHLSL_MEMBER_OP(type, type, >>) oHLSL_VBRACKET_OP(scalar_t)
#define oHLSL_MELOPTS2() oHLSL_MELOPT2(T, +) oHLSL_MELOPT2(T, -) oHLSL_MELOPT2(T, *) oHLSL_MELOPT2(T, /) oHLSL_MELOPT2(T, %) oHLSL_MELOPT2(bool, ==) oHLSL_MELOPT2(bool, !=) oHLSL_MELOPT2(bool, <) oHLSL_MELOPT2(bool, >) oHLSL_MELOPT2(bool, <=) oHLSL_MELOPT2(bool, >=)
#define oHLSL_MELOPTS3() oHLSL_MELOPT3(T, +) oHLSL_MELOPT3(T, -) oHLSL_MELOPT3(T, *) oHLSL_MELOPT3(T, /) oHLSL_MELOPT3(T, %) oHLSL_MELOPT3(bool, ==) oHLSL_MELOPT3(bool, !=) oHLSL_MELOPT3(bool, <) oHLSL_MELOPT3(bool, >) oHLSL_MELOPT3(bool, <=) oHLSL_MELOPT3(bool, >=)
#define oHLSL_MELOPTS4() oHLSL_MELOPT4(T, +) oHLSL_MELOPT4(T, -) oHLSL_MELOPT4(T, *) oHLSL_MELOPT4(T, /) oHLSL_MELOPT4(T, %) oHLSL_MELOPT4(bool, ==) oHLSL_MELOPT4(bool, !=) oHLSL_MELOPT4(bool, <) oHLSL_MELOPT4(bool, >) oHLSL_MELOPT4(bool, <=) oHLSL_MELOPT4(bool, >=)
#define oHLSL_ELOPS(N) oHLSL_ELOP##N(*) oHLSL_ELOP##N(/) oHLSL_ELOP##N(+) oHLSL_ELOP##N(-) oHLSL_ELOP##N(%) oHLSL_ELOPT##N(<<, int) oHLSL_ELOPT##N(>>, int) oHLSL_ELOPT##N(&, int) oHLSL_ELOPT##N(|, int) oHLSL_ELOPT##N(^, int)
#define oHLSL_ELUFNS(fn) oHLSL_ELUFN2(fn) oHLSL_ELUFN3(fn) oHLSL_ELUFN4(fn)
#define oHLSL_ELBFNS(pubfn, implfn) oHLSL_ELBFN2(pubfn, implfn) oHLSL_ELBFN3(pubfn, implfn) oHLSL_ELBFN4(pubfn, implfn)
#define oHLSL_CMP(num) oHLSL_CONCAT(oHLSL_CMP, num)(!=) oHLSL_CONCAT(oHLSL_CMP, num)(==) oHLSL_CONCAT(oHLSL_CMP, num)(<) oHLSL_CONCAT(oHLSL_CMP, num)(>)  oHLSL_CONCAT(oHLSL_CMP, num)(<=)  oHLSL_CONCAT(oHLSL_CMP, num)(>=)
#define oHLSL_VBRACKET_OP(return_t) const return_t& operator[](size_t i) const { return *(&x + i); } return_t& operator[](size_t i) { return *(&x + i); }
#define oHLSL_MBRACKET_OP(return_t) const return_t& operator[](size_t i) const { return *(&Column0 + i); } return_t& operator[](size_t i) { return *(&Column0 + i); }

#define oHLSL_SW2__(a,b) inline const TVEC2<T> a##b() const { return TVEC2<T>(a,b); }
#define oHLSL_SWIZZLE2(type) oHLSL_SW2__(x,x) oHLSL_SW2__(x,y) oHLSL_SW2__(y,x) oHLSL_SW2__(y,y)

#define oHLSL_SW3__(a,b,c) inline const TVEC3<T> a##b##c() const { return TVEC3<T>(a,b,c); }
#define oHLSL_SWIZZLE3(type) oHLSL_SWIZZLE2(type) oHLSL_SW2__(y,z) oHLSL_SW2__(z,y) oHLSL_SW2__(x,z) oHLSL_SW2__(z,x) oHLSL_SW2__(z,z) \
	oHLSL_SW3__(x,x,x) oHLSL_SW3__(x,x,y) oHLSL_SW3__(x,x,z) oHLSL_SW3__(x,y,x) oHLSL_SW3__(x,y,y) oHLSL_SW3__(x,y,z) oHLSL_SW3__(x,z,x) \
	oHLSL_SW3__(x,z,y) oHLSL_SW3__(x,z,z) oHLSL_SW3__(y,x,x) oHLSL_SW3__(y,x,y) oHLSL_SW3__(y,x,z) oHLSL_SW3__(y,y,x) oHLSL_SW3__(y,y,y) \
	oHLSL_SW3__(y,y,z) oHLSL_SW3__(y,z,x) oHLSL_SW3__(y,z,y) oHLSL_SW3__(y,z,z) oHLSL_SW3__(z,x,x) oHLSL_SW3__(z,x,y) oHLSL_SW3__(z,x,z) \
	oHLSL_SW3__(z,y,x) oHLSL_SW3__(z,y,y) oHLSL_SW3__(z,y,z) oHLSL_SW3__(z,z,x) oHLSL_SW3__(z,z,y) oHLSL_SW3__(z,z,z)
	
// todo: Add the rest of the swizzle3 with W permutations and swizzle4 permutations
#define oHLSL_SWIZZLE4(type) oHLSL_SWIZZLE3(type) \
	oHLSL_SW2__(z,w) oHLSL_SW2__(w,z) oHLSL_SW2__(x,w) oHLSL_SW2__(w,x) oHLSL_SW2__(y,w) oHLSL_SW2__(w,y) oHLSL_SW2__(w,w) \
	oHLSL_SW3__(w,w,w) oHLSL_SW3__(y,z,w) \
	inline const TVEC4<T>& xyzw() const { return *(TVEC4<T>*)this; }

// Wrap root types in a namespace so that NoStepInto regex's can be used to 
// avoid stepping into trivial ctors and operators. Use a very short name 
// because we've run into name truncation warnings especially when using 
// std::bind.
namespace oHLSL {

// Unions of xyzw and rgba turned out to be annoying in the debugger in 
// practice, but to support the HLSL specification this should be defined. The 
// suggestion: port code indexing rgba elements to use xyzw instead or write
// a debugger script that filters one or the other out.
//#define oHLSL_HAS_RGBA_ELEMENTS

template<typename T> struct TVEC2
{
	typedef T element_type;
	#ifdef oHLSL_HAS_RGBA_ELEMENTS
		union { T x; T r; };
		union { T y; T g; };
	#else
		T x,y;
	#endif
	inline TVEC2() {}
	inline TVEC2(const TVEC2& _TVector) : x(_TVector.x), y(_TVector.y) {}
	template<typename U> TVEC2(const TVEC2<U>& _UVector) : x(static_cast<T>(_UVector.x)), y(static_cast<T>(_UVector.y)) {}
	template<typename U> explicit TVEC2(U _XY) { x = y = static_cast<T>(_XY); }
	inline TVEC2(T _XY) : x(_XY), y(_XY) {}
	inline TVEC2(T _X, T _Y) : x(_X), y(_Y) {}
	TVEC2 operator-() const { return TVEC2(-x, -y); }
	oHLSL_SWIZZLE2(T);
	oHLSL_MELOPTS2();
	oHLSL_MEMBER_OPS(TVEC2<T>, T);
};

template<typename T> struct TVEC3
{
	typedef T element_type;
	#ifdef oHLSL_HAS_RGBA_ELEMENTS
		union { T x; T r; };
		union { T y; T g; };
		union { T z; T b; };
	#else
		T x,y,z;
	#endif
	inline TVEC3() {};
	inline TVEC3(const TVEC3& _TVector) : x(_TVector.x), y(_TVector.y), z(_TVector.z) {}
	template<typename U> TVEC3(const TVEC3<U>& _UVector) : x(static_cast<T>(_UVector.x)), y(static_cast<T>(_UVector.y)), z(static_cast<T>(_UVector.z)) {}
	template<typename U> explicit TVEC3(U _XYZ) { x = y = z = static_cast<T>(_XYZ); }
	inline TVEC3(T _XYZ) : x(_XYZ), y(_XYZ), z(_XYZ) {}
	inline TVEC3(T _X, T _Y, T _Z) : x(_X), y(_Y), z(_Z) {}
	inline TVEC3(const TVEC2<T>& _XY, T _Z) : x(_XY.x), y(_XY.y), z(_Z) {}
	TVEC3 operator-() const { return TVEC3(-x, -y, -z); }
	oHLSL_SWIZZLE3(T);
	oHLSL_MELOPTS3();
	oHLSL_MEMBER_OPS(TVEC3<T>, T);
};

template<typename T> struct TVEC4
{
	typedef T element_type;
	#ifdef oHLSL_HAS_RGBA_ELEMENTS
		union { T x; T r; };
		union { T y; T g; };
		union { T z; T b; };
		union { T w; T a; };
	#else
		T x,y,z,w;
	#endif
	inline TVEC4() {};
	inline TVEC4(const TVEC4& _TVector) : x(_TVector.x), y(_TVector.y), z(_TVector.z), w(_TVector.w) {}
	template<typename U> TVEC4(const TVEC4<U>& _UVector) : x(static_cast<T>(_UVector.x)), y(static_cast<T>(_UVector.y)), z(static_cast<T>(_UVector.z)), w(static_cast<T>(_UVector.w)) {}
	template<typename U> explicit TVEC4(U _XYZW) { x = y = z = w = static_cast<T>(_XYZW); }
	inline TVEC4(T _XYZW) : x(_XYZW), y(_XYZW), z(_XYZW), w(_XYZW) {}
	inline TVEC4(const TVEC2<T>& _XY, T _Z, T _W) : x(_XY.x), y(_XY.y), z(_Z), w(_Z) {}
	inline TVEC4(const TVEC3<T>& _XYZ, T _W) : x(_XYZ.x), y(_XYZ.y), z(_XYZ.z), w(_W) {}
	inline TVEC4(const TVEC2<T>& _XY, const TVEC2<T>& _ZW) : x(_XY.x), y(_XY.y), z(_ZW.x), w(_ZW.y) {}
	inline TVEC4(T _X, const TVEC3<T>& _YZW) : x(_X), y(_YZW.y), z(_YZW.z), w(_YZW.w) {}
	inline TVEC4(T _X, T _Y, T _Z, T _W) : x(_X), y(_Y), z(_Z), w(_W) {}
	TVEC4 operator-() const { return TVEC4(-x, -y, -z, -w); }
	oHLSL_SWIZZLE4(T);
	oHLSL_MELOPTS4();
	inline TVEC3<T>& xyz() { return *(TVEC3<T>*)&x; }
	oHLSL_MEMBER_OPS(TVEC4<T>, T);
};

template<typename T> struct TMAT3
{
	typedef T element_type;
	typedef TVEC3<T> vector_type;
	// Column-major 3x3 matrix
	TVEC3<T> Column0;
	TVEC3<T> Column1;
	TVEC3<T> Column2;
	TMAT3() {}
	TMAT3(const TMAT3& _Matrix) : Column0(_Matrix.Column0), Column1(_Matrix.Column1), Column2(_Matrix.Column2) {}
	TMAT3(const TVEC3<T>& _Column0, const TVEC3<T>& _Column1, const TVEC3<T>& _Column2) : Column0(_Column0), Column1(_Column1), Column2(_Column2) {}
	oHLSL_MBRACKET_OP(TVEC3<T>);
};

template<typename T> struct TMAT4
{
	typedef T element_type;
	typedef TVEC4<T> vector_type;
	// Column-major 4x4 matrix
	TVEC4<T> Column0;
	TVEC4<T> Column1;
	TVEC4<T> Column2;
	TVEC4<T> Column3;
	TMAT4() {}
	TMAT4(const TMAT4& _Matrix) : Column0(_Matrix.Column0), Column1(_Matrix.Column1), Column2(_Matrix.Column2), Column3(_Matrix.Column3) {}
	TMAT4(const TVEC4<T>& _Column0, const TVEC4<T>& _Column1, const TVEC4<T>& _Column2, const TVEC4<T>& _Column3) : Column0(_Column0), Column1(_Column1), Column2(_Column2), Column3(_Column3) {}
	TMAT4(const TMAT3<T>& _ScaleRotation, const TVEC3<T>& _Translation) : Column0(_ScaleRotation.Column0, 0), Column1(_ScaleRotation.Column1, 0), Column2(_ScaleRotation.Column2, 0), Column3(_Translation, 1.0f) {}
	operator TMAT3<T>() const { return TMAT3<T>(Column0.xyz(), Column1.xyz(), Column2.xyz()); }
	oHLSL_MBRACKET_OP(TVEC4<T>);
};

} // namespace oHLSL

// Useful for min/max concepts where a must always be greater than b, so the
// rest of a calling function can execute normally, and if the values need to 
// be swapped, use this.
template<typename T> void swap_if_greater(oHLSL::TVEC2<T>& a, oHLSL::TVEC2<T>& b) { if (a.x > b.x) std::swap(a.x, b.x); if (a.y > b.y) std::swap(a.y, b.y); }
template<typename T> void swap_if_greater(oHLSL::TVEC3<T>& a, oHLSL::TVEC3<T>& b) { if (a.x > b.x) std::swap(a.x, b.x); if (a.y > b.y) std::swap(a.y, b.y); if (a.z > b.z) std::swap(a.z, b.z); }
template<typename T> void swap_if_greater(oHLSL::TVEC4<T>& a, oHLSL::TVEC4<T>& b) { if (a.x > b.x) std::swap(a.x, b.x); if (a.y > b.y) std::swap(a.y, b.y); if (a.z > b.z) std::swap(a.z, b.z); if (a.w > b.w) std::swap(a.w, b.w); }

template<typename T> oHLSL::TVEC2<T> operator-(const oHLSL::TVEC2<T>& a) { return oHLSL::TVEC2<T>(-a.x, -a.y); }
template<typename T> oHLSL::TVEC3<T> operator-(const oHLSL::TVEC3<T>& a) { return oHLSL::TVEC3<T>(-a.x, -a.y, -a.z); }
template<typename T> oHLSL::TVEC4<T> operator-(const oHLSL::TVEC4<T>& a) { return oHLSL::TVEC4<T>(-a.x, -a.y, -a.z, -a.w); }
template<typename T> oHLSL::TVEC3<T> operator*(const oHLSL::TMAT3<T>& a, const oHLSL::TVEC3<T>& b) { return mul(a, b); }
template<typename T> oHLSL::TMAT3<T> operator*(const oHLSL::TMAT3<T>& a, const oHLSL::TMAT3<T>& b) { return mul(a, b); }
template<typename T> oHLSL::TVEC3<T> operator*(const oHLSL::TMAT4<T>& a, const oHLSL::TVEC3<T>& b) { return mul(a, b); }
template<typename T> oHLSL::TVEC4<T> operator*(const oHLSL::TMAT4<T>& a, const oHLSL::TVEC4<T>& b) { return mul(a, b); }
template<typename T> oHLSL::TMAT4<T> operator*(const oHLSL::TMAT4<T>& a, const oHLSL::TMAT4<T>& b) { return mul(a, b); }

oHLSL_ELOPS(2) oHLSL_ELOPS(3) oHLSL_ELOPS(4) oHLSL_CMP(2) oHLSL_CMP(3) oHLSL_CMP(4)

// The oHLSL namespace is intended only to enable NoStepInto settings in MSVC, 
// so don't pollute client code with the requirement of namespace specification.
using namespace oHLSL;

#endif
