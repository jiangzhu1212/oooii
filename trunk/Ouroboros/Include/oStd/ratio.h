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
// Approximation of the upcoming C++11 std::ratio interface
#pragma once
#ifndef oStd_ratio_h
#define oStd_ratio_h

#include <type_traits>

namespace oStd {

template<unsigned long long N, unsigned long long D = 1> class ratio
{
	// @tony: This is not even close to the standards requirement of 
	// reduction of terms or sign conservation and doesn't support any of the 
	// meta-math yet.

public:
	static const unsigned long long num = N;
	static const unsigned long long den = D;
};

typedef ratio<1, 1000000000000000000> atto;
typedef ratio<1, 1000000000000000> femto;
typedef ratio<1, 1000000000000> pico;
typedef ratio<1, 1000000000> nano;
typedef ratio<1, 1000000> micro;
typedef ratio<1, 1000> milli;
typedef ratio<1, 100> centi;
typedef ratio<1, 10> deci;
typedef ratio<10, 1> deca;
typedef ratio<100, 1> hecto;
typedef ratio<1000, 1> kilo;
typedef ratio<1000000, 1> mega;
typedef ratio<1000000000, 1> giga;
typedef ratio<1000000000000, 1> tera;
typedef ratio<1000000000000000, 1> peta;
typedef ratio<1000000000000000000, 1> exa;

	namespace detail {
		template <unsigned long long A, unsigned long long B> struct oGCD { static const unsigned long long value = oGCD<A, B%A>::value; };
		template <unsigned long long A> struct oGCD<A, 0>{ static const unsigned long long value = A; };
	}

} // namespace oStd

namespace std {
	template<unsigned long long N1, unsigned long long D1, unsigned long long N2, unsigned long long D2>
	struct common_type<oStd::ratio<N1, D1>, oStd::ratio<N2, D2>>
	{
		typedef oStd::ratio<
			oStd::detail::oGCD<oStd::ratio<N1, D1>::num, oStd::ratio<N2, D2>::num>::value, 
			oStd::detail::oGCD<oStd::ratio<N1, D1>::den, oStd::ratio<N2, D2>::den>::value
		> type;
	};
}

#endif
