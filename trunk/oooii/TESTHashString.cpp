// $(header)
#include <oooii/oString.h>
#include <oooii/oHashString.h>
#include <oooii/oTest.h>

struct TESTHashString : public oTest
{
	RESULT Run(char* _StrStatus, size_t _SizeofStrStatus) override
	{
		const char* testHashStr = "TestHashString";

		oHashString hashStr("TestHashString");
		oHashString sameHashStr("TestHashString");
		oHashString lowercaseHashStr("testhashstring");
		oHashString hashStr2("TestHashString2");
		oHashString longHashStr("SuperExtraReallyUnnecessarilyOverlyYetUnderstandablyLongString1");
		oHashString everSoSlightlyDifferentButStillReallyLongHashStr("SuperExtraReallyUnnecessarilyOverlyYetUnderstandablyLongString2");
		oHashString emptyHashStr("");

		oTESTB(hashStr == sameHashStr, "Hash String equality test failed.");
		oTESTB(hashStr != hashStr2, "Hash String inequality test failed.");
		oTESTB(hashStr == hashStr, "Same Hash String equality test failed.");
		oTESTB(hashStr == lowercaseHashStr, "Lowercase equality test failed.");
		oTESTB(longHashStr != everSoSlightlyDifferentButStillReallyLongHashStr, "Very long Hash String inequality test failed.");
		oTESTB(hashStr < hashStr2 || hashStr2 < hashStr, "Hash String < test failed.");
		oTESTB(hashStr > hashStr2 || hashStr2 > hashStr, "Hash String > test failed.");
		oTESTB(hashStr != emptyHashStr, "Empty Hash inequality test failed.");

		oHashString tempHashStr;
		tempHashStr.Set(longHashStr.GetHash());
		oTESTB(tempHashStr == longHashStr, "Set(tHash) failed.");

		tempHashStr.Set(testHashStr);
		oTESTB(tempHashStr == hashStr, "Set(const char*) failed.");

		return SUCCESS;
	}
};

oTEST_REGISTER(TESTHashString);
