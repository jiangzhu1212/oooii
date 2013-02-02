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
// oInitOnce allows for threadsafe read access to an object useful in scenarios 
// where an object is initialized once in the constructor of a class but 
// accessed for read via threadsafe methods. This helps reduce thread_cast 
// pollution by encapsulating an often used pattern. Const access to the 
// underlying object is achieved through -> and * just like std::iterators.

// NOTE: Do not use this in classes that might be serialized to disk because
// the topology of this class differs between debug and release builds.

#pragma once
#ifndef oInitOnce_h
#define oInitOnce_h

#include <oBasis/oThreadsafe.h>

template<typename T>
class oInitOnce
{
public:
	#ifdef _DEBUG
		oInitOnce() : Initialized(false) {}
		template<typename U> oInitOnce(const U& _Value) : Initialized(false) { Initialize(_Value); }
	#else
		oInitOnce() {}
		template<typename U> oInitOnce(const U& _Value) : Value(_Value) {}
	#endif

	// Initialize and ctor can be used on any type for which T has an assignment
	// operator.
	template<typename U> void Initialize(const U& _Value)
	{
		#ifdef _DEBUG
			oASSERT(!Initialized, "Initialize called more than once, Thread safety cannot be ensured.");
			Initialized = true;
		#endif
		Value = _Value;
	}

	template<typename U> void Initialize(U&& _Value)
	{
#ifdef _DEBUG
		oASSERT(!Initialized, "Initialize called more than once, Thread safety cannot be ensured.");
		Initialized = true;
#endif
		Value = std::move(_Value);
	}

	// For more complex types where the initialization is often a function that 
	// takes the object by-reference. Prefer the by-value initialize if a copy can
	// be reasonably made, but if not pass this to an init function, try not to
	// expose the reference in generic code and let the reference die so oInitOnce
	// can enforce single initialization.
	T& Initialize()
	{
		#ifdef _DEBUG
			oASSERT(!Initialized, "Initialize called more than once, Thread safety cannot be ensured.");
			Initialized = true;
		#endif
		return Value;
	}
	const T& operator*() const threadsafe { return thread_cast<const T&>(Value); } // oInitOnce guarantees values never change, so read-only access will be threadsafe
	const T* c_ptr() const threadsafe { return thread_cast<const T*>(&Value); } // oInitOnce guarantees values never change, so read-only access will be threadsafe
	const T* operator->() const threadsafe { return c_ptr(); } 

private:
	T Value;

	#ifdef _DEBUG
		bool Initialized;
	#endif
};

#endif
