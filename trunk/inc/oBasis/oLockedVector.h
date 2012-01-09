// $(header)
// Sometimes its not necessary to overthink thread safety - such as is often the
// case with registering objects with a manager and accessing them by index.
// Registration is infrequent and access doesn't change the topology of the 
// list.
// This is a threadsafe vector that locks on most operations. We try to match 
// STL when possible, but it is not STL compliant as much of STL vector cannot 
// be made threadsafe easily (iterators, insertion, etc.) 
// NOTE: Even const API may lock the mutex as appropriate, so this interface 
// isn't const in the strictest sense of the word, so don't use this in ROM-
// accessed code.

// GetLockedSTLVector 
// can be called to retrieve an STL compliant vector but will block access
// to the vector until the LockedSTLVector has passed out of scope.
#pragma once
#ifndef oLockedVector_h
#define oLockedVector_h

#include <oBasis/oStdMutex.h>
#include <oBasis/oThreadsafe.h>
#include <vector>

template<typename MutexT, typename T, typename Alloc = std::allocator<T>>
class oLockedVectorBase : private std::vector<T, Alloc>
{
public:
	typedef std::vector<T, Alloc> base;

	oLockedVectorBase(Alloc _Allocator = Alloc()) : std::vector<T, Alloc>(_Allocator) {}

	inline void reserve(size_t _Size) threadsafe
	{
		oStd::lock_guard<MutexT> lock(raw_mutex());
		raw_base()->reserve(_Size);
	}

	inline void push_back(const T& _Element) threadsafe
	{
		oStd::lock_guard<MutexT> lock(raw_mutex());
		raw_base()->push_back(_Element);
	}

	inline void pop_back() threadsafe
	{
		oStd::lock_guard<MutexT> lock(raw_mutex());
		raw_base()->pop_back();
	}

	inline bool pop_back(T* _Element) threadsafe
	{
		oStd::lock_guard<MutexT> lock(raw_mutex());
		if (raw_base()->size() == 0)
			return false;

		*_Element = raw_base()->back();
		raw_base()->pop_back();
		return true;
	}

	inline void resize(size_t _NewSize) threadsafe
	{
		oStd::lock_guard<MutexT> lock(raw_mutex());
		return raw_base()->resize(_NewSize);
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
		oStd::lock_guard<MutexT> lock(raw_mutex());
		return raw_base()->clear();
	}

	inline bool erase(const T& _Search) threadsafe
	{
		oStd::lock_guard<MutexT> lock(raw_mutex());
		base::iterator it = std::find(raw_base()->begin(), raw_base()->end(), _Search);
		if (it != raw_base()->end())
		{
			raw_base()->erase(it);
			return true;
		}

		return false;
	}

	// sets capacity() to size()
	inline void trim() threadsafe
	{
		oStd::lock_guard<MutexT> lock(raw_mutex());
		oZeroCapacity(*raw_base());
	}

	// Return true to continue a foreach, return false to short-circuit
	// and not process further items in the list
	inline bool foreach(oFUNCTION<bool(T& _Element)> _Callback) threadsafe
	{
		oStd::shared_lock<MutexT> lock(raw_mutex());
		for (base::iterator it = raw_base()->begin(); it != raw_base()->end(); ++it)
			if (!_Callback(*it))
				return false;
		return true;
	}

	inline bool const_foreach(oFUNCTION<bool(const T& _Element)> _Callback) const threadsafe
	{
		oStd::shared_lock<MutexT> lock(raw_mutex());
		for (base::const_iterator it = raw_base()->begin(); it != raw_base()->end(); ++it)
			if (!_Callback(*it))
				return false;
		return true;
	}

	// @oooii-tony: Can we use oLockedPointer here instead?

	// Locking objects for raw access to the STL vector
	class LockedSTLVector : oNoncopyable
	{
	public:
		LockedSTLVector(MutexT& _Mutex, base* _RawVector)
			: Mutex(_Mutex)
			, RawVector(_RawVector)
		{
			Mutex.lock();
		}

		~LockedSTLVector()
		{
			Mutex.unlock();  
		}

		base& operator*() { return *RawVector; }
		base* operator->() { return RawVector; }

	private:
		MutexT& Mutex;
		base* RawVector;
	};
	class ConstLockedSTLVector : oNoncopyable
	{
	public:
		ConstLockedSTLVector(MutexT& mutex, const base* _RawVector)
			: Mutex(mutex)
			, RawVector(_RawVector)
		{
			Mutex.lock_shared();
		}

		~ConstLockedSTLVector()
		{
			Mutex.unlock_shared();
		}

		const base& operator*() const { return *RawVector; }
		const base* operator->() const { return RawVector; }

	private:
		MutexT& Mutex;
		const base* RawVector;
	};

	LockedSTLVector lock() threadsafe { return LockedSTLVector(raw_mutex(), raw_base()); }
	ConstLockedSTLVector const_lock() const threadsafe { return ConstLockedSTLVector(raw_mutex(), const_cast<base*>(static_cast<const threadsafe base*>(this))); }

private:
	// Gives raw unprotected access to the base. Should only be called by functions that are 
	// protecting the thread safety of the base
	inline base* raw_base() threadsafe { return thread_cast<base*>( static_cast<threadsafe base*>(this) ); }
	inline const base* raw_base() const threadsafe { return thread_cast<const base*>( static_cast<const threadsafe base*>(this) ); }
	inline MutexT& raw_mutex() const threadsafe { return thread_cast<MutexT&>(Mutex); }
	MutexT Mutex;
};

// A locked vector that uses a lightweight RWMutex but is not recursive
template<typename T, typename Alloc = std::allocator<T>>
class oLockedVector : public oLockedVectorBase<oStd::shared_mutex, T, Alloc> { public: oLockedVector(Alloc _Allocator = Alloc()) : oLockedVectorBase<oStd::shared_mutex, T, Alloc>(_Allocator) {} };

// A locked vector that is recursive using a heavier critical section
template<typename T, typename Alloc = std::allocator<T>>
class oRecursiveLockedVector : public oLockedVectorBase<oStd::recursive_mutex, T, Alloc> { public: oRecursiveLockedVector(Alloc _Allocator = Alloc()) : oLockedVectorBase<oStd::recursive_mutex, T, Alloc>(_Allocator) {} };

#endif
