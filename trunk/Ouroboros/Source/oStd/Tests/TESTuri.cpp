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
#include <oStd/uri.h>

namespace oStd {
	namespace tests {

struct URI_PARTS_TEST
{
	const char* URI;
	const char* Scheme;
	const char* Authority;
	const char* Path;
	const char* Query;
	const char* Fragment;
};

static const URI_PARTS_TEST sTESTuri_parts[] =
{
	{ "file:///C:/trees/sys3/int/src/core/tests/TestString.cpp", "file", "", "/C:/trees/sys3/int/src/core/tests/TestString.cpp", "", "" },
	{ "http://msdn.microsoft.com/en-us/library/bb982727.aspx#regexgrammar", "http", "msdn.microsoft.com", "/en-us/library/bb982727.aspx", "", "regexgrammar" },
	{ "file://abyss/trees/sys3/int/src/core/all%20tests/TestString.cpp", "file", "abyss", "/trees/sys3/int/src/core/all tests/TestString.cpp", "", "" },
	{ "FiLe:///C:\\Test/../A file.TxT", "file", "", "/C:/A file.TxT", "", "" },
	{ "file:///c:/a%20file.txt", "file", "", "/c:/a file.txt", "", "" },
	{ "ftp:///c:/a%20file.txt", "ftp", "", "/c:/a file.txt", "", "" },
};

static void TESTuri_parts()
{
	oFORI(i, sTESTuri_parts)
	{
		const auto& t = sTESTuri_parts[i];
		uri u(t.URI);

		auto s = u.scheme();
		auto a = u.authority();
		auto p = u.path();
		auto q = u.query();
		auto f = u.fragment();

		oCHECK0(!strcmp(s, t.Scheme));
		oCHECK0(!strcmp(a, t.Authority));
		oCHECK(!p.compare(t.Path), "fail(%d): path(%s) == %s", i, u.c_str(), t.Path);
		oCHECK0(!strcmp(q, t.Query));
		oCHECK0(!strcmp(f, t.Fragment));

		oCHECK0(u == u);
		if (i)
		{
			uri u2(sTESTuri_parts[i-1].URI);
			oCHECK(u != u2, "fail(%d): %s != %s", i, u.c_str(), u2.c_str());
		}
	}
}

struct URI_ABSOLUTE
{
	const char* String;
	bool IsAbsolute;
};

static const URI_ABSOLUTE sTESTuri_absolute[] =
{
	{ "http:", true },
	{ "file:", true },
	{ "http:path", true },
	{ "file://server/path", true },
	{ "file://server/path#fragment", false },
	{ "file://server/path?query", true },
	{ "file:../path/path2/file.ext", true },
	{ "#file:", false },
	{ "?file:", false },
	{ "./file:", false },
	{ "/file:", false },
	{ "file.txt", false },
	{ "file.txt#item", false },
	{ "file.txt?query=false", false },
	{ "//server/file.txt", false },
	{ "file:#file", false },
	{ "file:file#file", false },
	{ "http://server/file#file", false },
};

static void TESTuri_absolute()
{
	oFORI(i, sTESTuri_absolute)
	{
		const auto& t = sTESTuri_absolute[i];
		uri u(t.String);
		oCHECK(u.absolute() == t.IsAbsolute, "fail(%d): %s is %sabsolute when %sexpected to be."
			, i
			, t.String
			, t.IsAbsolute ? "not " : ""
			, t.IsAbsolute ? "" : "not ");
	}
}

struct URI_SAME_DOCUMENT
{
	const char* String1;
	const char* String2;
	bool IsSame;
};

static const URI_SAME_DOCUMENT sTESTuri_same_document[] = 
{
	{ "", "file:///path/path2/file.txt", true },
	{ "#some_item", "file:///path/path2/file.txt", true },
	{ "file.txt", "file:///path/path2/file.txt", true },
	{ "file.txt?modified", "file:///path/path2/file.txt?modified", true },
	{ "file.txt#some_item", "file:///path/path2/file.txt", true },
	{ "../path2/file.txt#some_item", "file:///path/path2/file.txt", true },
	{ "/path/path2/file.txt#some_item", "file:///path/path2/file.txt", true },
	{ "file:path/path2/file.txt#some_item", "file:///path/path2/file.txt", true },
	{ "file:///path/path2/file.txt#some_item", "file:///path/path2/file.txt", true },

	{ "a", "file:///path/path2/file.txt", false },
	{ "a#some_item", "file:///path/path2/file.txt", false },
	{ "a/file.txt", "file:///path/path2/file.txt", false },
	{ "file.txt?notmodified", "file:///path/path2/file.txt?modified", false },
	{ "../path/file.txt#some_item", "file:///path/path2/file.txt", false },
	{ "/path/path/file.txt#some_item", "file:///path/path2/file.txt", false },
	{ "file:path/path/file.txt#some_item", "file:///path/path2/file.txt", false },
	{ "file:///path/path/file.txt#some_item", "file:///path/path2/file.txt", false },
};

void TESTuri_same_document()
{
	oFORI(i, sTESTuri_same_document)
	{
		const auto& t = sTESTuri_same_document[i];
		uri a(t.String1), b(t.String2);
		oCHECK(a.is_same_document(b), "fail(%d): %s is_same_document %s", i, t.String1, t.String2);
	}
}

struct URI_RESOLVE
{
	const char* Ref;
	const char* Resolved;
};

static const uri sTESTuri_make_absolute_base("http://a/b/c/d;p?q");
static const URI_RESOLVE sTESTuri_make_absolute[] =
{
	// http://tools.ietf.org/html/rfc3986#section-5.4.1
	{ "g:h"	, "g:h" },
	{ "g", "http://a/b/c/g" },
	{ "./g", "http://a/b/c/g" },
	{ "g/", "http://a/b/c/g/" },
	{ "/g", "http://a/g" },
	{ "//g", "http://g" },
	{ "?y", "http://a/b/c/d;p?y" },
	{ "g?y", "http://a/b/c/g?y" },
	{ "#s", "http://a/b/c/d;p?q#s" },
	{ "g#s", "http://a/b/c/g#s" },
	{ "g?y#s", "http://a/b/c/g?y#s" },
	{ ";x", "http://a/b/c/;x" },
	{ "g;x", "http://a/b/c/g;x" },
	{ "g;x?y#s", "http://a/b/c/g;x?y#s" },
	{ "", "http://a/b/c/d;p?q" },
	{ ".", "http://a/b/c/" },
	{ "./", "http://a/b/c/" },
	{ "..", "http://a/b/" },
	{ "../", "http://a/b/" },
	{ "../g", "http://a/b/g" },
	{ "../..", "http://a/" },
	{ "../../", "http://a/" },
	{ "../../g", "http://a/g" },

	// http://tools.ietf.org/html/rfc3986#section-5.4.2
	{ "../../../g", "http://a/g" },
	{ "../../../../g", "http://a/g" },
	{ "/./g", "http://a/g" },
	{ "/../g", "http://a/g" },
	//	{ "g.", "http://a/b/c/g." }, // FIXME
	{ ".g", "http://a/b/c/.g" },
	//	{ "g..", "http://a/b/c/g.." }, // FIXME
	{ "..g", "http://a/b/c/..g" },
	{ "./../g", "http://a/b/g" },
	{ "./g/.", "http://a/b/c/g/" },
	{ "g/./h", "http://a/b/c/g/h" },
	{ "g/../h", "http://a/b/c/h" },
	{ "g;x=1/./y", "http://a/b/c/g;x=1/y" },
	{ "g;x=1/../y", "http://a/b/c/y" },
	{ "g?y/./x", "http://a/b/c/g?y/./x" },
	{ "g?y/../x", "http://a/b/c/g?y/../x" },
	{ "g#s/./x", "http://a/b/c/g#s/./x" },
	{ "g#s/../x", "http://a/b/c/g#s/../x" },
};

static const uri sTESTuri_make_absolute_base2("file://DATA/Test/Scenes/TestTextureSet.xml");
static const URI_RESOLVE sTESTuri_make_absolute2[] = 
{
	{ "/test.xml", "file://DATA/test.xml" },
	{ "#Node", "file://DATA/Test/Scenes/TestTextureSet.xml#Node" },
	{ "../Textures/Hatch1.png", "file://DATA/Test/Textures/Hatch1.png" },
	{ "test.xml", "file://DATA/Test/Scenes/test.xml" },
	{ "file://DATA2/test.xml", "file://DATA2/test.xml" },
	{ "http://DATA/test.xml", "http://DATA/test.xml" },
	{ "http://DATA2/test.xml", "http://DATA2/test.xml" },
};

struct URI_REPLACE
{
	const char* Orig;
	const char* NewScheme;
	const char* NewAuthority;
	const char* NewPath;
	const char* NewQuery;
	const char* NewFragment;
	const char* Expected;
};

static const URI_REPLACE sTESTuri_replace[] = 
{
	{ "file:///c:/users/bob/desktop/test.xml", nullptr, nullptr, nullptr, nullptr, "test", "file:///c:/users/bob/desktop/test.xml#test" },
	{ "file://USER/desktop/test.xml", nullptr, nullptr, nullptr, "test", nullptr, "file://USER/desktop/test.xml?test" },
};

static void TESTuri_make_absolute()
{
	oFORI(i, sTESTuri_make_absolute)
	{
		const auto& t = sTESTuri_make_absolute[i];
		uri u(sTESTuri_make_absolute_base, t.Ref); 
		uri r(t.Resolved);
		oCHECK(u == r, "fail(%d) (absolute): %s + %s != %s", i, sTESTuri_make_absolute_base.c_str(), t.Ref, t.Resolved);
	}

	oFORI(i, sTESTuri_make_absolute2)
	{
		const auto& t = sTESTuri_make_absolute2[i];
		uri u(sTESTuri_make_absolute_base2, t.Ref); 
		uri r(t.Resolved);
		oCHECK(u == r, "fail(%d) (absolute2): %s + %s != %s", i, sTESTuri_make_absolute_base2.c_str(), t.Ref, t.Resolved);
	}
}

static const uri sTESTuri_make_relative_base("file://DATA/Test/Scenes/TestTextureSet.xml");

static const URI_RESOLVE sTESTuri_make_relative[] = 
{
	{ "file://DATA/Test/Scenes/TestTextureSet.xml", "TestTextureSet.xml" }, // Should be ""
	{ "file://DATA/Test/Scenes/TestTextureSet.xml#item", "TestTextureSet.xml#item" }, // Should be "#item"
	{ "file://DATA/Test/Scenes/TestTextureSet.xml?query", "TestTextureSet.xml?query" }, // Should be "?query"

	{ "file://DATA/Test/Scenes/test/test/test.xml", "test/test/test.xml" },
	{ "file://DATA/Test/Scenes/test/test.xml", "test/test.xml" },
	{ "file://DATA/Test/Scenes/test.xml", "test.xml" },
	{ "file://DATA/Test/test.xml", "../test.xml" },
	{ "file://DATA/test.xml", "../../test.xml" },
	{ "file://DATA/", "../../" },
};

static void TESTuri_make_relative()
{
	oFORI(i, sTESTuri_make_relative)
	{
		const auto& t = sTESTuri_make_relative[i];
		uri u(t.Ref);
		u.make_relative(sTESTuri_make_relative_base);
		uri r(t.Resolved);
		oCHECK(u == r, "fail(%d) (relative): %s - %s != %s", i, sTESTuri_make_relative_base.c_str(), t.Ref, t.Resolved);
	}
}

static void TESTuri_replace()
{
	oFORI(i, sTESTuri_replace)
	{
		const auto& t = sTESTuri_replace[i];

		uri u(t.Orig);
		u.replace(t.NewScheme, t.NewAuthority, t.NewPath, t.NewQuery, t.NewFragment);
		uri e(t.Expected);
		oCHECK(u == e, "fail(%d) (replace) got %s, expected %s", i, u.c_str(), t.Expected);
	}
}

void TESTuri()
{
	TESTuri_parts();
	TESTuri_absolute();
	TESTuri_same_document();
	TESTuri_make_absolute();
	TESTuri_make_relative();
	TESTuri_replace();
}

	} // namespace tests
} // namespace oStd