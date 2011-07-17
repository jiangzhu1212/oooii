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

// See oMutex.h for why this is exposed. This header must forward-declare -
// without including any platform headers - the platform space for the class.
// This is necessary to ensure that a mutex remains fast by avoiding a 
// virtual interface and still remain somewhat abstract.

// The goal is to define the macros oMUTEX_FOOTPRINT() and oRWMUTEX_FOOTPRINT()
// for use in oMutex.h
#pragma once
#ifndef oMutexInternal_h
#define oMutexInternal_h

#if defined(_WIN32) || defined(_WIN64)

	// oMUTEX_FOOTPRINT
	#ifdef _WIN64
		#define oMUTEX_FOOTPRINT() mutable unsigned long long Footprint[5] // RTL_CRITICAL_SECTION
	#elif defined(_WIN32)
		#define oMUTEX_FOOTPRINT() mutable unsigned int Footprint[6]
	#endif

	// oRWMUTEX_FOOTPRINT
	#ifdef _DEBUG
		// Debug info for tagging which thread we're on to allow for protection against
		// attempts to recursively lock.
		#define oRWMUTEX_FOOTPRINT() void* Footprint; mutable size_t ThreadID
	#else
		#define oRWMUTEX_FOOTPRINT() void* Footprint
	#endif

#else
	#error Unsupported platform (oMUTEX_FOOTPRINT, oRWMUTEX_FOOTPRINT)
#endif
#endif
