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
// Wrappers for common operations on STL containers that are not in <algorithm>
#pragma once
#ifndef oAlgorithm_h
#define oAlgorithm_h

#include <algorithm>
#include <functional>
#include <map>
#include <regex>
#include <string>
#include <unordered_map>
#include <vector>
#include <oBasis/oArray.h>
#include <oBasis/oFixedString.h>
#include <oBasis/oString.h>

// Some defines to make the source in this header a bit more approachable. These
// are not designed or intended to be used outside this header.
#define oALGO_ARRAY_TEMPLATE typename T, size_t N
#define oALGO_ARRAY oArray<T, N>

#define oALGO_VECTOR_TEMPLATE typename T, typename AllocatorT
#define oALGO_VECTOR std::vector<T, AllocatorT>

#define oALGO_LIST_TEMPLATE typename T, typename AllocatorT
#define oALGO_LIST std::list<T, AllocatorT>

#define oALGO_STRING_TEMPLATE typename T, typename TraitsT, typename AllocatorT
#define oALGO_STRING std::basic_string<T, TraitsT, AllocatorT>

#define oALGO_MAP_TEMPLATE typename KeyT, typename ValT, typename KeyLessT, typename AllocatorT
#define oALGO_MAP std::map<KeyT, ValT, KeyLessT, AllocatorT>

#define oALGO_UNORDERED_MAP_TEMPLATE typename KeyT, typename ValT, typename HasherT, typename KeyEqualT, typename AllocatorT
#define oALGO_UNORDERED_MAP std::unordered_map<KeyT, ValT, HasherT, KeyEqualT, AllocatorT>

// Convenience wrappers for std::find that shortens a search through the whole
// container (no explicit iterators) and also supports containers std::find 
// doesn't, such as those that use this->find() instead.
template<typename T, typename ContainerT> typename ContainerT::iterator oStdFind(ContainerT& _Container, const T& _Item) { return std::find(_Container.begin(), _Container.end(), _Item); }
template<typename T, typename ContainerT> typename ContainerT::const_iterator oStdFind(const ContainerT& _Container, const T& _Item) { return std::find(_Container.begin(), _Container.end(), _Item); }
template<typename ContainerT, typename Predicate> typename ContainerT::iterator oStdFindIf(ContainerT& _Container, const Predicate& _Predicate) { return std::find_if(_Container.begin(), _Container.end(), _Predicate); }
template<typename ContainerT, typename Predicate> typename ContainerT::const_iterator oStdFindIf(const ContainerT& _Container, const Predicate& _Predicate) { return std::find_if(_Container.begin(), _Container.end(), _Predicate); }
template<oALGO_UNORDERED_MAP_TEMPLATE> typename oALGO_UNORDERED_MAP::iterator oStdFind(oALGO_UNORDERED_MAP& _Container, const KeyT& _Item) { return _Container.empty() ? _Container.end() : _Container.find(_Item); }
template<oALGO_UNORDERED_MAP_TEMPLATE> typename oALGO_UNORDERED_MAP::const_iterator oStdFind(const oALGO_UNORDERED_MAP& _Container, const KeyT& _Item) { return _Container.empty() ? _Container.end() : _Container.find(_Item); }
template<oALGO_MAP_TEMPLATE> typename oALGO_MAP::iterator oStdFind(oALGO_MAP& _Container, const KeyT& _Item) { return _Container.empty() ? _Container.end() : _Container.find(_Item); }
template<oALGO_MAP_TEMPLATE> typename oALGO_MAP::const_iterator oStdFind(const oALGO_MAP& _Container, const KeyT& _Item) { return _Container.empty() ? _Container.end() : _Container.find(_Item); }

