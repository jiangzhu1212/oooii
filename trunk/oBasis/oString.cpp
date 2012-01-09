// $(header)
#include <oBasis/oString.h>
#include <oBasis/oAssert.h>
#include <oBasis/oByteSwizzle.h>
#include <cctype>
#include <cerrno>
#include <iterator>
#include <cmath>

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

char* oTrimLeft(char* _Trimmed, size_t _SizeofTrimmed, const char* _StrSource, const char* _ToTrim)
{
	_StrSource += strspn(_StrSource, _ToTrim);
	strcpy_s(_Trimmed, _SizeofTrimmed, _StrSource);
	return _Trimmed;
}

char* oTrimRight(char* _Trimmed, size_t _SizeofTrimmed, const char* _StrSource, const char* _ToTrim)
{
	const char* end = &_StrSource[strlen(_StrSource)-1];
	while (strchr(_ToTrim, *end) && end > _StrSource)
		end--;
	strncpy_s(_Trimmed, _SizeofTrimmed, _StrSource, end+1-_StrSource);
	return _Trimmed;
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

char* oAddTruncationElipse(char* _StrDestination, size_t _SizeofStrDestination)
{
	oASSERT(_SizeofStrDestination >= 4, "String is too short for an elipse.");
	_StrDestination[_SizeofStrDestination-1] = '\0';
	_StrDestination[_SizeofStrDestination-2] = '.';
	_StrDestination[_SizeofStrDestination-3] = '.';
	_StrDestination[_SizeofStrDestination-4] = '.';
	return _StrDestination;
}

errno_t oReplace(char* _StrResult, size_t _SizeofStrResult, const char* _StrSource, const char* _StrFind, const char* _StrReplace)
{
	if (!_StrResult || !_StrSource) return EINVAL;
	if (!_StrFind)
		return strcpy_s(_StrResult, _SizeofStrResult, _StrSource);
	if (!_StrReplace)
		_StrReplace = "";

	size_t findLen = strlen(_StrFind);
	size_t replaceLen = strlen(_StrReplace);

	errno_t err = 0;

	const char* s = strstr(_StrSource, _StrFind);
	while (s)
	{
		size_t len = s - _StrSource;
		err = strncpy_s(_StrResult, _SizeofStrResult, _StrSource, len);
		if (err) return err;
		_StrResult += len;
		_SizeofStrResult -= len;
		err = strcpy_s(_StrResult, _SizeofStrResult, _StrReplace);
		if (err) return err;
		_StrResult += replaceLen;
		_SizeofStrResult -= replaceLen;
		_StrSource += len + findLen;
		s = strstr(_StrSource, _StrFind);
	}

	// copy the rest
	return strcpy_s(_StrResult, _SizeofStrResult, _StrSource);
}

const char* oStrStrReverse(const char* _Str, const char* _SubStr)
{
	const char* c = _Str + strlen(_Str) - 1;
	const size_t SubStrLen = strlen(_SubStr);
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

errno_t oInsert(char* _StrSource, size_t _SizeofStrResult, char* _InsertionPoint, size_t _ReplacementLength, const char* _Insertion)
{
	size_t insertionLength = strlen(_Insertion);
	size_t afterInsertionLength = strlen(_InsertionPoint) - _ReplacementLength;
	size_t newLen = static_cast<size_t>(_InsertionPoint - _StrSource) + afterInsertionLength;
	if (newLen+1 > _SizeofStrResult) // +1 for null terminator
		return EINVAL;

	// to avoid the overwrite of a direct memcpy, copy the remainder
	// of the string out of the way and then copy it back in.
	char* tmp = (char*)_alloca(afterInsertionLength);
	memcpy(tmp, _InsertionPoint + _ReplacementLength, afterInsertionLength);
	memcpy(_InsertionPoint, _Insertion, insertionLength);
	char* p = _InsertionPoint + insertionLength;
	memcpy(p, tmp, afterInsertionLength);
	p[afterInsertionLength] = 0;
	return 0;
}

errno_t oVStrAppend(char* _StrDestination, size_t _SizeofStrDestination, const char* _Format, va_list _Args)
{
	size_t len = strlen(_StrDestination);
	if( -1 == vsnprintf_s(_StrDestination + len, _TRUNCATE, _SizeofStrDestination - len, _Format, _Args) )
		return ENOMEM;
	return 0;
}

const char* oOrdinal(int _Number)
{
	char buf[16];
	_itoa_s(_Number, buf, sizeof(buf), 10);
	size_t len = strlen(buf);
	char tens = len >= 2 ? buf[len-2] : '0';
	if (tens != '1')
		switch (buf[len-1])
	{
		case '1': return "st";
		case '2': return "nd";
		case '3': return "rd";
		default: break;
	}
	return "th";
}

errno_t oFormatMemorySize(char* _StrDestination, size_t _SizeofStrDestination, unsigned long long _NumBytes, size_t _NumPrecisionDigits)
{
	int result = 0;

	char fmt[32];
	sprintf_s(fmt, "%%.0%uf %%s", _NumPrecisionDigits);

	#ifdef _WIN64
		if (_NumBytes > 0x10000000000)
			result = sprintf_s(_StrDestination, _SizeofStrDestination, fmt, static_cast<float>(_NumBytes) / (1024.0f * 1024.0f * 1024.0f * 1024.0f), "tb");
		else
	#endif
	{
		if (_NumBytes > 1024*1024*1024)
			result = sprintf_s(_StrDestination, _SizeofStrDestination, fmt, static_cast<float>(_NumBytes) / (1024.0f * 1024.0f * 1024.0f), "gb");
		else if (_NumBytes > 1024*1024)
			result = sprintf_s(_StrDestination, _SizeofStrDestination, fmt, static_cast<float>(_NumBytes) / (1024.0f * 1024.0f), "mb");
		else if (_NumBytes > 1024)
			result = sprintf_s(_StrDestination, _SizeofStrDestination, fmt, static_cast<float>(_NumBytes) / 1024.0f, "kb");
		else
			result = sprintf_s(_StrDestination, _SizeofStrDestination, "%u bytes", _NumBytes);
	}

	return -1 == result ? ERANGE : 0;
}

static inline const char* plural(unsigned int n) { return n == 1 ? "" : "s"; }

errno_t oFormatTimeSize(char* _StrDestination, size_t _SizeofStrDestination, double _TimeInSeconds, bool _Abbreviated)
{
	oASSERT(_TimeInSeconds >= 0.0, "Negative time (did you do start - end instead of end - start?)");
	if (_TimeInSeconds < 0.0)
		return EINVAL;

	int result = 0;

	const static double ONE_MINUTE = 60.0;
	const static double ONE_HOUR = 60.0 * ONE_MINUTE;
	const static double ONE_DAY = 24.0 * ONE_HOUR;

	unsigned int day = static_cast<unsigned int>(_TimeInSeconds / ONE_DAY);
	unsigned int hour = static_cast<unsigned int>(fmod(_TimeInSeconds, ONE_DAY) / ONE_HOUR);
	unsigned int minute = static_cast<unsigned int>(fmod(_TimeInSeconds, ONE_HOUR) / ONE_MINUTE);
	unsigned int second = static_cast<unsigned int>(fmod(_TimeInSeconds, ONE_MINUTE));
	unsigned int millisecond = static_cast<unsigned int>((_TimeInSeconds - floor(_TimeInSeconds)) * 1000.0);

	*_StrDestination = 0;
	bool oneWritten = false;
	#define APPEND_TIME(_Var, _StrAbbrev) do { if (_Var) { oStrAppend(_StrDestination, _SizeofStrDestination, "%s%u%s%s%s", (*_StrDestination == 0) ? "" : " ", _Var, _Abbreviated ? "" : " ", _Abbreviated ? _StrAbbrev : #_Var, !_Abbreviated ? plural(_Var) : ""); oneWritten = true; } } while(false)
	APPEND_TIME(day, "d");
	APPEND_TIME(hour, "h");
	APPEND_TIME(minute, "m");
	APPEND_TIME(second, "s");
	APPEND_TIME(millisecond, "ms");

	if (!oneWritten)
		oStrAppend(_StrDestination, _SizeofStrDestination, "0 %s", _Abbreviated ? "s" : "seconds");

	return -1 == result ? ERANGE : 0;
}

errno_t oFormatCommas(char* _StrDestination, size_t _SizeofStrDestination, int _Number)
{
	char tmp[256];
	_itoa_s(_Number, tmp, 10);
	size_t lenMinus1 = strlen(tmp) - 1;
	size_t w = lenMinus1 % 3; // offset of first comma
	size_t r = 0;

	if (w+2 < _SizeofStrDestination)
		return ERANGE;

	if (w)
	{
		memcpy(_StrDestination, tmp, w);
		_StrDestination[w] = ',';
	}

	while (w < (_SizeofStrDestination-3-1-1) && r <= lenMinus1) // -3 for 3 nums, -1 for comma, -1 for terminator
	{
		for (size_t i = 0; i < 3; i++)
			_StrDestination[w++] = tmp[r++];

		if (r < lenMinus1)
			_StrDestination[w++] = ',';
	}

	_StrDestination[w] = 0;
	return (r <= lenMinus1) ? ERANGE : 0;
}

char* oConvertFourcc(char* _StrDestination, size_t _SizeofStrDestination, int _Fourcc)
{
	if (_SizeofStrDestination < 5)
		return 0;

	oByteSwizzle32 s;
	s.AsInt = _Fourcc;

	_StrDestination[0] = s.AsChar[3];
	_StrDestination[1] = s.AsChar[2];
	_StrDestination[2] = s.AsChar[1];
	_StrDestination[3] = s.AsChar[0];
	_StrDestination[4] = 0;

	return _StrDestination;
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
		*_ppContext = static_cast<char*>(malloc(n + 1 + sizeof(char*)));
		strcpy_s(*_ppContext, n + 1, _Token);
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

static inline bool IsOpt(const char* arg) { return *arg == '-' || *arg == '/'; }
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
			if (!strcmp(o->LongName, currarg))
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

	i++; // skip to next arg
	*_ppValue = local_argv[i];
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
