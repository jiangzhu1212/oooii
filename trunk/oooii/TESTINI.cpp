// $(header)
#include <oooii/oINI.h>
#include <oooii/oFile.h>
#include <oooii/oRef.h>
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

			oRef<threadsafe oINI> ini;
			{
				char* pBuffer = 0;
				size_t size = 0;
				oTESTB(oFile::LoadBuffer((void**)&pBuffer, &size, malloc, iniPath, true), "Failed to load %s", iniPath);
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

oTEST_REGISTER(TESTINI);
