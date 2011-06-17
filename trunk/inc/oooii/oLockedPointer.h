// $(header)

// A scoped pointer object that uses the same pattern as oRef to lock and 
// unlock threadsafe API and return a non-threadsafe pointer that is
// made safe for the scope of the oLockedPointer object.
// The user must define:
// void intrusive_ptr_lock_read(T* p);
// void intrusive_ptr_unlock_read(T* p);
// void intrusive_ptr_lock(T* p);
// void intrusive_ptr_unlock(T* p);
#pragma once
#ifndef oLockedPointer_h
#define oLockedPointer_h

#include <oooii/oStddef.h>
#include <oooii/oNonCopyable.h>

template<class T> struct oConstLockedPointer : oNoncopyable
{
	oConstLockedPointer() : _p(0) {}
	oConstLockedPointer(const threadsafe T* _Pointer) : _p(_Pointer) { if (_p) intrusive_ptr_lock_read(_p); }
	~oConstLockedPointer() { if (_p) intrusive_ptr_unlock_read(_p); }

	oConstLockedPointer<T>& operator=(const threadsafe  T* _Pointer)
	{
		if (_p) intrusive_ptr_unlock_read(_p);
		_p = _Pointer;
		if (_p) intrusive_ptr_lock_read(_p);
		return *this;
	}

	const T* c_ptr() { return thread_cast<T*>(_p); }
	const T* operator->() { return c_ptr(); }
	operator const T*() { return c_ptr(); }

private:
	const threadsafe  T* _p;
};

template<class T> struct oLockedPointer : oNoncopyable
{
	oLockedPointer() : _p(0) {}
	oLockedPointer(threadsafe T* _Pointer) : _p(_Pointer) { if (_p) intrusive_ptr_lock(_p); }
	~oLockedPointer() { if (_p) intrusive_ptr_unlock(_p); }

	oLockedPointer<T>& operator=(threadsafe T* _Pointer)
	{
		if (_p) intrusive_ptr_unlock(_p);
		_p = _Pointer;
		if (_p) intrusive_ptr_lock(_p);
		return *this;
	}

	const T* c_ptr() const { return thread_cast<T*>(_p); }
	T* c_ptr() { return thread_cast<T*>(_p); }

	const T* operator->() const { return c_ptr(); }
	T* operator->() { return c_ptr(); }

	operator const T*() const { return c_ptr(); }
	operator T*() { return c_ptr(); }

private:
	threadsafe T* _p;
};

#endif
