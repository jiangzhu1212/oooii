// $(header)
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
