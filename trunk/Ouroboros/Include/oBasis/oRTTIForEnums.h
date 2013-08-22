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
#pragma once
#ifndef oRTTIForEnums_h
#define oRTTIForEnums_h

// @oooii-jeff: We could also add support for "simple enums" (enums that don't have sparse values) 
// with perhaps a separate type, so that they can be to-/from-stringed faster (and validated as a bonus too)?

// @oooii-tony: Enums I like to think about:
// Array Enums: Start at zero, has oMY_ENUM_COUNT to use for array alloc/
//              asserting indexed data size. There could be an "invalid" value,
//              but perhaps oMY_ENUM_INVALID == oMY_ENUM_COUNT as a standard.
// BitMask Enums: Enums that are OR'ed together to create an options mask. The
//                challenge is when passed around the type is just an int since
//                oMY_ENUM_1 | oMY_ENUM_2 is not of type oMY_ENUM. Thus there's
//                missing meta-data on the type of a field that RTTI typically
//                uses to read data correctly. Not sure what to do about this.
// Random/Canonical Enum: An actual custom set where each value must be treated
//                        as-is.

struct oRTTI_DATA_ENUM // : oRTTI
{
	struct VALUE
	{
		int Value;
		const char* Name;
	};

	uchar Type;
	uchar Size;
	ushort NumValues;
	const char* TypeName;
	const VALUE* Values;
	bool CaseSensitive;
};

// Declaration macros

#define oRTTI_ENUM_DECLARATION(rtti_add_caps, type_name) \
	extern oRTTI_DATA_ENUM oRTTI_##type_name; \
	rtti_add_caps(DECLARATION, type_name) \
	namespace oStd { \
		inline bool from_string(type_name* _pEnum, const char* _String) { return oRTTI_OF(type_name).FromString(_String, _pEnum); } \
		inline char* to_string(char* _StrDestination, size_t _SizeofDestination, const type_name& _Enum) { return oRTTI_OF(type_name).ToString(_StrDestination, _SizeofDestination, &_Enum); } \
		inline const char* as_string(const type_name& _Enum) { return oRTTI_OF(type_name).AsString(&_Enum); } \
	} 

#define oRTTI_ENUM_DECLARATION_NON_CANONICAL(rtti_add_caps, non_canonical_type_name, type_name) \
	typedef non_canonical_type_name type_name; \
	oRTTI_ENUM_DECLARATION(rtti_add_caps, type_name)

// Description macros

#define oRTTI_ENUM_BEGIN_DESCRIPTION(rtti_add_caps, type_name)									\
	rtti_add_caps(DESCRIPTION, type_name)

#define oRTTI_ENUM_END_DESCRIPTION_COMMON__(type_name, case_sensitive) \
	oRTTI_DATA_ENUM oRTTI_##type_name = { \
		oRTTI_TYPE_ENUM, \
		sizeof(type_name), \
		sRTTINumValues_##type_name, \
		#type_name, \
		sRTTIValues_##type_name, \
		case_sensitive, \
	};

#define oRTTI_ENUM_END_DESCRIPTION(type_name) \
	oRTTI_ENUM_END_DESCRIPTION_COMMON__(type_name, false)

// oRTTI_OF(type_name).FromString will use case-sensitive compares to determine
// enum values.
#define oRTTI_ENUM_END_DESCRIPTION_CASE_SENSITIVE(type_name) \
	oRTTI_ENUM_END_DESCRIPTION_COMMON__(type_name, true)

#define oRTTI_ENUM_BEGIN_VALUES(type_name) \
	static const oRTTI_DATA_ENUM::VALUE sRTTIValues_##type_name[] = {

#define oRTTI_ENUM_END_VALUES(type_name) \
	}; \
	static const int sRTTINumValues_##type_name = sizeof(sRTTIValues_##type_name) / sizeof(oRTTI_DATA_ENUM::VALUE);

#define oRTTI_ENUM_VALIDATE_COUNT(type_name, count) \
	static_assert(sRTTINumValues_##type_name == (size_t)count, "Not all enum values accounted for");

#define oRTTI_ENUM_NO_VALUES(type_name)	\
	static const oRTTI_DATA_ENUM::VALUE* sRTTIValues_##type_name = NULL; \
	static const int sRTTINumValues_##type_name = 0;

#define oRTTI_VALUE(enum_value)						{ enum_value, #enum_value },
#define oRTTI_VALUE_CUSTOM(enum_value, custom_name)	{ enum_value, custom_name },

#endif