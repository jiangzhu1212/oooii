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
// fnva1 hash.
#pragma once
#ifndef oBase_fnv1a_h
#define oBase_fnv1a_h

#include <oBase/uint128.h>

namespace ouro {

template<typename T> struct fnv1a_traits
{
	typedef T value_type;
	static const value_type seed;
	static const value_type value;
};

template<> struct fnv1a_traits<unsigned int>
{
	typedef unsigned int value_type;
	static const unsigned int seed = 2166136261u;
	static const unsigned int value = 16777619u;
};

template<> struct fnv1a_traits<unsigned long>
{
	typedef unsigned long value_type;
	static const unsigned long seed = 2166136261u;
	static const unsigned long value = 16777619u;
};

template<> struct fnv1a_traits<unsigned long long>
{
	typedef unsigned long long value_type;
	static const unsigned long long seed = 14695981039346656037ull;
	static const unsigned long long value = 1099511628211ull;
};

#ifdef oHAS_CONSTEXPR
	template<> struct fnv1a_traits<uint128>
	{
		static constexpr uint128 seed = { 0x6c62272e07bb0142ull, 0x62b821756295c592ull }; // 144066263297769815596495629667062367629
		static constexpr uint128 value = { 0x0000000001000000ull, 0x000000000000013bull }; // 309485009821345068724781371
	};
#endif

template<typename T>		
inline T fnv1a(const void* _pBuffer, size_t _SizeofBuffer, const T& _Seed = fnv1a_traits<T>::seed)
{
	fnv1a_traits<T>::value_type h = _Seed;
	const char* s = static_cast<const char*>(_pBuffer);
	while (_SizeofBuffer--)
	{
		h ^= *s++;
		h *= fnv1a_traits<T>::value;
	}
	return h;
}

template<typename T>		
inline T fnv1a(const char* _String, const T& _Seed = fnv1a_traits<T>::seed)
{
	fnv1a_traits<T>::value_type h = _Seed;
	while (*_String)
	{
		h ^= *_String++;
		h *= fnv1a_traits<T>::value;
	}
	return h;
}

} // namespace ouro

#endif
