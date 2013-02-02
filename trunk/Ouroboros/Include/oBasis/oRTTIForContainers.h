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
#ifndef oRTTIForContainers_h
#define oRTTIForContainers_h

struct oRTTI_DATA_CONTAINER // : oRTTI
{
	struct INFO
	{
		const char* TypeName;
		uint Size;
		bool IsPlainArray;
		oRTTIConstructor Constructor;
		oRTTIDestructor Destructor;
		oRTTIContainerSetItemCount SetItemCount;
		oRTTIContainerGetItemCount GetItemCount;
		oRTTIContainerGetItemPtr GetItemPtr;
	};

	uchar Type;
	uint ItemSize;
	const oRTTI* ItemType;
	const INFO* ContainerType;
};

// Declaration macros

#define oRTTI_CONTAINED_TYPE_DECLARATION(container_name, contained_type_name) \
	extern oRTTI_DATA_CONTAINER oRTTI_##container_name##_##contained_type_name;

#define oRTTI_CONTAINER_DECLARATION(container_name) \
	extern oRTTI_DATA_CONTAINER::INFO oRTTIContainer_##container_name;


// Description macros

#define oRTTI_CONTAINED_TYPE_DESCRIPTION(container_name, contained_type_name) \
	oRTTI_DATA_CONTAINER oRTTI_##container_name##_##contained_type_name = { \
		oRTTI_TYPE_CONTAINER, \
		oInvalid, \
		(const oRTTI*)&oRTTI_##contained_type_name, \
		&oRTTIContainer_##container_name \
	};

#define oRTTI_CONTAINER_BEGIN_DESCRIPTION(container_name) \
	oRTTI_DATA_CONTAINER::INFO oRTTIContainer_##container_name = { \
		#container_name,

#define oRTTI_CONTAINER_END_DESCRIPTION(container_name) \
	};


#endif