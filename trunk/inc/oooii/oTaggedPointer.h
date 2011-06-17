// $(header)
// A pointer that gives up some of its address space to protect against
// ABA concurrency issues.
#pragma once
#ifndef oTaggedPointer_h
#define oTaggedPointer_h

#include <oooii/oAssert.h>
#include <oooii/oAtomic.h>

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
		oTaggedPointer(void* _Pointer, size_t _Tag) { Assign(_Pointer, _Tag); }
		oTaggedPointer(const threadsafe oTaggedPointer& _Other) : TagAndPointer(_Other.TagAndPointer) {}

		void Assign(const void* _Pointer, size_t _Tag)
		{
			Pointer = (uintptr_t)_Pointer;
			Tag = (uintptr_t)_Tag;
			oASSERT(Pointer == (uintptr_t)_Pointer, "Address space too large to support tagging");
		}

		const oTaggedPointer<T>& operator=(const T* _Pointer) { Assign(_Pointer, 0); return *this; }
		const oTaggedPointer<T>& operator=(const threadsafe oTaggedPointer<T>& _Pointer) { TagAndPointer = _Pointer.TagAndPointer; return *this; }

		bool operator==(const oTaggedPointer<T>& _Other) const { return TagAndPointer == _Other.TagAndPointer; }
		bool operator==(const threadsafe oTaggedPointer<T>& _Other) const { return TagAndPointer == _Other.TagAndPointer; }
		bool operator!=(const oTaggedPointer<T>& _Other) const { return TagAndPointer != _Other.TagAndPointer; }
		bool operator!=(const threadsafe oTaggedPointer<T>& _Other) const { return TagAndPointer != _Other.TagAndPointer; }
		T* operator->() { return (T*)Pointer; }
		const T* operator->() const { return (const T*)Pointer; }
		threadsafe T* operator->() threadsafe { return (threadsafe T*)Pointer; }
		const threadsafe T* operator->() const threadsafe { return (const threadsafe T*)Pointer; }
		operator bool() const { return !!Pointer; }
		operator T*() const { return (T*)Pointer; }
		size_t GetTag() const { return Tag; }
		void SetTag(size_t _Tag) { Tag = _Tag; }
		T* GetPointer() const { return (T*)Pointer; }

		static inline bool CAS(threadsafe oTaggedPointer<T>* Destination, const oTaggedPointer<T>& New, const oTaggedPointer<T>& Old)
		{
			return oCAS(&Destination->TagAndPointer, New.TagAndPointer, Old.TagAndPointer) == Old.TagAndPointer;
		}

protected:
	#pragma warning(disable:4201) // nameless struct/union
	#pragma warning(disable:4408) // anonymous union did not declare any data members
	union
	{
		uintptr_t TagAndPointer;
		struct
		{
			uintptr_t Pointer:NumPointerBits;
			uintptr_t Tag:NumTagBits;
		};
	};
	#pragma warning(default:4408)
	#pragma warning(default:4201)
};

#endif
