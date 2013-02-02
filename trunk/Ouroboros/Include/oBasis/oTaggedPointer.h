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
// A pointer that gives up some of its address space to protect against ABA 
// concurrency issues.
#pragma once
#ifndef oTaggedPointer_h
#define oTaggedPointer_h

#include <oBasis/oAssert.h>
#include <oBasis/oStdAtomic.h>

template<
	typename T, 
	size_t TagBits = 
	// 32-bit Address space does not allow for much room. IMHO, stop using 32-bit. 
	// If you see race conditions using structures based on oTaggedPointer, this 
	// could very well be why. But if you have a race condition only when 16 
	// simultaneous writes are taking place, let me know because I'd love to see 
	// your system! (and know why you're using 32-bit for it!)
	#ifdef o32BIT
		4
	#else 
		8
	#endif
>

class oTaggedPointer
{
public:
	static const size_t NumTagBits = TagBits;
	static const size_t NumPointerBits = (8*sizeof(void*))-TagBits;

	oTaggedPointer() : TagAndPointer(0) {}
	oTaggedPointer(void* _Pointer, size_t _Tag)
	{
		Pointer = (uintptr_t)_Pointer;
		Tag = (uintptr_t)_Tag;
		oASSERT(Pointer == (uintptr_t)_Pointer, "Address space too large to support tagging");
	}
		
	oTaggedPointer(const threadsafe oTaggedPointer& _That) : TagAndPointer(_That.TagAndPointer) {}
	const oTaggedPointer<T>& operator=(const oTaggedPointer<T>& _That) { TagAndPointer = _That.TagAndPointer; return *this; }
	bool operator==(const oTaggedPointer<T>& _That) const { return TagAndPointer == _That.TagAndPointer; }
	bool operator==(const threadsafe oTaggedPointer<T>& _That) const { return TagAndPointer == _That.TagAndPointer; }
	bool operator!=(const oTaggedPointer<T>& _That) const { return TagAndPointer != _That.TagAndPointer; }
	bool operator!=(const threadsafe oTaggedPointer<T>& _That) const { return TagAndPointer != _That.TagAndPointer; }
	size_t tag() const { return Tag; }
	T* ptr() const { return (T*)Pointer; }
	static inline bool CAS(threadsafe oTaggedPointer<T>* Destination, const oTaggedPointer<T>& New, const oTaggedPointer<T>& Old)
	{
		return oStd::atomic_compare_exchange(&Destination->TagAndPointer, New.TagAndPointer, Old.TagAndPointer);
	}

protected:
	union
	{
		uintptr_t TagAndPointer;
		struct
		{
			uintptr_t Pointer:NumPointerBits;
			uintptr_t Tag:NumTagBits;
		};
	};
};

#endif
