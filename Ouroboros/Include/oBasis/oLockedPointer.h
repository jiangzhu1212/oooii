/**************************************************************************
 * The MIT License                                                        *
 * Copyright (c) 2014 Antony Arciuolo.                                    *
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
// A scoped pointer object that uses the same pattern as ouro::intrusive_ptr 
// to lock and unlock threadsafe API and return a non-threadsafe pointer that 
// is made safe for the scope of the oLockedPointer object. The user must define:
// void intrusive_ptr_lock_shared(T* p);
// void intrusive_ptr_unlock_shared(T* p);
// void intrusive_ptr_lock(T* p);
// void intrusive_ptr_unlock(T* p);
#pragma once
#ifndef oLockedPointer_h
#define oLockedPointer_h

template<class T> struct oConstLockedPointer
{
	oConstLockedPointer() : _p(nullptr) {}
	oConstLockedPointer(const threadsafe T* _Pointer) : _p(_Pointer) { if (_p) intrusive_ptr_lock_shared(_p); }
	~oConstLockedPointer() { if (_p) intrusive_ptr_unlock_shared(_p); }

	oConstLockedPointer<T>& operator=(const threadsafe T* _Pointer)
	{
		if (_p) intrusive_ptr_unlock_shared(_p);
		_p = _Pointer;
		if (_p) intrusive_ptr_lock_shared(_p);
		return *this;
	}

	const T* c_ptr() { return thread_cast<T*>(_p); }
	const T* operator->() { return c_ptr(); }
	operator const T*() { return c_ptr(); }

private:
	oConstLockedPointer(const oConstLockedPointer&);
	oConstLockedPointer& operator=(const oConstLockedPointer&);
	const threadsafe  T* _p;
};

template<class T> struct oLockedPointer
{
	oLockedPointer() : _p(nullptr) {}
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
	oLockedPointer(const oLockedPointer&);
	oLockedPointer& operator=(const oLockedPointer&);
	threadsafe T* _p;
};

#endif
