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

// A simple intrusive smart pointer implementation. This requires the user to 
// define the following unqualified API:
// void intrusive_ptr_add_ref(T* p);
// void intrusive_ptr_release(T* p);
// (p) // (as in if (p != 0) {...})
//
// oRef was coded instead of using boost::intrusive_ptr because I feel that ref-
// counted pointers should be castable to raw pointers. Ref counting is a sign 
// of ownership. To me (a console video game developer primarily) allowing 
// objects to hang around if someone refers to them shouldn't be arbitrary, and 
// thus I would like to see care with the usage of smart pointers. If a function 
// or class does not own the pointer's contents, then there's no reason for it 
// to have a smart pointer in its API. boost::intrusive_ptr requires an explicit 
// call to .get() which clutters up the code and is a nusance. Notice as well 
// how much less code is required for the smart pointer because so many operators 
// and operations do not have to be recoded to enforce overzealous type safety.
//
// NOTE: A novel behavior of oRef is that you can pass its address to a function
// to receive a value. This is used in the Microsoft-style factory pattern 
// bool CreateMyObject(MyObject** ppObject). In this style, we can pass the 
// address of an oRef to receive the new object, but doing so will not release 
// any prior value in the oRef since this circumvents reference counting since
// most objects are constructed with a refcount of 1.
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

#include <oBasis/oPlatformFeatures.h>
#include <oBasis/oThreadsafe.h>

template<class T> struct oRef
{
	oRef() : Pointer(0) {}
	~oRef() { if (Pointer) intrusive_ptr_release(Pointer); Pointer = nullptr; }

	oRef(const oRef<T>& _That)
	{
		Pointer = const_cast<T*>(_That.c_ptr());
		if (Pointer) intrusive_ptr_add_ref(Pointer);
	}

	// Specify ref = false if initializing from a factory call that
	// returns a ref of 1.
	oRef(const T* _That, bool ref = true)
	{
		Pointer = static_cast<T*>(const_cast<T*>(_That));
		if (Pointer && ref) intrusive_ptr_add_ref(Pointer);
	}

	oRef<T>& operator=(T* _That) threadsafe
	{
		if (_That) intrusive_ptr_add_ref(_That);
		if (Pointer) intrusive_ptr_release(Pointer);
		Pointer = _That;
		return thread_cast<oRef<T>&>(*this);
	}
	
	oRef<T>& operator=(const oRef<T>& _That) threadsafe
	{
		// const_cast is ok because we're casting away the constness of the oRef and not the underlying type
		if (_That) intrusive_ptr_add_ref(const_cast<T*>(_That.c_ptr()));
		if (Pointer) intrusive_ptr_release(Pointer);
		Pointer = const_cast<T*>(_That.c_ptr()); 
		return thread_cast<oRef<T>&>(*this);
	}

	#ifdef oHAS_MOVE_CTOR
		oRef(oRef<T>&& _That)
			: Pointer(_That.Pointer)
		{
			_That.Pointer = nullptr;
		}

		oRef<T>& operator=(oRef<T>&& _That) threadsafe
		{
			if (Pointer != _That.Pointer)
			{
				if (Pointer) intrusive_ptr_release(Pointer);
				Pointer = _That.Pointer;
				_That.Pointer = nullptr;
			}
			return thread_cast<oRef<T>&>(*this);
		}
	#endif

	T* operator->() threadsafe { return Pointer; }
	const T* operator->() const threadsafe { return Pointer; }
	
	operator T*() threadsafe { return Pointer; }
	operator const T*() const threadsafe { return Pointer; }

	// This version of the operator should only be used for "create" style 
	// functions, and functions that "retrieve" oRef's i.e. QueryInterface. It is 
	// inherently not threadsafe during create until after the create returns. It 
	// is not threadsafe for retrieves either. Since the callee is going to 
	// increment the refcount and this operator does not decrement the refcount 
	// of any existing pointers, if two threads tried to make a call to 
	// QueryInterface with the same oRef, there would be a race condition and the 
	// possibility for a leak.
	T** operator &() { return &Pointer; } 

	// The const variant only makes sense for "retrieve" style calls, but 
	// otherwise see comments above.
	const T** operator &() const { return const_cast<const T**>(&Pointer); } 
	
	T* c_ptr() threadsafe { return Pointer; }
	const T* c_ptr() const threadsafe { return Pointer; }

private:
	T* Pointer;
};

#endif
