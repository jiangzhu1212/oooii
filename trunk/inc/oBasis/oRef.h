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

// A simple intrusive smart pointer implementation. This requires the user to 
// define the following unqualified API:
// void intrusive_ptr_add_ref(T* p);
// void intrusive_ptr_release(T* p);
// (p) // (as in if (p != 0) {...})
//
// oRef was coded instead of using boost::intrusive_ptr because I feel that ref-
// counted pointers should be castable to raw pointers. ref counting is a sign 
// of ownership. To me (a console video game developer primarily) allowing 
// objects to hang around if someone refers to them shouldn't be arbitrary, and 
// thus I would like to see care with the usage of smart pointers. If a function 
// or class does not own the pointer's contents, then there's no reason for it 
// to have a smart pointer in its API. boost::intrusive_ptr requires an explicit 
// call to .get(), which clutters up the code and is a nusance. Notice as well 
// how much less code is required for the smart pointer because so many operators 
// and operations do not have to be recoded to enforce overzealous type safety.
//
// NOTE: A novel behavior of oRef is that you can pass its address to a function
// and it will release any prior refcount before proceeding. This is used in the
// Microsoft-style factory pattern bool CreateMyObject(MyObject** ppObject). In
// this style, we can pass the address of an oRef and it will free any prior 
// value and allow the internal code to assign a new MyObject that has a refcount
// of 1 on construction. To really get the address of the underlying pointer,
// use oRef::address().
//
// NOTE: Threading falls out naturally as part of the intrusive model, implementations
// of intrusive_ptr_add_ref/intrusive_ptr_release should take threadsafe pointers if
// oRefs of threadsafe pointers are to be used. For example if oRef<threadsafe foo> SmartFoo
// is declared the refcounting functions should take threadsafe pointers.  If the only
// thing declared is oRef<SmartFoo> then the refcounting functions should take
// raw pointers, oRef itself is always threadsafe.
// 
// NOTE: It is often desirable to skip stepping into these boilerplate 
// operations in debugging. To do this in MSVC9, open regedit.exe and navigate to
// Win32 HKEY_LOCAL_MACHINE\Software\Microsoft\VisualStudio\9.0\NativeDE\StepOver
// Win64 HKEY_LOCAL_MACHINE\Software\Wow6423Node\Microsoft\VisualStudio\9.0\NativeDE\StepOver
// There, add regular expressions to match oRef (and any other code you might 
// want to step over).
// So add a new String value named "step over oRef" with value "oRef.*". To skip
// over std::vector code add "step over std::vector" with value "std\:\:vector".
// More here: http://blogs.msdn.com/b/andypennell/archive/2004/02/06/69004.aspx

#pragma once
#ifndef oRef_h
#define oRef_h

#include <oBasis/oThreadsafe.h>

template<class T> struct oRef
{
	oRef() : _p(0) {}
	~oRef() { if (_p) intrusive_ptr_release(_p); _p = nullptr; }

	oRef(const oRef<T>& rhs)
	{
		_p = const_cast<T*>(rhs.c_ptr());
		if (_p) intrusive_ptr_add_ref(_p);
	}

	// Specify ref = false if initializing from a factory call that
	// returns a ref of 1.
	oRef(const T* rhs, bool ref = true)
	{
		_p = static_cast<T*>(const_cast<T*>(rhs));
		if (_p && ref) intrusive_ptr_add_ref(_p);
	}

	oRef<T>& operator=(T* rhs) threadsafe
	{
		if(rhs) intrusive_ptr_add_ref(rhs);
		if(_p) intrusive_ptr_release(_p);
		_p = rhs;
		return thread_cast<oRef<T>&>(*this);
	}
	
	oRef<T>& operator=(const oRef<T>& rhs) threadsafe
	{
		// const_cast is ok because we're casting away the constness of the oRef and not the underlying type
		if(rhs) intrusive_ptr_add_ref(const_cast<T*>(rhs.c_ptr()));
		if(_p) intrusive_ptr_release(_p);
		_p = const_cast<T*>(rhs.c_ptr()); 
		return thread_cast<oRef<T>&>(*this);
	}

	T* operator->() threadsafe { return _p; }
	const T* operator->() const threadsafe { return _p; }
	
	operator T*() threadsafe { return _p; }
	operator const T*() const threadsafe { return _p; }

	T** operator &() threadsafe { return thread_cast<T**>(&_p); } 
	
	T* c_ptr() threadsafe { return _p; }
	const T* c_ptr() const threadsafe { return _p; }

private:
	T* _p;
};

#endif
