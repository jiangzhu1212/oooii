// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.
// fnva1 hash.
#pragma once
#ifndef oBase_fnv1a_h
#define oBase_fnv1a_h

#include <oBase/uint128.h>
#include <oString/string.h> // to_lower

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
inline T fnv1a(const void* buf, size_t buf_size, const T& _Seed = fnv1a_traits<T>::seed)
{
	fnv1a_traits<T>::value_type h = _Seed;
	const char* s = static_cast<const char*>(buf);
	while (buf_size--)
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

template<typename T>		
inline T fnv1ai(const char* _String, const T& _Seed = fnv1a_traits<T>::seed)
{
	fnv1a_traits<T>::value_type h = _Seed;
	while (*_String)
	{
		char c = *_String++;
		h ^= to_lower(c);
		h *= fnv1a_traits<T>::value;
	}
	return h;
}

namespace detail { union wchar_bytes { wchar_t wc; char c[2]; }; }

template<typename T>		
inline T fnv1a(const wchar_t* _String, const T& _Seed = fnv1a_traits<T>::seed)
{
	fnv1a_traits<T>::value_type h = _Seed;
	while (*_String)
	{
		detail::wchar_bytes ch;
		ch.wc = *_String++;
		h ^= ch.c[0];
		h *= fnv1a_traits<T>::value;
		h ^= ch.c[1];
		h *= fnv1a_traits<T>::value;
	}
	return h;
}

template<typename T>		
inline T fnv1ai(const wchar_t* _String, const T& _Seed = fnv1a_traits<T>::seed)
{
	fnv1a_traits<T>::value_type h = _Seed;
	while (*_String)
	{
		detail::wchar_bytes ch;
		ch.wc = to_lower(*_String++);
		h ^= ch.c[0];
		h *= fnv1a_traits<T>::value;
		h ^= ch.c[1];
		h *= fnv1a_traits<T>::value;
	}
	return h;
}

// npot bitcount hashes: http://www.isthe.com/chongo/tech/comp/fnv/
// (use next-higher type and xor reduce)
template<typename T, typename U>
T fnv1a_reduced(const U& _Fnv1aHash, int _NumBits) { return static_cast<T>((_Fnv1aHash >> _NumBits) ^ (U(1) << _NumBits)); }

}

#endif
