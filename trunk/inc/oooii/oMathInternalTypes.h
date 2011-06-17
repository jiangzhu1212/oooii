// $(header)

// This is an internal header that contains the macros and templates for 
// defining math types such as vectors and matrices. This is not intended to be 
// used publicly, or even included outside oMath.h. Use the typedefs found in 
// oMath.h for these types. This is separated out from oMath.h to allow for a 
// more readable public oMath.h header.
#pragma once
#ifndef oMathInternalTypes_h
#define oMathInternalTypes_h

#include <oooii/oLimits.h>

// _____________________________________________________________________________
// Internal macros used in the definition of tuples below

#define oMATH_CONCAT(a,b) a##b

#define oMATH_MEMBER_OP(return_t, param_t, op) inline const return_t& operator op##=(const param_t& a) { *this = *this op a; return *this; }
#define oMATH_MEMPER_EQOP(param_t, op) bool operator op(const param_t& a) const { return ::operator op(*this, a); }
#define oMATH_ELOP2(op) template<typename T> inline oMathNS::TVEC2<T> operator op(const oMathNS::TVEC2<T>& a, const oMathNS::TVEC2<T>& b) { return oMathNS::TVEC2<T>(a.x op b.x, a.y op b.y); } template<typename T> inline oMathNS::TVEC2<T> operator op(const oMathNS::TVEC2<T>& a, const T& b) { return oMathNS::TVEC2<T>(a.x op b, a.y op b); } template<typename T> inline oMathNS::TVEC2<T> operator op(const T& a, const oMathNS::TVEC2<T>& b) { return oMathNS::TVEC2<T>(a op b.x, a op b.y); }
#define oMATH_ELOP3(op) template<typename T> inline oMathNS::TVEC3<T> operator op(const oMathNS::TVEC3<T>& a, const oMathNS::TVEC3<T>& b) { return oMathNS::TVEC3<T>(a.x op b.x, a.y op b.y, a.z op b.z); } template<typename T> inline oMathNS::TVEC3<T> operator op(const oMathNS::TVEC3<T>& a, const T& b) { return oMathNS::TVEC3<T>(a.x op b, a.y op b, a.z op b); } template<typename T> inline oMathNS::TVEC3<T> operator op(const T& a, const oMathNS::TVEC3<T>& b) { return oMathNS::TVEC3<T>(a op b.x, a op b.y, a op b.z); }
#define oMATH_ELOP4(op) template<typename T> inline oMathNS::TVEC4<T> operator op(const oMathNS::TVEC4<T>& a, const oMathNS::TVEC4<T>& b) { return oMathNS::TVEC4<T>(a.x op b.x, a.y op b.y, a.z op b.z, a.w op b.w); } template<typename T> inline oMathNS::TVEC4<T> operator op(const oMathNS::TVEC4<T>& a, const T& b) { return oMathNS::TVEC4<T>(a.x op b, a.y op b, a.z op b, a.w op b); } template<typename T> inline oMathNS::TVEC4<T> operator op(const T& a, const oMathNS::TVEC4<T>& b) { return oMathNS::TVEC4<T>(a op b.x, a op b.y, a op b.z, a op b.w); }
#define oMATH_ELUFN2(fn) template<typename T> inline oMathNS::TVEC2<T> fn(const oMathNS::TVEC2<T>& a) { return oMathNS::TVEC2<T>(fn(a.x), fn(a.y)); }
#define oMATH_ELUFN3(fn) template<typename T> inline oMathNS::TVEC3<T> fn(const oMathNS::TVEC3<T>& a) { return oMathNS::TVEC3<T>(fn(a.x), fn(a.y), fn(a.z)); }
#define oMATH_ELUFN4(fn) template<typename T> inline oMathNS::TVEC4<T> fn(const oMathNS::TVEC4<T>& a) { return oMathNS::TVEC3<T>(fn(a.x), fn(a.y), fn(a.z), fn(a.w)); }
#define oMATH_ELBFN2(pubfn, implfn) template<typename T> inline oMathNS::TVEC2<T> pubfn(const oMathNS::TVEC2<T>& a, const oMathNS::TVEC2<T>& b) { return oMathNS::TVEC2<T>(implfn(a.x, b.x), implfn(a.y, b.y)); }
#define oMATH_ELBFN3(pubfn, implfn) template<typename T> inline oMathNS::TVEC3<T> pubfn(const oMathNS::TVEC3<T>& a, const oMathNS::TVEC3<T>& b) { return oMathNS::TVEC3<T>(implfn(a.x, b.x), implfn(a.y, b.y), implfn(a.z, b.z)); }
#define oMATH_ELBFN4(pubfn, implfn) template<typename T> inline oMathNS::TVEC4<T> pubfn(const oMathNS::TVEC4<T>& a, const oMathNS::TVEC4<T>& b) { return oMathNS::TVEC4<T>(implfn(a.x, b.x), implfn(a.y, b.y), implfn(a.z, b.z), implfn(a.w, b.w)); }
#define oMATH_EQ2() template<typename T> inline bool operator==(const oMathNS::TVEC2<T>& a, const oMathNS::TVEC2<T>& b) { return oEqual(a.x, b.x) && oEqual(a.y, b.y); }
#define oMATH_EQ3() template<typename T> inline bool operator==(const oMathNS::TVEC3<T>& a, const oMathNS::TVEC3<T>& b) { return oEqual(a.x, b.x) && oEqual(a.y, b.y) && oEqual(a.z, b.z); }
#define oMATH_EQ4() template<typename T> inline bool operator==(const oMathNS::TVEC4<T>& a, const oMathNS::TVEC4<T>& b) { return oEqual(a.x, b.x) && oEqual(a.y, b.y) && oEqual(a.z, b.z) && oEqual(a.w, b.w); }
#define oMATH_NEQ(type) template<typename T> inline bool operator!=(const type& a, const type& b) { return !(a == b); }
#define oMATH_CMP2(fn,cmp) template<typename T> inline bool fn(const oMathNS::TVEC2<T>& a, const oMathNS::TVEC2<T>& b) { return a.x cmp b.x && a.y cmp b.y; }
#define oMATH_CMP3(fn,cmp) template<typename T> inline bool fn(const oMathNS::TVEC3<T>& a, const oMathNS::TVEC3<T>& b) { return a.x cmp b.x && a.y cmp b.y && a.z cmp b.z; }
#define oMATH_CMP4(fn,cmp) template<typename T> inline bool fn(const oMathNS::TVEC4<T>& a, const oMathNS::TVEC4<T>& b) { return a.x cmp b.x && a.y cmp b.y && a.z cmp b.z && a.w cmp b.w; }
#define oMATH_ANYCMP2(fn,cmp) template<typename T> inline bool fn(const oMathNS::TVEC2<T>& a, const oMathNS::TVEC2<T>& b) { return a.x cmp b.x || a.y cmp b.y; }
#define oMATH_ANYCMP3(fn,cmp) template<typename T> inline bool fn(const oMathNS::TVEC3<T>& a, const oMathNS::TVEC3<T>& b) { return a.x cmp b.x || a.y cmp b.y || a.z cmp b.z; }
#define oMATH_ANYCMP4(fn,cmp) template<typename T> inline bool fn(const oMathNS::TVEC4<T>& a, const oMathNS::TVEC4<T>& b) { return a.x cmp b.x || a.y cmp b.y || a.z cmp b.z || a.w cmp b.w; }
// Macros to get through the boilerplate for operators, compares, etc.
#define oMATH_MEMBER_OPS(type, scalar_t) oMATH_MEMPER_EQOP(type, ==) oMATH_MEMPER_EQOP(type, !=) oMATH_MEMBER_OP(type, scalar_t, *) oMATH_MEMBER_OP(type, scalar_t, /) oMATH_MEMBER_OP(type, scalar_t, +) oMATH_MEMBER_OP(type, scalar_t, -) oMATH_MEMBER_OP(type, type, *) oMATH_MEMBER_OP(type, type, /) oMATH_MEMBER_OP(type, type, +) oMATH_MEMBER_OP(type, type, -) oMATH_VBRACKET_OP(scalar_t)
#define oMATH_ELOPS(N) oMATH_ELOP##N(*) oMATH_ELOP##N(/) oMATH_ELOP##N(+) oMATH_ELOP##N(-) oMATH_ELOP##N(%)
#define oMATH_ELUFNS(fn) oMATH_ELUFN2(fn) oMATH_ELUFN3(fn) oMATH_ELUFN4(fn)
#define oMATH_ELBFNS(pubfn, implfn) oMATH_ELBFN2(pubfn, implfn) oMATH_ELBFN3(pubfn, implfn) oMATH_ELBFN4(pubfn, implfn)
#define oMATH_CMP(num) oMATH_NEQ(oMATH_CONCAT(oMathNS::TVEC, num)<T>) oMATH_CONCAT(oMATH_EQ, num)() \
	oMATH_CONCAT(oMATH_CMP, num)(less_than, <) oMATH_CONCAT(oMATH_CMP, num)(greater_than, >)  oMATH_CONCAT(oMATH_CMP, num)(less_than_equal, <=)  oMATH_CONCAT(oMATH_CMP, num)(greater_than_equal, >=) \
	oMATH_CONCAT(oMATH_ANYCMP, num)(any_less_than, <) oMATH_CONCAT(oMATH_ANYCMP, num)(any_greater_than, >)  oMATH_CONCAT(oMATH_ANYCMP, num)(any_less_than_equal, <=)  oMATH_CONCAT(oMATH_ANYCMP, num)(any_greater_than_equal, >=)
