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
#include <oBasis/oXMLSerialize.h>
#include <oBasis/oStrTok.h>
#include <oBase/fixed_string.h>
#include <oBasis/oError.h>
#include <vector>

using namespace ouro;

bool oXMLReadCompound(void* _pDestination, const oRTTI& _RTTI, const xml& _XML, xml::node _Node, bool _FailOnMissingValues)
{
	if (_RTTI.GetType() != oRTTI_TYPE_COMPOUND)
		return oErrorSetLast(std::errc::invalid_argument);

	std::vector<sstring> FromStringFailed;

	for (int b = 0; b < _RTTI.GetNumBases(); b++)
	{
		const oRTTI* baseRTTI = _RTTI.GetBaseRTTI(b);
		oXMLReadCompound(byte_add(_pDestination, _RTTI.GetBaseOffset(b)), *baseRTTI, _XML, _Node, _FailOnMissingValues);
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
				if (f->Flags & oRTTI_COMPOUND_ATTR_XML_STYLE_NODE)
				{
					xml::node n = (f->Flags & oRTTI_COMPOUND_ATTR_XML_STYLE_NODE_EMBEDDED) == oRTTI_COMPOUND_ATTR_XML_STYLE_NODE_EMBEDDED 
						? _Node 
						: _XML.first_child(_Node, f->Name);

					if (n)
					{
						notFound = false;
						oCHECK_SIZE(int, f->Size);
						if (!f->RTTI->FromString(_XML.node_value(n), f->GetDestPtr(_pDestination), static_cast<int>(f->Size)))
							FromStringFailed.push_back(f->Name);
					}
				}
				else
				{
					const char* v = _XML.find_attr_value(_Node, f->Name);
					if (oSTRVALID(v))
					{
						notFound = false;
						oCHECK_SIZE(int, f->Size);
						if (!f->RTTI->FromString(v, f->GetDestPtr(_pDestination), static_cast<int>(f->Size)))
							FromStringFailed.push_back(f->Name);
					}
				}
			}
			break;

		case oRTTI_TYPE_COMPOUND:
			{
				xml::node node = _XML.first_child(_Node, f->Name);
				if (node)
				{
					notFound = false;
					if (!oXMLReadCompound(f->GetDestPtr(_pDestination), *f->RTTI, _XML, node, _FailOnMissingValues))
						FromStringFailed.push_back(f->Name);
				}
			}
			break;

		case oRTTI_TYPE_CONTAINER:
			{
				bool isEmbedded = (f->Flags & oRTTI_COMPOUND_ATTR_XML_STYLE_NODE_EMBEDDED) == oRTTI_COMPOUND_ATTR_XML_STYLE_NODE_EMBEDDED;
				xml::node node = isEmbedded ? _Node : _XML.first_child(_Node, f->Name);
				if (node)
				{
					notFound = false;
					if (!oXMLReadContainer(f->GetDestPtr(_pDestination), f->Size, *f->RTTI, isEmbedded ? f->Name : "item", (f->Flags & oRTTI_COMPOUND_ATTR_XML_STYLE_RAW_ARRAY) != 0, _XML, node, _FailOnMissingValues))
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
			oTRACE("No XML attribute/node for: %s::%s in XML node %s in %s", _RTTI.GetName(compoundName), f->Name, _XML.node_name(_Node), _XML.name());

			if (_FailOnMissingValues)
				return false;
		}
	}

	if (!FromStringFailed.empty())
	{
		xxlstring ErrorString;
		snprintf(ErrorString, "Error parsing the following type(s):");
		oFORI(i, FromStringFailed)
		{
			sncatf(ErrorString, " '%s'", FromStringFailed[i].c_str());
		}
		oTRACE("%s", ErrorString.c_str());
		return oErrorSetLast(std::errc::protocol_error, "%s", ErrorString);
	}

	return true;
}

