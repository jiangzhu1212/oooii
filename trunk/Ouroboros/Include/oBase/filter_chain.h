// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.
// Describes an ordered list of regular expressions that refine a query 
// through a series of inclusion patterns and exclusion patterns. For example, 
// if writing a parsing tool that operated on C++ source this can be used to 
// handle command line options for including all symbols from source files 
// beginning with 's' but not std::vector symbols, except std::vector<SYMBOL>.
// ParseCpp -includefiles "s.*" -excludesymbols "std\:\:vector.*" -include "std\:\:vector<SYMBOL>"

#pragma once
#ifndef oBase_filter_chain_h
#define oBase_filter_chain_h

#include <oBase/algorithm.h>
#include <oBase/throw.h>
#include <vector>

namespace ouro {

class filter_chain
{
public:
	enum type
	{
		exclude1,
		include1,
		exclude2,
		include2,
	};

	struct filter
	{
		const char* regex;
		type type;
	};
	
	filter_chain() {}
	filter_chain(const filter* _pFilters, size_t _NumFilters, char* _StrError, size_t _SizeofStrError) { compile(Filters, _pFilters, _NumFilters, _StrError, _SizeofStrError); }
	template<size_t num> filter_chain(const filter (&_pFilters)[num], char* _StrError, size_t _SizeofStrError) { compile(Filters, _pFilters, num, _StrError, _SizeofStrError); }
	template<size_t num, size_t size> filter_chain(const filter (&_pFilters)[num], char (&_StrError)[size]) { compile(Filters, _pFilters, num, _StrError, size); }
	filter_chain(filter_chain&& _That) { operator=(std::move(_That)); }

	filter_chain& operator=(filter_chain&& _That)
	{
		if (this != &_That)
			Filters = std::move(_That.Filters);
		return *this;
	}


	// Returns true if one of the symbols specified passes all filters. A pass is a match 
	// to an include-type pattern or a mismatch to an exclude-type pattern. Two symbols are 
	// provided so that filters can be interleaved. (This was originally written for including/
	// excluding C++ symbols (sym1) in various source files (sym2).)
	bool passes(const char* _Symbol1, const char* _Symbol2 = nullptr, bool _PassesWhenEmpty = true) const
	{
		if (Filters.empty()) return _PassesWhenEmpty;
		// Initialize starting value to the opposite of inclusion. Thus setting up the first 
		// filter as defining the most general set from which subsequent filters will reduce.
		bool passes = Filters[0].first == exclude1 || Filters[0].first == exclude2;
		for (const auto& pair : Filters)
		{
			const char* s = _Symbol1;
			if (pair.first == include2 || pair.first == exclude2) s = _Symbol2;
			if (!s) passes = true;
			else if (regex_match(s, pair.second)) passes = pair.first & 0x1; // incl enums are odd, excl are even.
		}
		return passes;
	}

private:
	typedef std::vector<std::pair<type, std::regex> > filters_t;
	filters_t Filters;

	// Compile an ordered list of regular expressions that will mark symbols as 
	// either included or excluded.
	void compile(filters_t& _CompiledFilters, const filter* _pFilters, size_t _NumFilters, char* _StrError, size_t _SizeofStrError)
	{
		_CompiledFilters.reserve(_NumFilters);
		for (size_t i = 0; _pFilters && _pFilters->regex && i < _NumFilters; i++, _pFilters++)
		{
			std::regex re;
			try { re = std::regex(_pFilters->regex, std::regex_constants::icase); }
			catch (std::regex_error& e)
			{
				_CompiledFilters.clear();
				oTHROW_INVARG("could not compile regular expression \"%s\"", e.what());
			}

			_CompiledFilters.push_back(filters_t::value_type(_pFilters->type, re));
		}
	}
};

 }

#endif
