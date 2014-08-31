// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.
#pragma once
#ifndef oJSONSerialize_h
#define oJSONSerialize_h

#include <oBasis/oRTTI.h>
#include <oBase/json.h>

bool oJSONReadCompound(void* _pDestination, const oRTTI& _RTTI, const ouro::json& _JSON, ouro::json::node _Node, bool _FailOnMissingValues);
bool oJSONReadContainer(void* _pDestination, int _DestSizeInBytes, const oRTTI& _RTTI, const ouro::json& _JSON, ouro::json::node _Node, bool _FailOnMissingValues);

bool oJSONWriteCompound(char* _StrDestination, size_t _SizeofStrDestination, const void* _pSource, const oRTTI& _RTTI);
bool oJSONWriteContainer(char* _StrDestination, size_t _SizeofStrDestination, const void* _pSource, int _SourceSize, const oRTTI& _RTTI);

#endif
