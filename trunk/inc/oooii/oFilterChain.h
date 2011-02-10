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
#pragma once
#ifndef oFilterChain_h
#define oFilterChain_h

// Encapsulate the details of a chain of regular expressions (std::tr1::regex)
// for filtering a set. Evaluation overrides right to left in priority, so 
// a base filter starts on the left, and is refined with additional filters.
// Currently this supports two criteria, such as symbol and filename for source
// code. This is intended to better enable filter command line parameter 
// filtering when operating on large sets. Rather than just direct string 
// matching this provides hierarchical/override regular expression pattern 
// matching in an easily usable form.

#include <oooii/oInterface.h>

interface oFilterChain : public oInterface
{
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
		TYPE FilterType;
	};

	static bool Create(const FILTER* _pFilters, size_t _NumFilters, threadsafe oFilterChain** _ppFilterChain);
	template<size_t size> static inline bool Create(const FILTER (&_pFilters)[size], threadsafe oFilterChain** _ppFilterChain) { return Create(_pFilters, size, _ppFilterChain); }

	virtual void GetFilterChain(FILTER* _pFilters, size_t _NumFilters) threadsafe = 0;
	template<size_t size> inline void GetFilterChain(FILTER (&_pFilters)[size]) { GetFilterChain(_pFilters, size); }
	
	virtual bool Passes(const char* _Symbol1, const char* _Symbol2, bool _PassesWhenEmpty = true) const threadsafe = 0;
};

#endif
