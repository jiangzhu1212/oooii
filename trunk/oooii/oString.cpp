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
#include <oooii/oAssert.h>
#include <oooii/oErrno.h>
#include <oooii/oNonCopyable.h>
#include <oooii/oPath.h>
#include <oooii/oStddef.h>
#include <oooii/oSTL.h>
#include <oooii/oString.h>
#include <oooii/oSwizzle.h>
#include <half.h>
#include <map>
#include <unordered_map>

using namespace std;
using namespace std::tr1;

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

errno_t oFormatTimeSize(char* _StrDestination, size_t _SizeofStrDestination, double _TimeInSeconds)
{
	oASSERT(_TimeInSeconds >= 0.0, "Negative time (did you do start - end instead of end - start?)");
	if (_TimeInSeconds < 0.0)
	{
		oSetLastError(EINVAL, "Negative time (did you do start - end instead of end - start?)");
		return EINVAL;
	}

	int result = 0;

	const static double ONE_MINUTE = 60.0;
	const static double ONE_HOUR = 60.0 * ONE_MINUTE;
	const static double ONE_DAY = 24.0 * ONE_HOUR;

	unsigned int days = static_cast<unsigned int>(_TimeInSeconds / ONE_DAY);
	unsigned int hours = static_cast<unsigned int>(fmod(_TimeInSeconds, ONE_DAY) / ONE_HOUR);
	unsigned int minutes = static_cast<unsigned int>(fmod(_TimeInSeconds, ONE_HOUR) / ONE_MINUTE);
	unsigned int seconds = static_cast<unsigned int>(fmod(_TimeInSeconds, ONE_MINUTE));

	if (days)
		result = sprintf_s(_StrDestination, _SizeofStrDestination, "%u day%s %u hour%s %u minute%s %u second%s", days, plural(days), hours, plural(hours), minutes, plural(minutes), seconds, plural(seconds));
	else if (hours)
		result = sprintf_s(_StrDestination, _SizeofStrDestination, "%u hour%s %u minute%s %u second%s", hours, plural(hours), minutes, plural(minutes), seconds, plural(seconds));
	else if (minutes)
		result = sprintf_s(_StrDestination, _SizeofStrDestination, "%u minute%s %u second%s", minutes, plural(minutes), seconds, plural(seconds));
	else
		result = sprintf_s(_StrDestination, _SizeofStrDestination, "%u second%s", seconds, plural(seconds));

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

const char* oGetTypeName(const char* _TypeinfoName)
{
	static struct { const char* prefix; unsigned int len; } sMapping[] =
	{
		{ "enum ", 5 },
		{ "struct ", 7 },
		{ "class ", 6 },
		{ "union ", 6 },
	};

	for (size_t i = 0; i < oCOUNTOF(sMapping); i++)
		if (!memcmp(_TypeinfoName, sMapping[i].prefix, sMapping[i].len)) return _TypeinfoName + sMapping[i].len;

	return _TypeinfoName;
}

const char* oGetNextMatchingBrace(const char* _pPointingAtOpenBrace, char _CloseBrace)
{
	int open = 1;
	char open_brace = *_pPointingAtOpenBrace;
	const char* cur = _pPointingAtOpenBrace + 1;
	while (*cur && open > 0)
	{
		if (*cur == _CloseBrace)
			open--;
		else if (*cur == open_brace)
			open++;
		cur++;
	}

	if (open > 0)
		return 0;
	return cur - 1;
}

const char* oGetNextMatchingBrace(const char* _pPointingAtOpenBrace, const char* _OpenBrace, const char* _CloseBrace)
{
	int open = 1;
	size_t lOpen = strlen(_OpenBrace);
	size_t lClose = strlen(_CloseBrace);

	const char* cur = _pPointingAtOpenBrace + lOpen;
	while (*cur && open > 0)
	{
		if (!memcmp(cur, _CloseBrace, lClose))
			open--;
		else if (!memcmp(cur, _OpenBrace, lOpen))
			open++;
		cur++;
	}

	if (open > 0)
		return 0;
	return cur - 1;
}

static oIFDEF_BLOCK::TYPE GetType(const cmatch& _Matches)
{
	const char* opener = _Matches[1].first;
	const char* Else = _Matches[4].first;
	const char* Endif = _Matches[6].first;

	if (!memcmp(opener, "ifdef", 5)) return oIFDEF_BLOCK::IFDEF;
	else if (!memcmp(opener, "ifndef", 6)) return oIFDEF_BLOCK::IFNDEF;
	else if (!memcmp(opener, "if", 2)) return oIFDEF_BLOCK::IF;
	else if (!memcmp(Else, "elif", 4)) return oIFDEF_BLOCK::ELIF;
	else if (!memcmp(Else, "else", 4)) return oIFDEF_BLOCK::ELSE;
	else if (!memcmp(Endif, "endif", 4)) return oIFDEF_BLOCK::ENDIF;
	return oIFDEF_BLOCK::UNKNOWN;
}

// Matches #ifdef SYM, #ifndef SYM, #else, #endif
// Match1: ifndef or ifdef
// Match2: n or empty
// Match3: SYM or empty
// Match4: else or elseif or empty
// Match5: if or empty
// Match6: endif
static regex reIfdef("#[ \\t]*(if(n?)def)[ \\t]+([a-zA-Z0-9_]+)|#[ \\t]*(else(if)?)|#[ \\t]*(endif)", std::tr1::regex_constants::optimize); // @oooii-tony: ok static (duplication won't affect correctness)

bool oGetNextMatchingIfdefBlocks(oIFDEF_BLOCK* _pBlocks, size_t _MaxNumBlocks, size_t *_pNumValidBlocks, const char* _StrSourceCodeBegin, const char* _StrSourceCodeEnd)
{
	oIFDEF_BLOCK* pLastBlock = 0; // used to late-populate BlockEnd
	oIFDEF_BLOCK* pCurBlock = 0;
	size_t blockIndex = 0;
	int open = 0;

	const char* cur = _StrSourceCodeBegin;

	cmatch matches;
	while (regex_search(cur, matches, reIfdef))
	{
		if (_StrSourceCodeEnd && matches[0].first >= _StrSourceCodeEnd)
			break;

		if (blockIndex >= _MaxNumBlocks-1)
		{
			oSetLastError(EINVAL, "Block buffer too small");
			return false;
		}

		oIFDEF_BLOCK::TYPE type = GetType(matches);

		// increment openness for each opening statement
		switch (type)
		{
		case oIFDEF_BLOCK::IF:
		case oIFDEF_BLOCK::IFDEF:
		case oIFDEF_BLOCK::IFNDEF:
			open++;
			break;
		default:
			break;
		}

		// add new blocks on first and at-level-1 openness
		if (open == 1)
		{
			pCurBlock = &_pBlocks[blockIndex++];
			memset(pCurBlock, 0, sizeof(oIFDEF_BLOCK));
			pCurBlock->Type = type;

			// Record expression and block info if at same level
			switch (type)
			{
			case oIFDEF_BLOCK::IF:
			case oIFDEF_BLOCK::IFDEF:
			case oIFDEF_BLOCK::IFNDEF:
				pCurBlock->ExpressionStart = matches[3].first;
				pCurBlock->ExpressionEnd = matches[3].second;
				pCurBlock->BlockStart = matches[0].second;
				break;

			case oIFDEF_BLOCK::ELIF:
				// todo: record expression
				// pass thru

			case oIFDEF_BLOCK::ELSE:
			case oIFDEF_BLOCK::ENDIF:
				pCurBlock->BlockStart = matches[0].second;
				pLastBlock->BlockEnd = matches[0].first;
				break;

			default:
				oASSERT(false, "Unsupported #if or #elif statement");
			}
		}

		// If a closing statement, close out everything
		if (type == oIFDEF_BLOCK::ENDIF)
		{
			open--;
			if (!open)
			{
				pCurBlock->BlockStart = matches[0].second;
				pCurBlock->BlockEnd = matches[0].second;
				break;
			}
		}

		// Move to next position
		cur = matches[0].second;

		if (open == 1)
			pLastBlock = pCurBlock;
	}

	if (_pNumValidBlocks)
		*_pNumValidBlocks = blockIndex;

	return true;
}

char* oZeroSection(char* _pPointingAtOpenBrace, const char* _OpenBrace, const char* _CloseBrace, char _Replacement)
{
	char* close = const_cast<char*>(oGetNextMatchingBrace(_pPointingAtOpenBrace, _OpenBrace, _CloseBrace));
	close += strlen(_CloseBrace);

	char* cur = _pPointingAtOpenBrace;

	while (cur < close)
	{
		size_t offset = __min(strcspn(cur, oNEWLINE)-1, static_cast<size_t>(close-cur));
		memset(cur, _Replacement, offset);
		cur += offset;
		cur += strspn(cur, oNEWLINE);
	}

	return close;
}

char* oZeroLineComments(char* _String, const char* _CommentPrefix, char _Replacement)
{
	if (_String)
	{
		size_t l = strlen(_CommentPrefix);
		while (*_String)
		{
			if (!memcmp(_CommentPrefix, _String, l))
				while (*_String && *_String != '\n')
					*_String++ = _Replacement;
			_String++;
		}
	}

	return _String;
}

typedef std::tr1::unordered_map<std::string, std::string> macros_t;

// An internal version that works on an already-hash macros container
static char* oZeroIfdefs(const macros_t& _Macros, char* _StrSourceCodeBegin, char* _StrSourceCodeEnd, char _Replacement = ' ')
{
	size_t numBlocks = 0;
	oIFDEF_BLOCK blocks[32];
	if (!oGetNextMatchingIfdefBlocks(blocks, &numBlocks, _StrSourceCodeBegin, _StrSourceCodeEnd))
		return 0;
	
	if (numBlocks)
	{
		bool zeroBlock = false;
		for (oIFDEF_BLOCK* pCurBlock = blocks; pCurBlock->Type != oIFDEF_BLOCK::ENDIF; pCurBlock++)
		{
			switch (pCurBlock->Type)
			{
				case oIFDEF_BLOCK::IFDEF:
					zeroBlock = _Macros.end() == _Macros.find(std::string(pCurBlock->ExpressionStart, pCurBlock->ExpressionEnd));
					break;
				case oIFDEF_BLOCK::IFNDEF:
					zeroBlock = _Macros.end() != _Macros.find(std::string(pCurBlock->ExpressionStart, pCurBlock->ExpressionEnd));
					break;

				case oIFDEF_BLOCK::ELSE:
					// else will be the last block before endif, so it means that if we 
					// haven't zeroed a block yet, we should now
					zeroBlock = !zeroBlock;
					break;

				default:
					oASSERT(false, "Unhandled case (Did #if/#elif get implemented?)");
					oSetLastError(EINVAL, "Unhandled case (Did #if/#elif get implemented?)");
					return 0;
			}

			if (zeroBlock)
			{
				for (const char* cur = pCurBlock->BlockStart; cur < pCurBlock->BlockEnd; cur++)
					*const_cast<char*>(cur) = _Replacement;
			}

			else
			{
				// If we're not going to zero this block, then recurse into it looking for
				// other blocks to zero

				if (!oZeroIfdefs(_Macros, const_cast<char*>(pCurBlock->BlockStart), const_cast<char*>(pCurBlock->BlockEnd), _Replacement))
					return 0;
			}
		}
	}

	return _StrSourceCodeBegin;
}

static void HashMacros(macros_t& _OutMacros, const oMACRO* _pMacros)
{
	while (_pMacros && _pMacros->Symbol && _pMacros->Value)
	{
		_OutMacros[_pMacros->Symbol] = _pMacros->Value;
		_pMacros++;
	}
}

char* oZeroIfdefs(char* _StrSourceCode, const oMACRO* _pMacros, char _Replacement)
{
	macros_t macros;
	HashMacros(macros, _pMacros);
	return oZeroIfdefs(macros, _StrSourceCode, 0, _Replacement);
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

const char* oGetSimpleTypename(const char* _TypeinfoName)
{
	static struct { const char* s; size_t len; } sPrefixesToSkip[] = 
	{
		{ "struct ", 7 },
		{ "class ", 6 },
		{ "interface ", 10 },
		{ "union ", 6 },
	};

	const char* n = _TypeinfoName;
	for (size_t i = 0; i < oCOUNTOF(sPrefixesToSkip); i++)
		if (!memcmp(n, sPrefixesToSkip[i].s, sPrefixesToSkip[i].len))
			return n + sPrefixesToSkip[i].len;
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
			strcpy_s(_StrDestination, _SizeofStrDestination, token);
			oCloseStrTok(&ctx);
		}
	}

	return _StrDestination;
}

