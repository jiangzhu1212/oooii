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
#include <oPlatform/oFile.h>
#include <oBasis/oSize.h>
#include <oBasis/oError.h>
#include <oPlatform/oWindows.h>
#include <cstdio>

struct FIND_CONTEXT
{
	HANDLE hContext;
	char Wildcard[_MAX_PATH];
};

template<typename WIN32_TYPE> void oFileConvert(oFILE_DESC* _pDesc, const WIN32_TYPE* _pData)
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
}

static bool oFileSetLastError()
{
	errno_t err = 0;
	_get_errno(&err);
	char strerr[256];
	strerror_s(strerr, err);
	return oErrorSetLast(oERROR_IO, "%s", strerr);
}

bool oFileExists(const char* _Path)
{
	return INVALID_FILE_ATTRIBUTES != GetFileAttributesA(_Path);
}

bool oFileOpen(const char* _Path, oFILE_OPEN _Open, oHFILE* _phFile)
{
	static char* opt = "rwa";

	char open[3];
	open[0] = opt[_Open % 3];
	open[1] = _Open >= oFILE_OPEN_TEXT_READ ? 't' : 'b';
	open[2] = 0;

	errno_t err = fopen_s((FILE**)_phFile, _Path, open);
	if (err)
	{
		char strerr[256];
		strerror_s(strerr, err);
		oErrorSetLast(oERROR_IO, "Failed to open %s (%s)", oSAFESTRN(_Path), strerr);
		return false;
	}

	return true;
}

bool oFileClose(oHFILE _hFile)
{
	if (!_hFile)
		return oErrorSetLast(oERROR_INVALID_PARAMETER);
	if (0 != fclose((FILE*)_hFile))
		return oErrorSetLast(oERROR_IO);
	return true;
}

unsigned long long oFileTell(oHFILE _hFile)
{
	unsigned long long offset = _ftelli64((FILE*)_hFile);
	if (offset == 1L)
		return oFileSetLastError();
	return offset;
}

bool oFileSeek(oHFILE _hFile, long long _Offset, oFILE_SEEK _Origin)
{ 
	if (_fseeki64((FILE*)_hFile, _Offset, (int)_Origin) == -1)
		return oFileSetLastError();
	return true; 
}

unsigned long long oFileRead(oHFILE _hFile, void* _pDestination, unsigned long long _SizeofDestination, unsigned long long _ReadSize)
{
	detail::oSizeT<size_t> CheckedReadSize(_ReadSize);
	detail::oSizeT<size_t> CheckedSizeofDestination(_SizeofDestination);
	size_t bytesRead = fread_s(_pDestination, CheckedSizeofDestination, 1, CheckedReadSize, (FILE*)_hFile);
	if (CheckedReadSize != bytesRead)
	{
		if (oFileAtEnd(_hFile))
			oErrorSetLast(oERROR_END_OF_FILE);
		else
			oFileSetLastError();
	}

	return bytesRead;
}

unsigned long long oFileWrite(oHFILE _hFile, const void* _pSource, unsigned long long _WriteSize, bool _Flush )
{
	detail::oSizeT<size_t> CheckedWriteSize(_WriteSize);
	size_t bytesWritten = fwrite(_pSource, 1, CheckedWriteSize, (FILE*)_hFile);
	if (CheckedWriteSize != bytesWritten)
		oFileSetLastError();
	if (_Flush)
		fflush((FILE*)_hFile);
	return bytesWritten;
}

unsigned long long oFileGetSize(oHFILE _hFile)
{
	HANDLE hFile = oGetFileHandle((FILE*)_hFile);
	LARGE_INTEGER fsize;
	if (!GetFileSizeEx(hFile, &fsize))
	{
		oWinSetLastError();
		return 0;
	}

	return fsize.QuadPart;
}

bool oFileAtEnd(oHFILE _hFile)
{
	return !!feof((FILE*)_hFile);
}

bool oFileGetDesc(const char* _Path, oFILE_DESC* _pDesc)
{
	WIN32_FILE_ATTRIBUTE_DATA FileData;
	if (0 == GetFileAttributesExA(_Path, GetFileExInfoStandard, &FileData))
		return oWinSetLastError();
	oFileConvert(_pDesc, &FileData);
	return true;
}

bool oFileEnum(const char* _WildcardPath, oFUNCTION<bool(const char* _FullPath, const oFILE_DESC& _Desc)> _EnumFunction)
{
	if (!_WildcardPath || !*_WildcardPath || !_EnumFunction)
		return oErrorSetLast(oERROR_INVALID_PARAMETER);

	WIN32_FIND_DATA fd;
	HANDLE hFind = FindFirstFile(_WildcardPath, &fd);
	if (hFind == INVALID_HANDLE_VALUE)
		return oErrorSetLast(oERROR_NOT_FOUND);

	oFILE_DESC desc;
	char ResolvedPath[_MAX_PATH];
	strcpy_s(ResolvedPath, _WildcardPath);

	do
	{
		oFileConvert(&desc, &fd);
		oTrimFilename(ResolvedPath);
		strcat_s(ResolvedPath, fd.cFileName);

		bool result = _EnumFunction(ResolvedPath, desc);
		if (!result)
			return true;

		if (!FindNextFile(hFind, &fd))
		{
			if (!FindClose(hFind))
				return oWinSetLastError();
			return true;
		}

	} while (1);

	oASSERT_NOEXECUTION;
}

