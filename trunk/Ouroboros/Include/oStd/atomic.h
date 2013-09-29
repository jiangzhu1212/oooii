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
#pragma once
#ifndef oStd_atomic_h
#define oStd_atomic_h

#include <oStd/intrinsics.h>
#include <cstddef> // ptrdiff_t
#include <type_traits>

// @oooii-tony: I don't fully understand all the strong v. weak and memory_order 
// semantics. I'm sure it has to do with Acq and Rel versions of _Interlocked,
// but I don't want to look up the mapping right now, so just do something basic.

namespace oStd {

	// @oooii-tony: These are not the standard. I haven't yet understood easily 
	// how these map. Why can't people just use the words read and read-write so
	// that anyone who has every opened a file understands the behavior??

	__forceinline void atomic_thread_fence_read() { _ReadBarrier(); }
	__forceinline void atomic_thread_fence_read_write() { _ReadWriteBarrier(); }

	template<typename T, typename U> T atomic_exchange(volatile T* _X, U _Y) { return atomic_exchange(_X, (T)_Y); }
	template<typename T> T atomic_exchange(volatile T* _X, T _Y) { std::make_signed<T>::type t = atomic_exchange((std::make_signed<T>::type*)_X, *(std::make_signed<T>::type*)&_Y); return *(T*)&t; }
	template<typename T> T* atomic_exchange(T* volatile* _X, T* _Y) { return (T*)_InterlockedExchangePointer((void* volatile*)_X, (void*)_Y); }
	template<> inline bool atomic_exchange<bool>(volatile bool* _X, bool _Y) { char c = _InterlockedExchange8((volatile char*)_X, *(char*)&_Y); return *(bool*)&c; }
	template<> inline char atomic_exchange<char>(volatile char* _X, char _Y) { return _InterlockedExchange8(_X, _Y); }
	template<> inline short atomic_exchange<short>(volatile short* _X, short _Y) { return _InterlockedExchange16(_X, _Y); }
	template<> inline int atomic_exchange<int>(volatile int* _X, int _Y) { return _InterlockedExchange((volatile long*)_X, *(long*)&_Y); }
	template<> inline long atomic_exchange<long>(volatile long* _X, long _Y) { return _InterlockedExchange(_X, _Y); }
	template<> inline float atomic_exchange<float>(volatile float* _X, float _Y) { long L = _InterlockedExchange((volatile long*)_X, *(long*)&_Y); return *(float*)&L; }

	template<typename T, typename U> bool atomic_compare_exchange(volatile T* _X, U _New, U _Old) { return atomic_compare_exchange(_X, (T)_New, (T)_Old); }
	template<typename T> bool atomic_compare_exchange(volatile T* _X, T _New, T _Old) { return atomic_compare_exchange((std::make_signed<T>::type*)_X, *(std::make_signed<T>::type*)&_New, *(std::make_signed<T>::type*)&_Old); }
	template<typename T> bool atomic_compare_exchange(T* volatile* _X, T* _New, T* _Old) { return (T*)_InterlockedCompareExchangePointer((void* volatile*)_X, _New, _Old) == (T*)_Old; }
	template<> inline bool atomic_compare_exchange<bool>(volatile bool* _X, bool _New, bool _Old) { return _InterlockedCompareExchange8((volatile char*)_X, *(char*)&_New, *(char*)&_Old) == *(char*)&_Old; }
	template<> inline bool atomic_compare_exchange<char>(volatile char* _X, char _New, char _Old) { return _InterlockedCompareExchange8(_X, _New, _Old) == _Old; }
	template<> inline bool atomic_compare_exchange<short>(volatile short* _X, short _New, short _Old) { return _InterlockedCompareExchange16(_X, _New, _Old) == _Old; }
	template<> inline bool atomic_compare_exchange<int>(volatile int* _X, int _New, int _Old) { return _InterlockedCompareExchange((volatile long*)_X, *(long*)&_New, *(long*)&_Old) == *(long*)&_Old; }
	template<> inline bool atomic_compare_exchange<float>(volatile float* _X, float _New, float _Old) { return _InterlockedCompareExchange((volatile long*)_X, *(long*)&_New, *(long*)&_Old) == *(long*)&_Old; }
	template<> inline bool atomic_compare_exchange<long>(volatile long* _X, long _New, long _Old) { return _InterlockedCompareExchange(_X, _New, _Old) == _Old; }