#define oMATH_VBRACKET_OP(return_t) const return_t& operator[](size_t i) const { return *(&x + i); } return_t& operator[](size_t i) { return *(&x + i); }
#define oMATH_MBRACKET_OP(return_t) const return_t& operator[](size_t i) const { return *(&Column0 + i); } return_t& operator[](size_t i) { return *(&Column0 + i); }

#define oMATH_SWIZZLE2(type) \
	inline const TVEC2<T>& XY() const { return *(TVEC2<T>*)this; } \
	inline const TVEC2<T>& YX() const { return TVEC2<T>(y, x); } \
	inline const TVEC2<T>& XX() const { return TVEC2<T>(x, x); } \
	inline const TVEC2<T>& YY() const { return TVEC2<T>(y, y); } \

// @oooii-tony: todo: Add the rest of the permutations
#define oMATH_SWIZZLE3(type) oMATH_SWIZZLE2(type) \
	inline const TVEC2<T>& YZ() const { return *(TVEC2<T>*)&y; } \
	inline const TVEC2<T>& ZY() const { return TVEC2<T>(z, y); } \
	inline const TVEC2<T>& XZ() const { return TVEC2<T>(x, z); } \
	inline const TVEC2<T>& ZX() const { return TVEC2<T>(z, x); } \
	inline const TVEC2<T>& ZZ() const { return TVEC2<T>(z, z); } \
	inline const TVEC3<T>& XYZ() const { return *(TVEC3<T>*)this; } \
	inline const TVEC3<T>& XXX() const { return TVEC3<T>(x, x, x); } \
	inline const TVEC3<T>& YYY() const { return TVEC3<T>(y, y, y); } \
	inline const TVEC3<T>& ZZZ() const { return TVEC3<T>(z, z, z); } \

