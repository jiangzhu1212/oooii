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
// This header is designed to cross-compile in both C++ and HLSL. This 
// defines for C++ HLSL-specified versions of atomic/interlocked operators.
#ifndef oHLSL
	#pragma once
#endif
#ifndef oHLSLAtomics_h
#define oHLSLAtomics_h

#ifndef oHLSL

#include <oHLSL/oHLSLIntrinsics.h>

inline void InterlockedCompareExchange(bool& dest, bool compare_value, bool value, bool& original_value) { char v = _InterlockedCompareExchange8((volatile char*)&dest, *(char*)&value, *(char*)&compare_value); original_value = *(bool*)v; }
inline void InterlockedCompareExchange(char& dest, char compare_value, char value, char& original_value) { original_value = _InterlockedCompareExchange8(&dest, value, compare_value); }
inline void InterlockedCompareExchange(short& dest, short compare_value, short value, short& original_value) { original_value = _InterlockedCompareExchange16(&dest, value, compare_value); }
inline void InterlockedCompareExchange(int& dest, int compare_value, int value, int& original_value) { original_value = (int)_InterlockedCompareExchange((volatile long*)&dest, (long)value, (long)compare_value); }
inline void InterlockedCompareExchange(long& dest, long compare_value, long value, long& original_value) { original_value = _InterlockedCompareExchange(&dest, value, compare_value); }
inline void InterlockedCompareExchange(long long& dest, long long compare_value, long long value, long long& original_value) { original_value = _InterlockedCompareExchange64(&dest, value, compare_value); }

inline void InterlockedCompareExchange(unsigned char& dest, unsigned char compare_value, unsigned char value, unsigned char& original_value) { char v = _InterlockedCompareExchange8((volatile char*)&dest, *(char*)&value, *(char*)&compare_value); original_value = *(unsigned char*)&v; }
inline void InterlockedCompareExchange(unsigned short& dest, unsigned short compare_value, unsigned short value, unsigned short& original_value) { short v = _InterlockedCompareExchange16((volatile short*)&dest, *(short*)&value, *(short*)&compare_value); original_value = *(unsigned short*)&v; }
inline void InterlockedCompareExchange(unsigned int& dest, unsigned int compare_value, unsigned int value, unsigned int& original_value) { long v = _InterlockedCompareExchange((volatile long*)&dest, *(long*)&value, *(long*)&compare_value); original_value = *(unsigned int*)&v; }
inline void InterlockedCompareExchange(unsigned long& dest, unsigned long compare_value, unsigned long value, unsigned long& original_value) { long v = _InterlockedCompareExchange((volatile long*)&dest, *(long*)&value, *(long*)&compare_value); original_value = *(unsigned long*)&v; }
inline void InterlockedCompareExchange(unsigned long long& dest, unsigned long long compare_value, unsigned long long value, unsigned long long& original_value) { long long v = _InterlockedCompareExchange64((volatile long long*)&dest, *(long long*)&value, *(long long*)&compare_value); original_value = *(unsigned long long*)&v; }

inline void InterlockedCompareStore(bool& dest, bool compare_value, bool value) { _InterlockedCompareExchange8((volatile char*)&dest, *(char*)&value, *(char*)&compare_value); }
inline void InterlockedCompareStore(char& dest, char compare_value, char value) { _InterlockedCompareExchange8(&dest, value, compare_value); }
inline void InterlockedCompareStore(short& dest, short compare_value, short value) { _InterlockedCompareExchange16(&dest, value, compare_value); }
inline void InterlockedCompareStore(int& dest, int compare_value, int value) { _InterlockedCompareExchange((volatile long*)&dest, (long)value, (long)compare_value); }
inline void InterlockedCompareStore(long& dest, long compare_value, long value) { _InterlockedCompareExchange(&dest, value, compare_value); }
inline void InterlockedCompareStore(long long& dest, long long compare_value, long long value) { _InterlockedCompareExchange64(&dest, value, compare_value); }

inline void InterlockedCompareStore(unsigned char& dest, unsigned char compare_value, unsigned char value) { char v = _InterlockedCompareExchange8((volatile char*)&dest, *(char*)&value, *(char*)&compare_value); }
inline void InterlockedCompareStore(unsigned short& dest, unsigned short compare_value, unsigned short value) { short v = _InterlockedCompareExchange16((volatile short*)&dest, *(short*)&value, *(short*)&compare_value); }
inline void InterlockedCompareStore(unsigned int& dest, unsigned int compare_value, unsigned int value) { long v = _InterlockedCompareExchange((volatile long*)&dest, *(long*)&value, *(long*)&compare_value); }
inline void InterlockedCompareStore(unsigned long& dest, unsigned long compare_value, unsigned long value) { long v = _InterlockedCompareExchange((volatile long*)&dest, *(long*)&value, *(long*)&compare_value); }
inline void InterlockedCompareStore(unsigned long long& dest, unsigned long long compare_value, unsigned long long value) { _InterlockedCompareExchange64((volatile long long*)&dest, *(long long*)&value, *(long long*)&compare_value); }

