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
// Implementations in this source file use oFile API directly, so are not truly
// platform-specific once basic oFile APIs are implemented.
#include <oPlatform/oFile.h>
#include <oPlatform/oSystem.h> // oSystemGetPath
#include <oBasis/oError.h>
#include "oFileInternal.h"

bool oFileIsText(oHFILE _hFile)
{
	// http://code.activestate.com/recipes/173220-test-if-a-file-or-string-is-text-or-binary/
	// "The difference between text and binary is ill-defined, so this duplicates 
	// "the definition used by Perl's -T flag, which is: <br/> The first block 
	// "or so of the file is examined for odd characters such as strange control
	// "codes or characters with the high bit set. If too many strange characters 
	// (>30%) are found, it's a -B file, otherwise it's a -T file. Also, any file 
	// containing null in the first block is considered a binary file."

	static const size_t BLOCK_SIZE = 512;
	static const float PERCENTAGE_THRESHOLD_TO_BE_BINARY = 0.10f; // 0.30f; // @oooii-tony: 30% seems too high to me.

	char buf[BLOCK_SIZE];

	// Read the block and restore any prior position
	unsigned long long origin = oFileTell(_hFile);
	if (origin && !oFileSeek(_hFile, 0, oSEEK_SET))
		return false; // propagate oFileSeek error

	unsigned long long actualSize = oFileRead(_hFile, buf, BLOCK_SIZE, BLOCK_SIZE);
	if (origin && !oFileSeek(_hFile, origin, oSEEK_SET))
		return false; // propagate oFileSeek error

	// Count non-text characters
	size_t nonTextCount = 0;
	for (size_t i = 0; i < actualSize; i++)
		if (buf[i] == 0 || (buf[i] & 0x80))
			nonTextCount++;

	// Determine results
	float percentNonAscii = nonTextCount / static_cast<float>(actualSize);
	return percentNonAscii < PERCENTAGE_THRESHOLD_TO_BE_BINARY;
}

bool oFileIsText(const char* _Path)
{
	oHFILE hFile = nullptr;
	if (!oFileOpen(_Path, oFILE_OPEN_BIN_READ, &hFile))
		return false; // propagate oFileOpen error
	bool result = oFileIsText(hFile);
	oFileClose(hFile);
	return result;
}

bool oFileTouch(const char* _Path, time_t _PosixTimestamp)
{
	oHFILE hFile = nullptr;
	if (!oFileOpen(_Path, oFILE_OPEN_BIN_APPEND, &hFile))
		return false; // propagate oFileOpen error
	bool result = oFileTouch(hFile, _PosixTimestamp);
	oFileClose(hFile);
	return result;
}

char* oFileCreateTempFolder(char* _TempPath, size_t _SizeofTempPath)
{
	bool result = false;
	oSystemGetPath(_TempPath, _SizeofTempPath, oSYSPATH_TMP);
	size_t pathLength = strlen(_TempPath);
	while (true)
	{
		sprintf_s(_TempPath + pathLength, _SizeofTempPath - pathLength, "%i", rand());
		bool result = oFileCreateFolder(_TempPath);
		if (result || oErrorGetLast() != oERROR_REDUNDANT)
			break;
	}

	return result ? _TempPath : nullptr;
}

static bool oFileLoad_OpenAndSize(oHFILE* _phFile, size_t* _pSize, const char* _Path)
{
	if (!oFileOpen(_Path, oFILE_OPEN_BIN_READ, _phFile))
		return false; // propagate oFileOpen error

	*_pSize = detail::oSizeT<size_t>(oFileGetSize(*_phFile));
	return true;
}

static bool oFileLoad_ReadAndClose(void* _pOutBuffer, size_t _SizeofOutBuffer, size_t* _pOutSize, size_t _ReadSize, oHFILE _hFile, const char* _Path, bool _AsString)
{
	oOnScopeExit closeFile([&] { oVERIFY(oFileClose(_hFile)); }); // right now we're passing through the eror from read, so if close is going to overwrite that error, let's take a look first with the oVERIFY

	if (_ReadSize > _SizeofOutBuffer) // ReadSize has +1 for nul terminator in it
	{
		oStringS fileSize, outSize;
		return oErrorSetLast(oERROR_AT_CAPACITY, "Size of out buffer (%s) is not large enough to hold the %s of file %s", oFormatMemorySize(outSize, _SizeofOutBuffer, 2), oFormatMemorySize(fileSize, _ReadSize, 2), _Path);
	}

	unsigned long long actualSize = oFileRead(_hFile, _pOutBuffer, _SizeofOutBuffer, _ReadSize);
	if (!actualSize)
		return false;

	if (_AsString)
		((char*)_pOutBuffer)[actualSize] = 0;
	if (_pOutSize)
		*_pOutSize = detail::oSizeT<size_t>(actualSize + (_AsString ? 1 : 0));

	return true;
}

