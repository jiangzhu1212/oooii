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
// Simplify classes that behave a lot like intrinsic types by simplifying the 
// implementation requirements. This exposes macros for the cases where 
// inheriting from these mixins would cause a simple struct to lose its non-
// aggregate status.
#pragma once
#ifndef oStd_operators_h
#define oStd_operators_h

#include <oStd/macros.h>

// If the user type implements:
//bool operator<(const T& x) const;
//bool operator==(const T& x) const;
//T& operator+=(const T& x);
//T& operator-=(const T& x);
//T& operator*=(const T& x);
//T& operator/=(const T& x);
//T& operator%=(const T& x);
//T& operator|=(const T& x);
//T& operator&=(const T& x);
//T& operator^=(const T& x);
//T& operator++();
//T& operator--();
// Then derive from operators the following operators will be automatically 
// available:
// !=, >=, <=, >, ++(int), --(int), +, -, /, *, %, |, &, ^

// Support heterogeneous operators
#define oOPERATORS_NEQ2(_ThisT, _ThatT) friend bool operator!=(const _ThisT& _This, const _ThatT& _That) { return !(_This == _That); }
#define oOPERATORS_GEQ2(_ThisT, _ThatT) friend bool operator>=(const _ThisT& _This, const _ThatT& _That) { return !(_This < _That); }
#define oOPERATORS_LEQ2(_ThisT, _ThatT) friend bool operator<=(const _ThisT& _This, const _ThatT& _That) { return (_This < _That) || (_This == _That); }
#define oOPERATORS_GTH2(_ThisT, _ThatT) friend bool operator>(const _ThisT& _This, const _ThatT& _That) { return !(_This <= _That); }
#define oOPERATORS_DERIVED2(_ThisT, _ThatT, _Op) friend _ThisT operator _Op(const _ThisT& _This, const _ThatT& _That) { _ThisT z(_This); z _Op##= _That; return z; }
#define oOPERATORS_DERIVED_REVERSE2(_ThisT, _ThatT, _Op) friend _ThisT operator _Op(const _ThatT& _That, const _ThisT& _This) { _ThisT z(_This); z _Op##= _That; return z; }
#define oOPERATORS_DERIVED_COMMUTATIVE2(_ThisT, _ThatT, _Op) oOPERATORS_DERIVED2(_ThisT, _ThatT, _Op) oOPERATORS_DERIVED_REVERSE2(_ThisT, _ThatT, _Op)

// Simplify heterogeneous operators for the common case
#define oOPERATORS_NEQ(_Type) oOPERATORS_NEQ2(_Type, _Type)
#define oOPERATORS_GEQ(_Type) oOPERATORS_GEQ2(_Type, _Type)
#define oOPERATORS_LEQ(_Type) oOPERATORS_LEQ2(_Type, _Type)
#define oOPERATORS_GTH(_Type) oOPERATORS_GTH2(_Type, _Type)
#define oOPERATORS_DERIVED(_Type, _Op) oOPERATORS_DERIVED2(_Type, _Type, _Op)
#define oOPERATORS_POSTINC(_Type) friend _Type operator++(_Type& _This, int) { _Type x(_This); ++_This; return x; }
#define oOPERATORS_POSTDEC(_Type) friend _Type operator--(_Type& _This, int) { _Type x(_This); --_This; return x; }

