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
#include <oPlatform/oFile.h>
#include <oBasis/oError.h>
#include <oPlatform/Windows/oWindows.h>
#include "oIOCP.h"
#include "oFileInternal.h"
#include <cstdio>
#include "oDispatchQueueGlobalIOCP.h"

struct FIND_CONTEXT
{
	HANDLE hContext;
	char Wildcard[_MAX_PATH];
};

template<typename WIN32_TYPE> static void oFileConvert(oSTREAM_DESC* _pDesc, const WIN32_TYPE* _pData)
{
	_pDesc->Created = oStd::date_cast<time_t>(_pData->ftCreationTime);
	_pDesc->Accessed = oStd::date_cast<time_t>(_pData->ftLastAccessTime);
	_pDesc->Written = oStd::date_cast<time_t>(_pData->ftLastWriteTime);
	_pDesc->Size = (_pData->nFileSizeHigh * (static_cast<unsigned long long>(MAXDWORD) + 1)) + _pData->nFileSizeLow;
	_pDesc->Directory = !!(_pData->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY);
	//_pDesc->Archive = !!(_pData->dwFileAttributes & FILE_ATTRIBUTE_ARCHIVE);
	//_pDesc->Compressed = !!(_pData->dwFileAttributes & FILE_ATTRIBUTE_COMPRESSED);
	//_pDesc->Encrypted = !!(_pData->dwFileAttributes & FILE_ATTRIBUTE_ENCRYPTED);
	_pDesc->Hidden = !!(_pData->dwFileAttributes & FILE_ATTRIBUTE_HIDDEN);
	_pDesc->ReadOnly = !!(_pData->dwFileAttributes & FILE_ATTRIBUTE_READONLY);
	//_pDesc->System = !!(_pData->dwFileAttributes & FILE_ATTRIBUTE_SYSTEM);
	//_pDesc->Offline = !!(_pData->dwFileAttributes & FILE_ATTRIBUTE_OFFLINE);
}

static bool oFileSetLastError()
{
	errno_t err = 0;
	_get_errno(&err);
	char strerr[256];
	strerror_s(strerr, err);
	return oErrorSetLast(std::errc::io_error, "%s", strerr);
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
		oErrorSetLast(std::errc::io_error, "Failed to open %s (%s)", oSAFESTRN(_Path), strerr);
		return false;
	}

	return true;
}

bool oFileClose(oHFILE _hFile)
{
	if (!_hFile)
		return oErrorSetLast(std::errc::invalid_argument);
	if (0 != fclose((FILE*)_hFile))
		return oErrorSetLast(std::errc::io_error);
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
	oSizeT CheckedReadSize(_ReadSize);
	oSizeT CheckedSizeofDestination(_SizeofDestination);
	size_t bytesRead = fread_s(_pDestination, CheckedSizeofDestination, 1, CheckedReadSize, (FILE*)_hFile);
	if (CheckedReadSize != bytesRead && !oFileAtEnd(_hFile))
		oFileSetLastError();
	return bytesRead;
}

