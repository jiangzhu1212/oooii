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
#include <oBasis/oString.h>
#include <oStd/assert.h>
#include <oStd/byte.h>
#include <oBasis/oInt.h>
#include <cctype>
#include <cerrno>
#include <iterator>
#include <cmath>

template<typename T> T SS(T _String) { return _String ? _String : T(""); }

template<typename CHAR_T> static void ZeroBalance(CHAR_T* _StrDestination, size_t _NumDestinationChars)
{
	size_t len = oStrlen(_StrDestination);
	memset(_StrDestination + len, 0, (_NumDestinationChars - len) * sizeof(CHAR_T));
}

size_t oStrlen(const char* _StrSource)
{
	return strlen(SS(_StrSource));
}

size_t oStrlen(const wchar_t* _StrSource)
{
	return wcslen(SS(_StrSource));
}

char* oStrcpy(char* _StrDestination, size_t _NumDestinationChars, const char* _StrSource, bool _ZeroBuffer)
{
	if (oStrlen(_StrSource) >= _NumDestinationChars) return nullptr;
	if (0 != strcpy_s(_StrDestination, _NumDestinationChars, SS(_StrSource))) return nullptr;
	if (_ZeroBuffer) ZeroBalance(_StrDestination, _NumDestinationChars);
	return _StrDestination;
}

wchar_t* oStrcpy(wchar_t* _StrDestination, size_t _NumDestinationChars, const wchar_t* _StrSource, bool _ZeroBuffer)
{
	if (oStrlen(_StrSource) >= _NumDestinationChars) return nullptr;
	if (0 != wcscpy_s(_StrDestination, _NumDestinationChars, SS(_StrSource))) return nullptr;
	if (_ZeroBuffer) ZeroBalance(_StrDestination, _NumDestinationChars);
	return _StrDestination;
}

char* oStrcpy(char* _StrDestination, size_t _NumDestinationChars, const wchar_t* _StrSource, bool _ZeroBuffer)
{
	if (oStrlen(_StrSource) >= _NumDestinationChars) return nullptr;
	size_t sz = 0;
	const wchar_t* src = SS(_StrSource);
	if (0 != wcsrtombs_s(&sz, _StrDestination, _NumDestinationChars, &src, _NumDestinationChars, nullptr)) return nullptr;
	if (_ZeroBuffer) ZeroBalance(_StrDestination, _NumDestinationChars);
	return _StrDestination;
}

wchar_t* oStrcpy(wchar_t* _StrDestination, size_t _NumDestinationChars, const char* _StrSource, bool _ZeroBuffer)
{
	if (oStrlen(_StrSource) >= _NumDestinationChars) return nullptr;
	size_t sz = 0;
	const char* src = SS(_StrSource);
	if (0 != mbsrtowcs_s(&sz, _StrDestination, _NumDestinationChars, &src, _NumDestinationChars, nullptr)) return nullptr;
	if (_ZeroBuffer) ZeroBalance(_StrDestination, _NumDestinationChars);
	return _StrDestination;
}

char* oStrncpy(char* _StrDestination, size_t _NumDestinationChars, const char* _StrSource, size_t _NumChars)
{
	return strncpy_s(_StrDestination, _NumDestinationChars, _StrSource, _NumChars) ? nullptr : _StrDestination;
}

wchar_t* oStrncpy(wchar_t* _StrDestination, size_t _NumDestinationChars, const wchar_t* _StrSource, size_t _NumChars)
{
	return wcsncpy_s(_StrDestination, _NumDestinationChars, _StrSource, _NumChars) ? nullptr : _StrDestination;
}

int oStrcmp(const char* _Str1, const char* _Str2)
{
	return strcmp(_Str1, _Str2);
}

int oStrcmp(const wchar_t* _Str1, const wchar_t* _Str2)
{
	return wcscmp(_Str1, _Str2);
}

int oStricmp(const char* _Str1, const char* _Str2)
{
	return _stricmp(_Str1, _Str2);
}

int oStricmp(const wchar_t* _Str1, const wchar_t* _Str2)
{
	return _wcsicmp(_Str1, _Str2);
}

int oStrncmp(const char* _Str1, const char* _Str2, size_t _NumChars)
{
	return strncmp(_Str1, _Str2, _NumChars);
}

int oStrncmp(const wchar_t* _Str1, const wchar_t* _Str2, size_t _NumChars)
{
	return wcsncmp(_Str1, _Str2, _NumChars);
}

int oStrnicmp(const char* _Str1, const char* _Str2, size_t _NumChars)
{
	return _strnicmp(_Str1, _Str2, _NumChars);
}

int oStrnicmp(const wchar_t* _Str1, const wchar_t* _Str2, size_t _NumChars)
{
	return _wcsnicmp(_Str1, _Str2, _NumChars);
}

