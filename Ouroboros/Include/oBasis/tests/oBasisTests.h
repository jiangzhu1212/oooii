// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.
// Declarations of unit tests of components found in oBasis
#pragma once
#ifndef oBasisTests_h
#define oBasisTests_h

#include <oBase/path.h>
#include <oBasis/oPlatformFeatures.h>
#include <oSurface/surface.h>

struct oBasisTestServices
{
	std::function<bool(ouro::path& _AbsolutePath, const ouro::path& _RelativePath, bool _PathMustExist)> ResolvePath;
	std::function<bool(void** _ppBuffer, size_t* _pSize, const char* _FullPath, bool _AsText)> AllocateAndLoadBuffer;
	std::function<void(void* _pBuffer)> DeallocateLoadedBuffer;
	std::function<int()> Rand;
	std::function<size_t()> GetTotalPhysicalMemory;
};

// oBasisTests follows a pattern: all functions return true if successful, false
// if not successful. In either case oErrorSetLast is used to set a string as
// to what occurred. In success the last error is set to 0.

oAPI bool oBasisTest_oBuffer(const oBasisTestServices& _Services);

oAPI bool oBasisTest_oINISerialize();
oAPI bool oBasisTest_oJSONSerialize();
oAPI bool oBasisTest_oMath();
oAPI bool oBasisTest_oRTTI();
oAPI bool oBasisTest_oString();
oAPI bool oBasisTest_oURI();
oAPI bool oBasisTest_oURIQuerySerialize();
oAPI bool oBasisTest_oXML();
oAPI bool oBasisTest_oXMLSerialize();

#endif
