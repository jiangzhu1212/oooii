// $(header)
#include <oBasis/oURI.h>
#include <oBasis/oError.h>
#include <cstring>

static const char* sTestUris[] = 
{
	"file:///C:/trees/sys3/int/src/core/tests/TestString.cpp",
	"http://msdn.microsoft.com/en-us/library/bb982727.aspx#regexgrammar",
};

static const oURIParts sExpectedParts[] = 
{
	oURIParts("file", "", "C:/trees/sys3/int/src/core/tests/TestString.cpp", nullptr, nullptr),
	oURIParts("http", "msdn.microsoft.com", "en-us/library/bb982727.aspx", nullptr, "regexgrammar"),
};

bool oBasisTest_oURI()
{
	oURIParts parts;
	for (size_t i = 0; i < oCOUNTOF(sTestUris); i++)
	{
		if (!oURIDecompose(sTestUris[i], &parts))
			return oErrorSetLast(oERROR_GENERIC, "decompose failed: %s", sTestUris[i]);
		
		#define oURITESTB(_Part) if (strcmp(parts._Part, sExpectedParts[i]._Part)) return oErrorSetLast(oERROR_GENERIC, "Did not get expected results for " #_Part " %s", sTestUris[i])
		oURITESTB(Scheme);
		oURITESTB(Authority);
		oURITESTB(Path);
		oURITESTB(Query);
		oURITESTB(Fragment);
	}

	oErrorSetLast(oERROR_NONE, "");
	return true;
}

