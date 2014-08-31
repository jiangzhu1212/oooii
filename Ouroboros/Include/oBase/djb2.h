// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.
// djb2 hash
#pragma once
#ifndef oBase_djb2_h
#define oBase_djb2_h

#include <oBase/uint128.h>

namespace ouro {

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
T djb2(const void* buf, size_t buf_size, const T& _Seed = djb2_traits<T>::seed)
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
	const char* s = static_cast<const char*>(buf);
	djb2_traits<T>::value_type h = _Seed;
	while(buf_size--)
		h = (h + (h << 5)) ^ *s++;
	return h;
	// $(CitedCodeEnd)
}

}

#endif
