// $(header)
#include <oooii/oWindows.h>
#include <oooii/oCPU.h>
#include <oooii/oAssert.h>
#include <oooii/oMath.h>
#include <oooii/oString.h>

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

bool oCPU::GetDesc(unsigned int _CPUIndex, DESC* _pDesc)
{
	if (_CPUIndex > 0) // @oooii-tony: More than 1 CPU not yet supported (buy me a dual core machine for testing!)
		return false;

	memset(_pDesc, 0, sizeof(DESC));

	SYSTEM_INFO si;
	GetSystemInfo(&si);
	switch (si.wProcessorArchitecture)
	{
		case PROCESSOR_ARCHITECTURE_AMD64: _pDesc->Type = X64; break;
		case PROCESSOR_ARCHITECTURE_IA64: _pDesc->Type = IA64; break;
		case PROCESSOR_ARCHITECTURE_INTEL: _pDesc->Type = X86; break;
		default: _pDesc->Type = UNKNOWN; break;
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
			CACHE_DESC& c = lpi[i].Cache.Type == CacheData ? _pDesc->DataCacheDescs[cacheLevel] : _pDesc->InstructionCacheDescs[cacheLevel];
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

	if (_pDesc->Type == X86 || _pDesc->Type == X64)
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
