// $(header)

// Utility functions helpful when dealing with memory buffers and 
// pointers, especially when it is useful to go back and forth 
// between thinking of the buffer as bytes and as its type without
// a lot of casting.
#pragma once
#ifndef oByte_h
#define oByte_h

#include <stddef.h>

// Alignment
template<typename T> inline T oByteAlign(T _Value, size_t _Alignment) { return (T)(((size_t)_Value + _Alignment - 1) & ~(_Alignment - 1)); }
template<typename T> inline T oByteAlignDown(T _Value, size_t _Alignment) { return (T)((size_t)_Value & ~(_Alignment - 1)); }
template<typename T> inline bool oIsByteAligned(T _Value, size_t _Alignment) { return oByteAlign(_Value, _Alignment) == _Value; }

// Offsets
template<typename T> inline T* oByteAdd(T* _Pointer, size_t _NumBytes) { return reinterpret_cast<T*>(((char*)_Pointer) + _NumBytes); }
template<typename T, typename U> inline T* oByteAdd(T* _RelativePointer, U* _BasePointer) { return reinterpret_cast<T*>(((char*)_RelativePointer) + reinterpret_cast<size_t>(_BasePointer)); }
template<typename T> inline T* oByteAdd(T* _Pointer, size_t _Stride, size_t _Count) { return reinterpret_cast<T*>(((char*)_Pointer) + _Stride * _Count); }
template<typename T, typename U> inline ptrdiff_t oByteDiff(T* t, U* u) { return (ptrdiff_t)((char*)t - (char*)u); }
template<typename T> inline size_t oBytePadding(T value, size_t alignment) { return static_cast<T>(oByteAlign(value, alignment)) - value; }
template<typename T> inline size_t oIndexOf(T* el, T* base) { return oByteDiff(el, base) / sizeof(T); }
template<typename T> inline bool oIsPow2(T n) { return n ? (((n) & ((n)-1)) == 0) : false; }

// Endian swapping
inline unsigned short oByteSwap(unsigned short x) { return (x<<8) | (x>>8); }
inline unsigned int oByteSwap(unsigned int x) { return (x<<24) | ((x<<8) & 0x00ff0000) | ((x>>8) & 0x0000ff00) | (x>>24); }
inline unsigned long long oByteSwap(unsigned long long x) { return (x<<56) | ((x<<40) & 0x00ff000000000000ll) | ((x<<24) & 0x0000ff0000000000ll) | ((x<<8) & 0x000000ff00000000ll) | ((x>>8) & 0x00000000ff000000ll) | ((x>>24) & 0x0000000000ff0000ll) | ((x>>40) & 0x000000000000ff00ll) | (x>>56); }
inline short oByteSwap(short x) { unsigned short r = oByteSwap(*(unsigned short*)&x); return *(short*)&r; }
inline int oByteSwap(int x) { unsigned int r = oByteSwap(*(unsigned int*)&x); return *(int*)&r; }
inline long long oByteSwap(long long x) { unsigned long long r = oByteSwap(*(unsigned long long*)&x); return *(long long*)&r; }

#endif