namespace detail {

template<typename T, typename ContainerT> size_t oPushBackUnique(ContainerT& _Container, const T& _Item)
{
	size_t index = _Container.size();
	typename ContainerT::iterator it = oStdFind(_Container, _Item);
	if (it == _Container.end())
		_Container.push_back(_Item);
	else
		index = std::distance(_Container.begin(), it);
	return index;
};

template<typename T, typename ContainerT> void oSafeSet(ContainerT& _Container, size_t _Index, const T& _Item)
{
	if (_Container.size() <= _Index)
		_Container.resize(_Index + 1);
	_Container[_Index] = _Item;
}

template<typename T, typename ContainerT> size_t oSparseSet(ContainerT& _Container, const T& _Item)
{
	for (typename ContainerT::iterator it = _Container.begin(); it != _Container.end(); ++it)
	{
		if (!*it)
		{
			*it = _Item;
			return std::distance(_Container.begin(), it);
			break;
		}
	}

	_Container.push_back(_Item);
	return _Container.size()-1;
}

template<typename T, typename ContainerT> bool oFindAndErase(ContainerT& _Container, const T& _Item)
{
	typename ContainerT::iterator it = oStdFind(_Container, _Item);
	if (it != _Container.end())
	{
		_Container.erase(it);
		return true;
	}
	return false;
}

template<typename T, typename ContainerT, typename Predicate> bool oFindIfAndErase(ContainerT& _Container, const Predicate& _Predicate, T* _pFound = 0)
{
	typename ContainerT::iterator it = oStdFindIf(_Container, _Predicate);
	if (it != _Container.end())
	{
		if (_pFound)
			*_pFound = *it;
		_Container.erase(it);
		return true;
	}
	return false;
}

template<typename ContainerT> char* oToStringT(char* _StrDestination, size_t _SizeofStrDestination, const ContainerT& _Container)
{
	*_StrDestination = 0;
	typename ContainerT::const_iterator itLast = _Container.end() - 1;
	for (typename ContainerT::const_iterator it = _Container.begin(); it != _Container.end(); ++it)
	{
		if (!oToString(_StrDestination, _SizeofStrDestination, *it))
			return nullptr;
		if (it != itLast && !oStrcat(_StrDestination, _SizeofStrDestination, ","))
			return nullptr;
		size_t len = strlen(_StrDestination);
		_StrDestination += len;
		_SizeofStrDestination -= len;
	}
	return _StrDestination;
}

template<typename T, typename ContainerT> bool oFromStringT(ContainerT* _pContainer, const char* _StrSource)
{
	char* ctx = nullptr;
	const char* token = oStrTok(_StrSource, ",", &ctx);
	while (token)
	{
		T obj;
		token += strspn(token, oWHITESPACE);
		if (!oFromString(&obj, token))
		{
			oStrTokClose(&ctx);
			_pContainer->clear(); // Incomplete so clear
			return false;
		}
		if (_pContainer->size() == _pContainer->max_size())
			return false;
		_pContainer->push_back(obj);
		token = oStrTok(nullptr, ",", &ctx);
	}
	return true;
}

} // namespace detail

// _____________________________________________________________________________
// Common operation wrappers

template <oALGO_ARRAY_TEMPLATE> char* oToString(char* _StrDestination, size_t _SizeofStrDestination, const oALGO_ARRAY& _Array) { return detail::oToStringT(_StrDestination, _SizeofStrDestination, _Array); }
template <oALGO_ARRAY_TEMPLATE, size_t size> char* oToString(char (&_StrDestination)[size], const oALGO_ARRAY& _Array) { return detail::oToStringT(_StrDestination, size, _Array); }
template <oALGO_ARRAY_TEMPLATE> bool oFromString(oALGO_ARRAY* _pValue, const char* _StrSource) { return detail::oFromStringT<T>(_pValue, _StrSource); }
template <oALGO_VECTOR_TEMPLATE> char* oToString(char* _StrDestination, size_t _SizeofStrDestination, const oALGO_VECTOR& _Vector) { return detail::oToStringT(_StrDestination, _SizeofStrDestination, _Vector); }
template <oALGO_VECTOR_TEMPLATE, size_t size> char* oToString(char (&_StrDestination)[size], const oALGO_VECTOR& _Vector) { return detail::oToStringT(_StrDestination, size, _Vector); }
template <oALGO_VECTOR_TEMPLATE> bool oFromString(oALGO_VECTOR* _pValue, const char* _StrSource) { return detail::oFromStringT<T>(_pValue, _StrSource); }
inline bool oFromString(std::string* _pString, const char* _StrSource) { if (!oSTRVALID(_StrSource)) return false; *_pString = _StrSource; return true; }


// Copy from wchar_t std::basic_string to char std::basic_string
inline void oStrcpy(std::string& _StrDestination, const std::wstring& _StrSource)
{
	_StrDestination.assign(_StrSource.begin(), _StrSource.end());
}

// Copy from char std::basic_string to wchar_t std::basic_string
inline void oStrcpy(std::wstring& _StrDestination, const std::string& _StrSource)
{
	_StrDestination.assign(_StrSource.begin(), _StrSource.end());
}

