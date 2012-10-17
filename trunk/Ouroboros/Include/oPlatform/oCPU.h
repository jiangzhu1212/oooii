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
// Utility for querying the Central Processing Unit hardware of the current 
// computer.
#pragma once
#ifndef oCPU_h
#define oCPU_h

enum oCPU_TYPE
{
	oCPU_UNKNOWN,
	oCPU_X86,
	oCPU_X64,
	oCPU_IA64,
	oCPU_ARM,
};

oAPI const char* oAsString(const oCPU_TYPE& _Type);

enum oCPU_SUPPORT_TYPE
{
	oCPU_NOT_FOUND, // the feature is not exposed
	oCPU_NO_SUPPORT,
	oCPU_HW_SUPPORT_ONLY,
	oCPU_FULL_SUPPORT, // both the current platform and HW support the feature
};

oAPI const char* oAsString(const oCPU_SUPPORT_TYPE& _Type);

struct oCPU_CACHE_DESC
{
	unsigned int Size;
	unsigned int LineSize;
	unsigned int Associativity;
};

struct oCPU_DESC
{
	oCPU_TYPE Type;
	int NumProcessors;
	int NumProcessorPackages;
	int NumNumaNodes;
	int NumHardwareThreads;
	oCPU_CACHE_DESC DataCacheDescs[3];
	oCPU_CACHE_DESC InstructionCacheDescs[3];
	char String[32];
	char BrandString[64];
};

// Returns false if the specified CPU doesn't exist.
oAPI bool oCPUEnum(int _CPUIndex, oCPU_DESC* _pDesc);

// Returns the human-readable feature string for the nth feature supported by 
// this CPU. This will return nullptr at the end of the list. This string can be 
// passed to. Basically this is the "documentation" for what can be queried by
// oCPUCheckFeatureSupport. It's meant more for reference than directly useful
// in an application.
oAPI const char* oCPUEnumFeatures(int _FeatureIndex);

// Given a string that is the same as returned by oCPUEnumFeatures, return at 
// what level that feature is supported. If an unrecognized string is passed, 
// this will return oCPU_NO_SUPPORT.
oAPI oCPU_SUPPORT_TYPE oCPUCheckFeatureSupport(int _CPUIndex, const char* _Feature);

#endif
