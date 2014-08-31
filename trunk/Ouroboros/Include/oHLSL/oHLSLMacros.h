// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.
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
