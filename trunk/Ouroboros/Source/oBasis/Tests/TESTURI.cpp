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
#include <oBasis/oURI.h>
#include <oBasis/oError.h>
#include <oBasis/oPath.h>
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
	oURIParts("file", "", "/C:/trees/sys3/int/src/core/tests/TestString.cpp", nullptr, nullptr),
	oURIParts("http", "msdn.microsoft.com", "/en-us/library/bb982727.aspx", nullptr, "regexgrammar"),
	oURIParts("file", "abyss", "/trees/sys3/int/src/core/all tests/TestString.cpp", nullptr, nullptr),
};

static const oStd::uri_string sExpectedPaths[] = 
{
	oStd::uri_string("C:\\trees\\sys3\\int\\src\\core\\tests\\TestString.cpp"),
	oStd::uri_string("\\\\msdn.microsoft.com\\en-us\\library\\bb982727.aspx"), // this doesn't make sense from an http server best-practices view, but this is what's expected
	oStd::uri_string("\\\\abyss\\trees\\sys3\\int\\src\\core\\all tests\\TestString.cpp"),
};

typedef std::pair<oStd::uri_string,oStd::uri_string> oStringURIPair;

static const oStd::uri_string sReferenceResolutionBaseURI = oStd::uri_string("http://a/b/c/d;p?q");
static const oStringURIPair sReferenceResolutionExamples[] =
{
	// http://tools.ietf.org/html/rfc3986#section-5.4.1
	oStringURIPair(oStd::uri_string("g:h"	   ), oStd::uri_string("g:h")),
	oStringURIPair(oStd::uri_string("g"      ), oStd::uri_string("http://a/b/c/g")),
	oStringURIPair(oStd::uri_string("./g"    ), oStd::uri_string("http://a/b/c/g")),
	oStringURIPair(oStd::uri_string("g/"     ), oStd::uri_string("http://a/b/c/g/")),
	oStringURIPair(oStd::uri_string("/g"     ), oStd::uri_string("http://a/g")),
	oStringURIPair(oStd::uri_string("//g"    ), oStd::uri_string("http://g")),
	oStringURIPair(oStd::uri_string("?y"     ), oStd::uri_string("http://a/b/c/d;p?y")),
	oStringURIPair(oStd::uri_string("g?y"    ), oStd::uri_string("http://a/b/c/g?y")),
	oStringURIPair(oStd::uri_string("#s"     ), oStd::uri_string("http://a/b/c/d;p?q#s")),
	oStringURIPair(oStd::uri_string("g#s"    ), oStd::uri_string("http://a/b/c/g#s")),
	oStringURIPair(oStd::uri_string("g?y#s"  ), oStd::uri_string("http://a/b/c/g?y#s")),
	oStringURIPair(oStd::uri_string(";x"     ), oStd::uri_string("http://a/b/c/;x")),
	oStringURIPair(oStd::uri_string("g;x"    ), oStd::uri_string("http://a/b/c/g;x")),
	oStringURIPair(oStd::uri_string("g;x?y#s"), oStd::uri_string("http://a/b/c/g;x?y#s")),
	oStringURIPair(oStd::uri_string(""       ), oStd::uri_string("http://a/b/c/d;p?q")),
	oStringURIPair(oStd::uri_string("."      ), oStd::uri_string("http://a/b/c/")),
	oStringURIPair(oStd::uri_string("./"     ), oStd::uri_string("http://a/b/c/")),
	oStringURIPair(oStd::uri_string(".."     ), oStd::uri_string("http://a/b/")),
	oStringURIPair(oStd::uri_string("../"    ), oStd::uri_string("http://a/b/")),
	oStringURIPair(oStd::uri_string("../g"   ), oStd::uri_string("http://a/b/g")),
	oStringURIPair(oStd::uri_string("../.."  ), oStd::uri_string("http://a/")),
	oStringURIPair(oStd::uri_string("../../" ), oStd::uri_string("http://a/")),
	oStringURIPair(oStd::uri_string("../../g"), oStd::uri_string("http://a/g")),

	// http://tools.ietf.org/html/rfc3986#section-5.4.2
	oStringURIPair(oStd::uri_string("../../../g"   ), oStd::uri_string("http://a/g")),
	oStringURIPair(oStd::uri_string("../../../../g"), oStd::uri_string("http://a/g")),
	oStringURIPair(oStd::uri_string("/./g"         ), oStd::uri_string("http://a/g")),
	oStringURIPair(oStd::uri_string("/../g"        ), oStd::uri_string("http://a/g")),
//	oStringURIPair(oStd::uri_string("g."           ), oStd::uri_string("http://a/b/c/g.")), // FIXME
	oStringURIPair(oStd::uri_string(".g"           ), oStd::uri_string("http://a/b/c/.g")),
//	oStringURIPair(oStd::uri_string("g.."          ), oStd::uri_string("http://a/b/c/g..")), // FIXME
	oStringURIPair(oStd::uri_string("..g"          ), oStd::uri_string("http://a/b/c/..g")),
	oStringURIPair(oStd::uri_string("./../g"       ), oStd::uri_string("http://a/b/g")),
	oStringURIPair(oStd::uri_string("./g/."        ), oStd::uri_string("http://a/b/c/g/")),
	oStringURIPair(oStd::uri_string("g/./h"        ), oStd::uri_string("http://a/b/c/g/h")),
	oStringURIPair(oStd::uri_string("g/../h"       ), oStd::uri_string("http://a/b/c/h")),
	oStringURIPair(oStd::uri_string("g;x=1/./y"    ), oStd::uri_string("http://a/b/c/g;x=1/y")),
	oStringURIPair(oStd::uri_string("g;x=1/../y"   ), oStd::uri_string("http://a/b/c/y")),
	oStringURIPair(oStd::uri_string("g?y/./x"      ), oStd::uri_string("http://a/b/c/g?y/./x")),
	oStringURIPair(oStd::uri_string("g?y/../x"     ), oStd::uri_string("http://a/b/c/g?y/../x")),
	oStringURIPair(oStd::uri_string("g#s/./x"      ), oStd::uri_string("http://a/b/c/g#s/./x")),
	oStringURIPair(oStd::uri_string("g#s/../x"     ), oStd::uri_string("http://a/b/c/g#s/../x")),
};

