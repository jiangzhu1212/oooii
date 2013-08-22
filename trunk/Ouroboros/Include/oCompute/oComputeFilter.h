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
// This code contains code that cross-compiles in C++ and HLSL. This contains
// the non-sampling (non-texture-dependent) math for various filters.
#ifndef oHLSL
	#pragma once
#endif
#ifndef oComputeFilter_h
#define oComputeFilter_h

#ifndef oHLSL
	#include <oHLSL/oHLSLMath.h>
#endif

inline float2 oCalcCrossComponents(float _TopLeft, float _TopRight, float _BottomLeft, float _BottomRight)
{
	return float2(_BottomRight - _TopLeft, _BottomLeft - _TopRight);
}

// Return what is a Sobel version of ddx/ddy based on intensity values in the 
// same space sampled from the eight grid items surrounding a central focus grid
// item. In graphics, this is usually a rasterized image where for any given
// pixel its Sobel values are taken from the 8 neighbor pixels.
inline float2 oCalcSobelComponents(float _TopLeft, float _TopCenter, float _TopRight, float _MiddleLeft, float _MiddleRight, float _BottomLeft, float _BottomCenter, float _BottomRight)
{
	//  SobelX    SobelY
	//  1  0 -1   1  2  1
	//  2  0 -2   0  0  0
	//  1  0 -1  -1 -2 -1

	return float2(
		_BottomRight + 2*_MiddleRight + _TopRight - _BottomLeft - 2*_MiddleLeft - _TopLeft,
		_BottomLeft + 2*_BottomCenter + _BottomRight - _TopLeft - 2*_TopCenter - _TopRight);
}

inline float2 oCalcPrewittComponents(float _TopLeft, float _TopCenter, float _TopRight, float _MiddleLeft, float _MiddleRight, float _BottomLeft, float _BottomCenter, float _BottomRight)
{
	//  PrewittX  PrewittY
	//  1  0 -1   1  1  1
	//  1  0 -1   0  0  0
	//  1  0 -1  -1 -1 -1

	return float2(
		_BottomRight + _MiddleRight + _TopRight - _BottomLeft - _MiddleLeft - _TopLeft,
		_BottomLeft + _BottomCenter + _BottomRight - _TopLeft - _TopCenter - _TopRight);
}

inline float2 oCalcCompassComponents(float _TopLeft, float _TopCenter, float _TopRight, float _MiddleLeft, float _MiddleCenter, float _MiddleRight, float _BottomLeft, float _BottomCenter, float _BottomRight)
{
	//  CompassX  CompassY
	//  1 -1 -1  -1 -1 -1
	//  1  2 -1   1  2 -1
	//  1 -1 -1   1  1 -1

	return float2(
		2*_MiddleCenter + _BottomRight + _MiddleRight + _TopRight - _BottomLeft - _MiddleLeft - _TopLeft,
		2*_MiddleCenter + _BottomLeft + _BottomCenter + _BottomRight - _TopLeft - _TopCenter - _TopRight);
}


#endif