// @oooii-tony: todo: Add the rest of the permutations
#define oMATH_SWIZZLE4(type) oMATH_SWIZZLE3(type) \
	inline const TVEC2<T>& ZW() const { return *(TVEC2<T>*)&z; } \
	inline const TVEC2<T>& WZ() const { return TVEC2<T>(w, z); } \
	inline const TVEC2<T>& XW() const { return TVEC2<T>(x, w); } \
	inline const TVEC2<T>& WX() const { return TVEC2<T>(w, x); } \
	inline const TVEC2<T>& YW() const { return TVEC2<T>(y, w); } \
	inline const TVEC2<T>& WY() const { return TVEC2<T>(w, y); } \
	inline const TVEC3<T>& WWW() const { return TVEC3<T>(w, w, w); } \
	inline const TVEC3<T>& YZW() const { return *(TVEC3<T>*)&y; } \
	inline const TVEC4<T>& XYZW() const { return *(TVEC4<T>*)this; }

// Wrap root types in a namespace so that NoStepInto regex's can be used to 
// avoid stepping into trivial ctors and operators. Use a very short name 
// because we've run into name truncation warnings especially when using oBIND.
namespace oMathNS {

template<typename T> struct TVEC2
{
	T x,y;
	inline TVEC2() {}
	inline TVEC2(const TVEC2& _Vector) : x(_Vector.x), y(_Vector.y) {}
	inline TVEC2(T _XY) : x(_XY), y(_XY) {}
	inline TVEC2(T _X, T _Y) : x(_X), y(_Y) {}
	oMATH_SWIZZLE2(T);
	oMATH_MEMBER_OPS(TVEC2<T>, T);
};

template<typename T> struct TVEC3
{
	T x,y,z;
	inline TVEC3() {};
	inline TVEC3(const TVEC3& _Vector) : x(_Vector.x), y(_Vector.y), z(_Vector.z) {}
	inline TVEC3(T _XYZ) : x(_XYZ), y(_XYZ), z(_XYZ) {}
	inline TVEC3(T _X, T _Y, T _Z) : x(_X), y(_Y), z(_Z) {}
	inline TVEC3(const TVEC2<T>& _XY, T _Z) : x(_XY.x), y(_XY.y), z(_Z) {}
	oMATH_SWIZZLE3(T);
	oMATH_MEMBER_OPS(TVEC3<T>, T);
};

template<typename T> struct TVEC4
{
	T x,y,z,w;
	inline TVEC4() {};
	inline TVEC4(const TVEC4& _Vector) : x(_Vector.x), y(_Vector.y), z(_Vector.z), w(_Vector.w) {}
	inline TVEC4(T _XYZW) : x(_XYZW), y(_XYZW), z(_XYZW), w(_XYZW) {}
	inline TVEC4(const TVEC2<T>& _XY, T _Z, T _W) : x(_XY.x), y(_XY.y), z(_Z), w(_Z) {}
	inline TVEC4(const TVEC3<T>& _XYZ, T _W) : x(_XYZ.x), y(_XYZ.y), z(_XYZ.z), w(_W) {}
	inline TVEC4(const TVEC2<T>& _XY, const TVEC2<T>& _ZW) : x(_XY.x), y(_XY.y), z(_ZW.z), w(_ZW.w) {}
	inline TVEC4(T _X, const TVEC3<T>& _YZW) : x(_X), y(_YZW.y), z(_YZW.z), w(_YZW.w) {}
	inline TVEC4(T _X, T _Y, T _Z, T _W) : x(_X), y(_Y), z(_Z), w(_W) {}
	oMATH_SWIZZLE4(T);
	inline TVEC3<T>& XYZ() { return *(TVEC3<T>*)&x; }
	oMATH_MEMBER_OPS(TVEC4<T>, T);
};

template<typename T> struct TQUAT
{
	T x,y,z,w;
	inline TQUAT() {};
	inline TQUAT(const TQUAT& _Quaternion) : x(_Quaternion.x), y(_Quaternion.y), z(_Quaternion.z), w(_Quaternion.w) {}
	inline TQUAT(T _X, T _Y, T _Z, T _W) : x(_X), y(_Y), z(_Z), w(_W) {}
	oMATH_VBRACKET_OP(T);
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
	TMAT3<T> GetUpper3x3() const { return TMAT3<T>(Column0.XYZ(), Column1.XYZ(), Column2.XYZ()); }
	oMATH_MBRACKET_OP(TVEC4<T>);
	static const TMAT4<T> Identity;
};

template<typename T, typename TVec> struct TAABOX
{
	// NOTE: Use Transform() to change the space of a TAABOX, do not use GetMin()
	// and GetMax() and transform those as points. TAABOX represents MINIMUM and 
	// MAXIMUM values in whatever space is represented. This is slightly different 
	// than using two extreme corners of a box to represent the box because of 
	// transformation. No matter how this box is transformed, Min is the min value 
	// in the new space and Max is the max value in the new space.

	TAABOX() { Clear(); }
	TAABOX(const TAABOX<T,TVec>& box) : Min(box.Min), Max(box.Max) {}
	TAABOX(const TVec& _Min, const TVec& _Max) : Min(_Min), Max(_Max) {}
	inline const TAABOX<T,TVec>& operator=(const TVec& _Box) { Min = box.Min; Max = box.Max; return *this; }
	inline const TVec& GetMin() const { return Min; }
	inline const TVec& GetMax() const { return Max; }
	inline void SetMin(const TVec& _Min) { Min = _Min; }
	inline void SetMax(const TVec& _Max) { Max = _Max; }
	inline bool IsEmpty() const { return any_less_than(Max, Min); } 
	inline void Clear() { Min = TVec(oNumericLimits<T>::GetMax()); Max = TVec(oNumericLimits<T>::GetSignedMin()); }
	inline TVec GetDimensions() const { return abs(Max - Min); }
	inline TVec GetCenter() const { return Min + GetDimensions() / T(2.0f); }
	inline T GetBoundingRadius() const { return length(Max - Min) / T(2.0f); }
	inline void ExtendBy(const TVec& _Point) { Min = oMin(Min, _Point); Max = oMax(Max, _Point); }
	inline void ExtendBy(const TAABOX<T, TVec>& _Box) { ExtendBy(_Box.Min); ExtendBy(_Box.Max); }
	inline void ExtractCorners(TVEC3<T> _Corners[8]) const { _Corners[0] = Min; _Corners[1] = float3(Max.x, Min.y, Min.z); _Corners[2] = float3(Min.x, Max.y, Min.z); _Corners[3] = float3(Max.x, Max.y, Min.z); _Corners[4] = float3(Min.x, Min.y, Max.z); _Corners[5] = float3(Max.x, Min.y, Max.z); _Corners[6] = float3(Min.x, Max.y, Max.z); _Corners[7] = Max; }

	// Transforming a bounding box naively can generate a min point that has 
	// values larger than the max point (think about rotating a box 180 degrees,
	// the min/max values for a particular axis would be flipped). The extra code
	// here ensures Min and Max are always the min values and max values in any
	// orientation.

	inline void Transform(const TMAT4<T>& _Matrix)
	{
		Min = _Matrix * Min;
		Max = _Matrix * Max;
		SwapIfGreater(Min, Max);
	}

protected:
	TVec Min;
	TVec Max;

	template<typename T> inline void Swap(T& a, T& b) { T c = a; a = b; b = c; }
	template<typename T> inline void SwapIfGreater(oMathNS::TVEC2<T>& a, oMathNS::TVEC2<T>& b) { if (a.x > b.x) Swap(a.x, b.x); if (a.y > b.y) Swap(a.y, b.y); }
	template<typename T> inline void SwapIfGreater(oMathNS::TVEC3<T>& a, oMathNS::TVEC3<T>& b) { if (a.x > b.x) Swap(a.x, b.x); if (a.y > b.y) Swap(a.y, b.y); if (a.z > b.z) Swap(a.z, b.z); }
	template<typename T> inline void SwapIfGreater(oMathNS::TVEC4<T>& a, oMathNS::TVEC4<T>& b) { if (a.x > b.x) Swap(a.x, b.x); if (a.y > b.y) Swap(a.y, b.y); if (a.z > b.z) Swap(a.z, b.z); if (a.w > b.w) Swap(a.w, b.w); }
};

template<typename T> struct TPLANE : public TVEC4<T>
{
	TPLANE<T>() {}
	TPLANE<T>(const TVEC3<T>& _Normal, const T& _Offset) : TVEC4<T>(normalize(_Normal), _Offset) {}
	TPLANE<T>(const T& _NormalX, const T& _NormalY, const T& _NormalZ, const T& _Offset) : TVEC4<T>(normalize(TVEC3<T>(_NormalX, _NormalY, _NormalZ)), _Offset) {}
	TPLANE<T>(const TVEC3<T>& _Normal, const TVEC3<T> _Point) { XYZ() = normalize(_Normal); w = dot(n, _Point); }

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

	inline const TVEC4<T>& GetPlane(size_t _Index) const { return (&Near)[_Index]; }

	// Returns true if values are valid or false if planes don't meet in 8 corners.
	bool ExtractCorners(TVEC3<T> _Corners[8]) const
	{
		// @oooii-tony: TODO implement oIntersects for double
		bool isect = oIntersects(&_Corners[LEFT_TOP_NEAR], Left, Top, Near);
		isect = isect && oIntersects(&_Corners[LEFT_TOP_FAR], Left, Top, Far);
		isect = isect && oIntersects(&_Corners[LEFT_BOTTOM_NEAR], Left, Bottom, Near);
		isect = isect && oIntersects(&_Corners[LEFT_BOTTOM_FAR], Left, Bottom, Far);
		isect = isect && oIntersects(&_Corners[RIGHT_TOP_NEAR], Right, Top, Near);
		isect = isect && oIntersects(&_Corners[RIGHT_TOP_FAR], Right, Top, Far);
		isect = isect && oIntersects(&_Corners[RIGHT_BOTTOM_NEAR], Right, Bottom, Near);
		isect = isect && oIntersects(&_Corners[RIGHT_BOTTOM_FAR], Right, Bottom, Far);
		return isect;
	}
};

template<typename T> struct TSPHERE : public TVEC4<T>
{
	TSPHERE<T>() {}
	TSPHERE<T>(const TVEC3<T>& _Position, T _Radius) : TVEC4<T>(_Position, _Radius) {}
	T GetRadius() const { return w; }
	const TVEC3<T>& GetPosition() const { return XYZ(); }
	TAABOX<T, TVEC3<T>> GetBoundingAABox() const { return TAABOX<T, TVEC3<T>>(TVEC3<T>(x - w, y - w, z - w), TVEC3<T>(x + w, y + w, z + w)); }
};

} // namespace oMathNS

