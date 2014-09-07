// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.
#include <oBasis/oJSONSerialize.h>
#include <oBasis/oError.h>
#include <oBase/fixed_string.h>
#include <vector>

using namespace ouro;

bool oJSONReadValue(void* _pDest, int _SizeOfDest, const oRTTI& _RTTI, const json& _JSON, json::node _Node)
{
	const char* value = _JSON.node_value(_Node);
	if (!value)
		return false;

	if ((_RTTI.GetTraits() & (type_trait_flag::is_integralf | type_trait_flag::is_floating_pointf)) != 0 && (_RTTI.GetNumStringTokens() == 1) && (*value != '\"'))
	{
		sstring Typename;
		if (strstr(_RTTI.GetName(Typename), "char"))
		{
			int Value;
			if (!from_string(&Value, value)) 
				return false;
			*((char*)_pDest) = (Value & 0xff);
			return true;
		}
		return _RTTI.FromString(value, _pDest, _SizeOfDest);
	}
	else
	{
		xxlstring buf;
		if (!json_escape_decode(buf.c_str(), buf.capacity(), value))
			return false;
		return _RTTI.FromString(buf.c_str(), _pDest, _SizeOfDest);
	}
}

bool oJSONReadCompound(void* _pDestination, const oRTTI& _RTTI, const json& _JSON, json::node _Node, bool _FailOnMissingValues)
{
	if (_RTTI.GetType() != oRTTI_TYPE_COMPOUND)
		return oErrorSetLast(std::errc::invalid_argument);

	std::vector<sstring> FromStringFailed;

	for (int b = 0; b < _RTTI.GetNumBases(); b++)
	{
		const oRTTI* baseRTTI = _RTTI.GetBaseRTTI(b);
		oJSONReadCompound(byte_add(_pDestination, _RTTI.GetBaseOffset(b)), *baseRTTI, _JSON, _Node, _FailOnMissingValues);
	}

	for (int i = 0; i < _RTTI.GetNumAttrs(); i++)
	{
		const oRTTI_ATTR* f = _RTTI.GetAttr(i);
		bool notFound = (f->Flags & oRTTI_COMPOUND_ATTR_OPTIONAL) != oRTTI_COMPOUND_ATTR_OPTIONAL;
		switch(f->RTTI->Type)
		{
		case oRTTI_TYPE_ENUM:
		case oRTTI_TYPE_ATOM:
			{
				json::node node = _JSON.first_child(_Node, f->Name);
				if (node)
				{
					notFound = false;
					if (!oJSONReadValue(f->GetDestPtr(_pDestination), as_int(f->Size), *f->RTTI, _JSON, node))
						FromStringFailed.push_back(f->Name);
				}
			}
			break;

		case oRTTI_TYPE_COMPOUND:
			{
				json::node node = _JSON.first_child(_Node, f->Name);
				if (node)
				{
					notFound = false;
					if (!oJSONReadCompound(f->GetDestPtr(_pDestination), *f->RTTI, _JSON, node, _FailOnMissingValues))
						FromStringFailed.push_back(f->Name);
				}
			}
			break;

		case oRTTI_TYPE_CONTAINER:
			{
				json::node node = _JSON.first_child(_Node, f->Name);
				if (node)
				{
					notFound = false;
					if (!oJSONReadContainer(f->GetDestPtr(_pDestination), as_int(f->Size), *f->RTTI, _JSON, node, _FailOnMissingValues))
						FromStringFailed.push_back(f->Name);
				}
			}
			break;

		default:
			{
				notFound = false;
				sstring rttiName;
				oTRACE("No support for RTTI type: %s", f->RTTI->TypeToString(rttiName));
			}
			break;
		}

		if (notFound)
		{
			sstring compoundName;
			oTRACE("No JSON attribute/node for: %s::%s in JSON node %s in %s", _RTTI.GetName(compoundName), f->Name, _JSON.node_name(_Node), _JSON.name());

			if (_FailOnMissingValues)
				return false;
		}
	}

	if (!FromStringFailed.empty())
	{
		xxlstring ErrorString;
		snprintf(ErrorString, "Error parsing the following type(s):");
		for (const auto& s : FromStringFailed)
			sncatf(ErrorString, " '%s'", s.c_str());
		oTRACE("%s", ErrorString.c_str());
		return oErrorSetLast(std::errc::protocol_error, "%s", ErrorString);
	}

	return true;
}

