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
