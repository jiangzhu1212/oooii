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
// Collection of some utility functions useful when working with STL
#pragma once
#ifndef oSTL_h
#define oSTL_h

#pragma warning(disable:4530) // C++ exception handler used, but unwind semantics are not enabled. Specify /EHsc
#include <algorithm>
#include <array>
#include <functional>
#include <string>
#include <vector>
#include <regex>
#pragma warning(default:4530)

template<typename T, size_t N> class oArray
{
	// std::tr1 array doesn't separate the notion of "active" size() verses max
	// capacity, so wrap it to add this functionality for those times where we 
	// basically want statically allocated memory for a std::vector without 
	// dealing with some of the subtlty of a std::allocator that doesn't really
	// allocate.
public:
	typedef std::tr1::array<T, N> array_t;
	typedef typename array_t::const_iterator const_iterator;
	typedef typename array_t::const_pointer const_pointer;
	typedef typename array_t::const_reference const_reference;
	typedef typename array_t::const_iterator const_reverse_iterator;
	typedef typename array_t::difference_type difference_type;
	typedef typename array_t::iterator iterator;
	typedef typename array_t::pointer pointer;
	typedef typename array_t::reference reference;
	typedef typename array_t::iterator reverse_iterator;
	typedef typename array_t::size_type size_type;
	typedef typename array_t::value_type value_type;

	oArray()
		: Size(0)
	{
	}

	oArray(const oArray& _Other)
		: Array(_Other.Array)
		, Size(_Other.Size)
	{
	}

	// void assign(iterator _First, iterator _Last) {}

	void fill(const T& _Value) { std::fill(begin(), end(), _Value); }
	void assign(size_type _N, const T& _Value) { Size = _N; fill(_Value); }

	reference at(size_type _Index) { return Array.at(_Index); }
	const_reference at(size_type _Index) const { return Array.at(_Index); }

	reference operator[](size_t _Index) { return Array[_Index]; }
	const_reference operator[](size_t _Index) const { return Array[_Index]; }
	const array_t& operator=(const array_t& _Other) { Array = _Other.Array; Size = _Other.Size; }

	reference front() { return Array.front(); }
	reference back() { return Array.at(Size-1); }
	const_reference front() const { return Array.front(); }
	const_reference back() const { return Array.at(Size-1); }

	iterator begin() { return Array.begin(); }
	iterator end() { return Array.begin() + Size; }
	const_iterator begin() const { return Array.begin(); }
	const_iterator end() const { return Array.begin() + Size; }
	const_iterator cbegin() const { return Array.cbegin(); }
	const_iterator cend() const { return Array.begin() + Size; }

	reverse_iterator rbegin() { return Array.rend() + Size; }
	reverse_iterator rend() { return Array.rend(); }
	const_reverse_iterator crbegin() const { return Array.rend() + Size; }
	const_reverse_iterator crend() const { return Array.crend(); }

	T* data() { return Array.data(); }
	const T* data() const { return Array.data(); }

	void resize(size_t _NewSize, T _InitialValue = T())
	{
		oASSERT(_NewSize < max_size(), "New size %u too large for oArray", _NewSize);

		if (_NewSize < Size)
			for (size_t i = _NewSize; i < Size; i++)
				Array[i].~T();

		for (size_t i = Size; i < _NewSize; i++)
			Array[i] = _InitialValue;

		Size = _NewSize;
	}

	void clear() { resize(0); }
	bool empty() const { return Size == 0; }
	size_t max_size() const { return Array.max_size(); }
	size_t size() const { return Size; }
	size_t capacity() const { return Array.max_size(); }

	void push_back(const T& _Value)
	{
		oASSERT(Size < max_size(), "Array is at capacity (%u)", max_size());
		Array[Size++] = _Value;
	}

	void pop_back()
	{
		if (Size)
			Array[--Size].~T();
	}

	iterator insert(iterator _Position, const T& _Value)
	{
		oASSERT(Size < max_size(), "Array is at capacity (%u)", max_size());
		size_t PositionIndex = std::distance(begin(), _Position);

		for (size_t i = Size; i > PositionIndex; i--)
			Array[i] = Array[i-1];
		Array[PositionIndex] = _Value;
		Size++;

		return _Position;
	}

	// void insert(iterator _Position, size_type _N, const T& _Value) {}
	// void insert(iterator _Position, iterator _First, iterator _Last) {}

	iterator erase(iterator _Position)
	{


		return _Position;
	}

	iterator erase(iterator _First, iterator _Last)
	{
		for (iterator it = _First; it != _Last; ++it)
			(*it).~T();

		memcpy(_First, _Last, sizeof(T) * std::distance(_Last, end()));
		Size -= std::distance(_First, _Last);

		return _First;
	}

protected:
	array_t Array;
	size_t Size;
};

