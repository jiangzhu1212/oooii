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
// Collection of primitive macros useful in many system-level cases
#pragma once
#ifndef oMacros_h
#define oMacros_h

#include <oBasis/oPlatformFeatures.h>

#define oINTERNAL_STRINGIZE__(x) #x

// Creates a single symbol from the two specified symbols
#define oCONCAT(x, y) x##y

// Safely converts the specified value into a string at pre-processor time
#define oSTRINGIZE(x) oINTERNAL_STRINGIZE__(x)

// Returns the number of elements in a fixed-size array
#define oCOUNTOF(x) (sizeof(x)/sizeof((x)[0]))

// Returns the decltype of a member
#define oDECLTYPE(s, m) decltype(((s*)0)->m)

// Returns the type_info of a member
#define oTYPEID(s, m) typeid(((s*)0)->m)

// Returns the size of a member
#define oSIZEOF(s, m) sizeof(((s*)0)->m)

// Returns the number of bytes from the base of the specified struct for the 
// member variable x.
#ifdef o64BIT
	#define oOFFSETOF(s, m) (uint)((ptrdiff_t)&reinterpret_cast<const volatile char&>((((s *)0)->m)))
#else
	#define oOFFSETOF(s, m) (uint)&reinterpret_cast<const volatile char&>((((s *)0)->m))
#endif

// Shorthand for a simple loop through a fixed array
#define oFORI(_IndexVariable, _Array) for (size_t _IndexVariable = 0; _IndexVariable < oCOUNTOF(_Array); _IndexVariable++)

// This is intended for initialization of relatively small fixed arrays that 
// tend to appear in public API structs. This way we don't have to use a 
// function such as memset in API header code.
#define oINIT_ARRAY(_Array, _Value) do { oFORI(i__, _Array) (_Array)[i__] = (_Value); } while (false)

// For signed (int) values, here is an outrageously negative number. Use this as 
// a special value to indicate 'default'.
#define oDEFAULT 0x80000000

// Make constant sizes more readable and less error-prone as we start specifying
// sizes that require 64-bit storage and thus 64-bit specifiers.
#define oKB(n) (n * 1024LL)
#define oMB(n) (oKB(n) * 1024LL)
#define oGB(n) (oMB(n) * 1024LL)
#define oTB(n) (oGB(n) * 1024LL)

// Wrappers that should be used to protect against null pointers to strings
#define oSAFESTR(str) ((str) ? (str) : "")
#define oSAFESTRN(str) ((str) ? (str) : "(null)")

// It is often used to test for a null or empty string, so encapsulate the 
// pattern in a more self-documenting macro.
#define oSTRVALID(str) ((str) && (str)[0] != '\0')

#define oNEWLINE "\r\n"
#define oWHITESPACE " \t\v\f" oNEWLINE

#define oWNEWLINE L"\r\n"
#define oWWHITESPACE L" \t\v\f" oWNEWLINE

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

// Extensible enum: C++11 uses this pattern especially in <mutex> that makes 
// enum-style solutions more compatible with method overloading and templating
#define oDECLARE_FLAG(_Name) struct _Name##Flag {}; static const _Name##Flag _Name
#define oDEFINE_FLAG(_Namespace, _Name) const _Namespace::_Name##Flag _Namespace::_Name

// Implements a move function that has one field to move
#define oMOVE1(_This, _That, _MoveField) do \
{	if (&(_This) != &(_That)) \
	{	(_This)._MoveField = (_That)._MoveField; \
		(_That)._MoveField = nullptr; \
	} \
} while (false)

#endif
