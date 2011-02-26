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
#include "pch.h"
#include <oooii/oStdio.h>
#include <oooii/oAssert.h>
#include <oooii/oByte.h>
#include <oooii/oErrno.h>
#include <oooii/oPath.h>
#include <oooii/oStddef.h>
#include <oooii/oSwizzle.h>
#include <oooii/oWindows.h>
#include <io.h>
#include <regex>
using namespace std::tr1;

double oTimer()
{
	LARGE_INTEGER ticks, freq;
	QueryPerformanceFrequency(&freq);
	QueryPerformanceCounter(&ticks);
	return ticks.QuadPart / static_cast<double>(freq.QuadPart);
}

oScopedPartialTimeout::oScopedPartialTimeout(unsigned int* _pTimeoutMSCountdown)
	: pTimeoutMSCountdown(_pTimeoutMSCountdown)
	, Start(oTimer())
{
}

oScopedPartialTimeout::~oScopedPartialTimeout()
{
	UpdateTimeout();
}

void oScopedPartialTimeout::UpdateTimeout()
{
	if (*pTimeoutMSCountdown != oINFINITE_WAIT)
	{
		double CurrentTime = oTimer();
		unsigned int diff = oTimerGetDiffMS(Start, CurrentTime);
		unsigned int OldCountdown = *pTimeoutMSCountdown;
		*pTimeoutMSCountdown = OldCountdown < diff ? 0 :  OldCountdown - diff;
		Start = CurrentTime;
	}
}

errno_t oGetHostname(char* _Hostname, size_t _SizeofHostname)
{
	DWORD dwSize = static_cast<DWORD>(_SizeofHostname);
	if (dwSize != _SizeofHostname) return EINVAL;
	return GetComputerNameEx(ComputerNameDnsHostname, _Hostname, &dwSize) ? 0 : EINVAL;
}

void oMemset4(void* _pDestination, long _Value, size_t _NumBytes)
{
	// Sets an int value at a time. This is probably slower than c's memset, but 
	// this sets a full int value rather than a char value.

	// First move _pDestination up to long alignment

	char* pPrefix = (char*)_pDestination;
	long* p = (long*)oByteAlign(_pDestination, sizeof(long));
	size_t nPrefixBytes = oByteDiff(pPrefix, _pDestination);
	long* pEnd = oByteAdd(p, _NumBytes - nPrefixBytes);
	char* pPostfix = (char*)oByteAlignDown(pEnd, sizeof(long));
	size_t nPostfixBytes = oByteDiff(pEnd, pPostfix);

	oASSERT(oByteAdd(_pDestination, _NumBytes) == pEnd, "");
	oASSERT(oByteAdd(_pDestination, _NumBytes) == oByteAdd(pPostfix, nPostfixBytes), "");

	oByteSwizzle32 s;
	s.AsInt = _Value;

	// Duff's device up to alignment
	switch (nPrefixBytes)
	{
		case 3: *pPrefix = s.AsChar[3];
		case 2: *pPrefix = s.AsChar[2];
		case 1: *pPrefix = s.AsChar[1];
		case 0: break;
		default: oASSUME(0);
	}

	// Do aligned assignment
	while (p < (long*)pPostfix)
		*p++ = _Value;

	// Duff's device any remaining bytes
	switch (nPostfixBytes)
	{
		case 3: *pPrefix++ = s.AsChar[3];
		case 2: *pPrefix++ = s.AsChar[2];
		case 1: *pPrefix++ = s.AsChar[1];
		case 0: break;
		default: oASSUME(0);
	}
}

void oMemcpy2d(void* oRESTRICT _pDestination, size_t _DestinationPitch, const void* oRESTRICT _pSource, size_t _SourcePitch, size_t _SourceRowSize, size_t _NumRows)
{
	const void* end = oByteAdd(_pDestination, _DestinationPitch, _NumRows);
	for (; _pDestination < end; _pDestination = oByteAdd(_pDestination, _DestinationPitch), _pSource = oByteAdd(_pSource, _SourcePitch))
		memcpy(_pDestination, _pSource, _SourceRowSize);
}

