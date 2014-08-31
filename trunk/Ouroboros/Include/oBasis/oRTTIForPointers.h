// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.
#pragma once
#ifndef oRTTIForPointers_h
#define oRTTIForPointers_h

struct oRTTI_DATA_POINTER // : oRTTI
{
	struct INFO
	{
		const char* Name;
		int Size;
		oRTTIConstructor Constructor;
		oRTTIDestructor	Destructor;
		oRTTIPointerGet	Getter;
		oRTTIPointerSet	Setter;
		oRTTIPointerCopy Copier;
	};

	uchar Type;
	const oRTTI* ItemType;
	const INFO* PointerType;
};

// Declaration macros

#define oRTTI_POINTER_TYPE_DECLARATION(pointer_name, referenced_type_name) \
	extern oRTTI_DATA_POINTER oRTTI_##pointer_name##_##referenced_type_name;


// Description macros

#define oRTTI_POINTER_TYPE_DESCRIPTION(pointer_name, referenced_type_name) \
	oRTTI_DATA_POINTER oRTTI_##pointer_name##_##referenced_type_name = { \
	oRTTI_TYPE_POINTER, \
	(const oRTTI*)&oRTTI_##referenced_type_name, \
	&oRTTIPointer_##pointer_name \
};


#endif