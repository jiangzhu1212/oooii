// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.

// A simple class that calls a std::function when the current scope ends to 
// support exception-safe programming patterns.

#pragma once
#include <oBase/callable.h>

namespace ouro {

class finally
{
public:
	finally() {}
	explicit finally(std::function<void()>&& _Callable) : OnScopeExit(std::move(_Callable)) {}

	#ifndef oHAS_VARIADIC_TEMPLATES
		#define oFINALLY_CTOR(_nArgs) oCALLABLE_CONCAT(oCALLABLE_TEMPLATE, _nArgs) \
			explicit finally(oCALLABLE_CONCAT(oCALLABLE_PARAMS, _nArgs)) \
				{ OnScopeExit = std::move(oCALLABLE_CONCAT(oCALLABLE_BIND, _nArgs)); }
		oCALLABLE_PROPAGATE(oFINALLY_CTOR);
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
	// doubly called on scope exit (including if the function itself throws).
	void operator()()
	{
		OnScopeExit();
		OnScopeExit = nullptr;
	}

	operator bool() const { return !!OnScopeExit; }

private:
	std::function<void()> OnScopeExit;

	finally(const finally&); /* = delete */
	const finally& operator=(const finally&); /* = delete */
};

}
