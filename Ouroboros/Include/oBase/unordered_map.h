/**************************************************************************
 * The MIT License                                                        *
 * Copyright (c) 2014 Antony Arciuolo.                                    *
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
// Basically this is a super-set of std::map and std::unordered_map that uses 
// std::map in debug builds and std::unordered_map in release builds. Why?

// unordered_map for MSVC is pathologically slow in its destructor. Because of 
// potential use of 3rd-party pre-compiled libs the idea of changing iterator 
// debug level will cause build headaches. Read more about the issue here:

// "C++11 Hash Containers and Debug Mode"
// November 29, 2011
// http://drdobbs.com/blogs/cpp/232200410?pgno=2
// Also note Stephan T. Lavavej's response to the blog.

#ifndef oBase_unordered_map_h
#define oBase_unordered_map_h

#include <map>
#include <unordered_map>

namespace ouro {

// returns the value as the hash (useful for when hashes are pre-computed)
template<typename T> struct std_noop_hash : public std::unary_function<T, size_t> { size_t operator()(const T& v) const { return static_cast<size_t>(v); } };

template
	< typename _KeyType
	, typename _MappedType
	, typename _HashType = std::hash<_KeyType>
	, typename _KeyEqualType = std::equal_to<_KeyType>
	, typename _KeyLessType = std::less<_KeyType>
	, typename _AllocatorType = std::allocator<std::pair<const _KeyType, _MappedType>>
	>
class unordered_map : public
#ifdef _DEBUG
	std::map<_KeyType, _MappedType, _KeyLessType, _AllocatorType>
#else
	std::unordered_map<_KeyType, _MappedType, _HashType, _KeyEqualType, _AllocatorType>
#endif
{
public:
#ifdef _DEBUG
	typedef std::map<_KeyType, _MappedType, _KeyLessType, _AllocatorType> map_type;
#else
	typedef std::unordered_map<_KeyType, _MappedType, _HashType, _KeyEqualType
		, _AllocatorType> map_type;
#endif

	typedef _KeyEqualType key_equal;
	typedef _KeyLessType key_less;
	typedef _HashType hasher;
	typedef _KeyEqualType key_equal;

	unordered_map() {}
	unordered_map(size_type bucket_count
		, const _HashType& hash = _HashType()
		, const _KeyEqualType& equal = _KeyEqualType()
		, const _KeyLessType& less = _KeyLessType()
		, const _AllocatorType& alloc = _AllocatorType()) :
		#ifdef _DEBUG
			std::map<_KeyType, _MappedType, _KeyLessType, _AllocatorType>(less, alloc)
		#else
			std::unordered_map<_KeyType, _MappedType, _HashType, _KeyEqualType
				, _AllocatorType>(bucket_count, hash, equal, alloc)
		#endif
	{}
};

}

#endif