// Insert _Item only if its value does not already exist in the container
// Returns the index at which the item was inserted, or where a pre-existing
// one was found.
template<oALGO_ARRAY_TEMPLATE> size_t oPushBackUnique(oALGO_ARRAY& _Array, const T& _Item) { return detail::oPushBackUnique<T, oALGO_ARRAY >(_Array, _Item); }
template<oALGO_VECTOR_TEMPLATE> size_t oPushBackUnique(oALGO_VECTOR& _Vector, const T& _Item) { return detail::oPushBackUnique<T, oALGO_VECTOR >(_Vector, _Item); }

// Sets _Container[_Index] = _Item in a way that first ensures size() is capable
template<oALGO_ARRAY_TEMPLATE> void oSafeSet(oALGO_ARRAY& _Array, size_t _Index, const T& _Item){ detail::oSafeSet<T, oALGO_ARRAY >(_Array, _Index, _Item); }
template<oALGO_VECTOR_TEMPLATE> void oSafeSet(oALGO_VECTOR& _Vector, size_t _Index, const T& _Item) { detail::oSafeSet<T, oALGO_VECTOR >(_Vector, _Index, _Item); }

// Scans list for the first occurrence of a value whose operator bool() returns 
// false and sets _Item to that slot and returns the slot's index. If there are 
// no false values, push_back is called.
template<oALGO_ARRAY_TEMPLATE> size_t oSparseSet(oALGO_ARRAY& _Array, const T& _Item) { return detail::oSparseSet<T, oArray>(_Array, _Item); }
template<oALGO_VECTOR_TEMPLATE> size_t oSparseSet(oALGO_VECTOR& _Vector, const T& _Item) { return detail::oSparseSet<T, oALGO_VECTOR >(_Vector, _Item); }

// Returns true if the item is found, false if not found
template<oALGO_ARRAY_TEMPLATE> bool oContains(const oALGO_ARRAY& _Array, const T& _Item) { return _Array.end() != oStdFind(_Array, _Item); }
template<oALGO_VECTOR_TEMPLATE> bool oContains(const oALGO_VECTOR& _Vector, const T& _Item) { return _Vector.end() != oStdFind(_Vector, _Item); }

// Finds _Item by value and erases it, returning true if an erase took place or false if the item was not found
template<oALGO_ARRAY_TEMPLATE> bool oFindAndErase(oALGO_ARRAY& _Array, const T& _Item) { return detail::oFindAndErase<T, oALGO_ARRAY >(_Array, _Item); }
template<oALGO_VECTOR_TEMPLATE> bool oFindAndErase(oALGO_VECTOR& _Vector, const T& _Item) { return detail::oFindAndErase<T, oALGO_VECTOR >(_Vector, _Item); }
template<oALGO_UNORDERED_MAP_TEMPLATE> bool oFindAndErase(oALGO_UNORDERED_MAP& _UnorderedMap, const KeyT& _Item) { return detail::oFindAndErase(_UnorderedMap, _Item); }
template<oALGO_MAP_TEMPLATE> bool oFindAndErase(oALGO_MAP& _Map, const KeyT& _Item) { return detail::oFindAndErase(_Map, _Item); }

// Same as above, but using oStdFindIf
template<oALGO_ARRAY_TEMPLATE, class Predicate> bool oFindIfAndErase(oALGO_ARRAY& _Array, const Predicate& _Predicate, T* _pFound = nullptr) { return detail::oFindIfAndErase<T, oALGO_ARRAY >(_Array, _Predicate, _pFound); }
template<oALGO_VECTOR_TEMPLATE, class Predicate> bool oFindIfAndErase(oALGO_VECTOR& _Vector, const Predicate& _Predicate, T* _pFound = nullptr) { return detail::oFindIfAndErase<T, oALGO_VECTOR >(_Vector, _Predicate, _pFound); }
template<oALGO_LIST_TEMPLATE, class Predicate> bool oFindIfAndErase(oALGO_LIST& _Vector, const Predicate& _Predicate, T* _pFound = nullptr) { return detail::oFindIfAndErase<T, oALGO_LIST >(_Vector, _Predicate, _pFound); }
template<oALGO_UNORDERED_MAP_TEMPLATE, class Predicate> bool oFindIfAndErase(oALGO_UNORDERED_MAP _UnorderedMap, const Predicate& _Predicate, typename oALGO_UNORDERED_MAP::value_type* _pFound = nullptr) { detail::oFindIfAndErase(_UnorderedMap, Predicate, _pFound); }
template<oALGO_MAP_TEMPLATE, class Predicate> bool oFindAndErase(oALGO_MAP_TEMPLATE& _Map, const Predicate& _Predicate, typename oALGO_MAP::value_type* _pFound = nullptr) { return detail::oFindIfAndErase(_Map, _Predicate, _pFound); }

