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
#include "oIOCP.h"
#include "oFileInternal.h"
#include <cstdio>

struct FIND_CONTEXT
{
	HANDLE hContext;
	char Wildcard[_MAX_PATH];
};

template<typename WIN32_TYPE> static void oFileConvert(oFILE_DESC* _pDesc, const WIN32_TYPE* _pData)
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

static void oSetIOCPOpOffset(oIOCPOp* _op, oSize64 _offset)
{
	_op->Offset = _offset.c_type()&0xffffffff;
	_op->OffsetHigh = _offset>>32;
}

struct oFileReader_Impl : public oFileReader
{
	oDEFINE_REFCOUNT_IOCP_INTERFACE(Refcount, IOCP);
	oDEFINE_NOOP_QUERYINTERFACE();

	struct Operation
	{
		void		*pData;
		oFileRange Range;
		callback_t  Callback;
	};

	oFileReader_Impl(const char* _Path, bool* _pSuccess)
		: IOCP(nullptr)
		, Path(_Path)
	{
		if( strlen(_Path) > Path->capacity() )
			return;

		hFile = nullptr;
		*_pSuccess = true;

		hFile = CreateFile(GetPath(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL);
		if (hFile == INVALID_HANDLE_VALUE)
		{
			oWinSetLastError();
			*_pSuccess = false;
			return;
		}
		else
		{
			oFILE_DESC FDesc;

			oFileGetDesc(_Path, &FDesc);
			FileDesc.Initialize(FDesc);
			oIOCP::DESC IOCPDesc;

			IOCPDesc.Handle = hFile;
			IOCPDesc.IOCompletionRoutine = oBIND(&oFileReader_Impl::IOCPCallback, this, oBIND1);
			IOCPDesc.MaxOperations = 16;
			IOCPDesc.PrivateDataSize = sizeof(Operation);

			if(!oIOCPCreate(IOCPDesc, this, &IOCP))
			{
				oErrorSetLast(oERROR_INVALID_PARAMETER, "Could not create IOCP.");
				CloseHandle(hFile);
				*_pSuccess = false;
				return;
			}
		}
	}

	~oFileReader_Impl()
	{
		if (hFile != INVALID_HANDLE_VALUE)
			CloseHandle(hFile);
	}

	void IOCPCallback(oIOCPOp* _pIOCPOp)
	{
		Operation* pOp;
		_pIOCPOp->GetPrivateData(&pOp);

		pOp->Callback(true, pOp->pData, pOp->Range, this );
		_pIOCPOp->DestructPrivateData<Operation>();
		IOCP->ReturnOp(_pIOCPOp);
	}

	void DispatchRead(void* _pData, const oFileRange& _Range, callback_t _Callback) threadsafe override
	{
		if ((_Range.Offset + _Range.Size) <= (unsigned long long)FileDesc->Size)
		{
			oIOCPOp* pIOCPOp = IOCP->AcquireSocketOp();
			if( !pIOCPOp )
			{
				oErrorSetLast(oERROR_AT_CAPACITY, "IOCPOpPool is empty, you're sending too fast.");
				_Callback(false, _pData, _Range, this);
				return;
			}

			Operation* pOp;
			pIOCPOp->ConstructPrivateData(&pOp);
			oSetIOCPOpOffset(pIOCPOp, _Range.Offset);
			pOp->pData = _pData;
			pOp->Callback = _Callback;
			pOp->Range = _Range;

			DWORD numBytesRead;

			ReadFile(hFile, _pData, _Range.Size, &numBytesRead, (WSAOVERLAPPED*)pIOCPOp);
		}
		else
		{
			oErrorSetLast(oERROR_IO, "Specified range will read past file size");
			_Callback(false, _pData, _Range, this);
			return;
		}
	}

	bool Read(void* _pData, const oFileRange& _Range) threadsafe
	{
		bool bSuccess = false;
		oCountdownLatch Latch("Sync Read Latch", 1);
		oFileReader::callback_t Callback =
			[&](bool _Success, const void* _pData, const oFileRange& _Range, threadsafe interface oFileReader* _pFile)
		{
			bSuccess = _Success;
			Latch.Release();
		};

		DispatchRead(_pData, _Range, Callback);

		Latch.Wait();

		return bSuccess;
	}

	void GetDesc(oFILE_DESC* _pDesc) threadsafe override
	{
		*_pDesc = *FileDesc;
	}

	const char* GetPath() const threadsafe
	{
		return Path->c_str();
	}

	oInitOnce<oFILE_DESC> FileDesc;
	oIOCP*		IOCP;
	oRefCount Refcount;
	oInitOnce<oStringPath> Path;
	oHandle hFile;
};

oAPI bool oFileReaderCreate(const char* _pFilePath, threadsafe oFileReader** _ppReadFile)
{
	bool success = false;
	oCONSTRUCT(_ppReadFile, oFileReader_Impl(_pFilePath, &success));
	return success;
}

struct oFileReaderWindowed_Impl : oFileReader
{
	oDEFINE_REFCOUNT_INTERFACE(Refcount);
	oDEFINE_NOOP_QUERYINTERFACE();

