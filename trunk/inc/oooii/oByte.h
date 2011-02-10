/**************************************************************************
 * The MIT License                                                        *
 * Copyright (c) 2011 Antony Arciuolo & Kevin Myers                       *
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
#pragma once
#ifndef oByte_h
#define oByte_h

// Endian swapping
inline unsigned short oByteSwap(unsigned short x) { return (x<<8) | (x>>8); }
inline unsigned int oByteSwap(unsigned int x) { return (x<<24) | ((x<<8) & 0x00ff0000) | ((x>>8) & 0x0000ff00) | (x>>24); }
inline unsigned long long oByteSwap(unsigned long long x) { return (x<<56) | ((x<<40) & 0x00ff000000000000ll) | ((x<<24) & 0x0000ff0000000000ll) | ((x<<8) & 0x000000ff00000000ll) | ((x>>8) & 0x00000000ff000000ll) | ((x>>24) & 0x0000000000ff0000ll) | ((x>>40) & 0x000000000000ff00ll) | (x>>56); }
inline short oByteSwap(short x) { unsigned short r = oByteSwap(*(unsigned short*)&x); return *(short*)&r; }
inline int oByteSwap(int x) { unsigned int r = oByteSwap(*(unsigned int*)&x); return *(int*)&r; }
inline long long oByteSwap(long long x) { unsigned long long r = oByteSwap(*(unsigned long long*)&x); return *(long long*)&r; }

// Alignment
inline size_t oByteAlign(size_t value, size_t alignment) { return ((value + alignment - 1) & ~(alignment - 1)); }
inline void* oByteAlign(void* value, size_t alignment) { return reinterpret_cast<void*>(((reinterpret_cast<size_t>(value) + alignment - 1) & ~(alignment - 1))); }
inline size_t oByteAlignDown(size_t value, size_t alignment) { return (value & ~(alignment - 1)); }
inline void* oByteAlignDown(void* value, size_t alignment) { return reinterpret_cast<void*>((reinterpret_cast<size_t>(value) & ~(alignment - 1))); }
inline bool oIsByteAligned(size_t value, size_t alignment) { return oByteAlign(value, alignment) == value; }
inline bool oIsByteAligned(void* value, size_t alignment) { return oByteAlign(value, alignment) == value; }

// Offsets
template<typename T> inline size_t oBytePadding(T value, size_t alignment) { return static_cast<T>(oByteAlign(value, alignment)) - value; }
template<typename T> inline T* oByteAdd(T* _Pointer, size_t _NumBytes) { return reinterpret_cast<T*>(((char*)_Pointer) + _NumBytes); }
template<typename T> inline T* oByteAdd(T* _Pointer, size_t _Stride, size_t _Count) { return reinterpret_cast<T*>(((char*)_Pointer) + _Stride * _Count); }
template<typename T, typename U> inline ptrdiff_t oByteDiff(T* t, U* u) { return (ptrdiff_t)((char*)t - (char*)u); }
template<typename T> inline size_t oIndexOf(T* el, T* base) { return oByteDiff(el, base) / sizeof(T); }
template<typename T> inline bool oIsPow2(T n) { return n ? (((n) & ((n)-1)) == 0) : false; }

#endif
