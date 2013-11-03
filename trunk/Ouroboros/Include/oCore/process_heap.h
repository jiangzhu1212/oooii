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
// This object represents a memory heap that is independent of and persistent 
// across modules loaded into a process. This is useful for implementing objects 
// that require process-wide scope and need to retain their memory independent 
// of dynamic modules that might come and go or cause duplicate global static 
// objects such as singletons.
#pragma once
#ifndef oCore_process_heap_h
#define oCore_process_heap_h

#include <oBase/macros.h>
#include <functional>

namespace ouro {
	namespace process_heap {

enum scope
{
	per_process,
	per_thread,
};

enum tracking
{
	none,
	leak_tracked,
};

// This ensures the process heap has been initialized
void ensure_initialized();

// Allocate memory from the process-global heap directly.
void* allocate(size_t _Size);

// Deallocate memory allocated from allocate() or find_or_allocate(). No 
// matter how many times find_or_allocate evaluates to a pre-existing pointer, 
// this deallocate will free the memory, potentially leaving dangling pointers. 
// It is up to the user to retain control of when the memory actually gets 
// freed. If a destructor was specified in find_or_allocate then it is called 
// before the actual freeing of the memory.
void deallocate(void* _Pointer);

// Call this at the end of a thread to free all thread-local memory.
void exit_thread();

// If the specified named allocation has already been allocated, this will fill 
// _pPointer with that pointer and return false. If the name is unrecognized or 
// in a different scope a new allocation will be made, the specified 
// _PlacementConstructor will be run and this will return true. Use deallocate 
// to free the memory, at which time the specified _Destructor will be run first 
// to finalize the object. A call to exit_thread() will deallocate all 
// allocations for its calling thread, so for per_thread allocations explicit
// calls to deallocate is not necessary if exit_thread is called.
bool find_or_allocate(size_t _Size
	, const char* _Name
	, scope _Scope
	, tracking _Tracking
	, const std::function<void(void* _Pointer)>& _PlacementConstructor
	, const std::function<void(void* _Pointer)>& _Destructor
	, void** _pPointer);

template<typename T>
bool find_or_allocate(
	const char* _Name
	, scope _Scope
	, tracking _Tracking
	, const std::function<void(void* _Pointer)>& _PlacementConstructor
	, const std::function<void(void* _Pointer)>& _Destructor
	, T** _pPointer)
{	return find_or_allocate(sizeof(T), _Name, _Scope, _Tracking, _PlacementConstructor, _Destructor, (void**)_pPointer); }

// Returns true if the specified allocation already exists and *_pPointer is 
// valid and false if the allocation does not exist.
bool find(const char* _Name, scope _Scope, void** _pPointer);

// This should be used for singleton members to ensure allocations are not 
// falsely detected as leaks.
template<typename T> struct std_allocator
{
	// Below we'll use a hash, but we want all its allocations to come out of the
	// specific process heap.

	oDEFINE_STD_ALLOCATOR_BOILERPLATE(std_allocator)
	std_allocator() {}
	template<typename U> std_allocator(std_allocator<U> const& other) {}
	inline pointer allocate(size_type count, const_pointer hint = 0) { return static_cast<pointer>(process_heap::allocate(sizeof(T) * count)); }
	inline void deallocate(pointer p, size_type count) { process_heap::deallocate(p); }
	inline const std_allocator& operator=(const std_allocator& other) {}
};
oDEFINE_STD_ALLOCATOR_VOID_INSTANTIATION(std_allocator)

	} // namespace process_heap
} // namespace ouro

oDEFINE_STD_ALLOCATOR_OPERATOR_EQUAL(ouro::process_heap::std_allocator) { return true; }

#endif
