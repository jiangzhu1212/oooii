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
// Ouroboros encapsulates file handles so that usage options are limited only to 
// the most performant and threadsafe code paths. However some interfaces still
// require the fopen-style API, so we're not going to delete it, but hide it 
// here.
// REALLY REALLY TRY HARD NOT TO USE THESE API. If oFileReader/oFileWriter do
// not do what you want, discuss the issue with someone.
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

bool oFileOpen(const char* _Path, oFILE_OPEN _Open, oHFILE* _phFile);
bool oFileClose(oHFILE _hFile);
unsigned long long oFileTell(oHFILE _hFile);
bool oFileSeek(oHFILE _hFile, long long _Offset, oFILE_SEEK _Origin = oSEEK_SET);
unsigned long long oFileRead(oHFILE _hFile, void* _pDestination, unsigned long long _SizeofDestination, unsigned long long _ReadSize);
unsigned long long oFileWrite(oHFILE _hFile, const void* _pSource, unsigned long long _WriteSize, bool _Flush = false);
unsigned long long oFileGetSize(oHFILE _hFile);
bool oFileAtEnd(oHFILE _hFile);
bool oFileTouch(oHFILE _hFile, time_t _PosixTimestamp);
bool oFileIsText(oHFILE _hFile);
inline bool oFileIsBinary(oHFILE _hFile) { return !oFileIsText(_hFile); }

#endif
