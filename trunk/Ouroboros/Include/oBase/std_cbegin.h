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
// Missing cbegin/cend is considered an oversight in C++11, so here it is.
#pragma once
#ifndef oBase_std_cbegin_h
#define oBase_std_cbegin_h
#include <oBase/compiler_config.h>
#include <iterator>
#include <array>

#if oHAS_CBEGIN == 0
	namespace std {
		template<typename T> auto cbegin(const T& x) -> decltype(x.cbegin()) { return x.cbegin(); }
		template<typename T> auto cend(const T& x) -> decltype(x.cend()) { return x.cend(); }
		template<typename T, size_t size> auto cbegin(const array<T, size>& x) -> decltype(x.cbegin()) { return x.cbegin(); }
		template<typename T, size_t size> auto cend(const array<T, size>& x) -> decltype(x.cend()) { return x.cend(); }
		template<typename T, int size> const T* cbegin(const T (&x)[size]) { return &x[0]; }
		template<typename T, int size> const T* cend(const T (&x)[size]) { return &x[size]; }
		template<typename T, size_t size> auto crbegin(const array<T, size>& x) -> decltype(x.crbegin()) { return x.crbegin(); }
		template<typename T, size_t size> auto crend(const array<T, size>& x) -> decltype(x.crend()) { return x.crend(); }
		template<typename T> auto crbegin(const T& x) -> decltype(x.crbegin()) { return x.crbegin(); }
		template<typename T> auto crend(const T& x) -> decltype(x.crend()) { return x.crend(); }
	} // namespace std
#endif

#endif
