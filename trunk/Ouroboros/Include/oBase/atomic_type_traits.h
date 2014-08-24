/**************************************************************************
 * The MIT License                                                        *
 * Copyright (c) 2014 Antony Arciuolo.                                    *
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
// Get the builtin type underlying a std::atomic
#pragma once
#ifndef oBase_atomic_type_traits_h
#define oBase_atomic_type_traits_h

#include <atomic>

namespace ouro {

template<typename T> struct remove_atomic { typedef T type; };

template<> struct remove_atomic<std::atomic<bool>> { typedef bool type; };
template<> struct remove_atomic<std::atomic<char>> { typedef char type; };
template<> struct remove_atomic<std::atomic<signed char>> { typedef signed char type; };
template<> struct remove_atomic<std::atomic<unsigned char>> { typedef unsigned char type; };
template<> struct remove_atomic<std::atomic<short>> { typedef short type; };
template<> struct remove_atomic<std::atomic<unsigned short>> { typedef unsigned short type; };
template<> struct remove_atomic<std::atomic<int>> { typedef int type; };
template<> struct remove_atomic<std::atomic<unsigned int>> { typedef unsigned int type; };
template<> struct remove_atomic<std::atomic<long>> { typedef long type; };
template<> struct remove_atomic<std::atomic<unsigned long>> { typedef unsigned long type; };
template<> struct remove_atomic<std::atomic<long long>> { typedef long long type; };
template<> struct remove_atomic<std::atomic<unsigned long long>> { typedef unsigned long long type; };

template<> struct remove_atomic<std::atomic_bool> { typedef bool type; };
template<> struct remove_atomic<std::atomic_char> { typedef char type; };
template<> struct remove_atomic<std::atomic_schar> { typedef signed char type; };
template<> struct remove_atomic<std::atomic_uchar> { typedef unsigned char type; };
template<> struct remove_atomic<std::atomic_short> { typedef short type; };
template<> struct remove_atomic<std::atomic_ushort> { typedef unsigned short type; };
template<> struct remove_atomic<std::atomic_int> { typedef int type; };
template<> struct remove_atomic<std::atomic_uint> { typedef unsigned int type; };
template<> struct remove_atomic<std::atomic_long> { typedef long type; };
template<> struct remove_atomic<std::atomic_ulong> { typedef unsigned long type; };
template<> struct remove_atomic<std::atomic_llong> { typedef long long type; };
template<> struct remove_atomic<std::atomic_ullong> { typedef unsigned long long type; };

}

#endif
