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
#include <oStd/algorithm.h>
#include <oStd/assert.h>
#include <oBasis/oString.h>
#include <oBasis/oPath.h>
#include <oBasis/oError.h>
#include <oStd/for.h>
#include <map>
#include <regex>
#include <unordered_map>

using namespace std;
using namespace std::tr1;

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
			oErrorSetLast(std::errc::invalid_argument, "Block buffer too small");
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
	char* close = oStd::next_matching(_pPointingAtOpenBrace, _OpenBrace, _CloseBrace);
	close += oStrlen(_CloseBrace);

	char* cur = _pPointingAtOpenBrace;

	while (cur < close)
	{
		size_t offset = __min(strcspn(cur, oNEWLINE), static_cast<size_t>(close-cur));
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
		size_t l = oStrlen(_CommentPrefix);
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
					oErrorSetLast(std::errc::invalid_argument, "Unhandled case (Did #if/#elif get implemented?)");
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
			oStrcpy(_StrDestination, _SizeofStrDestination, token);
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

static regex reInclude("#[ \\t]*include[ \\t]+(<|\")([^>\"]+)(?:>|\")", std::tr1::regex_constants::optimize); // @oooii-tony: ok static (duplication won't affect correctness)

bool oGetNextInclude(char* _StrDestination, size_t _SizeofStrDestination, const char** _ppContext)
{
	bool result = false;
	cmatch matches;
	if (_ppContext && *_ppContext)
	{
		if (regex_search(*_ppContext, matches, reInclude))
		{
			oStd::copy_match_result(_StrDestination, _SizeofStrDestination, matches, 2);
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
		oStd::copy_match_result(includePath, matches, 2);
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

			oZeroLineComments(&include[0], "//");
			oZeroSection(&include[0], "/*", "*/");

			oTRACE("-- %s loaded: %u chars", includePath, (unsigned int)include.size());
			if (!oStd::insert(_StrSourceCode, _SizeofStrSourceCode, const_cast<char*>(matches[0].first), matchLength, include.c_str()))
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
	if (oStd::replace(_StrDestination, _SizeofStrDestination, oGetFilebase(_Path), ".", "_"))
		return nullptr;
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

	str += oPrintf(str, _SizeofStrDestination, "const %s sBuffer[] = \n{ // *** AUTO-GENERATED BUFFER, DO NOT EDIT ***", StaticArrayTraits<T>::GetType());
	for (size_t i = 0; i < nWords; i++)
	{
		size_t numberOfElementsLeft = std::distance(str, end);
		if ((i % StaticArrayTraits<T>::WORDS_PER_LINE) == 0 && numberOfElementsLeft > 2)
		{
			*str++ = '\n';
			*str++ = '\t';
			numberOfElementsLeft -= 2;
		}

		str += oPrintf(str, numberOfElementsLeft, StaticArrayTraits<T>::GetFormat(), *words++);
	}

	// handle any remaining bytes
	const size_t nExtraBytes = _SizeofBuffer % sizeof(T);
	if (nExtraBytes)
	{
		unsigned long long tmp = 0;
		memcpy(&tmp, &reinterpret_cast<const unsigned char*>(_pBuffer)[sizeof(T) * nWords], nExtraBytes);
		str += oPrintf(str, std::distance(str, end), StaticArrayTraits<T>::GetFormat(), static_cast<T>(tmp));
	}

	str += oPrintf(str, std::distance(str, end), "\n};\n");

	// add accessor function

	char bufferId[_MAX_PATH];
	CodifyBufferName(bufferId, _BufferName);

	unsigned long long sz = _SizeofBuffer; // explicitly size this out so printf formatting below can remain the same between 32- and 64-bit
	str += oPrintf(str, std::distance(str, end), "void GetDesc%s(const char** ppBufferName, const void** ppBuffer, size_t* pSize) { *ppBufferName = \"%s\"; *ppBuffer = sBuffer; *pSize = %llu; }\n", bufferId, oGetFilebase(_BufferName), sz);

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

typedef std::tr1::unordered_map<std::string, std::string> headers_t;
bool CollectHeaders(headers_t& _Headers, const char* _StrSourceCode, const char* _SourceCodeDirectory, const macros_t& _Macros, const char* _HeaderSearchPath, const oFUNCTION<bool(const char* _Path)>& _PathExists, const oFUNCTION<bool(void* _pDestination, size_t _SizeofDestination, const char* _Path)>& _LoadHeaderFile)
{
	// @oooii-tony: I run into this pattern a lot... I have a C-interface that 
	// works with char*'s and is careful to assign all allocation of such buffers
	// to the user. I have a need for a string hash that needs to handle its own
	// allocation, so std::string is a decent option, but I am forever creating 
	// temp std::strings when using char*'s to interact with it. Is there a better
	// way?

	// Make an internal copy so we can clean up the defines
	std::vector<char> sourceCodeCopy(oStrlen(_StrSourceCode) + 1);
	oStrcpy(oStd::data(sourceCodeCopy), sourceCodeCopy.capacity(), _StrSourceCode);

	if (!oZeroIfdefs(_Macros, oStd::data(sourceCodeCopy), 0))
		return false;

	const char* pContext = oStd::data(sourceCodeCopy);
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
			return oErrorSetLast(std::errc::no_such_file_or_directory, "Could not find %s in search path %s;%s", headerRelativePath, _SourceCodeDirectory, _HeaderSearchPath);

		// This risks redundantly checking the same file, but allows a quick check
		// to prevent finding the same include multiple times just above
		_Headers[strHeaderRelativePath] = headerFullPath;

		if (!_LoadHeaderFile(oStd::data(headerCode), headerCode.capacity(), headerFullPath))
			return oErrorSetLast(std::errc::io_error, "Load failed: %s", headerFullPath);

		if (!CollectHeaders(_Headers, (const char*)oStd::data(headerCode), _SourceCodeDirectory, _Macros, _HeaderSearchPath, _PathExists, _LoadHeaderFile))
			return false; // pass through error from CollectHeaders
	}

	return true;
}

bool oHeadersAreUpToDate(const char* _StrSourceCode
	, const char* _SourceFullPath
	, const oMACRO* _pMacros
	, const oFUNCTION<bool(const char* _Path)>& _PathExists
	, const oFUNCTION<time_t(const char* _Path)>& _GetModifiedDate
	, const oFUNCTION<bool(void* _pDestination, size_t _SizeofDestination, const char* _Path)>& _LoadHeaderFile
	, const char* _HeaderSearchPath)
{
	macros_t macros;
	HashMacros(macros, _pMacros);

	// Make an internal copy so we can clean up the defines
	std::vector<char> sourceCodeCopy(oStrlen(_StrSourceCode) + 1);
	oStrcpy(oStd::data(sourceCodeCopy), oStd::size(sourceCodeCopy), _StrSourceCode);

	if (!oZeroIfdefs(macros, oStd::data(sourceCodeCopy), 0))
		return false;

	// Get the base point, the specified source file
	const time_t sourceTimestamp = _GetModifiedDate(_SourceFullPath);
	if (!sourceTimestamp)
		return oErrorSetLast(std::errc::no_such_file_or_directory, "Could not find %s", _SourceFullPath);

	char sourcePath[_MAX_PATH];
	oStrcpy(sourcePath, _SourceFullPath);
	oTrimFilename(sourcePath);

	// Hash headers so we don't recurse down deep include trees more than once
	headers_t headers;
	if (!CollectHeaders(headers, oStd::data(sourceCodeCopy), sourcePath, macros, _HeaderSearchPath, _PathExists, _LoadHeaderFile))
		return false;

	// Go through unique headers and check dates
	oFOR(auto pair, headers)
	{
		const std::string& headerFullPath = pair.second;
		const time_t headerTimestamp = _GetModifiedDate(headerFullPath.c_str());
		if (!headerTimestamp)
			return oErrorSetLast(std::errc::no_such_file_or_directory, "Could not find %s", _SourceFullPath);

		if (headerTimestamp > sourceTimestamp)
		{
			oErrorSetLast(0, "%s is out of date because %s is newer", _SourceFullPath, headerFullPath.c_str());
			return false;
		}
	}

	return true;
}