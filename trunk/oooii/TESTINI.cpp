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
#include "pch.h"
#include <oooii/oINI.h>
#include <oooii/oRef.h>
#include <oooii/oStdio.h>
#include <oooii/oString.h>
#include <oooii/oTest.h>

static const char* sExpectedSectionNames[] = { "CD0", "CD1", "CD2", };
static const char* sExpectedArtists[] = { "Bob Dylan", "Bonnie Tyler", "Dolly Parton", };
static const char* sExpectedYears[] = { "1985", "1988", "1982", };
static const char* sExpectedCD2Values[] = { "Greatest Hits", "Dolly Parton", "USA", "RCA", "9.90", "1982" };

struct TESTINI : public oTest
{
	RESULT Run(char* _StrStatus, size_t _SizeofStrStatus) override
	{
		static const char* sFiles[] = { "test.ini", "test2.ini" };

		for (size_t f = 0; f < oCOUNTOF(sFiles); f++)
		{
			char iniPath[_MAX_PATH];
			oTESTB(oTestManager::Singleton()->FindFullPath(iniPath, sFiles[f]), "Failed to find %s", sFiles[f]);

			threadsafe oRef<oINI> ini;
			{
				char* pBuffer = 0;
				size_t size = 0;
				oTESTB(oLoadBuffer((void**)&pBuffer, &size, malloc, iniPath, true), "Failed to load %s", iniPath);
				oTESTB(oINI::Create(iniPath, pBuffer, &ini), "Failed to create ini from %s", iniPath);
				free(pBuffer);
			}

			int i = 0;
			for (oINI::HSECTION hSection = ini->GetFirstSection(); hSection; hSection = ini->GetNextSection(hSection), i++)
			{
				oTESTB(!strcmp(ini->GetSectionName(hSection), sExpectedSectionNames[i]), "%s: Name of %d%s section is incorrect", sFiles[f], i, oOrdinal(i));
				oTESTB(!strcmp(ini->GetValue(hSection, "Artist"), sExpectedArtists[i]), "%s: Artist in %d%s section did not match", sFiles[f], i, oOrdinal(i));
				oTESTB(!strcmp(ini->GetValue(hSection, "Year"), sExpectedYears[i]), "%s: Year in %d%s section did not match", sFiles[f], i, oOrdinal(i));
			}

			oINI::HSECTION hSection = ini->GetSection("CD2");
			i = 0;
			for (oINI::HENTRY hEntry = ini->GetFirstEntry(hSection); hEntry; hEntry = ini->GetNextEntry(hEntry), i++)
				oTESTB(!strcmp(ini->GetValue(hEntry), sExpectedCD2Values[i]), "%s: Reading CD2 %d%s entry.", sFiles[f], i, oOrdinal(i));
		}

		return SUCCESS;
	}
};

TESTINI TestINI;
