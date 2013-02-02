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
#include <oBasis/oRTTI.h>
#include <oBasis/oError.h>
#include <oBasis/oFixedString.h>
#include <oBasis/oArray.h>
#include <vector>

// TODO: Fix and move Array descriptions
bool oArraySetItemCount(const oRTTI& _RTTI, void* _pContainer, int _ContainerSizeInBytes, int _NewSize, bool _ConstructNewItems)
{
	int sizeOffset = _ContainerSizeInBytes - sizeof(size_t);
	if (_RTTI.GetSize() * _NewSize > sizeOffset) return false;
	*(size_t*)oByteAdd(_pContainer, sizeOffset) = _NewSize;
	return true;
}
int oArrayGetItemCount(const oRTTI& _RTTI, const void* _pContainer, int _ContainerSizeInBytes)
{
	int sizeOffset = _ContainerSizeInBytes - sizeof(size_t);
	return (int)*(size_t*)oByteAdd(_pContainer, sizeOffset);
}
void* oArrayGetItemPtr(const oRTTI& _RTTI, const void* _pContainer, int _ContainerSizeInBytes, int _Index)
{
	int offset = _RTTI.GetSize() * _Index;
	void* result = oByteAdd(const_cast<void*>(_pContainer), offset);
	return offset < (_ContainerSizeInBytes - (int)sizeof(size_t)) ? result : nullptr;
}
oRTTI_CONTAINER_BEGIN_DESCRIPTION(oArray)
	sizeof(oArray<void*,0>),
	true,
	nullptr,
	nullptr,
	oArraySetItemCount,
	oArrayGetItemCount,
	oArrayGetItemPtr
oRTTI_CONTAINER_END_DESCRIPTION(oArray)

// Container declaration for regular c-style arrays like int[20]
bool cArraySetItemCount(const oRTTI& _RTTI, void* _pContainer, int _ContainerSizeInBytes, int _NewSize, bool _ConstructNewItems)
{
	return (_RTTI.GetSize() * _NewSize < _ContainerSizeInBytes);
}
int cArrayGetItemCount(const oRTTI& _RTTI, const void* _pContainer, int _ContainerSizeInBytes)
{
	return _ContainerSizeInBytes / _RTTI.GetSize();
}
void* cArrayGetItemPtr(const oRTTI& _RTTI, const void* _pContainer, int _ContainerSizeInBytes, int _Index)
{
	int offset = _RTTI.GetSize() * _Index;
	void* result = oByteAdd(const_cast<void*>(_pContainer), offset);
	return offset < _ContainerSizeInBytes ? result : nullptr;
}
oRTTI_CONTAINER_BEGIN_DESCRIPTION(c_array)
	0,
	true,
	nullptr,
	nullptr,
	cArraySetItemCount,
	cArrayGetItemCount,
	cArrayGetItemPtr
oRTTI_CONTAINER_END_DESCRIPTION(c_array)

// Container declaration for regular c-style arrays like int[20]
bool StdVectorSetItemCount(const oRTTI& _RTTI, void* _pContainer, int _ContainerSizeInBytes, int _NewSize, bool _ConstructNewItems)
{
	const std::vector<void*>& Container = *(const std::vector<void*>*)_pContainer;
	size_t Capacity = Container.capacity() * sizeof(void*);
	return ((size_t)_RTTI.GetSize() * _NewSize < Capacity);
}
int StdVectorGetItemCount(const oRTTI& _RTTI, const void* _pContainer, int _ContainerSizeInBytes)
{
	const std::vector<void*>& Container = *(const std::vector<void*>*)_pContainer;
	size_t Capacity = Container.capacity() * sizeof(void*);
	return (int)Capacity / _RTTI.GetSize();
}
void* StdVectorGetItemPtr(const oRTTI& _RTTI, const void* _pContainer, int _ContainerSizeInBytes, int _Index)
{
	const std::vector<void*>& Container = *(const std::vector<void*>*)_pContainer;
	size_t Capacity = Container.capacity() * sizeof(void*);
	size_t offset = (size_t)_RTTI.GetSize() * _Index;
	void* result = oByteAdd((void*)(&Container[0]), offset);
	return offset < Capacity ? result : nullptr;
}
oRTTI_CONTAINER_BEGIN_DESCRIPTION(std_vector)
	sizeof(std::vector<void*>),
	true,
	nullptr,
	nullptr,
	StdVectorSetItemCount,
	StdVectorGetItemCount,
	StdVectorGetItemPtr
