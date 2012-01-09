// $(header)
// Basically this is a super-set of std::map and std::unordered_map
// that uses std::map in debug builds and std::unordered_map in
// release builds. Why?

// unordered_map for MSVC is pathologically slow in its destructor.
// Because of the several 3rd-party libs we use, the idea of 
// changing the iterator debug level was decided against. Read more
// about the issue here:

// "C++11 Hash Containers and Debug Mode"
// November 29, 2011
// http://drdobbs.com/blogs/cpp/232200410?pgno=2
// Also note Stephan T. Lavavej's response to the blog.

// NOTE: At the time of original authoring, the intent was to fix a 
// serious performance issue, and not a lot of attention was paid to
// unifying the APIs where they may differ.

#ifndef oUnorderedMap_h
#define oUnorderedMap_h

#include <map>
#include <unordered_map>

template
	< typename _KeyType
	, typename _MappedType
	, typename _HashType = std::hash<_KeyType>
	, typename _KeyEqualType = std::equal_to<_KeyType>
	, typename _KeyLessType = std::less<_KeyType>
	, typename _AllocatorType = std::allocator<std::pair<const _KeyType, _MappedType>>
	>
class oUnorderedMap : public
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
	typedef std::unordered_map<_KeyType, _MappedType, _HashType, _KeyEqualType, _AllocatorType> map_type;
#endif

	typedef _KeyEqualType key_equal;
	typedef _KeyLessType key_less;
	typedef _HashType hasher;
	typedef _KeyEqualType key_equal;

	oUnorderedMap() {}
	oUnorderedMap(size_type bucket_count
		, const _HashType& hash = _HashType()
		, const _KeyEqualType& equal = _KeyEqualType()
		, const _KeyLessType& less = _KeyLessType()
		, const _AllocatorType& alloc = _AllocatorType()) :
		#ifdef _DEBUG
			std::map<_KeyType, _MappedType, _KeyLessType, _AllocatorType>(less, alloc)
		#else
			std::unordered_map<_KeyType, _MappedType, _HashType, _KeyEqualType, _AllocatorType>(bucket_count, hash, equal, alloc)
		#endif
	{}
};

#endif
