// $(header)
// Policy header that defines a common template interface for converting types 
// to and from string. This can be extended by client code by defining the
// functions for a new type according to the rules documented below.
// By default intrinsic types are defined.
#pragma once
#ifndef oStringize_h
#define oStringize_h

// Returns _StrDestination if successful, or nullptr if the parameters are 
// invalid or too small.
template<typename T> char* oToString(char* _StrDestination, size_t _SizeofStrDestination, const T& _Value);
template<size_t size, typename T> char* oToString(char (&_StrDestination)[size], const T& _Value) { return oToString<T>(_StrDestination, size, _Value); }

// Returns true if successful or false of the specified string is malformed
// for the type, or if the parameters are otherwise invalid.
template<typename T> bool oFromString(T* _pValue, const char* _StrSource);

// Returns a const string representation of the specified value. This is most
// useful for enums when the object's value never changes.
template<typename T> const char* oAsString(const T& _Value);

#endif
