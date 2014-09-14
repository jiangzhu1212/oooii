// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.

// compiler-dependent definitions

#pragma once

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
