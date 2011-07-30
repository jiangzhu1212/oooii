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
	oDECLARE_HANDLE(Handle)

	// NOTE: Prefer using LoadBuffer and SaveBuffer when possible
	struct ScopedFile
	{
		// Use bool test for success/failure
		// (ScopedFile myFile(myPath, "wb"); if (!myFile) return kFailure;)
		ScopedFile(const char* _Path, const char* _Mode);
		~ScopedFile();
		// @oooii-eric: TODO: Should get rid of the FILE* casts.
		operator FILE*() const { return (FILE*)FileHandle; }
		operator FILE*() { return (FILE*)FileHandle; }
		operator Handle() const { return FileHandle; }
		operator Handle() { return FileHandle; }
		operator bool() const { return !!FileHandle; }
		operator bool() { return !!FileHandle; }
	protected:
		Handle FileHandle;
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

	bool Open(const char* _Path, bool _ForWrite, bool _AsText, oFile::Handle* _phFile);
	bool Close(oFile::Handle _hFile);

	unsigned long long Tell(oFile::Handle _hFile);
	bool Seek(oFile::Handle _hFile, long long _Offset, int _Origin = SEEK_SET);

	// NOTE: FRead's _SizeofDestination is the maximum number of bytes that can be
	// read into _pDestination before memory corruption would occur.
	size_t FRead(void* _pDestination, size_t _SizeofDestination, unsigned long long _ReadSize, Handle _hFile);
	size_t FWrite(const void* _pDestination, size_t _Size, Handle _hFile, bool _Flush = false);

	bool AtEndOfFile(Handle _hFile);
	
	// Get's the descriptoin for a file.  Returns false if the file doesn't exist
	bool GetDesc(const char* _Path, DESC* _pDesc); 

	// Go through all files matching the specified wildcard and call the specified 
	// function that will contain the details of each file. The function should 
	// return true to continue to the next file, or false to abort the enumeration.
	bool EnumFiles(const char* _WildcardPath, oFUNCTION<bool(const char* _FullPath, const DESC& _Desc)> _EnumFunction);

	// Returns size of an open file. This may open and close the file on some 
	// systems.
	unsigned long long GetSize(oFile::Handle _hFile);

	bool Touch(const char* _Path, time_t _PosixTimestamp);

	// Remember, the file must've been open with read access 
	// (i.e. fopen(..., "wb" or "wt"))
	bool Touch(oFile::Handle _hFile, time_t _PosixTimestamp);

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
	bool IsText(oFile::Handle _hFile);
	bool IsText(const char* _Path);

	inline bool IsBinary(oFile::Handle _hFile) { return !IsText(_hFile); }
	inline bool IsBinary(const char* _Path) { return !IsText(_Path); }

	bool MemMap(void* _HintPointer, unsigned long long _Size, bool _ReadOnly, oFile::Handle _hFile, unsigned long long _Offset, void** _ppMappedMemory);
	bool MemUnmap(void* _MappedPointer);

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
