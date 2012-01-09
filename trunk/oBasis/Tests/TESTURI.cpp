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

