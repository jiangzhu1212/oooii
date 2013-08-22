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
