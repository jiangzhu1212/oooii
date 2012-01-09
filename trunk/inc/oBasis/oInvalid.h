// $(header)
// A nothing class designed to control the difference between setting an integer
// type to all 0xff's based on type. Mainly this should be used as an initializer
// for unsigned int types or counting types.
#pragma once
#ifndef oInvalid_h
#define oInvalid_h

class oInvalid_t
{
public:
	operator int() const { return -1; }
	operator unsigned int() const { return ~0u; }
	operator unsigned long() const { return ~0u; }
	operator unsigned long long() const { return ~0ull; }
};

const oInvalid_t oInvalid;

template<typename T> bool operator==(const oInvalid_t& _Invalid, const T& _Value) { return (T)_Invalid == _Value; }
template<typename T> bool operator==(const T& _Value, const oInvalid_t& _Invalid) { return _Invalid == _Value; }

template<typename T> bool operator!=(const oInvalid_t& _Invalid, const T& _Value) { return !((T)_Invalid == _Value); }
template<typename T> bool operator!=(const T& _Value, const oInvalid_t& _Invalid) { return !((T)_Invalid == _Value); }

#endif
