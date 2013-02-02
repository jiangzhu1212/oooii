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
#include <oBasis/oXMLSerialize.h>
#include <oBasis/oFixedString.h>
#include <vector>

bool oXMLReadCompound(void* _pDestination, const oRTTI& _RTTI, const threadsafe oXML* _pXML, oXML::HNODE _hNode, bool _FailOnMissingValues)
{
	if (_RTTI.GetType() != oRTTI_TYPE_COMPOUND)
		return oErrorSetLast(oERROR_INVALID_PARAMETER);

	std::vector<oStringS> FromStringFailed;

	for (int i=0; i < _RTTI.GetNumAttrs(); ++i)
	{
		const oRTTI_ATTR* f = _RTTI.GetAttr(i);
		bool notFound = true;
		switch(f->RTTI->Type)
		{
		case oRTTI_TYPE_ENUM:
		case oRTTI_TYPE_ATOM:
			{
				if (f->Flags & oRTTI_COMPOUND_ATTR_XML_STYLE_NODE)
				{
					oXML::HNODE node = _pXML->FindNode(_hNode, f->Name);
					if (node)
					{
						notFound = false;
						if (!f->RTTI->FromString(_pXML->GetNodeValue(node), f->GetDestPtr(_pDestination)))
							FromStringFailed.push_back(f->Name);
					}
				}
				else
				{
					oStringL value;
					if (_pXML->GetAttributeValue(_hNode, f->Name, value.c_str(), value.capacity()))
					{
						notFound = false;
						if (!f->RTTI->FromString(value.c_str(), f->GetDestPtr(_pDestination)))
							FromStringFailed.push_back(f->Name);
					}
				}
			}
			break;

		case oRTTI_TYPE_COMPOUND:
			{
				oXML::HNODE node = _pXML->FindNode(_hNode, f->Name);
				if (node)
				{
					notFound = false;
					oXMLReadCompound(f->GetDestPtr(_pDestination), *f->RTTI, _pXML, node, _FailOnMissingValues);
				}
			}
			break;

		case oRTTI_TYPE_CONTAINER:
			{
				oXML::HNODE node = _pXML->FindNode(_hNode, f->Name);
				if (node)
				{
					notFound = false;
					oXMLReadContainer(f->GetDestPtr(_pDestination), f->Size, *f->RTTI, _pXML, node, _FailOnMissingValues);
				}
			}
			break;

		default:
			{
				notFound = false;
				oStringS rttiName;
				oTRACE("No support for RTTI type: %s", f->RTTI->TypeToString(rttiName));
			}
			break;
		}

		if (notFound)
		{
			oStringS compoundName;
			oTRACE("No XML attribute/node for: %s::%s in XML node %s in %s", _RTTI.GetName(compoundName), f->Name, _pXML->GetNodeName(_hNode), _pXML->GetDocumentName());
		}
	}

	if (!FromStringFailed.empty())
		return !_FailOnMissingValues;

	return true;
}

bool oXMLReadContainer(void* _pDestination, int _DestSizeInBytes, const oRTTI& _RTTI, const threadsafe oXML* _pXML, oXML::HNODE _hNode, bool _FailOnMissingValues)
{
	if (_RTTI.GetType() != oRTTI_TYPE_CONTAINER)
		return oErrorSetLast(oERROR_INVALID_PARAMETER);

	std::vector<oStringS> FromStringFailed;

	int i=0;
	for (oXML::HNODE node = _pXML->GetFirstChild(_hNode); node; node = _pXML->GetNextSibling(node), ++i)
	{
		const oRTTI* itemRTTI = _RTTI.GetItemRTTI();
		switch(itemRTTI->Type)
		{
		case oRTTI_TYPE_ENUM:
		case oRTTI_TYPE_ATOM:
			if (!_RTTI.SetItemCount(_pDestination, _DestSizeInBytes, i+1))
				FromStringFailed.push_back("item");
			if (!itemRTTI->FromString(_pXML->GetNodeValue(node), _RTTI.GetItemPtr(_pDestination, _DestSizeInBytes, i)))
				FromStringFailed.push_back("item");
			break;

		case oRTTI_TYPE_COMPOUND:
			if (!_RTTI.SetItemCount(_pDestination, _DestSizeInBytes, i+1))
				FromStringFailed.push_back("item");
			oXMLReadCompound(_RTTI.GetItemPtr(_pDestination, _DestSizeInBytes, i), *itemRTTI, _pXML, node, _FailOnMissingValues);
			break;

		case oRTTI_TYPE_CONTAINER:
			if (!_RTTI.SetItemCount(_pDestination, _DestSizeInBytes, i+1))
				FromStringFailed.push_back("item");
			oXMLReadContainer(_RTTI.GetItemPtr(_pDestination, _DestSizeInBytes, i), _RTTI.GetItemSize(), *itemRTTI, _pXML, node, _FailOnMissingValues);
			break;

		default:
			{
				oStringS rttiName;
				itemRTTI->TypeToString(rttiName.c_str(), rttiName.capacity());
				oTRACE("No support for RTTI type: %s", rttiName.c_str());
			}
			break;
		}
	}

	if (!FromStringFailed.empty())
		return !_FailOnMissingValues;

	return true;
}

