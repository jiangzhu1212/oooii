// $(header)
#include <oooii/oFile.h>
#include <oooii/oAssert.h>
#include <oooii/oErrno.h>
#include <oooii/oPath.h>
#include <io.h>

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

} // namespace detail

ScopedFile::ScopedFile(const char* _Path, const char* _Mode)
{
	if (0 == fopen_s(&pFile, _Path, _Mode))
		strcpy_s(Path, oSAFESTR(_Path));
	else
	{
		errno_t err = 0;
		_get_errno(&err);
		oSetLastError(err, "Failed to open %s", _Path);
		pFile = 0;
		*Path = 0;
	}
}

ScopedFile::~ScopedFile()
{
	if (pFile)
		fclose(pFile);
}

bool FindFirst( DESC* _pDesc, char (&_Path)[_MAX_PATH], const char* _Wildcard, void** _pFindContext )
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
	detail::convert(_pDesc, &fd);
	
	strcpy_s(_Path, _Wildcard);
	oTrimFilename(_Path);
	strcat_s(_Path, fd.cFileName);

	return true;
}

bool FindNext( DESC* _pDesc, char (&_Path)[_MAX_PATH], void* _pFindContext )
{
	detail::FIND_CONTEXT* ctx = static_cast<detail::FIND_CONTEXT*>(_pFindContext);
	if (!_pDesc || !ctx || ctx->hContext == INVALID_HANDLE_VALUE) return false;
	WIN32_FIND_DATA fd;
	if (!FindNextFile(ctx->hContext, &fd))
		return false;
	detail::convert(_pDesc, &fd);
	strcpy_s(_Path, fd.cFileName);

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
		oSetLastErrorNative(::GetLastError());
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
		oSetLastErrorNative(::GetLastError());
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
		oSetLastErrorNative(::GetLastError());
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

bool GetDesc( const char* _Path, DESC* _pDesc )
{
	WIN32_FILE_ATTRIBUTE_DATA FileData;
	if( 0 == GetFileAttributesExA( _Path, GetFileExInfoStandard, &FileData ) )
		return false;

	detail::convert(_pDesc, &FileData);
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

bool LoadBuffer(void* _pOutBuffer, size_t _SizeofOutBuffer, size_t* _pOutSize, const char* _Path, bool _AsText)
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