inline void InterlockedExchange(bool& dest, bool value, bool& original_value) { char v = _InterlockedExchange8((volatile char*)&dest, *(char*)&value); original_value = *(bool*)&v; }
inline void InterlockedExchange(char& dest, char value, char& original_value) { original_value = _InterlockedExchange8(&dest, value); }
inline void InterlockedExchange(short& dest, short value, short& original_value) { original_value = _InterlockedExchange16(&dest, value); }
inline void InterlockedExchange(int& dest, int value, int& original_value) { original_value = (int)_InterlockedExchange((volatile long*)&dest, *(long*)&value); }
inline void InterlockedExchange(long& dest, long value, long& original_value) { original_value = _InterlockedExchange(&dest, value); }
inline void InterlockedExchange(long long& dest, long long value, long long& original_value) { original_value = _InterlockedExchange64(&dest, value); }

inline void InterlockedExchange(unsigned char& dest, unsigned char value, unsigned char& original_value) { char v = _InterlockedExchange8((volatile char*)&dest, *(char*)&value); original_value = *(bool*)&v; }
inline void InterlockedExchange(unsigned short& dest, unsigned short value, unsigned short& original_value) { short v = _InterlockedExchange16((volatile short*)&dest, *(short*)&value); original_value = *(unsigned short*)&v; }
inline void InterlockedExchange(unsigned int& dest, unsigned int value, unsigned int& original_value) { long v = _InterlockedExchange((volatile long*)&dest, *(long*)&value); original_value = *(unsigned int*)&v; }
inline void InterlockedExchange(unsigned long& dest, unsigned long value, unsigned long& original_value) { long v = _InterlockedExchange((volatile long*)&dest, *(long*)&value); original_value = *(unsigned long*)&v; }
inline void InterlockedExchange(unsigned long long& dest, unsigned long long value, unsigned long long& original_value) { long long v = _InterlockedExchange64((volatile long long*)&dest, *(long long*)&value); original_value = *(unsigned long long*)&v; }

inline void InterlockedExchange(bool& dest, bool value) { _InterlockedExchange8((volatile char*)&dest, *(char*)&value); }
inline void InterlockedExchange(char& dest, char value) { _InterlockedExchange8(&dest, value); }
inline void InterlockedExchange(short& dest, short value) { _InterlockedExchange16(&dest, value); }
inline void InterlockedExchange(int& dest, int value) { _InterlockedExchange((volatile long*)&dest, *(long*)&value); }
inline void InterlockedExchange(long& dest, long value) { _InterlockedExchange(&dest, value); }
inline void InterlockedExchange(long long& dest, long long value) { _InterlockedExchange64(&dest, value); }

inline void InterlockedExchange(unsigned char& dest, unsigned char value) { _InterlockedExchange8((volatile char*)&dest, *(char*)&value); }
inline void InterlockedExchange(unsigned short& dest, unsigned short value) { _InterlockedExchange16((volatile short*)&dest, *(short*)&value); }
inline void InterlockedExchange(unsigned int& dest, unsigned int value) { _InterlockedExchange((volatile long*)&dest, *(long*)&value); }
inline void InterlockedExchange(unsigned long& dest, unsigned long value) { _InterlockedExchange((volatile long*)&dest, *(long*)&value); }
inline void InterlockedExchange(unsigned long long& dest, unsigned long long value) { _InterlockedExchange64((volatile long long*)&dest, *(long long*)&value); }

inline void InterlockedAdd(int& dest, int value, long& original_value) { original_value = (int)_InterlockedExchangeAdd((volatile long*)&dest, (long)value); }
inline void InterlockedAdd(long& dest, long value, long& original_value) { original_value = _InterlockedExchangeAdd(&dest, value); }
//inline void InterlockedAdd(long long& dest, long long value, long long& original_value) { original_value = _InterlockedExchangeAdd64(&dest, value); }

inline void InterlockedAdd(unsigned int& dest, unsigned int value, unsigned int& original_value) { long v = _InterlockedExchangeAdd((volatile long*)&dest, (long)value); original_value = *(unsigned int*)&v; }
inline void InterlockedAdd(unsigned long& dest, unsigned long value, unsigned long& original_value) { long v = _InterlockedExchangeAdd((volatile long*)&dest, *(long*)&value); original_value = *(unsigned long*)&v; }
//inline void InterlockedAdd(unsigned long long& dest, unsigned long long value, unsigned long long& original_value) { long long v = _InterlockedExchangeAdd64((volatile long long*)&dest, *(long long*)&value); original_value = *(unsigned long long*)&v; }

