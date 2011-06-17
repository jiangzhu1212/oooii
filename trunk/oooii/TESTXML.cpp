// $(header)
#include <oooii/oXML.h>
#include <oooii/oFile.h>
#include <oooii/oRef.h>
#include <oooii/oString.h>
#include <oooii/oTest.h>

static const char* sExpectedArtists[] = { "Bob Dylan", "Bonnie Tyler", "Dolly Parton", };

struct TESTXML : public oTest
{
	RESULT Run(char* _StrStatus, size_t _SizeofStrStatus) override
	{
		char xmlPath[_MAX_PATH];
		oTESTB(oTestManager::Singleton()->FindFullPath(xmlPath, "test.xml"), "Failed to find test.xml");

		oRef<threadsafe oXML> xml;
		{
			char* pBuffer = 0;
			size_t size = 0;
			oTESTB(oFile::LoadBuffer((void**)&pBuffer, &size, malloc, xmlPath, true), "Failed to load %s", xmlPath);
			oTESTB(oXML::Create(xmlPath, pBuffer, 50, 50, &xml), "Failed to create xml from %s", xmlPath);
			free(pBuffer);
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

		return SUCCESS;
	}
};

oTEST_REGISTER(TESTXML);