bool oFileTouch(oHFILE _hFile, time_t _PosixTimestamp)
{
	HANDLE hFile = oGetFileHandle((FILE*)_hFile);
	if (hFile == INVALID_HANDLE_VALUE)
		return oErrorSetLast(oERROR_NOT_FOUND, "File handle incorrect");
	FILETIME time;
	oUnixTimeToFileTime(_PosixTimestamp, &time);
	if (!SetFileTime(hFile, 0, 0, &time))
		return oWinSetLastError();
	return true;
}

bool oFileMarkReadOnly(const char* _Path, bool _ReadOnly)
{
	DWORD attrib = GetFileAttributesA(_Path);
	if (_ReadOnly)
		attrib |= FILE_ATTRIBUTE_READONLY;
	else
		attrib &=~FILE_ATTRIBUTE_READONLY;
	if (!SetFileAttributesA(_Path, attrib))
		return oWinSetLastError();
	return true;
}

bool oFileMarkHidden(const char* _Path, bool _Hidden)
{
	DWORD attrib = GetFileAttributesA(_Path);
	if (_Hidden)
		attrib |= FILE_ATTRIBUTE_HIDDEN;
	else
		attrib &=~FILE_ATTRIBUTE_HIDDEN;
	if (!SetFileAttributesA(_Path, attrib))
		return oWinSetLastError();
	return true;
}

bool oFileDelete1(const char* _Path, const oFILE_DESC& _Desc)
{
	const char* filebase = oGetFilebase(_Path);
	if (strcmp("..", filebase) && strcmp(".", filebase))
		oFileDelete(_Path);
	return true;
}

bool oFileDelete(const char* _Path)
{
	oFILE_DESC d;
	if (!oFileGetDesc(_Path, &d))
		return false; // propagate oFileGetDesc error

	if (d.Directory)
	{
		// First clear out all contents, then the dir can be removed

		char wildcard[_MAX_PATH];
		strcpy_s(wildcard, _Path);
		oEnsureSeparator(wildcard);
		strcat_s(wildcard, "*");

		oVERIFY(oFileEnum(wildcard, oFileDelete1));

		if (!RemoveDirectory(_Path))
			return oWinSetLastError();
	}

	else if (!DeleteFileA(_Path))
		return oWinSetLastError();

	return true;
}

bool oFileCreateFolder(const char* _Path)
{
	if (oFileExists(_Path))
		return oErrorSetLast(oERROR_REDUNDANT, "Path %s already exists", _Path);

	// CreateDirectory will only create a new immediate dir in the specified dir,
	// so manually recurse if it fails...

	if (!CreateDirectory(_Path, 0))
	{
		if (GetLastError() == ERROR_PATH_NOT_FOUND)
		{
			oStringPath parent(_Path);
			oTrimFilename(parent, true);

			if (!oFileCreateFolder(parent))
				return false; // pass thru error message

			// Now try again
			if (!CreateDirectory(_Path, 0))
				return oWinSetLastError();
		}

		else
			return oWinSetLastError();
	}
	return true;
}

// This is platform specific because it is different on windows
oAPI void oFileMakeText( char* _pBinaryFile, size_t _szBinaryFile )
{
	size_t s = 0;
	for(size_t e = 0; e < _szBinaryFile; ++s, ++e )
	{
		if( _pBinaryFile[e] == '\r' && _pBinaryFile[e + 1] == '\n')
		{
			++e;
		}
		_pBinaryFile[s] = _pBinaryFile[e];
	}

	_pBinaryFile[__min( s, _szBinaryFile - 1)] = 0;
}



bool oFileCompare(const char* _Path1, const char* _Path2)
{
  if (!oFileExists(_Path1))
    return false;

  if (!oFileExists(_Path2))
    return false;

  oRef<oBuffer> buffer1;
  if (!oBufferCreate(_Path1, false, &buffer1))
    return false;

  oLockedPointer<oBuffer> lockedBuffer1(buffer1);

  oRef<oBuffer> buffer2;
  if (!oBufferCreate(_Path2, false, &buffer2))
    return false;

  oLockedPointer<oBuffer> lockedBuffer2(buffer2);

  // Compare with Golden Image
  if (lockedBuffer1->GetSize() != lockedBuffer2->GetSize())
    return false;

  if(memcmp(lockedBuffer1->GetData(), lockedBuffer2->GetData(), lockedBuffer1->GetSize()) != 0)
    return false;

  return true;


}
