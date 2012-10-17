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
// Allocates indices, which can be used for fixed-size pool management. The 
// allocated arena will contain a linked list of free indices, so it must be 
// sized to contain the number of indices desired, so 
// NumIndices * sizeof(unsigned int). THIS CLASS IS NOT THREADSAFE. See 
// oConcurrentIndexAllocator for a threadsafe implementation.
#pragma once
#ifndef oIndexAllocator_h
#define oIndexAllocator_h

#include <oBasis/oPlatformFeatures.h>
#include <oBasis/oThreadsafe.h>

class oIndexAllocatorBase
{
private:
	oIndexAllocatorBase(const oIndexAllocatorBase&);
	oIndexAllocatorBase& operator=(oIndexAllocatorBase&);
protected:
	void* Arena;
	size_t ArenaBytes;
	unsigned int Freelist;
	void InternalReset();
	oIndexAllocatorBase();
	oIndexAllocatorBase(void* _pArena, size_t _SizeofArena); // size in BYTES, not # of indices
	virtual ~oIndexAllocatorBase();
public:
	static const unsigned int InvalidIndex;
	static const size_t SizeOfIndex = sizeof(unsigned int);
	void Initialize(void* _pArena, size_t _SizeofArena);
	void* Deinitialize(); // returns arena as specified in initialize
	void Reset(); // deallocate all indices
	inline bool IsValid() const { return Arena != 0; }
	inline bool IsEmpty() const { return GetSize() == 0; } // (SLOW! see GetSize())
	virtual size_t GetSize() const; // number of indices allocated (SLOW! this loops through entire freelist each call)
	size_t GetCapacity() const threadsafe;
};

class oIndexAllocator : public oIndexAllocatorBase
{
public:
	oIndexAllocator();
	oIndexAllocator(void* _pArena, size_t _SizeofArena); // call deinitialized explicitly to free arena
	virtual ~oIndexAllocator();
	virtual size_t GetSize() const override;
	inline unsigned int Allocate() { unsigned int allocatedIndex = Freelist; Freelist = static_cast<unsigned int*>(Arena)[Freelist]; return allocatedIndex; }
	inline void Deallocate(unsigned int _Index) { static_cast<unsigned int*>(Arena)[_Index] = Freelist; Freelist = _Index; }
};

#endif
