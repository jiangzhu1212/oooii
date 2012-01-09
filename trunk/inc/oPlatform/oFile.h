// $(header)

// File-related utilities. These are implemented using the basic 
// fopen/platform API - nothing particularly asynchronous or otherwise
// ambitious through this API.
#pragma once
#ifndef oFile_h
#define oFile_h

#include <oBasis/oByte.h>
#include <oBasis/oFunction.h>
#include <cstdio>

oDECLARE_HANDLE(oHFILE);

enum oFILE_SEEK
{
	oSEEK_SET,
	oSEEK_CUR,
	oSEEK_END,
};

enum oFILE_OPEN
{
	oFILE_OPEN_BIN_READ,
	oFILE_OPEN_BIN_WRITE,
	oFILE_OPEN_BIN_APPEND,
	oFILE_OPEN_TEXT_READ,
	oFILE_OPEN_TEXT_WRITE,
	oFILE_OPEN_TEXT_APPEND,
};

struct oFILE_DESC
{
	time_t Created;
	time_t Accessed;
	time_t Written;
	unsigned long long Size;
	bool Directory:1;
	bool Archive:1;
	bool Compressed:1;
	bool Encrypted:1;
	bool Hidden:1;
	bool ReadOnly:1;
	bool System:1;
	bool Offline:1;
};

oAPI bool oFileExists(const char* _Path);
oAPI bool oFileOpen(const char* _Path, oFILE_OPEN _Open, oHFILE* _phFile);
oAPI bool oFileClose(oHFILE _hFile);
oAPI unsigned long long oFileTell(oHFILE _hFile);
oAPI bool oFileSeek(oHFILE _hFile, long long _Offset, oFILE_SEEK _Origin = oSEEK_SET);
oAPI unsigned long long oFileRead(oHFILE _hFile, void* _pDestination, unsigned long long _SizeofDestination, unsigned long long _ReadSize);
oAPI unsigned long long oFileWrite(oHFILE _hFile, const void* _pSource, unsigned long long _WriteSize, bool _Flush = false);
oAPI unsigned long long oFileGetSize(oHFILE _hFile);
oAPI bool oFileAtEnd(oHFILE _hFile);
oAPI bool oFileGetDesc(const char* _Path, oFILE_DESC* _pDesc);
oAPI bool oFileEnum(const char* _WildcardPath, oFUNCTION<bool(const char* _FullPath, const oFILE_DESC& _Desc)> _EnumFunction);
oAPI bool oFileTouch(oHFILE _hFile, time_t _PosixTimestamp);
oAPI bool oFileMarkReadOnly(const char* _Path, bool _ReadOnly = true);
oAPI bool oFileMarkHidden(const char* _Path, bool _Hidden = true);

// Delete works on either files or folders (recursively).
oAPI bool oFileDelete(const char* _Path);
oAPI bool oFileCreateFolder(const char* _Path);

// More generic API using above API
oAPI bool oFileIsText(oHFILE _hFile);
oAPI bool oFileIsText(const char* _Path);
oAPI bool oFileTouch(const char* _Path, time_t _PosixTimestamp);
oAPI char* oFileCreateTempFolder(char* _TempPath, size_t _SizeofTempPath);
template<size_t size> char* oFileCreateTempFolder(char (&_TempPath)[size]) { return oFileCreateTempFolder(_TempPath, size); }

inline bool IsBinary(oHFILE _hFile) { return !oFileIsText(_hFile); }
inline bool IsBinary(const char* _Path) { return !oFileIsText(_Path); }

// Converts a binary file in memory to text in-place
oAPI void oFileMakeText(char* _pBinaryFile, size_t _szBinaryFile);

// Uses the specified _Allocate function to allocate a buffer and read the whole
// file into that buffer.
oAPI bool oFileLoad(void** _ppOutBuffer, size_t* _pOutSize, oFUNCTION<void*(size_t _Size)> _Allocate, const char* _Path, bool _AsText);
template<typename T> bool oFileLoad(T** _ppOutBuffer, size_t* _pOutSize, oFUNCTION<void*(size_t _Size)> _Allocate, const char* _Path, bool _AsText) { return oFileLoad((void**)_ppOutBuffer, _pOutSize, _Allocate, _Path, _AsText); }

// Loads into a pre-allocated buffer. This fails with an oErrorGetLast() of 
// EINVAL if the buffer is too small to contain the contents of the file.
oAPI bool oFileLoad(void* _pOutBuffer, size_t _SizeofOutBuffer, size_t* _pOutSize, const char* _Path, bool _AsText);
template<typename T, size_t size> bool oFileLoad(T (&_pOutBuffer)[size], size_t* _pOutSize, const char* _Path, bool _AsText) { return oFileLoad(_pOutBuffer, size, _pOutSize, _Path, _AsText); }

// Loads the "header" (known number of bytes at the beginning of a file) into a buffer, returning the actual number of bytes read if the file is smaller than the buffer
oAPI bool oFileLoadHeader(void* _pHeader, size_t _SizeofHeader, const char* _Path, bool _AsText);
template<typename T> bool oFileLoadHeader(T* _pHeader, const char* _Path, bool _AsText) { return oFileLoadHeader(_pHeader, sizeof(T), _Path, _AsText); }


// files contain same data?
bool oFileCompare(const char* _Path1, const char* _Path2);


// Writes the entire buffer to the specified file. Open is specified to allow 
// text and write v. append specification. Any read Open will result in this
// function returning false with oERROR_INVALID_PARAMETER.
oAPI bool oFileSave(const char* _Path, const void* _pSource, size_t _SizeofSource, oFILE_OPEN _Open);

// Similar to Posix's mmap API.
oAPI bool oFileMap(void* _HintPointer, unsigned long long _Size, bool _ReadOnly, oHFILE _hFile, unsigned long long _Offset, void** _ppMappedMemory);
oAPI bool oFileUnmap(void* _MappedPointer);

// (oBuffer support) Load a file into a newly allocated buffer using malloc to allocate the memory.
bool oBufferCreate(const char* _Path, bool _IsText, threadsafe interface oBuffer** _ppBuffer);

class oScopedFile
{
	oHFILE hFile;
	char Path[_MAX_PATH];
public:

	oScopedFile(const char* _Path, oFILE_OPEN _Open, bool* _pSuccess = false)
	{
		strcpy_s(Path, oSAFESTRN(_Path));
		bool success = oFileOpen(_Path, _Open, &hFile);
		if (_pSuccess)
			*_pSuccess = success;
	}

	~oScopedFile()
	{
		oFileClose(hFile);
	}

	operator bool() const { return !!hFile; }
	operator oHFILE() const { return hFile; }
};

#endif