inline void InterlockedAdd(int& dest, int value) { _InterlockedExchangeAdd((volatile long*)&dest, (long)value); }
inline void InterlockedAdd(long& dest, long value) { _InterlockedExchangeAdd(&dest, value); }
//inline void InterlockedAdd(long long& dest, long long value) { _InterlockedExchangeAdd64(&dest, value); }

inline void InterlockedAdd(unsigned int& dest, unsigned int value) { _InterlockedExchangeAdd((volatile long*)&dest, (long)value); }
inline void InterlockedAdd(unsigned long& dest, unsigned long value) { _InterlockedExchangeAdd((volatile long*)&dest, *(long*)&value); }
//inline void InterlockedAdd(unsigned long long& dest, unsigned long long value) { _InterlockedExchangeAdd64((volatile long long*)&dest, *(long long*)&value); }

inline void InterlockedAnd(short& dest, short value) { _InterlockedAnd16(&dest, value); }
inline void InterlockedAnd(int& dest, int value) { _InterlockedAnd((volatile long*)&dest, (long)value); }
inline void InterlockedAnd(long& dest, long value) { _InterlockedAnd(&dest, value); }
inline void InterlockedAdd(long long& dest, long long value) { _InterlockedAnd64(&dest, value); }

inline void InterlockedAnd(unsigned short& dest, unsigned short value) { _InterlockedAnd16((volatile short*)&dest, *(short*)&value); }
inline void InterlockedAnd(unsigned int& dest, unsigned int value) { _InterlockedAnd((volatile long*)&dest, *(long*)&value); }
inline void InterlockedAnd(unsigned long& dest, unsigned long value) { _InterlockedAnd((volatile long*)&dest, *(long*)&value); }
inline void InterlockedAnd(unsigned long long& dest, unsigned long long value) { _InterlockedAnd64((volatile long long*)&dest, *(long long*)&value); }

inline void InterlockedAnd(short& dest, short value, short& original_value) { original_value = _InterlockedAnd16(&dest, value); }
inline void InterlockedAnd(int& dest, int value, int& original_value) { original_value = (int)_InterlockedAnd((volatile long*)&dest, (long)value); }
inline void InterlockedAnd(long& dest, long value, long& original_value) { original_value = _InterlockedAnd(&dest, value); }
inline void InterlockedAdd(long long& dest, long long value, long long& original_value) { original_value = _InterlockedAnd64(&dest, value); }

inline void InterlockedAnd(unsigned short& dest, unsigned short value, unsigned short& original_value) { short v = _InterlockedAnd16((volatile short*)&dest, *(short*)&value); original_value = *(unsigned short*)&v; }
inline void InterlockedAnd(unsigned int& dest, unsigned int value, unsigned int& original_value) { long v = _InterlockedAnd((volatile long*)&dest, *(long*)&value); original_value = *(unsigned long*)&v; }
inline void InterlockedAnd(unsigned long& dest, unsigned long value, unsigned long& original_value) { long v = _InterlockedAnd((volatile long*)&dest, *(long*)&value); original_value = *(unsigned int*)&v; }
inline void InterlockedAnd(unsigned long long& dest, unsigned long long value, unsigned long long& original_value) { long long v = _InterlockedAnd64((volatile long long*)&dest, *(long long*)&value); original_value = *(unsigned long*)&v; }

inline void InterlockedOr(short& dest, short value) { _InterlockedOr16(&dest, value); }
inline void InterlockedOr(long& dest, long value) { _InterlockedOr(&dest, value); }
inline void InterlockedOr(int& dest, int value) { _InterlockedOr((volatile long*)&dest, (long)value); }
inline void InterlockedOr(long long& dest, long long value) { _InterlockedOr64(&dest, value); }

inline void InterlockedOr(unsigned short& dest, unsigned short value) { _InterlockedOr16((volatile short*)&dest, *(short*)&value); }
inline void InterlockedOr(unsigned int& dest, unsigned int value) { _InterlockedOr((volatile long*)&dest, *(long*)&value); }
inline void InterlockedOr(unsigned long& dest, unsigned long value) { _InterlockedOr((volatile long*)&dest, *(long*)&value); }
inline void InterlockedOr(unsigned long long& dest, unsigned long long value) { _InterlockedOr64((volatile long long*)&dest, *(long long*)&value); }

