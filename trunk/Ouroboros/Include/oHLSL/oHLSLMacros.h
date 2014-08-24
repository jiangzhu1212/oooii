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
// This header is designed to cross-compile in both C++ and HLSL. This defines
// macros to work around HLSL-specific keywords that are not in C++.
#ifndef oHLSL
	#pragma once
#endif
#ifndef oHLSLMacros_h
#define oHLSLMacros_h

#ifdef oHLSL
	#define oIN(Type, Param) in Type Param
	#define oOUT(Type, Param) out Type Param
	#define oINOUT(Type, Param) inout Type Param
	
	#define oHLSL_ALLOW_UAV_CONDITION [allow_uav_condition]
	#define oHLSL_LOOP [loop]
	#define oHLSL_UNROLL [unroll]
	#define oHLSL_UNROLL1(x) [unroll(x)]
	#define oHLSL_FASTOPT [fastopt]
#else
	#define oIN(Type, Param) const Type& Param
	#define oOUT(Type, Param) Type& Param
	#define oINOUT(Type, Param) Type& Param

	#define oHLSL_ALLOW_UAV_CONDITION
	#define oHLSL_LOOP
	#define oHLSL_UNROLL
	#define oHLSL_UNROLL1(x)
	#define oHLSL_FASTOP
#endif
#endif
