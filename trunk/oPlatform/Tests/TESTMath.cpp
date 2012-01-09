// $(header)
#include <oBasisTests/oBasisTests.h>
#include <oPlatform/oTest.h>

struct TESTMath : public oTest
{
	RESULT Run(char* _StrStatus, size_t _SizeofStrStatus) override
	{
		oTESTB(oBasisTest_oMath(), "%s", oErrorGetLastString());
		return SUCCESS;
	}
};

oTEST_REGISTER(TESTMath);
