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
// NOTE: These declarations and this header exist at a very low level. Generic 
// code and algorithms can leverage thread-based techniques and such usage 
// should be encouraged, so declare these for oBasis generic code, but leave the
// implementation to a platform-level library.
#pragma once
#ifndef oBasisRequirements_h
#define oBasisRequirements_h

#include <oBase/function.h>
#include <oBase/guid.h>

typedef oFUNCTION<void(void* _pMemory)> oLIFETIME_TASK;

// _____________________________________________________________________________
// Platform requirements

// For allocating buffers assigned to a thread_local pointer. Because there can 
// be separate instances of a thread_local in different modules (DLLs), a GUID
// must be specified. This function allocates only once for a given GUID and on
// that once it also calls _Create on the allocation. The implementation should
// also at that time schedule _Destroy as well as the freeing of the specified
// memory using oConcurrency::thread_at_exit.
void oThreadlocalMalloc(const ouro::guid& _GUID, const oLIFETIME_TASK& _Create, const oLIFETIME_TASK& _Destroy, size_t _Size, void** _ppAllocation);
template<typename T> void oThreadlocalMalloc(const ouro::guid& _GUID, const oLIFETIME_TASK& _Create, const oLIFETIME_TASK& _Destroy, T** _ppAllocation) { oThreadlocalMalloc(_GUID, _Create, _Destroy, sizeof(T), (void**)_ppAllocation); }

#endif