	oFileReaderWindowed_Impl(threadsafe oFileReader* _pReader, const oFileRange& _Window, bool* _pSuccess)
		: Reader(_pReader)
		, WindowStart(_Window.Offset)
		, WindowEnd(_Window.Offset + _Window.Size)
	{
		if(!Reader)
		{
			oErrorSetLast(oERROR_INVALID_PARAMETER, "Failed to specify reader");
			return;
		}

		oFILE_DESC FDesc;
		Reader->GetDesc(&FDesc);

		if (*WindowStart > FDesc.Size || *WindowEnd > FDesc.Size)
		{
			oErrorSetLast(oERROR_INVALID_PARAMETER, "Window is outside of the file's range");
			return;
		}

		FDesc.Size = *WindowEnd - *WindowStart; // patch the file size
		FileDesc.Initialize(FDesc);
		*_pSuccess = true;
	}

	bool WindowRange(const oFileRange& _Range, oFileRange* _pWindowedRange) threadsafe
	{
		oSize64 Start = *WindowStart + _Range.Offset; 

		if(Start > *WindowEnd)
			return false;

		oSize64 End = Start + _Range.Size;

		if(End > *WindowEnd)
			return false;

		_pWindowedRange->Offset = Start;
		_pWindowedRange->Size = End - Start;
		return true;
	}
	virtual void GetDesc(oFILE_DESC* _pDesc) threadsafe override
	{
		*_pDesc = *FileDesc;
	}
	virtual const char* GetPath() const threadsafe override
	{
		return Reader->GetPath();
	}
	virtual void DispatchRead(void* _pData, const oFileRange& _Range, callback_t _Callback) threadsafe override
	{
		oFileRange Range;
		if(!WindowRange(_Range, &Range))
		{
			oErrorSetLast(oERROR_IO, "Specified range will read past file size");
			_Callback(false, _pData, _Range, this);
			return;
		}

		return Reader->DispatchRead(_pData, Range, _Callback);
	}
	virtual bool Read(void* _pData, const oFileRange& _Range) threadsafe override
	{
		oFileRange Range;
		if(!WindowRange(_Range, &Range))
		{
			oErrorSetLast(oERROR_IO, "Specified range will read past file size");
			return false;
		}

		return Reader->Read(_pData, _Range);
	}

	oRefCount Refcount;
	oInitOnce<oFILE_DESC> FileDesc;
	oRef<threadsafe oFileReader> Reader;
	oInitOnce<oSize64> WindowStart;
	oInitOnce<oSize64> WindowEnd;
};
oAPI bool oFileReaderCreateWindowed( threadsafe oFileReader* _pReader, const oFileRange& _Window, threadsafe oFileReader** _ppReadFile )
{
	bool success = false;
	oCONSTRUCT(_ppReadFile, oFileReaderWindowed_Impl(_pReader, _Window, &success));
	return success;
}

struct oFileWriter_Impl : public oFileWriter
{
	oDEFINE_REFCOUNT_IOCP_INTERFACE(Refcount, IOCP);
	oDEFINE_NOOP_QUERYINTERFACE();

	struct Operation
	{
		const void		*pData;
		oFileRange Range;
		callback_t  Callback;
	};

	oFileWriter_Impl(const char* _Path, bool* _pSuccess)
		: IOCP(nullptr)
		, Path(_Path)
	{
		if( strlen(_Path) > Path->capacity() )
			return;

		hFile = nullptr;
		*_pSuccess = true;

		oStringPath parent(_Path);
		*oGetFilebase(parent) = 0;

		if (!oFileCreateFolder(parent) && oErrorGetLast() != oERROR_REDUNDANT)
			return; // pass through error

		hFile = CreateFile(GetPath(), GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, FILE_FLAG_OVERLAPPED, NULL);
		if (hFile == INVALID_HANDLE_VALUE)
		{
			oWinSetLastError();
			*_pSuccess = false;
			return;
		}
		else
		{
			oIOCP::DESC IOCPDesc;

			IOCPDesc.Handle = hFile;
			IOCPDesc.IOCompletionRoutine = oBIND(&oFileWriter_Impl::IOCPCallback, this, oBIND1);
			IOCPDesc.MaxOperations = 16;
			IOCPDesc.PrivateDataSize = sizeof(Operation);

			if(!oIOCPCreate(IOCPDesc, this, &IOCP))
			{
				oErrorSetLast(oERROR_INVALID_PARAMETER, "Could not create IOCP.");
				CloseHandle(hFile);
				*_pSuccess = false;
				return;
			}
		}
	}

