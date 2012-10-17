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
// Use oStream API for most common I/O operations. In some cases, especially 
// when dealing directly with platform API calls, more direct file access is
// needed, however there is a certain amount of policy and common behavior to 
// install on top of platform calls. To enable this, expose file APIs only to
// oPlatform code and only advertise oStream and advanced oFile API outside of 
// oPlatform implementation.
#pragma once
#ifndef oFileInternal_h
#define oFileInternal_h

#include <oPlatform/oFile.h>

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

bool oFileGetDesc(const char* _Path, oSTREAM_DESC* _pDesc);
bool oFileExists(const char* _Path);
bool oFileOpen(const char* _Path, oFILE_OPEN _Open, oHFILE* _phFile);
bool oFileClose(oHFILE _hFile);
unsigned long long oFileTell(oHFILE _hFile);
bool oFileSeek(oHFILE _hFile, long long _Offset, oFILE_SEEK _Origin = oSEEK_SET);
unsigned long long oFileRead(oHFILE _hFile, void* _pDestination, unsigned long long _SizeofDestination, unsigned long long _ReadSize);
unsigned long long oFileWrite(oHFILE _hFile, const void* _pSource, unsigned long long _WriteSize, bool _Flush = false);
unsigned long long oFileGetSize(oHFILE _hFile);
bool oFileAtEnd(oHFILE _hFile);
bool oFileTouch(oHFILE _hFile, time_t _PosixTimestamp);
bool oFileCopy(const char* _PathFrom, const char* _PathTo, bool _Recursive = true);
bool oFileMove(const char* _PathFrom, const char* _PathTo, bool _Force = false);
bool oFileDelete(const char* _Path);

// Creates the entire specified path even if intermediary paths do not exist
bool oFileCreateFolder(const char* _Path);

#endif
