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
#pragma once
#ifndef oRTTIForEnums_h
#define oRTTIForEnums_h

// @oooii-jeff: We could also add support for "simple enums" (enums that don't have sparse values) 
// with perhaps a separate type, so that they can be to-/from-stringed faster (and validated as a bonus too)?

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
};

// Declaration macros

#define oRTTI_ENUM_DECLARATION(rtti_add_caps, type_name) \
	extern oRTTI_DATA_ENUM oRTTI_##type_name; \
	rtti_add_caps(DECLARATION, type_name) \
	inline bool oFromString(type_name* _pEnum, const char* _String) { return oRTTI_OF(type_name).FromString(_String, _pEnum); } \
	inline char* oToString(char* _StrDestination, size_t _SizeofDestination, const type_name& _Enum) { return oRTTI_OF(type_name).ToString(_StrDestination, _SizeofDestination, &_Enum); }


// Description macros

#define oRTTI_ENUM_BEGIN_DESCRIPTION(rtti_add_caps, type_name)									\
	rtti_add_caps(DESCRIPTION, type_name)

#define oRTTI_ENUM_END_DESCRIPTION(type_name) \
	oRTTI_DATA_ENUM oRTTI_##type_name = { \
		oRTTI_TYPE_ENUM, \
		sizeof(type_name), \
		sRTTINumValues_##type_name, \
		#type_name, \
		sRTTIValues_##type_name, \
	}; \
	oAPI const char* oAsString(const type_name _Enum) { return oRTTI_OF(type_name).AsString(&_Enum); }

#define oRTTI_ENUM_BEGIN_VALUES(type_name) \
	static const oRTTI_DATA_ENUM::VALUE sRTTIValues_##type_name[] = {

#define oRTTI_ENUM_END_VALUES(type_name) \
	}; \
	static const int sRTTINumValues_##type_name = sizeof(sRTTIValues_##type_name) / sizeof(oRTTI_DATA_ENUM::VALUE);

#define oRTTI_ENUM_NO_VALUES(type_name)	\
	static const oRTTI_DATA_ENUM::VALUE* sRTTIValues_##type_name = NULL; \
	static const int sRTTINumValues_##type_name = 0;

#define oRTTI_VALUE(enum_value)						{ enum_value, #enum_value },
#define oRTTI_VALUE_CUSTOM(enum_value, custom_name)	{ enum_value, custom_name },

#endif