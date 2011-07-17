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

// This object represents a memory heap that is independent of and persistent 
// across modules loaded into a process. This is useful for implementing objects 
// that require process-wide scope and need to retain their memory independent 
// of dynamic modules that might come and go or cause duplicate global static 
// objects.
#pragma once
#ifndef oProcessHeap_h
#define oProcessHeap_h

#include <oooii/oStddef.h>

namespace oProcessHeap
{
	// Allocate from the process-global heap directly. This memory is page-aligned.
	void* Allocate(size_t _Size);

	// Allocate from the process-global heap using Allocate(), unless this 
	// allocation has already been done by another call in the same process. If 
	// so, the pointer will be filled with that previous allocation, thus allowing
	// multiple modules to always get the same value. If there was an allocation,
	// this returns true. If there was a match, this returns false. Because this 
	// can resolve to a prior-allocated objects, atomicity must be preserved 
	// across not only the raw allocation, but any constructor necessary to create 
	// a valid object. To facilitate this, pass an oFUNCTION that reduces to 
	// calling placement new on the void* parameter. For raw allocations that are
	// valid in and of themselves, pass a noop function.
	bool FindOrAllocate(const char* _Name, size_t _Size, oFUNCTION<void (void* _Pointer)> _PlacementConstructor, void** _pPointer);

	// Deallocate memory allocated from Allocate() or FindOrAllocate(). No matter
	// how many times FindOrAllocate evaluates to a pre-existing pointer, this
	// deallocate will free the memory, potentially leaving dangling pointers, so
	// it is up to the user to retain control of when the memory actually gets 
	// freed. Also remember that this does not call a destructor, so like regular 
	// placement new, the user is expected to handle the call to any destructor
	// explicitly.
	void Deallocate(void* _Pointer);

	void Lock();
	void UnLock();

	struct  ScopedLock : oNoncopyable
	{
		ScopedLock() { Lock(); }
		~ScopedLock() { UnLock(); }
	};
} // namespace oHeap

#endif