	template<typename T> T atomic_fetch_add(volatile T* _X, T _Y) { std::make_signed<std::tr1::remove_volatile<T>::type>::type t = atomic_fetch_add((std::make_signed<std::tr1::remove_volatile<T>::type>::type*)_X, (std::make_signed<std::tr1::remove_volatile<T>::type>::type)_Y); return *(T*)&t; }
	//template<> inline short atomic_fetch_add(volatile short* _X, short _Y) { return _InterlockedAdd16(_X, _Y); }
	template<> inline long atomic_fetch_add(volatile long* _X, long _Y)
	{
		#if 0 // only on itanium
			return (int)_InterlockedAdd((long*)_X, (long)_Y);
		#else
			_InterlockedExchangeAdd((long*)_X, (long)_Y);
			return *_X + _Y;
		#endif
	}

	template<> inline int atomic_fetch_add(volatile int* _X, int _Y) { return atomic_fetch_add((volatile long*)_X, (long)_Y); }

	template<typename T> T atomic_fetch_and(volatile T* _X, T _Y) { std::make_signed<std::tr1::remove_volatile<T>::type>::type t = atomic_fetch_and((std::make_signed<std::tr1::remove_volatile<T>::type>::type*)_X, (std::make_signed<std::tr1::remove_volatile<T>::type>::type)_Y); return *(T*)&t; }
	template<> inline short atomic_fetch_and(volatile short* _X, short _Y) { return _InterlockedAnd16(_X, _Y); }
	template<> inline int atomic_fetch_and(volatile int* _X, int _Y) { return (int)_InterlockedAnd((volatile long*)_X, (long)_Y); }
	template<> inline long atomic_fetch_and(volatile long* _X, long _Y) { return _InterlockedAnd(_X, _Y); }

	template<typename T> T atomic_fetch_or(volatile T* _X, T _Y) { std::make_signed<std::tr1::remove_volatile<T>::type>::type t = atomic_fetch_or((std::make_signed<std::tr1::remove_volatile<T>::type>::type*)_X, (std::make_signed<std::tr1::remove_volatile<T>::type>::type)_Y); return *(T*)&t; }
	template<> inline short atomic_fetch_or(volatile short* _X, short _Y) { return _InterlockedOr16(_X, _Y); }
	template<> inline int atomic_fetch_or(volatile int* _X, int _Y) { return (int)_InterlockedOr((volatile long*)_X, (long)_Y); }
	template<> inline long atomic_fetch_or(volatile long* _X, long _Y) { return _InterlockedOr(_X, _Y); }

	template<typename T> T atomic_fetch_xor(volatile T* _X, T _Y) { std::make_signed<std::tr1::remove_volatile<T>::type>::type t = atomic_fetch_xor((std::make_signed<std::tr1::remove_volatile<T>::type>::type*)_X, (std::make_signed<std::tr1::remove_volatile<T>::type>::type)_Y); return *(T*)&t; }
	template<> inline short atomic_fetch_xor(volatile short* _X, short _Y) { return _InterlockedXor16(_X, _Y); }
	template<> inline int atomic_fetch_xor(volatile int* _X, int _Y) { return (int)_InterlockedXor((volatile long*)_X, (long)_Y); }
	template<> inline long atomic_fetch_xor(volatile long* _X, long _Y) { return _InterlockedXor(_X, _Y); }