static regex reInclude("#[ \\t]*include[ \\t]+(<|\")([^>\"]+)(?:>|\")", std::tr1::regex_constants::optimize); // @oooii-tony: ok static (duplication won't affect correctness)

bool oGetNextInclude(char* _StrDestination, size_t _SizeofStrDestination, const char** _ppContext)
{
	bool result = false;
	cmatch matches;
	if (_ppContext && *_ppContext)
	{
		if (regex_search(*_ppContext, matches, reInclude))
		{
			oRegexCopy(_StrDestination, _SizeofStrDestination, matches, 2);
			*_ppContext = 1 + matches[matches.size()-1].second;
			result = true;
		}

		else
			*_ppContext = 0;
	}

	return result;
}

errno_t oMergeIncludes(char* _StrSourceCode, size_t _SizeofStrSourceCode, const char* _SourceFullPath, oLOAD_BUFFER_FUNCTION _Load)
{
	map<string, string> includes;
	char includePath[_MAX_PATH];

	// includes can include other includes, so keep doing passes
	cmatch matches;
	while (regex_search(_StrSourceCode, matches, reInclude))
	{
		oRegexCopy(includePath, matches, 2);
		bool isSystemPath = *matches[1].first == '<';
		oTRACE("#include %s%s%s.", isSystemPath?"<":"\"", includePath, isSystemPath?">":"\"");
		size_t matchLength = static_cast<size_t>(matches[0].length()) + 1; // +1 for newline

		string& include = includes[includePath];
		if (include.empty())
		{
			char inc[200*1024];
			if (!_Load(&inc, oCOUNTOF(inc), _SourceFullPath))
			{
				oSetLastError(EIO, "Load failed: %s%s%s", isSystemPath?"<":"\"", includePath, isSystemPath?">":"\"");
				return EIO;
			}

			include = inc;

			oZeroLineComments(&include[0], "//");
			oZeroSection(&include[0], "/*", "*/");

			oTRACE("-- %s loaded: %u chars", includePath, (unsigned int)include.size());
			if (0 != oInsert(_StrSourceCode, _SizeofStrSourceCode, const_cast<char*>(matches[0].first), matchLength, include.c_str()))
			{
				oSetLastError(EINVAL, "Merge failed: %s%s%s (source buffer too small)", isSystemPath?"<":"\"", includePath, isSystemPath?">":"\"");
				return EINVAL;
			}
		}

		else
		{
			// We've already imported this include, so don't import it again.
			memset(const_cast<char*>(matches[0].first), ' ', matchLength);
			oTRACE("-- Already loaded: %s", includePath);
		}
	}

	oSetLastError(0);
	return 0;
}

