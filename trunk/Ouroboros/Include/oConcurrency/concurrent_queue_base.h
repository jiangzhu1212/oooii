/**************************************************************************
 * The MIT License                                                        *
 * Copyright (c) 2013 OOOii.                                              *
 * antony.arciuolo@oooii.com                                              *
 * kevin.myers@oooii.com                                                  *
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
// All queues have some commonly-implemented APIs, so factor that code out to a 
// base class.
#pragma once
#ifndef oConcurrency_concurrent_queue_base_h
#define oConcurrency_concurrent_queue_base_h

#include <oConcurrency/thread_safe.h>
#include <exception>

#define oDEFINE_CONCURRENT_QUEUE_TYPE(_ValueT, _SizeT) \
	typedef _SizeT size_type; \
	typedef _ValueT value_type; \
	typedef value_type& reference; \
	typedef const value_type& const_reference; \
	typedef value_type* pointer; \
	typedef const value_type* const_pointer

namespace oConcurrency {

template<typename T, typename Derived>
class concurrent_queue_base
{
public:
	typedef T value_type;
	typedef value_type& reference;

	// Spins until an element can be popped from the queue
	void pop(reference _Element) threadsafe
	{
		while (!static_cast<threadsafe Derived*>(this)->try_pop(_Element));
	}

	// Spins until the queue is empty
	void clear() threadsafe
	{
		value_type e;
		while (static_cast<threadsafe Derived*>(this)->try_pop(e));
	}
};

const std::error_category& container_category();
/*enum class*/ namespace container_errc { enum value { not_empty }; };
/*constexpr*/ inline std::error_code make_error_code(container_errc::value _ErrorCode) { return std::error_code(static_cast<int>(_ErrorCode), container_category()); }
/*constexpr*/ inline std::error_condition make_error_condition(container_errc::value _ErrorCode) { return std::error_condition(static_cast<int>(_ErrorCode), container_category()); }
class container_error : public std::logic_error { public: container_error(container_errc::value _ErrorCode) : logic_error(container_category().message(_ErrorCode)) {} };

} // namespace oConcurrency

#endif
