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
// Convenience code for the common case when you have a threadsafe class, you 
// lock, and then you thread cast to access your member variables.
// Example Use: From a non static threadsafe member function.
//   auto lockedThis = oLockThis(MyMutex); //lockedThis is usable as a this 
//   pointer, but can access non threadsafe flagged member variables and 
//   functions.
//   lockedThis->MyMemberVariable = true;
#pragma once
#ifndef oLockThis_h
#define oLockThis_h

#include <oConcurrency/mutex.h>

template<typename MUTEX_TYPE, typename THIS_TYPE, template <typename> class LOCK_GUARD_TYPE> class oLockThisImpl
{
	typedef typename std::remove_cv<MUTEX_TYPE>::type mutex_t;
	typedef typename std::remove_cv<THIS_TYPE>::type this_t;
public:
	oLockThisImpl(MUTEX_TYPE& _Mutex, THIS_TYPE* _pThis)
		: Lock(const_cast<mutex_t&>(_Mutex))
	{
		LockedThis = thread_cast<this_t*>(_pThis);
	}

	this_t* operator->() { return LockedThis; }
	const this_t* operator->() const { return LockedThis; }
	this_t& operator*() { return *LockedThis; }
	const this_t& operator*() const { return *LockedThis; }
	this_t* c_ptr() { return LockedThis; }
	const this_t* c_ptr() const { return LockedThis; }

private:
	this_t* LockedThis;
	LOCK_GUARD_TYPE<mutex_t> Lock;
};

template<typename MUTEX_TYPE, typename THIS_TYPE> oLockThisImpl<MUTEX_TYPE, THIS_TYPE, oConcurrency::lock_guard> oLockThisAuto(MUTEX_TYPE& _Mutex, THIS_TYPE* _pThis)
{
	return oLockThisImpl<MUTEX_TYPE,THIS_TYPE, oConcurrency::lock_guard>(_Mutex, _pThis);
}

template<typename MUTEX_TYPE, typename THIS_TYPE> oLockThisImpl<MUTEX_TYPE, THIS_TYPE, ouro::shared_lock> oLockSharedThisAuto(MUTEX_TYPE& _Mutex, THIS_TYPE* _pThis)
{
	return oLockThisImpl<MUTEX_TYPE,THIS_TYPE, ouro::shared_lock>(_Mutex, _pThis);
}

#define oLockThis(_Mutex) oLockThisAuto(_Mutex, this)
#define oLockSharedThis(_Mutex) oLockSharedThisAuto(_Mutex, this)

#endif
