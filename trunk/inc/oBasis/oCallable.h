// $(header)

// Pre-variadic macros we need to just type out templates with different counts
// of parameters, so here it is. This facilitates the std::thread pattern of 
// wrapping the need for std::bind inside the calls to functions that take 
// functors.
#pragma once
#ifndef oCallable_h
#define oCallable_h

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

	#define oCALLABLE_TEMPLATE0 template<typename Callable>
	#define oCALLABLE_TEMPLATE1 template<typename Callable, oARG_TYPENAMES1>
	#define oCALLABLE_TEMPLATE2 template<typename Callable, oARG_TYPENAMES2>
	#define oCALLABLE_TEMPLATE3 template<typename Callable, oARG_TYPENAMES3>
	#define oCALLABLE_TEMPLATE4 template<typename Callable, oARG_TYPENAMES4>
	#define oCALLABLE_TEMPLATE5 template<typename Callable, oARG_TYPENAMES5>
	#define oCALLABLE_TEMPLATE6 template<typename Callable, oARG_TYPENAMES6>
	#define oCALLABLE_TEMPLATE7 template<typename Callable, oARG_TYPENAMES7>
	#define oCALLABLE_TEMPLATE8 template<typename Callable, oARG_TYPENAMES8>
	#define oCALLABLE_TEMPLATE9 template<typename Callable, oARG_TYPENAMES9>
	#define oCALLABLE_TEMPLATE10 template<typename Callable, oARG_TYPENAMES10>

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
#else
	#error TODO: Implement all uses of the Callable pattern with true variadic macros.
#endif
#endif
