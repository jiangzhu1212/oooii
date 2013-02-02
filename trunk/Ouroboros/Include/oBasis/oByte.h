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
// Utility functions helpful when dealing with memory buffers and 
// pointers, especially when it is useful to go back and forth 
// between thinking of the buffer as bytes and as its type without
// a lot of casting.
#pragma once
#ifndef oByte_h
#define oByte_h

#include <stddef.h>
#include <oBasis/oInt.h>

// Alignment
template<typename T> T oByteAlign(T _Value, size_t _Alignment) { return (T)(((size_t)_Value + _Alignment - 1) & ~(_Alignment - 1)); }
template<typename T> T oByteAlignDown(T _Value, size_t _Alignment) { return (T)((size_t)_Value & ~(_Alignment - 1)); }
template<typename T> bool oIsByteAligned(T _Value, size_t _Alignment) { return oByteAlign(_Value, _Alignment) == _Value; }

// Offsets
template<typename T> T* oByteAdd(T* _Pointer, size_t _NumBytes) { return reinterpret_cast<T*>(((char*)_Pointer) + _NumBytes); }
template<typename T> T* oByteSub(T* _Pointer, size_t _NumBytes) { return reinterpret_cast<T*>(((char*)_Pointer) - _NumBytes); }
template<typename T, typename U> T* oByteAdd(T* _RelativePointer, U* _BasePointer) { return reinterpret_cast<T*>(((char*)_RelativePointer) + reinterpret_cast<size_t>(_BasePointer)); }
template<typename T> T* oByteAdd(T* _Pointer, size_t _Stride, size_t _Count) { return reinterpret_cast<T*>(((char*)_Pointer) + _Stride * _Count); }
template<typename T, typename U> ptrdiff_t oByteDiff(T* t, U* u) { return (ptrdiff_t)((char*)t - (char*)u); }
template<typename T> size_t oBytePadding(T value, size_t alignment) { return oSizeT(static_cast<T>(oByteAlign(value, alignment)) - value); }
inline size_t oIndexOf(void* el, void* base, size_t elSize) { return oByteDiff(el, base) / elSize; }
template<typename T> size_t oIndexOf(T* el, T* base) { return oIndexOf(el, base, sizeof(T)); }
template<typename T> bool oIsPow2(T n) { return n ? (((n) & ((n)-1)) == 0) : false; }

// Endian swapping
inline unsigned char oByteSwap(unsigned char x) { return x; } //useful in template code were T may or may not be a char
inline unsigned short oByteSwap(unsigned short x) { return (x<<8) | (x>>8); }
inline unsigned int oByteSwap(unsigned int x) { return (x<<24) | ((x<<8) & 0x00ff0000) | ((x>>8) & 0x0000ff00) | (x>>24); }
inline unsigned long long oByteSwap(unsigned long long x) { return (x<<56) | ((x<<40) & 0x00ff000000000000ll) | ((x<<24) & 0x0000ff0000000000ll) | ((x<<8) & 0x000000ff00000000ll) | ((x>>8) & 0x00000000ff000000ll) | ((x>>24) & 0x0000000000ff0000ll) | ((x>>40) & 0x000000000000ff00ll) | (x>>56); }
inline char oByteSwap(char x) { return x; } //useful in template code were T may or may not be a char
inline short oByteSwap(short x) { unsigned short r = oByteSwap(*(unsigned short*)&x); return *(short*)&r; }
inline int oByteSwap(int x) { unsigned int r = oByteSwap(*(unsigned int*)&x); return *(int*)&r; }
inline long long oByteSwap(long long x) { unsigned long long r = oByteSwap(*(unsigned long long*)&x); return *(long long*)&r; }
inline float oByteSwap(float x) { unsigned int r = oByteSwap(*(unsigned int*)&x); return *(float*)&r; }
inline double oByteSwap(double x) { unsigned long long r = oByteSwap(*(unsigned long long*)&x); return *(double*)&r; }
template<typename T> SafeInt<T> oByteSwap(SafeInt<T> x) { return oByteSwap((T)x); }

// Validation

// Returns true if the specified _TestPointer is inside of a range specified by 
// a base pointer and a size
inline bool oInRange(const void* _TestPointer, const void* _BasePointer, size_t _SizeofRange) { return (_TestPointer >= _BasePointer) && (_TestPointer < oByteAdd(_BasePointer, _SizeofRange)); }

// Retursn true if the specified _TestPointer is inside of a range specified by
// a base pointer and an end pointer that points to the first byte beyond the
// range.
inline bool oInRange(const void* _TestPointer, const void* _BasePointer, const void* _EndPointer) { return (_TestPointer >= _BasePointer) && (_TestPointer < _EndPointer); }

#endif
