/**************************************************************************
 * The MIT License                                                        *
 * Copyright (c) 2013 Antony Arciuolo.                                    *
 * arciuolo@gmail.com                                                     *
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
#include <oBasis/tests/oBasisTests.h>
#include <oBase/fixed_string.h>
#include <oBase/finally.h>
#include <oBase/murmur3.h>
#include "oBasisTestCommon.h"

using namespace ouro;

bool oBasisTest_oHash(const oBasisTestServices& _Services)
{
	static const char* TestFile = "Test/Textures/lena_1.png";

	path path;
	if (!_Services.ResolvePath(path, TestFile, true))
		return oErrorSetLast(std::errc::no_such_file_or_directory, "not found: %s", TestFile);

	void* pBuffer = nullptr;
	size_t Size = 0;
	oTESTB(_Services.AllocateAndLoadBuffer(&pBuffer, &Size, path, false), "%s not loaded", path.c_str());
	finally FreeBuffer([&] { _Services.DeallocateLoadedBuffer(pBuffer); });

	const uint128 ExpectedHash(1358230486757162696ull, 12886233335009572520ull);
	uint128 ComputedHash = murmur3(pBuffer, Size);
	oTESTB(ExpectedHash == ComputedHash, "Hash doesn't match");

	oErrorSetLast(0);
	return true;
}