template<typename T> struct StaticArrayTraits {};
template<> struct StaticArrayTraits<unsigned char>
{
	static const size_t WORDS_PER_LINE = 20;
	static inline const char* GetFormat() { return "0x%02x,"; }
	static inline const char* GetType() { return "unsigned char"; }
};
template<> struct StaticArrayTraits<unsigned short>
{
	static const size_t WORDS_PER_LINE = 16;
	static inline const char* GetFormat() { return "0x%04x,"; }
	static inline const char* GetType() { return "unsigned short"; }
};
template<> struct StaticArrayTraits<unsigned int>
{
	static const size_t WORDS_PER_LINE = 10;
	static inline const char* GetFormat() { return "0x%08x,"; }
	static inline const char* GetType() { return "unsigned int"; }
};
template<> struct StaticArrayTraits<unsigned long long>
{
	static const size_t WORDS_PER_LINE = 10;
	static inline const char* GetFormat() { return "0x%016llx,"; }
	static inline const char* GetType() { return "unsigned long long"; }
};

char* CodifyBufferName(char* _StrDestination, size_t _SizeofStrDestination, const char* _Path)
{
	if (oReplace(_StrDestination, _SizeofStrDestination, oGetFilebase(_Path), ".", "_"))
		return 0;
	return _StrDestination;
}

