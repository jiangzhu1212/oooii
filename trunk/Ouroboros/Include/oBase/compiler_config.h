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
// compiler-dependent definitions
#pragma once
#ifndef oBase_compiler_h
#define oBase_compiler_h

#define oVS2010_VER 1600
#define oVS2012_VER 1700
#define oVS2013_VER 1800

#if defined(_MSC_VER) && _MSC_VER == oVS2012_VER

#define oCACHE_LINE_SIZE 64

#if defined(_WIN64) || defined(WIN64)
	#define o64BIT 1
	#define o32BIT 0
	#define oDEFAULT_MEMORY_ALIGNMENT 16
	#define oHAS_DOUBLE_WIDE_ATOMIC_BUG 0
#elif defined(_WIN32) || defined(WIN32)
	#define o64BIT 0
	#define o32BIT 1
	#define oDEFAULT_MEMORY_ALIGNMENT 4
	#define oHAS_DOUBLE_WIDE_ATOMIC_BUG 1
#else
	#error unsupported platform
#endif

#define oDEBUGBREAK __debugbreak()

// C++11 support
#define oALIGNAS(x) __declspec(align(x))
#define oALIGNOF(x) __alignof(x)
#define oNOEXCEPT
#define oTHREAD_LOCAL __declspec(thread)
#define oHAS_CBEGIN 0

// low-level optimization support
#define oRESTRICT __restrict
#define oASSUME(x) __assume(x)
#define oFORCEINLINE __forceinline

// disable warnings that don't seem to be squelched in project settings
#pragma warning(disable:4481) // nonstandard extension used: override specifier 'override'
#pragma warning(disable:4324) // structure was padded due to _declspec(align())

#else
	#error unsupported compiler
#endif

#endif
