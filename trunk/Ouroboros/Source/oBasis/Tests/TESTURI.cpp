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
#include <oBasis/oURI.h>
#include <oBasis/oError.h>
#include "oBasisTestCommon.h"
#include <cstring>

static const char* sTestUris[] = 
{
	"file:///C:/trees/sys3/int/src/core/tests/TestString.cpp",
	"http://msdn.microsoft.com/en-us/library/bb982727.aspx#regexgrammar",
	"file://abyss/trees/sys3/int/src/core/all%20tests/TestString.cpp",
};

static const oURIParts sExpectedParts[] = 
{
	oURIParts("file", "", "C:/trees/sys3/int/src/core/tests/TestString.cpp", nullptr, nullptr),
	oURIParts("http", "msdn.microsoft.com", "en-us/library/bb982727.aspx", nullptr, "regexgrammar"),
	oURIParts("file", "abyss", "trees/sys3/int/src/core/all tests/TestString.cpp", nullptr, nullptr),
};

static const oStringURI sExpectedPaths[] = 
{
	oStringURI("C:\\trees\\sys3\\int\\src\\core\\tests\\TestString.cpp"),
	oStringURI(""),
	oStringURI("\\\\abyss\\trees\\sys3\\int\\src\\core\\all tests\\TestString.cpp"),
};

bool oBasisTest_oURI()
{
	oURIParts parts;
	for (size_t i = 0; i < oCOUNTOF(sTestUris); i++)
	{
		if (!oURIDecompose(sTestUris[i], &parts))
			return oErrorSetLast(oERROR_GENERIC, "decompose failed: %s", sTestUris[i]);
		
		#define oURITESTB(_Part) if (oStrcmp(parts._Part, sExpectedParts[i]._Part)) return oErrorSetLast(oERROR_GENERIC, "Did not get expected results for " #_Part " %s", sTestUris[i])
		oURITESTB(Scheme);
		oURITESTB(Authority);
		oURITESTB(Path);
		oURITESTB(Query);
		oURITESTB(Fragment);

		oStringURI testPath;
		oStringURI expectedCleaned;
		oURIToPath(testPath, sTestUris[i]);

		if (oIsUNCPath(sExpectedPaths[i]))
			oStrcpy(expectedCleaned, sExpectedPaths[i].c_str());
		else
			oCleanPath(expectedCleaned, sExpectedPaths[i]);

		oTESTB(oStrncmp(testPath, expectedCleaned, testPath.capacity()) == 0, "URI %d did not convert back to a path properly. got %s expected %s", i, testPath.c_str(), sExpectedPaths[i]);

		oURIPartsToPath(testPath, parts);
		oTESTB(oStrncmp(testPath, expectedCleaned, testPath.capacity()) == 0, "URIParts %d did not convert back to a path properly. got %s expected %s", i, testPath.c_str(), sExpectedPaths[i]);

		oStringURI reconstructedURI;
		oURIRecompose(reconstructedURI, sExpectedParts[i]);
		oTESTB(oStrncmp(reconstructedURI, sTestUris[i], reconstructedURI.capacity()) == 0, "URI parts %d did not convert back to proper uri. got %s expected %s", i, reconstructedURI, sTestUris[i]);

		oURI A, B, C;
		A = "FiLe:///C:\\Test/../A file.TxT";
		B = "file:///c:/a%20file.txt";
		C = "ftp:///c:/a%20file.txt";

		oTESTB(A == B, "oURI equality test failed");
		oTESTB(B != C, "oURI inequality test failed");
	}

	oErrorSetLast(oERROR_NONE, "");
	return true;
}

