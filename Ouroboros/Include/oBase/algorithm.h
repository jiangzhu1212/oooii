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
// Wrappers for common operations on STL containers that are not in <algorithm>
#pragma once
#ifndef oBase_algorithm_h
#define oBase_algorithm_h

#include <algorithm>
#include <cstdarg>
#include <functional>
#include <map>
#include <regex>
#include <string>
#include <unordered_map>
#include <vector>
#include <oBase/fixed_vector.h>
#include <oBase/throw.h>
#include <oBase/types.h>
#include <oBase/unordered_map.h>

namespace ouro {

// _____________________________________________________________________________
// Common operation wrappers

// Convenience wrappers for std::find for the common case of searching the 
// entire container as well as supporting containers std::find 
// doesn't, such as those that use this->find() instead (std::map).
template<typename T, typename ContainerT> typename ContainerT::iterator find(ContainerT& _Container, const T& _Item) { return std::find(std::begin(_Container), std::end(_Container), _Item); }
template<typename T, typename ContainerT> typename ContainerT::const_iterator find(const ContainerT& _Container, const T& _Item) { return std::find(std::cbegin(_Container), std::cend(_Container), _Item); }
template<typename ContainerT, typename Predicate> typename ContainerT::iterator find_if(ContainerT& _Container, const Predicate& _Predicate) { return std::find_if(std::begin(_Container), std::end(_Container), _Predicate); }
template<typename ContainerT, typename Predicate> typename ContainerT::const_iterator find_if(const ContainerT& _Container, const Predicate& _Predicate) { return std::find_if(std::begin(_Container), std::end(_Container), _Predicate); }

// containers that store std::pairs need special-casing
template<typename KeyT, typename ValT, typename HasherT, typename KeyEqualT, typename AllocatorT>
typename std::unordered_map<KeyT, ValT, HasherT, KeyEqualT, AllocatorT>::iterator
	find(std::unordered_map<KeyT, ValT, HasherT, KeyEqualT, AllocatorT>& _Container, const KeyT& _Item)
		{ return _Container.empty() ? std::end(_Container) : _Container.find(_Item); }

template<typename KeyT, typename ValT, typename HasherT, typename KeyEqualT, typename AllocatorT>
typename std::unordered_map<KeyT, ValT, HasherT, KeyEqualT, AllocatorT>::const_iterator
	find(const std::unordered_map<KeyT, ValT, HasherT, KeyEqualT, AllocatorT>& _Container, const KeyT& _Item)
		{ return _Container.empty() ? std::end(_Container) : _Container.find(_Item); }

template<typename KeyT, typename ValT, typename KeyLessT, typename AllocatorT>
typename std::map<KeyT, ValT, KeyLessT, AllocatorT>::iterator
	find(std::map<KeyT, ValT, KeyLessT, AllocatorT>& _Container, const KeyT& _Item)
		{ return _Container.empty() ? std::end(_Container) : _Container.find(_Item); }

template<typename KeyT, typename ValT, typename KeyLessT, typename AllocatorT>
typename std::map<KeyT, ValT, KeyLessT, AllocatorT>::const_iterator
	find(const std::map<KeyT, ValT, KeyLessT, AllocatorT>& _Container, const KeyT& _Item)
		{ return _Container.empty() ? std::end(_Container) : _Container.find(_Item); }

template<typename KeyT, typename MappedT, typename HashT, typename KeyEqualT, typename KeyLessT, typename AllocatorT>
typename ouro::unordered_map<KeyT, MappedT, HashT, KeyEqualT, KeyLessT, AllocatorT>::iterator
	find(ouro::unordered_map<KeyT, MappedT, HashT, KeyEqualT, KeyLessT, AllocatorT>& _Container, const KeyT& _Key)
		{ return _Container.empty() ? std::end(_Container) : _Container.find(_Key); }

template<typename KeyT, typename MappedT, typename HashT, typename KeyEqualT, typename KeyLessT, typename AllocatorT>
typename ouro::unordered_map<KeyT, MappedT, HashT, KeyEqualT, KeyLessT, AllocatorT>::const_iterator
	find(const ouro::unordered_map<KeyT, MappedT, HashT, KeyEqualT, KeyLessT, AllocatorT>& _Container, const KeyT& _Key)
		{ return _Container.empty() ? std::end(_Container) : _Container.find(_Key); }

// Convenience wrapper for std::fill for the common case of filling the entire
// container.
template<typename T, typename ContainerT> void fill(ContainerT& _Container, const T& _Item) { std::fill(std::begin(_Container), std::end(_Container), _Item); }

// Returns the base pointer to the buffer encapsulated by the specified type
template<typename T> typename T::value_type* data(T& x) { return x.data(); }
template<typename T> const typename T::value_type* data(const T& x) { return x.data(); }

// Returns the number of elements in the specified container
template<typename T> size_t size(const T& x) { return x.size(); }
template<class T, size_t N> size_t size(const T (&x)[N]) { return sizeT; }

// Insert _Item only if its value does not already exist in the container
// Returns the index at which the item was inserted, or where a pre-existing
// one was found.

template<typename T, typename ContainerT>
size_t push_back_unique(ContainerT& _Container, const T& _Value)
{
	size_t index = _Container.size();
	typename ContainerT::iterator it = ouro::find(_Container, _Value);
	if (it == std::end(_Container))
		_Container.push_back(_Value);
	else
		index = std::distance(std::begin(_Container), it);
	return index;
}

template<typename T, typename ContainerT, typename Predicate>
size_t push_back_unique_if(ContainerT& _Container, const T& _Item, Predicate&& _Predicate)
{
	size_t index = _Container.size();
	auto pred = [&](const T& _testItem) -> bool{
		if(_Predicate(_Item, _testItem))
			return true;
		else
			return false;
	};

	typename ContainerT::iterator it = ouro::find_if(_Container, pred);
	if (it == std::end(_Container))
		_Container.push_back(_Item);
	else
		index = std::distance(std::begin(_Container), it);
	return index;
};

// Returns false if there was a preexisting item and does not set the new item.
// Returns true if there was no preexisting item and the specified item was set.
template<typename KeyT, typename ValT, typename KeyLessT, typename AllocatorT>
bool unique_set(std::map<KeyT, ValT, KeyLessT, AllocatorT>& _Container, const KeyT& _Key, const ValT& _Item)
{
	if (std::end(_Container) != ouro::find(_Container, _Key))
		return false;
	_Container[_Key] = _Item;
	return true;
}

// Sets _Container[_Index] = _Item in a way that first ensures size() is capable
template<typename T, typename ContainerT>
void safe_set(ContainerT& _Container, size_t _Index, const T& _Item)
{
	if (_Container.size() <= _Index)
		_Container.resize(_Index + 1);
	_Container[_Index] = _Item;
}

template<typename T, typename ContainerT>
void safe_set(ContainerT& _Container, size_t _Index, T&& _Item)
{
	if (_Container.size() <= _Index)
		_Container.resize(_Index + 1);
	_Container[_Index] = std::move(_Item);
}

// Sets _Container[_Index] = _Item only if _Index is within the current range
// of the container. If not, this returns false.
template<typename T, typename IndexT, typename ContainerT>
bool ranged_set(ContainerT& _Container, IndexT _Index, const T& _Item)
{
	if (_Index >= 0 && _Index < SafeInt<IndexT>(_Container.size()))
	{
		_Container[_Index] = _Item;
		return true;
	}
	return false;
}

template<typename T, typename IndexT, typename ContainerT>
bool ranged_set(ContainerT& _Container, IndexT _Index, T&& _Item)
{
	if (_Index >= 0 && _Index < as_type<IndexT>(_Container.size()))
	{
		_Container[_Index] = std::move(_Item);
		return true;
	}
	return false;
}

// Scans list for the first occurrence of a value whose operator bool() returns 
// false and sets _Item to that slot and returns the slot's index. If there are 
// no false values, push_back is called.
template<typename T, typename ContainerT>
size_t sparse_set(ContainerT& _Container, const T& _Item)
{
	for (typename ContainerT::iterator it = std::begin(_Container); it != std::end(_Container); ++it)
	{
		if (!*it)
		{
			*it = _Item;
			return std::distance(std::begin(_Container), it);
			break;
		}
	}

	_Container.push_back(_Item);
	return _Container.size()-1;
}

// Uses strtok_r to tokenize the specified string into a vector of tokens
inline void tokenize(std::vector<std::string>& _Container, const char* _StrTok, const char* _StrDelim)
{
	_Container.clear();
	if (_StrTok)
	{
		std::string copy(_StrTok);
		char* ctx = nullptr;
		const char* tok = strtok_r((char*)copy.c_str(), _StrDelim, &ctx);
		while (tok)
		{
			_Container.push_back(tok);
			tok = strtok_r(nullptr, _StrDelim, &ctx);
		}
	}
}

// Returns true if the item is found, false if not found
template<typename T, typename ContainerT>
bool find_and_erase(ContainerT& _Container, const T& _Item)
{
	typename ContainerT::iterator it = ouro::find(_Container, _Item);
	if (it != std::end(_Container))
	{
		_Container.erase(it);
		return true;
	}
	return false;
}

// Returns true if the key is found and erased, false if not found
template<typename KeyT, typename ValT, typename HasherT, typename KeyEqualT, typename AllocatorT>
bool find_and_erase(std::unordered_map<KeyT, ValT, HasherT, KeyEqualT, AllocatorT>& _Container, const KeyT& _Key)
{
	typename std::unordered_map<KeyT, ValT, HasherT, KeyEqualT, AllocatorT>::iterator it = ouro::find(_Container, _Key);
	if (it != std::end(_Container))
	{
		_Container.erase(it);
		return true;
	}
	return false;
}

template<typename KeyT, typename ValT, typename KeyLessT, typename AllocatorT>
bool find_and_erase(std::map<KeyT, ValT, KeyLessT, AllocatorT>& _Container, const KeyT& _Key)
{
	typename std::map<KeyT, ValT, KeyLessT, AllocatorT>::iterator it = ouro::find(_Container, _Key);
	if (it != std::end(_Container))
	{
		_Container.erase(it);
		return true;
	}
	return false;
}

// Returns true if the key is found and erased, false if not found
template<typename KeyT, typename MappedT, typename HashT, typename KeyEqualT, typename KeyLessT, typename AllocatorT>
bool find_and_erase(ouro::unordered_map<KeyT, MappedT, HashT, KeyEqualT, KeyLessT, AllocatorT>& _Container, const KeyT& _Key)
{
	typename ouro::unordered_map<KeyT, MappedT, HashT, KeyEqualT, KeyLessT, AllocatorT>::iterator it = ouro::find(_Container, _Key);
	if (it != std::end(_Container))
	{
		_Container.erase(it);
		return true;
	}
	return false;
}

// Same as above, but using ouro::find_if
template<typename ContainerT, typename Predicate>
bool find_if_and_erase(ContainerT& _Container, const Predicate& _Predicate, typename ContainerT::pointer _pFound = 0)
{
	typename ContainerT::iterator it = ouro::find_if(_Container, _Predicate);
	if (it != std::end(_Container))
	{
		if (_pFound)
			*_pFound = *it;
		_Container.erase(it);
		return true;
	}
	return false;
}

// Returns true if the item is found, false if not found
template<typename T, typename ContainerT> bool contains(const ContainerT& _Array, const T& _Item) { return _Array.cend() != find(_Array, _Item); }

// Add a new element to the list and return a reference to it.
template<typename ContainerT> typename ContainerT::iterator append(ContainerT& _Array) { _Array.resize(_Array.size() + 1); return std::end(_Array) - 1; }

// Insert into a std::vector at its sorted spot, supply a compare function of 
// type std::function<bool(const T& _ContainerItem, const T& _NewItem)>
template<typename T, typename AllocatorT, class CompareT> size_t sorted_insert(std::vector<T, AllocatorT>& _Container, const T& _Item, const CompareT& _Compare)
{
	std::vector<T, AllocatorT>::iterator it = std::begin(_Container);
	for (; it != std::end(_Container); ++it)
		if (_Compare(*it, _Item))
			break;
	it = _Container.insert(it, _Item);
	return std::distance(std::begin(_Container), it);
}

// Removes all duplicates from a std::vector, but reorders the vector to do so.
template <typename T, typename AllocatorT> void sort_and_remove_duplicates(std::vector<T, AllocatorT>& _Vector)
{
	std::sort(std::begin(_Vector), std::end(_Vector));
	_Vector.erase(std::unique(std::begin(_Vector), std::end(_Vector)), std::end(_Vector));
}

// Frees the memory associated with a standard container and otherwise resets
// all size/capacity to 0.
template<typename StdContainerT> void free_memory(StdContainerT& _StdContainer)
{
	_StdContainer.clear();
	_StdContainer.shrink_to_fit();
}

// Simplify for_each
template <typename Container, typename Callable>
void for_each(Container& _Container, const Callable& _Callable)
{
	std::for_each(std::begin(_Container), std::end(_Container), _Callable);
}

// Simplify for_each when visiting std::pairs
template <typename Container, typename Callable>
void for_each_value(Container& _Container, const Callable& _Callable) { std::for_each(std::begin(_Container), std::end(_Container), [&](const Container::reference _Arg) { _Callable(_Arg.second); }); }

// Quickly remove an element from a vector O(1) if you don't care about the 
// order of the elements in the vector. Works on any container that supports 
// pop_back and back operations.
template <class Container, class Iterator>
void unordered_erase(Container& _Container, Iterator& _Iterator)
{
	#ifdef oHAS_MOVE_CTOR
		*_Iterator = std::move(_Container.back());
	#else
		*_Iterator = _Container.back();
	#endif
	_Container.pop_back();
}

// Simplify the erase remove idiom
template <typename Container>
void erase_remove(Container& _Container, typename Container::value_type _Value)
{
	_Container.erase(std::remove(std::begin(_Container), std::end(_Container), _Value), std::end(_Container));
}

template <typename Container, typename Predicate>
void erase_remove_if(Container& _Container, Predicate _Pred)
{
	_Container.erase(std::remove_if(std::begin(_Container), std::end(_Container), _Pred), std::end(_Container));
}

// _____________________________________________________________________________
// std regex utilities

inline void copy_match_result(std::string& _OutString, const std::cmatch& _Matches, size_t _NthMatch)
{
	_OutString.assign(_Matches[_NthMatch].first, _Matches[_NthMatch].length());
}

inline void copy_match_result(char *_StrDestination, size_t _SizeofStrDestination, const std::cmatch& _Matches, size_t _NthMatch)
{
	#ifdef _MSC_VER
		auto it = stdext::checked_array_iterator<char*>(_StrDestination, _SizeofStrDestination);
	#else
		if (static_cast<size_t>(_Matches[_NthMatch].length()) >= _SizeofStrDestination)
			throw std::system_error(std::make_error_code(std::errc::no_buffer_space));
		auto it = _StrDestination;
	#endif

	std::copy(_Matches[_NthMatch].first, _Matches[_NthMatch].first + _Matches[_NthMatch].length(), it);
}

template<size_t N> void copy_match_result(char (&_StrDestination)[N], const std::cmatch& _Matches, size_t _NthMatch) { copy_match_result(_StrDestination, N, _Matches, _NthMatch); }

// _____________________________________________________________________________
// std string utilities

template<typename T, typename TraitsT, typename AllocatorT>
void trim_left(std::basic_string<T, TraitsT, AllocatorT>& _String, const char* _TrimChars = " \t\r\n")
{
	if (_String.empty()) return;
	size_t pos = _String.find_first_not_of(_TrimChars);
	if (pos != _String.npos) _String.erase(0, pos);
	else _String.clear();
}

template<typename T, typename TraitsT, typename AllocatorT>
void trim_right(std::basic_string<T, TraitsT, AllocatorT>& _String, const char* _TrimChars = " \t\r\n")
{
	if (_String.empty()) return;
	size_t pos = _String.find_last_not_of(_TrimChars);
	if (pos != _String.npos) _String.erase(pos + 1);
	else _String.clear();
}

template<typename T, typename TraitsT, typename AllocatorT>
void trim(std::basic_string<T, TraitsT, AllocatorT>& _String, const char* _TrimChars = " \t\r\n")
{
	trim_right(_String, _TrimChars);
	trim_left(_String, _TrimChars);
}

// Replace all occurrences of _Find in _String with _Replace. This returns the 
// number of replacements made.
template<typename T, typename TraitsT, typename AllocatorT>
int replace_all(std::basic_string<T, TraitsT, AllocatorT>& _String, const std::basic_string<T, TraitsT, AllocatorT>& _Find, const std::basic_string<T, TraitsT, AllocatorT>& _Replace)
{
	int nReplacements = 0;
	std::basic_string<T, TraitsT, AllocatorT>::size_type FindLength = _Find.size();
	std::basic_string<T, TraitsT, AllocatorT>::size_type ReplaceLength = _Replace.size();
	std::basic_string<T, TraitsT, AllocatorT>::size_type pos = _String.find(_Find, 0);
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
template<typename T, typename TraitsT, typename AllocatorT>
int replace_all(std::basic_string<T, TraitsT, AllocatorT>& _String, const char* _Find, const char* _Replace)
{
	int nReplacements = 0;
	std::basic_string<T, TraitsT, AllocatorT>::size_type FindLength = strlen(_Find);
	std::basic_string<T, TraitsT, AllocatorT>::size_type ReplaceLength = strlen(_Replace);
	std::basic_string<T, TraitsT, AllocatorT>::size_type pos = _String.find(_Find, 0);
	for (; pos != std::string::npos; pos += ReplaceLength)
	{
		_String.replace(pos, FindLength, _Replace);
		nReplacements++;
		pos = _String.find(_Find, 0);
	}
	return nReplacements;
}

// avoid securecrt warning when using standard API
template <typename InputIterator, typename OutputIterator, typename UnaryOperation>
OutputIterator transform(InputIterator _First, InputIterator _Last, OutputIterator _Result, UnaryOperation _Op)
{
	for (; _First != _Last; ++_First, ++_Result)
		*_Result = _Op(*_First);
	return _Result;
}

} // namespace ouro

#endif
