// $(header)
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
		strcpy_s(pBuffer, size, sTestXML);

		oXML::DESC d;
		d.DocumentName = "Test XML Document";
		d.XMLString = pBuffer;
		d.EstimatedNumNodes = 100;
		d.EstimatedNumAttributes = 1000;
		d.FreeXMLString = FreeBuffer;
		d.CopyXMLString = false;
		oTESTB(oXMLCreate(d, &xml), "Failed to create xml from %s", d.DocumentName);
	}

	oXML::HNODE hCatalog = xml->GetFirstChild(0, "CATALOG");
	oTESTB(hCatalog, "Cannot find CATALOG node");
	oTESTB(!strcmp(xml->FindAttributeValue(hCatalog, "title"), "My play list"), "CATALOG title is incorrect");

	int count = -1;
	oTESTB(xml->GetTypedAttributeValue(hCatalog, "count", &count) && count == 3, "Failed to get a valid count");

	int i = 0;
	for (oXML::HNODE n = xml->GetFirstChild(hCatalog); n; n = xml->GetNextSibling(n), i++)
	{
		oTESTB(!strcmp(xml->GetNodeName(n), i == 1 ? "SpecialCD" : "CD"), "Invalid node name %s", xml->GetNodeName(n));
		oXML::HNODE hArtist = xml->GetFirstChild(n, "ARTIST");
		oTESTB(hArtist, "Invalid CD structure");
		oTESTB(!strcmp(xml->GetNodeValue(hArtist), sExpectedArtists[i]), "Artist in %d%s section did not match", i, oOrdinal(i));
	}

	oErrorSetLast(oERROR_NONE, "");
	return true;
}
