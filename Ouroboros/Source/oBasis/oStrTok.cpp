// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.
#include <oBasis/oStrTok.h>
#include <oBase/assert.h>
#include <oMemory/byte.h>
#include <oString/string.h>
#include <cctype>
#include <cerrno>
#include <iterator>
#include <cmath>

bool oStrTokFinishedSuccessfully(char** _ppContext)
{
	// strtok2_s will zero out context if 
	// it finishes successfully.
	return !*_ppContext;
}

void oStrTokClose(char** _ppContext)
{
	char* start = *_ppContext;
	if (start)
	{
		*_ppContext = nullptr;
		start += strlen(start);
		char* origAlloc = *reinterpret_cast<char**>(start+1);
		free(origAlloc);
	}
}

char* oStrTok(const char* _Token, const char* _Delimiter, char** _ppContext, const char* _OpenScopeChars, const char* _CloseScopeChars)
{
	char* start;

	// on first usage, make a copy of the string for thread safety 
	// and so we can poke at the buffer.
	if (_Token)
	{
		// copy string and also store pointer so it can be freed
		size_t n = strlen(_Token);
		if (!n)
			return nullptr;
		*_ppContext = static_cast<char*>(malloc(n + 1 + sizeof(char*)));
		strlcpy(*_ppContext, _Token, n + 1);
		*reinterpret_cast<char**>((*_ppContext) + n + 1) = *_ppContext;
		start = *_ppContext;
	}

	else
		start = *_ppContext;

	int opens = 0;

	// skip empty tokens
	while (*start)
	{
		if (!strchr(_Delimiter, *start) && opens == 0) break;
		else if (strchr(_OpenScopeChars, *start)) opens++;
		else if (strchr(_CloseScopeChars, *start)) opens--;
		if (opens < 0)
		{
			// Unmatched scope characters
			oStrTokClose(&start);
			return 0;
		}

		start++;
	}

	// if at end or with unmatched scope, get out
	if (!*start || opens != 0)
	{
		oStrTokClose(&start);
		if (opens == 0)
			*_ppContext = 0;
		return 0;
	}

	char* end = start;
	while (*end)
	{
		if (strchr(_Delimiter, *end) && opens == 0)
		{
			*end = 0;
			*_ppContext = end + 1;
			return start;
		}

		else if (strchr(_OpenScopeChars, *end)) opens++;
		else if (strchr(_CloseScopeChars, *end)) opens--;
		if (opens < 0)
		{
			// Unmatched scope characters
			oStrTokClose(&end);
			return 0;
		}

		end++;
	}

	*_ppContext = end;
	return start;
}

const char* oStrTokSkip(const char* _pToken, const char* _pDelimiters, int _Count, bool _SkipDelimiters)
{
	if (_Count < 0)
		return nullptr;
	if (_Count == 0)
		return _pToken;

	const char* pos = _pToken;
	for (int i=0; i<_Count; ++i)
	{
		// Skip initial delimiters
		if (_SkipDelimiters)
			pos += strspn(pos, _pDelimiters);

		// Skip until next delimiter
		pos += strcspn(pos, _pDelimiters);
	}

	// Skip trailing delimiters
	if (_SkipDelimiters)
		pos += strspn(pos, _pDelimiters);

	return pos;
}

void oStrParse(const char* _StrSource, const char* _Delimiter, const std::function<void(const char* _Value)>& _Enumerator)
{
	char* ctx;
	const char* token = oStrTok(_StrSource, _Delimiter, &ctx);

	while (token)
	{
		_Enumerator(token);
		token = oStrTok(0, _Delimiter, &ctx);
	}
}
