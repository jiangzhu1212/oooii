// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.
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

#define oRTTI_STD_CONTAINED_TYPE_DECLARATION(container_name, contained_type_name, container_type) \
	extern oRTTI_DATA_CONTAINER oRTTI_##container_name##_##contained_type_name; \
	extern oRTTI_DATA_CONTAINER::INFO oRTTIContainer_##container_name##_##contained_type_name;

#define oRTTI_CONTAINER_DECLARATION(container_name) \
	extern oRTTI_DATA_CONTAINER::INFO oRTTIContainer_##container_name;


// Description macros

#define oRTTI_CONTAINED_TYPE_DESCRIPTION(container_name, contained_type_name) \
	oRTTI_DATA_CONTAINER oRTTI_##container_name##_##contained_type_name = { \
		oRTTI_TYPE_CONTAINER, \
		ouro::invalid, \
		(const oRTTI*)&oRTTI_##contained_type_name, \
		&oRTTIContainer_##container_name \
	};

#define oRTTI_CONTAINER_BEGIN_DESCRIPTION(container_name) \
	oRTTI_DATA_CONTAINER::INFO oRTTIContainer_##container_name = { \
		#container_name,

#define oRTTI_CONTAINER_END_DESCRIPTION(container_name) \
	};

template<typename ContainerT> bool oStdContainerSetItemCount(const oRTTI& _RTTI, ContainerT* _pStdContainer, int _ContainerSizeInBytes, int _NewSize, bool _ConstructNewItems)
{
	_pStdContainer->resize(_NewSize);
	return true;
}

template<typename ContainerT> int oStdContainerGetItemCount(const oRTTI& _RTTI, const ContainerT* _pStdContainer, int _ContainerSizeInBytes)
{
	return as_int(_pStdContainer->size());
}

template<typename ContainerT> void* oStdContainerGetItemPtr(const oRTTI& _RTTI, const ContainerT* _pStdContainer, int _ContainerSizeInBytes, int _Index)
{
	ContainerT& Container = *const_cast<ContainerT*>(_pStdContainer);
	return &Container[_Index];
}

#define oRTTI_STD_CONTAINED_TYPE_DESCRIPTION(container_name, contained_type_name, container_type) \
	oRTTI_DATA_CONTAINER::INFO oRTTIContainer_##container_name##_##contained_type_name = { \
		#container_name, \
		sizeof(container_type), \
		false, \
		nullptr, \
		nullptr, \
		(oRTTIContainerSetItemCount)oStdContainerSetItemCount<container_type>, \
		(oRTTIContainerGetItemCount)oStdContainerGetItemCount<container_type>, \
		(oRTTIContainerGetItemPtr)oStdContainerGetItemPtr<container_type>, \
	}; \
	oRTTI_DATA_CONTAINER oRTTI_##container_name##_##contained_type_name = { \
	oRTTI_TYPE_CONTAINER, \
	sizeof(contained_type_name), \
	(const oRTTI*)&oRTTI_##contained_type_name, \
	&oRTTIContainer_##container_name##_##contained_type_name \
};


#endif