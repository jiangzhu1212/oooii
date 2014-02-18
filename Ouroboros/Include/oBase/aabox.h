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
// An axis-aligned bounding box.
#ifndef oHLSL
	#pragma once
#endif
#ifndef oBase_aabox_h
#define oBase_aabox_h

#ifdef oHLSL

struct aaboxf
{
	float3 Min;
	float3 Max;
};

#else

#include <oHLSL/oHLSLMath.h>
#include <oHLSL/oHLSLTypes.h>
#include <float.h>

namespace ouro {

template<typename T, typename TVec> struct aabox
{
	enum ctor_type
	{
		pos_size,
		min_max,
	};

	typedef T element_type;
	typedef TVec vector_type;
	aabox() { clear(); }
	aabox(const aabox<T,TVec>& _That) : Min(_That.Min), Max(_That.Max) {}
	aabox(aabox<T,TVec>&& _That) { operator=(std::move(_That)); }
	aabox(ctor_type _Type, const TVec& _Val1, const TVec& _Val2) : Min(_Val1), Max(_Type == pos_size ? (_Val1 + _Val2) : _Val2) {}
	inline const aabox<T,TVec>& operator=(const TVec& _That) { Min = _That.Min; Max = _That.Max; return *this; }
	inline aabox<T,TVec>& operator=(TVec&& _That) { if (this != &_That) { Min = _That.Min; Max = _That.Max; _That.clear(); } return *this; }
	inline bool empty() const { return any(Max < Min); } 
	inline void clear() { Min = TVec(FLT_MAX); Max = TVec(-FLT_MAX); }
	inline TVec size() const { return abs(Max - Min); }
	inline TVec center() const { return Min + size() / T(2.0); }
	inline T bounding_radius() const { return length(Max - Min) / T(2.0); }
	inline bool operator==(const aabox<T, TVec>& _That) const { return equal(Min, _That.Min) && equal(Max, _That.Max); }
	TVec Min;
	TVec Max;
};

// Get the four corners that make up the rectangle
template<typename T> void decompose(const aabox<T, TVEC2<T>>& _Box, TVEC2<T> _Vertices[4])
{
	const TVEC2<T>& Min = _Box.Min;
	const TVEC2<T>& Max = _Box.Max;
	_Vertices[0] = Min;
	_Vertices[1] = TVEC2<T>(Max.x, Min.y);
	_Vertices[2] = TVEC2<T>(Min.x, Max.y);
	_Vertices[3] = Max;
}

// Get the eight corners that make up the box
template<typename T> void decompose(const aabox<T, TVEC3<T>>& _Box, TVEC3<T> _Vertices[8])
{
	const TVEC3<T>& Min = _Box.Min;
	const TVEC3<T>& Max = _Box.Max;
	_Vertices[0] = Min;
	_Vertices[1] = TVEC3<T>(Max.x, Min.y, Min.z);
	_Vertices[2] = TVEC3<T>(Min.x, Max.y, Min.z);
	_Vertices[3] = TVEC3<T>(Max.x, Max.y, Min.z);
	_Vertices[4] = TVEC3<T>(Min.x, Min.y, Max.z);
	_Vertices[5] = TVEC3<T>(Max.x, Min.y, Max.z);
	_Vertices[6] = TVEC3<T>(Min.x, Max.y, Max.z);
	_Vertices[7] = Max;
}

template<typename T, typename TVec> void extend_by(aabox<T, TVec>& _AABox, const TVec& _Point) { _AABox.Min = min(_AABox.Min, _Point); _AABox.Max = max(_AABox.Max, _Point); }
template<typename T, typename TVec> void extend_by(aabox<T, TVec>& _AABox1, const aabox<T, TVec>& _AABox2) { oExtendBy(_AABox1, _AABox2.Min); oExtendBy(_AABox1, _AABox2.Max); }
template<typename T, typename TVec> void translate(aabox<T, TVec>& _AABox, const TVec& _Translation) { _AABox.Min += _Translation; _AABox.Max += _Translation; }

// returns -1 if _Box0 partially contains _Box1, 0 if not contained at all, 1 if _Box0 wholly contains _Box1
template<typename T, typename TVec>
int contains(const aabox<T, TVec>& _Box0, const aabox<T, TVec>& _Box1)
{
	if (any(_Box0.Min > _Box1.Max) || any(_Box0.Max < _Box1.Min))
		return 0;
	if (all(_Box1.Min >= _Box0.Min) && all(_Box1.Max <= _Box0.Max))
		return 1;
	return -1;
}

template<typename T, typename TVec>
int contains(const aabox<T, TVec>& _Box, const TVec& _Point)
{
	TVec d = _Point - _Box.center();
	bool bInX = abs(d.x) < _Box.size().x / 2.0f;
	bool bInY = abs(d.y) < _Box.size().y / 2.0f;
	bool bInZ = abs(d.z) < _Box.size().z / 2.0f;
	return (bInX && bInY && bInZ) ? 1 : 0;
}

typedef aabox<float, TVEC3<float>> aaboxf; typedef aabox<double, TVEC3<double>> aaboxd;
typedef aabox<int, TVEC2<int>> rect;
typedef aabox<float, TVEC2<float>> rectf; typedef aabox<double, TVEC2<double>> rectd;

} // namespace ouro

// Axis-aligned boxes/rects are always axis-aligned. An axis-aligned box that 
// is naively transformed becomes an oriented bounding box. Take the points of
// an oriented bounding box and calculate the axis-aligned box that contains
// them to produce a transformed box.
template<typename T, typename TVec> 
ouro::aabox<T, TVec> mul(const TMAT4<T>& _Matrix, const ouro::aabox<T, TVec>& _Bound)
{
	TVec verts[8];
	ouro::decompose(_Bound, verts);
	ouro::aabox<T, TVec> box;
	for (int i = 0; i < 8; i++)
	{
		const float3 transformed = mul(_Matrix, TVEC4<T>(verts[i], T(1))).xyz();
		ouro::extend_by(box, transformed);
	}
	return box;
}

#endif
#endif