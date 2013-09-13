/**************************************************************************
 * The MIT License                                                        *
 * Copyright (c) 2013 Antony Arciuolo.                                    *
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
#include <oPlatform/Windows/oWindows.h>
#include <oStd/assert.h>
#include <oHLSL/oHLSLBit.h>
#include <oPlatform/oCPU.h>

oRTTI_ENUM_BEGIN_DESCRIPTION(oRTTI_CAPS_ARRAY, oCPU_TYPE)
	oRTTI_ENUM_BEGIN_VALUES(oCPU_TYPE)
		oRTTI_VALUE_CUSTOM(oCPU_UNKNOWN, "unknown")
		oRTTI_VALUE_CUSTOM(oCPU_X86, "x86")
		oRTTI_VALUE_CUSTOM(oCPU_X64, "x64")
		oRTTI_VALUE_CUSTOM(oCPU_IA64, "ia64")
		oRTTI_VALUE_CUSTOM(oCPU_ARM, "ARM")
	oRTTI_ENUM_END_VALUES(oCPU_TYPE)
oRTTI_ENUM_END_DESCRIPTION(oCPU_TYPE)

oRTTI_ENUM_BEGIN_DESCRIPTION(oRTTI_CAPS_ARRAY, oCPU_SUPPORT_TYPE)
	oRTTI_ENUM_BEGIN_VALUES(oCPU_SUPPORT_TYPE)
		oRTTI_VALUE_CUSTOM(oCPU_NOT_FOUND, "not found")
		oRTTI_VALUE_CUSTOM(oCPU_NO_SUPPORT, "no support")
		oRTTI_VALUE_CUSTOM(oCPU_HW_SUPPORT_ONLY, "HW support only")
		oRTTI_VALUE_CUSTOM(oCPU_FULL_SUPPORT, "full support")
	oRTTI_ENUM_END_VALUES(oCPU_SUPPORT_TYPE)
oRTTI_ENUM_END_DESCRIPTION(oCPU_SUPPORT_TYPE)

struct CPU_FEATURE
{
	const char* Name;
	int CPUInfoIndex;
	int CheckBit;
	oWINDOWS_VERSION MinVersion;
};

static const CPU_FEATURE sFeatureStrings[] = 
{
	{ "X87FPU", 3, 0, oWINDOWS_UNKNOWN },
	{ "Hyperthreading", 3, 28, oWINDOWS_UNKNOWN },
	{ "8ByteAtomicSwap", 3, 8, oWINDOWS_UNKNOWN },
	{ "SSE1", 2, 25, oWINDOWS_UNKNOWN },
	{ "SSE2", 3, 26, oWINDOWS_UNKNOWN },
	{ "SSE3", 2, 0, oWINDOWS_UNKNOWN },
	{ "SSE4.1", 2, 19,  oWINDOWS_UNKNOWN },
	{ "SSE4.2", 2, 20, oWINDOWS_UNKNOWN },
	{ "XSAVE/XSTOR", 2, 26, oWINDOWS_7_SP1 },
	{ "OSXSAVE", 2, 27, oWINDOWS_7_SP1 },
	{ "AVX1", oInvalid, 0, oWINDOWS_7_SP1 }, // AVX detection is a special-case
};

static char* CPUGetString(char* _StrDestination, size_t _NumberOfElements)
{
	int CPUInfo[4];
	if (_NumberOfElements <= sizeof(CPUInfo))
		return nullptr;
	__cpuid(CPUInfo, 0);
	memset(_StrDestination, 0, _NumberOfElements);
	*((int*)_StrDestination) = CPUInfo[1];
	*((int*)(_StrDestination+4)) = CPUInfo[3];
	*((int*)(_StrDestination+8)) = CPUInfo[2];
	return _StrDestination;
}

template<size_t size> static inline char* CPUGetString(char (&_StrDestination)[size]) { return CPUGetString(_StrDestination, size); }

static char* CPUGetBrandString(char* _StrDestination, size_t _SizeofStrDestination)
{
	int CPUInfo[4];
	if (_SizeofStrDestination <= sizeof(CPUInfo))
		return nullptr;
	memset(_StrDestination, 0, _SizeofStrDestination);
	__cpuid(CPUInfo, 0x80000002);
	memcpy(_StrDestination, CPUInfo, sizeof(CPUInfo));
	__cpuid(CPUInfo, 0x80000003);
	memcpy(_StrDestination + 16, CPUInfo, sizeof(CPUInfo));
	__cpuid(CPUInfo, 0x80000004);
	memcpy(_StrDestination + 32, CPUInfo, sizeof(CPUInfo));
	return oStd::clean_whitespace(_StrDestination, _SizeofStrDestination, _StrDestination);
}

template<size_t size> static inline char* CPUGetBrandString(char (&_StrDestination)[size]) { return CPUGetBrandString(_StrDestination, size); }

bool oCPUGetDesc(oCPU_DESC* _pDesc)
{
	DWORD size_ex = 0;
	GetLogicalProcessorInformationEx(RelationNumaNode, 0, &size_ex);
	oASSERT(GetLastError() == ERROR_INSUFFICIENT_BUFFER, "");
	SYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX* lpi_ex = static_cast<SYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX*>(_alloca(size_ex));
	memset(lpi_ex, 0xff, size_ex);
	if (TRUE != GetLogicalProcessorInformationEx(RelationNumaNode, lpi_ex, &size_ex))
		return false;

	// Skip to the requested CPU index

	static const int _CPUIndex = 0;
	size_t offset_ex = 0;
	for (int cpu = 0; cpu < _CPUIndex; cpu++)
	{
		offset_ex += lpi_ex->Size;
		if (offset_ex >= size_ex)
			return false;
		lpi_ex = oStd::byte_add(lpi_ex, lpi_ex->Size);
	}

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
		if ((lpi[i].ProcessorMask & lpi_ex->NumaNode.GroupMask.Mask) != 0)
		{
			switch (lpi[i].Relationship)
			{
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
				break;

				case RelationProcessorPackage:
				_pDesc->NumProcessorPackages++;
				break;

				default:
				break;
			}
		}
	}

	if (_pDesc->Type == oCPU_X86 || _pDesc->Type == oCPU_X64)
	{
		// http://msdn.microsoft.com/en-us/library/hskdteyh(VS.80).aspx

		CPUGetString(_pDesc->String);
		CPUGetBrandString(_pDesc->BrandString);
	}

	return true;
}

const char* oCPUEnumFeatures(int _FeatureIndex)
{
	return (_FeatureIndex < 0 || _FeatureIndex >= oCOUNTOF(sFeatureStrings)) ? nullptr : sFeatureStrings[_FeatureIndex].Name;
}

oCPU_SUPPORT_TYPE oCPUCheckFeatureSupport(const char* _Feature)
{
	int CPUInfo[4];
	__cpuid(CPUInfo, 1);

	int i = 0;
	const char* s = oCPUEnumFeatures(i);
	while (s)
	{
		if (!oStrcmp(_Feature, s))
		{
			const CPU_FEATURE& f = sFeatureStrings[i];
			oWINDOWS_VERSION WinVer = oGetWindowsVersion();

			if (f.CPUInfoIndex == oInvalid)
			{
				// special cases
				if (!oStrcmp(_Feature, "AVX1"))
				{
					// http://insufficientlycomplicated.wordpress.com/2011/11/07/detecting-intel-advanced-vector-extensions-avx-in-visual-studio/
					bool HasXSAVE_XSTOR = CPUInfo[2] & (1<<27) || false;
					bool HasAVX = CPUInfo[2] & (1<<28) || false;
					
					if (WinVer >= f.MinVersion)
					{
						if (HasXSAVE_XSTOR && HasAVX)
						{
							// Check if the OS will save the YMM registers
							unsigned long long xcrFeatureMask = _xgetbv(_XCR_XFEATURE_ENABLED_MASK);
							if ((xcrFeatureMask & 0x6) || false)
								return oCPU_FULL_SUPPORT;
							else
							{
								oErrorSetLast(std::errc::not_supported, "Feature \"%s\" not supported by the current platform", oSAFESTRN(_Feature));
								return oCPU_HW_SUPPORT_ONLY;
							}
						}

						else
						{
							oErrorSetLast(std::errc::not_supported, "Feature \"%s\" not supported by the current platform", oSAFESTRN(_Feature));
							return oCPU_HW_SUPPORT_ONLY;
						}
					}

					else
					{
						oErrorSetLast(std::errc::not_supported, "Feature \"%s\" not supported by the current platform", oSAFESTRN(_Feature));
						return oCPU_HW_SUPPORT_ONLY;
					}
				}

				break;
			}

			else
			{
				if ((CPUInfo[f.CPUInfoIndex] & (1<<f.CheckBit)) == 0)
				{
					char buf[32];
					oErrorSetLast(std::errc::not_supported, "Feature \"%s\" not supported for the current HW: %s", oSAFESTRN(_Feature), CPUGetString(buf));
					return oCPU_NO_SUPPORT;
				}

				if (WinVer < f.MinVersion)
				{
					oErrorSetLast(std::errc::not_supported, "Feature \"%s\" not supported by the current platform", oSAFESTRN(_Feature));
					return oCPU_HW_SUPPORT_ONLY;
				}

				return oCPU_FULL_SUPPORT;
			}
		}

		s = oCPUEnumFeatures(++i);
	}

	oErrorSetLast(std::errc::not_supported, "Feature \"%s\" not supported for checking", oSAFESTRN(_Feature));
	return oCPU_NOT_FOUND;
}
