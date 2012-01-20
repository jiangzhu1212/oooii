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
// A simple class that calls an oFUNCTION when it goes out of scope. This is 
// useful for using an exit-early-on-failure pattern, but being able to 
// centralize cleanup code without gotos or scope worries.
#pragma once
#ifndef oOnScopeExit_h
#define oOnScopeExit_h

#include <oBasis/oCallable.h>
#include <oBasis/oFunction.h>
#include <oBasis/oPlatformFeatures.h>

class oOnScopeExit
{
public:
	#ifndef oHAS_VARIADIC_TEMPLATES
		oCALLABLE_TEMPLATE0 explicit oOnScopeExit(oCALLABLE_PARAMS0) { initialize(oCALLABLE_BIND0); }
		oCALLABLE_TEMPLATE1 explicit oOnScopeExit(oCALLABLE_PARAMS1) { initialize(oCALLABLE_BIND1); }
		oCALLABLE_TEMPLATE2 explicit oOnScopeExit(oCALLABLE_PARAMS2) { initialize(oCALLABLE_BIND2); }
		oCALLABLE_TEMPLATE3 explicit oOnScopeExit(oCALLABLE_PARAMS3) { initialize(oCALLABLE_BIND3); }
		oCALLABLE_TEMPLATE4 explicit oOnScopeExit(oCALLABLE_PARAMS4) { initialize(oCALLABLE_BIND4); }
		oCALLABLE_TEMPLATE5 explicit oOnScopeExit(oCALLABLE_PARAMS5) { initialize(oCALLABLE_BIND5); }
		oCALLABLE_TEMPLATE6 explicit oOnScopeExit(oCALLABLE_PARAMS6) { initialize(oCALLABLE_BIND6); }
		oCALLABLE_TEMPLATE7 explicit oOnScopeExit(oCALLABLE_PARAMS7) { initialize(oCALLABLE_BIND7); }
		oCALLABLE_TEMPLATE8 explicit oOnScopeExit(oCALLABLE_PARAMS8) { initialize(oCALLABLE_BIND8); }
		oCALLABLE_TEMPLATE9 explicit oOnScopeExit(oCALLABLE_PARAMS9) { initialize(oCALLABLE_BIND9); }
		oCALLABLE_TEMPLATE10 explicit oOnScopeExit(oCALLABLE_PARAMS10) { initialize(oCALLABLE_BIND10); }
	#endif

	~oOnScopeExit() { if (OnDestroy) OnDestroy(); }

private:
	oTASK OnDestroy;
	void initialize(oCALLABLE _Callable) { OnDestroy = _Callable; }
};

#endif
