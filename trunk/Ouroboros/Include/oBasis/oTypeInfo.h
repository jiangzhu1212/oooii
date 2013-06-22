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
// Extension to C++'s type_info that includes basic, generic built-in 
// construction. Note that vtable() can be somewhat expensive since it must call
// the default constructor to have C++ populate the vtable correctly. If the 
// default constructor is complex, then that call will reflect such complexity.
// If the destructor is not properly implemented, then resource issues could 
// arise too. Use with care.
#pragma once
#ifndef oTypeInfo_h
#define oTypeInfo_h

#include <oBasis/oHash.h>
#include <oBasis/oMathTypes.h>
#include <oBasis/oTypeID.h>
#include <oStd/stringize.h>
#include <type_traits>

template<typename T> struct remove_traits
{
	typedef 
		typename std::remove_pointer<
		typename std::remove_reference<
		typename std::remove_all_extents<
		typename std::remove_cv<T>::type>::type>::type>::type type;
};

enum oRUNTIME_TYPE_TRAITS
{
	oTRAIT_IS_VOID = 1<<0,
	oTRAIT_IS_INTEGRAL = 1<<1,
	oTRAIT_IS_FLOATING_POINT = 1<<2,
	oTRAIT_IS_ARRAY = 1<<3,
	oTRAIT_IS_POINTER = 1<<4,
	oTRAIT_IS_REFERENCE = 1<<5,
	oTRAIT_IS_MEMBER_OBJECT_POINTER = 1<<6,
	oTRAIT_IS_MEMBER_FUNCTION_POINTER = 1<<7,
	oTRAIT_IS_ENUM = 1<<8,
	oTRAIT_IS_UNION = 1<<9,
	oTRAIT_IS_CLASS = 1<<10,
	oTRAIT_IS_FUNCTION = 1<<11,
	oTRAIT_IS_ARITHMETIC = 1<<12,
	oTRAIT_IS_FUNDAMENTAL = 1<<13,
	oTRAIT_IS_OBJECT = 1<<14,
	oTRAIT_IS_SCALAR = 1<<15,
	oTRAIT_IS_COMPOUND = 1<<16,
	oTRAIT_IS_MEMBER_POINTER = 1<<17,
	oTRAIT_IS_CONST = 1<<18,
	oTRAIT_IS_VOLATILE = 1<<19,
	oTRAIT_IS_POD = 1<<20,
	oTRAIT_IS_EMPTY = 1<<21,
	oTRAIT_IS_POLYMORPHIC = 1<<22,
	oTRAIT_IS_ABSTRACT = 1<<23,
	oTRAIT_HAS_TRIVIAL_CONSTRUCTOR = 1<<24,
	oTRAIT_HAS_TRIVIAL_COPY = 1<<25,
	oTRAIT_HAS_TRIVIAL_ASSIGN = 1<<26,
	oTRAIT_HAS_TRIVIAL_DESTRUCTOR = 1<<27,
	oTRAIT_HAS_VIRTUAL_DESTRUCTOR = 1<<28,
	oTRAIT_IS_SIGNED = 1<<29,
	oTRAIT_IS_UNSIGNED = 1<<30,
	oTRAIT_IS_LINEAR_ALGEBRA = 1<<31,
};

typedef void (*type_info_default_constructor)(void* instance);
typedef void (*type_info_copy_constructor)(void* instance, const void* source);
typedef void (*type_info_destructor)(void* instance);

template<typename T, typename U> struct sizeof_void
{
	static const size_t size = sizeof(U);
	static const size_t element_size = sizeof(std::remove_extent<U>::type);
	static const size_t num_elements = sizeof(U) / sizeof(std::remove_extent<U>::type);
};

template<typename U> struct sizeof_void<std::true_type, U>
{
	static const size_t size = 0;
	static const size_t element_size = 0;
	static const size_t num_elements = 0;
};

static inline const char* move_past_whitespace(const char* s)
{
	const char* ss = s;
	while (*ss && *ss++ != ' ');
	return *ss ? ss : s;
}