bool oXMLReadContainer(void* _pDestination, int _DestSizeInBytes, const oRTTI& _RTTI, const char* _pElementName, bool _IsRaw, const xml& _XML, xml::node _Node, bool _FailOnMissingValues)
{
	if (_RTTI.GetType() != oRTTI_TYPE_CONTAINER)
		return oErrorSetLast(std::errc::invalid_argument);

	std::vector<sstring> FromStringFailed;

	const oRTTI* itemRTTI = _RTTI.GetItemRTTI();

	if (_IsRaw)
	{
		switch(itemRTTI->Type)
		{
		case oRTTI_TYPE_ENUM:
		case oRTTI_TYPE_ATOM:
			{
				int numTokensPerElement = itemRTTI->GetNumStringTokens();
				if (numTokensPerElement < 0)
					return oErrorSetLast(std::errc::invalid_argument, "Using an ambiguous element type for raw array (a type that doesn't have a fixed amount of tokens in xml format)");

				// The elements of this container don't have XML tags, so we rely on the NumStringTokens of the atom (which is 1 for enums)
				// So we have to read all elements here in one go, we're assuming the array has been resized to the amount we need to read.
				const char* nodeValue = _XML.node_value(_Node);
				int numElements = _RTTI.GetItemCount(_pDestination, _DestSizeInBytes);
				for (int i=0; i<numElements; ++i)
				{
					oCHECK_SIZE(int, _RTTI.GetItemSize());
					if (!itemRTTI->FromString(nodeValue, _RTTI.GetItemPtr(_pDestination, _DestSizeInBytes, i), static_cast<int>(_RTTI.GetItemSize())))
						FromStringFailed.push_back(_pElementName);
					nodeValue = oStrTokSkip(nodeValue, " \t\r\n", numTokensPerElement);
					if (!nodeValue)
						return !_FailOnMissingValues || oErrorSetLast(std::errc::protocol_error, "Container with XML style RAW is missing values");
				}
				break;
			}
		default:
			return oErrorSetLast(std::errc::invalid_argument, "Containers with XML style RAW is only supported if the elements are enums or atoms");
		}
		return true;
	}

	int i=0;
	for (xml::node node = _XML.first_child(_Node, _pElementName); node; node = _XML.next_sibling(node, _pElementName), ++i)
	{
		int cur_count = _RTTI.GetItemCount(_pDestination, _DestSizeInBytes);
		int new_count = i + 1;

		switch(itemRTTI->Type)
		{
		case oRTTI_TYPE_ENUM:
		case oRTTI_TYPE_ATOM:
			if (cur_count < new_count && !_RTTI.SetItemCount(_pDestination, _DestSizeInBytes, new_count))
				FromStringFailed.push_back("item");
			oCHECK_SIZE(int, _RTTI.GetItemSize());
			if (!itemRTTI->FromString(_XML.node_value(node), _RTTI.GetItemPtr(_pDestination, _DestSizeInBytes, i), static_cast<int>(_RTTI.GetItemSize())))
				FromStringFailed.push_back(_pElementName);
			break;

		case oRTTI_TYPE_COMPOUND:
			if (cur_count < new_count && !_RTTI.SetItemCount(_pDestination, _DestSizeInBytes, new_count))
				FromStringFailed.push_back("item");
			oXMLReadCompound(_RTTI.GetItemPtr(_pDestination, _DestSizeInBytes, i), *itemRTTI, _XML, node, _FailOnMissingValues);
			break;

		case oRTTI_TYPE_CONTAINER:
			if (cur_count < new_count && !_RTTI.SetItemCount(_pDestination, _DestSizeInBytes, new_count))
				FromStringFailed.push_back("item");
			oXMLReadContainer(_RTTI.GetItemPtr(_pDestination, _DestSizeInBytes, i), _RTTI.GetItemSize(), *itemRTTI, "item", false, _XML, node, _FailOnMissingValues);
			break;

		default:
			{
				sstring rttiName;
				itemRTTI->TypeToString(rttiName.c_str(), rttiName.capacity());
				oTRACE("No support for RTTI type: %s", rttiName.c_str());
			}
			break;
		}
	}

	if (!FromStringFailed.empty())
	{
		xxlstring ErrorString;
		snprintf(ErrorString, "Error parsing the following type(s):");
		oFOR(sstring& item, FromStringFailed)
		{
			sncatf(ErrorString, " '%s'", item.c_str());
		}
		oTRACE("%s", ErrorString.c_str());
		return oErrorSetLast(std::errc::protocol_error, "%s", ErrorString);
	}

	return true;
}
