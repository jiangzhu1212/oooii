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
// A generic interface to allocating memory that can be passed around to 
// other systems.
#ifndef oBase_allocate_h
#define oBase_allocate_h

#include <utility>

namespace ouro {

/* enum class */ namespace memory_alignment
{	enum value {

	align_to_1,
	align_to_2,
	align_to_4,
	align_to_8,
	align_to_16,
	align_to_32,
	align_to_64,
	align_to_128,
	align_to_256,
	align_to_512,
	align_to_1024,
	align_to_2048,
	align_to_4096,

	count,
	align_to_default = align_to_16,
	align_to_cache_line = align_to_64,

};}

/* enum class */ namespace memory_type
{	enum value {

	default_memory,

	count,

};}

/* enum class */ namespace subsystem
{	enum value {

	ai,
	app,
	cpu,
	input,
	io,
	graphics,
	physics,
	sound,
	ui,

	count,

};}

union allocate_options
{
	unsigned int options;
	struct
	{
		unsigned int alignment : 4; // memory_alignment::value, basically a shift to a power of two for alignment
		unsigned int type : 4; // the type of memory being required. More to come on this...
		unsigned int subsystem : 4; // subsystem::value
		unsigned int usage : 6; // subsystem-specific (thus often user-defined)
		unsigned int extra : 14; // passed through: can be used for user-data
	};
};

typedef void* (*allocate_fn)(size_t _NumBytes, unsigned int _Options);
typedef void (*deallocate_fn)(const void* _Pointer);

class scoped_allocation
{
public:
	scoped_allocation()
		: Pointer(nullptr)
		, Size(0)
		, Deallocate(nullptr)
	{}

	scoped_allocation(void* _Pointer, size_t _Size, deallocate_fn _Deallocate)
		: Pointer(_Pointer)
		, Size(_Size)
		, Deallocate(_Deallocate)
	{}

	scoped_allocation(scoped_allocation&& _That) 
		: Pointer(_That.Pointer)
		, Size(_That.Size)
		, Deallocate(_That.Deallocate)
	{
		_That.Pointer = nullptr;
		_That.Size = 0;
		_That.Deallocate = nullptr;
	}

	~scoped_allocation() { if (Pointer && Deallocate) Deallocate(Pointer); }

	scoped_allocation& operator=(scoped_allocation&& _That)
	{
		if (this != &_That)
		{
			if (Pointer && Deallocate) Deallocate(Pointer);
			Pointer = _That.Pointer; _That.Pointer = nullptr;
			Size = _That.Size; _That.Size = 0;
			Deallocate = _That.Deallocate; _That.Deallocate = nullptr;
		}
		return *this;
	}

	void swap(scoped_allocation& _That)
	{
		std::swap(Pointer, _That.Pointer);
		std::swap(Size, _That.Size);
		std::swap(Deallocate, _That.Deallocate);
	}

	// Don't eviscerate size and deallocate functions so that release() can be passed as a parameter
	// to a function that might also want to take results from size() and get_deallocate()
	void* release() { void* p = Pointer; Pointer = nullptr; /*Size = 0; Deallocate = nullptr;*/ return p; }

	operator bool() const { return !!Pointer; }
	template<typename T> operator T*() const { return static_cast<T*>(Pointer); }
	size_t size() const { return Size; }
	deallocate_fn get_deallocate() { return Deallocate; }

private:
	void* Pointer;
	size_t Size;
	deallocate_fn Deallocate;

	scoped_allocation(const scoped_allocation&);/* = delete; */
	const scoped_allocation& operator=(const scoped_allocation&);/* = delete; */
};

struct allocator
{
	allocator() : allocate(nullptr), deallocate(nullptr) {}
	allocator(allocate_fn _Allocate, deallocate_fn _Deallocate) : allocate(_Allocate), deallocate(_Deallocate) {}
	allocate_fn allocate;
	deallocate_fn deallocate;

	scoped_allocation scoped_allocate(size_t _Size, unsigned int _Options = memory_alignment::align_to_default) const { return scoped_allocation(allocate(_Size, _Options), _Size, deallocate); }

	template<typename T>
	T* construct(unsigned int _Options = memory_alignment::align_to_default) { void* p = allocate(sizeof(T), _Options); return (T*)new (p) T(); }
	
	template<typename T>
	T* construct_array(size_t _Capacity, unsigned int _Options = memory_alignment::align_to_default)
	{
		T* p = (T*)allocate(sizeof(T) * _Capacity, _Options);
		for (size_t i = 0; i < _Capacity; i++)
			new (p + i) T();
		return p;
	}

	template<typename T>
	void destroy(T* p) { if (p) p->~T(); }

	template<typename T>
	void destroy_array(T* p, size_t _Capacity) { if (p) { for (size_t i = 0; i < _Capacity; i++) p[i].~T(); deallocate(p); } }
};

void* default_allocate(size_t _Size, unsigned int _Options);
void default_deallocate(const void* _Pointer);

void* noop_allocate(size_t _Size, unsigned int _Options);
void noop_deallocate(const void* _Pointer);

extern allocator default_allocator;
extern allocator noop_allocator;

} // namespace ouro

#endif