unsigned long long oFileWrite(oHFILE _hFile, const void* _pSource, unsigned long long _WriteSize, bool _Flush )
{
	oSizeT CheckedWriteSize(_WriteSize);
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

bool oFileGetDesc(const char* _Path, oSTREAM_DESC* _pDesc)
{
	WIN32_FILE_ATTRIBUTE_DATA FileData;
	if (0 == GetFileAttributesExA(_Path, GetFileExInfoStandard, &FileData))
		return oWinSetLastError();
	oFileConvert(_pDesc, &FileData);
	return true;
}

bool oFileEnum(const char* _WildcardPath, oFUNCTION<bool(const char* _FullPath, const oSTREAM_DESC& _Desc)> _EnumFunction)
{
	if (!_WildcardPath || !*_WildcardPath || !_EnumFunction)
		return oErrorSetLast(std::errc::invalid_argument);

	WIN32_FIND_DATA fd;
	HANDLE hFind = FindFirstFile(_WildcardPath, &fd);
	oStd::finally closeSearch([&](){ if(hFind != INVALID_HANDLE_VALUE) FindClose(hFind); });

	if (hFind == INVALID_HANDLE_VALUE)
		return oErrorSetLast(std::errc::no_such_file_or_directory);

	oSTREAM_DESC desc;
	char ResolvedPath[_MAX_PATH];
	oStrcpy(ResolvedPath, _WildcardPath);

	do
	{
		oFileConvert(&desc, &fd);
		oTrimFilename(ResolvedPath);
		oStrcat(ResolvedPath, fd.cFileName);

		bool result = _EnumFunction(ResolvedPath, desc);
		if (!result)
			return true;

		if (!FindNextFile(hFind, &fd))
			return true;

	} while (1);

	return false;
}

bool oFileEnumFilesRecursively(const char* _Path, oFUNCTION<bool(const char* _FullPath, const oSTREAM_DESC& _Desc)> _EnumFunction)
{
	oStd::path_string WildCard = _Path;
	oEnsureSeparator(WildCard);
	oStrAppendf(WildCard, "*.*");
	oFileEnum(WildCard,
		[&](const char* _pPath, const oSTREAM_DESC& _Desc)->bool
		{
			if( !_EnumFunction(_pPath, _Desc) )
				return false;

			oSTREAM_DESC FileDesc;
			if( !oFileGetDesc(_pPath, &FileDesc) )
				return false;

			const char* pSrcBase = oGetFilebase(_pPath);
			if(FileDesc.Directory && oStrcmp("..", pSrcBase) && oStrcmp(".", pSrcBase))
			{
				if(!oFileEnumFilesRecursively(_pPath, _EnumFunction))
					return false;
			}

			return true;
		});

	return true;
}

bool oFileTouch(oHFILE _hFile, time_t _UnixTimestamp)
{
	HANDLE hFile = oGetFileHandle((FILE*)_hFile);
	if (hFile == INVALID_HANDLE_VALUE)
		return oErrorSetLast(std::errc::no_such_file_or_directory, "File handle incorrect");
	FILETIME time = oStd::date_cast<FILETIME>(_UnixTimestamp);
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

bool oFileCopy1(const char* _PathFrom, const oSTREAM_DESC& _DescFrom, const char* _PathTo, const oSTREAM_DESC& _DescTo, bool _Recursive)
{
	const char* filebase = oGetFilebase(_PathFrom);
	if (oStrcmp("..", filebase) && oStrcmp(".", filebase))
	{
		oStd::path_string dest(_PathTo);
		if (_DescTo.Directory)
		{
			oEnsureSeparator(dest);
			oStrAppendf(dest, filebase);
		}

		if (_DescFrom.Directory)
		{
			if (_Recursive)
			{
				if (!oFileCreateFolder(dest))
					return false;
				
				oFileCopy(_PathFrom, dest, _Recursive);
			}
			return true;
		}

		if (!CopyFileA(_PathFrom, dest, FALSE))
			return oWinSetLastError();

		oFileMarkReadOnly(dest, false);
	}
	return true;
}

bool oFileCopy(const char* _PathFrom, const char* _PathTo, bool _Recursive)
{
	oSTREAM_DESC src;
	if (!oFileGetDesc(_PathFrom, &src))
		return oErrorSetLast(std::errc::no_such_file_or_directory, "Source not found: %s", oSAFESTRN(_PathFrom));

	oSTREAM_DESC dst;
	if (!oFileGetDesc(_PathTo, &dst)) // assume dst is same type as src if dst doesn't exist
		dst = src;
	
	if (src.Directory && !dst.Directory)
		return oErrorSetLast(std::errc::invalid_argument, "Trying to copy a directory (%s) to a file (%s)", _PathFrom, _PathTo);

	if (src.Directory)
	{
		char wildcard[_MAX_PATH];
		oStrcpy(wildcard, _PathFrom);
		oEnsureSeparator(wildcard);
		oStrcat(wildcard, "*");

		oVERIFY(oFileEnum(wildcard, oBIND(oFileCopy1, oBIND1, oBIND2, oBINDREF(_PathTo), oBINDREF(dst), oBINDREF(_Recursive))));
	}

	else if (!oFileCopy1(_PathFrom, src, _PathTo, dst, _Recursive))
		return false;

	return true;
}

bool oFileMove(const char* _PathFrom, const char* _PathTo, bool _Force)
{
	oSTREAM_DESC src;
	if (!oFileGetDesc(_PathFrom, &src))
		return oErrorSetLast(std::errc::no_such_file_or_directory, "Source not found: %s", oSAFESTRN(_PathFrom));

	// @oooii-tony: might want to do more checking if this is a dir or file...
	// let's see how the platform call behaves...

	if (_Force && oFileExists(_PathTo) && !oFileDelete(_PathTo))
		return oErrorSetLast(std::errc::io_error, "Cannot delete pre-existing destination %s before moving.", oSAFESTRN(_PathTo));

	if (!MoveFileA(_PathFrom, _PathTo))
	{
		oWinSetLastError();
		oStd::xlstring tmp(oErrorGetLastString());
		return oErrorSetLast(oErrorGetLast(), "rename failed: %s -> %s\n%s", oSAFESTRN(_PathFrom), oSAFESTRN(_PathTo), tmp.c_str());
	}
	return true;
}

bool oFileDelete1(const char* _Path, const oSTREAM_DESC& _Desc)
{
	const char* filebase = oGetFilebase(_Path);
	if (oStrcmp("..", filebase) && oStrcmp(".", filebase))
		oFileDelete(_Path);
	return true;
}

bool oFileDelete(const char* _Path)
{
	oSTREAM_DESC d;
	if (!oFileGetDesc(_Path, &d))
		return false; // propagate oFileGetDesc error

	if (d.Directory)
	{
		// First clear out all contents, then the dir can be removed

		char wildcard[_MAX_PATH];
		oStrcpy(wildcard, _Path);
		oEnsureSeparator(wildcard);
		oStrcat(wildcard, "*");

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
		return oErrorSetLast(std::errc::operation_in_progress, "Path %s already exists", _Path);

	// CreateDirectory will only create a new immediate dir in the specified dir,
	// so manually recurse if it fails...

	if (!CreateDirectory(_Path, 0))
	{
		if (GetLastError() == ERROR_PATH_NOT_FOUND)
		{
			oStd::path_string parent(_Path);
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

bool oFileMap(const char* _Path, bool _ReadOnly, const oSTREAM_RANGE& _MapRange, void** _ppMappedMemory)
{
	oHFILE hFile;
	if (!oFileOpen(_Path, _ReadOnly ? oFILE_OPEN_BIN_READ : oFILE_OPEN_BIN_WRITE, &hFile))
		return false; // pass through error

	SYSTEM_INFO si;
	GetSystemInfo(&si);
	oStd::byte_swizzle64 alignedOffset;
	alignedOffset.as_unsigned_long_long = oStd::byte_align_down(_MapRange.Offset, si.dwAllocationGranularity);
	unsigned long long offsetPadding = _MapRange.Offset - alignedOffset.as_unsigned_long_long;
	unsigned long long alignedSize = _MapRange.Size + offsetPadding;

	HANDLE FileHandle = oGetFileHandle((FILE*)hFile);

	DWORD fProtect = _ReadOnly ? PAGE_READONLY : PAGE_READWRITE;
	HANDLE hMapped = CreateFileMapping(FileHandle, nullptr, fProtect, 0, 0, nullptr);
	if (!hMapped)
		return oWinSetLastError();

	void* p = MapViewOfFile(hMapped, fProtect == PAGE_READONLY ? FILE_MAP_READ : FILE_MAP_WRITE, alignedOffset.as_unsigned_int[1], alignedOffset.as_unsigned_int[0], oULLong(alignedSize));
	if (!p)
		return oWinSetLastError();

	// Detach the file from C-lib, but also decrement the ref count on the HANDLE
	if (!oFileClose(hFile))
		return false; // pass through error

	// Close the ref held by CreateFileMapping
	CloseHandle(hMapped);
	*_ppMappedMemory = oStd::byte_add(p, oULLong(offsetPadding));

	// So now we exit with a ref count of 1 on the underlying HANDLE, that held by
	// MapViewOfFile, that way the file is fully closed when oFileUnmap is called.
	return true;
}

bool oFileUnmap(void* _MappedPointer)
{
	SYSTEM_INFO si;
	GetSystemInfo(&si);
	void* p = oStd::byte_align_down(_MappedPointer, si.dwAllocationGranularity);
	if (!UnmapViewOfFile(p))
		return oWinSetLastError();
	return true;
}
