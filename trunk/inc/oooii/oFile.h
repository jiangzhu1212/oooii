// $(header)

// File-related utilities. These are implemented using the basic 
// fopen/platform API - nothing particularly asynchronous or otherwise
// ambitious through this API.
#pragma once
#ifndef oFile_h
#define oFile_h

#include <oooii/oByte.h>
#include <oooii/oStddef.h>
#include <stdio.h>

namespace oFile
{
	// NOTE: Prefer using LoadBuffer and SaveBuffer when possible
	struct ScopedFile
	{
		// Use bool test for success/failure
		// (ScopedFile myFile(myPath, "wb"); if (!myFile) return kFailure;)
		ScopedFile(const char* _Path, const char* _Mode);
		~ScopedFile();
		operator FILE*() { return pFile; }
		operator bool() const { return !!pFile; }
	protected:
		FILE* pFile;
		char Path[_MAX_PATH];
	};

	struct DESC
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

	// Get's the descriptoin for a file.  Returns false if the file doesn't exist
	bool GetDesc(const char* _Path, DESC* _pDesc); 

	// Find first file prefixed with the specified wildcard. If this returns 
	// true, then context can be passed to oFindNextFile and must be closed when 
	// finished by using CloseFind. If this returns false, context will be NULL 
	// and thus not need to be closed. False is returned if there are no more 
	// matches.
	bool FindFirst(DESC* _pDesc, char (&_Path)[_MAX_PATH], const char* _Wildcard, void** _pFindContext);
	
	// Iterate on a context created using FindFirst. False means there are no
	// more files.
	bool FindNext(DESC* _pDesc, char (&_Path)[_MAX_PATH], void* _pFindContext);

	// This call must be made on the find context created with a successful call 
	// to FindFirst. If FindFirst fails, CloseFind should not be called.
	bool CloseFind(void* _pFindContext);

	// Returns size of an open file. This may open and close the file on some 
	// systems.
	size_t GetSize(FILE* _File);

	bool Touch(const char* _Path, time_t _PosixTimestamp);

	// Remember, the file must've been open with read access 
	// (i.e. fopen(..., "wb" or "wt"))
	bool Touch(FILE* _File, time_t _PosixTimestamp);

	bool MarkReadOnly(const char* _Path, bool _ReadOnly = true);

	bool MarkHidden(const char* _Path, bool _Hidden = true);

	bool Delete(const char* _Path);

	// Returns true if created, false otherwise. If the path already exists, 
	// oGetLastError will be EEXIST.
	bool CreateFolder(const char* _Path);

	// Creates a new and unique directory under the system's temp folder
	void CreateTempFolder(char* _TempPath, size_t _SizeofTempPath);
	template<size_t size> inline void CreateTempFolder(char (&_TempPath)[size]) { CreateTempFolder(_TempPath, size); }

	// Returns true if the specified path exist
	bool Exists(const char* _Path);

	// Returns true if the specified file is a text file using the criteria found
	// in Perl, that is if a block in the beginning of the file contains less 
	// than a certain percentage of bytes that are 128 < byte < 0.
	bool IsText(FILE* _File);
	bool IsText(const char* _Path);

	inline bool IsBinary(FILE* _File) { return !IsText(_File); }
	inline bool IsBinary(const char* _Path) { return !IsText(_Path); }

	// Uses the specified allocator to allocate _OutBuffer and fill it with the 
	// entire contents of the file specified by _Path. Returns false if there was 
	// an error.
	bool LoadBuffer(void** _ppOutBuffer, size_t* _pOutSize, oFUNCTION<void*(size_t _Size)> _Allocate, const char* _Path, bool _AsText);

	// Loads into a pre-allocated buffer. This fails with an oGetLastError() of 
	// EINVAL if the buffer is too small to contain the contents of the file.
	bool LoadBuffer(void* _pOutBuffer, size_t _SizeofOutBuffer, size_t* _pOutSize, const char* _Path, bool _AsText);
	template<typename T, size_t size> inline bool LoadBuffer(T (&_pOutBuffer)[size], size_t* _pOutSize, const char* _Path, bool _AsText) { return LoadBuffer(_pOutBuffer, size, _pOutSize, _Path, _AsText); }

	// Loads the "header" (known number of bytes at the beginning of a file) into a buffer, returning the actual number of bytes read if the file is smaller than the buffer
	bool LoadHeader(void* _pHeader, size_t _SizeofHeader, const char* _Path, bool _AsText);
	bool SaveBuffer(const char* _Path, const void* _pSource, size_t _SizeofSource, bool _AsText, bool _Append);
};

#endif
