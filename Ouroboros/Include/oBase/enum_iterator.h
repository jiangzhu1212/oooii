// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.

// support for iterating enum classes with range based for under the 
// assumption the enum starts at 0 and has a member called count

#pragma once
#include <type_traits>

namespace ouro {

template<typename E>
class enum_iterator
{
public:
   class iterator
   {
		public:
			typedef typename std::underlying_type<E>::type type_t;
			iterator(type_t _e) : e(_e) { static_assert(std::is_enum<E>::value, "invalid enum type"); }
			E operator*( void ) const { return (E)e; }
			void operator++(void) { e++; }
			bool operator!=(iterator that) { return e != that.e; }
	 private:
			type_t e;
   };
};

}

namespace std {

template<typename E> typename ouro::enum_iterator<E>::iterator begin(ouro::enum_iterator<E>) { return typename ouro::enum_iterator<E>::iterator((int)0); }
template<typename E> typename ouro::enum_iterator<E>::iterator end(ouro::enum_iterator<E>) { return typename ouro::enum_iterator<E>::iterator((int)E::count); }
template<typename E> struct less_enum { bool operator()(const E& a, const E& b) const { return (std::underlying_type<E>::type)a < (std::underlying_type<E>::type)b; } };
template<typename E> struct greater_enum { bool operator()(const E& a, const E& b) const { return (std::underlying_type<E>::type)a > (std::underlying_type<E>::type)b; } };

}

namespace ouro {

template<typename E> bool in_range_incl(const E& a, const E& start, const E& end) { return !std::less_enum<E>()(a,start) && !std::greater_enum<E>()(a,end); }

}

#define oCHECK_COUNTS_MATCH(_enum, _array) static_assert((size_t)_enum::count == oCOUNTOF(_array), "array mismatch");
