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
#include <oBasis/oINI.h>
#include <oBasis/oRef.h>
#include <oBasis/oString.h>
#include <cstring>
#include "oBasisTestCommon.h"

static const char* sExpectedSectionNames[] = { "CD0", "CD1", "CD2", };
static const char* sExpectedArtists[] = { "Bob Dylan", "Bonnie Tyler", "Dolly Parton", };
static const char* sExpectedYears[] = { "1985", "1988", "1982", };
static const char* sExpectedCD2Values[] = { "Greatest Hits", "Dolly Parton", "USA", "RCA", "9.90", "1982" };

static const char* sTestINI = {
	"[CD0]\n" \
	"Title=Empire Burlesque\n" \
	"Artist=Bob Dylan\n" \
	"Country=USA\n" \
	"Company=Columbia\n" \
	"Price=10.90\n" \
	"Year=1985\n" \
	"\n" \
	"[CD1]\n" \
	"Title=Hide your heart\n" \
	"Artist=Bonnie Tyler\n" \
	"Country=UK\n" \
	"Company=CBS Records\n" \
	"Price=9.90\n" \
	"Year=1988\n" \
	"\n" \
	"[CD2]\n" \
	"Title=Greatest Hits\n" \
	"Artist=Dolly Parton\n" \
	"Country=USA\n" \
	"Company=RCA\n" \
	"Price=9.90\n" \
	"Year=1982\n"
};

static const char* sTestINI_RN = {
	"[CD0]\r\n" \
	"Title=Empire Burlesque\r\n" \
	"Artist=Bob Dylan\r\n" \
	"Country=USA\r\n" \
	"Company=Columbia\r\n" \
	"Price=10.90\r\n" \
	"Year=1985\r\n" \
	"\r\n" \
	"[CD1]\r\n" \
	"Title=Hide your heart\r\n" \
	"Artist=Bonnie Tyler\r\n" \
	"Country=UK\r\n" \
	"Company=CBS Records\r\n" \
	"Price=9.90\r\n" \
	"Year=1988\r\n" \
	"\r\n" \
	"[CD2]\r\n" \
	"Title=Greatest Hits\r\n" \
	"Artist=Dolly Parton\r\n" \
	"Country=USA\r\n" \
	"Company=RCA\r\n" \
	"Price=9.90\r\n" \
	"Year=1982\r\n" \
	"\r\n"
};

bool oBasisTest_TestINI(const char *_pINIData)
{
	const char* ININame = "Test INI";

	oRef<threadsafe oINI> ini;
	oTESTB(oINICreate(ININame, _pINIData, &ini), "Failed to create ini from %s", ININame);

	bool AtLeastOneExecuted = false;
	int i = 0;
	for (oINI::HSECTION hSection = ini->GetFirstSection(); hSection; hSection = ini->GetNextSection(hSection), i++)
	{
		oTESTB(!oStrcmp(ini->GetSectionName(hSection), sExpectedSectionNames[i]), "%s: Name of %d%s section is incorrect", ININame, i, oOrdinal(i));
		oTESTB(!oStrcmp(ini->GetValue(hSection, "Artist"), sExpectedArtists[i]), "%s: Artist in %d%s section did not match", ININame, i, oOrdinal(i));
		oTESTB(!oStrcmp(ini->GetValue(hSection, "Year"), sExpectedYears[i]), "%s: Year in %d%s section did not match", ININame, i, oOrdinal(i));
		AtLeastOneExecuted = true;
	}

	oTESTB(AtLeastOneExecuted, "No sections were read");

	oINI::HSECTION hSection = ini->GetSection("CD2");
	oTESTB(hSection, "Section [CD2] could not be read");
	i = 0;
	for (oINI::HENTRY hEntry = ini->GetFirstEntry(hSection); hEntry; hEntry = ini->GetNextEntry(hEntry), i++)
		oTESTB(!oStrcmp(ini->GetValue(hEntry), sExpectedCD2Values[i]), "%s: Reading CD2 %d%s entry.", ININame, i, oOrdinal(i));

	oErrorSetLast(oERROR_NONE, "");
	return true;
}
bool oBasisTest_oINI()
{
	bool bSuccess = true;
	bSuccess = oBasisTest_TestINI(sTestINI);
	bSuccess = oBasisTest_TestINI(sTestINI_RN);
	return bSuccess;
}