// _____________________________________________________________________________
// container base implementation

namespace oSTL {
	namespace detail {

// Insert _Item only if its value does not already exist in the container
// Returns the index at which the item was inserted, or where a pre-existing
// one was found.
template<typename T, typename containerT> size_t oPushBackUnique(containerT& _Container, const T& _Item)
{
	size_t index = _Container.size();
	typename containerT::iterator it = std::find(_Container.begin(), _Container.end(), _Item);
	if (it == _Container.end())
		_Container.push_back(_Item);
	else
		index = std::distance(_Container.begin(), it);
	return index;
};

// Sets _Container[_Index] = _Item in a way that first ensures size() is capable
template<typename T, typename containerT> void oSafeSet(containerT& _Container, size_t _Index, const T& _Item)
{
	if (_Container.size() <= _Index)
		_Container.resize(_Index + 1);
	_Container[_Index] = _Item;
}

// Finds _Item by value, and returns the value after erasing it
template<typename T, typename containerT> bool oFindAndErase(containerT& _Container, const T& _Item)
{
	typename containerT::iterator it = std::find(_Container.begin(), _Container.end(), _Item);
	if (it != _Container.end())
	{
		_Container.erase(it);
		return true;
	}
	return false;
}

// Same as above, but using std::find_if
template<typename T, typename containerT, typename predicateT> bool oFindIfAndErase(containerT& _Container, const predicateT& _Predicate, T* _pFound = 0)
{
	typename containerT::iterator it = std::find_if(_Container.begin(), _Container.end(), _Predicate);
	if (it != _Container.end())
	{
		if (_pFound)
			*_pFound = *it;
		_Container.erase(it);
		return true;
	}
	return false;
}

	} // namespace oSTL
} // namespace detail

// _____________________________________________________________________________
// oArray utilities

template<typename T, size_t N> size_t oPushBackUnique(oArray<T, N>& _Array, const T& _Item) { return oSTL::detail::oPushBackUnique<T, oArray<T, N> >(_Array, _Item); }
template<typename T, size_t N> void oSafeSet(oArray<T, N>& _Array, size_t _Index, const T& _Item){ oSTL::detail::oSafeSet<T, oArray<T, N> >(_Array, _Index, _Item); }
template<typename T, size_t N> bool oContains(const oArray<T, N>& _Array, const T& _Item) { return _Array.end() != std::find(_Array.begin(), _Array.end(), _Item); }
template<typename T, size_t N> bool oFindAndErase(oArray<T, N>& _Array, const T& _Item) { return oSTL::detail::oFindAndErase<T, oArray<T, N> >(_Array, _Item); }
template<typename T, size_t N, class predicateT> bool oFindIfAndErase(oArray<T, N>& _Array, const predicateT& _Predicate, T* _pFound = 0) { return oSTL::detail::oFindIfAndErase<T, oArray<T, N> >(_Array, _Predicate, _pFound); }

// Get the base pointer to the buffer containing the vector
template <typename T, size_t N> T* oGetData(oArray<T, N>& _Array) { return _Array.empty() ? 0 : _Array.data(); }