static const oStd::uri_string sReferenceResolutionBaseURI2 = oStd::uri_string("file://DATA/Test/Scenes/TestTextureSet.xml");
static const oStringURIPair sReferenceResolutionExamples2[] =
{
	oStringURIPair(oStd::uri_string("/test.xml"),					oStd::uri_string("file://DATA/test.xml")),
	oStringURIPair(oStd::uri_string("#Node"),						oStd::uri_string("file://DATA/Test/Scenes/TestTextureSet.xml#Node")),
	oStringURIPair(oStd::uri_string("../Textures/Hatch1.png"),	oStd::uri_string("file://DATA/Test/Textures/Hatch1.png")),
	oStringURIPair(oStd::uri_string("test.xml"),					oStd::uri_string("file://DATA/Test/Scenes/test.xml")),
	oStringURIPair(oStd::uri_string("file://DATA2/test.xml"),		oStd::uri_string("file://DATA2/test.xml")),
	oStringURIPair(oStd::uri_string("http://DATA/test.xml"),		oStd::uri_string("http://DATA/test.xml")),
	oStringURIPair(oStd::uri_string("http://DATA2/test.xml"),		oStd::uri_string("http://DATA2/test.xml")),
};

static const oStd::uri_string sReferenceMakeRelativeBaseURI = oStd::uri_string("file://DATA/Test/Scenes/TestTextureSet.xml");
static const oStringURIPair sReferenceMakeRelativeExamples[] = 
{
	oStringURIPair(oStd::uri_string("file://DATA/Test/Scenes/TestTextureSet.xml"),		oStd::uri_string("TestTextureSet.xml")), // Should be ""
	oStringURIPair(oStd::uri_string("file://DATA/Test/Scenes/TestTextureSet.xml#item"),	oStd::uri_string("TestTextureSet.xml#item")), // Should be "#item"
	oStringURIPair(oStd::uri_string("file://DATA/Test/Scenes/TestTextureSet.xml?query"),	oStd::uri_string("TestTextureSet.xml?query")), // Should be "?query"

	oStringURIPair(oStd::uri_string("file://DATA/Test/Scenes/test/test/test.xml"),		oStd::uri_string("test/test/test.xml")),
	oStringURIPair(oStd::uri_string("file://DATA/Test/Scenes/test/test.xml"),				oStd::uri_string("test/test.xml")),
	oStringURIPair(oStd::uri_string("file://DATA/Test/Scenes/test.xml"),					oStd::uri_string("test.xml")),
	oStringURIPair(oStd::uri_string("file://DATA/Test/test.xml"),							oStd::uri_string("../test.xml")),
	oStringURIPair(oStd::uri_string("file://DATA/test.xml"),								oStd::uri_string("../../test.xml")),
	oStringURIPair(oStd::uri_string("file://DATA/"),										oStd::uri_string("../../")),
};