template<size_t size> inline char* CodifyBufferName(char (&_StrDestination)[size], const char* _Path) { return CodifyBufferName(_StrDestination, size, _Path); }

template<typename T>
static size_t CodifyData(char* _StrDestination, size_t _SizeofStrDestination, const char* _BufferName, const T* _pBuffer, size_t _SizeofBuffer)
{
	const T* words = static_cast<const T*>(_pBuffer);
	char* str = _StrDestination;
	char* end = str + _SizeofStrDestination - 1; // -1 for terminator
	const size_t nWords = _SizeofBuffer / sizeof(T);

	str += sprintf_s(str, _SizeofStrDestination, "const %s sBuffer[] = \n{ // *** AUTO-GENERATED BUFFER, DO NOT EDIT ***", StaticArrayTraits<T>::GetType());
	for (size_t i = 0; i < nWords; i++)
	{
		size_t numberOfElementsLeft = std::distance(str, end);
		if ((i % StaticArrayTraits<T>::WORDS_PER_LINE) == 0 && numberOfElementsLeft > 2)
		{
			*str++ = '\n';
			*str++ = '\t';
			numberOfElementsLeft -= 2;
		}

		str += sprintf_s(str, numberOfElementsLeft, StaticArrayTraits<T>::GetFormat(), *words++);
	}

	// handle any remaining bytes
	const size_t nExtraBytes = _SizeofBuffer % sizeof(T);
	if (nExtraBytes)
	{
		unsigned long long tmp = 0;
		memcpy(&tmp, &reinterpret_cast<const unsigned char*>(_pBuffer)[sizeof(T) * nWords], nExtraBytes);
		str += sprintf_s(str, std::distance(str, end), StaticArrayTraits<T>::GetFormat(), static_cast<T>(tmp));
	}

	str += sprintf_s(str, std::distance(str, end), "\n};\n");

	// add accessor function

	char bufferId[_MAX_PATH];
	CodifyBufferName(bufferId, _BufferName);

	str += sprintf_s(str, std::distance(str, end), "void GetDesc%s(const char** ppBufferName, const void** ppBuffer, size_t* pSize) { *ppBufferName = \"%s\"; *ppBuffer = sBuffer; *pSize = %ull; }\n", bufferId, oGetFilebase(_BufferName), _SizeofBuffer);

	if (str < end)
		*str++ = 0;

	return std::distance(_StrDestination, str);
}