// Get the const base pointer to the buffer containing the vector
template <typename T, size_t N> const T* oGetData(const oArray<T, N>& _Array) { return _Array.empty() ? 0 : _Array.data(); }

// _____________________________________________________________________________
// std vector utilities

template<typename T, typename Alloc> size_t oPushBackUnique(std::vector<T, Alloc>& _Vector, const T& _Item) { return oSTL::detail::oPushBackUnique<T, std::vector<T, Alloc> >(_Vector, _Item); }
template<typename T, typename Alloc> void oSafeSet(std::vector<T, Alloc>& _Vector, size_t _Index, const T& _Item) { oSTL::detail::oSafeSet<T, std::vector<T, Alloc> >(_Vector, _Index, _Item); }
template<typename T, class Alloc> bool oContains(const std::vector<T, Alloc>& _Vector, const T& _Item) { return _Vector.end() != std::find(_Vector.begin(), _Vector.end(), _Item); }
template<typename T, class Alloc> bool oFindAndErase(std::vector<T, Alloc>& _Vector, const T& _Item) { return oSTL::detail::oFindAndErase<T, std::vector<T, Alloc> >(_Vector, _Item); }
template<typename T, class Alloc, class Predicate> bool oFindIfAndErase(std::vector<T, Alloc>& _Vector, const Predicate& _Predicate, T* _pFound = 0) { return oSTL::detail::oFindIfAndErase<T, std::vector<T, Alloc> >(_Vector, _Predicate, _pFound); }

// Force-deallocate the reserved memory/capacity of the vector
template <typename T, class Alloc> void oZeroCapacity(std::vector<T, Alloc>& _Vector)
{
	// Stroustrup. TC++PL (third edition). Section 16.3.8.
	std::vector<T, Alloc>().swap(_Vector);
}

// Just like malloc, reserve can fail to allocate, so check it
template <typename T> bool oSafeReserve(T& _Container, typename T::size_type _Capacity)
{
	size_t oldCapacity = _Container.capacity();
	bool couldFail = oldCapacity != _Capacity;
	_Container.reserve(_Capacity);
	if (couldFail && _Container.capacity() == _Capacity)
		return false;
	return true;
}

// Get the base pointer to the buffer containing the vector
template <typename T, class Alloc> T* oGetData(std::vector<T, Alloc>& _Vector) { return _Vector.empty() ? 0 : &_Vector.front(); }

// Get the const base pointer to the buffer containing the vector
template <typename T, class Alloc> const T* oGetData(const std::vector<T, Alloc>& _Vector) { return _Vector.empty() ? 0 : &_Vector.front(); }

// _____________________________________________________________________________
// regex helpers

inline void oRegexCopy(std::string& _OutString, const std::tr1::cmatch& _Matches, size_t _NthMatch) { _OutString.assign(_Matches[_NthMatch].first, _Matches[_NthMatch].length()); }
errno_t oRegexCopy(char *_StrDestination, size_t _SizeofStrDestination, const std::tr1::cmatch& _Matches, size_t _NthMatch);
template<size_t size> inline errno_t oRegexCopy(char (&_StrDestination)[size], const std::tr1::cmatch& _Matches, size_t _NthMatch) { return oRegexCopy(_StrDestination, size, _Matches, _NthMatch); }

// Convert a regex error into a human-readable string
const char* oRegexGetError(std::tr1::regex_constants::error_type _RegexError);

// Handles the exception thrown by a bad compile and fills the specified string 
// with the error generated when this function returns false. If this returns true,
// _OutRegex contains a valid, compiled regex.
bool oTryCompilelRegex(std::tr1::regex& _OutRegex, char* _StrError, size_t _SizeofStrError, const char* _StrRegex, std::tr1::regex_constants::syntax_option_type _Flags = std::tr1::regex_constants::ECMAScript);
template<size_t size> inline bool oTryCompilelRegex(std::tr1::regex& _OutRegex, char (&_StrError)[size], const char* _StrRegex, std::tr1::regex_constants::syntax_option_type _Flags = std::tr1::regex_constants::ECMAScript) { return oTryCompilelRegex(_OutRegex, _StrError, size, _StrRegex, _Flags); }