// Add a new element to the list and return a reference to it.
template<oALGO_ARRAY_TEMPLATE> T* oAppend(oALGO_ARRAY& _Array) { _Array.resize(_Array.size() + 1); return &_Array.back(); }
template<oALGO_VECTOR_TEMPLATE> T* oAppend(oALGO_VECTOR& _Vector) { _Vector.resize(_Vector.size() + 1); return &_Vector.back(); }

// Get the base pointer to the buffer containing the vector
template <oALGO_ARRAY_TEMPLATE> T* oGetData(oALGO_ARRAY& _Array) { return _Array.empty() ? 0 : _Array.data(); }
template <oALGO_ARRAY_TEMPLATE> const T* oGetData(const oALGO_ARRAY& _Array) { return _Array.empty() ? 0 : _Array.data(); }
template <oALGO_VECTOR_TEMPLATE> T* oGetData(oALGO_VECTOR& _Vector) { return _Vector.empty() ? 0 : (T*)&_Vector.front(); }
template <oALGO_VECTOR_TEMPLATE> const T* oGetData(const oALGO_VECTOR& _Vector) { return _Vector.empty() ? 0 : (const T*)&_Vector.front(); }

// Returns the size of the vector/array in bytes (.size() really returns the count)
template <oALGO_ARRAY_TEMPLATE> size_t oGetDataSize(oALGO_ARRAY& _Array) { return _Array.empty() ? 0 : _Array.size() * sizeof(T); }
template <oALGO_VECTOR_TEMPLATE> size_t oGetDataSize(const oALGO_VECTOR& _Vector) { return _Vector.empty() ? 0 : _Vector.size() * sizeof(T); }

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

// Insert into a std::vector at it's sorted spot, supply a compare function of type oFUNCTION<bool(const T& _ContainerItem, const T& _NewItem)>
template<oALGO_VECTOR_TEMPLATE, class CompareT> size_t oSortedInsert(oALGO_VECTOR& _Container, const T& _Item, CompareT _Compare)
{
	oALGO_VECTOR::iterator it = _Container.begin();
	for (; it != _Container.end(); ++it)
		if (_Compare(*it, _Item))
			break;
	it = _Container.insert(it, _Item);
	return std::distance(_Container.begin(), it);
}

// Removes all duplicates from a std::vector, but reorders the vector to do so.
template <oALGO_VECTOR_TEMPLATE> void oSortAndRemoveDuplicates(oALGO_VECTOR& _Vector)
{
	std::sort(_Vector.begin(), _Vector.end());
	_Vector.erase(std::unique(_Vector.begin(), _Vector.end()), _Vector.end());
}

// less typing
template <class Container, class Func>
void oForAll(Container &_container, Func _func)
{
	std::for_each(begin(_container), end(_container), _func);
}

// less typing
template <class Container, class Func>
void oForAllValues(Container &_container, Func _func)
{
	std::for_each(begin(_container), end(_container), [&] (const Container::reference _arg) { _func(_arg.second); });
}

//Quickly remove an element from a vector O(1) if you don't care about the order of the elements in the vector. 
//	Works on any container that support pop_back and back operations
template <class Container, class Iterator>
void oUnorderedRemove(Container &_container, Iterator &_iterator)
{
#ifdef oHAS_MOVE_CTOR
	*_iterator = std::move(_container.back());
#else
	*_iterator = _container.back();
#endif
	_container.pop_back();
}

//Add missing c++11 non member functions, already proposed for c++17, was an oversight
template<class T>
auto cbegin(const T& _arg) -> decltype(_arg.cbegin())
{
	return _arg.cbegin();
}

template<class T>
auto cend(const T& _arg) -> decltype(_arg.cend())
{
	return _arg.cend();
}

template<class T, int SIZE>
const T* cbegin(const T (&_arg)[SIZE])
{
	return &_arg[0];
}

template<class T, int SIZE>
const T* cend(const T (&_arg)[SIZE])
{
	return &_arg[SIZE];
}

template<class T>
auto crbegin(const T& _arg) -> decltype(_arg.crbegin())
{
	return _arg.crbegin();
}

template<class T>
auto crend(const T& _arg) -> decltype(_arg.crend())
{
	return _arg.crend();
}