template<typename T> inline bool operator==(const oMathNS::TQUAT<T>& a, const oMathNS::TQUAT<T>& b) { return oEqual(a.x, b.x) && oEqual(a.y, b.y) && oEqual(a.z, b.z) && oEqual(a.w, b.w); }
template<typename T> inline bool operator!=(const oMathNS::TQUAT<T>& a, const oMathNS::TQUAT<T>& b) { return !(a == b); }

template<typename T> inline oMathNS::TVEC2<T> operator-(const oMathNS::TVEC2<T>& a) { return oMathNS::TVEC2<T>(-a.x, -a.y); }
template<typename T> inline oMathNS::TVEC3<T> operator-(const oMathNS::TVEC3<T>& a) { return oMathNS::TVEC3<T>(-a.x, -a.y, -a.z); }
template<typename T> inline oMathNS::TVEC4<T> operator-(const oMathNS::TVEC4<T>& a) { return oMathNS::TVEC4<T>(-a.x, -a.y, -a.z, -a.w); }
template<typename T> inline oMathNS::TVEC3<T> operator*(const oMathNS::TMAT3<T>& a, const oMathNS::TVEC3<T>& b) { return mul(a, b); }
template<typename T> inline oMathNS::TMAT3<T> operator*(const oMathNS::TMAT3<T>& a, const oMathNS::TMAT3<T>& b) { return mul(a, b); }
template<typename T> inline oMathNS::TVEC3<T> operator*(const oMathNS::TMAT4<T>& a, const oMathNS::TVEC3<T>& b) { return mul(a, b); }
template<typename T> inline oMathNS::TVEC4<T> operator*(const oMathNS::TMAT4<T>& a, const oMathNS::TVEC4<T>& b) { return mul(a, b); }
template<typename T> inline oMathNS::TMAT4<T> operator*(const oMathNS::TMAT4<T>& a, const oMathNS::TMAT4<T>& b) { return mul(a, b); }
template<typename T> inline oMathNS::TQUAT<T> operator*(const oMathNS::TQUAT<T>&a, const oMathNS::TQUAT<T>& b) { return mul(a, b); }

// The namespace is really only to enable NoStepInto settings in MSVC, so don't
// pollute the rest of the headers with the requirement of namespace specification.
using namespace oMathNS;

oMATH_ELOPS(2) oMATH_ELOPS(3) oMATH_ELOPS(4)
oMATH_CMP(2) oMATH_CMP(3) oMATH_CMP(4)

#endif
