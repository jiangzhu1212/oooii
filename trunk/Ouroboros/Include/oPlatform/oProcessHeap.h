/**************************************************************************
 * The MIT License                                                        *
 * Copyright (c) 2013 OOOii.                                              *
 * antony.arciuolo@oooii.com                                              *
 * kevin.myers@oooii.com                                                  *
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

#include <oBasis/oStddef.h>
#include <oBasis/oStdAllocator.h>

// Allocate from the process-global heap directly. This memory is page-aligned.
void* oProcessHeapAllocate(size_t _Size);

// Allocate from the process-global heap using Allocate(), unless this 
// allocation has already been done by another call in the same process. If 
// so, the pointer will be filled with that previous allocation thus allowing
// multiple modules to always get the same value. If there was an allocation,
// this returns true. If there was a match, this returns false. Because this 
// can resolve to a prior-allocated objects, atomicity must be preserved 
// across not only the raw allocation, but any constructor necessary to create 
// a valid object. To facilitate this, pass an oFUNCTION that reduces to 
// calling placement new on the void* parameter. For raw allocations that are
// valid in and of themselves, pass a noop function.
bool oProcessHeapFindOrAllocate(const oGUID& _AllocationID, bool _IsThreadLocal, bool _IsLeakTracked, size_t _Size, oFUNCTION<void (void* _Pointer)> _PlacementConstructor, const char* _DebugName, void** _pPointer);

// Returns true if the specified allocation already exists and *_pPointer is 
// valid and false if the allocation does not exist.
bool oProcessHeapFind(const oGUID& _AllocationID, bool _IsThreadLocal, void** _pPointer);

// Deallocate memory allocated from Allocate() or FindOrAllocate(). No matter
// how many times FindOrAllocate evaluates to a pre-existing pointer, this
// deallocate will free the memory, potentially leaving dangling pointers, so
// it is up to the user to retain control of when the memory actually gets 
// freed. Also remember that this does not call a destructor, so like regular 
// placement new, the user is expected to handle the call to any destructor
// explicitly.
void oProcessHeapDeallocate(void* _Pointer);

// oProcessHeap is guaranteed to be running once this function returns
void oProcessHeapEnsureRunning();

// This should be used for singleton members to ensure allocations are not 
// falsely detected as leaks.
template<typename T> struct oProcessHeapAllocator
{
	// Below we'll use a hash, but we want all its allocations to come out of the
	// specific process heap.

	oDEFINE_STD_ALLOCATOR_BOILERPLATE(oProcessHeapAllocator)
	oProcessHeapAllocator() {}
	template<typename U> oProcessHeapAllocator(oProcessHeapAllocator<U> const& other) {}
	inline pointer allocate(size_type count, const_pointer hint = 0) { return static_cast<pointer>(oProcessHeapAllocate(sizeof(T) * count)); }
	inline void deallocate(pointer p, size_type count) { oProcessHeapDeallocate(p); }
	inline const oProcessHeapAllocator& operator=(const oProcessHeapAllocator& other) {}
};

oDEFINE_STD_ALLOCATOR_VOID_INSTANTIATION(oProcessHeapAllocator)
oDEFINE_STD_ALLOCATOR_OPERATOR_EQUAL(oProcessHeapAllocator) { return true; }

#endif