namespace detail
{
	template<class T>
	class oReversePointer
	{
	public:
		oReversePointer(const T* _iterator) : Iterator(_iterator) {}
		const oReversePointer<T> operator++()
		{
			return oReversePointer<T>(--Iterator);
		}
		const oReversePointer<T> operator++(int)
		{
			return oReversePointer<T>(Iterator--);
		}
		const oReversePointer<T> operator--()
		{
			return oReversePointer<T>(++Iterator);
		}
		const oReversePointer<T> operator--(int)
		{
			return oReversePointer<T>(Iterator++);
		}
		bool operator==(const oReversePointer& _other)
		{
			return Iterator == _other.Iterator;
		}
		bool operator!=(const oReversePointer& _other)
		{
			return Iterator != _other.Iterator;
		}
		const T& operator*()
		{
			return *Iterator;
		}
	private:
		const T* Iterator;
	};
}

template<class T, int SIZE>
detail::oReversePointer<T> crbegin(const T (&_arg)[SIZE])
{
	return detail::oReversePointer<T>(&_arg[SIZE-1]);
}

template<class T, int SIZE>
detail::oReversePointer<T> crend(const T (&_arg)[SIZE])
{
	return detail::oReversePointer<T>(&_arg[-1]);
}

//not sure if non member size is in c++17, but might as well. no need for separate size and oCountOf
template<class T>
size_t size(const T& _arg)
{
	return _arg.size();
}

template<class T, int SIZE>
size_t size(const T (&_arg)[SIZE])
{
	return oCOUNTOF(_arg);
}

// _____________________________________________________________________________
// std regex utilties

inline void oRegexCopy(std::string& _OutString, const std::tr1::cmatch& _Matches, size_t _NthMatch) { _OutString.assign(_Matches[_NthMatch].first, _Matches[_NthMatch].length()); }

// Returns 0 on success, ENOENT if the specified match doesn't exist, and ERANGE 
// if the destination isn't large enough to hold the result.
errno_t oRegexCopy(char *_StrDestination, size_t _SizeofStrDestination, const std::tr1::cmatch& _Matches, size_t _NthMatch);
template<size_t size> inline errno_t oRegexCopy(char (&_StrDestination)[size], const std::tr1::cmatch& _Matches, size_t _NthMatch) { return oRegexCopy(_StrDestination, size, _Matches, _NthMatch); }

// Convert a regex error into a human-readable string
const char* oRegexGetError(std::tr1::regex_constants::error_type _RegexError);

// Handles the exception thrown by a bad compile and fills the specified string 
// with the error generated when this function returns false. If this returns 
// true, _OutRegex contains a valid, compiled regex.
bool oTryCompileRegex(std::tr1::regex& _OutRegex, char* _StrError, size_t _SizeofStrError, const char* _StrRegex, std::tr1::regex_constants::syntax_option_type _Flags = std::tr1::regex_constants::ECMAScript);
template<size_t size> inline bool oTryCompilelRegex(std::tr1::regex& _OutRegex, char (&_StrError)[size], const char* _StrRegex, std::tr1::regex_constants::syntax_option_type _Flags = std::tr1::regex_constants::ECMAScript) { return oTryCompilelRegex(_OutRegex, _StrError, size, _StrRegex, _Flags); }

// _____________________________________________________________________________
// std string utilities

template<oALGO_STRING_TEMPLATE> void oTrimLeft(oALGO_STRING& _String, const char* _TrimChars = " \t\r\n")
{
	if (_String.empty()) return;
	size_t pos = _String.find_first_not_of(_TrimChars);
	if (pos != _String.npos) _String.erase(0, pos);
	else _String.clear();
}

template<oALGO_STRING_TEMPLATE> void oTrimRight(oALGO_STRING& _String, const char* _TrimChars = " \t\r\n")
{
	if (_String.empty()) return;
	size_t pos = _String.find_last_not_of(_TrimChars);
	if (pos != _String.npos) _String.erase(pos + 1);
	else _String.clear();
}

template<oALGO_STRING_TEMPLATE> void oTrim(oALGO_STRING& _String, const char* _TrimChars = " \t\r\n")
{
	oTrimRight(_String, _TrimChars);
	oTrimLeft(_String, _TrimChars);
}