char* oStrcat(char* _StrDestination, size_t _NumDestinationChars, const char* _Source)
{
	return strcat_s(_StrDestination, _NumDestinationChars, _Source) ? nullptr : _StrDestination;
}

wchar_t* oStrcat(wchar_t* _StrDestination, size_t _NumDestinationChars, const wchar_t* _Source)
{
	return wcscat_s(_StrDestination, _NumDestinationChars, _Source) ? nullptr : _StrDestination;
}

char* oStrncat(char* _StrDestination, size_t _NumDestinationChars, const char* _Source)
{
	return strncat_s(_StrDestination, _NumDestinationChars, _Source, _TRUNCATE) ? nullptr : _StrDestination;
}

wchar_t* oStrncat(wchar_t* _StrDestination, size_t _NumDestinationChars, const wchar_t* _Source)
{
	return wcsncat_s(_StrDestination, _NumDestinationChars, _Source, _TRUNCATE) ? nullptr : _StrDestination;
}

const char* oStrStr( const char* _pStr, const char* _pSubStr, size_t _MaxCharCount /*= oInvalid*/ )
{
	if(oInvalid == _MaxCharCount)
		return strstr(_pStr, _pSubStr);

	size_t SearchLen = oStrlen(_pSubStr);
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
	int status = vsnprintf_s(_StrDestination, _NumDestinationChars, _TRUNCATE, _Format, _Args);
	if (status == -1)
		oAddTruncationElipse(_StrDestination, _NumDestinationChars);
	return status;
}

int oVPrintf(wchar_t* _StrDestination, size_t _NumDestinationChars, const wchar_t* _Format, va_list _Args)
{
	int status = _vsnwprintf_s(_StrDestination, _NumDestinationChars, _TRUNCATE, _Format, _Args);
	if (status == -1)
		oAddTruncationElipse(_StrDestination, _NumDestinationChars);
	return status;
}

void oToLower(char* _String)
{
	while (*_String)
		*_String++ = static_cast<char>(tolower(*_String));
}

void oToUpper(char* _String)
{
	while (*_String)
		*_String++ = static_cast<char>(toupper(*_String));
}

char* oNewlinesToDos(char* _StrDestination, size_t _SizeofStrDestination, const char* _StrSource)
{
	char* d = _StrDestination;
	char* end = d + _SizeofStrDestination;
	while (*_StrSource)
	{
		if (*_StrSource == '\n' && d != end)
			*d++ = '\r';
		if (d != end)
			*d++ = *_StrSource++;
		else
		{
			oASSERT(false, "oNewlinesToDos: destination string too small.");
		}
	}

	*d = 0;
	return _StrDestination;
}

char* oPruneWhitespace(char* _StrDestination, size_t _SizeofStrDestination, const char* _StrSource, char _Replacement, const char* _ToPrune)
{
	char* w = _StrDestination;
	const char* r = _StrSource;
	_SizeofStrDestination--;
	while (static_cast<size_t>(std::distance(_StrDestination, w)) < _SizeofStrDestination && *r)
	{
		if (strchr(_ToPrune, *r))
		{
			*w++ = _Replacement;
			r += strspn(r, _ToPrune);
		}

		else
			*w++ = *r++;
	}

	*w = 0;

	return _StrDestination;
}

char* oAddTruncationElipse(char* _StrDestination, size_t _NumDestinationChars)
{
	oASSERT(_NumDestinationChars >= 4, "String buffer is too short for an elipse.");
	_StrDestination[_NumDestinationChars-1] = '\0';
	_StrDestination[_NumDestinationChars-2] = '.';
	_StrDestination[_NumDestinationChars-3] = '.';
	_StrDestination[_NumDestinationChars-4] = '.';
	return _StrDestination;
}

wchar_t* oAddTruncationElipse(wchar_t* _StrDestination, size_t _NumDestinationChars)
{
	oASSERT(_NumDestinationChars >= 4, "String buffer is too short for an elipse.");
	_StrDestination[_NumDestinationChars-1] = '\0';
	_StrDestination[_NumDestinationChars-2] = '.';
	_StrDestination[_NumDestinationChars-3] = '.';
	_StrDestination[_NumDestinationChars-4] = '.';
	return _StrDestination;
}