void oMemset2d(void* _pDestination, size_t _Pitch, int _Value, size_t _SetPitch, size_t _NumRows)
{
	const void* end = oByteAdd(_pDestination, _Pitch, _NumRows);
	for (; _pDestination < end; _pDestination = oByteAdd(_pDestination, _Pitch))
		memset(_pDestination, _Value, _SetPitch);
}

void oMemset2d4(void* _pDestination, size_t _Pitch, long _Value, size_t _SetPitch, size_t _NumRows)
{
	oASSERT((_SetPitch % sizeof(long)) == 0, "");
	const void* end = oByteAdd(_pDestination, _Pitch, _NumRows);
	for (; _pDestination < end; _pDestination = oByteAdd(_pDestination, _Pitch))
		oMemset4(_pDestination, _Value, _SetPitch);
}

void oMemcpyAsym(void* oRESTRICT _pDestination, size_t _DestinationStride, const void* oRESTRICT _pSource, size_t _SourceStride, size_t _NumElements)
{
	const void* end = oByteAdd(_pDestination, _DestinationStride, _NumElements);
	for (; _pDestination < end; _pDestination = oByteAdd(_pDestination, _DestinationStride), _pSource = oByteAdd(_pSource, _SourceStride))
		memcpy(_pDestination, _pSource, _SourceStride);
}

void oMemcpyToUshort(unsigned short* _pDestination, const unsigned int* _pSource, size_t _NumElements)
{
	const unsigned int* end = &_pSource[_NumElements];
	while (_pSource < end)
	{
		oASSERT(*_pSource <= 65535, "Truncating an unsigned int (%d) to a short in a way that will change its value.", *_pSource);
		*_pDestination++ = (*_pSource++) & 0xff;
	}
}

void oMemcpyToUint(unsigned int* _pDestination, const unsigned short* _pSource, size_t _NumElements)
{
	const unsigned short* end = &_pSource[_NumElements];
	while (_pSource < end)
		*_pDestination++ = *_pSource++;
}

