// $(header)
#include <oooii/oURI.h>
#include <oooii/oTest.h>

static const char* sTestUris[] = 
{
	"file:///C:/trees/sys3/int/src/core/tests/TestString.cpp",
	"http://msdn.microsoft.com/en-us/library/bb982727.aspx#regexgrammar",
};

static const oURI::Decomposition sExpectedParts[] = 
{
	oURI::Decomposition("file", "", "C:/trees/sys3/int/src/core/tests/TestString.cpp"),
	oURI::Decomposition("http", "msdn.microsoft.com", "en-us/library/bb982727.aspx", 0, "regexgrammar"),
};

struct TESTURI : public oTest
{
	RESULT Run(char* _StrStatus, size_t _SizeofStrStatus) override
	{
		oURI::Decomposition parts;

		for (size_t i = 0; i < oCOUNTOF(sTestUris); i++)
		{
			oTESTB(0 == oURI::Decompose(sTestUris[i], parts), "decompose failed: %s", sTestUris[i]);
			oTESTB(parts == sExpectedParts[i], "Did not get expected results for %s", sTestUris[i]);
		}

		return SUCCESS;
	}
};

oTEST_REGISTER(TESTURI);