bool oFileLoad(void** _ppOutBuffer, size_t* _pOutSize, oFUNCTION<void*(size_t _Size)> _Allocate, const char* _Path, bool _AsString)
{
	if (!_ppOutBuffer || !_Allocate || !oSTRVALID(_Path))
		return oErrorSetLast(oERROR_INVALID_PARAMETER);

	oHFILE hFile = nullptr;
	size_t size = 0;
	if (!oFileLoad_OpenAndSize(&hFile, &size, _Path))
		return false; // pass through error

	*_ppOutBuffer = _Allocate(size + (_AsString ? 1 : 0));
	if (!*_ppOutBuffer)
	{
		oStringS fileSize;
		oFileClose(hFile);
		return oErrorSetLast(oERROR_AT_CAPACITY, "Out of memory allocating %s", oFormatMemorySize(fileSize, size, 2));
	}

	if (!oFileLoad_ReadAndClose(*_ppOutBuffer, size, _pOutSize, size, hFile, _Path, _AsString))
		return false; // pass through error

	return true;
}

bool oFileLoad(void* _pOutBuffer, size_t _SizeofOutBuffer, size_t* _pOutSize, const char* _Path, bool _AsString)
{
	if (!_pOutBuffer || !oSTRVALID(_Path))
		return oErrorSetLast(oERROR_INVALID_PARAMETER);

	oHFILE hFile = nullptr;
	size_t size = 0;
	if (!oFileLoad_OpenAndSize(&hFile, &size, _Path))
		return false; // pass through error

	if (!oFileLoad_ReadAndClose(_pOutBuffer, _SizeofOutBuffer, _pOutSize, size, hFile, _Path, _AsString))
		return false; // pass through error

	return true;
}

bool oFileLoadHeader(void* _pHeader, size_t _SizeofHeader, const char* _Path)
{
	const bool _AsText = false;

	if (!_pHeader || !_SizeofHeader || !oSTRVALID(_Path))
		return oErrorSetLast(oERROR_INVALID_PARAMETER);
	
	oHFILE hFile = nullptr;
	if (!oFileOpen(_Path, _AsText ? oFILE_OPEN_TEXT_READ : oFILE_OPEN_BIN_READ, &hFile))
		return false; // propagate oFileOpen error

	unsigned long long actualSize = oFileRead(hFile, _pHeader, _SizeofHeader, _SizeofHeader);
	if (_AsText)
		((char*)_pHeader)[actualSize] = 0;

	oFileClose(hFile);

	if (actualSize != static_cast<unsigned long long>(_SizeofHeader))
	{
		char header[64];
		char actual[64];
		return oErrorSetLast(oERROR_IO, "Expected %s, but read %s as header from file %s", oFormatMemorySize(header, _SizeofHeader, 2), oFormatMemorySize(actual, actualSize, 2), _Path);
	}

	return true;
}

bool oFileSave(const char* _Path, const void* _pSource, size_t _SizeofSource, bool _AsText, bool _AppendToExistingFile)
{
	if (!oSTRVALID(_Path) || !_pSource)
		return oErrorSetLast(oERROR_INVALID_PARAMETER);

	oHFILE hFile = nullptr;

	int Open = _AsText ? oFILE_OPEN_TEXT_WRITE : oFILE_OPEN_BIN_WRITE;
	if (_AppendToExistingFile)
		Open++;

	if (!oFileOpen(_Path, static_cast<oFILE_OPEN>(Open), &hFile))
		return false; // propagate oFileOpen error

	unsigned long long actualWritten = oFileWrite(hFile, _pSource, _SizeofSource);
	oFileClose(hFile);

	if (actualWritten != _SizeofSource)
	{
		char source[64];
		char actual[64];
		return oErrorSetLast(oERROR_IO, "Expected to write %s, but wrote %s to file %s", oFormatMemorySize(source, _SizeofSource, 2), oFormatMemorySize(actual, actualWritten, 2), _Path);
	}

	return true;
}

#include <oBasis/oBuffer.h>
bool oBufferCreate(const char* _Path, oBuffer** _ppBuffer, bool _AsString)
{
	void* b = nullptr;
	size_t size = 0;
	bool success = oFileLoad(&b, &size, malloc, _Path, _AsString);
	return success ? oBufferCreate(_Path, b, size, free, _ppBuffer) : success;
}