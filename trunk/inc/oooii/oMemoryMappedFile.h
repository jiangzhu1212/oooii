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
#pragma once
#ifndef oMemoryMappedFile_h
#define oMemoryMappedFile_h

#include <oooii/oInterface.h>
#include <memory.h>

//Currently only read only memory mapped files are supported. Any attempt to write
//	will throw an access violation. Also only one view can be mapped at a time. a second
//	call to MapView without unmapping the first will block until the first is unmapped.
//	these limitations could be fixed if needed.
interface oMemoryMappedFile : oInterface
{
	static bool Create(const char* _Path, threadsafe oMemoryMappedFile** _ppMappedFile);

	virtual unsigned long long GetFileSize() const threadsafe = 0;

	// If _Size is zero the entire file is mapped from _Offset to the end of the file
	virtual void* Map(unsigned long long _Offset, unsigned int _Size) threadsafe = 0;
	virtual void Unmap() threadsafe = 0;
};

#endif
