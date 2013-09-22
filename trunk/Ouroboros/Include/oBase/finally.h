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
// A simple class that calls a std::function when the current scope ends. This 
// is useful for using an exit-early-on-failure pattern, but being able to 
// centralize cleanup code without gotos or scope worries. The C++ luminaries 
// are against this pattern because good design should leverage RAII, but how
// does RAII work with std::thread? If client code doesn't explicitly call
// join(), the app just terminates, and there's no thread_guard. So continue to
// favor RAII, but have this fallback just in case.
#pragma once
#ifndef oBase_finally_h
#define oBase_finally_h

#include <oStd/callable.h>

namespace ouro {

class finally
{
public:
	finally() {}
	explicit finally(oCALLABLE&& _Callable) : OnScopeExit(std::move(_Callable)) {}

	#ifndef oHAS_VARIADIC_TEMPLATES
		#define oSTD_FINALLY_CTOR(_nArgs) oCALLABLE_CONCAT(oCALLABLE_TEMPLATE, _nArgs) \
			explicit finally(oCALLABLE_CONCAT(oCALLABLE_PARAMS, _nArgs)) \
				{ OnScopeExit = std::move(oCALLABLE_CONCAT(oCALLABLE_BIND, _nArgs)); }
		oCALLABLE_PROPAGATE(oSTD_FINALLY_CTOR);
	#else
		#error Add variadic template support
	#endif

	~finally() { if (OnScopeExit) OnScopeExit(); }

	finally(finally&& _That) { operator=(std::move(_That)); }
	finally& operator=(finally&& _That) 
	{
		if (this != &_That) 
			OnScopeExit = std::move(_That.OnScopeExit);
		return *this;
	}

	// allow this to be explicitly called in client code in a way that it's not
	// doubly code on scope exit (including if the function itself throws).
	void operator()()
	{
		oCALLABLE c = std::move(OnScopeExit);
		c();
	}

	operator bool() const { return !!OnScopeExit; }

private:
	oCALLABLE OnScopeExit;

	finally(const finally&);
	const finally& operator=(const finally&);
};

} // namespace ouro

#endif
