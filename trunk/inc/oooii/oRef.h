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
#ifndef oRef_h
#define oRef_h

#include <oooii/oStddef.h>

template<class T> class oRef
{
	// A ref-counted pointer. This requires the user to define the 
	// following unqualified API:
	//
	// void intrusive_ptr_add_ref(T* p);
	// void intrusive_ptr_release(T* p);
	// (p) // (as in if (p != 0) {...})
	//
	// ref was coded instead of using boost::intrusive_ptr because
	// I feel that ref-counted pointers should be castable to raw
	// pointers. ref counting is a sign of ownership. To me (a console 
	// video game developer primarily) allowing objects to hang 
	// around if someone refers to them shouldn't be arbitrary, and 
	// thus I would like to see care with the usage of smart pointers.
	// If a function or class does not own the pointer's contents, then 
	// there's no reason for it to have a smart pointer in its API.
	// boost::intrusive_ptr requires an explicit call to .get(), which
	// clutters up the code and is a nusance. Notice as well how much
	// less code is required for the smart pointer because so many 
	// operators and operations do not have to be recoded to enforce
	// overzealous type safety.
public:
	oRef() : _p(0) {}
	~oRef() { if (_p) intrusive_ptr_release(_p); }

	oRef(const oRef<T>& rhs)
	{
		_p = const_cast<T*>(rhs.c_ptr());
		if (_p) intrusive_ptr_add_ref(_p);
	}

	// Specify ref = false if initializing from a factory call that
	// returns a ref of 1.
	oRef(const threadsafe T* rhs, bool ref = true)
	{
		_p = static_cast<T*>(const_cast<T*>(rhs));
		if (_p && ref) intrusive_ptr_add_ref(_p);
	}

	const threadsafe oRef<T>& operator=(const threadsafe T* rhs) threadsafe
	{
		if (_p) intrusive_ptr_release(_p);
		_p = const_cast<T*>(rhs);
		if (_p) intrusive_ptr_add_ref(_p);
		return *this;
	}
	
	const threadsafe oRef<T>& operator=(const threadsafe oRef<T>& rhs) threadsafe
	{
		if (_p) intrusive_ptr_release(_p);
		_p = const_cast<T*>(rhs.c_ptr());
		if (_p) intrusive_ptr_add_ref(_p);
		return *this;
	}
	
	// Assign without incrementing the new value's refcount. Sometimes
	// factories or Get*() in Microsoft APIs give raw pointers that already
	// have their ref incremented, so use this on the initial assignment.
	const threadsafe oRef<T>& operator/=(const threadsafe T* rhs) threadsafe
	{
		if (_p) intrusive_ptr_release(_p);
		_p = const_cast<T*>(rhs);
		return *this;
	}

	T* operator->() const { return _p; }
	threadsafe T* operator->() const threadsafe { return _p; }
	operator const T*() const { return _p; }
	operator const threadsafe T*() const threadsafe { return _p; }
	operator T*() { return _p; }
	operator threadsafe T*() threadsafe { return _p; }

	// & is used during creation where it is expected that the resulting action 
	// will leave us with a refcount of one, therefore first assign to NULL 
	// clearing out any prior reference. Careful, because if used in other 
	// contexts this will clear the pointer inappropriately. Use myRef.address()
	// to pass the direct pointer value's address.
	T** operator &() { (*this) = NULL; return &_p; }
	threadsafe T** operator &() threadsafe { (*this) = NULL; return (threadsafe T**)&_p; } // & is used during creation where it is expected that the resulting action will leave us with a refcount of one, therefore first assign to NULL clearing out any prior reference
	const threadsafe T** operator &() const threadsafe { (*this) = NULL; return &_p; } // & is used during creation where it is expected that the resulting action will leave us with a refcount of one, therefore first assign to NULL clearing out any prior reference
	
	T* c_ptr() { return _p; }
	const T* c_ptr() const { return _p; }
	threadsafe T* c_ptr() const threadsafe { return _p; }

	T** address() { return &_p; }
	const T** address() const { return &_p; }
	threadsafe T** address() threadsafe { return &_p; }
	const threadsafe T** address() const threadsafe { return &_p; }

private:
	T* _p;
};

#endif
