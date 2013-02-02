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
#ifndef oRTTIForAtoms_h
#define oRTTIForAtoms_h

struct oRTTI_DATA_ATOM // : oRTTI
{
	uchar Type;
	bool IsPlainData;
	ushort Size;
	const char* TypeName;
	const oRTTI* BaseType;
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


// Description macros

#define oRTTI_ATOM_DESCRIPTION(atom_type, atom_type_name, atom_is_plain_data, atom_base_type, atom_fromstring, atom_tostring, atom_copy, atom_construct, atom_destruct, atom_get_serialization_size, atom_serialize, atom_deserialize, rtti_add_caps)	\
	oRTTI_DATA_ATOM oRTTI_##atom_type_name = { \
		oRTTI_TYPE_ATOM, \
		atom_is_plain_data, \
		sizeof(atom_type), \
		#atom_type_name, \
		(const oRTTI*)&oRTTI_##atom_base_type, \
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

#define oRTTI_ATOM_DEFAULT_DESCRIPTION(atom_type_name, atom_type) \
	typedef bool (*s_##atom_type_name##_FromString)(atom_type *, const char*); \
	typedef char* (*s_##atom_type_name##_ToString)(char*, size_t, const atom_type &); \
	static void s_##atom_type_name##_Copy(atom_type * _pValueDst, const atom_type * _pValueSrc) { *_pValueDst = *_pValueSrc; } \
	static void s_##atom_type_name##_Constructor(const oRTTI& _RTTI, atom_type *_pValue) { *(atom_type *)_pValue = 0; } \
	static int s_##atom_type_name##_GetSerializeSize(const void* _pValue) { return sizeof(atom_type); } \
	static bool s_##atom_type_name##_Serialize(const void* _pValue, void* _pData, bool _EndianSwap) { *(atom_type *)_pData = *(atom_type *)_pValue; return true; } \
	static bool s_##atom_type_name##_Deserialize(const void* _pData, void* _pValue, bool _EndianSwap) { *(atom_type *)_pValue = *(atom_type *)_pData; return true; } \
	oRTTI_ATOM_DESCRIPTION(atom_type, atom_type_name, true, atom_type_name, \
		(oRTTIFromString)(s_##atom_type_name##_FromString)oFromString, \
		(oRTTIToString)(s_##atom_type_name##_ToString)oToString, \
		(oRTTICopy)s_##atom_type_name##_Copy, \
		(oRTTIConstructor)s_##atom_type_name##_Constructor, \
		nullptr, \
		s_##atom_type_name##_GetSerializeSize, \
		s_##atom_type_name##_Serialize, \
		s_##atom_type_name##_Deserialize, \
		oRTTI_CAPS_ARRAY)
// FIXME: Should become oRTTI_CAPS_DEFAULT when support for pointers is finished

#endif