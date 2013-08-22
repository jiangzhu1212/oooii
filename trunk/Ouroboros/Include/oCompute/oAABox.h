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
// This header is designed to cross-compile in both C++ and HLSL. This defines
// an axis-aligned bounding box.
#ifndef oHLSL
	#pragma once
#endif
#ifndef oCompute_aabox_h
#define oCompute_aabox_h

#include <oHLSL/oHLSLMacros.h>
#include <oHLSL/oHLSLMath.h>
#include <oHLSL/oHLSLTypes.h>

#ifdef oHLSL

#else

// Wrap in a namespace so that NoStepInto can be used for VS2010+.
namespace oCompute {
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
		inline bool empty() const { return any_less_than(Max, Min); } 
		inline void clear() { Min = TVec(std::numeric_limits<T>::max()); Max = TVec(std::numeric_limits<T>::lowest()); }
		inline TVec size() const { return abs(Max - Min); }
		inline TVec center() const { return Min + size() / T(2.0); }
		inline T bounding_radius() const { return length(Max - Min) / T(2.0); }
		inline bool operator==(const aabox<T, TVec>& _That) const { return oStd::equal(Min, _That.Min) && oStd::equal(Max, _That.Max); }
		TVec Min;
		TVec Max;
	};
} // namespace oCompute

typedef oCompute::aabox<float, TVEC3<float>> oAABoxf; typedef oCompute::aabox<double, TVEC3<double>> oAABoxd;
typedef oCompute::aabox<int, TVEC2<int>> oRECT;
typedef oCompute::aabox<float, TVEC2<float>> oRECTF;

template<typename T> void oDecompose(const oCompute::aabox<T, TVEC2<T>>& _Box, TVEC2<T> _Vertices[4])
{
	const TVEC2<T>& Min = _Box.Min;
	const TVEC2<T>& Max = _Box.Max;
	_Vertices[0] = Min;
	_Vertices[1] = TVEC2<T>(Max.x, Min.y);
	_Vertices[2] = TVEC2<T>(Min.x, Max.y);
	_Vertices[3] = Max;
}

// Get the eight corners that make up the box
template<typename T> void oDecompose(const oCompute::aabox<T, TVEC3<T>>& _Box, TVEC3<T> _Vertices[8])
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

template<typename T, typename TVec> void oExtendBy(oCompute::aabox<T, TVec>& _AABox, const TVec& _Point) { _AABox.Min = min(_AABox.Min, _Point); _AABox.Max = max(_AABox.Max, _Point); }
template<typename T, typename TVec> void oExtendBy(oCompute::aabox<T, TVec>& _AABox1, const oCompute::aabox<T, TVec>& _AABox2) { oExtendBy(_AABox1, _AABox2.Min); oExtendBy(_AABox1, _AABox2.Max); }
template<typename T, typename TVec> void oTranslate(oCompute::aabox<T, TVec>& _AABox, const TVec& _Translation) { _AABox.Min += _Translation; _AABox.Max += _Translation; }
template<typename T, typename TVec> inline bool oContains(const oCompute::aabox<T, TVec>& _Box, const TVec& _Point) { return greater_than_equal(_Point, _Box.Min) && less_than_equal(_Point, _Box.Max); }

// Axis-aligned boxes/rects are always axis-aligned. An axis-aligned box that 
// is naively transformed becomes an oriented bounding box. Take the points of
// an oriented bounding box and calculate the axis-aligned box that contains
// them to produce a transformed box.
template<typename T, typename TVec> 
oCompute::aabox<T, TVec> mul(const TMAT4<T>& _Matrix, const oCompute::aabox<T, TVec>& _Bound)
{
	TVec verts[8];
	oDecompose(_Bound, verts);
	oCompute::aabox<T, TVec> box;
	oHLSL_UNROLL
	for (int i = 0; i < 8; i++)
	{
		const float3 transformed = mul(_Matrix, TVEC4<T>(verts[i], T(1))).xyz();
		box.Min = min(box.Min, transformed);
		box.Max = max(box.Max, transformed);
	}
	return box;
}

#endif
#endif
