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

// Pre-variadic macros we need to just type out templates with different counts
// of parameters, so here it is. This facilitates the std::thread pattern of 
// wrapping the need for std::bind inside the calls to functions that take 
// functors.
#pragma once
#ifndef oCallable_h
#define oCallable_h

#include <oBasis/oMacros.h>

// Callable pattern: a workaround for not having variadic templates. This 
// pattern seems to be emerging from C++11 APIs, so support it ahead of compiler
// support in this way.
#ifndef oHAS_VARIADIC_TEMPLATES
	#define oCALLABLE std::tr1::function<void()>

	#define oARG_TYPENAMES0
	#define oARG_TYPENAMES1 typename Arg0
	#define oARG_TYPENAMES2 typename Arg0, typename Arg1
	#define oARG_TYPENAMES3 typename Arg0, typename Arg1, typename Arg2
	#define oARG_TYPENAMES4 typename Arg0, typename Arg1, typename Arg2, typename Arg3
	#define oARG_TYPENAMES5 typename Arg0, typename Arg1, typename Arg2, typename Arg3, typename Arg4
	#define oARG_TYPENAMES6 typename Arg0, typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5
	#define oARG_TYPENAMES7 typename Arg0, typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5, typename Arg6
	#define oARG_TYPENAMES8 typename Arg0, typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5, typename Arg6, typename Arg7
	#define oARG_TYPENAMES9 typename Arg0, typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5, typename Arg6, typename Arg7, typename Arg8
	#define oARG_TYPENAMES10 typename Arg0, typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5, typename Arg6, typename Arg7, typename Arg8, typename Arg9

	#define oARG_DECL0
	#define oARG_DECL1 Arg0 Arg0__
	#define oARG_DECL2 Arg0 Arg0__, Arg1 Arg1__
	#define oARG_DECL3 Arg0 Arg0__, Arg1 Arg1__, Arg2 Arg2__
	#define oARG_DECL4 Arg0 Arg0__, Arg1 Arg1__, Arg2 Arg2__, Arg3 Arg3__
	#define oARG_DECL5 Arg0 Arg0__, Arg1 Arg1__, Arg2 Arg2__, Arg3 Arg3__, Arg4 Arg4__
	#define oARG_DECL6 Arg0 Arg0__, Arg1 Arg1__, Arg2 Arg2__, Arg3 Arg3__, Arg4 Arg4__, Arg5 Arg5__
	#define oARG_DECL7 Arg0 Arg0__, Arg1 Arg1__, Arg2 Arg2__, Arg3 Arg3__, Arg4 Arg4__, Arg5 Arg5__, Arg6 Arg6__
	#define oARG_DECL8 Arg0 Arg0__, Arg1 Arg1__, Arg2 Arg2__, Arg3 Arg3__, Arg4 Arg4__, Arg5 Arg5__, Arg6 Arg6__, Arg7 Arg7__
	#define oARG_DECL9 Arg0 Arg0__, Arg1 Arg1__, Arg2 Arg2__, Arg3 Arg3__, Arg4 Arg4__, Arg5 Arg5__, Arg6 Arg6__, Arg7 Arg7__, Arg8 Arg8__
	#define oARG_DECL10 Arg0 Arg0__, Arg1 Arg1__, Arg2 Arg2__, Arg3 Arg3__, Arg4 Arg4__, Arg5 Arg5__, Arg6 Arg6__, Arg7 Arg7__, Arg8 Arg8__, Arg9 Arg9__

	#define oARG_PASS0
	#define oARG_PASS1 Arg0__
	#define oARG_PASS2 Arg0__, Arg1__
	#define oARG_PASS3 Arg0__, Arg1__, Arg2__
	#define oARG_PASS4 Arg0__, Arg1__, Arg2__, Arg3__
	#define oARG_PASS5 Arg0__, Arg1__, Arg2__, Arg3__, Arg4__
	#define oARG_PASS6 Arg0__, Arg1__, Arg2__, Arg3__, Arg4__, Arg5__
	#define oARG_PASS7 Arg0__, Arg1__, Arg2__, Arg3__, Arg4__, Arg5__, Arg6__
	#define oARG_PASS8 Arg0__, Arg1__, Arg2__, Arg3__, Arg4__, Arg5__, Arg6__, Arg7__
	#define oARG_PASS9 Arg0__, Arg1__, Arg2__, Arg3__, Arg4__, Arg5__, Arg6__, Arg7__, Arg8__
	#define oARG_PASS10 Arg0__, Arg1__, Arg2__, Arg3__, Arg4__, Arg5__, Arg6__, Arg7__, Arg8__, Arg9__

	#define oCALLABLE_TEMPLATE0 template<oCALLABLE_ARG_TYPENAMES0>
	#define oCALLABLE_TEMPLATE1 template<oCALLABLE_ARG_TYPENAMES1>
	#define oCALLABLE_TEMPLATE2 template<oCALLABLE_ARG_TYPENAMES2>
	#define oCALLABLE_TEMPLATE3 template<oCALLABLE_ARG_TYPENAMES3>
	#define oCALLABLE_TEMPLATE4 template<oCALLABLE_ARG_TYPENAMES4>
	#define oCALLABLE_TEMPLATE5 template<oCALLABLE_ARG_TYPENAMES5>
	#define oCALLABLE_TEMPLATE6 template<oCALLABLE_ARG_TYPENAMES6>
	#define oCALLABLE_TEMPLATE7 template<oCALLABLE_ARG_TYPENAMES7>
	#define oCALLABLE_TEMPLATE8 template<oCALLABLE_ARG_TYPENAMES8>
	#define oCALLABLE_TEMPLATE9 template<oCALLABLE_ARG_TYPENAMES9>
	#define oCALLABLE_TEMPLATE10 template<oCALLABLE_ARG_TYPENAMES10>

	#define oCALLABLE_ARG_TYPENAMES0 typename Callable
	#define oCALLABLE_ARG_TYPENAMES1 typename Callable, oARG_TYPENAMES1
	#define oCALLABLE_ARG_TYPENAMES2 typename Callable, oARG_TYPENAMES2
	#define oCALLABLE_ARG_TYPENAMES3 typename Callable, oARG_TYPENAMES3
	#define oCALLABLE_ARG_TYPENAMES4 typename Callable, oARG_TYPENAMES4
	#define oCALLABLE_ARG_TYPENAMES5 typename Callable, oARG_TYPENAMES5
	#define oCALLABLE_ARG_TYPENAMES6 typename Callable, oARG_TYPENAMES6
	#define oCALLABLE_ARG_TYPENAMES7 typename Callable, oARG_TYPENAMES7
	#define oCALLABLE_ARG_TYPENAMES8 typename Callable, oARG_TYPENAMES8
	#define oCALLABLE_ARG_TYPENAMES9 typename Callable, oARG_TYPENAMES9
	#define oCALLABLE_ARG_TYPENAMES10 typename Callable, oARG_TYPENAMES10

	#define oCALLABLE_ARG_TYPENAMES_PASS0 Callable
	#define oCALLABLE_ARG_TYPENAMES_PASS1 Callable, Arg0
	#define oCALLABLE_ARG_TYPENAMES_PASS2 Callable, Arg0, Arg1
	#define oCALLABLE_ARG_TYPENAMES_PASS3 Callable, Arg0, Arg1, Arg2
	#define oCALLABLE_ARG_TYPENAMES_PASS4 Callable, Arg0, Arg1, Arg2, Arg3
	#define oCALLABLE_ARG_TYPENAMES_PASS5 Callable, Arg0, Arg1, Arg2, Arg3, Arg4
	#define oCALLABLE_ARG_TYPENAMES_PASS6 Callable, Arg0, Arg1, Arg2, Arg3, Arg4, Arg5
	#define oCALLABLE_ARG_TYPENAMES_PASS7 Callable, Arg0, Arg1, Arg2, Arg3, Arg4, Arg5, Arg6
	#define oCALLABLE_ARG_TYPENAMES_PASS8 Callable, Arg0, Arg1, Arg2, Arg3, Arg4, Arg5, Arg6, Arg7
	#define oCALLABLE_ARG_TYPENAMES_PASS9 Callable, Arg0, Arg1, Arg2, Arg3, Arg4, Arg5, Arg6, Arg7, Arg8
	#define oCALLABLE_ARG_TYPENAMES_PASS10 Callable, Arg0, Arg1, Arg2, Arg3, Arg4, Arg5, Arg6, Arg7, Arg8, Arg9

	// @oooii-tony: Change these to Callable&& Function__ to leverage move ctors
	#define oCALLABLE_PARAMS0 Callable Function__
	#define oCALLABLE_PARAMS1 Callable Function__, oARG_DECL1
	#define oCALLABLE_PARAMS2 Callable Function__, oARG_DECL2
	#define oCALLABLE_PARAMS3 Callable Function__, oARG_DECL3
	#define oCALLABLE_PARAMS4 Callable Function__, oARG_DECL4
	#define oCALLABLE_PARAMS5 Callable Function__, oARG_DECL5
	#define oCALLABLE_PARAMS6 Callable Function__, oARG_DECL6
	#define oCALLABLE_PARAMS7 Callable Function__, oARG_DECL7
	#define oCALLABLE_PARAMS8 Callable Function__, oARG_DECL8
	#define oCALLABLE_PARAMS9 Callable Function__, oARG_DECL9
	#define oCALLABLE_PARAMS10 Callable Function__, oARG_DECL10

	#define oCALLABLE_PASS0 Function__
	#define oCALLABLE_PASS1 Function__, oARG_PASS1
	#define oCALLABLE_PASS2 Function__, oARG_PASS2
	#define oCALLABLE_PASS3 Function__, oARG_PASS3
	#define oCALLABLE_PASS4 Function__, oARG_PASS4
	#define oCALLABLE_PASS5 Function__, oARG_PASS5
	#define oCALLABLE_PASS6 Function__, oARG_PASS6
	#define oCALLABLE_PASS7 Function__, oARG_PASS7
	#define oCALLABLE_PASS8 Function__, oARG_PASS8
	#define oCALLABLE_PASS9 Function__, oARG_PASS9
	#define oCALLABLE_PASS10 Function__, oARG_PASS10

	#define oCALLABLE_BIND0 std::tr1::bind(Function__)
	#define oCALLABLE_BIND1 std::tr1::bind(Function__, oARG_PASS1)
	#define oCALLABLE_BIND2 std::tr1::bind(Function__, oARG_PASS2)
	#define oCALLABLE_BIND3 std::tr1::bind(Function__, oARG_PASS3)
	#define oCALLABLE_BIND4 std::tr1::bind(Function__, oARG_PASS4)
	#define oCALLABLE_BIND5 std::tr1::bind(Function__, oARG_PASS5)
	#define oCALLABLE_BIND6 std::tr1::bind(Function__, oARG_PASS6)
	#define oCALLABLE_BIND7 std::tr1::bind(Function__, oARG_PASS7)
	#define oCALLABLE_BIND8 std::tr1::bind(Function__, oARG_PASS8)
	#define oCALLABLE_BIND9 std::tr1::bind(Function__, oARG_PASS9)
	#define oCALLABLE_BIND10 std::tr1::bind(Function__, oARG_PASS10)

	#define oCALLABLE_CALL0 Function__(oARG_PASS0)
	#define oCALLABLE_CALL1 Function__(oARG_PASS1)
	#define oCALLABLE_CALL2 Function__(oARG_PASS2)
	#define oCALLABLE_CALL3 Function__(oARG_PASS3)
	#define oCALLABLE_CALL4 Function__(oARG_PASS4)
	#define oCALLABLE_CALL5 Function__(oARG_PASS5)
	#define oCALLABLE_CALL6 Function__(oARG_PASS6)
	#define oCALLABLE_CALL7 Function__(oARG_PASS7)
	#define oCALLABLE_CALL8 Function__(oARG_PASS8)
	#define oCALLABLE_CALL9 Function__(oARG_PASS9)
	#define oCALLABLE_CALL10 Function__(oARG_PASS10)

	#define oCALLABLE_RETURN_TYPE0 typename std::result_of<Callable()>::type
	#define oCALLABLE_RETURN_TYPE1 typename std::result_of<Callable(Arg0)>::type
	#define oCALLABLE_RETURN_TYPE2 typename std::result_of<Callable(Arg0,Arg1)>::type
	#define oCALLABLE_RETURN_TYPE3 typename std::result_of<Callable(Arg0,Arg1,Arg2)>::type
	#define oCALLABLE_RETURN_TYPE4 typename std::result_of<Callable(Arg0,Arg1,Arg2,Arg3)>::type
	#define oCALLABLE_RETURN_TYPE5 typename std::result_of<Callable(Arg0,Arg1,Arg2,Arg3,Arg4)>::type
	#define oCALLABLE_RETURN_TYPE6 typename std::result_of<Callable(Arg0,Arg1,Arg2,Arg3,Arg4,Arg5)>::type
	#define oCALLABLE_RETURN_TYPE7 typename std::result_of<Callable(Arg0,Arg1,Arg2,Arg3,Arg4,Arg5,Arg6)>::type
	#define oCALLABLE_RETURN_TYPE8 typename std::result_of<Callable(Arg0,Arg1,Arg2,Arg3,Arg4,Arg5,Arg6,Arg7)>::type
	#define oCALLABLE_RETURN_TYPE9 typename std::result_of<Callable(Arg0,Arg1,Arg2,Arg3,Arg4,Arg5,Arg6,Arg7,Arg8)>::type
	#define oCALLABLE_RETURN_TYPE10 typename std::result_of<Callable(Arg0,Arg1,Arg2,Arg3,Arg4,Arg5,Arg6,Arg7,Arg8,Arg9)>::type

	#define oCALLABLE_PROPAGATE(_Macro) \
		_Macro(0) _Macro(1) _Macro(2) _Macro(3) _Macro(4) _Macro(5) _Macro(6) _Macro(7) _Macro(8) _Macro(9) _Macro(10)

	#define oDEFINE_CALLABLE_RETURN_WRAPPER(_Index, _PublicMethodReturnValue, _PublicMethod, _PublicMethodThreadsafety, _ImplementationMethod) oCONCAT(oCALLABLE_TEMPLATE, _Index) inline _PublicMethodReturnValue _PublicMethod(oCONCAT(oCALLABLE_PARAMS, _Index)) _PublicMethodThreadsafety { return _ImplementationMethod(oCONCAT(oCALLABLE_BIND, _Index)); }
	#define oDEFINE_CALLABLE_WRAPPER(_Index, _PublicMethod, _PublicMethodThreadsafety, _ImplementationMethod) oCONCAT(oCALLABLE_TEMPLATE, _Index) inline void _PublicMethod(oCONCAT(oCALLABLE_PARAMS, _Index)) _PublicMethodThreadsafety { _ImplementationMethod(oCONCAT(oCALLABLE_BIND, _Index)); }
	#define oDEFINE_CALLABLE_CTOR_WRAPPER(_Index, _PublicCtorExplicit, _PublicCtor, _ImplementationMethod) oCONCAT(oCALLABLE_TEMPLATE, _Index) _PublicCtorExplicit _PublicCtor(oCONCAT(oCALLABLE_PARAMS, _Index)) { _ImplementationMethod(oCONCAT(oCALLABLE_BIND, _Index)); }

	#define oDEFINE_CALLABLE_RETURN_WRAPPERS(_PublicMethodReturnValue, _PublicMethod, _PublicMethodThreadsafety, _ImplementationMethod) \
		oDEFINE_CALLABLE_RETURN_WRAPPER(1, _PublicMethodReturnValue, _PublicMethod, _PublicMethodThreadsafety, _ImplementationMethod) \
		oDEFINE_CALLABLE_RETURN_WRAPPER(2, _PublicMethodReturnValue, _PublicMethod, _PublicMethodThreadsafety, _ImplementationMethod) \
		oDEFINE_CALLABLE_RETURN_WRAPPER(3, _PublicMethodReturnValue, _PublicMethod, _PublicMethodThreadsafety, _ImplementationMethod) \
		oDEFINE_CALLABLE_RETURN_WRAPPER(4, _PublicMethodReturnValue, _PublicMethod, _PublicMethodThreadsafety, _ImplementationMethod) \
		oDEFINE_CALLABLE_RETURN_WRAPPER(5, _PublicMethodReturnValue, _PublicMethod, _PublicMethodThreadsafety, _ImplementationMethod) \
		oDEFINE_CALLABLE_RETURN_WRAPPER(6, _PublicMethodReturnValue, _PublicMethod, _PublicMethodThreadsafety, _ImplementationMethod) \
		oDEFINE_CALLABLE_RETURN_WRAPPER(7, _PublicMethodReturnValue, _PublicMethod, _PublicMethodThreadsafety, _ImplementationMethod) \
		oDEFINE_CALLABLE_RETURN_WRAPPER(8, _PublicMethodReturnValue, _PublicMethod, _PublicMethodThreadsafety, _ImplementationMethod) \
		oDEFINE_CALLABLE_RETURN_WRAPPER(9, _PublicMethodReturnValue, _PublicMethod, _PublicMethodThreadsafety, _ImplementationMethod) \
		oDEFINE_CALLABLE_RETURN_WRAPPER(10, _PublicMethodReturnValue, _PublicMethod, _PublicMethodThreadsafety, _ImplementationMethod)

	#define oDEFINE_CALLABLE_WRAPPERS(_PublicMethod, _PublicMethodThreadsafety, _ImplementationMethod) \
		oDEFINE_CALLABLE_WRAPPER(1, _PublicMethod, _PublicMethodThreadsafety, _ImplementationMethod) \
		oDEFINE_CALLABLE_WRAPPER(2, _PublicMethod, _PublicMethodThreadsafety, _ImplementationMethod) \
		oDEFINE_CALLABLE_WRAPPER(3, _PublicMethod, _PublicMethodThreadsafety, _ImplementationMethod) \
		oDEFINE_CALLABLE_WRAPPER(4, _PublicMethod, _PublicMethodThreadsafety, _ImplementationMethod) \
		oDEFINE_CALLABLE_WRAPPER(5, _PublicMethod, _PublicMethodThreadsafety, _ImplementationMethod) \
		oDEFINE_CALLABLE_WRAPPER(6, _PublicMethod, _PublicMethodThreadsafety, _ImplementationMethod) \
		oDEFINE_CALLABLE_WRAPPER(7, _PublicMethod, _PublicMethodThreadsafety, _ImplementationMethod) \
		oDEFINE_CALLABLE_WRAPPER(8, _PublicMethod, _PublicMethodThreadsafety, _ImplementationMethod) \
		oDEFINE_CALLABLE_WRAPPER(9, _PublicMethod, _PublicMethodThreadsafety, _ImplementationMethod) \
		oDEFINE_CALLABLE_WRAPPER(10, _PublicMethod, _PublicMethodThreadsafety, _ImplementationMethod)

	#define oDEFINE_CALLABLE_CTOR_WRAPPERS(_PublicCtorExplicit, _PublicCtor, _ImplementationMethod) \
		oDEFINE_CALLABLE_CTOR_WRAPPER(0, _PublicCtorExplicit, _PublicCtor, _ImplementationMethod) \
		oDEFINE_CALLABLE_CTOR_WRAPPER(1, _PublicCtorExplicit, _PublicCtor, _ImplementationMethod) \
		oDEFINE_CALLABLE_CTOR_WRAPPER(2, _PublicCtorExplicit, _PublicCtor, _ImplementationMethod) \
		oDEFINE_CALLABLE_CTOR_WRAPPER(3, _PublicCtorExplicit, _PublicCtor, _ImplementationMethod) \
		oDEFINE_CALLABLE_CTOR_WRAPPER(4, _PublicCtorExplicit, _PublicCtor, _ImplementationMethod) \
		oDEFINE_CALLABLE_CTOR_WRAPPER(5, _PublicCtorExplicit, _PublicCtor, _ImplementationMethod) \
		oDEFINE_CALLABLE_CTOR_WRAPPER(6, _PublicCtorExplicit, _PublicCtor, _ImplementationMethod) \
		oDEFINE_CALLABLE_CTOR_WRAPPER(7, _PublicCtorExplicit, _PublicCtor, _ImplementationMethod) \
		oDEFINE_CALLABLE_CTOR_WRAPPER(8, _PublicCtorExplicit, _PublicCtor, _ImplementationMethod) \
		oDEFINE_CALLABLE_CTOR_WRAPPER(9, _PublicCtorExplicit, _PublicCtor, _ImplementationMethod) \
		oDEFINE_CALLABLE_CTOR_WRAPPER(10, _PublicCtorExplicit, _PublicCtor, _ImplementationMethod)

#else
	#error TODO: Implement all uses of the Callable pattern with true variadic macros.
#endif
#endif