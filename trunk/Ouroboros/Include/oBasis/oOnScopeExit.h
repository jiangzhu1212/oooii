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
// A simple class that calls an oFUNCTION when it goes out of scope. This is 
// useful for using an exit-early-on-failure pattern, but being able to 
// centralize cleanup code without gotos or scope worries.
#pragma once
#ifndef oOnScopeExit_h
#define oOnScopeExit_h

#include <vector>
#include <oBasis/oCallable.h>
#include <oBasis/oFunction.h>
#include <oBasis/oNonCopyable.h>
#include <oBasis/oPlatformFeatures.h>
#include <oBasis/oAlgorithm.h>

class oOnScopeExit : oNoncopyable
{
public:
	oOnScopeExit() {}

	#ifndef oHAS_VARIADIC_TEMPLATES
		#define oONSCOPEEXIT_CTOR(_nArgs) oCONCAT(oCALLABLE_TEMPLATE, _nArgs) explicit oOnScopeExit(oCONCAT(oCALLABLE_PARAMS, _nArgs)) { PushTask(oCONCAT(oCALLABLE_BIND, _nArgs)); }
		oCALLABLE_PROPAGATE(oONSCOPEEXIT_CTOR);
	#else
		#error Add variadic template support
	#endif

	~oOnScopeExit() { oForAll(OnDestroy, [](oTASK& _task) {_task();}); }

	oOnScopeExit(oOnScopeExit&& _That) : OnDestroy(std::move(_That.OnDestroy)) {}
	oOnScopeExit& operator=(oOnScopeExit&& _That) { if (this != &_That) { OnDestroy = std::move(_That.OnDestroy); } return *this; }

	void PushTask(oCALLABLE _Callable) { OnDestroy.push_back(_Callable); }
	operator bool() const { return !OnDestroy.empty(); }

private:
	std::vector<oTASK> OnDestroy;
};

#endif