namespace oFile {
namespace detail {

static void convert(DESC* _pDesc, const WIN32_FIND_DATA* _pData, const char* _Wildcard)
{
	_pDesc->Created = oFileTimeToUnixTime(&_pData->ftCreationTime);
	_pDesc->Accessed = oFileTimeToUnixTime(&_pData->ftLastAccessTime);
	_pDesc->Written = oFileTimeToUnixTime(&_pData->ftLastWriteTime);
	_pDesc->Size = (_pData->nFileSizeHigh * (static_cast<unsigned long long>(MAXDWORD) + 1)) + _pData->nFileSizeLow;
	_pDesc->Directory = !!(_pData->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY);
	_pDesc->Archive = !!(_pData->dwFileAttributes & FILE_ATTRIBUTE_ARCHIVE);
	_pDesc->Compressed = !!(_pData->dwFileAttributes & FILE_ATTRIBUTE_COMPRESSED);
	_pDesc->Encrypted = !!(_pData->dwFileAttributes & FILE_ATTRIBUTE_ENCRYPTED);
	_pDesc->Hidden = !!(_pData->dwFileAttributes & FILE_ATTRIBUTE_HIDDEN);
	_pDesc->ReadOnly = !!(_pData->dwFileAttributes & FILE_ATTRIBUTE_READONLY);
	_pDesc->System = !!(_pData->dwFileAttributes & FILE_ATTRIBUTE_SYSTEM);
	_pDesc->Offline = !!(_pData->dwFileAttributes & FILE_ATTRIBUTE_OFFLINE);

#ifdef UNICODE
	WCHAR buf[_MAX_PATH];
	oVB(GetFullPathName(_pData->cFileName, oCOUNTOF(buf), buf, 0));
	oStrConvert(_pDesc->Filename, buf);
#else
	oVB(GetFullPathName(_pData->cFileName, oCOUNTOF(_pDesc->Filename), _pDesc->Filename, 0));
#endif
}

struct FIND_CONTEXT
{
	HANDLE hContext;
	char Wildcard[_MAX_PATH];
};

} // namespace detail

bool FindFirst(DESC* _pDesc, const char* _Wildcard, void** _pFindContext)
{
	if (!_pDesc || !_Wildcard || !*_Wildcard || !_pFindContext) return false;
	WIN32_FIND_DATA fd;
	*_pFindContext = 0;
	HANDLE h = FindFirstFile(_Wildcard, &fd);
	if (h == INVALID_HANDLE_VALUE)
		return false;
	detail::FIND_CONTEXT* ctx = new detail::FIND_CONTEXT;
	ctx->hContext = h;
	strcpy_s(ctx->Wildcard, _Wildcard);
	*_pFindContext = ctx;
	detail::convert(_pDesc, &fd, _Wildcard);
	return true;
}

bool FindNext(DESC* _pDesc, void* _pFindContext)
{
	detail::FIND_CONTEXT* ctx = static_cast<detail::FIND_CONTEXT*>(_pFindContext);
	if (!_pDesc || !ctx || ctx->hContext == INVALID_HANDLE_VALUE) return false;
	WIN32_FIND_DATA fd;
	if (!FindNextFile(ctx->hContext, &fd))
		return false;
	detail::convert(_pDesc, &fd, ctx->Wildcard);
	return true;
}

bool CloseFind(void* _pFindContext)
{
	if (_pFindContext)
	{
		detail::FIND_CONTEXT* ctx = static_cast<detail::FIND_CONTEXT*>(_pFindContext);
		HANDLE hContext = ctx->hContext;
		delete _pFindContext;
		if (hContext != INVALID_HANDLE_VALUE && !FindClose(hContext))
			return false;
	}
	return true;
}

size_t GetSize(FILE* _File)
{
	long origin = ftell(_File);
	if (0 != fseek(_File, 0, SEEK_END))
		return 0;
	long size = ftell(_File);
	fseek(_File, origin, SEEK_SET);
	return size;
}

bool Touch(const char* _Path, time_t _PosixTimestamp)
{
	bool result = false;
	FILE* f = 0;
	if (0 == fopen_s(&f, _Path, "ab"))
	{
		result = Touch(f, _PosixTimestamp);
		fclose(f);
	}

	else
	{
		errno_t err = 0;
		_get_errno(&err);
		oSetLastError(err, "Failed to open %s", _Path);
	}

	return result;
}

bool Touch(FILE* _File, time_t _PosixTimestamp)
{
	HANDLE hFile = (HANDLE)_get_osfhandle(_fileno(_File));
	if (hFile == INVALID_HANDLE_VALUE)
	{
		oSetLastError(ENOENT, "File handle incorrect");
		return false;
	}
	FILETIME time;
	oUnixTimeToFileTime(_PosixTimestamp, &time);
	if (!SetFileTime(hFile, 0, 0, &time))
	{
		char err[1024];
		oGetNativeErrorDesc(err, ::GetLastError());
		oSetLastError(EINVAL, err);
		return false;
	}
	
	return true;
}

bool MarkReadOnly(const char* _Path, bool _ReadOnly)
{
	#ifdef UNICODE
		WCHAR PATH[1024];
		oStrConvert(PATH, _Path);
	#else
		const char* PATH = _Path;
	#endif

	DWORD attrib = GetFileAttributes(PATH);

	if (_ReadOnly)
		attrib |= FILE_ATTRIBUTE_READONLY;
	else
		attrib &=~FILE_ATTRIBUTE_READONLY;

	if (!SetFileAttributes(PATH, attrib))
	{
		char err[1024];
		oGetNativeErrorDesc(err, ::GetLastError());
		oSetLastError(EINVAL, err);
		return false;
	}

	return true;
}

bool MarkHidden(const char* _Path, bool _Hidden)
{
	#ifdef UNICODE
		WCHAR PATH[1024];
		oStrConvert(PATH, _Path);
	#else
		const char* PATH = _Path;
	#endif
	
	DWORD attrib = GetFileAttributes(PATH);

	if (_Hidden)
		attrib |= FILE_ATTRIBUTE_HIDDEN;
	else
		attrib &=~FILE_ATTRIBUTE_HIDDEN;

	if (!SetFileAttributes(PATH, attrib))
	{
		char err[1024];
		oGetNativeErrorDesc(err, ::GetLastError());
		oSetLastError(EINVAL, err);
		return false;
	}

	return true;
}

bool Delete(const char* _Path)
{
	#ifdef UNICODE
		WCHAR PATH[1024];
		oStrConvert(PATH, _Path);
	#else
		const char* PATH = _Path;
	#endif
	return !!DeleteFile(PATH);
}

bool CreateFolder(const char* _Path)
{
	if (Exists(_Path))
	{
		oSetLastError(EEXIST, "Path %s already exists", _Path);
		return false;
	}

	#ifdef UNICODE
		WCHAR FN[_MAX_PATH];
		oStrConvert(FN, _Path);
	#else
		const char* FN = _Path;
	#endif
	return !!CreateDirectory(FN, 0);
}

bool Exists(const char* _Path)
{
	#ifdef UNICODE
		WCHAR FN[_MAX_PATH];
		oStrConvert(FN, _Path);
	#else
		const char* FN = _Path;
	#endif
	return INVALID_FILE_ATTRIBUTES != GetFileAttributes(FN);
}

bool IsText(FILE* _File)
{
	// http://code.activestate.com/recipes/173220-test-if-a-file-or-string-is-text-or-binary/
	// "The difference between text and binary is ill-defined, so this duplicates 
	// "the definition used by Perl's -T flag, which is: <br/> The first block 
	// "or so of the file is examined for odd characters such as strange control
	// "codes or characters with the high bit set. If too many strange characters 
	// (>30%) are found, it's a -B file, otherwise it's a -T file. Also, any file 
	// containing null in the first block is considered a binary file."

	static const size_t BLOCK_SIZE = 512;
	static const float PERCENTAGE_THRESHOLD_TO_BE_BINARY = 0.10f;// 0.30f; // @oooii-tony: 30% seems too high to me.

	char buf[BLOCK_SIZE];

	// Read the block and restore any prior position
	long origin = ftell(_File);
	if (origin && 0 != fseek(_File, 0, SEEK_SET))
	{
		int err = 0;
		_get_errno(&err);
		oSetLastError(err, "Failed file seek");
		return false;
	}

	size_t actualSize = fread(buf, 1, BLOCK_SIZE, _File);

	if (origin && 0 != fseek(_File, origin, SEEK_SET))
	{
		int err = 0;
		_get_errno(&err);
		oSetLastError(err, "Failed file seek");
		return false;
	}

	// Count non-text characters
	size_t nonTextCount = 0;
	for (size_t i = 0; i < actualSize; i++)
		if (buf[i] == 0 || (buf[i] & 0x80))
			nonTextCount++;

	// Determine results
	float percentNonAscii = nonTextCount / static_cast<float>(actualSize);
	return percentNonAscii < PERCENTAGE_THRESHOLD_TO_BE_BINARY;
}

bool IsText(const char* _Path)
{
	bool result = false;
	FILE* f = 0;
	if (0 == fopen_s(&f, _Path, "rb"))
	{
		result = IsText(f);
		fclose(f);
	}

	return result;
}

} // namespace oFile

