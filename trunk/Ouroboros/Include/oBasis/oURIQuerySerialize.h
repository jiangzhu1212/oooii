// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.
#pragma once
#ifndef oURIQuerySerialize_h
#define oURIQuerySerialize_h

#include <oBasis/oURI.h>
#include <oBasis/oRTTI.h>

bool oURIQueryReadCompound(void* _pDestination, const oRTTI& _RTTI, const char* _URIQuery, bool _FailOnMissingValues);
bool oURIQueryWriteCompound(char* _StrDestination, size_t _SizeofStrDestination, const void* _pSource, const oRTTI& _RTTI);

#endif
