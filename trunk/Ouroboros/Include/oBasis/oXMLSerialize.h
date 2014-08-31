// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.
#pragma once
#ifndef oXMLSerialize_h
#define oXMLSerialize_h

#include <oBasis/oRTTI.h>
#include <oBase/xml.h>

bool oXMLReadCompound(void* _pDestination, const oRTTI& _RTTI, const ouro::xml& _XML, ouro::xml::node _Node, bool _FailOnMissingValues);
bool oXMLReadContainer(void* _pDestination, int _DestSizeInBytes, const oRTTI& _RTTI, const char* _pElementName, bool _IsRaw, const ouro::xml& _XML, ouro::xml::node _Node, bool _FailOnMissingValues);

#endif
