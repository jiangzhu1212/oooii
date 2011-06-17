// $(header)

// This is a threadsafe vector that locks on all operations. We try to match STL 
// when possible, but it is not STL compliant as much of STL vector cannot be 
// made threadsafe easily (iterators, insertion, et cetra) GetLockedSTLVector 
// can be called to retrieve an STL compliant vector but will block access
// to the vector until the LockedSTLVector has passed out of scope.
#pragma once
#ifndef oLockedVector_h
#define oLockedVector_h

#include <oooii/oMutex.h>
#include <oooii/oSTL.h>

template<typename T_MUTEX, typename T, typename Alloc = std::allocator<T>>
class oLockedVectorBase : private std::vector<T, Alloc>
{
public:
	oLockedVectorBase( Alloc _Allocator = Alloc() ) : std::vector<T, Alloc>( _Allocator )
	{}

	typedef std::vector<T, Alloc> base;

	inline void reserve( size_t size ) threadsafe
	{
		T_MUTEX::ScopedLock ScopeLock(m_mutex);
		raw_base()->reserve(size);
	}

	inline void push_back(const T& element) threadsafe
	{
		T_MUTEX::ScopedLock ScopeLock(m_mutex);
		raw_base()->push_back(element);
	}
	inline void pop_back() threadsafe
	{
		T_MUTEX::ScopedLock ScopeLock(m_mutex);
		raw_base()->pop_back();
	}
	inline bool pop_back( T* element ) threadsafe
	{
		T_MUTEX::ScopedLock ScopeLock(m_mutex);
		if( raw_base()->size() == 0 )
			return false;

		*element = raw_base()->back();
		raw_base()->pop_back();
		return true;
	}

	inline void resize( size_t newSize ) threadsafe
	{
		T_MUTEX::ScopedLock ScopeLock(m_mutex);
		return raw_base()->resize( newSize );
	}
	inline size_t size() threadsafe
	{
		return raw_base()->size();
	}

	inline size_t size() const threadsafe
	{
		return raw_base()->size();
	}

	inline bool empty() const threadsafe
	{
		return raw_base()->empty();
	}

	inline void clear() threadsafe
	{
		T_MUTEX::ScopedLock ScopeLock(m_mutex);
		return raw_base()->clear();
	}

	inline bool erase( const T& search ) threadsafe
	{
		T_MUTEX::ScopedLock ScopeLock(m_mutex);
		base::iterator iter = std::find( raw_base()->begin(), raw_base()->end(), search );
		if( iter !=  raw_base()->end() )
		{
			raw_base()->erase( iter );
			return true;
		}

		return false;
	}

	// sets capacity() to size()
	inline void trim() threadsafe
	{
		T_MUTEX::ScopedLock ScopeLock(m_mutex);
		base& b = *raw_base();
		oZeroCapacity(b);
	}

	// Return true to continue a foreach, return false to short-circuit
	// and not process further items in the list
	typedef oFUNCTION<bool( T& element )> ForeachCallback;

	inline bool foreach( ForeachCallback callback ) threadsafe
	{
		T_MUTEX::ScopedLock ScopeLock( m_mutex );
		for( base::iterator iter = raw_base()->begin(); iter != raw_base()->end(); ++iter )
			if( !callback( *iter ) )
				return false;
		return true;
	}

	// Locking objects for raw access to the STL vector
	class LockedSTLVector : oNoncopyable
	{
	public:
		LockedSTLVector( threadsafe T_MUTEX& mutex, base* rawVector ) :
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
		threadsafe T_MUTEX& m_mutex;
		base* m_rawVector;
	};
	class ConstLockedSTLVector : oNoncopyable
	{
	public:
		ConstLockedSTLVector( threadsafe T_MUTEX& mutex, const base* rawVector ) :
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
		threadsafe T_MUTEX& m_mutex;
		const base* m_rawVector;
	};

	LockedSTLVector lock() threadsafe { return LockedSTLVector( m_mutex, raw_base() ); }
	ConstLockedSTLVector const_lock() const threadsafe { return ConstLockedSTLVector( const_cast<threadsafe T_MUTEX&>( m_mutex ), const_cast<base*>( static_cast<const threadsafe base*>(this) ) ); }  // Const_cast necessary to acess mutex.  Const is to protect the underlying object

private:
	// Gives raw unprotected access to the base. Should only be called by functions that are 
	// protecting the thread safety of the base
	inline base* raw_base() threadsafe { return thread_cast<base*>( static_cast<threadsafe base*>(this) ); }
	inline const base* raw_base() const threadsafe { return thread_cast<const base*>( static_cast<const threadsafe base*>(this) ); }
	T_MUTEX m_mutex;
};

// A locked vector that uses a lightweight RWMutex but is not recursive
template<typename T, typename Alloc = std::allocator<T>>
class oLockedVector : public oLockedVectorBase<oRWMutex, T, Alloc> { public: oLockedVector(Alloc _Allocator = Alloc()) : oLockedVectorBase<oRWMutex, T, Alloc>(_Allocator) {} };

// A locked vector that is recursive using a heavier critical section
template<typename T, typename Alloc = std::allocator<T>>
class oRecursiveLockedVector : public oLockedVectorBase<oMutex, T, Alloc> { public: oRecursiveLockedVector(Alloc _Allocator = Alloc()) : oLockedVectorBase<oMutex, T, Alloc>(_Allocator) {} };

#endif
