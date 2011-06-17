// $(header)

// Mutex for multithreaded synchronization. Though mutex is really a 
// platform concept, it is so fundamental to ensure threadsafe API and 
// is really a very simple object to wrap that a bootstrap exception
// is made and some forward-declaritive hoops are jumped through to
// ensure this interface is available even for generic code.
#pragma once
#ifndef oMutex_h
#define oMutex_h

#include <oooii/oMutexInternal.h>
#include <oooii/oNoncopyable.h>
#include <oooii/oStddef.h>

class oMutex : oNoncopyable
{
	// oMutex is recursive. oMutex is assumed mutable, so all API is const
	oMUTEX_FOOTPRINT();
public:
	oMutex();
	~oMutex();
	void* GetNativeHandle() const threadsafe;
	void Lock() const threadsafe;
	bool TryLock() const threadsafe;
	void Unlock() const threadsafe;

	class ScopedLock : oNoncopyable
	{
		const threadsafe oMutex& m;
	public:
		inline ScopedLock(const threadsafe oMutex& _Mutex) : m(_Mutex) { m.Lock(); }
		inline ~ScopedLock() { m.Unlock(); }
	};
};

class oRWMutex : oNoncopyable
{
	// oRWMutex is non-recursive.
	oRWMUTEX_FOOTPRINT();
public:
	oRWMutex();
	~oRWMutex();

	void* GetNativeHandle() threadsafe;
	const void* GetNativeHandle() const threadsafe;
	void Lock() threadsafe; // Locks exclusively, no other lock attempt can go through
	void LockRead() const threadsafe; // Locks shared, other shared locks can go through, but not exclusives
	void Unlock() threadsafe;
	void UnlockRead() const threadsafe;

	#ifdef oHAS_RWMUTEX_TRYLOCK
		bool TryLock() threadsafe;
		bool TryLockRead() const threadsafe;
	#endif

	class ScopedLock : oNoncopyable
	{
		threadsafe oRWMutex& m;
	public:
		inline ScopedLock(threadsafe oRWMutex& _Mutex) : m(_Mutex) { m.Lock(); }
		inline ~ScopedLock() { m.Unlock(); }
	};

	class ScopedLockRead : oNoncopyable
	{
		const threadsafe oRWMutex& m;
	public:
		inline ScopedLockRead(const threadsafe oRWMutex& _Mutex) : m(_Mutex) { m.LockRead(); }
		inline ~ScopedLockRead() { m.UnlockRead(); }
	};
};

#endif