	template<typename T> T atomic_increment(volatile T* _X) { std::make_signed<std::tr1::remove_volatile<T>::type>::type t = atomic_increment((std::make_signed<std::tr1::remove_volatile<T>::type>::type*)_X); return *(T*)&t; }
	template<> inline short atomic_increment(volatile short* _X) { return _InterlockedIncrement16(_X); }
	template<> inline int atomic_increment(volatile int* _X) { return (int)_InterlockedIncrement((volatile long*)_X); }
	template<> inline long atomic_increment(volatile long* _X) { return _InterlockedIncrement(_X); }

	template<typename T> T atomic_decrement(T* _X) { std::make_signed<std::tr1::remove_volatile<T>::type>::type t = atomic_decrement((std::make_signed<std::tr1::remove_volatile<T>::type>::type*)_X); return *(T*)&t; }
	template<> inline short atomic_decrement(short* _X) { return _InterlockedDecrement16(_X); }
	template<> inline int atomic_decrement(int* _X) { return (int)_InterlockedDecrement((volatile long*)_X); }
	template<> inline long atomic_decrement(long* _X) { return _InterlockedDecrement(_X); }

	#ifdef oHAS_64BIT_ATOMICS
		template<> inline long long atomic_exchange<long long>(volatile long long* _X, long long _Y) { return _InterlockedExchange64(_X, _Y); }
		template<> inline double atomic_exchange<double>(volatile double* _X, double _Y) { long long L = _InterlockedExchange64((volatile long long*)_X, *(long long*)&_Y); return *(double*)&L; }
		template<> inline bool atomic_compare_exchange<long long>(volatile long long* _X, long long _New, long long _Old) { return _InterlockedCompareExchange64(_X, _New, _Old) == _Old; }
		template<> inline bool atomic_compare_exchange<double>(volatile double* _X, double _New, double _Old) { return _InterlockedCompareExchange64((volatile long long*)_X, *(long long*)&_New, *(long long*)&_Old) == *(long long*)&_Old; }
		template<> inline long long atomic_fetch_add(volatile long long* _X, long long _Y) { return _InterlockedAdd64(_X, _Y); }
		template<> inline long long atomic_fetch_and(volatile long long* _X, long long _Y) { return _InterlockedAnd64(_X, _Y); }
		template<> inline long long atomic_increment(volatile long long* _X) { return _InterlockedIncrement64(_X); }
		template<> inline long long atomic_decrement(long long* _X) { return _InterlockedDecrement64(_X); }
		template<> inline long long atomic_fetch_or(volatile long long* _X, long long _Y) { return _InterlockedOr64(_X, _Y); }
		template<> inline long long atomic_fetch_xor(volatile long long* _X, long long _Y) { return _InterlockedXor64(_X, _Y); }
	#endif

template<typename T> class atomic_base
{
protected:
	T Value;
	atomic_base(const atomic_base&);
	atomic_base& operator=(const atomic_base&);
public:
	atomic_base() {}
	/*constexpr*/ atomic_base(T _Value) : Value(_Value) {}
	bool operator=(T _That) volatile { exchange(_That.Value); return true; }
  T exchange(T _Value) volatile { return atomic_exchange(&Value, _Value); }

	bool compare_exchange(T _ExpectedValue, T _NewValue) volatile
	{	// NOTE: This does NOT update _ExpectedValue as the spec describes at this time.
		return atomic_compare_exchange(&Value, _NewValue, _ExpectedValue);
	}
	
	operator T() const volatile { return Value; }
};

template<typename T> class atomic_integral : public atomic_base<T>
{
	atomic_integral(const atomic_integral&);
	atomic_integral& operator=(const atomic_integral&);
public:
	atomic_integral() {}
	/*constexpr*/ atomic_integral(T _Value) : atomic_base(_Value) {}

	T fetch_add(T _Value) volatile { T OldV; do { OldV = Value; } while (!compare_exchange(OldV, OldV+_Value)); return OldV; }
	T fetch_sub(T _Value) volatile { T OldV; do { OldV = Value; } while (!compare_exchange(OldV, OldV-_Value)); return OldV; }
	T operator+=(T _Value) volatile { return fetch_add(_Value) + _Value; }
	T operator-=(T _Value) volatile { return fetch_sub(_Value) - _Value; }

	T operator++() volatile { atomic_increment(&Value); return Value; }
	T operator++(int) volatile { T v = atomic_increment(&Value); return v-1; }
	T operator--() volatile { atomic_decrement(&Value); return Value; }
	T operator--(int) volatile { T v = atomic_decrement(&Value); return v+1; }
};

#define oATOMIC_INTEGRAL(_Type) \
	template<> class atomic<_Type> : public atomic_integral<_Type> { public: atomic() {}; atomic(_Type _Value) : atomic_integral(_Value) {} }; \
	template<> class atomic<unsigned _Type> : public atomic_integral<unsigned _Type> { public: atomic() {}; atomic(unsigned _Type _Value) : atomic_integral(_Value) {} }

template<typename T> class atomic : public atomic_base<T> {};
template<> class atomic<bool> : public atomic_base<bool> { public: atomic(bool _Value) : atomic_base(_Value) {} };
oATOMIC_INTEGRAL(char); oATOMIC_INTEGRAL(short); oATOMIC_INTEGRAL(int); oATOMIC_INTEGRAL(long); oATOMIC_INTEGRAL(long long);

template<typename T> class atomic<T*>
{
	T* Value;
	atomic(const atomic&);
	atomic& operator=(const atomic&);
public:
	atomic() {}
	/*constexpr*/ atomic(T* _Value) : Value(_Value) {}
	bool operator=(T* _That) volatile { exchange(_That.Value); return true; }
  T* exchange(T* _Value) volatile { return atomic_exchange(&Value, _Value); }

	bool compare_exchange(T* _ExpectedValue, T* _NewValue) volatile
	{
		T* PriorValue = atomic_compare_exchange(&Value, _NewValue, _ExpectedValue);
		if (PriorValue == _ExpectedValue)
			return true;
		//_ExpectedValue = PriorValue;
		return false;
	}
	
	operator T*() const volatile { return Value; }

	T* fetch_add(ptrdiff_t _Value) volatile { return (T*)atomic_fetch_add(&Value, _Value); }
	T* fetch_sub(ptrdiff_t _Value) volatile { return (T*)atomic_fetch_add(&Value, -_Value); }

	T* operator+=(ptrdiff_t _Value) volatile { return fetch_add(_Value) + _Value; }
	T* operator-=(ptrdiff_t _Value) volatile { return fetch_sub(_Value) - _Value; }
};

typedef atomic<bool> atomic_bool;
typedef atomic<char> atomic_char;
typedef atomic<unsigned char> atomic_uchar;
typedef atomic<short> atomic_short;
typedef atomic<unsigned short> atomic_ushort;
typedef atomic<int> atomic_int;
typedef atomic<unsigned int> atomic_uint;
typedef atomic<long> atomic_long;
typedef atomic<unsigned long> atomic_ulong;
typedef atomic<long long> atomic_llong;
typedef atomic<unsigned long long> atomic_ullong;
typedef atomic<size_t> atomic_size_t;

struct atomic_flag
{
	atomic_flag() : Bool(false) {}

	bool test_and_set() volatile { return !Bool.compare_exchange(false, true); }
	void clear() volatile { Bool.exchange(false); }

protected:
	atomic_flag(const atomic_flag&); /* = delete; */
	atomic_flag& operator=(const atomic_flag&); /* = delete; */

	atomic_bool Bool;
};

inline bool atomic_flag_test_and_set(volatile atomic_flag* _pFlag) { return _pFlag->test_and_set(); }
//bool atomic_flag_test_and_set_explicit(volatile atomic_flag*, memory_order);
inline void atomic_flag_clear(volatile atomic_flag* _pFlag) { _pFlag->clear(); }
//void atomic_flag_clear_explicit(volatile atomic_flag*, memory_order);

} // namespace oStd

#endif
