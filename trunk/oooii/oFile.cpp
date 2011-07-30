// $(header)
#include <oooii/oFile.h>
#include <oooii/oAssert.h>
#include <oooii/oErrno.h>
#include <oooii/oHeap.h>
#include <oooii/oPath.h>
#include <oooii/oSwizzle.h>
#include <oooii/oSize.h>

namespace oFile {
namespace detail {

template<typename WIN32_TYPE>
void convert(DESC* _pDesc, const WIN32_TYPE* _pData)
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

struct FIND_CONTEXT
{
	HANDLE hContext;
	char Wildcard[_MAX_PATH];
};

}

ScopedFile::ScopedFile(const char* _Path, const char* _Mode)
{
	if (0 == fopen_s((FILE**)&FileHandle, _Path, _Mode))
		strcpy_s(Path, oSAFESTR(_Path));
	else
	{
		errno_t err = 0;
		_get_errno(&err);
		oSetLastError(err, "Failed to open %s", _Path);
		FileHandle = 0;
		*Path = 0;
	}
}

ScopedFile::~ScopedFile()
{
	if (FileHandle)
		fclose((FILE*)FileHandle);
}

bool EnumFiles(const char* _WildcardPath, oFUNCTION<bool(const char* _FullPath, const DESC& _Desc)> _EnumFunction)
{
	if (!_WildcardPath || !*_WildcardPath || !_EnumFunction)
	{
		oSetLastError(EINVAL);
		return false;
	}

	WIN32_FIND_DATA fd;
	HANDLE hFind = FindFirstFile(_WildcardPath, &fd);
	if (hFind == INVALID_HANDLE_VALUE)
	{
		oSetLastError(ENOENT);
		return false;
	}

	oFile::DESC desc;
	char ResolvedPath[_MAX_PATH];
	strcpy_s(ResolvedPath, _WildcardPath);
	
	do
	{
		detail::convert(&desc, &fd);
		oTrimFilename(ResolvedPath);
		strcat_s(ResolvedPath, fd.cFileName);

		bool result = _EnumFunction(ResolvedPath, desc);
		if (!result)
			return true;

		if (!FindNextFile(hFind, &fd))
		{
			if (!FindClose(hFind))
			{
				oWinSetLastError();
				return false;
			}

			return true;
		}

	} while (1);
}

unsigned long long GetSize(Handle _hFile)
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

bool Touch(const char* _Path, time_t _PosixTimestamp)
{
	bool result = false;
	FILE* f = nullptr;
	if (0 == fopen_s(&f, _Path, "ab"))
	{
		result = Touch((oFile::Handle)f, _PosixTimestamp);
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

bool Touch(Handle _hFile, time_t _PosixTimestamp)
{
	HANDLE hFile = oGetFileHandle((FILE*)_hFile);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		oSetLastError(ENOENT, "File handle incorrect");
		return false;
	}
	FILETIME time;
	oUnixTimeToFileTime(_PosixTimestamp, &time);
	if (!SetFileTime(hFile, 0, 0, &time))
	{
		oWinSetLastError();
		return false;
	}
	
	return true;
}

bool MarkReadOnly(const char* _Path, bool _ReadOnly)
{
	DWORD attrib = GetFileAttributesA(_Path);

	if (_ReadOnly)
		attrib |= FILE_ATTRIBUTE_READONLY;
	else
		attrib &=~FILE_ATTRIBUTE_READONLY;

	if (!SetFileAttributesA(_Path, attrib))
	{
		oWinSetLastError();
		return false;
	}

	return true;
}

bool MarkHidden(const char* _Path, bool _Hidden)
{
	DWORD attrib = GetFileAttributesA(_Path);

	if (_Hidden)
		attrib |= FILE_ATTRIBUTE_HIDDEN;
	else
		attrib &=~FILE_ATTRIBUTE_HIDDEN;

	if (!SetFileAttributesA(_Path, attrib))
	{
		oWinSetLastError();
		return false;
	}

	return true;
}

bool Delete(const char* _Path)
{
	return !!DeleteFileA(_Path);
}

bool CreateFolder(const char* _Path)
{
	if (Exists(_Path))
	{
		oSetLastError(EEXIST, "Path %s already exists", _Path);
		return false;
	}

	return !!CreateDirectory(_Path, 0);
}

void CreateTempFolder(char* _TempPath, size_t _SizeofTempPath)
{
	oGetSysPath(_TempPath, _SizeofTempPath, oSYSPATH_TMP);
	size_t pathLength = strlen(_TempPath);

	while (true)
	{
		sprintf_s(_TempPath + pathLength, _SizeofTempPath - pathLength, "%i", rand());

		if (oFile::CreateFolder(_TempPath) || oGetLastError() != EEXIST)
			break;
	}
}

bool Exists(const char* _Path)
{
	return INVALID_FILE_ATTRIBUTES != GetFileAttributesA(_Path);
}

bool IsText(Handle _hFile)
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
	long origin = ftell((FILE*)_hFile);
	if (origin && 0 != fseek((FILE*)_hFile, 0, SEEK_SET))
	{
		int err = 0;
		_get_errno(&err);
		oSetLastError(err, "Failed file seek");
		return false;
	}

	size_t actualSize = fread(buf, 1, BLOCK_SIZE, (FILE*)_hFile);

	if (origin && 0 != fseek((FILE*)_hFile, origin, SEEK_SET))
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
		result = IsText((Handle)f);
		fclose(f);
	}

