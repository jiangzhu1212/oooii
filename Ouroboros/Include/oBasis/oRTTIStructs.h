/**************************************************************************
 * The MIT License                                                        *
 * Copyright (c) 2014 Antony Arciuolo.                                    *
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
#ifndef oRTTIStructs_h
#define oRTTIStructs_h

#include <oBase/byte.h>
#include <oBase/fixed_string.h>

typedef oRTTI_DATA_COMPOUND<oRTTI_OBJECT>::ATTR oRTTI_ATTR_BASE;
struct oRTTI_ATTR : oRTTI_ATTR_BASE
{
	const void* GetSrcPtr(const void* _pCompound) const { return ouro::byte_add(_pCompound, Offset); }
	void* GetDestPtr(void* _pCompound) const { return ouro::byte_add(_pCompound, Offset); }
};

class oRTTI
{
	oRTTI(const oRTTI&)/* = delete*/;
	const oRTTI& operator=(const oRTTI&)/* = delete*/;

public:
	// Type information
	inline oRTTI_TYPE GetType() const { return (oRTTI_TYPE)Type; }

	char* GetName(char* _StrDestination, size_t _SizeofDestination) const;
	template<size_t capacity> inline char* GetName(ouro::fixed_string<char, capacity>& _StrDestination) const { return GetName(_StrDestination.c_str(), _StrDestination.capacity()); }

	int GetSize() const;

	char* TypeToString(char* _StrDestination, size_t _SizeofDestination) const;
	template<size_t capacity> inline char* TypeToString(ouro::fixed_string<char, capacity>& _StrDestination) const { return TypeToString(_StrDestination.c_str(), _StrDestination.capacity()); }

	// General
	bool FromString(const char* _String, void* _pValue, int _SizeOfValue = -1) const;
	char* ToString(char* _StrDestination, size_t _SizeofDestination, const void* _pValue) const;
	template<size_t capacity> inline char* ToString(ouro::fixed_string<char, capacity>& _StrDestination, const void* _pValue) const { return ToString(_StrDestination.c_str(), _StrDestination.capacity(), _pValue); }

	int GetNumStringTokens() const;

	// Atoms
	uint GetTraits() const;

	// Enums
	int GetNumValues() const;
	int GetValue(int _Index) const;
	const char* GetValueName(int _Index) const;
	const char* AsString(const void* _pEnumValue) const;

	// Compounds
	ouro::version GetVersion() const;
	int GetNumBases() const;
	const oRTTI* GetBaseRTTI(int _Index) const;
	int GetBaseOffset(int _Index) const;
	int GetNumAttrs() const;
	const oRTTI_ATTR* GetAttr(int _Index) const;
	void EnumAttrs(bool _IncludeBases, std::function<bool(const oRTTI_ATTR& _Attr)> _Callback) const;

	// Containers
	const oRTTI* GetItemRTTI() const;
	bool IsPlainArray() const;
	int GetItemSize() const;
	void* GetItemPtr(const void* _pContainer, int _ContainerSizeInBytes, int _Index) const;
	int GetItemCount(const void* _pContainer, int _ContainerSizeInBytes) const;
	bool SetItemCount(void* _pContainer, int _ContainerSizeInBytes, int _NewCount, bool _ConstructNewItems = true) const;


	uchar Type;
};

#endif 
