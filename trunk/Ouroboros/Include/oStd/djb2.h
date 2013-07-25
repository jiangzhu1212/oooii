/**************************************************************************
 * The MIT License                                                        *
 * Copyright (c) 2013 OOOii.                                              *
 * antony.arciuolo@oooii.com                                              *
 * kevin.myers@oooii.com                                                  *
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
// djb2 hash
#pragma once
#ifndef oStd_djb2_h
#define oStd_djb2_h

#include <oStd/uint128.h>

namespace oStd {

template<typename T> struct djb2_traits
{
	typedef T value_type;
	static const value_type seed;
};

template<> struct djb2_traits<unsigned int>
{
	typedef unsigned int value_type;
	static const value_type seed = 5381u;
};

template<> struct djb2_traits<unsigned long>
{
	typedef unsigned long value_type;
	static const value_type seed = 5381u;
};

template<> struct djb2_traits<unsigned long long>
{
	typedef unsigned long long value_type;
	static const value_type seed = 5381ull;
};

#ifdef oHAS_CONSTEXPR
	template<> struct djb2_traits<uint128>
	{
		typedef uint128 value_type;
		static constexpr uint128 seed = { 0ull, 5381ull };
	};
#endif

template<typename T>
T djb2(const void* _pBuffer, size_t _SizeofBuffer, const T& _Seed = djb2_traits<T>::seed)
{
	/** <citation
		usage="Implementation" 
		reason="STL's hash is decent, but there are better ones, such as this" 
		author="Ozan Yigit"
		description="http://www.cse.yorku.ca/~oz/hash.html"
		license="Public Domain"
		licenseurl="http://www.cse.yorku.ca/~oz/hash.html"
		modification="templated"
	/>*/
	// $(CitedCodeBegin)
	const char* s = static_cast<const char*>(_pBuffer);
	djb2_traits<T>::value_type h = _Seed;
	while(_SizeofBuffer--)
		h = (h + (h << 5)) ^ *s++;
	return h;
	// $(CitedCodeEnd)
}

} // namespace oStd

#endif
