// $(header)
// This header describes meta-information about the platform and defines certain
// keywords in a way all systems can use them, whether they're meaningful on 
// the current platform or not (interface, override).
#pragma once
#ifndef oPlatformFeatures_h
#define oPlatformFeatures_h

#ifdef _MSC_VER
	#pragma warning(disable:4481) // nonstandard extension used: override specifier 'override'
	#define oHAS_ASSUME

	#if _MSC_VER >= 1600
		#define oHAS_AUTO
		#define oHAS_SHMUTEX_TRYLOCK
		#define oHAS_NULLPTR
		#define oHAS_STATIC_ASSERT
		#define oHAS_MOVE_CTOR
		#define oHAS_8BIT_ATOMICS
		#define	oHAS_16BIT_ATOMICS
		#define oHAS_TYPE_TRAITS
	#else
		#define oCLICKABLE_OUTPUT_REQUIRES_SPACE_COLON
	#endif

	#define oLITTLEENDIAN

	#ifdef _WIN64
		#define oPOINTERSIZE 8
		#define oDEFAULT_MEMORY_ALIGNMENT 16
		#define o64BIT 1
	#else
		#define oPOINTERSIZE 4
		#define oDEFAULT_MEMORY_ALIGNMENT 8
		#define o32BIT 1
	#endif

	#define oRESTRICT __restrict
	#define oFORCEINLINE __forceinline
	#define oALIGN(amount) __declspec(align(amount))

#else
	#define override
#endif

#ifdef interface
	#define INTERFACE_DEFINED
#endif

#ifndef INTERFACE_DEFINED
	#ifdef _MSC_VER
		#define interface struct __declspec(novtable)
	#else
		#define interface struct
	#endif
	#define INTERFACE_DEFINED
#endif

#ifndef oHAS_NULLPTR
	#define nullptr NULL
#endif

#ifndef oHAS_THREAD_LOCAL
	#ifdef _MSC_VER
		#define thread_local __declspec(thread)
	#else
		#error Unsupported platform
	#endif
#endif

// Enable this in the compiler command line for debugging link/declaration issues
#ifndef oENABLE_BUILD_TRACES
	#define oENABLE_BUILD_TRACES 0
#endif

#ifndef oUSE_CLICKABLE_BUILD_TRACES
	#define oUSE_CLICKABLE_BUILD_TRACES 0
#endif

#if oENABLE_BUILD_TRACES == 0 
	#define oBUILD_TRACE(msg)
#else
	#if oUSE_CLICKABLE_BUILD_TRACES == 0
		#define oBUILD_TRACE(msg) __pragma(message("BUILD TRACE: " msg))
	#else
		#define oBUILD_TRACE(msg) __pragma(message(__FILE__ "(" #__LINE__ ") : BUILD TRACE: " msg))
	#endif
#endif

#if defined(_DLL) && !defined(oSTATICLIB)
	#ifdef _EXPORT_SYMBOLS
		oBUILD_TRACE("oAPI exporting dynamic module symbols")
		#define oAPI __declspec(dllexport)
	#else
		oBUILD_TRACE("oAPI importing dynamic module symbols")
		#define oAPI __declspec(dllimport)
	#endif
#else
	oBUILD_TRACE("oAPI linking static module symbols")
	#define oAPI
#endif

#endif
