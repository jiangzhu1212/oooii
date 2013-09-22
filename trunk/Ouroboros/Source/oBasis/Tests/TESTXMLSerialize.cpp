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
#include <oBasis/oURI.h>
#include <oBasis/oError.h>
#include <oBasis/oXMLSerialize.h>
#include "oBasisTestCommon.h"
#include <vector>

using namespace ouro;

static const char* sXMLArrayTest = 
	"<?xml version=\"1.0\" encoding=\"ISO-8859-1\" ?>"
	"<Values>"
		"<item>0</item>"
		"<item>1</item>"
		"<item>2</item>"
		"<item>3</item>"
	"</Values>";

static const char* sXMLEmbeddedArrayTest = 
	"<?xml version=\"1.0\" encoding=\"ISO-8859-1\" ?>"
	"<Parent>"
		"<Value>0</Value>"
		"<Value>1</Value>"
		"<Value>2</Value>"
		"<Value>3</Value>"
	"</Parent>";

static const char* sXMLRawArrayTest = 
	"<?xml version=\"1.0\" encoding=\"ISO-8859-1\" ?>"
	"<Values>0 1 2 3 4 5 6 7 8 9</Values>";


struct oXMLArrayTest
{
	std::vector<int> IntValues;
};
oRTTI_COMPOUND_DECLARATION(oRTTI_CAPS_NONE, oXMLArrayTest)

struct oXMLEmbeddedArrayTest
{
	std::vector<int> IntValues;
};
oRTTI_COMPOUND_DECLARATION(oRTTI_CAPS_NONE, oXMLEmbeddedArrayTest)

struct oXMLRawArrayTest
{
	std::vector<int> IntValues;
};
oRTTI_COMPOUND_DECLARATION(oRTTI_CAPS_NONE, oXMLRawArrayTest)

oRTTI_COMPOUND_BEGIN_DESCRIPTION(oRTTI_CAPS_ARRAY, oXMLArrayTest)
	oRTTI_COMPOUND_ABSTRACT(oXMLArrayTest)
	oRTTI_COMPOUND_VERSION(oXMLArrayTest, 0,1,0,0)
	oRTTI_COMPOUND_ATTRIBUTES_BEGIN(oXMLArrayTest)
		oRTTI_COMPOUND_ATTR(oXMLArrayTest, IntValues,			oRTTI_OF(std_vector_int),		"Values",		oRTTI_COMPOUND_ATTR_REGULAR | oRTTI_COMPOUND_ATTR_XML_STYLE_NODE)
	oRTTI_COMPOUND_ATTRIBUTES_END(oXMLArrayTest)
oRTTI_COMPOUND_END_DESCRIPTION(oXMLArrayTest)

oRTTI_COMPOUND_BEGIN_DESCRIPTION(oRTTI_CAPS_ARRAY, oXMLEmbeddedArrayTest)
	oRTTI_COMPOUND_ABSTRACT(oXMLEmbeddedArrayTest)
	oRTTI_COMPOUND_VERSION(oXMLEmbeddedArrayTest, 0,1,0,0)
	oRTTI_COMPOUND_ATTRIBUTES_BEGIN(oXMLEmbeddedArrayTest)
		oRTTI_COMPOUND_ATTR(oXMLEmbeddedArrayTest, IntValues,	oRTTI_OF(std_vector_int),		"Value",		oRTTI_COMPOUND_ATTR_REGULAR | oRTTI_COMPOUND_ATTR_XML_STYLE_NODE_EMBEDDED)
	oRTTI_COMPOUND_ATTRIBUTES_END(oXMLEmbeddedArrayTest)
oRTTI_COMPOUND_END_DESCRIPTION(oXMLEmbeddedArrayTest)

oRTTI_COMPOUND_BEGIN_DESCRIPTION(oRTTI_CAPS_ARRAY, oXMLRawArrayTest)
	oRTTI_COMPOUND_ABSTRACT(oXMLRawArrayTest)
	oRTTI_COMPOUND_VERSION(oXMLRawArrayTest, 0,1,0,0)
	oRTTI_COMPOUND_ATTRIBUTES_BEGIN(oXMLRawArrayTest)
		oRTTI_COMPOUND_ATTR(oXMLRawArrayTest, IntValues,		oRTTI_OF(std_vector_int),		"Values",		oRTTI_COMPOUND_ATTR_REGULAR | oRTTI_COMPOUND_ATTR_XML_STYLE_RAW_ARRAY)
	oRTTI_COMPOUND_ATTRIBUTES_END(oXMLRawArrayTest)
oRTTI_COMPOUND_END_DESCRIPTION(oXMLRawArrayTest)


static std::shared_ptr<xml> InitXML(const char* _pXMLString)
{
	// Exercise the buffer as if it were loaded from a file...
	size_t size = strlen(_pXMLString)+1;
	char* pBuffer = (char*)malloc(size);
	strlcpy(pBuffer, _pXMLString, size);

	try { return std::move(std::make_shared<xml>("Test XML Document", (char*)pBuffer, [](const char* _pXMLString){ free((void*)_pXMLString); }, 100000)); }
	catch (std::exception& e)
	{
		oErrorSetLast(e);
		return nullptr;
	}
}

static bool TestArray()
{
	std::shared_ptr<xml> XML = InitXML(sXMLArrayTest);
	if (!XML)
		return false; // Pass through error

	oXMLArrayTest Test;
	Test.IntValues.resize(4);

	oXMLReadCompound(&Test, oRTTI_OF(oXMLArrayTest), *XML, XML->root(), true);

	for (int i=0; i<oInt(Test.IntValues.size()); ++i)
	{
		oTESTB(i==Test.IntValues[i], "Error reading Array value at index %d, expected %d got %d",i,i,Test.IntValues[i]);
	}

	return true;
}

static bool TestEmbeddedArray()
{
	std::shared_ptr<xml> XML = InitXML(sXMLEmbeddedArrayTest);
	if (!XML)
		return false; // Pass through error

	oXMLEmbeddedArrayTest Test;
	Test.IntValues.resize(4);

	oXMLReadCompound(&Test, oRTTI_OF(oXMLEmbeddedArrayTest), *XML, XML->first_child(XML->root(), "Parent"), true);

	for (int i=0; i<oInt(Test.IntValues.size()); ++i)
	{
		oTESTB(i==Test.IntValues[i], "Error reading Embedded Array value at index %d, expected %d got %d",i,i,Test.IntValues[i]);
	}

	return true;
}

static bool TestRawArray()
{
	std::shared_ptr<xml> XML = InitXML(sXMLRawArrayTest);
	if (!XML)
		return false; // Pass through error

	oXMLRawArrayTest Test;
	Test.IntValues.resize(10);

	oXMLReadCompound(&Test, oRTTI_OF(oXMLRawArrayTest), *XML, XML->root(), true);

	for (int i=0; i<oInt(Test.IntValues.size()); ++i)
	{
		oTESTB(i==Test.IntValues[i], "Error reading Raw Array value at index %d, expected %d got %d",i,i,Test.IntValues[i]);
	}

	return true;
}

bool oBasisTest_oXMLSerialize()
{
	if (!TestArray())
		return false; // Pass through error

	if (!TestEmbeddedArray())
		return false; // Pass through error

	if (!TestRawArray())
		return false; // Pass through error

	oErrorSetLast(0, "");
	return true;
}