bool oJSONReadContainer(void* _pDestination, int _DestSizeInBytes, const oRTTI& _RTTI, const json& _JSON, json::node _Node, bool _FailOnMissingValues)
{
	if (_RTTI.GetType() != oRTTI_TYPE_CONTAINER)
		return oErrorSetLast(std::errc::invalid_argument);

	std::vector<sstring> FromStringFailed;

	const oRTTI* itemRTTI = _RTTI.GetItemRTTI();

	int i=0;
	for (json::node node = _JSON.first_child(_Node); node; node = _JSON.next_sibling(node), ++i)
	{
		switch(itemRTTI->Type)
		{
			case oRTTI_TYPE_ENUM:
			case oRTTI_TYPE_ATOM:
			{
				if (!_RTTI.SetItemCount(_pDestination, _DestSizeInBytes, i+1))
					FromStringFailed.push_back("item");
				if (!oJSONReadValue(_RTTI.GetItemPtr(_pDestination, _DestSizeInBytes, i), as_int(_RTTI.GetItemSize()), *itemRTTI, _JSON, node))
					FromStringFailed.push_back("item");
				break;
			}

			case oRTTI_TYPE_COMPOUND:
				if (!_RTTI.SetItemCount(_pDestination, _DestSizeInBytes, i+1))
					FromStringFailed.push_back("item");
				oJSONReadCompound(_RTTI.GetItemPtr(_pDestination, _DestSizeInBytes, i), *itemRTTI, _JSON, node, _FailOnMissingValues);
				break;

			case oRTTI_TYPE_CONTAINER:
				if (!_RTTI.SetItemCount(_pDestination, _DestSizeInBytes, i+1))
					FromStringFailed.push_back("item");
				oJSONReadContainer(_RTTI.GetItemPtr(_pDestination, _DestSizeInBytes, i), _RTTI.GetItemSize(), *itemRTTI, _JSON, node, _FailOnMissingValues);
				break;

			default:
			{
				sstring rttiName;
				itemRTTI->TypeToString(rttiName.c_str(), rttiName.capacity());
				oTRACE("No support for RTTI type: %s", rttiName.c_str());
				break;
			}
		}
	}

	if (!FromStringFailed.empty())
	{
		xxlstring ErrorString;
		snprintf(ErrorString, "Error parsing the following type(s):");
		for (sstring& item : FromStringFailed)
		{
			sncatf(ErrorString, " '%s'", item.c_str());
		}
		oTRACE("%s", ErrorString.c_str());
		return oErrorSetLast(std::errc::protocol_error, "%s", ErrorString);
	}

	return true;
}

bool oJSONWriteValue(char* _StrDestination, size_t _SizeofStrDestination, const void* _pSource, const oRTTI& _RTTI)
{
	xxlstring buf;
	sstring Typename;
	if (_RTTI.ToString(buf, _pSource))
	{
		if ((_RTTI.GetTraits() & (type_trait_flag::is_integralf | type_trait_flag::is_floating_pointf)) != 0  && (_RTTI.GetNumStringTokens() == 1))
		{
			if (strstr(_RTTI.GetName(Typename), "uchar"))
				sncatf(_StrDestination, _SizeofStrDestination, "%d", (int)*(uchar*)_pSource);
			else if (strstr(_RTTI.GetName(Typename), "char"))
				sncatf(_StrDestination, _SizeofStrDestination, "%d", (int)*(char*)_pSource);
			else
				sncatf(_StrDestination, _SizeofStrDestination, "%s", buf.c_str());
		}
		else
			json_escape_encode(_StrDestination, _SizeofStrDestination, buf.c_str());
	}
	else
	{	
		sncatf(_StrDestination, _SizeofStrDestination, "null");
	}
	return true;
}

