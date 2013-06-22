/**************************************************************************
 * The MIT License                                                        *
 * Copyright (c) 2013 OOOii.                                              *
 * antony.arciuolo@oooii.com                                              *
 * kevin.myers@oooii.com                                                  *
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

bool oFileTouch(const char* _Path, time_t _UnixTimestamp)
{
	oHFILE hFile = nullptr;
	if (!oFileOpen(_Path, oFILE_OPEN_BIN_APPEND, &hFile))
		return false; // propagate oFileOpen error
	bool result = oFileTouch(hFile, _UnixTimestamp);
	oFileClose(hFile);
	return result;
}

bool oFileEnsureParentFolderExists(const char* _Path)
{
	oStd::path_string parent(_Path);
	oTrimFilename(parent);
	if (!parent.empty() && !oFileCreateFolder(parent) && oErrorGetLast() != std::errc::operation_in_progress)
		return false; // pass through error
	return true;
}

char* oFileCreateTempFolder(char* _TempPath, size_t _SizeofTempPath)
{
	bool result = false;
	oSystemGetPath(_TempPath, _SizeofTempPath, oSYSPATH_SYSTMP);
	size_t pathLength = oStrlen(_TempPath);
	while (true)
	{
		oPrintf(_TempPath + pathLength, _SizeofTempPath - pathLength, "%i", rand());
		bool result = oFileCreateFolder(_TempPath);
		if (result || oErrorGetLast() != std::errc::operation_in_progress)
			break;
	}

	return result ? _TempPath : nullptr;
}

bool oFileLoadHeader(void* _pHeader, size_t _SizeofHeader, const char* _Path)
{
	const bool _AsText = false;

	if (!_pHeader || !_SizeofHeader || !oSTRVALID(_Path))
		return oErrorSetLast(std::errc::invalid_argument);
	
	oHFILE hFile = nullptr;
	if (!oFileOpen(_Path, _AsText ? oFILE_OPEN_TEXT_READ : oFILE_OPEN_BIN_READ, &hFile))
		return false; // propagate oFileOpen error

	unsigned long long actualSize = oFileRead(hFile, _pHeader, _SizeofHeader, _SizeofHeader);
	if (_AsText)
		((char*)_pHeader)[actualSize] = 0;

	oFileClose(hFile);

	if (actualSize != static_cast<unsigned long long>(_SizeofHeader))
	{
		oStd::sstring header, actual;
		return oErrorSetLast(std::errc::io_error, "Expected %s, but read %s as header from file %s", oFormatMemorySize(header, _SizeofHeader, 2), oFormatMemorySize(actual, actualSize, 2), _Path);
	}

	return true;
}

bool oFileSave(const char* _Path, const void* _pSource, size_t _SizeofSource, bool _AsText, bool _AppendToExistingFile)
{
	if (!oSTRVALID(_Path) || !_pSource)
		return oErrorSetLast(std::errc::invalid_argument);

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
		oStd::sstring source, actual;
		return oErrorSetLast(std::errc::io_error, "Expected to write %s, but wrote %s to file %s", oFormatMemorySize(source, _SizeofSource, 2), oFormatMemorySize(actual, actualWritten, 2), _Path);
	}

	return true;
}
