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

#include <oBase/algorithm.h>
#include <oBasis/oError.h>
#include <oStd/for.h>
#include <oBasis/oInitOnce.h>
#include <oConcurrency/thread_safe.h>
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

	oFilterChain(const FILTER* _pFilters, size_t _NumFilters, char* _StrError, size_t _SizeofStrError, bool* _pSuccess) { *_pSuccess = Compile(Filters.Initialize(), _pFilters, _NumFilters, _StrError, _SizeofStrError); }
	template<size_t num> oFilterChain(const FILTER (&_pFilters)[num], char* _StrError, size_t _SizeofStrError, bool* _pSuccess) { *_pSuccess = Compile(Filters.Initialize(), _pFilters, num, _StrError, _SizeofStrError); }
	template<size_t num, size_t size> oFilterChain(const FILTER (&_pFilters)[num], char (&_StrError)[size], bool* _pSuccess) { *_pSuccess = Compile(Filters.Initialize(), _pFilters, num, _StrError, size); }

	// Returns true if one of the symbols specified passes all filters.
	// A pass is a match to an include-type pattern or a mismatch to an
	// exclude-type pattern. Two symbols are provided so that filters
	// can be interleaved. (This was originally written for including/
	// excluding C++ symbols (sym1) in various source files (sym2).)
	bool Passes(const char* _Symbol1, const char* _Symbol2 = nullptr, bool _PassesWhenEmpty = true) const threadsafe
	{
		if (Filters->empty()) return _PassesWhenEmpty;
		// Initialize starting value to the opposite of inclusion, thus setting up
		// the first filter as defining the most general set from which subsequent
		// filters will reduce.
		bool passes = (*Filters)[0].first == EXCLUDE1 || (*Filters)[0].first == EXCLUDE2;
		
		oFOR(const auto& pair, *Filters)
		{
			const char* s = _Symbol1;
			if (pair.first == INCLUDE2 || pair.first == EXCLUDE2) s = _Symbol2;
			if (!s) passes = true;
			else if (regex_match(s, pair.second)) passes = pair.first & 0x1; // incl enums are odd, excl are even.
		}
		return passes;
	}

private:
	typedef std::vector<std::pair<TYPE, std::regex> > filters_t;
	oInitOnce<filters_t> Filters;

	// Compile an ordered list of regular expressions that will mark symbols as 
	// either included or excluded.
	bool Compile(filters_t& _CompiledFilters, const FILTER* _pFilters, size_t _NumFilters, char* _StrError, size_t _SizeofStrError)
	{
		_CompiledFilters.reserve(_NumFilters);
		for (size_t i = 0; _pFilters && _pFilters->RegularExpression && i < _NumFilters; i++, _pFilters++)
		{
			std::regex re;
			try { re = std::regex(_pFilters->RegularExpression, std::regex_constants::icase); }
			catch (std::regex_error& e)
			{
				_CompiledFilters.clear();
				return oErrorSetLast(std::errc::invalid_argument, "A regular expression could not be compiled: \"%s\"", e.what());
			}

			_CompiledFilters.push_back(filters_t::value_type(_pFilters->Type, re));
		}

		return true;
	}
};

#endif
