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
#include <oBasis/oRTTI.h>
#include <oCompute/oComputeConstants.h>
#include <oBase/aabox.h>
#include <oBase/plane.h>
#include <oBase/quat.h>
#include <oBase/sphere.h>
#include <oCompute/oFrustum.h>
#include <oCompute/linear_algebra.h>
#include <oBasis/oError.h>
#include <oBase/invalid.h>
#include <oBase/fixed_string.h>
#include <oBase/fixed_vector.h>
#include <vector>

using namespace ouro;

// TODO: Fix and move Array descriptions
bool ouro_fixed_vectorSetItemCount(const oRTTI& _RTTI, void* _pContainer, int _ContainerSizeInBytes, int _NewSize, bool _ConstructNewItems)
{
	int sizeOffset = _ContainerSizeInBytes - sizeof(size_t);
	if (_RTTI.GetSize() * _NewSize > sizeOffset) return false;
	*(size_t*)byte_add(_pContainer, sizeOffset) = _NewSize;
	return true;
}
int ouro_fixed_vectorGetItemCount(const oRTTI& _RTTI, const void* _pContainer, int _ContainerSizeInBytes)
{
	int sizeOffset = _ContainerSizeInBytes - sizeof(size_t);
	return (int)*(size_t*)byte_add(_pContainer, sizeOffset);
}
void* ouro_fixed_vectorGetItemPtr(const oRTTI& _RTTI, const void* _pContainer, int _ContainerSizeInBytes, int _Index)
{
	int offset = _RTTI.GetSize() * _Index;
	void* result = byte_add(const_cast<void*>(_pContainer), offset);
	return offset < (_ContainerSizeInBytes - (int)sizeof(size_t)) ? result : nullptr;
}
oRTTI_CONTAINER_BEGIN_DESCRIPTION(ouro_fixed_vector)
	sizeof(fixed_vector<void*,0>),
	true,
	nullptr,
	nullptr,
	ouro_fixed_vectorSetItemCount,
	ouro_fixed_vectorGetItemCount,
	ouro_fixed_vectorGetItemPtr
oRTTI_CONTAINER_END_DESCRIPTION(ouro_fixed_vector)