bool oLoadBuffer(void** _ppOutBuffer, size_t* _pOutSize, oFUNCTION<void*(size_t _Size)> _Allocate, const char* _Path, bool _AsText)
{
	bool result = false;
	FILE* f = 0;
	if (_ppOutBuffer && _Allocate)
	{
		errno_t err = fopen_s(&f, _Path, _AsText ? "rt" : "rb");
		if (!err)
		{
			size_t size = oFile::GetSize(f) + (_AsText ? 1 : 0); // for nul terminator
			*_ppOutBuffer = _Allocate(size);
			size_t actualSize = fread(*_ppOutBuffer, 1, size, f);
			if (_AsText)
				((char*)(*_ppOutBuffer))[actualSize] = 0;
			if (_pOutSize)
				*_pOutSize = actualSize;
			result = true;
			fclose(f);
		}

		else
			oSetLastError(err, "Failed to open file %s", oSAFESTRN(_Path));
	}

	else
		oSetLastError(EINVAL, "Invalid parameter");

	return result;
}

bool oLoadBuffer(void* _pOutBuffer, size_t _SizeofOutBuffer, size_t* _pOutSize, const char* _Path, bool _AsText)
{
	bool result = false;
	FILE* f = 0;
	if (_pOutBuffer)
	{
		errno_t err = fopen_s(&f, _Path, _AsText ? "rt" : "rb");
		if (!err)
		{
			size_t size = oFile::GetSize(f) + (_AsText ? 1 : 0); // for nul terminator

			if (size > _SizeofOutBuffer)
			{
				oSetLastError(EINVAL, "Buffer too small");
				return false;
			}
			
			size_t actualSize = fread(_pOutBuffer, 1, size, f);
			if (_AsText)
				((char*)_pOutBuffer)[actualSize] = 0;
			if (_pOutSize)
				*_pOutSize = actualSize;
			result = true;
			fclose(f);
		}

		else
			oSetLastError(err, "Failed to open file %s", oSAFESTRN(_Path));
	}

	else
		oSetLastError(EINVAL, "Invalid parameter");

	return result;
}

