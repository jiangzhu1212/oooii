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
#ifndef oRTTIForAtoms_h
#define oRTTIForAtoms_h

#include <oBase/type_info.h>

struct oRTTI_DATA_ATOM // : oRTTI
{
	uchar Type;
	bool IsPlainData;
	bool HasTemplatedSize;
	char NumStringTokens;
	uint Size;
	const char* TypeName;
	const oRTTI* BaseType;
	uint TypeTraits;
	oRTTIFromString FromString;
	oRTTIToString ToString;
	oRTTICopy CopyFunc;
	oRTTIConstructor Constructor;
	oRTTIDestructor Destructor;
	oRTTIGetSerializationSize GetSerializationSize;
	oRTTISerialize Serialize;
	oRTTIDeserialize Deserialize;
};

// Declaration macros

#define oRTTI_ATOM_DECLARATION(rtti_add_caps, atom_type) \
	extern oRTTI_DATA_ATOM oRTTI_##atom_type; \
	rtti_add_caps(DECLARATION, atom_type)

// Atom type has to be canonical, which in our case means that it can't
// have spaces or colons in it, e.g. "Type::SubClass" or "unsigned int"
// So you can use this macro to declare an atom that's non canonical
// for all the other RTTI macros then use the canonical compound type.
// Example: oRTTI_ATOM_DECLARATION_NON_CANONICAL(RTTI_CAPS_NONE, oStd::type, oStd_type)
#define oRTTI_ATOM_DECLARATION_NON_CANONICAL(rtti_add_caps, non_canonical_atom_type, atom_type) \
	typedef non_canonical_atom_type atom_type; \
	oRTTI_ATOM_DECLARATION(rtti_add_caps, atom_type)


// Description macros

#define oRTTI_ATOM_DESCRIPTION(atom_type, atom_type_name, atom_is_plain_data, atom_has_templated_size, atom_base_type, atom_num_string_tokens, atom_fromstring, atom_tostring, atom_copy, atom_construct, atom_destruct, atom_get_serialization_size, atom_serialize, atom_deserialize, rtti_add_caps)	\
	oRTTI_DATA_ATOM oRTTI_##atom_type_name = { \
		oRTTI_TYPE_ATOM, \
		atom_is_plain_data, \
		atom_has_templated_size, \
		atom_num_string_tokens, \
		sizeof(atom_type), \
		#atom_type_name, \
		(const oRTTI*)&oRTTI_##atom_base_type, \
		ouro::type_info<atom_type>::traits, \
		atom_fromstring, \
		atom_tostring, \
		atom_copy, \
		atom_construct, \
		atom_destruct, \
		atom_get_serialization_size, \
		atom_serialize, \
		atom_deserialize, \
	}; \
	rtti_add_caps(DESCRIPTION, atom_type_name)

