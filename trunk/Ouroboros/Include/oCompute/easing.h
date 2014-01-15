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
// Math functions for advanced interpolation (easing) values between two points
#pragma once
#ifndef oCompute_easing_h
#define oCompute_easing_h

#include <oHLSL/oHLSLMath.h>
#include <oCompute/oComputeConstants.h> // oPI

namespace ouro {

/** <citation
			usage="Adaptation" 
			reason="Pretty clean source code with demo, so trush its math, but code it here consistent with our code standards." 
			author="Robert Penner"
			description="http://robertpenner.com/easing/"
			description="https://github.com/jesusgollonet/ofpennereasing"
			description="http://robertpenner.com/easing/penner_chapter7_tweening.pdf"
			license="BSD License"
			licenseurl="http://opensource.org/licenses/bsd-license.php"
			modification=""
/>*/
// $(CitedCodeBegin)
template<typename T> T simple_linear_tween(float _TimeInSeconds, const T& _Start, const T& _Change, float _Duration) { return _Change * _TimeInSeconds/_Duration + _Start; }

template<typename T> T quadratic_ease_in(float _TimeInSeconds, const T& _Start, const T& _Change, float _Duration) { _TimeInSeconds /= _Duration; return _Change * pow(_TimeInSeconds, T(2)) + _Start; }
template<typename T> T quadratic_ease_out(float _TimeInSeconds, const T& _Start, const T& _Change, float _Duration) { _TimeInSeconds /= _Duration; return -_Change * _TimeInSeconds * (_TimeInSeconds - 2) + _Start; }
template<typename T> T quadratic_ease_inout(float _TimeInSeconds, const T& _Start, const T& _Change, float _Duration)
{
	_TimeInSeconds /= _Duration / T(2);
	if (_TimeInSeconds < T(1))
		return _Change / T(2) * pow(_TimeInSeconds, T(2)) + _Start;
	_TimeInSeconds--;
	return -_Change / T(2) * (_TimeInSeconds * (_TimeInSeconds - T(2)) - T(1)) + _Start;
}

template<typename T> T cubic_ease_in(float _TimeInSeconds, const T& _Start, const T& _Change, float _Duration) { _TimeInSeconds /= _Duration; return _Change * pow(_TimeInSeconds, T(3)) + _Start; }
template<typename T> T cubic_ease_out(float _TimeInSeconds, const T& _Start, const T& _Change, float _Duration) { _TimeInSeconds /= _Duration; _TimeInSeconds--; return _Change * (pow(_TimeInSeconds, T(3)) + T(1)) + _Start; }
template<typename T> T cubic_ease_inout(float _TimeInSeconds, const T& _Start, const T& _Change, float _Duration)
{
	_TimeInSeconds /= _Duration / T(2);
	if (_TimeInSeconds < T(1))
		return _Change / T(2) * pow(_TimeInSeconds, T(3)) + _Start;
	_TimeInSeconds -= T(2);
	return _Change / T(2) * (pow(_TimeInSeconds, T(3)) + T(2)) + _Start;
}

template<typename T> T quartic_ease_in(float _TimeInSeconds, const T& _Start, const T& _Change, float _Duration) { _TimeInSeconds /= _Duration; return _Change * pow(_TimeInSeconds, T(4)) + _Start; }
template<typename T> T quartic_ease_out(float _TimeInSeconds, const T& _Start, const T& _Change, float _Duration) { _TimeInSeconds /= _Duration; _TimeInSeconds--; return -_Change * (pow(_TimeInSeconds, T(4)) - T(1)) + _Start; }
template<typename T> T quartic_ease_inout(float _TimeInSeconds, const T& _Start, const T& _Change, float _Duration) 
{ 
	_TimeInSeconds /= _Duration / T(2);
	if (_TimeInSeconds < T(1))
		return _Change / T(2) * pow(_TimeInSeconds, T(4)) + _Start;
	_TimeInSeconds -= T(2);
	return -_Change / T(2) * (pow(_TimeInSeconds, T(4)) - T(2)) + _Start;
}

template<typename T> T quintic_ease_in(float _TimeInSeconds, const T& _Start, const T& _Change, float _Duration) { _TimeInSeconds /= _Duration; return _Change * pow(_TimeInSeconds, T(5)) + _Start; }
template<typename T> T quintic_ease_out(float _TimeInSeconds, const T& _Start, const T& _Change, float _Duration) { _TimeInSeconds /= _Duration; _TimeInSeconds--; return _Change * (pow(_TimeInSeconds, T(5)) + T(1)) + _Start; }
template<typename T> T quintic_ease_inout(float _TimeInSeconds, const T& _Start, const T& _Change, float _Duration)
{
	_TimeInSeconds /= _Duration / T(2);
	if (_TimeInSeconds < T(1))
		return _Change / T(2) * pow(_TimeInSeconds, T(5)) + _Start;
	_TimeInSeconds -= T(2);
	return _Change / T(2) * (pow(_TimeInSeconds, T(5)) + T(2)) + _Start;
}

template<typename T> T sin_ease_in(float _TimeInSeconds, const T& _Start, const T& _Change, float _Duration) { return -_Change * cos(_TimeInSeconds/_Duration * T(oPI) / T(2)) + _Change + _Start; }
template<typename T> T sin_ease_out(float _TimeInSeconds, const T& _Start, const T& _Change, float _Duration) { return _Change * sin(_TimeInSeconds/_Duration * T(oPI) / T(2)) + _Start; }
template<typename T> T sin_ease_inout(float _TimeInSeconds, const T& _Start, const T& _Change, float _Duration) { return-_Change/T(2) * (cos(T(oPI)*_TimeInSeconds/_Duration) - T(1)) + _Start; }

template<typename T> T exponential_ease_in(float _TimeInSeconds, const T& _Start, const T& _Change, float _Duration) { return _Change * pow(T(2), T(10) * (_TimeInSeconds/_Duration - T(1))) + _Start; }
template<typename T> T exponential_ease_out(float _TimeInSeconds, const T& _Start, const T& _Change, float _Duration) { return _Change * (-pow(T(2), T(-10) * _TimeInSeconds/_Duration) + T(1)) + _Start; }
template<typename T> T exponential_ease_inout(float _TimeInSeconds, const T& _Start, const T& _Change, float _Duration)
{
	_TimeInSeconds /= _Duration / T(2);
	if (_TimeInSeconds < 1)
		return _Change/T(2) * pow(T(2), T(10) * (_TimeInSeconds - T(1))) + _Start;
	_TimeInSeconds--;
	return _Change/T(2) * (-pow(T(2), T(-10) * _TimeInSeconds) + T(2)) + _Start;
}

template<typename T> T circular_ease_in(float _TimeInSeconds, const T& _Start, const T& _Change, float _Duration) { _TimeInSeconds /= _Duration; return (T)(-_Change * (sqrt(1 - pow(_TimeInSeconds, 2)) - 1) + _Start); }
template<typename T> T circular_ease_out(float _TimeInSeconds, const T& _Start, const T& _Change, float _Duration) { _TimeInSeconds /= _Duration; _TimeInSeconds--; return (T)(_Change * sqrt(1 - pow(_TimeInSeconds, 2)) + _Start); }
template<typename T> T circular_ease_inout(float _TimeInSeconds, const T& _Start, const T& _Change, float _Duration)
{
	_TimeInSeconds /= _Duration / T(2);
	if (_TimeInSeconds < T(1))
		return -_Change/T(2) * (sqrt(T(1) - pow(_TimeInSeconds, T(2))) - T(1)) + _Start;
	_TimeInSeconds -= T(2);
	return _Change/T(2) * (sqrt(T(1) - pow(_TimeInSeconds, T(2))) + T(1)) + _Start;
}
// $(CitedCodeEnd)

} // namespace ouro

#endif