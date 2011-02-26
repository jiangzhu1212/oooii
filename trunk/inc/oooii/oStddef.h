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
#ifndef oStddef_h
#define oStddef_h

#include <oooii/oNoncopyable.h>
#include <functional>

#define oMAJOR_VERSION (0)
#define oMINOR_VERSION (1)
#define oREVISION (0)
#define oCHANGELIST (0)

// Use volatile correctness on methods to indicate the thread safety
// of their calls. This is not the same as the pointless effort of 
// declaring data volatile. Here, a class pointer should be declared
// volatile to disable non-volatile method calls the same way declaring
// a class pointer const disables all methods not labelled const. On
// methods, volatile doesn't have any assembly-level memory access 
// fencing implications - it's just a flag that hopefully can move some
// runtime errors of calling unsafe methods to compile time.
#ifndef threadsafe
	#define threadsafe volatile
#endif

// Create a different type of cast so that when inspecting usages the 
// user can see when it is intended that the code is casting away thread-
// safety.
template<typename T, typename U> inline T thread_cast(const U& threadsafeObject) { return const_cast<T>(threadsafeObject); }

#define oSTRINGIZE__(x) #x
#define oSTRINGIZE(x) oSTRINGIZE__(x)

#define oCOUNTOF(x) (sizeof(x)/sizeof((x)[0]))

#ifdef _MSC_VER
	#pragma warning(disable:4481) // nonstandard extension used: override specifier 'override'
	// warning c4073: initializers put in library initialization area
	// valid values for when are: compiler lib user
	#define	oSTATIC_INIT(when) __pragma(warning(disable:4073)) __pragma(init_seg(when)) __pragma(warning(default:4073))
	#define __LOC__ __FILE__ "("oSTRINGIZE(__LINE__)") : OOOii: "
	#define oSTATIC_WARNING(msg) __pragma(message(__LOC__ msg))
	#define oTHREADLOCAL __declspec(thread)
	#define oRESTRICT __restrict
	#define oFORCEINLINE __forceinline

	// These are intended to be the same as MEMORY_ALLOCATION_ALIGNMENT, but
	// without the Windows header.
	#ifdef _WIN64
		#define oDEFAULT_MEMORY_ALIGNMENT 16
		#define o64BIT 1
	#else
		#define oDEFAULT_MEMORY_ALIGNMENT 8
		#define o32BIT 1
	#endif

#else
	#define override
	#error Unsupported platform (oSTATIC_INIT)
	#error Unsupported platform (oSTATIC_WARNING)
	#error Unsupported platform (oTHREADLOCAL)
	#error Unsupported platform (oRESTRICT)
	#error Unsupported platform (oFORCEINLINE)
	#error Unsupported platform (oDEFAULT_MEMORY_ALIGNMENT)
	#error Unsupported platform (o??BIT)
#endif

#ifdef interface
	#define INTERFACE_DEFINED
#endif

#ifndef INTERFACE_DEFINED
	#ifdef _MSC_VER
		#define interface struct __declspec(novtable)
	#else
		#define interface struct
	#endif
	#define INTERFACE_DEFINED
#endif

#if defined(_DLL) && !defined(oSTATICLIB)
	#ifdef _EXPORT_SYMBOLS
		#define oAPI __declspec(dllexport)
	#else
		#define oAPI __declspec(dllimport)
	#endif
#else
	#define oAPI
#endif

#define oDECLARE_HANDLE(_HandleName) typedef struct _HandleName##__tag {}* _HandleName;
#define oDECLARE_DERIVED_HANDLE(_BaseHandleName, _DerivedHandleName) typedef struct _DerivedHandleName##__tag : public _BaseHandleName##__tag {}* _DerivedHandleName;

// Convenience macro for classes overriding new and delete
#define oDECLARE_NEW_DELETE() \
	void* operator new(size_t size, void* memory) { return memory; } \
	void operator delete(void* p, void* memory) {} \
	void* operator new(size_t size); \
	void* operator new[](size_t size); \
	void operator delete(void* p); \
	void operator delete[](void* p)

#define oSAFESTR(str) ((str) ? (str) : "")
#define oSAFESTRN(str) ((str) ? (str) : "(null)")

#define oKB(n) (n * 1024LL)
#define oMB(n) (oKB(n) * 1024LL)
#define oGB(n) (oMB(n) * 1024LL)
#define oTB(n) (oGB(n) * 1024LL)

// It is often useful to temporarily allocate from the heap. To avoid leaks and
// keep any error handling simple, use this scoped version. For more complete
// buffers that have longer lifetimes, see oBuffer.
class oScopedAllocation : oNoncopyable
{
	void* Data;
	size_t Size;
public:
	oScopedAllocation(size_t _Size) : Data(new char[_Size]), Size(_Size) {}
	~oScopedAllocation() { if (Data) delete Data; }
	inline void* GetData() { return Data; }
	inline const void* GetData() const { return Data; }
	template<typename T> inline T* GetData() { return (T*)Data; }
	template<typename T> inline const T* GetData() const { return (const T*)Data; }
	inline size_t GetSize() const { return Size; }
};

// Extensible pattern for uniform conversion of C++ objects (especially enums) to string. 
// All the user needs to do is define a function of the templated type to add a new type.
template<typename T> const char* oAsString(const T& _Object);

#define oSTACK_ALLOC(size, alignment) oByteAlign(_alloca(oByteAlign(size, alignment)), alignment);

// Constants used throughout the code for asynchronous/time-based operations. Look to 
// comments on an API to understand when it is appropriate to use these.
const static unsigned int oINFINITE_WAIT = ~0u;
const static unsigned int oTIMED_OUT = ~0u;

// #define all the colors out of this so that when it's no longer in the tr1 
// namespace, or if the implementation changes to boost, then we're ready here.
#define oFUNCTION std::tr1::function
#define oBIND std::tr1::bind
#define oBIND1 std::tr1::placeholders::_1
#define oBIND2 std::tr1::placeholders::_2
#define oBIND3 std::tr1::placeholders::_3
#define oBIND4 std::tr1::placeholders::_4
#define oBIND5 std::tr1::placeholders::_5
#define oBIND6 std::tr1::placeholders::_6
#define oBIND7 std::tr1::placeholders::_7
#define oBIND8 std::tr1::placeholders::_8
#define oBIND9 std::tr1::placeholders::_9

// In several utility functions it is useful to see if a file exists (such as 
// going through a search path looking for a file, so define a common signature 
// here.
typedef oFUNCTION<bool(const char* _Path)> oPATH_EXISTS_FUNCTION;

// In several utility functiosn it is useful to load an entire small file into
// a buffer, so define a common signature here.
typedef oFUNCTION<bool(void* _pDestination, size_t _SizeofDestination, const char* _Path)> oLOAD_BUFFER_FUNCTION;

#endif
