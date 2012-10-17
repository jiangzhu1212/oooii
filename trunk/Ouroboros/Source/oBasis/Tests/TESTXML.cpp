/**************************************************************************
 * The MIT License                                                        *
 * Copyright (c) 2011 Antony Arciuolo & Kevin Myers                       *
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
#include <oBasis/oRef.h>
#include <oBasis/oString.h>
#include <oBasis/oXML.h>
#include "oBasisTestCommon.h"

static const char* sExpectedArtists[] = { "Bob Dylan", "Bonnie Tyler", "Dolly Parton", };

static const char* sTestXML = {
	"<?xml version=\"1.0\" encoding=\"ISO-8859-1\" ?>" \
	"<!--  Edited by XMLSpy -->" \
	"<CATALOG title=\"My play list\" count=\"3\">" \
	"	<CD>" \
	"		<TITLE>Empire Burlesque</TITLE>" \
	"		<ARTIST>Bob Dylan</ARTIST>" \
	"		<COUNTRY>USA</COUNTRY>" \
	"		<COMPANY>Columbia</COMPANY>" \
	"		<PRICE>10.90</PRICE>" \
	"		<YEAR>1985</YEAR>" \
	"	</CD>" \
	"	<SpecialCD>" \
	"		<TITLE>Hide your heart</TITLE>" \
	"		<ARTIST>Bonnie Tyler</ARTIST>" \
	"		<COUNTRY>UK</COUNTRY>" \
	"		<COMPANY>CBS Records</COMPANY>" \
	"		<PRICE>9.90</PRICE>" \
	"		<YEAR>1988</YEAR>" \
	"	</SpecialCD>" \
	"	<CD>" \
	"		<TITLE>Greatest Hits</TITLE>" \
	"		<ARTIST>Dolly Parton</ARTIST>" \
	"		<COUNTRY>USA</COUNTRY>" \
	"		<COMPANY>RCA</COMPANY>" \
	"		<PRICE>9.90</PRICE>" \
	"		<YEAR>1982</YEAR>" \
	"	</CD>" \
	"</CATALOG>"
};

void FreeBuffer(const char* _XMLString) { free((void*)_XMLString); }

bool oBasisTest_oXML()
{
	oRef<threadsafe oXML> xml;
	{
		// Exercise the buffer as if it were loaded from a file...
		size_t size = strlen(sTestXML)+1;
		char* pBuffer = (char*)malloc(size);
		oStrcpy(pBuffer, size, sTestXML);

		oXML::DESC d;
		d.DocumentName = "Test XML Document";
		d.XMLString = pBuffer;
		d.EstimatedNumNodes = 100;
		d.EstimatedNumAttributes = 1000;
		d.FreeXMLString = FreeBuffer;
		d.CopyXMLString = false;
		oTESTB0(oXMLCreate(d, &xml));
	}

	oXML::HNODE hCatalog = xml->GetFirstChild(0, "CATALOG");
	oTESTB(hCatalog, "Cannot find CATALOG node");
	oTESTB(!oStrcmp(xml->FindAttributeValue(hCatalog, "title"), "My play list"), "CATALOG title is incorrect");

	int count = -1;
	oTESTB(xml->GetTypedAttributeValue(hCatalog, "count", &count) && count == 3, "Failed to get a valid count");

	int i = 0;
	for (oXML::HNODE n = xml->GetFirstChild(hCatalog); n; n = xml->GetNextSibling(n), i++)
	{
		oTESTB(!oStrcmp(xml->GetNodeName(n), i == 1 ? "SpecialCD" : "CD"), "Invalid node name %s", xml->GetNodeName(n));
		oXML::HNODE hArtist = xml->GetFirstChild(n, "ARTIST");
		oTESTB(hArtist, "Invalid CD structure");
		oTESTB(!oStrcmp(xml->GetNodeValue(hArtist), sExpectedArtists[i]), "Artist in %d%s section did not match", i, oOrdinal(i));
	}

	// Test compacted XML
	oXML::DESC d;
	d.DocumentName = "Test CompactedXML";
	d.XMLString = "<html><head><title>foo bar</title></head><body/></html>";
	d.CopyXMLString = true;
	oTESTB0(oXMLCreate(d, &xml));

	oXML::HNODE HeadNode = xml->GetFirstChild(xml->GetFirstChild(0), "head");
	oTESTB(0 == oStricmp(xml->GetNodeName(HeadNode), "head"), "Failed to get head node");
	
	oXML::HNODE Title = xml->GetFirstChild(HeadNode, "title");
	oTESTB(0 == oStricmp(xml->GetNodeValue(Title), "foo bar"), "Title is wrong");
	oErrorSetLast(oERROR_NONE, "");
	return true;
}
