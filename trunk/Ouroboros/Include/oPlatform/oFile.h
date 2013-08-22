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
// File-related utilities. Many common features of the file system are 
// exposed through the oStream API and implemented with oFileSchemeHandler. 
// There are many robust functions that file systems support that are not 
// supported by schemes such as http, however there are still required. Such API 
// is exposed here.

// There is often still need of a common policy layer on top of platform API 
// only to be used by platform implementations themselves. For such API, use 
// oFileInternal.h, but that header should not be used outside of oPlatform 
// source.
#pragma once
#ifndef oFile_h
#define oFile_h

#include <oStd/byte.h>
#include <oBasis/oInterface.h>
#include <oStd/function.h>
#include <oBasis/oOBJ.h>
#include <oPlatform/oStream.h>
#include <cstdio>

// _____________________________________________________________________________
// Path-based accessors/mutators on files

// The EnumFunction should return true to continue enummeration, false to 
// short-circuit and end early.
oAPI bool oFileEnum(const char* _WildcardPath, oFUNCTION<bool(const char* _FullPath, const oSTREAM_DESC& _Desc)> _EnumFunction);

// The EnumFunction should return true to continue enummeration, false to 
// short-circuit and end early.  This returns all files recursively under
// the supplied path which must be a directory
oAPI bool oFileEnumFilesRecursively(const char* _Path, oFUNCTION<bool(const char* _FullPath, const oSTREAM_DESC& _Desc)> _EnumFunction);

// Modifies the specified file's read-only attribute
oAPI bool oFileMarkReadOnly(const char* _Path, bool _ReadOnly = true);

// Modifies the specified file's hidden attribute
oAPI bool oFileMarkHidden(const char* _Path, bool _Hidden = true);

// Creates the entire specified path even if intermediary paths do not exist.
// One should generally use oFileEnsureParentFolderExists unless they no for 
// certain the path is a folder
bool oFileCreateFolder(const char* _Path);

// A layer built on top of oFileCreateFolder that ensures the path to the 
// specified file exists before passing the path to a platform save function 
// that fails if any folder in the path hierarchy doesn't exist. If this fails,
// do not continue to whatver MySave(_Path, pMyBuffer) call that would follow.
// Prefer this to oFileCreateFolder because this encapsulates a few extra 
// error checks.
oAPI bool oFileEnsureParentFolderExists(const char* _Path);

// Updates the specified file's last-modified timestamp
oAPI bool oFileTouch(const char* _Path, time_t _UnixTimestamp);

// Creates a unique folder under the system's temp folder
oAPI char* oFileCreateTempFolder(char* _TempPath, size_t _SizeofTempPath);
template<size_t size> char* oFileCreateTempFolder(char (&_TempPath)[size]) { return oFileCreateTempFolder(_TempPath, size); }

// Loads the "header" (known number of bytes at the beginning of a file) into a buffer, returning the actual number of bytes read if the file is smaller than the buffer
oAPI bool oFileLoadHeader(void* _pHeader, size_t _SizeofHeader, const char* _Path);
template<typename T> bool oFileLoadHeader(T* _pHeader, const char* _Path) { return oFileLoadHeader(_pHeader, sizeof(T), _Path); }

// Writes the entire buffer to the specified file. Open is specified to allow 
// text and write v. append specification. Any read Open will result in this
// function returning false with std::errc::invalid_argument.
oAPI bool oFileSave(const char* _Path, const void* _pSource, size_t _SizeofSource, bool _AsText, bool _AppendToExistingFile = false);

// Memory-maps the specified file, similar to mmap or MapViewOfFile, but with a 
// set of policy constraints to keep its usage simple. Because oFile doesn't
// expose file handles, the specified file is opened and exposed exclusively as 
// the mapped pointer and the file is closed when unmapped. If _ReadOnly is 
// false, the pointer will be mapped write only.
oAPI bool oFileMap(const char* _Path, bool _ReadOnly, const oSTREAM_RANGE& _MapRange, void** _ppMappedMemory);
oAPI bool oFileUnmap(void* _MappedPointer);

//useful to quickly deter ming if a file exists or not without opening it. sometimes you may only need to know if the file exists or not without ever opening it.
oAPI bool oFileExists(const char* _Path);

#endif
