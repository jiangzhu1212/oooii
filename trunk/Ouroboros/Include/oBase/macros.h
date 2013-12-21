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
// Collection of primitive macros useful in many system-level cases
#pragma once
#ifndef oBase_macros_h
#define oBase_macros_h

#include <oBase/config.h>
#include <cstddef>

// _____________________________________________________________________________
// Preprocessor macros

// Creates a single symbol from the two specified symbols
#define oCONCAT(x, y) x##y

// Safely converts the specified value into a string at pre-processor time
#define oINTERNAL_STRINGIZE__(x) #x
#define oSTRINGIZE(x) oINTERNAL_STRINGIZE__(x)

// _____________________________________________________________________________
// Constant/parameter macros

// Returns the number of elements in a fixed-size array
#define oCOUNTOF(x) (sizeof(x)/sizeof((x)[0]))

// For signed (int) values, here is an outrageously negative number. Use this as 
// a special value to indicate 'default'.
#define oDEFAULT 0x80000000

// Make constant sizes more readable and less error-prone as we start specifying
// sizes that require 64-bit storage and thus 64-bit specifiers.
#define oKB(n) (n * 1024LL)
#define oMB(n) (oKB(n) * 1024LL)
#define oGB(n) (oMB(n) * 1024LL)
#define oTB(n) (oGB(n) * 1024LL)

#define oNEWLINE "\r\n"
#define oWHITESPACE " \t\v\f" oNEWLINE

#define oWNEWLINE L"\r\n"
#define oWWHITESPACE L" \t\v\f" oWNEWLINE

#define oDIGIT "0123456789"
#define oDIGIT_UNSIGNED oDIGIT "+"
#define oDIGIT_SIGNED oDIGIT "+-"

// _____________________________________________________________________________
// Runtime macros

// Use this in switch statements where there is no valid default behavior. On 
// some compilers this can generate more efficient code.
#define oNODEFAULT default: oASSUME(0)

// Shorthand for a simple loop through a fixed array
#define oFORI(_IndexVariable, _Array) for (unsigned int _IndexVariable = 0; _IndexVariable < oCOUNTOF(_Array); _IndexVariable++)

// Wrappers that should be used to protect against null pointers to strings
#define oSAFESTR(str) ((str) ? (str) : "")
#define oSAFESTRN(str) ((str) ? (str) : "(null)")

// It is often used to test for a null or empty string, so encapsulate the 
// pattern in a more self-documenting macro.
#define oSTRVALID(str) ((str) && (str)[0] != '\0')

// _____________________________________________________________________________
// Declaration macros

// Convenience macro for classes overriding new and delete
#define oDECLARE_NEW_DELETE() \
	void* operator new(size_t size, void* memory) { return memory; } \
	void operator delete(void* p, void* memory) {} \
	void* operator new(size_t size); \
	void* operator new[](size_t size); \
	void operator delete(void* p); \
	void operator delete[](void* p)

// Encapsulate the pattern of declaring typed handles by defining a typed pointer
#define oDECLARE_HANDLE(_HandleName) typedef struct _HandleName##__tag {}* _HandleName;
#define oDECLARE_DERIVED_HANDLE(_BaseHandleName, _DerivedHandleName) typedef struct _DerivedHandleName##__tag : public _BaseHandleName##__tag {}* _DerivedHandleName;

// Declare an enum whose size will be that of the specified type.
#define oDECLARE_SMALL_ENUM(_Name, _Type) __pragma(warning(disable:4480)) enum _Name : _Type __pragma(warning(default:4480))

// _____________________________________________________________________________
// std::allocator boilerplate code helpers

#define oDEFINE_STD_ALLOCATOR_BOILERPLATE(ClassName) \
	typedef size_t size_type; typedef ptrdiff_t difference_type; typedef T value_type; typedef T* pointer; typedef T const* const_pointer; typedef T& reference; typedef T const& const_reference; \
	template<typename U> struct rebind { typedef ClassName<U> other; }; \
	pointer address(reference p) const { return &p; } \
	const_pointer address(const_reference p) const { return &p; } \
	size_type max_size() const { return static_cast<size_type>(~0) / sizeof(value_type); } \
	void construct(pointer p) { ::new (static_cast<void*>(p)) T(); } \
	void construct(pointer p, const T& val) { ::new (static_cast<void*>(p)) T(val); } \
	void destroy(pointer p) { p->~T(); }

#define oDEFINE_STD_ALLOCATOR_VOID_INSTANTIATION(ClassName) \
	template <> class ClassName<void> \
	{ public: typedef void* pointer; typedef const void* const_pointer; typedef void value_type; \
	template <class U> struct rebind { typedef ClassName<U> other; }; \
	};

#define oDEFINE_STD_ALLOCATOR_OPERATOR_EQUAL(ClassName) \
	template<typename T, typename U> bool operator!=(ClassName<T> const& a, ClassName<U> const& b) { return !(a == b); } \
	template<typename T, typename U> bool operator==(ClassName<T> const& a, ClassName<U> const& b)

// As an example of this boilerplate usage, and as a semi-useful class itself,
// here is a std::allocator that takes user functions of the same signature as 
// malloc and free to do allocations in STL containers. Although this is 
// somewhat naughty in that state (the two function pointers) is retained, all 
// common STL implementations pass things through, and all copy and equality 
// semantics are enforced, so this works properly in all but the most contrived 
// cases.

template<typename T> struct std_user_allocator
{
	oDEFINE_STD_ALLOCATOR_BOILERPLATE(std_user_allocator)
	std_user_allocator() : UserAllocate(malloc), UserDeallocate(free) {}
	std_user_allocator(void* (*alloc)(size_t size), void (*dealloc)(void* p)) : UserAllocate(alloc), UserDeallocate(dealloc) {}
	template<typename U> std_user_allocator(std_user_allocator<U> const& other) : UserAllocate(other.UserAllocate), UserDeallocate(other.UserDeallocate) {}
	inline pointer allocate(size_type count, const_pointer hint = 0) { return static_cast<pointer>(UserAllocate(sizeof(T) * count)); }
	inline void deallocate(pointer p, size_type count) { UserDeallocate(p); }
	inline const std_user_allocator& operator=(const std_user_allocator& other) { UserAllocate = other.UserAllocate; UserDeallocate = other.UserDeallocate; return *this; }
	void* (*UserAllocate)(size_t size);
	void (*UserDeallocate)(void* p);
};

oDEFINE_STD_ALLOCATOR_VOID_INSTANTIATION(std_user_allocator)
oDEFINE_STD_ALLOCATOR_OPERATOR_EQUAL(std_user_allocator) { return a.UserAllocate == b.UserAllocate && a.UserDeallocate == b.UserDeallocate; }

#endif