static bool Test_oURIIsURI()
{
	// Valid URIs
	oTESTB0(oURIIsURI("http:"));
	oTESTB0(oURIIsURI("file:"));
	oTESTB0(oURIIsURI("http:path"));
	oTESTB0(oURIIsURI("file://server/path"));
	oTESTB0(oURIIsURI("file://server/path?query"));
	oTESTB0(oURIIsURI("file://server/path#fragment"));
	// Note that this is actually an unnormalized URI and not valid,
	// however it still counts as URI because it has a scheme by definition.
	// Normalization would have taken out the ..
	oTESTB0(oURIIsURI("file:../path/path2/file.ext"));
	// Colon is not a reserved character in path, fragment and query
	oTESTB0(!oURIIsURI("#file:"));
	oTESTB0(!oURIIsURI("?file:"));
	// However path must start with a / or . (which are reserved characters for scheme)
	oTESTB0(!oURIIsURI("./file:")); 
	oTESTB0(!oURIIsURI("/file:")); 
	// Common URIReferences
	oTESTB0(!oURIIsURI("file.txt"));
	oTESTB0(!oURIIsURI("file.txt#item"));
	oTESTB0(!oURIIsURI("file.txt?query=false"));
	// Note that this is a URIReference without a scheme, not a UNC path
	oTESTB0(!oURIIsURI("//server/file.txt")); 
	return true;
}

static bool Test_oURIIsAbsolute()
{
	// Valid Absolute URIs
	oTESTB0(oURIIsAbsolute("http:"));
	oTESTB0(oURIIsAbsolute("file:"));
	oTESTB0(oURIIsAbsolute("http:path"));
	oTESTB0(oURIIsAbsolute("file://server/path"));
	// Note that this is actually an unnormalized URI and not valid,
	// however it still counts as absolute because it has a scheme by definition.
	// Normalization would have taken out the ..
	oTESTB0(oURIIsAbsolute("file:../path/path2/file.ext"));
	// Absolute URIs can't have fragments
	oTESTB0(!oURIIsAbsolute("file:#file"));
	oTESTB0(!oURIIsAbsolute("file:file#file"));
	oTESTB0(!oURIIsAbsolute("http://server/file#file"));
	// Colon is not a reserved character in path, fragment and query
	oTESTB0(!oURIIsAbsolute("#file:"));
	oTESTB0(!oURIIsAbsolute("?file:"));
	// However path must start with a / or . (which are reserved characters for scheme)
	oTESTB0(!oURIIsAbsolute("./file:")); 
	oTESTB0(!oURIIsAbsolute("/file:")); 
	// Common URIReferences
	oTESTB0(!oURIIsAbsolute("file.txt"));
	oTESTB0(!oURIIsAbsolute("file.txt#item"));
	oTESTB0(!oURIIsAbsolute("file.txt?query=false"));
	// Note that this is a URIReference without a scheme, not a UNC path
	oTESTB0(!oURIIsAbsolute("//server/file.txt")); 
	return true;
}

static bool Test_oURIIsSameDocument()
{
	oTESTB0(oURIIsSameDocument("", "file:///path/path2/file.txt"));
	oTESTB0(oURIIsSameDocument("#some_item", "file:///path/path2/file.txt"));
	oTESTB0(oURIIsSameDocument("file.txt", "file:///path/path2/file.txt"));
	oTESTB0(oURIIsSameDocument("file.txt?modified", "file:///path/path2/file.txt?modified"));
	oTESTB0(oURIIsSameDocument("file.txt#some_item", "file:///path/path2/file.txt"));
	oTESTB0(oURIIsSameDocument("../path2/file.txt#some_item", "file:///path/path2/file.txt"));
	oTESTB0(oURIIsSameDocument("/path/path2/file.txt#some_item", "file:///path/path2/file.txt"));
	oTESTB0(oURIIsSameDocument("file:path/path2/file.txt#some_item", "file:///path/path2/file.txt"));
	oTESTB0(oURIIsSameDocument("file:///path/path2/file.txt#some_item", "file:///path/path2/file.txt"));

	oTESTB0(!oURIIsSameDocument("a", "file:///path/path2/file.txt"));
	oTESTB0(!oURIIsSameDocument("a#some_item", "file:///path/path2/file.txt"));
	oTESTB0(!oURIIsSameDocument("a/file.txt", "file:///path/path2/file.txt"));
	oTESTB0(!oURIIsSameDocument("file.txt?notmodified", "file:///path/path2/file.txt?modified"));
	oTESTB0(!oURIIsSameDocument("../path/file.txt#some_item", "file:///path/path2/file.txt"));
	oTESTB0(!oURIIsSameDocument("/path/path/file.txt#some_item", "file:///path/path2/file.txt"));
	oTESTB0(!oURIIsSameDocument("file:path/path/file.txt#some_item", "file:///path/path2/file.txt"));
	oTESTB0(!oURIIsSameDocument("file:///path/path/file.txt#some_item", "file:///path/path2/file.txt"));
	return true;
}

