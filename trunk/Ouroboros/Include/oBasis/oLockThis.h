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

#include <oBasis/oMutex.h>

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

private:
	this_t* LockedThis;
	LOCK_GUARD_TYPE<mutex_t> Lock;
};

template<typename MUTEX_TYPE, typename THIS_TYPE> oLockThisImpl<MUTEX_TYPE, THIS_TYPE, oLockGuard> oLockThisAuto(MUTEX_TYPE& _Mutex, THIS_TYPE* _pThis)
{
	return oLockThisImpl<MUTEX_TYPE,THIS_TYPE, oLockGuard>(_Mutex, _pThis);
}

template<typename MUTEX_TYPE, typename THIS_TYPE> oLockThisImpl<MUTEX_TYPE, THIS_TYPE, oSharedLock> oLockSharedThisAuto(MUTEX_TYPE& _Mutex, THIS_TYPE* _pThis)
{
	return oLockThisImpl<MUTEX_TYPE,THIS_TYPE, oSharedLock>(_Mutex, _pThis);
}

template<typename T> const T* oThreadCastAuto(const threadsafe T* _pThis) { return thread_cast<const T*>(_pThis); }
template<typename T> T* oThreadCastAuto(threadsafe T* _pThis) { return thread_cast<T*>(_pThis); }

#define oLockThis(_Mutex) oLockThisAuto(_Mutex, this)
#define oLockSharedThis(_Mutex) oLockSharedThisAuto(_Mutex, this)

// Use this for underlying API that is known-threadsafe but does not label 
// methods volatile (oooii threadsafe keyword). USE THIS CAREFULLY, use a raw
// thread_cast to cast away threadsafety... only use this to cull out 
// thread_casts where it is KNOWN the API is absolutely threadsafe.
#define oInherentlyThreadsafe() oThreadCastAuto(this)

#endif