errno_t oReplace(char* oRESTRICT _StrResult, size_t _SizeofStrResult, const char* oRESTRICT _StrSource, const char* _StrFind, const char* _StrReplace)
{
	if (!_StrResult || !_StrSource) return EINVAL;
	oASSERT(_StrResult != _StrSource, "_StrResult and _StrSource cannot be the same buffer");
	if (_StrResult == _StrSource) return EINVAL;
	if (!_StrFind)
		return oStrcpy(_StrResult, _SizeofStrResult, _StrSource) ? 0 : EINVAL;
	if (!_StrReplace)
		_StrReplace = "";

	size_t findLen = oStrlen(_StrFind);
	size_t replaceLen = oStrlen(_StrReplace);

	errno_t err = 0;

	const char* s = strstr(_StrSource, _StrFind);
	while (s)
	{
		size_t len = s - _StrSource;
		if (!oStrncpy(_StrResult, _SizeofStrResult, _StrSource, len))
			return EINVAL;
		_StrResult += len;
		_SizeofStrResult -= len;
		if (!oStrcpy(_StrResult, _SizeofStrResult, _StrReplace))
			return EINVAL;
		_StrResult += replaceLen;
		_SizeofStrResult -= replaceLen;
		_StrSource += len + findLen;
		s = strstr(_StrSource, _StrFind);
	}

	// copy the rest
	return oStrcpy(_StrResult, _SizeofStrResult, _StrSource) ? 0 : EINVAL;
}

const char* oStrStrReverse(const char* _Str, const char* _SubStr)
{
	const char* c = _Str + oStrlen(_Str) - 1;
	const size_t SubStrLen = oStrlen(_SubStr);
	while (c > _Str)
	{
		if (!memcmp(c, _SubStr, SubStrLen))
			return c;

		c--;
	}
	return 0;
}

char* oStrStrReverse(char* _Str, const char* _SubStr)
{
	return const_cast<char*>(oStrStrReverse(static_cast<const char*>(_Str), _SubStr));
}

char* oInsert(char* _StrSource, size_t _SizeofStrResult, char* _InsertionPoint, size_t _ReplacementLength, const char* _Insertion)
{
	size_t insertionLength = oStrlen(_Insertion);
	size_t afterInsertionLength = oStrlen(_InsertionPoint) - _ReplacementLength;
	size_t newLen = static_cast<size_t>(_InsertionPoint - _StrSource) + afterInsertionLength;
	if (newLen+1 > _SizeofStrResult) // +1 for null terminator
		return nullptr;

	// to avoid the overwrite of a direct memcpy, copy the remainder
	// of the string out of the way and then copy it back in.
	char* tmp = (char*)_alloca(afterInsertionLength);
	memcpy(tmp, _InsertionPoint + _ReplacementLength, afterInsertionLength);
	memcpy(_InsertionPoint, _Insertion, insertionLength);
	char* p = _InsertionPoint + insertionLength;
	memcpy(p, tmp, afterInsertionLength);
	p[afterInsertionLength] = 0;
	return p;
}

errno_t oStrVAppendf(char* _StrDestination, size_t _SizeofStrDestination, const char* _Format, va_list _Args)
{
	if (-1 == oStd::vsncatf(_StrDestination, _SizeofStrDestination, _Format, _Args))
		return ENOMEM;
	return 0;
}

char* oFormatMemorySize(char* _StrDestination, size_t _SizeofStrDestination, unsigned long long _NumBytes, size_t _NumPrecisionDigits)
{
	return -1 == oStd::format_bytes(_StrDestination, _SizeofStrDestination, _NumBytes, _NumPrecisionDigits) ? nullptr : _StrDestination;
}

char* oFormatCommas(char* _StrDestination, size_t _SizeofStrDestination, unsigned int _Number)
{
	_itoa_s(_Number, _StrDestination, _SizeofStrDestination, 10);
	size_t len = oStrlen(_StrDestination);

	size_t w = len % 3;
	if (!w)
		w = 3;

	while (w < len)
	{
		if (!oInsert(_StrDestination, _SizeofStrDestination, _StrDestination + w, 0, ","))
			return nullptr;
		w += 4;
	}

	return _StrDestination;
}

char* oFormatCommas(char* _StrDestination, size_t _SizeofStrDestination, int _Number)
{
	if (_Number < 0)
	{
		if (_SizeofStrDestination < 1)
			return nullptr;

		_Number = -_Number;
		*_StrDestination++ = '-';
		_SizeofStrDestination--;
	}

	return oFormatCommas(_StrDestination, _SizeofStrDestination, static_cast<unsigned int>(_Number));
}

