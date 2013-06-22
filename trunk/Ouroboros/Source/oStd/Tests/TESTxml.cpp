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
#include <oStd/xml.h>
#include <oStd/algorithm.h>

namespace oStd {
	namespace tests {

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

static const char* sCompactTestXML = "<html><head><title>foo bar</title></head><body/></html>";

void TESTxml()
{
	std::shared_ptr<oStd::xml> XML = std::make_shared<oStd::xml>("Test XML", (char*)sTestXML, nullptr, 200);

	xml::node hCatalog = XML->first_child(0, "CATALOG");
	oCHECK(hCatalog, "Cannot find CATALOG node");
	oCHECK(!strcmp(XML->find_attr_value(hCatalog, "title"), "My play list"), "CATALOG title is incorrect");

	int count = atoi(XML->find_attr_value(hCatalog, "count"));
	oCHECK(count == 3, "Failed to get a valid count");

	int i = 0;
	for (xml::node n = XML->first_child(hCatalog); n; n = XML->next_sibling(n), i++)
	{
		oCHECK(!strcmp(XML->node_name(n), i == 1 ? "SpecialCD" : "CD"), "Invalid node name %s", XML->node_name(n));
		xml::node hArtist = XML->first_child(n, "ARTIST");
		oCHECK(hArtist, "Invalid CD structure");
		oCHECK(!strcmp(XML->node_value(hArtist), sExpectedArtists[i]), "Artist in %d%s section did not match", i, ordinal(i));
	}

	// Test compacted XML
	XML = std::make_shared<xml>("Test CompactXML", (char*)sCompactTestXML, nullptr, 200);

	xml::node HeadNode = XML->first_child(XML->first_child(0), "head");
	oCHECK(!_stricmp(XML->node_name(HeadNode), "head"), "Failed to get head node");
	
	xml::node Title = XML->first_child(HeadNode, "title");
	oCHECK(!_stricmp(XML->node_value(Title), "foo bar"), "Title is wrong");
}

	} // namespace tests
} // namespace oStd
