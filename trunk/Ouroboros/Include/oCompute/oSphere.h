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
// a sphere.

#ifndef oHLSL
	#pragma once
#endif
#ifndef oCompute_sphere_h
#define oCompute_sphere_h

#include <oHLSL/oHLSLMacros.h>
#include <oHLSL/oHLSLMath.h>
#include <oHLSL/oHLSLTypes.h>

#ifdef oHLSL

#define oSpheref float4
#define oSphered double4

#else
	// Wrap in a namespace so that NoStepInto can be used for VS2010+.
	namespace oCompute {
		template<typename T> struct sphere : public TVEC4<T>
		{
			typedef T element_type;
			typedef TVEC3<element_type> vector_type;
			typedef TVEC4<element_type> base_type;
			sphere() {}
			sphere(const sphere& _That) { operator=(_That); }
			sphere(const base_type& _That) { operator=(_That); }
			sphere(const vector_type& _Position, element_type _Radius) : base_type(_Position, _Radius) {}
			element_type radius() const { return w; }
			const sphere& operator=(const sphere& _That) { return operator=(*(base_type*)&_That); }
			const sphere& operator=(const base_type& _That) { *(base_type*)this = _That; return *this; }
			operator base_type() { return *this; }
		};
	} // namespace oCompute

typedef oCompute::sphere<float> oSpheref; typedef oCompute::sphere<double> oSphered;

#endif
#endif
