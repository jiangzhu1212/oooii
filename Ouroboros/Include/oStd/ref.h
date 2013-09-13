/**************************************************************************
 * The MIT License                                                        *
 * Copyright (c) 2013 Antony Arciuolo.                                    *
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
// A simple intrusive smart pointer implementation. This requires the user to 
// define the following unqualified API:
// void intrusive_ptr_add_ref(T* p);
// void intrusive_ptr_release(T* p);
// (p) // (as in if (p != 0) {...})
//
// IDIOM: A novel behavior of ref is that you can pass its address to a function
// to receive a value. This is used in the Microsoft-style factory pattern 
// bool CreateMyObject(MyObject** ppObject). In this style the address of a ref 
// to receive the new object is passed. Doing so will not release any prior 
// value in the ref since this circumvents reference counting since the majority 
// of the time this is done as an initialization step, but in the rare case 
// where a ref is recycled, explicitly set it to nullptr before reuse.
//
// NOTE: It is often desirable to skip stepping into these boilerplate 
// operations in debugging. To do this in MSVC9 open regedit.exe and navigate to
// Win32 HKEY_LOCAL_MACHINE\Software\Microsoft\VisualStudio\9.0\NativeDE\StepOver
// Win64 HKEY_LOCAL_MACHINE\Software\Wow6423Node\Microsoft\VisualStudio\9.0\NativeDE\StepOver
// There, add regular expressions to match ref (and any other code you might 
// want to step over).
// So add a new String value named "step over ref" with value "ref.*". To skip
// over std::vector code add "step over std::vector" with value "std\:\:vector".
// More here: http://blogs.msdn.com/b/andypennell/archive/2004/02/06/69004.aspx

#pragma once
#ifndef oStd_ref_h
#define oStd_ref_h

namespace oStd {

template<class T> struct ref
{
	ref() : Pointer(0) {}
	~ref() { if (Pointer) intrusive_ptr_release(Pointer); Pointer = nullptr; }

	ref(const ref<T>& _That)
	{
		Pointer = const_cast<T*>(_That.c_ptr());
		if (Pointer) intrusive_ptr_add_ref(Pointer);
	}

	// Specify ref = false if initializing from a factory call that
	// returns a ref of 1.
	ref(const T* _That, bool ref = true)
	{
		Pointer = static_cast<T*>(const_cast<T*>(_That));
		if (Pointer && ref) intrusive_ptr_add_ref(Pointer);
	}

	ref<T>& operator=(T* _That) volatile
	{
		if (_That) intrusive_ptr_add_ref(_That);
		if (Pointer) intrusive_ptr_release(Pointer);
		Pointer = _That;
		return const_cast<ref<T>&>(*this);
	}
	
	ref<T>& operator=(const ref<T>& _That) volatile
	{
		// const_cast is ok because we're casting away the constness of the ref and 
		// not the underlying type
		if (_That) intrusive_ptr_add_ref(const_cast<T*>(_That.c_ptr()));
		if (Pointer) intrusive_ptr_release(Pointer);
		Pointer = const_cast<T*>(_That.c_ptr());
		return const_cast<ref<T>&>(*this);
	}

	ref(ref<T>&& _That) : Pointer(_That.Pointer) { _That.Pointer = nullptr; }

	ref<T>& operator=(ref<T>&& _That) volatile
	{
		if (Pointer != _That.Pointer)
		{
			if (Pointer) intrusive_ptr_release(const_cast<T*>(Pointer));
			Pointer = _That.Pointer;
			_That.Pointer = nullptr;
		}
		return const_cast<ref<T>&>(*this);
	}

	T* operator->() { return Pointer; }
	const T* operator->() const { return Pointer; }
	
	operator T*() { return Pointer; }
	operator const T*() const { return Pointer; }

	T* operator->() volatile { return Pointer; }
	const T* operator->() const volatile { return Pointer; }
	
	operator T*() volatile { return Pointer; }
	operator const T*() const volatile { return Pointer; }
	// This version of the operator should only be used for "create" style 
	// functions, and functions that "retrieve" ref's i.e. QueryInterface.
	T** operator &() { return &Pointer; } 

	// The const variant only makes sense for "retrieve" style calls, but 
	// otherwise see comments above.
	const T** operator &() const { return const_cast<const T**>(&Pointer); } 
	
	T* c_ptr() { return Pointer; }
	const T* c_ptr() const { return Pointer; }

	T* c_ptr() volatile { return Pointer; }
	const T* c_ptr() const volatile { return Pointer; }

private:
	T* Pointer;
};

} // namespace oStd

#endif
