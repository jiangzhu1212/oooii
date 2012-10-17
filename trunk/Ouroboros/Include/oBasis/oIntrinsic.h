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

// Forward declaration of compiler intrinsic functions. This is declared here
// instead of using <intrin.h> because of conflicts in MSVC between declarations
// of ceil in <intrin.h> and <math.h>
#pragma once
#ifndef oIntrinsic_h
#define oIntrinsic_h

#include <oBasis/oPlatformFeatures.h>

#ifdef __cplusplus
	extern "C" {
#endif

// _____________________________________________________________________________
// Bit scanning

unsigned char _BitScanForward(unsigned long* Index, unsigned long Mask);
unsigned char _BitScanReverse(unsigned long* Index, unsigned long Mask);
#pragma intrinsic(_BitScanReverse)
#pragma intrinsic(_BitScanForward)

#ifdef o64BIT
	unsigned char _BitScanForward64(unsigned long* Index, unsigned __int64 Mask);
	unsigned char _BitScanReverse64(unsigned long* Index, unsigned __int64 Mask);
	#pragma intrinsic(_BitScanReverse64)
	#pragma intrinsic(_BitScanForward64)
#endif

// _____________________________________________________________________________
// Atomics

void _ReadBarrier();
void _ReadWriteBarrier();

short _InterlockedIncrement16(short volatile *Addend);
long _InterlockedIncrement(long volatile *Addend);
long long _InterlockedIncrement64(long long volatile *Addend);
short _InterlockedDecrement16(short volatile *Addend);
long _InterlockedDecrement(long volatile *Addend);
long long _InterlockedDecrement64(long long volatile *Addend);
char _InterlockedExchange8(char volatile *Target, char Value);
short _InterlockedExchange16(short volatile *Target, short Value);
long _InterlockedExchange(long volatile *Target, long Value);
char  _InterlockedCompareExchange8(char  volatile *Destination, char  ExChange, char  Comperand);
short _InterlockedCompareExchange16(short volatile *Destination, short ExChange, short Comperand);
long _InterlockedCompareExchange(long volatile *Destination, long ExChange, long Comperand);
#pragma intrinsic(_InterlockedIncrement16)
#pragma intrinsic(_InterlockedIncrement)
#pragma intrinsic(_InterlockedDecrement16)
#pragma intrinsic(_InterlockedDecrement)
#pragma intrinsic(_InterlockedExchange)
#pragma intrinsic(_InterlockedCompareExchange)

#ifdef oHAS_8BIT_ATOMICS
	#pragma intrinsic(_InterlockedExchange8)
	#pragma intrinsic(_InterlockedCompareExchange8)
#endif

#ifdef oHAS_16BIT_ATOMICS
	#pragma intrinsic(_InterlockedExchange16)
	#pragma intrinsic(_InterlockedCompareExchange16)
#endif

#ifdef o64BIT
	void* _InterlockedExchangePointer(void* volatile *Target, void* Value);
	void* _InterlockedCompareExchangePointer(void* volatile *Destination, void* Exchange, void* Comperand);

	#pragma intrinsic(_InterlockedExchangePointer)
	#pragma intrinsic(_InterlockedCompareExchangePointer)
#else
	inline void* _InterlockedExchangePointer(void* volatile *Target, void* Value) { return (void*)_InterlockedExchange((long volatile*)Target, (long)Value); }
	inline void* _InterlockedCompareExchangePointer(void* volatile *Destination, void* Exchange, void* Comperand) { return (void*)_InterlockedCompareExchange((long volatile*)Destination, (long)(long*)Exchange, (long)(long*)Comperand); }
#endif

#ifdef oHAS_64BIT_ATOMICS
	long long _InterlockedIncrement64(long long volatile *Addend);
	long long _InterlockedDecrement64(long long volatile *Addend);
	long long _InterlockedExchange64(long long volatile *Target, long long Value);
	long long _InterlockedCompareExchange64(long long volatile *Destination, long long ExChange, long long Comperand);

	#ifdef o64BIT
		#pragma intrinsic(_InterlockedIncrement64)
		#pragma intrinsic(_InterlockedDecrement64)
		#pragma intrinsic(_InterlockedExchange64)
		#pragma intrinsic(_InterlockedCompareExchange64)
	#endif
#endif

#ifdef __cplusplus
	}
#endif
#endif