	~oFileWriter_Impl()
	{
		if (hFile != INVALID_HANDLE_VALUE)
			CloseHandle(hFile);
	}

	void IOCPCallback(oIOCPOp* _pIOCPOp)
	{
		Operation* pOp;
		_pIOCPOp->GetPrivateData(&pOp);

		pOp->Callback(true, pOp->pData, pOp->Range, this );
		_pIOCPOp->DestructPrivateData<Operation>();
		IOCP->ReturnOp(_pIOCPOp);
	}

	void GetDesc(oFILE_DESC* _pDesc) threadsafe override
	{
		oFileGetDesc(GetPath(), _pDesc);
	}

	void DispatchWrite(const void* _pData, const oFileRange& _Range, callback_t _Callback) threadsafe override
	{
		oIOCPOp* pIOCPOp = IOCP->AcquireSocketOp();
		if( !pIOCPOp )
		{
			oErrorSetLast(oERROR_AT_CAPACITY, "IOCPOpPool is empty, you're sending too fast.");
			return;
		}

		Operation* pOp;
		pIOCPOp->ConstructPrivateData(&pOp);
		oSetIOCPOpOffset(pIOCPOp, _Range.Offset);
		pOp->pData = _pData;
		pOp->Callback = _Callback;
		pOp->Range = _Range;

		DWORD numBytesRead;
		WriteFile(hFile, _pData, _Range.Size, &numBytesRead, (WSAOVERLAPPED*)pIOCPOp);
	}

	bool Write(const void* _pData, const oFileRange& _Range) threadsafe
	{
		bool bSuccess = false;
		oCountdownLatch Latch("Sync Write Latch", 1);
		oFileWriter::callback_t Callback =
			[&](bool _Success, const void* _pData, const oFileRange& _Range, threadsafe interface oFileWriter* _pFile)
		{
			bSuccess = _Success;
			Latch.Release();
		};

		DispatchWrite(_pData, _Range, Callback);

		Latch.Wait();

		return bSuccess;
	}

	const char* GetPath() const threadsafe
	{
		return Path->c_str();
	}

	oIOCP* IOCP;
	oRefCount Refcount;
	oInitOnce<oStringPath> Path;
	oHandle hFile;

};

oAPI bool oFileWriterCreate(const char* _pFilePath, threadsafe oFileWriter** _ppWriteFile)
{
	bool success = false;
	oCONSTRUCT(_ppWriteFile, oFileWriter_Impl(_pFilePath, &success));
	return success;
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
void oFileMakeText( char* _pBinaryFile, size_t _szBinaryFile )
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

bool oFileMap(const char* _Path, bool _ReadOnly, const oFileRange& _MapRange, void** _ppMappedMemory)
{
	oHFILE hFile;
	if (!oFileOpen(_Path, _ReadOnly ? oFILE_OPEN_BIN_READ : oFILE_OPEN_BIN_WRITE, &hFile))
		return false; // pass through error

	SYSTEM_INFO si;
	GetSystemInfo(&si);
	oByteSwizzle64 alignedOffset;
	alignedOffset.AsUnsignedLongLong = oByteAlignDown(_MapRange.Offset, si.dwAllocationGranularity);
	unsigned long long offsetPadding = _MapRange.Offset - alignedOffset.AsUnsignedLongLong;
	unsigned long long alignedSize = _MapRange.Size + offsetPadding;

	HANDLE FileHandle = oGetFileHandle((FILE*)hFile);

	DWORD fProtect = _ReadOnly ? PAGE_READONLY : PAGE_READWRITE;
	HANDLE hMapped = CreateFileMapping(FileHandle, nullptr, fProtect, 0, 0, nullptr);
	if (!hMapped)
		return oWinSetLastError();

	void* p = MapViewOfFile(hMapped, fProtect == PAGE_READONLY ? FILE_MAP_READ : FILE_MAP_WRITE, alignedOffset.AsUnsignedInt[1], alignedOffset.AsUnsignedInt[0], oSize64(alignedSize));
	if (!p)
		return oWinSetLastError();

	// Detach the file from C-lib, but also decrement the ref count on the HANDLE
	if (!oFileClose(hFile))
		return false; // pass through error

	// Close the ref held by CreateFileMapping
	CloseHandle(hMapped);
	*_ppMappedMemory = oByteAdd(p, oSize64(offsetPadding));

	// So now we exit with a ref count of 1 on the underlying HANDLE, that held by
	// MapViewOfFile, that way the file is fully closed when oFileUnmap is called.
	return true;
}

bool oFileUnmap(void* _MappedPointer)
{
	SYSTEM_INFO si;
	GetSystemInfo(&si);
	void* p = oByteAlignDown(_MappedPointer, si.dwAllocationGranularity);
	if (!UnmapViewOfFile(p))
		return oWinSetLastError();
	return true;
}
