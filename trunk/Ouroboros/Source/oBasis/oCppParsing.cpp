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
#include <oBasis/oCppParsing.h>
#include <oBasis/oStrTok.h>
#include <oBase/assert.h>
#include <oBasis/oError.h>
#include <oBase/algorithm.h>
#include <oStd/for.h>
#include <map>
#include <regex>
#include <unordered_map>

using namespace std;
using namespace ouro;

const char* oMoveToNextID(const char* _pCurrent, const char* _Stop)
{
	while (*_pCurrent && !strchr(_Stop, *_pCurrent) && !oIsCppID(*_pCurrent))
		_pCurrent++;
	return _pCurrent;
}

char* oMoveToNextID(char* _pCurrent, const char* _Stop)
{
	while (*_pCurrent && !strchr(_Stop, *_pCurrent) && !oIsCppID(*_pCurrent))
		_pCurrent++;
	return _pCurrent;
}

size_t oGetLineNumber(const char* _Start, const char* _Line)
{
	size_t n = 0;
	const char* cur = _Start;
	while (cur <= _Line)
	{
		cur += strcspn(cur, oNEWLINE);
		cur += strspn(cur, oNEWLINE);
		n++;
	}

	return n;
}

char* oGetStdVectorType(char* _StrDestination, size_t _SizeofStrDestination, const char* _TypeinfoName)
{
	*_StrDestination = 0;

	// hack for the difference betwee MS and GCC (MS has class in the name)
	const char* delims = !_memicmp(_TypeinfoName, "class ", 6) ? ",>" : ", >";
	const char* cur = _TypeinfoName + strcspn(_TypeinfoName, "<");
	if (cur)
	{
		cur++;
		char* ctx;
		char* token = oStrTok(cur, delims, &ctx, "<", ">");
		if (token)
		{
			strlcpy(_StrDestination, token, _SizeofStrDestination);
			oStrTokClose(&ctx);
		}
	}

	return _StrDestination;
}

bool oIsStdBindImplementationDetail(const char* _Symbol)
{
	static const char* stdbindstrs[] = { "std::tr1::_Pmf", "std::tr1::_Callable_", "std::tr1::_Bind", "std::tr1::_Impl", "std::tr1::_Function", };
	static size_t stdbindlens[] = { 14, 20, 15, 15, 19, };
	static_assert(oCOUNTOF(stdbindstrs) == oCOUNTOF(stdbindlens), "");
	oFORI(i, stdbindstrs)
		if (!memcmp(_Symbol, stdbindstrs[i], stdbindlens[i]))
			return true;
	return false;
}

static regex reInclude("#[ \\t]*include[ \\t]+(<|\")([^>\"]+)(?:>|\")", std::tr1::regex_constants::optimize); // @tony: ok static (duplication won't affect correctness)

bool oGetNextInclude(char* _StrDestination, size_t _SizeofStrDestination, const char** _ppContext)
{
	bool result = false;
	cmatch matches;
	if (_ppContext && *_ppContext)
	{
		if (regex_search(*_ppContext, matches, reInclude))
		{
			copy_match_result(_StrDestination, _SizeofStrDestination, matches, 2);
			*_ppContext = 1 + matches[matches.size()-1].second;
			result = true;
		}

		else
			*_ppContext = 0;
	}

	return result;
}

bool oMergeIncludes(char* _StrSourceCode, size_t _SizeofStrSourceCode, const char* _SourceFullPath, const oFUNCTION<bool(void* _pDestination, size_t _SizeofDestination, const char* _Path)>& _Load)
{
	map<string, string> includes;
	char includePath[_MAX_PATH];

	// includes can include other includes, so keep doing passes
	cmatch matches;
	while (regex_search(_StrSourceCode, matches, reInclude))
	{
		copy_match_result(includePath, matches, 2);
		bool isSystemPath = *matches[1].first == '<';
		oTRACE("#include %s%s%s.", isSystemPath?"<":"\"", includePath, isSystemPath?">":"\"");
		size_t matchLength = static_cast<size_t>(matches[0].length()) + 1; // +1 for newline

		string& include = includes[includePath];
		if (include.empty())
		{
			char inc[200*1024];
			if (!_Load(&inc, oCOUNTOF(inc), _SourceFullPath))
				return oErrorSetLast(std::errc::io_error, "Load failed: %s%s%s", isSystemPath?"<":"\"", includePath, isSystemPath?">":"\"");

			include = inc;

			zero_line_comments(&include[0], "//");
			zero_block_comments(&include[0], "/*", "*/");

			oTRACE("-- %s loaded: %u chars", includePath, (unsigned int)include.size());
			if (!insert(_StrSourceCode, _SizeofStrSourceCode, const_cast<char*>(matches[0].first), matchLength, include.c_str()))
				return oErrorSetLast(std::errc::invalid_argument, "Merge failed: %s%s%s (source buffer too small)", isSystemPath?"<":"\"", includePath, isSystemPath?">":"\"");
		}

		else
		{
			// We've already imported this include, so don't import it again.
			memset(const_cast<char*>(matches[0].first), ' ', matchLength);
			oTRACE("-- Already loaded: %s", includePath);
		}
	}

	oErrorSetLast(0);
	return true;
}

