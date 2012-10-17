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
// Simplify classes that behave a lot like intrinsic types by simplifying the 
// implementation requirements.
#pragma once
#ifndef oOperators_h
#define oOperators_h

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

template<typename T> struct oCompareable
{
	friend bool operator!=(const T& _This, const T& _That) { return !(_This == _That); }
	friend bool operator>=(const T& _This, const T& _That) { return !(_This < _That); }
	friend bool operator<=(const T& _This, const T& _That) { return (_This < _That) || (_This == _That); }
	friend bool operator>(const T& _This, const T& _That) { return !(_This <= _That); }
};

template<typename T> struct oIncrementable
{
	friend T& operator++(T& _This, int) { T x(_This); x++; return x; }
	friend T& operator--(T& _This, int) { T x(_This); x--; return x; }
};

#define oDERIVED_OP(_Type, _Op) friend _Type& operator _Op(const _Type& _This, const _Type& _That) { _Type z(_This); z _Op##= _That; return z; }

template<typename T> struct oArithmetic
{
	oDERIVED_OP(T, +) oDERIVED_OP(T, -) oDERIVED_OP(T, /) oDERIVED_OP(T, *) oDERIVED_OP(T, %)
};

template<typename T> struct oLogical
{
	oDERIVED_OP(T, |) oDERIVED_OP(T, &) oDERIVED_OP(T, ^)
};

template<typename T> struct oOperators : oCompareable<T>, oIncrementable<T>, oArithmetic<T>, oLogical<T>
{
};

#undef oDERIVED_OP

#endif
