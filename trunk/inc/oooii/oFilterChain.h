// $(header)
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

interface oFilterChain : oInterface
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