#define oRTTI_ATOM_DEFAULT_DESCRIPTION(rtti_add_caps, atom_type_name, atom_type, atom_num_string_tokens) \
	typedef bool (*s_##atom_type_name##_FromString)(atom_type *, const char*); \
	typedef char* (*s_##atom_type_name##_ToString)(char*, size_t, const atom_type &); \
	static void s_##atom_type_name##_Copy(atom_type * _pValueDst, const atom_type * _pValueSrc) { *_pValueDst = *_pValueSrc; } \
	static void s_##atom_type_name##_Constructor(const oRTTI& _RTTI, atom_type *_pValue) { *(atom_type *)_pValue = 0; } \
	static int s_##atom_type_name##_GetSerializeSize(const void* _pValue) { return sizeof(atom_type); } \
	static bool s_##atom_type_name##_Serialize(const void* _pValue, void* _pData, bool _EndianSwap) { *(atom_type *)_pData = *(atom_type *)_pValue; return true; } \
	static bool s_##atom_type_name##_Deserialize(const void* _pData, void* _pValue, bool _EndianSwap) { *(atom_type *)_pValue = *(atom_type *)_pData; return true; } \
	oRTTI_ATOM_DESCRIPTION(atom_type, atom_type_name, true, false, atom_type_name, atom_num_string_tokens, \
		(oRTTIFromString)(s_##atom_type_name##_FromString)ouro::from_string, \
		(oRTTIToString)(s_##atom_type_name##_ToString)ouro::to_string, \
		(oRTTICopy)s_##atom_type_name##_Copy, \
		(oRTTIConstructor)s_##atom_type_name##_Constructor, \
		nullptr, \
		s_##atom_type_name##_GetSerializeSize, \
		s_##atom_type_name##_Serialize, \
		s_##atom_type_name##_Deserialize, \
		rtti_add_caps)

#define oRTTI_ATOM_DEFAULT_DESCRIPTION_CONSTRUCTOR(rtti_add_caps, atom_type_name, atom_type, atom_num_string_tokens, ...) \
	typedef bool (*s_##atom_type_name##_FromString)(atom_type *, const char*); \
	typedef char* (*s_##atom_type_name##_ToString)(char*, size_t, const atom_type &); \
	static void s_##atom_type_name##_Copy(atom_type * _pValueDst, const atom_type * _pValueSrc) { *_pValueDst = *_pValueSrc; } \
	static void s_##atom_type_name##_Constructor(const oRTTI& _RTTI, atom_type *_pValue) { new (_pValue) atom_type(__VA_ARGS__); } \
	static int s_##atom_type_name##_GetSerializeSize(const void* _pValue) { return sizeof(atom_type); } \
	static bool s_##atom_type_name##_Serialize(const void* _pValue, void* _pData, bool _EndianSwap) { *(atom_type *)_pData = *(atom_type *)_pValue; return true; } \
	static bool s_##atom_type_name##_Deserialize(const void* _pData, void* _pValue, bool _EndianSwap) { *(atom_type *)_pValue = *(atom_type *)_pData; return true; } \
	oRTTI_ATOM_DESCRIPTION(atom_type, atom_type_name, true, false, atom_type_name, atom_num_string_tokens, \
	(oRTTIFromString)(s_##atom_type_name##_FromString)ouro::from_string, \
	(oRTTIToString)(s_##atom_type_name##_ToString)ouro::to_string, \
	(oRTTICopy)s_##atom_type_name##_Copy, \
	(oRTTIConstructor)s_##atom_type_name##_Constructor, \
	nullptr, \
	s_##atom_type_name##_GetSerializeSize, \
	s_##atom_type_name##_Serialize, \
	s_##atom_type_name##_Deserialize, \
	rtti_add_caps)

#define oRTTI_ATOM_DEFAULT_DESCRIPTION_STRING_OVERRIDE(rtti_add_caps, atom_type_name, atom_type, atom_num_string_tokens, atom_from_string_func, atom_to_string_func) \
	typedef bool (*s_##atom_type_name##_FromString)(atom_type *, const char*); \
	typedef char* (*s_##atom_type_name##_ToString)(char*, size_t, const atom_type &); \
	static void s_##atom_type_name##_Copy(atom_type * _pValueDst, const atom_type * _pValueSrc) { *_pValueDst = *_pValueSrc; } \
	static void s_##atom_type_name##_Constructor(const oRTTI& _RTTI, atom_type *_pValue) { *(atom_type *)_pValue = 0; } \
	static int s_##atom_type_name##_GetSerializeSize(const void* _pValue) { return sizeof(atom_type); } \
	static bool s_##atom_type_name##_Serialize(const void* _pValue, void* _pData, bool _EndianSwap) { *(atom_type *)_pData = *(atom_type *)_pValue; return true; } \
	static bool s_##atom_type_name##_Deserialize(const void* _pData, void* _pValue, bool _EndianSwap) { *(atom_type *)_pValue = *(atom_type *)_pData; return true; } \
	oRTTI_ATOM_DESCRIPTION(atom_type, atom_type_name, true, false, atom_type_name, atom_num_string_tokens, \
	(oRTTIFromString)(s_##atom_type_name##_FromString)atom_from_string_func, \
	(oRTTIToString)(s_##atom_type_name##_ToString)atom_to_string_func, \
	(oRTTICopy)s_##atom_type_name##_Copy, \
	(oRTTIConstructor)s_##atom_type_name##_Constructor, \
	nullptr, \
	s_##atom_type_name##_GetSerializeSize, \
	s_##atom_type_name##_Serialize, \
	s_##atom_type_name##_Deserialize, \
	rtti_add_caps)

#endif