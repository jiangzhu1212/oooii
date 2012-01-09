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
#include <oPlatform/oWindows.h>
#include <oBasis/oAssert.h>
#include <oPlatform/oCPU.h>

static errno_t CPUGetString(char* strDestination, size_t numberOfElements)
{
	int CPUInfo[4];
	if (numberOfElements <= sizeof(CPUInfo))
		return ERANGE;
	__cpuid(CPUInfo, 0);
	memset(strDestination, 0, numberOfElements);
	*((int*)strDestination) = CPUInfo[1];
	*((int*)(strDestination+4)) = CPUInfo[3];
	*((int*)(strDestination+8)) = CPUInfo[2];
	return 0;
}

template<size_t size> static inline errno_t CPUGetString(char (&strDestination)[size]) { return CPUGetString(strDestination, size); }

static errno_t CPUGetBrandString(char* _StrDestination, size_t _SizeofStrDestination)
{
	int CPUInfo[4];
	if (_SizeofStrDestination <= sizeof(CPUInfo))
		return ERANGE;

	memset(_StrDestination, 0, _SizeofStrDestination);
	__cpuid(CPUInfo, 0x80000002);
	memcpy(_StrDestination, CPUInfo, sizeof(CPUInfo));
	__cpuid(CPUInfo, 0x80000003);
	memcpy(_StrDestination + 16, CPUInfo, sizeof(CPUInfo));
	__cpuid(CPUInfo, 0x80000004);
	memcpy(_StrDestination + 32, CPUInfo, sizeof(CPUInfo));
	oPruneWhitespace(_StrDestination, _SizeofStrDestination, _StrDestination);
	return 0;
}

template<size_t size> static inline errno_t CPUGetBrandString(char (&_StrDestination)[size]) { return CPUGetBrandString(_StrDestination, size); }

bool oCPUEnum(unsigned int _Index, oCPU_DESC* _pDesc)
{
	if (_Index > 0) // @oooii-tony: More than 1 CPU not yet supported (buy me a dual core machine for testing!)
		return false;

	memset(_pDesc, 0, sizeof(oCPU_DESC));

	SYSTEM_INFO si;
	GetSystemInfo(&si);
	switch (si.wProcessorArchitecture)
	{
		case PROCESSOR_ARCHITECTURE_AMD64: _pDesc->Type = oCPU_X64; break;
		case PROCESSOR_ARCHITECTURE_IA64: _pDesc->Type = oCPU_IA64; break;
		case PROCESSOR_ARCHITECTURE_INTEL: _pDesc->Type = oCPU_X86; break;
		default: _pDesc->Type = oCPU_UNKNOWN; break;
	}

	DWORD size = 0;
	GetLogicalProcessorInformation(0, &size);
	oASSERT(GetLastError() == ERROR_INSUFFICIENT_BUFFER, "");
	SYSTEM_LOGICAL_PROCESSOR_INFORMATION* lpi = static_cast<SYSTEM_LOGICAL_PROCESSOR_INFORMATION*>(_alloca(size));
	memset(lpi, 0xff, size);
	GetLogicalProcessorInformation(lpi, &size);
	const size_t numInfos = size / sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION);

	for (size_t i = 0; i < numInfos; i++)
	{
		switch (lpi[i].Relationship)
		{
		case RelationNumaNode:
			_pDesc->NumNumaNodes++;
			break;

		case RelationProcessorCore:
			_pDesc->NumProcessors++;
			_pDesc->NumHardwareThreads += static_cast<unsigned int>(countbits(lpi[i].ProcessorMask));
			break;

		case RelationCache:
		{
			size_t cacheLevel = lpi[i].Cache.Level-1;
			oCPU_CACHE_DESC& c = lpi[i].Cache.Type == CacheData ? _pDesc->DataCacheDescs[cacheLevel] : _pDesc->InstructionCacheDescs[cacheLevel];
			c.Size = lpi[i].Cache.Size;
			c.LineSize = lpi[i].Cache.LineSize;
			c.Associativity = lpi[i].Cache.Associativity;
		}

		case RelationProcessorPackage:
			_pDesc->NumProcessorPackages++;
			break;

		default:
			break;
		}
	}

	if (_pDesc->Type == oCPU_X86 || _pDesc->Type == oCPU_X64)
	{
		// http://msdn.microsoft.com/en-us/library/hskdteyh(VS.80).aspx

		CPUGetString(_pDesc->String);
		CPUGetBrandString(_pDesc->BrandString);

		int CPUInfo[4];
		__cpuid(CPUInfo, 1);
		_pDesc->HasX87FPU = !!(CPUInfo[3] & (1<<0));
		_pDesc->Has8ByteAtomicSwap = !!(CPUInfo[3] & (1<<8));
		_pDesc->HasHyperThreading = !!(CPUInfo[3] & (1<<28));

		if (CPUInfo[2] & (1<<0))
			_pDesc->SSEVersion = 3;
		else if (CPUInfo[3] & (1<<26))
			_pDesc->SSEVersion = 2;
		else if (CPUInfo[3] & (1<<25))
			_pDesc->SSEVersion = 1;
		else
			_pDesc->SSEVersion = 0;
	}

	return true;
}