oRTTI_CONTAINER_END_DESCRIPTION(std_vector)


// TODO: Expand atom type support and move to appropriate place
oRTTI_ATOM_DEFAULT_DESCRIPTION(bool, bool)
oRTTI_ATOM_DEFAULT_DESCRIPTION(char, char)
oRTTI_ATOM_DEFAULT_DESCRIPTION(uchar, unsigned char)
oRTTI_ATOM_DEFAULT_DESCRIPTION(short, short)
oRTTI_ATOM_DEFAULT_DESCRIPTION(ushort, unsigned short)
oRTTI_ATOM_DEFAULT_DESCRIPTION(int, int)
oRTTI_ATOM_DEFAULT_DESCRIPTION(uint, unsigned int)
oRTTI_ATOM_DEFAULT_DESCRIPTION(llong, long long)
oRTTI_ATOM_DEFAULT_DESCRIPTION(ullong, unsigned long long)
oRTTI_ATOM_DEFAULT_DESCRIPTION(float, float)
oRTTI_ATOM_DEFAULT_DESCRIPTION(double, double)
oRTTI_ATOM_DEFAULT_DESCRIPTION(oRGBf, oRGBf)

// _____________________________________________________________________________

typedef oRTTI_DATA_COMPOUND<oRTTI_OBJECT> oRTTI_DATA_COMPOUND_BASE;

// _____________________________________________________________________________
// Type information

char* oRTTI::GetName(char* _StrDestination, size_t _SizeofDestination) const
{
	switch (Type)
	{
	case oRTTI_TYPE_ATOM:
		oStrcpy(_StrDestination, _SizeofDestination, ((oRTTI_DATA_ATOM*)this)->TypeName);
		break;

	case oRTTI_TYPE_ENUM:
		oStrcpy(_StrDestination, _SizeofDestination, ((oRTTI_DATA_ENUM*)this)->TypeName);
		break;

	case oRTTI_TYPE_POINTER: 
		{
			oRTTI_DATA_POINTER* ptr = (oRTTI_DATA_POINTER*)this;
			oStringS itemName;
			oStrAppendf(_StrDestination, _SizeofDestination, "%s_%s", ptr->PointerType->Name, ptr->ItemType->GetName(itemName));
			break;
		}

	case oRTTI_TYPE_CONTAINER: 
		{
			oRTTI_DATA_CONTAINER* container = (oRTTI_DATA_CONTAINER*)this;
			oStringS itemName;
			oStrAppendf(_StrDestination, _SizeofDestination, "%s_%s", container->ContainerType->TypeName, container->ItemType->GetName(itemName));
			break;
		}

	case oRTTI_TYPE_COMPOUND:
		oStrcpy(_StrDestination, _SizeofDestination, ((oRTTI_DATA_COMPOUND_BASE*)this)->TypeName);
		break;

	default:
		oStrcpy(_StrDestination, _SizeofDestination, "<unknown>");
		break;
	}
	return _StrDestination;
}

int oRTTI::GetSize() const
{
	switch (Type)
	{
	case oRTTI_TYPE_ATOM:
		return ((oRTTI_DATA_ATOM*)this)->Size;

	case oRTTI_TYPE_ENUM:
		return ((oRTTI_DATA_ENUM*)this)->Size;

	case oRTTI_TYPE_POINTER: 
		return sizeof(void*);

	case oRTTI_TYPE_CONTAINER: 
		return ((oRTTI_DATA_CONTAINER*)this)->ContainerType->Size;

	case oRTTI_TYPE_COMPOUND:
		return ((oRTTI_DATA_COMPOUND_BASE*)this)->Size;

	default:
		return oInvalid;
	}
}