bool oJSONWriteCompound(char* _StrDestination, size_t _SizeofStrDestination, const void* _pSource, const oRTTI& _RTTI)
{
	if (_RTTI.GetType() != oRTTI_TYPE_COMPOUND)
		return oErrorSetLast(std::errc::invalid_argument);

	sncatf(_StrDestination, _SizeofStrDestination, "{");
	bool firstNameValuePair = true;

	for (int i=0; i<_RTTI.GetNumAttrs(); ++i)
	{
		const oRTTI_ATTR* f = _RTTI.GetAttr(i);
		switch(f->RTTI->Type)
		{
			case oRTTI_TYPE_ENUM:
			case oRTTI_TYPE_ATOM:
			{
				if (!firstNameValuePair) sncatf(_StrDestination, _SizeofStrDestination, ","); firstNameValuePair = false;
				sncatf(_StrDestination, _SizeofStrDestination, "\"%s\":", f->Name);
				if (!oJSONWriteValue(_StrDestination, _SizeofStrDestination, f->GetSrcPtr(_pSource), *f->RTTI))
					return false; // forward error
				break;
			}

			case oRTTI_TYPE_COMPOUND:
			{
				if (!firstNameValuePair) sncatf(_StrDestination, _SizeofStrDestination, ","); firstNameValuePair = false;
				sncatf(_StrDestination, _SizeofStrDestination, "\"%s\":", f->Name);
				if (!oJSONWriteCompound(_StrDestination, _SizeofStrDestination, f->GetSrcPtr(_pSource), *f->RTTI))
					return false; // forward error
				break;
			}

			case oRTTI_TYPE_CONTAINER:
			{
				if (!firstNameValuePair) sncatf(_StrDestination, _SizeofStrDestination, ","); firstNameValuePair = false;
				sncatf(_StrDestination, _SizeofStrDestination, "\"%s\":", f->Name);
				if (!oJSONWriteContainer(_StrDestination, _SizeofStrDestination, f->GetSrcPtr(_pSource), f->Size, *f->RTTI))
					return false; // forward error
				break;
			}

			default:
			{
				sstring rttiName;
				return oErrorSetLast(std::errc::not_supported, "No support for RTTI type: %s", f->RTTI->TypeToString(rttiName));
			}
		}
	}

	sncatf(_StrDestination, _SizeofStrDestination, "}");
	return true;
}


bool oJSONWriteContainer(char* _StrDestination, size_t _SizeofStrDestination, const void* _pSource, int _SourceSize, const oRTTI& _RTTI)
{
	if (_RTTI.GetType() != oRTTI_TYPE_CONTAINER)
		return oErrorSetLast(std::errc::invalid_argument);

	std::vector<sstring> FromStringFailed;

	const oRTTI* itemRTTI = _RTTI.GetItemRTTI();

	sncatf(_StrDestination, _SizeofStrDestination, "[");
	bool firstValue = true;

	for (int i = 0; i < _RTTI.GetItemCount(_pSource, _SourceSize); i++)
	{
		lstring elem_buf;
		switch(itemRTTI->Type)
		{
			case oRTTI_TYPE_ENUM:
			case oRTTI_TYPE_ATOM:
			{
				if (!firstValue) sncatf(_StrDestination, _SizeofStrDestination, ","); firstValue = false;
				if (!oJSONWriteValue(_StrDestination, _SizeofStrDestination, _RTTI.GetItemPtr(_pSource, _SourceSize, i), *itemRTTI))
					return false; // forward error
				break;
			}

			case oRTTI_TYPE_COMPOUND:
			{
				if (!firstValue) sncatf(_StrDestination, _SizeofStrDestination, ","); firstValue = false;
				if (!oJSONWriteCompound(_StrDestination, _SizeofStrDestination, _RTTI.GetItemPtr(_pSource, _SourceSize, i), *itemRTTI))
					return false; // forward error
				break;
			}

			case oRTTI_TYPE_CONTAINER:
			{
				if (!firstValue) sncatf(_StrDestination, _SizeofStrDestination, ","); firstValue = false;
				if (!oJSONWriteContainer(_StrDestination, _SizeofStrDestination, _RTTI.GetItemPtr(_pSource, _SourceSize, i), itemRTTI->GetSize(), *itemRTTI))
					return false; // forward error
				break;
			}

			default:
			{
				sstring rttiName;
				return oErrorSetLast(std::errc::not_supported, "No support for RTTI type: %s", itemRTTI->TypeToString(rttiName));
			}
		}
	}
	
	sncatf(_StrDestination, _SizeofStrDestination, "]");
	return true;
}