	return result;
}

bool Open(const char* _Path, bool _ForWrite, bool _AsText, Handle* _phFile)
{
	char open[3];
	open[0] = _ForWrite ? 'w' : 'r';
	open[1] = _AsText ? 't' : 'b';
	open[2] = 0;

	errno_t err = fopen_s((FILE**)_phFile, _Path, open);
	if (err)
	{
		oSetLastError(err, "Failed to open %s", oSAFESTRN(_Path));
		return false;
	}

	return true;
}

bool Close(Handle _hFile)
{
	return 0 == fclose((FILE*)_hFile);
}

unsigned long long Tell(Handle _hFile)
{
	unsigned long long offset = _ftelli64((FILE*)_hFile);
	if (offset == 1L)
	{
		errno_t err = 0;
		_get_errno(&err);
		oSetLastError(err);
	}

	return offset;
}

bool Seek(Handle _hFile, long long _Offset, int _Origin)
{ 
	if (_fseeki64((FILE*)_hFile, _Offset, _Origin) == -1)
	{
		errno_t err = 0;
		_get_errno(&err);
		oSetLastError(err);
		return false;
	}

	return true; 
}

size_t FRead(void* _pDestination, size_t _SizeofDestination, unsigned long long _Size, Handle _hFile)
{
	size_t bytesRead = fread_s(_pDestination, _SizeofDestination, 1, oSize64( _Size ), (FILE*)_hFile);
	if (bytesRead != _Size)
	{
		if (AtEndOfFile(_hFile))
			oSetLastError(EEOF);
		else
		{
			errno_t err;
			_get_errno(&err);
			oSetLastError(err);
		}
	}
	
	return bytesRead;
}

size_t FWrite(const void* _Source, size_t _Size, Handle _hFile, bool _Flush)
{
	size_t bytesWritten = fwrite(_Source, 1, _Size, (FILE*)_hFile);
	if (bytesWritten != _Size)
	{
		errno_t err;
		_get_errno(&err);
		oSetLastError(err);
	}

	if (_Flush)
		fflush((FILE*)_hFile);

	return bytesWritten;
}

bool AtEndOfFile(Handle _hFile)
{
	return !!feof((FILE*)_hFile);
}

