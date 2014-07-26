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
// support for iterating enum classes with range based for under the 
// assumption the enum starts at 0 and has a member called count
#pragma once
#ifndef oBase_enum_iterator_h
#define oBase_enum_iterator_h

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

}

#endif
