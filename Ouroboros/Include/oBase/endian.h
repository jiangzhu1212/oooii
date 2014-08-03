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
// Utility functions helpful when dealing with little endian v. big endian data
// serialization.
#pragma once
#ifndef oBase_endian_h
#define oBase_endian_h

#include <stddef.h>
#include <type_traits>

namespace ouro {
	namespace detail {
		static const unsigned short endian_signature = 1;
	} // namespace detail

static const bool is_little_endian = *(const unsigned char*)&detail::endian_signature == 1;
static const bool is_big_endian = *(const unsigned char*)&detail::endian_signature == 0;

template<typename T> T endian_swap(const T& x) { static_assert(!std::is_floating_point<T>::value, "floating point values must use explicit api for endian swapping"); return x; }
template<> inline unsigned short endian_swap(const unsigned short& x) { return (x<<8) | (x>>8); }
template<> inline unsigned int endian_swap(const unsigned int& x) { return (x<<24) | ((x<<8) & 0x00ff0000) | ((x>>8) & 0x0000ff00) | (x>>24); }
template<> inline unsigned long long endian_swap(const unsigned long long& x) { return (x<<56) | ((x<<40) & 0x00ff000000000000ll) | ((x<<24) & 0x0000ff0000000000ll) | ((x<<8) & 0x000000ff00000000ll) | ((x>>8) & 0x00000000ff000000ll) | ((x>>24) & 0x0000000000ff0000ll) | ((x>>40) & 0x000000000000ff00ll) | (x>>56); }
template<> inline short endian_swap(const short& x) { unsigned short r = endian_swap(*(unsigned short*)&x); return *(short*)&r; }
template<> inline int endian_swap(const int& x) { unsigned int r = endian_swap(*(unsigned int*)&x); return *(int*)&r; }
template<> inline long long endian_swap(const long long& x) { unsigned long long r = endian_swap(*(unsigned long long*)&x); return *(long long*)&r; }

inline unsigned int endian_swap_float(const float& x) { return endian_swap(*(unsigned int*)&x); }
inline float endian_swap_float(const unsigned int& x) { unsigned int f = endian_swap(x); return *(float*)&f; }

inline unsigned long long endian_swap_double(const double& x) { return endian_swap(*(unsigned long long*)&x); }
inline double endian_swap_double(const unsigned long long& x) { unsigned long long f = endian_swap(x); return *(double*)&f; }

template<typename T> T to_big_endian(const T& x) { return is_little_endian ? endian_swap(x) : x; };
template<typename T> T from_big_endian(const T& x) { return to_big_endian(x); }
template<typename T> T to_little_endian(T x) { return is_little_endian ? x : endian_swap(x); };
template<typename T> T from_little_endian(T x) { return to_little_endian(x); }

inline unsigned int to_big_endian_float(const float& x) { return is_little_endian ? endian_swap_float(x) : *(unsigned int*)&x; };
inline float to_big_endian_float(const unsigned int& x) { return is_little_endian ? endian_swap_float(x) : *(float*)&x; };
inline unsigned int from_big_endian_float(const float& x) { return to_big_endian_float(x); }
inline float from_big_endian_float(const unsigned int& x) { return to_big_endian_float(x); }

inline unsigned long long to_big_endian_double(const double& x) { return is_little_endian ? endian_swap_double(x) : *(unsigned long long*)&x; };
inline double to_big_endian_double(const unsigned long long& x) { return is_little_endian ? endian_swap_double(x) : *(double*)&x; };
inline unsigned long long from_big_endian_double(const double& x) { return to_big_endian_double(x); }
inline double from_big_endian_double(const unsigned long long& x) { return to_big_endian_double(x); }

} // namespace ouro

#endif