bool GetDesc(const char* _Path, DESC* _pDesc)
{
	WIN32_FILE_ATTRIBUTE_DATA FileData;
	if( 0 == GetFileAttributesExA( _Path, GetFileExInfoStandard, &FileData ) )
		return false;

	detail::convert(_pDesc, &FileData);
	return true;
}

bool MemMap(void* _HintPointer, unsigned long long _Size, bool _ReadOnly, Handle _hFile, unsigned long long _Offset, void** _ppMappedMemory)
{
	if (_HintPointer)
	{
		oSetLastError(EINVAL, "_HintPointer must currently be nullptr.");
		return false;
	}

	if (!_hFile)
	{
		oSetLastError(EINVAL, "A valid file handle must be specified.");
		return false;
	}

	oByteSwizzle64 alignedOffset;
	alignedOffset.AsUnsignedLongLong = oByteAlignDown(_Offset, oHeap::GetSystemAllocationGranularity());
	unsigned long long offsetPadding = _Offset - alignedOffset.AsUnsignedLongLong;
	unsigned long long alignedSize = _Size + offsetPadding;
	HANDLE hFile = oGetFileHandle((FILE*)_hFile);
	
	HANDLE hMapped = CreateFileMapping(hFile, nullptr, _ReadOnly ? PAGE_READONLY : PAGE_READWRITE, 0, 0, nullptr);
	if (!hMapped)
	{
		oWinSetLastError();
		return false;
	}

	void* p = MapViewOfFile(hMapped, _ReadOnly ? FILE_MAP_READ : FILE_MAP_WRITE, alignedOffset.AsUnsignedInt[1], alignedOffset.AsUnsignedInt[0], oSize64( alignedSize ));
	if (!p)
	{
		oWinSetLastError();
		return false;
	}

	CloseHandle(hMapped); // there's still a ref held by MapViewOfFile, but why keep 2? This allows just the one release in MemUnMap when calling UnmapViewOfFile()

	*_ppMappedMemory = oByteAdd(p, oSize64( offsetPadding ));
	return true;
}

bool MemUnmap(void* _MappedPointer)
{
	void* p = oByteAlignDown(_MappedPointer, oHeap::GetSystemAllocationGranularity());
	if (!UnmapViewOfFile(p))
	{
		oWinSetLastError();
		return false;
	}
	return true;
}

bool LoadBuffer(void** _ppOutBuffer, size_t* _pOutSize, oFUNCTION<void*(size_t _Size)> _Allocate, const char* _Path, bool _AsText)
{
	bool result = false;
	FILE* f = 0;
	if (_ppOutBuffer && _Allocate)
	{
		errno_t err = fopen_s(&f, _Path, _AsText ? "rt" : "rb");
		if (!err)
		{
			oSize64 size = oFile::GetSize((Handle)f) + (_AsText ? 1 : 0); // for nul terminator
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

bool LoadBuffer(void* _pOutBuffer, size_t _SizeofOutBuffer, size_t* _pOutSize, const char* _Path, bool _AsText)
{
	bool result = false;
	FILE* f = 0;
	if (_pOutBuffer)
	{
		errno_t err = fopen_s(&f, _Path, _AsText ? "rt" : "rb");
		if (!err)
		{
			oSize64 size = oFile::GetSize((Handle)f) + (_AsText ? 1 : 0); // for nul terminator

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

bool LoadHeader( void* _pHeader, size_t _SizeofHeader, const char* _Path, bool _AsText )
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

bool SaveBuffer(const char* _Path, const void* _pSource, size_t _SizeofSource, bool _AsText, bool _Append)
{
	bool result = false;
	FILE* f;
	if (0 == fopen_s(&f, _Path, _AsText ? ( _Append ? "at" : "wt" ) : (_Append ? "ab" : "wb" ) ) )
	{
		size_t written = fwrite(_pSource, sizeof(char), _SizeofSource, f);
		result = (_SizeofSource == written);
		fclose(f);
	}

	return result;
}

} // namespace oFile
