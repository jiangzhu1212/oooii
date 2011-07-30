// $(header)
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

#define oCONCAT(x, y) x##y
#define oSTRINGIZE__(x) #x
#define oSTRINGIZE(x) oSTRINGIZE__(x)

#ifdef oENABLE_BUILD_MESSAGES // define this in the compiler command line, mostly for debugging
	#define oBUILD_MSG(msg) __pragma(message(__FILE__ "(" oSTRINGIZE(__LINE__) ") : BUILD: " msg))
#else
	#define oBUILD_MSG(msg)
#endif

#define oCOUNTOF(x) (sizeof(x)/sizeof((x)[0]))

// Use "-1" with unsigned values to indicate a bad/uninitialized state. Be 
// careful with size_t values because oINVALID will result in 0x00000000ffffffff
// on 64-bit systems. Use oINVALID_SIZE_T to ensure all bits are set.
#define oINVALID (~0u)

#ifdef _MSC_VER

	#if _MSC_VER >= 1600
		#define oHAS_RWMUTEX_TRYLOCK
		#define oHAS_NULLPTR
	#else
		#define oCLICKABLE_OUTPUT_REQUIRES_SPACE_COLON
	#endif

	#pragma warning(disable:4481) // nonstandard extension used: override specifier 'override'
	// warning c4073: initializers put in library initialization area
	// valid values for when are: compiler lib user
	#define	oSTATIC_INIT(when) __pragma(warning(disable:4073)) __pragma(init_seg(when)) __pragma(warning(default:4073))
	#define oSTATIC_WARNING(msg) __pragma(message(msg))
	#define oTHREADLOCAL __declspec(thread)
	#define oRESTRICT __restrict
	#define oFORCEINLINE __forceinline

	// These are intended to be the same as MEMORY_ALLOCATION_ALIGNMENT, but
	// without the Windows header.
	#ifdef _WIN64
		#define oDEFAULT_MEMORY_ALIGNMENT 16
		#define oINVALID_SIZE_T (~0ull)
		#define o64BIT 1
	#else
		#define oDEFAULT_MEMORY_ALIGNMENT 8
		#define oINVALID_SIZE_T (~0u)
		#define o32BIT 1
	#endif

#else
	#define override
	#error Unsupported compiler (oCOMPILER_???)
	#error Unsupported platform (oSTATIC_INIT)
	#error Unsupported platform (oSTATIC_WARNING)
	#error Unsupported platform (oTHREADLOCAL)
	#error Unsupported platform (oRESTRICT)
	#error Unsupported platform (oFORCEINLINE)
	#error Unsupported platform (oDEFAULT_MEMORY_ALIGNMENT)
	#error Unsupported platform (oINVALID_SIZE_T)
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

#ifndef oHAS_NULLPTR
	#define nullptr NULL
#endif

#if defined(_DLL) && !defined(oSTATICLIB)
	#ifdef _EXPORT_SYMBOLS
		oBUILD_MSG("oAPI exporting dynamic module symbols")
		#define oAPI __declspec(dllexport)
	#else
		oBUILD_MSG("oAPI importing dynamic module symbols")
		#define oAPI __declspec(dllimport)
	#endif
#else
	oBUILD_MSG("oAPI linking static module symbols")
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
#define oSTRNEXISTS(str) (str && str[0] != '\0')

#define oKB(n) (n * 1024LL)
#define oMB(n) (oKB(n) * 1024LL)
#define oGB(n) (oMB(n) * 1024LL)
#define oTB(n) (oGB(n) * 1024LL)

// Extensible pattern for uniform conversion of C++ objects (especially enums) to string. 
// All the user needs to do is define a function of the templated type to add a new type.
template<typename T> const char* oAsString(const T& _Object);

// Constants used throughout the code for asynchronous/time-based operations. Look to 
// comments on an API to understand when it is appropriate to use these.
const static unsigned int oINFINITE_WAIT = oINVALID;
const static unsigned int oTIMED_OUT = oINVALID;

// #define all the colors out of this so that when it's no longer in the tr1 
// namespace, or if the implementation changes to boost, then we're ready here.
#define oFUNCTION std::tr1::function
#define oBIND std::tr1::bind
template<typename T> std::tr1::reference_wrapper<T> oBINDREF__(T& _Value) { return std::tr1::reference_wrapper<T>(_Value); }
#define oBINDREF oBINDREF__ // #defined so it's the same color as other oBIND elements for Visual Assist, et al.
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

// 128 bit blob
struct uint128_t
{
	unsigned long long data[2];

	bool operator == (const uint128_t& other ) const
	{
		if( other.data[0] != data[0] )
			return false;

		if( other.data[1] != data[1] )
			return false;

		return true;
	}

	bool operator != (const uint128_t& other ) const
	{
		return !((*this) == other);
	}
};

#endif
