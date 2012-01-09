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
	"[CD0]" \
	"Title=Empire Burlesque" \
	"Artist=Bob Dylan" \
	"Country=USA" \
	"Company=Columbia" \
	"Price=10.90" \
	"Year=1985" \
	"" \
	"[CD1]" \
	"Title=Hide your heart" \
	"Artist=Bonnie Tyler" \
	"Country=UK" \
	"Company=CBS Records" \
	"Price=9.90" \
	"Year=1988" \
	"" \
	"[CD2]" \
	"Title=Greatest Hits" \
	"Artist=Dolly Parton" \
	"Country=USA" \
	"Company=RCA" \
	"Price=9.90" \
	"Year=1982"
};

bool oBasisTest_oINI()
{
	const char* ININame = "Test INI";

	oRef<threadsafe oINI> ini;
	{
		size_t size = strlen(sTestINI) + 1;
		char* pBuffer = new char[size];
		oTESTB(oINICreate(ININame, pBuffer, &ini), "Failed to create ini from %s", ININame);
		delete [] pBuffer;
	}

	int i = 0;
	for (oINI::HSECTION hSection = ini->GetFirstSection(); hSection; hSection = ini->GetNextSection(hSection), i++)
	{
		oTESTB(!strcmp(ini->GetSectionName(hSection), sExpectedSectionNames[i]), "%s: Name of %d%s section is incorrect", ININame, i, oOrdinal(i));
		oTESTB(!strcmp(ini->GetValue(hSection, "Artist"), sExpectedArtists[i]), "%s: Artist in %d%s section did not match", ININame, i, oOrdinal(i));
		oTESTB(!strcmp(ini->GetValue(hSection, "Year"), sExpectedYears[i]), "%s: Year in %d%s section did not match", ININame, i, oOrdinal(i));
	}

	oINI::HSECTION hSection = ini->GetSection("CD2");
	i = 0;
	for (oINI::HENTRY hEntry = ini->GetFirstEntry(hSection); hEntry; hEntry = ini->GetNextEntry(hEntry), i++)
		oTESTB(!strcmp(ini->GetValue(hEntry), sExpectedCD2Values[i]), "%s: Reading CD2 %d%s entry.", ININame, i, oOrdinal(i));

	oErrorSetLast(oERROR_NONE, "");
	return true;
}