// Combine types into heterogeneous classifications
#define oOPERATORS_COMPARABLE2(_ThisT, _ThatT) oOPERATORS_NEQ2(_ThisT, _ThatT) oOPERATORS_GEQ2(_ThisT, _ThatT) oOPERATORS_LEQ2(_ThisT, _ThatT) oOPERATORS_GTH2(_ThisT, _ThatT)
#define oOPERATORS_ARITHMETIC2(_ThisT, _ThatT) oOPERATORS_DERIVED2(_ThisT, _ThatT, +) oOPERATORS_DERIVED2(_ThisT, _ThatT, -) oOPERATORS_DERIVED2(_ThisT, _ThatT, /) oOPERATORS_DERIVED2(_ThisT, _ThatT, *) oOPERATORS_DERIVED2(_ThisT, _ThatT, %)
#define oOPERATORS_LOGICAL2(_ThisT, _ThatT) oOPERATORS_DERIVED2(_ThisT, _ThatT, |) oOPERATORS_DERIVED2(_ThisT, _ThatT, &) oOPERATORS_DERIVED2(_ThisT, _ThatT, ^)
#define oOPERATORS_SHIFT2(_ThisT, _ThatT) oOPERATORS_DERIVED2(_ThisT, _ThatT, <<) oOPERATORS_DERIVED2(_ThisT, _ThatT, >>)
#define oOPERATORS_ALL2(_ThisT, _ThatT) oOPERATORS_COMPARABLE2(_ThisT, _ThatT) oOPERATORS_INCREMENTABLE(_ThisT) oOPERATORS_ARITHMETIC2(_ThisT, _ThatT) oOPERATORS_LOGICAL2(_ThisT, _ThatT) oOPERATORS_SHIFT2(_ThisT, _ThatT)

// Simplify heterogeneous classifications for the common case
#define oOPERATORS_COMPARABLE(_Type) oOPERATORS_COMPARABLE2(_Type, _Type)
#define oOPERATORS_INCREMENTABLE(_Type) oOPERATORS_POSTINC(_Type) oOPERATORS_POSTDEC(_Type)
#define oOPERATORS_ARITHMETIC(_Type) oOPERATORS_ARITHMETIC2(_Type, _Type)
#define oOPERATORS_LOGICAL(_Type) oOPERATORS_LOGICAL2(_Type, _Type)
#define oOPERATORS_SHIFT(_Type) oOPERATORS_SHIFT2(_Type, _Type)
#define oOPERATORS_ALL(_Type) oOPERATORS_ALL2(_Type, _Type)

// Template form. Macros are actually an extension to support oOperators in 
// structs that cannot inherit from these mixins because it would change the 
// struct's non-aggregate status (happens with multiple inheritance).
#define oOPERATORS_BROADCAST(_OpMacro) _OpMacro(ThisT) oCONCAT(_OpMacro,2)(ThisT, A) oCONCAT(_OpMacro,2)(ThisT, B) oCONCAT(_OpMacro,2)(ThisT, C) oCONCAT(_OpMacro,2)(ThisT, D)
struct oOperatorsNullA; struct oOperatorsNullB; struct oOperatorsNullC; struct oOperatorsNullD;
template<typename ThisT> struct oIncrementable { oOPERATORS_INCREMENTABLE(ThisT) };
template<typename ThisT, typename A = oOperatorsNullA, typename B = oOperatorsNullB, typename C = oOperatorsNullC, typename D = oOperatorsNullD> struct oComparable { oOPERATORS_BROADCAST(oOPERATORS_COMPARABLE) };
template<typename ThisT, typename A = oOperatorsNullA, typename B = oOperatorsNullB, typename C = oOperatorsNullC, typename D = oOperatorsNullD> struct oArithmetic { oOPERATORS_BROADCAST(oOPERATORS_ARITHMETIC) };
template<typename ThisT, typename A = oOperatorsNullA, typename B = oOperatorsNullB, typename C = oOperatorsNullC, typename D = oOperatorsNullD> struct oLogical { oOPERATORS_BROADCAST(oOPERATORS_LOGICAL) };
template<typename ThisT, typename A = oOperatorsNullA, typename B = oOperatorsNullB, typename C = oOperatorsNullC, typename D = oOperatorsNullD> struct oOperators { oOPERATORS_BROADCAST(oOPERATORS_COMPARABLE) oOPERATORS_INCREMENTABLE(ThisT) oOPERATORS_BROADCAST(oOPERATORS_ARITHMETIC) oOPERATORS_BROADCAST(oOPERATORS_LOGICAL) };

#endif
