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

#include <oBasis/oSize.h>
#include <oBasis/oByte.h>
#include <oBasis/oFunction.h>
#include <cstdio>

// _____________________________________________________________________________
// Primary file read/write interface. These file interfaces support file I/O in
// a performance-optimal threadsafe manner and should be the preferred way to
// use files and pass files around. Do not use fopen or CreateFile or the like.

struct oFILE_DESC
{
	time_t Created;
	time_t Accessed;
	time_t Written;
	oSize64 Size;
	bool Directory:1;
	bool Archive:1;
	bool Compressed:1;
	bool Encrypted:1;
	bool Hidden:1;
	bool ReadOnly:1;
	bool System:1;
	bool Offline:1;
};

// Pass APPEND as an offset to DispatchWrite() or Write() to indicate an 
// "append" operation. Because DispatchWrite() is asynchronous, great care must 
// be taken when using this relative specification.
static const unsigned long long oFILE_APPEND = ~0ull;

struct oFileRange
{
	oFileRange()
		: Offset(0)
		, Size(0)
	{}

	// Offset from base of file for reading/writing to begin
	oSize64 Offset;

	// Number of bytes to read/write starting at the offset
	oSize64 Size;
};

interface oFile : oInterface
{
	// This is the base class from which read/write usage types are derived. Basic 
	// inspection API is provided and this interface can be QueryInterfaced for
	// the derived types.

	// Returns some vitals about this file
	virtual void GetDesc(oFILE_DESC* _pDesc) threadsafe = 0;

	// returns the full path to the file
	virtual const char* GetPath() const threadsafe = 0;
};

interface oFileReader : oFile
{
	// Read-only binary file. The underlying implementation does not provide for
	// under-the-hood to-text conversion, but oFileMakeText can be called on the 
	// result of a read to do the equivalent replacement of \r\n -> \n.

	// This function is called when the DispatchRead is complete and runs in the 
	// IO thread, except if there is an immediate error that prevents the read
	// from being scheduled, then the callback executes in the calling thread.
	typedef oFUNCTION<void(bool _Success, const void* _pData, const oFileRange& _Range, threadsafe oFileReader* _pFile)> callback_t;

	// Asynchronously dispatch a read. Any failure condition is passed through to 
	// the callback. oGetLastError is valid when the callback's _Success parameter
	// is false.
	virtual void DispatchRead(void* _pData, const oFileRange& _Range, callback_t _Callback) threadsafe = 0;

	// Synchronous file read, this will not return until _pData is filled with 
	// data from the specified range. No bounds checking is done on _pData, so 
	// ensure it is properly allocated to receive the specified range.
	virtual bool Read(void* _pData, const oFileRange& _Range) threadsafe = 0;
};

interface oFileWriter : oFile
{
	// Write-only binary file. To write in "append" mode, set oFileRange::Offset 
	// to oFILE_APPEND. The underlying implementation does not provide for under-
	// the hood to-text conversion, so the user must be responsible for 
	// constructing the buffer in such a way that it will appropriately be written
	// out. Modern text editors support multiple types of newlines, so it may be a 
	// valid choice to write out '\n' only and just allow for text readers on 
	// Windows platforms to be responsible for careful loading.

	// This function is called when the DispatchWrite is complete and runs in the 
	// IO thread, except if there is an immediate error that prevents the write 
	// from being scheduled, then the callback executes in the calling thread.
	typedef oFUNCTION<void(bool _Success, const void* _pData, const oFileRange& _Range, threadsafe oFileWriter* _pFile)> callback_t;

	// Asynchronously dispatch write. Any failure condition is passed through to 
	// the callback. oGetLastError is valid when the callback's _Success parameter 
	// is false.
	virtual void DispatchWrite(const void* _pData, const oFileRange& _Range, callback_t _Callback) threadsafe = 0;

	// Synchoronous file write, this will not return until _pData has been written
	// to the specified offset in the file.
	virtual bool Write(const void* _pData, const oFileRange& _Range) threadsafe = 0;
};

// _Path must be the full path to the file.
oAPI bool oFileReaderCreate(const char* _Path, threadsafe oFileReader** _ppReadFile);

// _Path must be the full path to the file. If any part of that path does not 
// exist it is created by this function. If that creation fails, this function
// will return false, check oErrorGetLast() for more information.
oAPI bool oFileWriterCreate(const char* _Path, threadsafe oFileWriter** _ppWriteFile);

// Create a subset of a file as another oFileReader.
oAPI bool oFileReaderCreateWindowed(threadsafe oFileReader* _pContainerFileReader, const oFileRange& _Range, threadsafe oFileReader** _ppReadFile);

// _____________________________________________________________________________
// Path-based accessors/mutators on files

// Retrieves a description of the specified file
oAPI bool oFileGetDesc(const char* _Path, oFILE_DESC* _pDesc);

// Returns true if the specified file exists, false if not found
oAPI bool oFileExists(const char* _Path);

// The EnumFunction should return true to continue enummeration, false to 
// short-circuit and end early.
oAPI bool oFileEnum(const char* _WildcardPath, oFUNCTION<bool(const char* _FullPath, const oFILE_DESC& _Desc)> _EnumFunction);

// Modifies the specified file's read-only attribute
oAPI bool oFileMarkReadOnly(const char* _Path, bool _ReadOnly = true);

// Modifies the specified file's hidden attribute
oAPI bool oFileMarkHidden(const char* _Path, bool _Hidden = true);

// Delete works on either files or folders (recursively).
oAPI bool oFileDelete(const char* _Path);

// Creates the entire specified path even if intermediary paths do not exist
oAPI bool oFileCreateFolder(const char* _Path);

// Returns true using the same heuristic as found in Linux
oAPI bool oFileIsText(const char* _Path);
inline bool oFileIsBinary(const char* _Path) { return !oFileIsText(_Path); }

// Updates the specified file's last-modified timestamp
oAPI bool oFileTouch(const char* _Path, time_t _PosixTimestamp);

// Creates a unique folder under the system's temp folder
oAPI char* oFileCreateTempFolder(char* _TempPath, size_t _SizeofTempPath);
template<size_t size> char* oFileCreateTempFolder(char (&_TempPath)[size]) { return oFileCreateTempFolder(_TempPath, size); }

// Replaces platform-specific line endings with '\n'.
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

// Writes the entire buffer to the specified file. Open is specified to allow 
// text and write v. append specification. Any read Open will result in this
// function returning false with oERROR_INVALID_PARAMETER.
oAPI bool oFileSave(const char* _Path, const void* _pSource, size_t _SizeofSource, bool _AsText, bool _AppendToExistingFile = false);

// Memory-maps the specified file, similar to mmap or MapViewOfFile, but with a 
// set of policy constraints to keep its usage simple. Because oFile doesn't
// expose file handles, the specified file is opened and exposed exclusively as 
// the mapped pointer and the file is closed when unmapped. If _ReadOnly is 
// false, the pointer will be mapped write only.
oAPI bool oFileMap(const char* _Path, bool _ReadOnly, const oFileRange& _MapRange, void** _ppMappedMemory);
oAPI bool oFileUnmap(void* _MappedPointer);

// (oBuffer support) Load a file into a newly allocated buffer using malloc to allocate the memory.
bool oBufferCreate(const char* _Path, bool _IsText, interface oBuffer** _ppBuffer);

#endif