bool oGetKeyValuePair(char* _KeyDestination, size_t _SizeofKeyDestination, char* _ValueDestination, size_t _SizeofValueDestination, char _KeyValueSeparator, const char* _KeyValuePairSeparators, const char* _SourceString, const char** _ppLeftOff)
{
	const char* k = _SourceString + strspn(_SourceString, oWHITESPACE); // move past whitespace
	const char strSep[2] = { _KeyValueSeparator, 0 };
	const char* sep = k + strcspn(k, strSep); // mark sep
	if (!sep) return false;
	const char* end = sep + 1 + strcspn(sep+1, _KeyValuePairSeparators); // make end of value

	if (_KeyDestination)
	{
		size_t keyLen = sep - k;
		memcpy_s(_KeyDestination, _SizeofKeyDestination-1, k, keyLen);
		_KeyDestination[__min(_SizeofKeyDestination-1, keyLen)] = 0;
	}
	
	if (_ValueDestination)
	{
		const char* v = sep + 1 + strspn(sep+1, oWHITESPACE);
		size_t valLen = end - v;
		memcpy_s(_ValueDestination, _SizeofValueDestination-1, v, valLen);
		_ValueDestination[__min(_SizeofValueDestination-1, valLen)] = 0;
	}

	if (_ppLeftOff)
		*_ppLeftOff = end;

	return true;
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
		start += oStrlen(start);
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
		size_t n = oStrlen(_Token);
		if (!n)
			return nullptr;
		*_ppContext = static_cast<char*>(malloc(n + 1 + sizeof(char*)));
		oStrcpy(*_ppContext, n + 1, _Token);
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

static inline bool IsOpt(const char* arg) { return *arg == '-'; }
static inline bool IsLongOpt(const char* arg) { return *arg == '-' && *(arg+1) == '-'; }

char oOptTok(const char** _ppValue, int _Argc, const char* _Argv[], const oOption* _pOptions)
{
	thread_local static const char** local_argv = 0;
	thread_local static int local_argc = 0;
	thread_local static const oOption* local_options = 0;
	thread_local static int i = 0;

	*_ppValue = "";

	if (_Argv)
	{
		local_argv = _Argv;
		local_argc = _Argc;
		local_options = _pOptions;
		i = 1; // skip exe name
	}

	else if (i >= local_argc)
		return 0;

	const char* currarg = local_argv[i];
	const char* nextarg = local_argv[i+1];

	if (!currarg)
		return 0;

	if (IsLongOpt(currarg))
	{
		currarg += 2;
		const oOption* o = local_options;
		while (o->LongName)
		{
			if (!oStrcmp(o->LongName, currarg))
			{
				if (o->ArgumentName)
				{
					if (i == _Argc-1 || IsOpt(nextarg))
					{
						i++;
						return ':';
					}

					*_ppValue = nextarg;
				}

				else
					*_ppValue = currarg;

				i++;
				if (o->ArgumentName)
					i++;

				return o->ShortName;
			}

			o++;
		}

		i++; // skip unrecognized opt
		return '?';
	}

	else if (IsOpt(local_argv[i]))
	{
		currarg++;
		const oOption* o = local_options;
		while (o->LongName)
		{
			if (*currarg == o->ShortName)
			{
				if (o->ArgumentName)
				{
					if (*(currarg+1) == ' ' || *(currarg+1) == 0)
					{
						if (i == _Argc-1 || IsOpt(nextarg))
						{
							i++;
							return ':';
						}

						*_ppValue = nextarg;
						i += 2;
					}

					else
					{
						*_ppValue = currarg + 1;
						i++;
					}
				}

				else
				{
					*_ppValue = currarg;
					i++;
				}

				return o->ShortName;
			}

			o++;
		}

		i++; // skip unrecognized opt
		return '?';
	}

	*_ppValue = local_argv[i++]; // skip to next arg
	return ' ';
}

char* oOptDoc(char* _StrDestination, size_t _SizeofStrDestination, const char* _AppName, const oOption* _pOptions)
{
	char* dst = _StrDestination;

	int w = _snprintf_s(dst, _SizeofStrDestination, _TRUNCATE, "%s ", _AppName);
	if (w == -1) return _StrDestination;
	dst += w;
	_SizeofStrDestination -= w;

	const oOption* o = _pOptions;
	while (o->LongName)
	{
		w = _snprintf_s(dst, _SizeofStrDestination, _TRUNCATE, "-%c%s%s ", o->ShortName, o->ArgumentName ? " " : "", o->ArgumentName ? o->ArgumentName : "");
		if (w == -1) return _StrDestination;
		dst += w;
		_SizeofStrDestination -= w;
		o++;
	}

	w = _snprintf_s(dst, _SizeofStrDestination, _TRUNCATE, "\n\n");
	if (w == -1) return _StrDestination;
	dst += w;
	_SizeofStrDestination -= w;

	o = _pOptions;
	while (o->LongName && _SizeofStrDestination > 0)
	{
		w = _snprintf_s(dst, _SizeofStrDestination, _TRUNCATE, "\t-%c\t%- 15s\t%s\n", o->ShortName, o->LongName ? o->LongName : "", o->Description);
		if (w == -1) return _StrDestination;
		dst += w;
		_SizeofStrDestination -= w;
		o++;
	}

	return _StrDestination;
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
				oStrcat(s, _Delim);
			oStrcat(s, _ppOptions[i][pIndices[i]]);
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