// Replace all occurrences of _Find in _String with _Replace. This returns the 
// number of replacements made.
template<oALGO_STRING_TEMPLATE> int oReplaceAll(oALGO_STRING& _String, const oALGO_STRING& _Find, const oALGO_STRING& _Replace)
{
	int nReplacements = 0;
	oALGO_STRING::size_type FindLength = _Find.size();
	oALGO_STRING::size_type ReplaceLength = _Replace.size();
	oALGO_STRING::size_type pos = _String.find(_Find, 0);
	for (; pos != std::string::npos; pos += ReplaceLength)
	{
		_String.replace(pos, FindLength, _Replace);
		nReplacements++;
		pos = _String.find(_Find, 0);
	}
	return nReplacements;
}

// Replace all occurrences of _Find in _String with _Replace. This returns the 
// number of replacements made.
template<oALGO_STRING_TEMPLATE> int oReplaceAll(oALGO_STRING& _String, const char* _Find, const char* _Replace)
{
	int nReplacements = 0;
	oALGO_STRING::size_type FindLength = strlen(_Find);
	oALGO_STRING::size_type ReplaceLength = strlen(_Replace);
	oALGO_STRING::size_type pos = _String.find(_Find, 0);
	for (; pos != std::string::npos; pos += ReplaceLength)
	{
		_String.replace(pos, FindLength, _Replace);
		nReplacements++;
		pos = _String.find(_Find, 0);
	}
	return nReplacements;
}

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
	bool operator()(const char* str) { return !oStrcmp(str, _t); }
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
	bool operator()(const char* str) { return !oStricmp(str, _t); }
private:
	oStdEqualsI() {}
	const char* _t;
};

template<> struct oStdEqualsI<const std::string>
{
	oStdEqualsI(const char* str) : _t(str) {}
	oStdEqualsI(const std::string& str) : _t(str.c_str()) {}
	bool operator()(const std::string& str) { return !oStricmp(str.c_str(), _t); }
private:
	oStdEqualsI() {}
	const char* _t;
};

template<typename T> struct oStdLess : public std::binary_function<T, T, bool> { bool operator()(const T& x, const T& y) const { return x < y; } };
template<> struct oStdLess<const char*> { int operator()(const char* x, const char* y) const { return oStrcmp(x, y) < 0; } };
template<typename T> struct oStdLessI : public std::binary_function<T, T, bool> { bool operator()(const T& x, const T& y) const { return x < y; } };
template<> struct oStdLessI<const char*> { bool operator()(const char* x, const char* y) const { return oStricmp(x, y) < 0; } };
template<typename CHAR_T, size_t CAPACITY> struct oStdLess<oFixedString<CHAR_T, CAPACITY>> { bool operator()(const oFixedString<CHAR_T, CAPACITY>& x, const oFixedString<CHAR_T, CAPACITY>& y) const { return oStrcmp(x.c_str(), y.c_str()) < 0; } };
template<typename CHAR_T, size_t CAPACITY> struct oStdLessI<oFixedString<CHAR_T, CAPACITY>> { bool operator()(const oFixedString<CHAR_T, CAPACITY>& x, const oFixedString<CHAR_T, CAPACITY>& y) const { return oStricmp(x.c_str(), y.c_str()) < 0; } };

template<typename T> struct oStdEqualTo : public std::binary_function<T, T, bool> { bool operator()(const T& x, const T& y) const { return x == y; } };
template<> struct oStdEqualTo<const char*> { int operator()(const char* x, const char* y) const { return !oStrcmp(x, y); } };
template<typename T> struct oStdEqualToI : public std::binary_function<T, T, bool> { bool operator()(const T& x, const T& y) const { return x == y; } };
template<> struct oStdEqualToI<const char*> { bool operator()(const char* x, const char* y) const { return !oStricmp(x, y); } };
template<typename CHAR_T, size_t CAPACITY> struct oStdEqualTo<oFixedString<CHAR_T, CAPACITY>> { bool operator()(const oFixedString<CHAR_T, CAPACITY>& x, const oFixedString<CHAR_T, CAPACITY>& y) const { return !oStrcmp(x.c_str(), y.c_str()); } };
template<typename CHAR_T, size_t CAPACITY> struct oStdEqualToI<oFixedString<CHAR_T, CAPACITY>> { bool operator()(const oFixedString<CHAR_T, CAPACITY>& x, const oFixedString<CHAR_T, CAPACITY>& y) const { return !oStricmp(x.c_str(), y.c_str()); } };

// std::hash function that returns exactly the same value as passed in. This is 
// useful where some other object caches hash values that then can be used 
// directly in an unordered_set.
template<typename T> struct oNoopHash : public std::unary_function<T, size_t> { size_t operator()(const T& v) const { return static_cast<size_t>(v); } };

#endif
