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
#include <memory.h>
#include <regex>
#include <string>
#include <oBase/throw.h>
#include <unordered_map>

using namespace std;

namespace ouro {

struct ifdef_block
{
	enum type
	{
		unknown,
		ifdef,
		ifndef,
		if_,
		elif,
		else_,
		endif,
	};

	enum type type;
	const char* expression_start; // what comes after one of the opening TYPES. NULL for ELSE and ENDIF
	const char* expression_end;
	const char* block_start; // The data within the block
	const char* block_end;
};

// Matches #ifdef SYM, #ifndef SYM, #else, #endif
// Match1: ifndef or ifdef
// Match2: n or empty
// Match3: SYM or empty
// Match4: else or elseif or empty
// Match5: if or empty
// Match6: endif
static regex reIfdef("#[ \\t]*(if(n?)def)[ \\t]+([a-zA-Z0-9_]+)|#[ \\t]*(else(if)?)|#[ \\t]*(endif)", regex_constants::optimize); // ok static (duplication won't affect correctness)

static enum ifdef_block::type get_type(const cmatch& _Matches)
{
	const char* Opener = _Matches[1].first;
	const char* Else = _Matches[4].first;
	const char* Endif = _Matches[6].first;

	if (!memcmp(Opener, "ifdef", 5)) return ifdef_block::ifdef;
	else if (!memcmp(Opener, "ifndef", 6)) return ifdef_block::ifndef;
	else if (!memcmp(Opener, "if", 2)) return ifdef_block::if_;
	else if (!memcmp(Else, "elif", 4)) return ifdef_block::elif;
	else if (!memcmp(Else, "else", 4)) return ifdef_block::else_;
	else if (!memcmp(Endif, "endif", 4)) return ifdef_block::endif;
	return ifdef_block::unknown;
}

static void next_matching_ifdef(ifdef_block* _pBlocks, size_t _MaxNumBlocks, size_t *_pNumValidBlocks, const char* _StrSourceCodeBegin, const char* _StrSourceCodeEnd)
{
	ifdef_block* pLastBlock = 0; // used to late-populate block_end
	ifdef_block* pCurBlock = 0;
	size_t blockIndex = 0;
	int open = 0;

	const char* cur = _StrSourceCodeBegin;

	cmatch matches;
	while (regex_search(cur, matches, reIfdef))
	{
		if (_StrSourceCodeEnd && matches[0].first >= _StrSourceCodeEnd)
			break;

		if (blockIndex >= _MaxNumBlocks-1)
			oTHROW(no_buffer_space, "block buffer too small");

		enum ifdef_block::type type = get_type(matches);

		// increment openness for each opening statement
		switch (type)
		{
			case ifdef_block::if_:
			case ifdef_block::ifdef:
			case ifdef_block::ifndef:
				open++;
				break;
			default:
				break;
		}

		// add new blocks on first and at-level-1 openness
		if (open == 1)
		{
			pCurBlock = &_pBlocks[blockIndex++];
			memset(pCurBlock, 0, sizeof(ifdef_block));
			pCurBlock->type = type;

			// Record expression and block info if at same level
			switch (type)
			{
				case ifdef_block::if_:
				case ifdef_block::ifdef:
				case ifdef_block::ifndef:
					pCurBlock->expression_start = matches[3].first;
					pCurBlock->expression_end = matches[3].second;
					pCurBlock->block_start = matches[0].second;
					break;

				case ifdef_block::elif:
					// todo: record expression
					// pass thru

				case ifdef_block::else_:
				case ifdef_block::endif:
					pCurBlock->block_start = matches[0].second;
					pLastBlock->block_end = matches[0].first;
					break;

				oASSUME(0);
			}
		}

		// If a closing statement, close out everything
		if (type == ifdef_block::endif)
		{
			open--;
			if (!open)
			{
				pCurBlock->block_start = matches[0].second;
				pCurBlock->block_end = matches[0].second;
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
}

typedef unordered_map<string, string> macros_t;

// An internal version that works on an already-hash macros container
static char* zero_ifdefs_internal(const macros_t& _Macros, char* _StrSourceCodeBegin, char* _StrSourceCodeEnd, char _Replacement)
{
	size_t numBlocks = 0;
	ifdef_block blocks[32];
	next_matching_ifdef(blocks, oCOUNTOF(blocks), &numBlocks, _StrSourceCodeBegin, _StrSourceCodeEnd);
	
	if (numBlocks)
	{
		bool zeroBlock = false;
		for (ifdef_block* pCurBlock = blocks; pCurBlock->type != ifdef_block::endif; pCurBlock++)
		{
			switch (pCurBlock->type)
			{
				case ifdef_block::ifdef:
					zeroBlock = _Macros.end() == _Macros.find(string(pCurBlock->expression_start, pCurBlock->expression_end));
					break;
				case ifdef_block::ifndef:
					zeroBlock = _Macros.end() != _Macros.find(string(pCurBlock->expression_start, pCurBlock->expression_end));
					break;

				case ifdef_block::else_:
					// else will be the last block before endif, so it means that if we 
					// haven't zeroed a block yet, we should now
					zeroBlock = !zeroBlock;
					break;

				default:
					throw std::invalid_argument("Unhandled case (Did #if/#elif get implemented?)");
			}

			if (zeroBlock)
			{
				for (const char* cur = pCurBlock->block_start; cur < pCurBlock->block_end; cur++)
					*const_cast<char*>(cur) = _Replacement;
			}

			else
			{
				// If we're not going to zero this block, then recurse into it looking for
				// other blocks to zero
				zero_ifdefs_internal(_Macros, const_cast<char*>(pCurBlock->block_start), const_cast<char*>(pCurBlock->block_end), _Replacement);
			}
		}
	}

	return _StrSourceCodeBegin;
}

static void hash_macros(macros_t& _OutMacros, const macro* _pMacros)
{
	while (_pMacros && _pMacros->symbol && _pMacros->value)
	{
		_OutMacros[_pMacros->symbol] = _pMacros->value;
		_pMacros++;
	}
}

char* zero_ifdefs(char* _StrSourceCode, const macro* _pMacros, char _Replacement)
{
	macros_t macros;
	hash_macros(macros, _pMacros);
	return zero_ifdefs_internal(macros, _StrSourceCode, nullptr, _Replacement);
}

} // namespace ouro