size_t oCodifyData(char* _StrDestination, size_t _SizeofStrDestination, const char* _BufferName, const void* _pBuffer, size_t _SizeofBuffer, size_t _WordSize)
{
	switch (_WordSize)
	{
		case sizeof(unsigned char): return CodifyData(_StrDestination, _SizeofStrDestination, _BufferName, static_cast<const unsigned char*>(_pBuffer), _SizeofBuffer);
		case sizeof(unsigned short): return CodifyData(_StrDestination, _SizeofStrDestination, _BufferName, static_cast<const unsigned short*>(_pBuffer), _SizeofBuffer);
		case sizeof(unsigned int): return CodifyData(_StrDestination, _SizeofStrDestination, _BufferName, static_cast<const unsigned int*>(_pBuffer), _SizeofBuffer);
		case sizeof(unsigned long long): return CodifyData(_StrDestination, _SizeofStrDestination, _BufferName, static_cast<const unsigned long long*>(_pBuffer), _SizeofBuffer);
		default: break;
	}

	return 0;
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

typedef std::tr1::unordered_map<std::string, std::string> headers_t;
bool CollectHeaders(headers_t& _Headers, const char* _StrSourceCode, const char* _SourceCodeDirectory, const macros_t& _Macros, const char* _HeaderSearchPath, oPATH_EXISTS_FUNCTION _PathExists, oLOAD_BUFFER_FUNCTION _LoadHeaderFile)
{
	// @oooii-tony: I run into this pattern a lot... I have a C-interface that 
	// works with char*'s and is careful to assign all allocation of such buffers
	// to the user. I have a need for a string hash that needs to handle its own
	// allocation, so std::string is a decent option, but I am forever creating 
	// temp std::strings when using char*'s to interact with it. Is there a better
	// way?

	// Make an internal copy so we can clean up the defines
	std::vector<char> sourceCodeCopy(strlen(_StrSourceCode) + 1);
	strcpy_s(oGetData(sourceCodeCopy), sourceCodeCopy.capacity(), _StrSourceCode);

	if (!oZeroIfdefs(_Macros, oGetData(sourceCodeCopy), 0))
		return false;

	const char* pContext = oGetData(sourceCodeCopy);
	char headerRelativePath[_MAX_PATH];
	std::string strHeaderRelativePath;
	strHeaderRelativePath.reserve(_MAX_PATH);

	char headerFullPath[_MAX_PATH];
	
	std::vector<char> headerCode(200 * 1024);

	while (oGetNextInclude(headerRelativePath, &pContext))
	{
		// once assignment used twice below rather than 2 ctor calls to temp std::strings
		strHeaderRelativePath = headerRelativePath;

		if (_Headers.end() != _Headers.find(strHeaderRelativePath))
			continue;

		if (!oFindInPath(headerFullPath, _HeaderSearchPath, headerRelativePath, _SourceCodeDirectory, _PathExists))
		{
			oSetLastError(ENOENT, "Could not find %s in search path %s;%s", headerRelativePath, _SourceCodeDirectory, _HeaderSearchPath);
			return false;
		}

		// This risks redundantly checking the same file, but allows a quick check
		// to prevent finding the same include multiple times just above
		_Headers[strHeaderRelativePath] = headerFullPath;

		if (!_LoadHeaderFile(oGetData(headerCode), headerCode.capacity(), headerFullPath))
		{
			oSetLastError(EIO, "Load failed: %s", headerFullPath);
			return false;
		}

		if (!CollectHeaders(_Headers, (const char*)oGetData(headerCode), _SourceCodeDirectory, _Macros, _HeaderSearchPath, _PathExists, _LoadHeaderFile))
			return false;
	}

	return true;
}

bool oHeadersAreUpToDate(const char* _StrSourceCode, const char* _SourceFullPath, const oMACRO* _pMacros, oPATH_EXISTS_FUNCTION _PathExists, oFUNCTION<time_t(const char* _Path)> _GetModifiedDate, oLOAD_BUFFER_FUNCTION _LoadHeaderFile, const char* _HeaderSearchPath)
{
	macros_t macros;
	HashMacros(macros, _pMacros);

	// Make an internal copy so we can clean up the defines
	std::vector<char> sourceCodeCopy(strlen(_StrSourceCode) + 1);
	strcpy_s(oGetData(sourceCodeCopy), oGetDataSize(sourceCodeCopy), _StrSourceCode);

	if (!oZeroIfdefs(macros, oGetData(sourceCodeCopy), 0))
		return false;

	// Get the base point, the specified source file
	const time_t sourceTimestamp = _GetModifiedDate(_SourceFullPath);
	if (!sourceTimestamp)
	{
		oSetLastError(ENOENT, "Could not find %s", _SourceFullPath);
		return false;
	}

	char sourcePath[_MAX_PATH];
	strcpy_s(sourcePath, _SourceFullPath);
	oTrimFilename(sourcePath);

	// Hash headers so we don't recurse down deep include trees more than once
	headers_t headers;
	if (!CollectHeaders(headers, oGetData(sourceCodeCopy), sourcePath, macros, _HeaderSearchPath, _PathExists, _LoadHeaderFile))
		return false;

	// Go through unique headers and check dates
	for (headers_t::const_iterator it = headers.begin(); it != headers.end(); ++it)
	{
		const std::string& headerFullPath = it->second;
		const time_t headerTimestamp = _GetModifiedDate(headerFullPath.c_str());
		if (!headerTimestamp)
		{
			oSetLastError(ENOENT, "Could not find %s", _SourceFullPath);
			return false;
		}

		if (headerTimestamp > sourceTimestamp)
		{
			oSetLastError(ETIME, "%s is out of date because %s is newer", _SourceFullPath, headerFullPath.c_str());
			return false;
		}
	}

	return true;
}

bool oStrTokFinishedSuccessfully(char** _ppContext)
{
	// strtok2_s will zero out context if 
	// it finishes successfully.
	return !*_ppContext;
}

void oCloseStrTok(char** _ppContext)
{
	char* start = *_ppContext;
	*_ppContext = nullptr;
	start += strlen(start);
	char* origAlloc = *reinterpret_cast<char**>(start+1);
	free(origAlloc);
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
			oCloseStrTok(&start);
			return 0;
		}

		start++;
	}

	// if at end or with unmatched scope, get out
	if (!*start || opens != 0)
	{
		oCloseStrTok(&start);
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
			oCloseStrTok(&end);
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
	oTHREADLOCAL static const char** local_argv = 0;
	oTHREADLOCAL static int local_argc = 0;
	oTHREADLOCAL static const oOption* local_options = 0;
	oTHREADLOCAL static int i = 0;

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

				i += 2;
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

// pass-through case
template<> errno_t oToString(char* _StrDestination, size_t _SizeofStrDestination, const char* const & _Value)
{
	return strcpy_s(_StrDestination, _SizeofStrDestination, _Value ? _Value : "(null)");
}

template<> errno_t oToString(char* _StrDestination, size_t _SizeofStrDestination, const char& _Value)
{
	if (_SizeofStrDestination < 2)
		return ERANGE;

	_StrDestination[0] = _Value;
	_StrDestination[1] = 0;
	return 0;
}

template<> errno_t oToString(char* _StrDestination, size_t _SizeofStrDestination, const unsigned char& _Value)
{
	if (_SizeofStrDestination < 2)
		return ERANGE;

	_StrDestination[0] = (signed)_Value;
	_StrDestination[1] = 0;
	return 0;
}

template<> errno_t oToString(char* _StrDestination, size_t _SizeofStrDestination, const bool& _Value) { return strcpy_s(_StrDestination, _SizeofStrDestination, _Value ? "true" : "false"); }
template<> errno_t oToString(char* _StrDestination, size_t _SizeofStrDestination, const short & _Value) { return _itoa_s(_Value, _StrDestination, _SizeofStrDestination, 10); }
template<> errno_t oToString(char* _StrDestination, size_t _SizeofStrDestination, const unsigned short& _Value) { return sprintf_s(_StrDestination, _SizeofStrDestination, "%hu", _Value); }
template<> errno_t oToString(char* _StrDestination, size_t _SizeofStrDestination, const int& _Value) { return _itoa_s(_Value, _StrDestination, _SizeofStrDestination, 10); }
template<> errno_t oToString(char* _StrDestination, size_t _SizeofStrDestination, const unsigned int& _Value) { return -1 == sprintf_s(_StrDestination, _SizeofStrDestination, "%u", _Value) ? EINVAL : 0; }
template<> errno_t oToString(char* _StrDestination, size_t _SizeofStrDestination, const long& _Value) { return _itoa_s(_Value, _StrDestination, _SizeofStrDestination, 10); }
template<> errno_t oToString(char* _StrDestination, size_t _SizeofStrDestination, const unsigned long& _Value) { return sprintf_s(_StrDestination, _SizeofStrDestination, "%u", _Value); }
template<> errno_t oToString(char* _StrDestination, size_t _SizeofStrDestination, const long long& _Value) { return _i64toa_s(_Value, _StrDestination, _SizeofStrDestination, 10); }
template<> errno_t oToString(char* _StrDestination, size_t _SizeofStrDestination, const unsigned long long& _Value) { return _ui64toa_s(*(int*)&_Value, _StrDestination, _SizeofStrDestination, 10); }
template<> errno_t oToString(char* _StrDestination, size_t _SizeofStrDestination, const float& _Value) { errno_t err = (-1 == sprintf_s(_StrDestination, _SizeofStrDestination, "%f", _Value)) ? EINVAL : 0; if (!err) oTrimRight(_StrDestination, _SizeofStrDestination, _StrDestination, "0"); return err; }
template<> errno_t oToString(char* _StrDestination, size_t _SizeofStrDestination, const double& _Value) { errno_t err = (-1 == sprintf_s(_StrDestination, _SizeofStrDestination, "%lf", _Value)) ? EINVAL : 0; if (!err) oTrimRight(_StrDestination, _SizeofStrDestination, _StrDestination, "0"); return err; }
template<> errno_t oToString(char* _StrDestination, size_t _SizeofStrDestination, const half& _Value) { errno_t err = oToString<float>(_StrDestination, _SizeofStrDestination, (float)_Value) ? 0 : EINVAL; if (!err) oTrimRight(_StrDestination, _SizeofStrDestination, _StrDestination, "0"); return err; }
template<> errno_t oToString(char* _StrDestination, size_t _SizeofStrDestination, const std::string& _Value) { return strcpy_s(_StrDestination, _SizeofStrDestination, _Value.c_str()); }

// pass-through case
template<> errno_t oFromString(const char** _pValue, const char* _StrSource)
{
	*_pValue = _StrSource;
	return 0;
}

// path case
template<> errno_t oFromString( char(* _pValue )[_MAX_PATH], const char* _StrSource)
{
	strcpy_s( *_pValue, _MAX_PATH, _StrSource );
	return 0;
}

template<> errno_t oFromString(bool* _pValue, const char* _StrSource)
{
	if (!_StrSource) return EINVAL;

	if (!_stricmp("true", _StrSource) || !_stricmp("t", _StrSource) || !_stricmp("yes", _StrSource) || !_stricmp("y", _StrSource))
		*_pValue = true;
	else 
		*_pValue = atoi(_StrSource) != 0;

	return 0;
}

template<> errno_t oFromString(char* _pValue, const char* _StrSource) { if (!_StrSource) return EINVAL; *_pValue = *_StrSource; return 0; }
template<> errno_t oFromString(unsigned char* _pValue, const char* _StrSource) { if (!_StrSource) return EINVAL; *_pValue = *(const unsigned char*)_StrSource; return 0; }

template<typename T> inline errno_t _FromString(T* _pValue, const char* _Format, const char* _StrSource)
{
	if (!_StrSource) return EINVAL;
	return (1 == sscanf_s(_StrSource, _Format, _pValue)) ? 0 : EINVAL;
}

template<> errno_t oFromString(short* _pValue, const char* _StrSource) { return _FromString(_pValue, "%hd", _StrSource); }
template<> errno_t oFromString(unsigned short* _pValue, const char* _StrSource) { return _FromString(_pValue, "%hu", _StrSource); }
template<> errno_t oFromString(int* _pValue, const char* _StrSource) { return _FromString(_pValue, "%d", _StrSource); }
template<> errno_t oFromString(unsigned int* _pValue, const char* _StrSource) { return _FromString(_pValue, "%u", _StrSource); }
template<> errno_t oFromString(long* _pValue, const char* _StrSource) { return _FromString(_pValue, "%d", _StrSource); }
template<> errno_t oFromString(unsigned long* _pValue, const char* _StrSource) { return _FromString(_pValue, "%u", _StrSource); }
template<> errno_t oFromString(long long* _pValue, const char* _StrSource) { return _FromString(_pValue, "%lld", _StrSource); }
template<> errno_t oFromString(unsigned long long* _pValue, const char* _StrSource) { return _FromString(_pValue, "%llu", _StrSource); }

template<> errno_t oFromString(float* _pValue, const char* _StrSource) { return _FromString(_pValue, "%f", _StrSource); }
template<> errno_t oFromString(double* _pValue, const char* _StrSource) { return _FromString(_pValue, "%lf", _StrSource); }
template<> errno_t oFromString(half* _pValue, const char* _StrSource) { float v; errno_t err = oFromString(&v, _StrSource); *_pValue = v; return err; }

