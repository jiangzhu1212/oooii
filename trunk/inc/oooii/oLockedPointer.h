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
#ifndef oLocked_h
#define oLocked_h

#include <oooii/oStddef.h>
#include <oooii/oNonCopyable.h>

template<class T> class oConstLockedPointer : oNoncopyable
{
	// A scoped pointer for objects that can be locked for read-only concurrent
	// access. The user must define the following unqualified API class T:
	//
	// void intrusive_ptr_lock_read(T* p);
	// void intrusive_ptr_unlock_read(T* p);
public:
	oConstLockedPointer() : _p(0) {}
	oConstLockedPointer(threadsafe const T* _Pointer) : _p(const_cast<threadsafe T*>(_Pointer)) { if (_p) intrusive_ptr_lock_read(_p); }
	~oConstLockedPointer() { if (_p) intrusive_ptr_unlock_read(_p); }

	oConstLockedPointer<T>& operator=(threadsafe const T* _Pointer)
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
	threadsafe T* _p;
};

template<class T> class oLockedPointer : oNoncopyable
{
	// A scoped pointer for objects that can be locked for read-only concurrent
	// access. The user must define the following unqualified API class T:
	//
	// void intrusive_ptr_lock(T* p);
	// void intrusive_ptr_unlock(T* p);
public:
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