// Container declaration for regular c-style arrays like int[20]
bool cArraySetItemCount(const oRTTI& _RTTI, void* _pContainer, int _ContainerSizeInBytes, int _NewSize, bool _ConstructNewItems)
{
	return (_RTTI.GetSize() * _NewSize <= _ContainerSizeInBytes);
}
int cArrayGetItemCount(const oRTTI& _RTTI, const void* _pContainer, int _ContainerSizeInBytes)
{
	return _ContainerSizeInBytes / _RTTI.GetSize();
}
void* cArrayGetItemPtr(const oRTTI& _RTTI, const void* _pContainer, int _ContainerSizeInBytes, int _Index)
{
	int offset = _RTTI.GetSize() * _Index;
	void* result = byte_add(const_cast<void*>(_pContainer), offset);
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

// TODO: Expand atom type support and move to appropriate place
oRTTI_ATOM_DEFAULT_DESCRIPTION_CONSTRUCTOR(oRTTI_CAPS_ARRAY, ouro_version, ouro_version, 1)
oRTTI_ATOM_DEFAULT_DESCRIPTION(oRTTI_CAPS_ARRAY_NO_STD, bool, bool, 1)
oRTTI_ATOM_DEFAULT_DESCRIPTION(oRTTI_CAPS_ARRAY, char, char, 1)
oRTTI_ATOM_DEFAULT_DESCRIPTION(oRTTI_CAPS_ARRAY, uchar, unsigned char, 1)
oRTTI_ATOM_DEFAULT_DESCRIPTION(oRTTI_CAPS_ARRAY, short, short, 1)
oRTTI_ATOM_DEFAULT_DESCRIPTION(oRTTI_CAPS_ARRAY, ushort, unsigned short, 1)
oRTTI_ATOM_DEFAULT_DESCRIPTION(oRTTI_CAPS_ARRAY, int, int, 1)
oRTTI_ATOM_DEFAULT_DESCRIPTION(oRTTI_CAPS_ARRAY, int2, int2, 2)
oRTTI_ATOM_DEFAULT_DESCRIPTION(oRTTI_CAPS_ARRAY, int3, int3, 3)
oRTTI_ATOM_DEFAULT_DESCRIPTION(oRTTI_CAPS_ARRAY, int4, int4, 4)
oRTTI_ATOM_DEFAULT_DESCRIPTION(oRTTI_CAPS_ARRAY, uint, unsigned int, 1)
oRTTI_ATOM_DEFAULT_DESCRIPTION(oRTTI_CAPS_ARRAY, uint2, uint2, 2)
oRTTI_ATOM_DEFAULT_DESCRIPTION(oRTTI_CAPS_ARRAY, uint3, uint3, 3)
oRTTI_ATOM_DEFAULT_DESCRIPTION(oRTTI_CAPS_ARRAY, uint4, uint4, 4)
oRTTI_ATOM_DEFAULT_DESCRIPTION(oRTTI_CAPS_ARRAY, llong, long long, 1)
oRTTI_ATOM_DEFAULT_DESCRIPTION(oRTTI_CAPS_ARRAY, ullong, unsigned long long, 1)
oRTTI_ATOM_DEFAULT_DESCRIPTION(oRTTI_CAPS_ARRAY, float, float, 1)
oRTTI_ATOM_DEFAULT_DESCRIPTION(oRTTI_CAPS_ARRAY, float2, float2, 2)
oRTTI_ATOM_DEFAULT_DESCRIPTION(oRTTI_CAPS_ARRAY, float3, float3, 3)
oRTTI_ATOM_DEFAULT_DESCRIPTION(oRTTI_CAPS_ARRAY, float4, float4, 4)
oRTTI_ATOM_DEFAULT_DESCRIPTION(oRTTI_CAPS_ARRAY, double, double, 1)
oRTTI_ATOM_DEFAULT_DESCRIPTION(oRTTI_CAPS_ARRAY, ouro_color, ouro_color, -1) // Can be either 1 or 4 string tokens.. so it's ambiguous
oRTTI_ATOM_DEFAULT_DESCRIPTION(oRTTI_CAPS_ARRAY, ouro_rgbf, ouro_rgbf, -1) // Can be either 1 or 3 string tokens.. so it's ambiguous
oRTTI_ATOM_DEFAULT_DESCRIPTION_CONSTRUCTOR(oRTTI_CAPS_ARRAY, float4x4, float4x4, 16, oIDENTITY4x4)
oRTTI_ATOM_DEFAULT_DESCRIPTION_CONSTRUCTOR(oRTTI_CAPS_ARRAY, quatf, quatf, 4, identity_quatf)
oRTTI_ATOM_DEFAULT_DESCRIPTION_CONSTRUCTOR(oRTTI_CAPS_ARRAY, planef, planef, 4, planef())
oRTTI_ATOM_DEFAULT_DESCRIPTION_CONSTRUCTOR(oRTTI_CAPS_ARRAY, spheref, spheref, 4, spheref())
oRTTI_ATOM_DEFAULT_DESCRIPTION_CONSTRUCTOR(oRTTI_CAPS_ARRAY, aaboxf, aaboxf, 6, aaboxf())
oRTTI_ATOM_DEFAULT_DESCRIPTION(oRTTI_CAPS_ARRAY, ouro_fourcc, ouro_fourcc, 1)
oRTTI_ATOM_DEFAULT_DESCRIPTION(oRTTI_CAPS_ARRAY, ouro_sstring, ouro_sstring, -1)
oRTTI_ATOM_DEFAULT_DESCRIPTION(oRTTI_CAPS_ARRAY, ouro_mstring, ouro_mstring, -1)
oRTTI_ATOM_DEFAULT_DESCRIPTION(oRTTI_CAPS_ARRAY, ouro_lstring, ouro_lstring, -1)
oRTTI_ATOM_DEFAULT_DESCRIPTION(oRTTI_CAPS_ARRAY, ouro_xlstring, ouro_xlstring, -1)
oRTTI_ATOM_DEFAULT_DESCRIPTION(oRTTI_CAPS_ARRAY, ouro_xxlstring, ouro_xxlstring, -1)
oRTTI_ATOM_DEFAULT_DESCRIPTION(oRTTI_CAPS_ARRAY, ouro_path_string, ouro_path_string, -1)
oRTTI_ATOM_DEFAULT_DESCRIPTION(oRTTI_CAPS_ARRAY, ouro_uri_string, ouro_uri_string, -1)
oRTTI_ATOM_DEFAULT_DESCRIPTION(oRTTI_CAPS_ARRAY, ouro_path, ouro_path, -1)
oRTTI_ATOM_DEFAULT_DESCRIPTION(oRTTI_CAPS_ARRAY, ouro_uri, ouro_uri, -1)
static ouro_input_key init_input_key() { return ouro::input::none; }
static ouro_input_skeleton_bone init_skeleton_bone() { return ouro::input::invalid_bone; }
oRTTI_ATOM_DEFAULT_DESCRIPTION_CONSTRUCTOR(oRTTI_CAPS_ARRAY, ouro_input_key, ouro_input_key, 4, init_input_key())
	oRTTI_ATOM_DEFAULT_DESCRIPTION_CONSTRUCTOR(oRTTI_CAPS_ARRAY, ouro_input_skeleton_bone, ouro_input_skeleton_bone, 4, init_skeleton_bone())

namespace ouro {

// @tony: Can/should these be moved to a header? (string's alloc may eval 
// differently than what's passed in v. code used in this CPP to execute). 
// Perhaps oStdStringSupport.h?
bool from_string(std::string* _pValue, const char* _StrSource)
{
	*_pValue = _StrSource;
	return true;
}

char* to_string(char* _StrDestination, size_t _SizeofDestination, const std::string* _pValue)
{
	strlcpy(_StrDestination, _pValue->c_str(), _SizeofDestination);
	return _StrDestination;
}

} // namespace ouro

oRTTI_ATOM_DEFAULT_DESCRIPTION_CONSTRUCTOR(oRTTI_CAPS_ARRAY, std_string, std_string, -1)


// _____________________________________________________________________________

typedef oRTTI_DATA_COMPOUND<oRTTI_OBJECT> oRTTI_DATA_COMPOUND_BASE;

// _____________________________________________________________________________
// Type information

char* oRTTI::GetName(char* _StrDestination, size_t _SizeofDestination) const
{
	switch (Type)
	{
		case oRTTI_TYPE_ATOM:
			strlcpy(_StrDestination, ((oRTTI_DATA_ATOM*)this)->TypeName, _SizeofDestination);
			break;

		case oRTTI_TYPE_ENUM:
			strlcpy(_StrDestination, ((oRTTI_DATA_ENUM*)this)->TypeName, _SizeofDestination);
			break;

		case oRTTI_TYPE_POINTER: 
		{
			oRTTI_DATA_POINTER* ptr = (oRTTI_DATA_POINTER*)this;
			sstring itemName;
			sncatf(_StrDestination, _SizeofDestination, "%s_%s", ptr->PointerType->Name, ptr->ItemType->GetName(itemName));
			break;
		}

		case oRTTI_TYPE_CONTAINER: 
		{
			oRTTI_DATA_CONTAINER* container = (oRTTI_DATA_CONTAINER*)this;
			sstring itemName;
			sncatf(_StrDestination, _SizeofDestination, "%s_%s", container->ContainerType->TypeName, container->ItemType->GetName(itemName));
			break;
		}

		case oRTTI_TYPE_COMPOUND:
			strlcpy(_StrDestination, ((oRTTI_DATA_COMPOUND_BASE*)this)->TypeName, _SizeofDestination);
			break;

		default:
			strlcpy(_StrDestination, "<unknown>", _SizeofDestination);
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
			return ouro::invalid;
	}
}

char* oRTTI::TypeToString(char* _StrDestination, size_t _SizeofDestination) const
{
	mstring name;
	sncatf(_StrDestination, _SizeofDestination, "oRTTI_OF(%s)", GetName(name));
	return _StrDestination;
}

// _____________________________________________________________________________
// General

bool oRTTI::FromString(const char* _String, void* _pValue, int _SizeOfValue) const
{
	switch (Type)
	{
		case oRTTI_TYPE_ATOM:
		{
			oRTTI_DATA_ATOM* atom = (oRTTI_DATA_ATOM*)this;

			int backup = _SizeOfValue;
			if (atom->HasTemplatedSize)
			{
				oASSERT(_SizeOfValue >= 4, "RTTI Atom with templated size expected to be at least 4 bytes");
				std::swap(backup, *(int*)_pValue);
			}

			// Atom types are required to have a FromString function
			bool result = atom->FromString(_pValue, _String);

			// Restore original memory if FromString failed
			if (atom->HasTemplatedSize && !result)
				std::swap(backup, *(int*)_pValue);

			return result;
		}

		case oRTTI_TYPE_COMPOUND: 
		{
			oRTTI_DATA_COMPOUND_BASE* compound = (oRTTI_DATA_COMPOUND_BASE*)this;
			if (compound->FromString)
			{
				oRTTI_OBJECT* object = (oRTTI_OBJECT*)_pValue;
				return (object->*(compound->FromString))(_String);
			}
			else
				return oErrorSetLast(std::errc::function_not_supported, "Compound type '%s' doesn't have a FromString function", compound->TypeName);
		}

		case oRTTI_TYPE_ENUM: 
		{
			oRTTI_DATA_ENUM* enumData = (oRTTI_DATA_ENUM*)this;
			oASSERT(enumData->Size == sizeof(int), "oRTTI_DATA_ENUM::VALUE is hardcoded as an int, however oRTTI_DATA_ENUM.Size != sizeof(int)");

			if (enumData->CaseSensitive)
			{
				for(int i=0; i<enumData->NumValues; ++i)
				{
					const oRTTI_DATA_ENUM::VALUE& enumValue = enumData->Values[i];
					if (strcmp(_String, enumValue.Name) == 0)
					{
						*(int*)_pValue = enumValue.Value;
						return true;
					}
				}
			}

			else
			{
				for(int i=0; i<enumData->NumValues; ++i)
				{
					const oRTTI_DATA_ENUM::VALUE& enumValue = enumData->Values[i];
					if (_stricmp(_String, enumValue.Name) == 0)
					{
						*(int*)_pValue = enumValue.Value;
						return true;
					}
				}
			}
			return oErrorSetLast(std::errc::invalid_argument, "Couldn't resolve enum '%s' %s to a value", enumData->TypeName, _String);
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
					strlcpy(_StrDestination, (char*)enumValue.Name, _SizeofDestination);
					return _StrDestination;
				}
			}
		}
		break;
	}
	
	// Not found
	return _StrDestination;
}