bool oLoadFileHeader( void* _pHeader, size_t _SizeofHeader, const char* _Path, bool _AsText )
{
	FILE* f = 0;
	if (_pHeader)
	{
		errno_t err = fopen_s(&f, _Path, _AsText ? "rt" : "rb");
		if (!err)
		{
			size_t actualSize = fread(_pHeader, 1, _SizeofHeader, f);
			if (_AsText)
				((char*)_pHeader)[actualSize] = 0;

			fclose(f);

			return actualSize == _SizeofHeader;
		}
		else
			oSetLastError(err, "Failed to open file %s", oSAFESTRN(_Path));
	}
	else
		oSetLastError(EINVAL, "Invalid parameter");

	return false;
}


bool oSaveBuffer(const char* _Path, const void* _pSource, size_t _SizeofSource, bool _AsText)
{
	bool result = false;
	FILE* f;
	if (0 == fopen_s(&f, _Path, _AsText ? "wt" : "wb"))
	{
		size_t written = fwrite(_pSource, sizeof(char), _SizeofSource, f);
		result = (_SizeofSource == written);
		fclose(f);
	}

	return result;
}

bool oSetEnvironmentVariable(const char* _Name, const char* _Value)
{
	#ifdef UNICODE
		WCHAR NAME[1024];
		WCHAR VALUE[1024];
		oStrConvert(NAME, _Name);
		oStrConvert(VALUE, _Value);
	#else
		const char* NAME = _Name;
		const char* VALUE = _Value;
	#endif

	return !!SetEnvironmentVariable(NAME, VALUE);
}

bool oGetEnvironmentVariable(char* _Value, size_t _SizeofValue, const char* _Name)
{
	#ifdef UNICODE
		WCHAR NAME[1024];
		WCHAR* VALUE = _alloca(_SizeofValue * sizeof(wchar_t));
		oStrConvert(NAME, _Name);
	#else
		const char* NAME = _Name;
		char* VALUE = _Value;
	#endif

	oASSERT((size_t)(int)_SizeofValue == _SizeofValue, "Invalid size");

	size_t len = GetEnvironmentVariable(NAME, VALUE, (int)_SizeofValue);

	#ifdef UNICODE
		if (result)
			oStrConvert(_Value, _SizeofValue, VALUE);
	#endif
	return len && len < _SizeofValue;
}

