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
#include <oBasis/oString.h>
#include <oStd/assert.h>
#include <oStd/byte.h>
#include <oBasis/oInt.h>
#include <cctype>
#include <cerrno>
#include <iterator>
#include <cmath>

template<typename T> T SS(T _String) { return _String ? _String : T(""); }

const char* oStrStr( const char* _pStr, const char* _pSubStr, size_t _MaxCharCount /*= oInvalid*/ )
{
	if(oInvalid == _MaxCharCount)
		return strstr(_pStr, _pSubStr);

	size_t SearchLen = strlen(_pSubStr);
	size_t SearchHead = 0;
	for(size_t i = 0; i < _MaxCharCount; ++i)
	{
		if(_pStr[i] == _pSubStr[SearchHead])
		{
			if( ++SearchHead == SearchLen)
			{
				// Found a match
				return _pStr + ( i - SearchLen );
			}
		}
		else
			SearchHead = 0;
	}
	return nullptr;
}


int oVPrintf(char* _StrDestination, size_t _NumDestinationChars, const char* _Format, va_list _Args)
{
	int status = oStd::vsnprintf(_StrDestination, _NumDestinationChars, _Format, _Args);
	if (status == -1)
		oStd::ellipsize(_StrDestination, _NumDestinationChars);
	return status;
}

int oVPrintf(wchar_t* _StrDestination, size_t _NumDestinationChars, const wchar_t* _Format, va_list _Args)
{
	int status = oStd::vsnwprintf(_StrDestination, _NumDestinationChars, _Format, _Args);
	if (status == -1)
		oStd::wcsellipsize(_StrDestination, _NumDestinationChars);
	return status;
}

errno_t oStrVAppendf(char* _StrDestination, size_t _SizeofStrDestination, const char* _Format, va_list _Args)
{
	if (-1 == oStd::vsncatf(_StrDestination, _SizeofStrDestination, _Format, _Args))
		return ENOMEM;
	return 0;
}

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

// returns if there is a carry/wrap-around
static bool Increment(int* _pValue, size_t _NumValues)
{
	bool carry = false;
	int _NewValue = (*_pValue + 1) % _NumValues;
	if (_NewValue < *_pValue)
		carry = true;
	*_pValue = _NewValue;
	return carry;
}

// returns if there's a carry/wrap-around at the highest level
static bool Increment(int* _pIndices, size_t* _pSizes, int _Count)
{
	int i = _Count - 1;
	for (; i >= 0; i--)
		if (!Increment(&_pIndices[i], _pSizes[i]))
			break;
	return i < 0;
}

void oPermutate(const char*** _ppOptions, size_t* _pSizes, int _NumOptions, oFUNCTION<void(const char* _Permutation)> _Visitor, const char* _Delim)
{
	int* pIndices = (int*)_alloca(_NumOptions * sizeof(int));
	memset(pIndices, 0, _NumOptions * sizeof(int));
	char s[4096];
	do 
	{
		*s = 0;
		for (int i = 0; i < _NumOptions; i++)
		{
			if (i)
				strlcat(s, _Delim);
			strlcat(s, _ppOptions[i][pIndices[i]]);
		}

		_Visitor(s);
		
	} while (!Increment(pIndices, _pSizes, _NumOptions));
}

void oStrParse(const char* _StrSource, const char* _Delimiter, oFUNCTION<void(const char* _Value)> _Enumerator)
{
	char* ctx;
	const char* token = oStrTok(_StrSource, _Delimiter, &ctx);

	while (token)
	{
		_Enumerator(token);
		token = oStrTok(0, _Delimiter, &ctx);
	}
}

int oStrCCount(const char* _StrSource, char _CountChar)
{
	int result = 0;
	while(*_StrSource)
	{
		if(*_StrSource == _CountChar)
			++result;
		++_StrSource;
	}
	return result;
}

int oStrFindFirstDiff(const char* _StrSource1, const char* _StrSource2)
{
	const char* origSrc1 = _StrSource1;
	while(*_StrSource1 && *_StrSource2 && *_StrSource1 == *_StrSource2)
	{
		++_StrSource1;
		++_StrSource2;
	}
	if(!*_StrSource1 && !*_StrSource2)
		return -1;
	return oInt(oStd::byte_diff(_StrSource1, origSrc1));
}