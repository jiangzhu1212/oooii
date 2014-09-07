// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.
#pragma once
#ifndef oINISerialize_h
#define oINISerialize_h

#include <oBasis/oRTTI.h>
#include <oString/ini.h>

bool oINIReadCompound(void* _pDestination, const oRTTI& _RTTI, const ouro::ini& _INI, ouro::ini::section _Section, bool _FailOnMissingValues);
bool oINIWriteCompound(char* _StrDestination, size_t _SizeofStrDestination, const void* _pSource, const oRTTI& _RTTI, const char* _Heading);

#endif