template<typename T> struct oTypeInfo
{
	typedef T type;

	// returns the regular RTTI for this type
	static const type_info& typeinfo() { return typeid(T); }

	// returns the regular RTTI name for this type
	static const char* name() { return typeid(T).name(); }

	// returns the RTTI name without "union ", "class " or "enum " prefixes.
	static const char* simple_name()
	{
		const char* TypeInfoName = name();
		if (std::is_union<T>::value) TypeInfoName += 5;
		if (std::is_class<T>::value) TypeInfoName += 5;
		if (std::is_enum<T>::value) TypeInfoName += 4;
		return move_past_whitespace(TypeInfoName);
	}

	// total size of the type
	static const size_t size = sizeof_void<std::integral_constant<bool, std::is_void<std::remove_cv<T>::type>::value>, T>::size;

	// if the type is a fixed array, return the size of one element of that array
	static const size_t element_size = sizeof_void<std::integral_constant<bool, std::is_void<std::remove_cv<T>::type>::value>, T>::element_size;
		
	// if the type is a fixed array, return the number of elements in that array
	static const size_t num_elements = sizeof_void<std::integral_constant<bool, std::is_void<std::remove_cv<T>::type>::value>, T>::num_elements;

	// creates a value that is either an oTYPE_ID or a hash of the RTTI name for
	// enums, classes, and unions
	static unsigned int make_id() { return is_type_id<T>::value ? oTypeID<T>::value : oHash_FNV1a(simple_name()); }

	// returns the vtable pointer for the specified class (nullptr if the class is 
	// not polymorphic)
	static const void* vtable()
	{
		const void* vtbl = nullptr;
		if (std::is_polymorphic<T>::value)
		{
			struct vtable { void* pointer; };
			T instance; // default construct
			#ifdef _MSC_VER
				vtbl = *(void**)((vtable*)&instance)->pointer;
			#else
				#error Unsupported platform
			#endif 
		}
		return vtbl;
	}

	// calls this type's default constructor on the specified memory (similar to
	// placement new)
	static void default_construct(void* _pInstance)
	{
		#pragma warning(disable:4345) // behavior change: an object of POD type constructed with an initializer of the form () 
		::new (_pInstance) T();
		#pragma warning(default:4345)
	}

	// calls this type's copy constructor copying from _pSource to _pInstance
	static void copy_construct(void* _pInstance, const void* _pSource) { return _pSource ? ::new (_pInstance) T(*static_cast<T*>(const_cast<void*>(_pSource))) : default_construct(_pInstance); }

	// calls this type's destructor
	static void destroy(void* _pInstance) { static_cast<T*>(_pInstance)->~T(); }

	// store traits for runtime access
	static const unsigned int traits = 
		((std::is_void<T>::value&0x1)<<0)|
		((std::is_integral<T>::value&0x1)<<1)|
		((std::is_floating_point<T>::value&0x1)<<2)|
		((std::is_array<T>::value&0x1)<<3)|
		((std::is_pointer<T>::value&0x1)<<4)|
		((std::is_reference<T>::value&0x1)<<5)|
		((std::is_member_object_pointer<T>::value&0x1)<<6)|
		((std::is_member_function_pointer<T>::value&0x1)<<7)|
		((std::is_enum<T>::value&0x1)<<8)|
		((std::is_union<T>::value&0x1)<<9)|
		((std::is_class<T>::value&0x1)<<10)|
		((std::is_function<T>::value&0x1)<<11)|
		((std::is_arithmetic<T>::value&0x1)<<12)|
		((std::is_fundamental<T>::value&0x1)<<13)|
		((std::is_object<T>::value&0x1)<<14)|
		((std::is_scalar<T>::value&0x1)<<15)|
		((std::is_compound<T>::value&0x1)<<16)|
		((std::is_member_pointer<T>::value&0x1)<<17)|
		((std::is_const<T>::value&0x1)<<18)|
		((std::is_volatile<T>::value&0x1)<<19)|
		((std::is_pod<T>::value&0x1)<<20)|
		((std::is_empty<T>::value&0x1)<<21)|
		((std::is_polymorphic<T>::value&0x1)<<22)|
		((std::is_abstract<T>::value&0x1)<<23)|
		((std::has_trivial_constructor<T>::value&0x1)<<24)|
		((std::has_trivial_copy<T>::value&0x1)<<25)|
		((std::has_trivial_assign<T>::value&0x1)<<26)|
		((std::has_trivial_destructor<T>::value&0x1)<<27)|
		((std::has_virtual_destructor<T>::value&0x1)<<28)|
		((std::is_signed<T>::value&0x1)<<29)|
		((std::is_unsigned<T>::value&0x1)<<30)|
		((is_linear_algebra<T>::value&0x1u)<<31u);
};

// same thing as above, but based off parsing the type for situations where 
// the template is not available.
inline unsigned int oTypeInfoMakeID(const char* _TypeInfoName) 
{
	unsigned int TypeID = oTYPE_UNKNOWN;
	if (!oStd::from_string((oTYPE_ID*)&TypeID, _TypeInfoName))
		TypeID = oHash_FNV1a(move_past_whitespace(_TypeInfoName));
	return TypeID;
}

struct oTYPE_DESC
{
	template<typename T> oTYPE_DESC(T* _pTypedDummyPointer)
		: TypeName(oTypeInfo<T>::name())
		, TypeID(oTypeInfo<T>::make_id())
		, Traits(oTypeInfo<T>::traits)
		, Size(static_cast<unsigned int>(oTypeInfo<T>::size))
		, VTable(oTypeInfo<T>::vtable())
		, DefaultConstructor(&oTypeInfo<T>::default_construct)
		, CopyConstructor(&oTypeInfo<T>::copy_construct)
		, Destructor(&oTypeInfo<T>::destroy)
	{}

	const char* TypeName;
	unsigned int TypeID;
	unsigned int Traits;
	unsigned int Size;
	const void* VTable;
	type_info_default_constructor DefaultConstructor;
	type_info_copy_constructor CopyConstructor;
	type_info_destructor Destructor;
};

#endif