char* oRTTI::TypeToString(char* _StrDestination, size_t _SizeofDestination) const
{
	oStringM name;
	oStrAppendf(_StrDestination, _SizeofDestination, "oRTTI_OF(%s)", GetName(name));
	return _StrDestination;
}

// _____________________________________________________________________________
// General

bool oRTTI::FromString(const char* _String, void* _pValue) const
{
	switch (Type)
	{
	case oRTTI_TYPE_ATOM:
		// Atom types are required to have a FromString function
		return ((oRTTI_DATA_ATOM*)this)->FromString(_pValue, _String);

	case oRTTI_TYPE_COMPOUND: 
		{
			oRTTI_DATA_COMPOUND_BASE* compound = (oRTTI_DATA_COMPOUND_BASE*)this;
			if (compound->FromString)
			{
				oRTTI_OBJECT* object = (oRTTI_OBJECT*)_pValue;
				return (object->*(compound->FromString))(_String);
			}
			else
				return oErrorSetLast(oERROR_NOT_FOUND, "Compound type '%s' doesn't have a FromString function", compound->TypeName);
		}

	case oRTTI_TYPE_ENUM: 
		{
			oRTTI_DATA_ENUM* enumData = (oRTTI_DATA_ENUM*)this;
			oASSERT(enumData->Size == sizeof(int), "oRTTI_DATA_ENUM::VALUE is hardcoded as an int, however oRTTI_DATA_ENUM.Size != sizeof(int)");

			for(int i=0; i<enumData->NumValues; ++i)
			{
				const oRTTI_DATA_ENUM::VALUE& enumValue = enumData->Values[i];
				if (oStricmp(_String, enumValue.Name) == 0)
				{
					*(int*)_pValue = enumValue.Value;
					return true;
				}
			}
			return oErrorSetLast(oERROR_NOT_FOUND, "Couldn't resolve enum '%s' %s to a value", enumData->TypeName, _String);
		}

		oNODEFAULT;
	}
}

char* oRTTI::ToString(char* _StrDestination, size_t _SizeofDestination, const void* _pValue) const
{
	switch (Type)
	{
	case oRTTI_TYPE_ATOM:
		// Atom types are required to have a ToString function
		return ((oRTTI_DATA_ATOM*)this)->ToString(_StrDestination, _SizeofDestination, _pValue);

	case oRTTI_TYPE_COMPOUND: 
		{
			oRTTI_DATA_COMPOUND_BASE* compound = (oRTTI_DATA_COMPOUND_BASE*)this;
			if (compound->ToString)
			{
				oRTTI_OBJECT* object = (oRTTI_OBJECT*)_pValue;
				return (object->*(compound->ToString))(_StrDestination, _SizeofDestination);
			}
		}
		break;

	case oRTTI_TYPE_ENUM: 
		{
			oRTTI_DATA_ENUM* enumData = (oRTTI_DATA_ENUM*)this;
			oASSERT(enumData->Size == sizeof(int), "oRTTI_DATA_ENUM::VALUE is hardcoded as an int, however oRTTI_DATA_ENUM.Size != sizeof(int)");

			for(int i=0; i<enumData->NumValues; ++i)
			{
				const oRTTI_DATA_ENUM::VALUE& enumValue = enumData->Values[i];
				if (*(int*)_pValue == enumValue.Value)
				{
					oStrcpy(_StrDestination, _SizeofDestination, (char*)enumValue.Name);
					return _StrDestination;
				}
			}
		}
		break;
	}
	
	// Not found
	return _StrDestination;
}

// _____________________________________________________________________________
// Enums

int oRTTI::GetNumValues() const
{
	oASSERT(Type == oRTTI_TYPE_ENUM, "");
	return ((oRTTI_DATA_ENUM*)this)->NumValues;
}