errno_t oGetEnvironmentString(char* _StrEnvironment, size_t _SizeofStrEnvironment)
{
	char* pEnv = GetEnvironmentStringsA();

	// @oooii-tony: replace nuls with newlines to make parsing this mega-string
	// a bit less obtuse

	char* c = pEnv;
	size_t len = strlen(pEnv);
	while (len)
	{
		c[len] = '\n';
		c += len+1;
		len = strlen(c);
	}

	errno_t err = strcpy_s(_StrEnvironment, _SizeofStrEnvironment, pEnv);
	oVB(FreeEnvironmentStringsA(pEnv));

	return err;
}

int oGetNativeErrorDesc(char* _StrDestination, size_t _SizeofStrDestination, size_t _NativeError)
{
	int len = 0;
	*_StrDestination = 0;
	FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM, 0, static_cast<DWORD>(_NativeError), 0, _StrDestination, static_cast<DWORD>(_SizeofStrDestination), 0);
	if (!*_StrDestination || !memcmp(_StrDestination, "???", 3))
	{
		#if defined(_DX9) || defined(_DX10) || defined(_DX11)
			#ifdef UNICODE
				oStrConvert(_StrDestination, _SizeofStrDestination, DXGetErrorString(nativeError));
			#else
				strcpy_s(_StrDestination, _SizeofStrDestination, DXGetErrorString((HRESULT)nativeError));
			#endif
		#else
				len = sprintf_s(_StrDestination, _SizeofStrDestination, "unrecognized error code 0x%08x", _NativeError);
		#endif
	}

	return len != -1 ? 0 : STRUNCATE;
}

oHDLL oLinkDLL(const char* _LibName, const char** _pInterfaceFunctionNames, void** _ppInterfaces, size_t _CountofInterfaces)
{
	HMODULE hModule = 0;
	#ifdef _DEBUG
		const char* BUILDSUFFIX = "D";
	#else
		const char* BUILDSUFFIX = "";
	#endif

	char dllPath[_MAX_PATH];

	const char* ext = oGetFileExtension(_LibName);
	if (!strcmp(".dll", ext))
		strcpy_s(dllPath, _LibName);
	else
	{
		oGetSysPath(dllPath, oSYSPATH_APP);
		sprintf_s(dllPath, "%s%s%s.dll", dllPath, _LibName, BUILDSUFFIX);
	}
	
	hModule = LoadLibraryA(dllPath);

	memset(_ppInterfaces, 0, sizeof(void*) * _CountofInterfaces);

	bool allInterfacesAcquired = false;
	if (hModule)
	{
		allInterfacesAcquired = true;
		for (unsigned int i = 0; i < _CountofInterfaces; i++)
		{
			_ppInterfaces[i] = GetProcAddress(hModule, _pInterfaceFunctionNames[i]);
			if (!_ppInterfaces[i])
			{
				oTRACE("Can't find %s::%s", _LibName, _pInterfaceFunctionNames[i]);
				allInterfacesAcquired = false;
			}
		}
	}

	else
		oASSERT(false, "Could not load %s", dllPath);

	if (hModule && !allInterfacesAcquired)
	{
		oASSERT(false, "Could not create all interfaces from %s. This might be because the DLLs are out of date, try copying newer versions into the bin dir for this application.", dllPath);
		FreeLibrary(hModule);
		hModule = 0;
	}

	return (oHDLL)hModule;
}

void oUnlinkDLL(oHDLL _hDLL)
{
	if (_hDLL)
		FreeLibrary((HMODULE)_hDLL);
}