static bool Test_oURIResolve()
{
	oFORI(i, sReferenceResolutionExamples)
	{
		oStd::uri_string Result;
		oURIResolve(Result, sReferenceResolutionBaseURI, sReferenceResolutionExamples[i].first);
		oTESTB(oStricmp(Result.c_str(), sReferenceResolutionExamples[i].second.c_str())==0, "Unexpected result oURIResolve returned '%s', expected '%s'", Result.c_str(), sReferenceResolutionExamples[i].second.c_str());
	}
	oFORI(i, sReferenceResolutionExamples2)
	{
		oStd::uri_string Result;
		oURIResolve(Result, sReferenceResolutionBaseURI2, sReferenceResolutionExamples2[i].first);
		oTESTB(oStricmp(Result.c_str(), sReferenceResolutionExamples2[i].second.c_str())==0, "Unexpected result oURIResolve returned '%s', expected '%s'", Result.c_str(), sReferenceResolutionExamples2[i].second.c_str());
	}

	return true;
}

static bool Test_oURIRelativize()
{
	oFORI(i, sReferenceMakeRelativeExamples)
	{
		oStd::uri_string Result;
		oURIRelativize(Result, sReferenceMakeRelativeBaseURI, sReferenceMakeRelativeExamples[i].first);
		oTESTB(oStricmp(Result.c_str(), sReferenceMakeRelativeExamples[i].second.c_str())==0, "Unexpected result oURIRelativize returned '%s', expected '%s'", Result.c_str(), sReferenceMakeRelativeExamples[i].second.c_str());
	}
	return true;
}

bool oBasisTest_oURI()
{
	oURIParts parts;
	oFORI(i, sTestUris)
	{
		if (!oURIDecompose(sTestUris[i], &parts))
			return oErrorSetLast(std::errc::protocol_error, "decompose failed: %s", sTestUris[i]);
		
		#define oURITESTB(_Part) if (oStrcmp(parts._Part, sExpectedParts[i]._Part)) return oErrorSetLast(std::errc::protocol_error, "Did not get expected results for " #_Part " %s", sTestUris[i])
		oURITESTB(Scheme);
		oURITESTB(Authority);
		oURITESTB(Path);
		oURITESTB(Query);
		oURITESTB(Fragment);

		oStd::uri_string testPath;
		oStd::uri_string expectedCleaned;
		oURIToPath(testPath, sTestUris[i]);

		if (oIsUNCPath(sExpectedPaths[i]))
			oStrcpy(expectedCleaned, sExpectedPaths[i].c_str());
		else
			oStd::clean_path(expectedCleaned, sExpectedPaths[i]);

		oTESTB(oStrncmp(testPath, expectedCleaned, testPath.capacity()) == 0, "URI %d did not convert back to a path properly. got %s expected %s", i, testPath.c_str(), sExpectedPaths[i]);

		oURIPartsToPath(testPath, parts);
		oTESTB(oStrncmp(testPath, expectedCleaned, testPath.capacity()) == 0, "URIParts %d did not convert back to a path properly. got %s expected %s", i, testPath.c_str(), sExpectedPaths[i]);

		oStd::uri_string reconstructedURI;
		oURIRecompose(reconstructedURI, sExpectedParts[i]);
		oTESTB(oStrncmp(reconstructedURI, sTestUris[i], reconstructedURI.capacity()) == 0, "URI parts %d did not convert back to proper uri. got %s expected %s", i, reconstructedURI, sTestUris[i]);
	}
	
	oURI A, B, C;
	A = "FiLe:///C:\\Test/../A file.TxT";
	B = "file:///c:/a%20file.txt";
	C = "ftp:///c:/a%20file.txt";

	oTESTB(A == B, "oURI equality test failed");
	oTESTB(B != C, "oURI inequality test failed");

	if (!Test_oURIIsURI())
		return false;

	if (!Test_oURIIsAbsolute())
		return false;

	if (!Test_oURIIsSameDocument())
		return false;
	
	if (!Test_oURIResolve())
		return false;

	if (!Test_oURIRelativize())
		return false;

	oErrorSetLast(0, "");
	return true;
}