int oRTTI::GetValue(int _Index) const
{
	oASSERT(Type == oRTTI_TYPE_ENUM, "");
	return ((oRTTI_DATA_ENUM*)this)->Values[_Index].Value;
}

const char* oRTTI::GetValueName(int _Index) const
{
	oASSERT(Type == oRTTI_TYPE_ENUM, "");
	return ((oRTTI_DATA_ENUM*)this)->Values[_Index].Name;
}

const char* oRTTI::AsString(const void* _pValue) const
{
	if (Type != oRTTI_TYPE_ENUM)
		return "<unsupported>";

	oRTTI_DATA_ENUM* enumData = (oRTTI_DATA_ENUM*)this;
	oASSERT(enumData->Size == sizeof(int), "oRTTI_DATA_ENUM::VALUE is hardcoded as an int, however oRTTI_DATA_ENUM.Size != sizeof(int)");

	for (int i=0; i<enumData->NumValues; ++i)
	{
		const oRTTI_DATA_ENUM::VALUE& enumValue = enumData->Values[i];
		if (*(int*)_pValue == enumValue.Value)
		{
			return enumValue.Name;
		}
	}
	return "<unknown>";
}

// _____________________________________________________________________________
// Compounds

int oRTTI::GetVersion() const
{
	if (Type != oRTTI_TYPE_COMPOUND) return 0;
	return ((oRTTI_DATA_COMPOUND_BASE*)this)->Version;
}

int oRTTI::GetNumAttrs() const
{
	if (Type != oRTTI_TYPE_COMPOUND) return 0;
	return ((oRTTI_DATA_COMPOUND_BASE*)this)->NumAttrs;
}

const oRTTI_ATTR* oRTTI::GetAttr(int _Index) const
{
	if (Type != oRTTI_TYPE_COMPOUND) return nullptr;
	return (const oRTTI_ATTR*)&((oRTTI_DATA_COMPOUND_BASE*)this)->Attrs[_Index];
}

// _____________________________________________________________________________
// Containers

const oRTTI* oRTTI::GetItemRTTI() const
{
	if (Type != oRTTI_TYPE_CONTAINER) return nullptr;
	return ((oRTTI_DATA_CONTAINER*)this)->ItemType;
}

bool oRTTI::IsPlainArray() const
{
	if (Type != oRTTI_TYPE_CONTAINER) return false;
	return ((oRTTI_DATA_CONTAINER*)this)->ContainerType->IsPlainArray;
}

int oRTTI::GetItemSize() const
{
	if (Type != oRTTI_TYPE_CONTAINER) return oInvalid;
	return ((oRTTI_DATA_CONTAINER*)this)->ItemSize;
}

void* oRTTI::GetItemPtr(const void* _pContainer, int _ContainerSizeInBytes, int _Index) const
{
	if (Type != oRTTI_TYPE_CONTAINER) return nullptr;
	return ((oRTTI_DATA_CONTAINER*)this)->ContainerType->GetItemPtr(*((oRTTI_DATA_CONTAINER*)this)->ItemType, _pContainer, _ContainerSizeInBytes, _Index);
}

int oRTTI::GetItemCount(const void* _pContainer, int _ContainerSizeInBytes) const
{
	if (Type != oRTTI_TYPE_CONTAINER) return oInvalid;
	return ((oRTTI_DATA_CONTAINER*)this)->ContainerType->GetItemCount(*((oRTTI_DATA_CONTAINER*)this)->ItemType, _pContainer, _ContainerSizeInBytes);
}

bool oRTTI::SetItemCount(void* _pContainer, int _ContainerSizeInBytes, int _NewCount, bool _ConstructNewItems) const
{
	if (Type != oRTTI_TYPE_CONTAINER) return nullptr;
	return ((oRTTI_DATA_CONTAINER*)this)->ContainerType->SetItemCount(*((oRTTI_DATA_CONTAINER*)this)->ItemType, _pContainer, _ContainerSizeInBytes, _NewCount, _ConstructNewItems);
}
