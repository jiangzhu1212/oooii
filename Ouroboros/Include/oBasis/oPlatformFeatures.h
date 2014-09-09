// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.
// This header describes meta-information about the platform and defines certain
// keywords in a way all systems can use them, whether they're meaningful on 
// the current platform or not (interface, override).
#pragma once
#ifndef oPlatformFeatures_h
#define oPlatformFeatures_h

#include <oCompiler.h>

#ifdef _MSC_VER
	#define oHAS_ASSUME

	#if _MSC_VER < 1600
		#define oCLICKABLE_OUTPUT_REQUIRES_SPACE_COLON
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
		oBUILD_TRACE("exporting dynamic module symbols")
		#define __declspec(dllexport)
	#else
		oBUILD_TRACE("importing dynamic module symbols")
		#define __declspec(dllimport)
	#endif
#else
	oBUILD_TRACE("linking static module symbols")
	#define oAPI
#endif

#define oMODULE_DEBUG_SUFFIX_A "D"

#ifdef _DEBUG
	#define oMODULE_DEBUG_SUFFIX oMODULE_DEBUG_SUFFIX_A
#else
	#define oMODULE_DEBUG_SUFFIX ""
#endif

#endif
