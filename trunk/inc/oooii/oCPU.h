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
#pragma once
#ifndef oCPU_h
#define oCPU_h

namespace oCPU {

enum TYPE
{
	UNKNOWN,
	X86,
	X64,
	IA64,
};

enum CACHE_TYPE
{
	DATA,
	INSTRUCTION,
};

struct CACHE_DESC
{
	unsigned int Size;
	unsigned int LineSize;
	unsigned int Associativity;
};

struct DESC
{
	TYPE Type;
	unsigned int NumProcessors;
	unsigned int NumProcessorPackages;
	unsigned int NumNumaNodes;
	unsigned int NumHardwareThreads;
	unsigned int SSEVersion;
	CACHE_DESC DataCacheDescs[3];
	CACHE_DESC InstructionCacheDescs[3];
	bool HasX87FPU;
	bool Has8ByteAtomicSwap;
	bool HasHyperThreading;
	char String[32];
	char BrandString[64];
};

// Returns false if the specified CPU doesn't exist.
bool GetDesc(unsigned int _CPUIndex, DESC* _pDesc);

} // namespace oCPU

#endif
