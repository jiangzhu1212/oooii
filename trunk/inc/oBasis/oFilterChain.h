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
// Describes an ordered list of regular expressions that refine a
// query through a series of inclusion patterns and exclusion 
// patterns. For example, if writing a parsing tool that operated
// on C++ source this can be used to handle command line options
// for including all symbols from source files beginning with 's',
// but not std::vector symbols, except std::vector<SYMBOL>.
// ParseCpp -includefiles "s.*" -excludesymbols "std\:\:vector.*" -include "std\:\:vector<SYMBOL>"
#pragma once
#ifndef oFilterChain_h
#define oFilterChain_h

#include <oBasis/oAlgorithm.h>
#include <oBasis/oThreadsafe.h>
#include <vector>

class oFilterChain
{
public:
	enum TYPE
	{
		EXCLUDE1,
		INCLUDE1,
		EXCLUDE2,
		INCLUDE2,
	};

	struct FILTER
	{
		const char* RegularExpression;
		TYPE Type;
	};

	oFilterChain(const FILTER* _pFilters, size_t _NumFilters, char* _StrError, size_t _SizeofStrError, bool* _pSuccess) { *_pSuccess = Compile(_pFilters, _NumFilters, _StrError, _SizeofStrError); }
	template<size_t num> oFilterChain(const FILTER (&_pFilters)[num], char* _StrError, size_t _SizeofStrError, bool* _pSuccess) { *_pSuccess = Compile(_pFilters, num, _StrError, _SizeofStrError); }
	template<size_t num, size_t size> oFilterChain(const FILTER (&_pFilters)[num], char (&_StrError)[size], bool* _pSuccess) { *_pSuccess = Compile(_pFilters, num, _StrError, size); }

	// Returns true if one of the symbols specified passes all filters.
	// A pass is a match to an include-type pattern or a mismatch to an
	// exclude-type pattern. Two symbols are provided so that filters
	// can be interleaved. (This was originally written for including/
	// excluding C++ symbols (sym1) in various source files (sym2).)
	bool Passes(const char* _Symbol1, const char* _Symbol2 = nullptr, bool _PassesWhenEmpty = true) const threadsafe
	{
		filters_t& filters = thread_cast<filters_t&>(Filters); // contents are mutable, so any thread can read at any time
		if (filters.empty()) return _PassesWhenEmpty;
		// Initialize starting value to the opposite of inclusion, thus setting up
		// the first filter as defining the most general set from which subsequent
		// filters will reduce.
		bool passes = filters[0].first == EXCLUDE1 || filters[0].first == EXCLUDE2;
		for (filters_t::const_iterator it = filters.begin(); it != filters.end(); ++it)
		{
			const char* s = _Symbol1;
			if (it->first == INCLUDE2 || it->first == EXCLUDE2) s = _Symbol2;
			if (!s) passes = true;
			else if (regex_match(s, it->second)) passes = it->first & 0x1; // incl enums are odd, excl are even.
		}
		return passes;
	}

private:
	typedef std::vector<std::pair<TYPE, std::tr1::regex> > filters_t;
	filters_t Filters;

	// Compile an ordered list of regular expressions that will mark symbols as 
	// either included or excluded.
	bool Compile(const FILTER* _pFilters, size_t _NumFilters, char* _StrError, size_t _SizeofStrError)
	{
		Filters.clear();
		Filters.reserve(_NumFilters);
		for (size_t i = 0; _pFilters && _pFilters->RegularExpression && i < _NumFilters; i++, _pFilters++)
		{
			std::tr1::regex re;
			if (!oTryCompileRegex(re, _StrError, _SizeofStrError, _pFilters->RegularExpression, std::tr1::regex_constants::icase))
			{
				Filters.clear();
				return false;
			}

			Filters.push_back(filters_t::value_type(_pFilters->Type, re));
		}

		return true;
	}
};

#endif
