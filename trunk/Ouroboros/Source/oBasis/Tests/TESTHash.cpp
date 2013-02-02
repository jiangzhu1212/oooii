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
#include <oBasisTests/oBasisTests.h>
#include <oBasis/oFixedString.h>
#include <oBasis/oOnScopeExit.h>
#include "oBasisTestCommon.h"

bool oBasisTest_oHash(const oBasisTestServices& _Services)
{
	static const char* TestFile = "Test/Textures/lena_1.png";

	oStringPath path;
	if (!_Services.ResolvePath(path, path.capacity(), TestFile, true))
		return oErrorSetLast(oERROR_NOT_FOUND, "not found: %s", TestFile);

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
