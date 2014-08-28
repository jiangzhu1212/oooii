// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.
#pragma once
#ifndef oMemory_byte_h
#define oMemory_byte_h

// Utility functions helpful when dealing with memory buffers and pointers, 
// especially when it is useful to go back and forth between thinking of the 
// buffer as bytes and as its type without a lot of casting.

#include <stddef.h>
#include <cstdint>

namespace ouro {

// _____________________________________________________________________________
// Alignment 

	template<typename T> T byte_align(T value, size_t alignment) { return (T)(((size_t)value + alignment - 1) & ~(alignment - 1)); }
template<typename T> T byte_align_down(T value, size_t alignment) { return (T)((size_t)value & ~(alignment - 1)); }
template<typename T> bool byte_aligned(T value, size_t alignment) { return byte_align(value, alignment) == value; }

// _____________________________________________________________________________
// Offsets

template<typename T> T* byte_add(T* ptr, size_t bytes) { return reinterpret_cast<T*>(((char*)ptr) + bytes); }
template<typename T> T* byte_sub(T* ptr, size_t bytes) { return reinterpret_cast<T*>(((char*)ptr) - bytes); }
template<typename T, typename U> T* byte_add(T* relative_ptr, U* base_ptr) { return reinterpret_cast<T*>(((char*)relative_ptr) + reinterpret_cast<size_t>(base_ptr)); }
template<typename T> T* byte_add(T* ptr, size_t stride, size_t count) { return reinterpret_cast<T*>(((char*)ptr) + stride * count); }
template<typename T, typename U> ptrdiff_t byte_diff(T* t, U* u) { return (ptrdiff_t)((char*)t - (char*)u); }
template<typename T> size_t byte_padding(T value, size_t alignment) { return oSizeT(static_cast<T>(byte_align(value, alignment)) - value); }
inline size_t index_of(const void* el, const void* base, size_t elSize) { return byte_diff(el, base) / elSize; }
template<typename T> size_t index_of(const T* el, const T* base) { return index_of(el, base, sizeof(T)); }

// _____________________________________________________________________________
// Validation

inline bool in_range(const void* test_ptr, const void* base_ptr, size_t range_size) { return (test_ptr >= base_ptr) && (test_ptr < byte_add(base_ptr, range_size)); }
inline bool in_range(const void* test_ptr, const void* base_ptr, const void* end_ptr) { return (test_ptr >= base_ptr) && (test_ptr < end_ptr); }

// _____________________________________________________________________________
// Swizzle structs

struct byte_swizzle16
{
	union
	{
		short as_short;
		unsigned short as_ushort;
		char as_char[2];
		unsigned char as_uchar[2];
	};
};

struct byte_swizzle32
{
	union
	{
		float as_float;
		int as_int;
		unsigned int as_uint;
		long as_long;
		unsigned long as_ulong;
		short as_short[2];
		unsigned short as_ushort[2];
		char as_char[4];
		unsigned char as_uchar[4];
	};
};

struct byte_swizzle64
{
	union
	{
		double as_double;
		unsigned long long as_ullong;
		long long as_llong;
		float as_float[2];
		int as_int[2];
		unsigned int as_uint[2];
		long as_long[2];
		unsigned long as_ulong[2];
		short as_short[4];
		unsigned short as_ushort[4];
		char as_char[8];
		unsigned char as_uchar[8];
	};
};

// _____________________________________________________________________________
// Quantization

// convert a normalized float value [0,1] to a certain bit representation and back.
inline uint8_t f32ton2(float x) { return static_cast<uint8_t>(x * 3.0f + 0.5f); }
inline float n2tof32(uint8_t x) { return x / 3.0f; }

inline uint8_t f32ton8(float x) { return static_cast<uint8_t>(x * 255.0f + 0.5f); }
inline float n8tof32(uint8_t x) { return x / 255.0f; }

inline uint16_t f32ton10(float x) { return static_cast<uint16_t>(x * 1023.0f + 0.5f); }
inline float n10tof32(uint16_t x) { return x / 1023.0f; }

inline uint16_t f32ton16(float x) { return static_cast<uint16_t>(x * 65535.0f + 0.5f); }
inline float n16tof32(uint16_t x) { return x / 65535.0f; }

// convert [0,1] to [-1,1] and back
inline float uf32tosf32(float x) { return x * 2.0f - 1.0f; }
inline float sf32touf32(float x) { return x * 0.5f + 0.5f; }

}

#endif
