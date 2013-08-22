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
// This header is designed to cross-compile in both C++ and HLSL. This defines
// for C++ HLSL-specified versions of atomic/interlocked operators.
// 
#ifndef oHLSL
	#pragma once
#endif
#ifndef oHLSLAtomics_h
#define oHLSLAtomics_h

#ifndef oHLSL

#include <oStd/oStdAtomic.h>

template<typename T> void InterlockedAdd(T& dest, T value, T& original_value) { original_value = oStd::atomic_fetch_add(&dest, value); }
template<typename T> void InterlockedAdd(T& dest, T value) { oStd::atomic_fetch_add(&dest, value); }
template<typename T> void InterlockedAnd(T& dest, T value, T& original_value) { original_value = oStd::atomic_fetch_and(&dest, value); }
template<typename T> void InterlockedAnd(T& dest, T value) { oStd::atomic_fetch_and(&dest, value); }
template<typename T> void InterlockedCompareExchange(T& dest, T compare_value, T value, T& original_value) { original_value = oStd::atomic_compare_exchange(&dest, value, compare_value); }
template<typename T> void InterlockedCompareExchange(T& dest, T compare_value, T value) { oStd::atomic_compare_exchange(&dest, value, compare_value); }
//template<typename T> void InterlockedCompareStore(R& dest, T compare_value, T value);
template<typename T> void InterlockedExchange(T& dest, T value, T& original_value) { original_value = oStd::atomic_exchange(&dest, value); }
template<typename T> void InterlockedExchange(T& dest, T value) { oStd::atomic_exchange(&dest, value); }

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

template<typename T> void InterlockedOr(T& dest, T value, T& original_value) { original_value = oStd::atomic_fetch_or(&dest, value); }
template<typename T> void InterlockedOr(T& dest, T value) { oStd::atomic_fetch_or(&dest, value); }
template<typename T> void InterlockedXor(T& dest, T value, T& original_value) { original_value = oStd::atomic_fetch_xor(&dest, value); }
template<typename T> void InterlockedXor(T& dest, T value) { oStd::atomic_fetch_xor(&dest, value); }

#endif
#endif