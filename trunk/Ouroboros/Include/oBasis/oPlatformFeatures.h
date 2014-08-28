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

#define oMODULE_DEBUG_SUFFIX_A "D"

#ifdef _DEBUG
	#define oMODULE_DEBUG_SUFFIX oMODULE_DEBUG_SUFFIX_A
#else
	#define oMODULE_DEBUG_SUFFIX ""
#endif

#endif
