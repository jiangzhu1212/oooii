// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.
#pragma once
#ifndef oRTTITypedefs_h
#define oRTTITypedefs_h

class oRTTI;

typedef void (*oRTTIConstructor)(const oRTTI& _RTTI, void* _pValue);
typedef void (*oRTTIDestructor)(const oRTTI& _RTTI, void* _pValue);

typedef char* (*oRTTIToString)(char* _StrDestination, size_t _SizeofDestination, const void* _pValue);
typedef bool (*oRTTIFromString)(void* _pValue, const char* _StrSource);

typedef void (*oRTTICopy)(void* _pDest, const void* _pSource);
typedef int (*oRTTIGetSerializationSize)(const void* _pData);
typedef bool (*oRTTISerialize)(const void* _pValue, void* _pData, bool _EndianSwap);
typedef bool (*oRTTIDeserialize)(const void* _pData, void* _pValue, bool _EndianSwap);

typedef void* (*oRTTIPointerGet)(void** _ppPointer);
typedef bool (*oRTTIPointerSet)(void** _ppPointer, void* _pValue);
typedef void (*oRTTIPointerCopy)(void** _ppDest, void** _ppSource);

typedef bool (*oRTTIContainerSetItemCount)(const oRTTI& _RTTI, void* _pContainer, int _ContainerSizeInBytes, int _NewSize, bool _ConstructNewItems);
typedef int (*oRTTIContainerGetItemCount)(const oRTTI& _RTTI, const void* _pContainer, int _ContainerSizeInBytes);
typedef void* (*oRTTIContainerGetItemPtr)(const oRTTI& _RTTI, const void* _pContainer, int _ContainerSizeInBytes, int _Index);


#endif