inline void InterlockedOr(short& dest, short value, short& original_value) { original_value = _InterlockedOr16(&dest, value); }
inline void InterlockedOr(long& dest, long value, long& original_value) { original_value = _InterlockedOr(&dest, value); }
inline void InterlockedOr(int& dest, int value, int& original_value) { original_value = (int)_InterlockedOr((volatile long*)&dest, (long)value); }
inline void InterlockedOr(long long& dest, long long value, long long& original_value) { original_value = _InterlockedOr64(&dest, value); }

inline void InterlockedOr(unsigned short& dest, unsigned short value, unsigned short& original_value) { short v = _InterlockedOr16((volatile short*)&dest, *(short*)&value); original_value = *(unsigned short*)&v; }
inline void InterlockedOr(unsigned int& dest, unsigned int value, unsigned int& original_value) { long v = _InterlockedOr((volatile long*)&dest, *(long*)&value); original_value = *(unsigned int*)&v; }
inline void InterlockedOr(unsigned long& dest, unsigned long value, unsigned long& original_value) { long v = _InterlockedOr((volatile long*)&dest, *(long*)&value); original_value = *(unsigned long*)&v; }
inline void InterlockedOr(unsigned long long& dest, unsigned long long value, unsigned long long& original_value) { long long v = _InterlockedOr64((volatile long long*)&dest, *(long long*)&value); original_value = *(unsigned long long*)&v; }

inline void InterlockedXor(short& dest, short value) { _InterlockedXor16(&dest, value); }
inline void InterlockedXor(long& dest, long value) { _InterlockedXor(&dest, value); }
inline void InterlockedXor(int& dest, int value) { _InterlockedXor((volatile long*)&dest, (long)value); }
inline void InterlockedXor(long long& dest, long long value) { _InterlockedXor64(&dest, value); }

inline void InterlockedXor(unsigned short& dest, unsigned short value) { _InterlockedXor16((volatile short*)&dest, *(short*)&value); }
inline void InterlockedXor(unsigned int& dest, unsigned int value) { _InterlockedXor((volatile long*)&dest, *(long*)&value); }
inline void InterlockedXor(unsigned long& dest, unsigned long value) { _InterlockedXor((volatile long*)&dest, *(long*)&value); }
inline void InterlockedXor(unsigned long long& dest, unsigned long long value) { _InterlockedXor64((volatile long long*)&dest, *(long long*)&value); }

inline void InterlockedXor(short& dest, short value, short& original_value) { original_value = _InterlockedXor16(&dest, value); }
inline void InterlockedXor(long& dest, long value, long& original_value) { original_value = _InterlockedXor(&dest, value); }
inline void InterlockedXor(int& dest, int value, int& original_value) { original_value = (int)_InterlockedXor((volatile long*)&dest, (long)value); }
inline void InterlockedXor(long long& dest, long long value, long long& original_value) { original_value = _InterlockedXor64(&dest, value); }

inline void InterlockedXor(unsigned short& dest, unsigned short value, unsigned short& original_value) { short v = _InterlockedXor16((volatile short*)&dest, *(short*)&value); original_value = *(unsigned short*)&v; }
inline void InterlockedXor(unsigned int& dest, unsigned int value, unsigned int& original_value) { long v = _InterlockedXor((volatile long*)&dest, *(long*)&value); original_value = *(unsigned int*)&v; }
inline void InterlockedXor(unsigned long& dest, unsigned long value, unsigned long& original_value) { long v = _InterlockedXor((volatile long*)&dest, *(long*)&value); original_value = *(unsigned long*)&v; }
inline void InterlockedXor(unsigned long long& dest, unsigned long long value, unsigned long long& original_value) { long long v = _InterlockedXor64((volatile long long*)&dest, *(long long*)&value); original_value = *(unsigned long long*)&v; }

template<typename T> void InterlockedMax(T& dest, T value, T& original_value)
{
	T New, Old;
	do
	{	Old = dest;
		New = Old > value ? Old : value;
		InterlockedCompareExchange(dest, Old, New, original_value);
	} while (original_value != Old);
}

template<typename T> void InterlockedMax(T& dest, T value)
{
	T original_value;
	InterlockedMax(dest, value, original_value);
}

template<typename T> void InterlockedMin(T& dest, T value, T& original_value)
{
	T New, Old;
	do
	{	Old = dest;
		New = Old < value ? Old : value;
		InterlockedCompareExchange(dest, Old, New, original_value);
	} while (original_value != Old);
}

template<typename T> void InterlockedMin(T& dest, T value)
{
	T original_value;
	InterlockedMin(dest, value, original_value);
}


#endif
#endif
