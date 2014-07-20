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
// Utility functions helpful when dealing with memory buffers and pointers, 
// especially when it is useful to go back and forth between thinking of the 
// buffer as bytes and as its type without a lot of casting.
#pragma once
#ifndef oBase_byte_h
#define oBase_byte_h

#include <stddef.h>

namespace ouro {

// Alignment 
template<typename T> T byte_align(T _Value, size_t _Alignment) { return (T)(((size_t)_Value + _Alignment - 1) & ~(_Alignment - 1)); }
template<typename T> T byte_align_down(T _Value, size_t _Alignment) { return (T)((size_t)_Value & ~(_Alignment - 1)); }
template<typename T> bool byte_aligned(T _Value, size_t _Alignment) { return byte_align(_Value, _Alignment) == _Value; }

// Offsets
template<typename T> T* byte_add(T* _Pointer, size_t _NumBytes) { return reinterpret_cast<T*>(((char*)_Pointer) + _NumBytes); }
template<typename T> T* byte_sub(T* _Pointer, size_t _NumBytes) { return reinterpret_cast<T*>(((char*)_Pointer) - _NumBytes); }
template<typename T, typename U> T* byte_add(T* _RelativePointer, U* _BasePointer) { return reinterpret_cast<T*>(((char*)_RelativePointer) + reinterpret_cast<size_t>(_BasePointer)); }
template<typename T> T* byte_add(T* _Pointer, size_t _Stride, size_t _Count) { return reinterpret_cast<T*>(((char*)_Pointer) + _Stride * _Count); }
template<typename T, typename U> ptrdiff_t byte_diff(T* t, U* u) { return (ptrdiff_t)((char*)t - (char*)u); }
template<typename T> size_t byte_padding(T value, size_t alignment) { return oSizeT(static_cast<T>(byte_align(value, alignment)) - value); }
inline size_t index_of(const void* el, const void* base, size_t elSize) { return byte_diff(el, base) / elSize; }
template<typename T> size_t index_of(const T* el, const T* base) { return index_of(el, base, sizeof(T)); }

// Validation
inline bool in_range(const void* _TestPointer, const void* _BasePointer, size_t _SizeofRange) { return (_TestPointer >= _BasePointer) && (_TestPointer < byte_add(_BasePointer, _SizeofRange)); }
inline bool in_range(const void* _TestPointer, const void* _BasePointer, const void* _EndPointer) { return (_TestPointer >= _BasePointer) && (_TestPointer < _EndPointer); }

// Swizzle structs
struct byte_swizzle16
{
	union
	{
		short as_short;
		unsigned short as_unsigned_short;
		char as_char[2];
		unsigned char as_unsigned_char[2];
	};
};

struct byte_swizzle32
{
	union
	{
		float as_float;
		int as_int;
		unsigned int as_unsigned_int;
		long as_long;
		unsigned long as_unsigned_long;
		short as_short[2];
		unsigned short as_unsigned_short[2];
		char as_char[4];
		unsigned char as_unsigned_char[4];
	};
};

struct byte_swizzle64
{
	union
	{
		double as_double;
		unsigned long long as_unsigned_long_long;
		long long as_long_long;
		float as_float[2];
		int as_int[2];
		unsigned int as_unsigned_int[2];
		long as_long[2];
		unsigned long as_unsigned_long[2];
		short as_short[4];
		unsigned short as_unsigned_short[4];
		char as_char[8];
		unsigned char as_unsigned_char[8];
	};
};

// quantization

// convert a normalized float value [0,1] to a certain bit representation and back.
inline unsigned char f32ton2(float x) { return static_cast<unsigned char>(x * 3.0f + 0.5f); }
inline float n2tof32(unsigned char x) { return x / 3.0f; }

inline unsigned char f32ton8(float x) { return static_cast<unsigned char>(x * 255.0f + 0.5f); }
inline float n8tof32(unsigned char x) { return x / 255.0f; }

inline unsigned short f32ton10(float x) { return static_cast<unsigned short>(x * 1023.0f + 0.5f); }
inline float n10tof32(unsigned short x) { return x / 1023.0f; }

inline unsigned short f32ton16(float x) { return static_cast<unsigned short>(x * 65535.0f + 0.5f); }
inline float n16tof32(unsigned short x) { return x / 65535.0f; }

// convert [0,1] to [-1,1] and back
inline float uf32tosf32(float x) { return x * 2.0f - 1.0f; }
inline float sf32touf32(float x) { return x * 0.5f + 0.5f; }

} // namespace ouro

#endif