int oRTTI::GetNumStringTokens() const
{
	switch (Type)
	{
		case oRTTI_TYPE_ENUM: 
			return 1;

		case oRTTI_TYPE_ATOM:
			return ((oRTTI_DATA_ATOM*)this)->NumStringTokens;

		default:
			return -1;
	}
}

// _____________________________________________________________________________
// Atoms

uint oRTTI::GetTraits() const
{
	switch (Type)
	{
		case oRTTI_TYPE_ATOM:
			return ((oRTTI_DATA_ATOM*)this)->TypeTraits;

		case oRTTI_TYPE_ENUM:
			// Assuming that all enum traits are the same..
			return ouro::type_info<type_trait_flag::value>::traits;

		default:
			// TODO: Do we need traits for COMPOUNDS, CONTAINERS, POINTERS too?
			return 0;
	}
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

version oRTTI::GetVersion() const
{
	if (Type != oRTTI_TYPE_COMPOUND) return version();
	return ((oRTTI_DATA_COMPOUND_BASE*)this)->Version;
}

int oRTTI::GetNumBases() const
{
	if (Type != oRTTI_TYPE_COMPOUND) return 0;
	return ((oRTTI_DATA_COMPOUND_BASE*)this)->NumBases;
}

const oRTTI* oRTTI::GetBaseRTTI(int _Index) const
{
	if (Type != oRTTI_TYPE_COMPOUND) return 0;
	return ((oRTTI_DATA_COMPOUND_BASE*)this)->Bases[_Index].RTTI;
}

int oRTTI::GetBaseOffset(int _Index) const
{
	if (Type != oRTTI_TYPE_COMPOUND) return 0;
	return ((oRTTI_DATA_COMPOUND_BASE*)this)->Bases[_Index].Offset;
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

void oRTTI::EnumAttrs(bool _IncludeBases, std::function<bool(const oRTTI_ATTR& _Attr)> _Callback) const
{
	if (Type != oRTTI_TYPE_COMPOUND) return;
	if (_IncludeBases)
	{
		for (int base = 0; base < GetNumBases(); base++)
			GetBaseRTTI(base)->EnumAttrs(_IncludeBases, _Callback);
	}
	for (int attr = 0; attr < GetNumAttrs(); attr++)
		_Callback(*GetAttr(attr));
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
	if (Type != oRTTI_TYPE_CONTAINER) return ouro::invalid;
	return ((oRTTI_DATA_CONTAINER*)this)->ItemSize;
}

void* oRTTI::GetItemPtr(const void* _pContainer, int _ContainerSizeInBytes, int _Index) const
{
	if (Type != oRTTI_TYPE_CONTAINER) return nullptr;
	return ((oRTTI_DATA_CONTAINER*)this)->ContainerType->GetItemPtr(*((oRTTI_DATA_CONTAINER*)this)->ItemType, _pContainer, _ContainerSizeInBytes, _Index);
}

int oRTTI::GetItemCount(const void* _pContainer, int _ContainerSizeInBytes) const
{
	if (Type != oRTTI_TYPE_CONTAINER) return ouro::invalid;
	return ((oRTTI_DATA_CONTAINER*)this)->ContainerType->GetItemCount(*((oRTTI_DATA_CONTAINER*)this)->ItemType, _pContainer, _ContainerSizeInBytes);
}

bool oRTTI::SetItemCount(void* _pContainer, int _ContainerSizeInBytes, int _NewCount, bool _ConstructNewItems) const
{
	if (Type != oRTTI_TYPE_CONTAINER) return nullptr;
	return ((oRTTI_DATA_CONTAINER*)this)->ContainerType->SetItemCount(*((oRTTI_DATA_CONTAINER*)this)->ItemType, _pContainer, _ContainerSizeInBytes, _NewCount, _ConstructNewItems);
}
