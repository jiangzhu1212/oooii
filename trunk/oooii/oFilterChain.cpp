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
#include <oooii/oFilterChain.h>
#include <oooii/oAssert.h>
#include <oooii/oErrno.h>
#include <oooii/oRefCount.h>
#include <oooii/oSTL.h>
#include <regex>
#include <vector>

using namespace std;
using namespace std::tr1;

struct CompiledFilter : public oFilterChain::FILTER
{
	CompiledFilter(const oFilterChain::FILTER& _Filter, char* _StrError, size_t _SizeofStrError)
		: oFilterChain::FILTER(_Filter)
	{
		*_StrError = 0;
		oTryCompilelRegex(CompiledRegularExpression, _StrError, _SizeofStrError, RegularExpression, std::tr1::regex_constants::icase);
	}

	regex CompiledRegularExpression;
};

const oGUID& oGetGUID( threadsafe const oFilterChain* threadsafe const * )
{
	// {C5486617-22A2-4e84-9095-6A8C9E86FF65}
	static const oGUID oIIDFilterChain = { 0xc5486617, 0x22a2, 0x4e84, { 0x90, 0x95, 0x6a, 0x8c, 0x9e, 0x86, 0xff, 0x65 } };
	return oIIDFilterChain;
}

struct FilterChain_Impl : public oFilterChain
{
	oDEFINE_REFCOUNT_INTERFACE(RefCount);
	oDEFINE_TRIVIAL_QUERYINTERFACE(oGetGUID<oFilterChain>());

	// If any compilation fails, _StrError will have an error message in it.
	FilterChain_Impl(const FILTER* _pFilters, size_t _NumFilters, char* _StrError, size_t _SizeofStrError)
		: Compiled(true)
	{
		Filters.reserve(_NumFilters);
		for (size_t i = 0; _pFilters->RegularExpression && i < _NumFilters; i++, _pFilters++)
		{
			Filters.push_back(CompiledFilter(*_pFilters, _StrError, _SizeofStrError));
			if (*_StrError)
			{
				Compiled = false;
				break;
			}
		}
	}

	void GetFilterChain(FILTER* _pFilters, size_t _NumFilters) threadsafe override
	{
		filters_t& filters = thread_cast<filters_t&>(Filters); // safe because it's read-only and there are no modifiers to the data
		memset(_pFilters, 0, sizeof(FILTER) * _NumFilters);
		for (size_t i = 0; i < __min(filters.size(), _NumFilters); i++, _pFilters++)
			*_pFilters = *static_cast<FILTER*>(&filters[i]);
	}

	bool Passes(const char* _Symbol1, const char* _Symbol2, bool _PassesWhenEmpty) const threadsafe override
	{
		oASSERT(Compiled, "filters not compiled");
		//oTRACE("Start oFilterChain on %s in %s%s", _Symbol1, _Symbol2, _Passes ? " (passes on empty)" : "");

		filters_t& filters = thread_cast<filters_t&>(Filters);
		if (filters.empty())
			return _PassesWhenEmpty;

		bool passes = filters[0].FilterType == EXCLUDE1 || filters[0].FilterType == EXCLUDE2;

		for (filters_t::iterator it = filters.begin(); it != filters.end(); ++it)
		{
			const char* s = _Symbol1;
			if (it->FilterType == INCLUDE2 || it->FilterType == EXCLUDE2)
				s = _Symbol2;

			if (!s)
				passes = true;

			else if (regex_match(s, it->CompiledRegularExpression))
				passes = it->FilterType & 0x1; // incl enums are odd, excl are even.

			//Detail("Refl.FilterChain"
			//	, "-- %s%s \"%s\" on \"%s\" %s"
			//	, (it->Type & 0x1) ? "Include " : "Exclude "
			//	, (it->Type < EXCLUDE2) ? "Symbol1" : "Symbol2"
			//	, it->RegularExpression
			//	, s
			//	, passes ? "passes" : "fails");
		}

		//oTRACE("Final result for %s in %s: %s", _Symbol1, _Symbol2, passes ? "passes" : "does not pass");
		return passes;
	}

protected:
	typedef vector<CompiledFilter> filters_t;
	filters_t Filters;
	oRefCount RefCount;
	bool Compiled;
};

bool oFilterChain::Create(const FILTER* _pFilters, size_t _NumFilters, threadsafe oFilterChain** _ppFilterChain)
{
	if (!_pFilters || !_NumFilters || !_ppFilterChain) return false;

	char err[1024];
	*_ppFilterChain = new FilterChain_Impl(_pFilters, _NumFilters, err, oCOUNTOF(err));
	if (*err)
	{
		oSetLastError(EINVAL, err);
		delete *_ppFilterChain;
		*_ppFilterChain = 0;
	}

	return !!*_ppFilterChain;
}