// _____________________________________________________________________________
// std algorithms support: equality predicates using string compare for strings
// structs with an 'I' on the end are case-insensitive

template<class T> struct oStdEquals : public std::unary_function<T, bool>
{	// provide the default operator==
	oStdEquals(const T& t) : _t(t) {}
	bool operator()(const T& t) const { return t == _t; }
private:
	oStdEquals() {}
	T _t;
};

template<> struct oStdEquals<const char*>
{	// compare strings rather than test pointer equality
	oStdEquals(const char* str) : _t(str) {}
	bool operator()(const char* str) { return !strcmp(str, _t); }
private:
	oStdEquals() {}
	const char* _t;
};

template<> struct oStdEquals<const std::string>
{
	oStdEquals(const char* str) : _t(str) {}
	oStdEquals(const std::string& str) : _t(str.c_str()) {}
	bool operator()(const std::string& str) { return !str.compare(_t); }
private:
	oStdEquals() {}
	const char* _t;
};

template<class T> struct oStdEqualsI : public std::unary_function<T, bool>
{
	oStdEqualsI(const T& t) : _t(t) {}
	bool operator()(const T& t) const { return t == _t; }
private:
	oStdEqualsI() {}
	T _t;
};

template<> struct oStdEqualsI<const char*>
{
	oStdEqualsI(const char* str) : _t(str) {}
	bool operator()(const char* str) { return !_stricmp(str, _t); }
private:
	oStdEqualsI() {}
	const char* _t;
};

template<> struct oStdEqualsI<const std::string>
{
	oStdEqualsI(const char* str) : _t(str) {}
	oStdEqualsI(const std::string& str) : _t(str.c_str()) {}
	bool operator()(const std::string& str) { return !_stricmp(str.c_str(), _t); }
private:
	oStdEqualsI() {}
	const char* _t;
};

template<typename T> struct oStdLess : public std::binary_function<T, T, int> { int operator()(const T& x, const T& y) const { return x < y; } };
template<> struct oStdLess<const char*> { int operator()(const char* x, const char* y) const { return strcmp(x, y) < 0; } };
template<typename T> struct oStdLessI : public std::binary_function<T, T, int> { int operator()(const T& x, const T& y) const { return x < y; } };
template<> struct oStdLessI<const char*> { int operator()(const char* x, const char* y) const { return _stricmp(x, y) < 0; } };
template<typename T> struct oStdEqualTo : public std::binary_function<T, T, int> { int operator()(const T& x, const T& y) const { return x == y; } };
template<> struct oStdEqualTo<const char*> { int operator()(const char* x, const char* y) const { return !strcmp(x, y); } };
template<typename T> struct oStdEqualToI : public std::binary_function<T, T, int> { int operator()(const T& x, const T& y) const { return x == y; } };
template<> struct oStdEqualToI<const char*> { int operator()(const char* x, const char* y) const { return !_stricmp(x, y); } };

//////////////////////////////////////////////////////////////////////////
// OOOii-Kevin: foreach macro that works with std::vector matches 
// BOOST_FOREACH syntax.  Does not work with any other container type
//////////////////////////////////////////////////////////////////////////
#define oFOREACH_BASE( val, col, instance ) \
	const size_t count##instance = (col).size(); \
	size_t bLoopOnce##instance = 0; \
	for(size_t index##instance = 0; index##instance < count##instance; ++index##instance, bLoopOnce##instance = 0 ) \
	for( val = (col)[ index##instance ]; bLoopOnce##instance == 0; bLoopOnce##instance = 1 )

// Instantiator needed to ensure we have the same instance for all variables __COUNTER__ is called once
#define oFOREACH_BASE_INSTANTIATOR( val, col, instance ) \
	oFOREACH_BASE( val, col, instance )

#define oFOREACH( val, col ) \
	oFOREACH_BASE_INSTANTIATOR( val, col, __COUNTER__ )

#endif
