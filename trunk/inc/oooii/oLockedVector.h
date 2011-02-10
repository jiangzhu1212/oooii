/**************************************************************************
 * The MIT License                                                        *
 * Copyright (c) 2011 Antony Arciuolo & Kevin Myers                       *
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
#ifndef oLockedVector_h
#define oLockedVector_h

#include <oooii/oThreading.h>
#include <vector>
#include <algorithm>

// This is a threadsafe vector that locks
// on all operations.  We try to match STL
// when possible, but it is not STL compliant
// as much of STL vector can not be made
// threadsafe easily (iterators, insertion, et cetra)
// GetLockedSTLVector can be called to retrieve
// an STL compliant vector but will block access
// to the vector until the LockedSTLVector has passed
// out of scope
template<typename T, typename T_ALLOC = std::allocator<T>>
class oLockedVector : private std::vector<T, T_ALLOC>
{
public:
	oLockedVector( T_ALLOC _Allocator = T_ALLOC() ) : std::vector<T, T_ALLOC>( _Allocator )
	{}

	typedef std::vector<T, T_ALLOC> base;

	inline void reserve( size_t size ) threadsafe
	{
		oRWMutex::ScopedLock ScopeLock(m_mutex);
		raw_base()->reserve(size);
	}

	inline void push_back(const T& element) threadsafe
	{
		oRWMutex::ScopedLock ScopeLock(m_mutex);
		raw_base()->push_back(element);
	}
	inline void pop_back() threadsafe
	{
		oRWMutex::ScopedLock ScopeLock(m_mutex);
		raw_base()->pop_back();
	}
	inline bool pop_back( T* element ) threadsafe
	{
		oRWMutex::ScopedLock ScopeLock(m_mutex);
		if( raw_base()->size() == 0 )
			return false;

		*element = raw_base()->back();
		raw_base()->pop_back();
		return true;
	}

	inline void resize( int newSize ) threadsafe
	{
		oRWMutex::ScopedLock ScopeLock(m_mutex);
		return raw_base()->resize( newSize );
	}
	inline size_t size() threadsafe
	{
		return raw_base()->size();
	}

	inline bool empty() threadsafe
	{
		oRWMutex::ScopedLock ScopeLock(m_mutex);
		return raw_base()->empty();
	}

	inline void clear() threadsafe
	{
		oRWMutex::ScopedLock ScopeLock(m_mutex);
		return raw_base()->clear();
	}

	inline void erase( const T& search ) threadsafe
	{
		oRWMutex::ScopedLock ScopeLock(m_mutex);
		base::iterator iter = std::find( raw_base()->begin(), raw_base()->end(), search );
		if( iter !=  raw_base()->end() )
			raw_base()->erase( iter );
	}

	// Return true to continue a foreach, return false to short-circuit
	// and not process further items in the list
	typedef oFUNCTION<bool( T& element )> ForeachCallback;

	inline bool foreach( ForeachCallback callback ) threadsafe
	{
		oRWMutex::ScopedLock ScopeLock( m_mutex );
		for( base::iterator iter = raw_base()->begin(); iter != raw_base()->end(); ++iter )
			if( !callback( *iter ) )
				return false;
		return true;
	}

	// Locking objects for raw access to the STL vector
	class LockedSTLVector : oNoncopyable
	{
	public:
		LockedSTLVector( threadsafe oRWMutex& mutex, base* rawVector ) :
			m_mutex( mutex )
				, m_rawVector( rawVector )
			{
				m_mutex.Lock();
			}
			~LockedSTLVector()
			{
				m_mutex.Unlock();  
			}

			base& operator*() { return *m_rawVector; }
			base* operator->() { return m_rawVector; }

	private:
		threadsafe oRWMutex& m_mutex;
		base* m_rawVector;
	};
	class ConstLockedSTLVector : oNoncopyable
	{
	public:
		ConstLockedSTLVector( threadsafe oRWMutex& mutex, const base* rawVector ) :
			m_mutex( mutex )
				, m_rawVector( rawVector )
			{
				m_mutex.LockRead();
			}
			~ConstLockedSTLVector()
			{
				m_mutex.UnlockRead();
			}

			const base& operator*() const { return *m_rawVector; }
			const base* operator->() const { return m_rawVector; }

	private:
		threadsafe oRWMutex& m_mutex;
		const base* m_rawVector;
	};


	LockedSTLVector lock() threadsafe { return LockedSTLVector( m_mutex, raw_base() ); }
	ConstLockedSTLVector const_lock() const threadsafe { return ConstLockedSTLVector( const_cast<threadsafe oRWMutex&>( m_mutex ), const_cast<base*>( static_cast<const threadsafe base*>(this) ) ); }  // Const_cast necessary to acess mutex.  Const is to protect the underlying object

private:
	// Gives raw unprotected access to the base. Should only be called by functions that are 
	// protecting the thread safety of the base
	inline base* raw_base() threadsafe { return thread_cast<base*>( static_cast<threadsafe base*>(this) ); }
	oRWMutex m_mutex;
};

#endif //oLockedVector_h
