// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.
// Collection of primitive macros useful in many system-level cases
#pragma once
#ifndef oBase_macros_h
#define oBase_macros_h

#include <oCompiler.h>
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

// _____________________________________________________________________________
// Runtime macros

// Wrappers that should be used to protect against null pointers to strings
#define oSAFESTR(str) ((str) ? (str) : "")
#define oSAFESTRN(str) ((str) ? (str) : "(null)")

// It is often used to test for a null or empty string, so encapsulate the 
// pattern in a more self-documenting macro.
#define oSTRVALID(str) ((str) && (str)[0] != '\0')

// Helpers for implementing move operators where the eviscerated value is typically zero
#define oMOVE0(field) do { field = _That.field; _That.field = 0; } while (false)
#define oMOVE_ATOMIC0(field) do { field = _That.field.exchange(0); } while (false)

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

#endif
