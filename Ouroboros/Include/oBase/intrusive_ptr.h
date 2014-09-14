// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.

// A simple intrusive smart pointer implementation. This requires the user to 
// define the following unqualified API:
// void intrusive_ptr_add_ref(T* p);
// void intrusive_ptr_release(T* p);
// (p) // (as in if (p != 0) {...})
//
// IDIOM: A novel behavior of intrusive_ptr is that you can pass its address to a function
// to receive a value. This is used in the Microsoft-style factory pattern 
// bool CreateMyObject(MyObject** ppObject). In this style the address of a intrusive_ptr 
// to receive the new object is passed. Doing so will not release any prior 
// value in the intrusive_ptr since this circumvents reference counting since the majority 
// of the time this is done as an initialization step, but in the rare case 
// where a intrusive_ptr is recycled, explicitly set it to nullptr before reuse.
//
// NOTE: It is often desirable to skip stepping into these boilerplate 
// operations in debugging. To do this in MSVC9 open regedit.exe and navigate to
// Win32 HKEY_LOCAL_MACHINE\Software\Microsoft\VisualStudio\9.0\NativeDE\StepOver
// Win64 HKEY_LOCAL_MACHINE\Software\Wow6423Node\Microsoft\VisualStudio\9.0\NativeDE\StepOver
// There, add regular expressions to match intrusive_ptr (and any other code you might 
// want to step over).
// So add a new String value named "step over intrusive_ptr" with value "intrusive_ptr.*". To skip
// over std::vector code add "step over std::vector" with value "std\:\:vector".
// More here: http://blogs.msdn.com/b/andypennell/archive/2004/02/06/69004.aspx

#pragma once

namespace ouro {

template<class T> struct intrusive_ptr
{
	intrusive_ptr() : Pointer(0) {}
	~intrusive_ptr() { if (Pointer) intrusive_ptr_release(Pointer); Pointer = nullptr; }

	intrusive_ptr(const intrusive_ptr<T>& _That)
	{
		Pointer = const_cast<T*>(_That.c_ptr());
		if (Pointer) intrusive_ptr_add_ref(Pointer);
	}

	// Specify intrusive_ptr = false if initializing from a factory call that
	// returns a intrusive_ptr of 1.
	intrusive_ptr(const T* _That, bool intrusive_ptr = true)
	{
		Pointer = static_cast<T*>(const_cast<T*>(_That));
		if (Pointer && intrusive_ptr) intrusive_ptr_add_ref(Pointer);
	}

	intrusive_ptr<T>& operator=(T* _That) volatile
	{
		if (_That) intrusive_ptr_add_ref(_That);
		if (Pointer) intrusive_ptr_release(Pointer);
		Pointer = _That;
		return const_cast<intrusive_ptr<T>&>(*this);
	}
	
	intrusive_ptr<T>& operator=(const intrusive_ptr<T>& _That) volatile
	{
		// const_cast is ok because we're casting away the constness of the intrusive_ptr and 
		// not the underlying type
		if (_That) intrusive_ptr_add_ref(const_cast<T*>(_That.c_ptr()));
		if (Pointer) intrusive_ptr_release(Pointer);
		Pointer = const_cast<T*>(_That.c_ptr());
		return const_cast<intrusive_ptr<T>&>(*this);
	}

	intrusive_ptr(intrusive_ptr<T>&& _That) : Pointer(_That.Pointer) { _That.Pointer = nullptr; }

	intrusive_ptr<T>& operator=(intrusive_ptr<T>&& _That) volatile
	{
		if (Pointer != _That.Pointer)
		{
			if (Pointer) intrusive_ptr_release(const_cast<T*>(Pointer));
			Pointer = _That.Pointer;
			_That.Pointer = nullptr;
		}
		return const_cast<intrusive_ptr<T>&>(*this);
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
	// functions, and functions that "retrieve" intrusive_ptr's i.e. QueryInterface.
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

}
