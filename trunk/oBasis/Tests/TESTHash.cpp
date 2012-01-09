// $(header)
#include <oBasisTests/oBasisTests.h>
#include <oBasis/oFixedString.h>
#include <oBasis/oOnScopeExit.h>
#include "oBasisTestCommon.h"

bool oBasisTest_oHash(const oBasisTestServices& _Services)
{
	static const char* TestFile = "Test/Textures/lena_1.png";

	oStringPath path;
	if (!_Services.ResolvePath(path, path.capacity(), TestFile))
		return oErrorSetLast(oERROR_NOT_FOUND, "%s not found", TestFile);

	void* pBuffer = nullptr;
	size_t Size = 0;
	oTESTB(_Services.AllocateAndLoadBuffer(&pBuffer, &Size, path, false), "%s not loaded", path.c_str());
	oOnScopeExit FreeBuffer([&] { _Services.DeallocateLoadedBuffer(pBuffer); });

	const uint128 ExpectedHash = {5036109567207105818, 7580512480722386524};
	uint128 ComputedHash = oHash_murmur3_x64_128(pBuffer, static_cast<unsigned int>(Size));
	oTESTB(ExpectedHash == ComputedHash, "Hash doesn't match");

	oErrorSetLast(oERROR_NONE);
	return true;
}
