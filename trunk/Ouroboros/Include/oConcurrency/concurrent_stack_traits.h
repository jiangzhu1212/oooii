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
// Traits for concurrent_stack. Of note is that there's not enough bits in 
// a 32-bit implementation, so this always uses 64-bit atomics which may not
// be supported by simpler 32-bit architectures.
#ifndef oConcurrency_concurrent_stack_traits_h
#define oConcurrency_concurrent_stack_traits_h

#include <oBase/config.h>
#include <oConcurrency/oConcurrency.h> // for is_fifo

// @tony: there's a warning about not handling registers correctly in the 
// 32-bit std::atomic implementation for 64-bit swaps in VS2012's xatomic.h
// that results in registers being corrupt so fall back on basic atomics 
// until the lib gets fixed.
#ifdef oHAS_DOUBLE_WIDE_ATOMIC_BUG
	#include <oHLSL/oHLSLAtomics.h>
#else
	#include <atomic>
#endif

namespace oConcurrency {

struct concurrent_stack_traits64
{
	typedef unsigned long long type;

	#ifdef oHAS_DOUBLE_WIDE_ATOMIC_BUG
		typedef type atomic_type;
	#else
		typedef std::atomic<type> atomic_type;
	#endif

	static const size_t tag_bits = 4;
	static const size_t size_bits = 12;
	static const size_t pointer_bits = 48;

	union head_type
	{
		type all;
		struct
		{
			type tag : tag_bits;
			type size : size_bits;
			type head : pointer_bits;
		};
	};

	// returns true if swap succeeded. In either case _Expected is 
	// updated with the loaded value of _Head.
	inline static bool cas(atomic_type& _Head, type& _Expected, type _Desired)
	{
		#ifdef oHAS_DOUBLE_WIDE_ATOMIC_BUG
			unsigned long long Orig = 0;
			InterlockedCompareExchange(_Head, _Expected, _Desired, Orig);
			bool Exchanged = Orig == _Expected;
			_Expected = Orig;
			return Exchanged;
		#else
			return _Head.compare_exchange_strong(_Expected, _Desired);
		#endif
	}
};

} // namespace oConcurrency

#endif
