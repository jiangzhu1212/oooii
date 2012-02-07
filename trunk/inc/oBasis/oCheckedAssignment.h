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

// Macros used to ensure validity when assigning one type to another. This is
// intended to handle assigning integer types (signed/unsigned, char short int 
// long int64, etc.) to and from each other with a safety check. At this time 
// these macros are used by oSize and oIndex.
#pragma once
#ifndef oCheckedAssignment_h
#define oCheckedAssignment_h

#include <oBasis/oAssert.h>
#include <oBasis/oLimits.h>
#include <typeinfo>
#include <type_traits>

#define oDEFINE_RANGE_CHECKS(_ClassName, _TemplateTypeVariable, _InternalValueVariable, _MaxValidValue) \
	template<typename U> inline void CheckMax(const U& _Value, const char* _Op) const { oASSERT(_Value >= 0 && ((std::common_type<_TemplateTypeVariable,U>::type)_Value) <= ((std::common_type<_TemplateTypeVariable,U>::type)_MaxValidValue), "Specified %s (%I64u) is too large for " #_ClassName "<%s>", _Op, (unsigned long long)_Value, typeid(_TemplateTypeVariable).name()); } \
	template<typename U> inline void CheckThisMax() const { oASSERT(_InternalValueVariable <= static_cast<const _TemplateTypeVariable>(oNumericLimits<U>::GetMax()), #_ClassName "<%s> size (%I64u) is to large to be converted to type %s", typeid(_TemplateTypeVariable).name(), static_cast<unsigned long long>(_InternalValueVariable), typeid(U).name()); } \
	template<typename U> inline void CheckIsSmaller(const U& _Value) const { oASSERT(_Value <= _InternalValueVariable, "Specified size (%I64u) is too large for current " #_ClassName "<%s> value (%I64u)", (unsigned long long)_Value, typeid(_TemplateTypeVariable).name(), static_cast<unsigned long long>(_InternalValueVariable)); } \
	template<typename U> inline void CheckIsNonzero(const U& _Value) const { oASSERT(!!_Value, "Specified size must be nonzero"); } \
	template<typename U> inline void CheckByteIndex(const U& _Value) const { oASSERT(sizeof(_TemplateTypeVariable) == sizeof(U), "The specified conversion to a %s (size %u bytes) does not match the byte size of " #_ClassName "<%s> (%u bytes)", typeid(U).name(), sizeof(U), typeid(_TemplateTypeVariable).name(), sizeof(_TemplateTypeVariable)); }

#define oDEFINE_ALWAYS_VALID_CHECK(_TemplateTypeVariable) \
	template<typename U> inline void CheckValid(const U& _Value, const char* _Op) const {}

#define oDEFINE_VALIDITY_CHECK(_TemplateTypeVariable, _InvalidValue) \
	template<typename U> inline void CheckValid(const U& _Value, const char* _Op) const { oASSERT(static_cast<_TemplateTypeVariable>(_Value) != (_InvalidValue), "Specified %s with a value not in a valid state", _Op); }

#define oDEFINE_CHECKED_OPERATORS(_ClassName, _TemplateTypeVariable, _InternalValueVariable) \
	/*Translate Value is so this code can handle supporting both (Size64 + Size64) as well as (Size64 + int) type cases*/ \
	_TemplateTypeVariable TranslateValue(const _ClassName<_TemplateTypeVariable> &_Value) const {return _Value._InternalValueVariable;} \
	template<typename U> U TranslateValue(const U &_Value) const {return _Value;} \
	template<typename U> _ClassName<_TemplateTypeVariable>(const _ClassName<U>& _Other) : _InternalValueVariable(static_cast<_TemplateTypeVariable>(_Other.c_type())) { CheckValid(_Other.c_type(), "ctor"); CheckMax(_Other.c_type(), "ctor"); } \
	template<typename U> const _ClassName& operator=(const U& _Value) { CheckValid(TranslateValue(_Value), "assignment"); CheckMax(TranslateValue(_Value), "assignment"); _InternalValueVariable = static_cast<_TemplateTypeVariable>(TranslateValue(_Value)); return *this; } \
	template<typename U> const _ClassName& operator+=(const U& _Value) { CheckValid(TranslateValue(_Value), "sum"); CheckValid(_InternalValueVariable + TranslateValue(_Value), "sum"); CheckMax(_InternalValueVariable + TranslateValue(_Value), "sum"); _InternalValueVariable += TranslateValue(_Value); return *this; } \
	template<typename U> const _ClassName& operator-=(const U& _Value) { CheckValid(TranslateValue(_Value), "difference"); CheckValid(_InternalValueVariable - TranslateValue(_Value), "difference"); CheckMax(_InternalValueVariable - TranslateValue(_Value), "difference"); CheckIsSmaller(TranslateValue(_Value)); _InternalValueVariable -= TranslateValue(_Value); return *this; } \
	template<typename U> const _ClassName& operator*=(const U& _Value) { CheckValid(TranslateValue(_Value), "product"); CheckValid(_InternalValueVariable * TranslateValue(_Value), "product"); CheckMax(_InternalValueVariable * TranslateValue(_Value), "product");_InternalValueVariable *= TranslateValue(_Value); return *this; } \
	template<typename U> const _ClassName& operator/=(const U& _Value) { CheckIsNonzero(TranslateValue(_Value)); CheckValid(TranslateValue(_Value), "quotient"); CheckValid(_InternalValueVariable / TranslateValue(_Value), "quotient"); _InternalValueVariable /= TranslateValue(_Value); return *this; } \
	template<typename U> const _ClassName operator+(const U& _Value) const { CheckMax(_InternalValueVariable + TranslateValue(_Value), "sum"); CheckValid(_InternalValueVariable + TranslateValue(_Value), "sum"); return _ClassName(_InternalValueVariable + TranslateValue(_Value)); } \
	template<typename U> const _ClassName operator-(const U& _Value) const { CheckValid(TranslateValue(_Value), "difference"); CheckIsSmaller(TranslateValue(_Value)); return _ClassName(_InternalValueVariable - TranslateValue(_Value)); } \
	template<typename U> const _ClassName operator*(const U& _Value) const { CheckValid(_InternalValueVariable * TranslateValue(_Value), "product"); CheckMax(_InternalValueVariable * TranslateValue(_Value), "product"); return _ClassName(_InternalValueVariable * TranslateValue(_Value)); } \
	template<typename U> const _ClassName operator/(const U& _Value) const { CheckIsNonzero(TranslateValue(_Value)); CheckValid(_InternalValueVariable / TranslateValue(_Value), "quotient"); return _ClassName(_InternalValueVariable / TranslateValue(_Value)); } \
	template<typename U> const _ClassName operator%(const U& _Value) const { CheckIsNonzero(TranslateValue(_Value)); CheckValid(_InternalValueVariable % TranslateValue(_Value), "quotient"); return _ClassName(_InternalValueVariable % TranslateValue(_Value)); } \
	template<typename U> const _ClassName operator<<(const U& _Value) const { CheckMax(_InternalValueVariable << TranslateValue(_Value), "<<"); CheckValid(_InternalValueVariable << TranslateValue(_Value), "<<"); return _ClassName(_InternalValueVariable << TranslateValue(_Value)); } \
	template<typename U> const _ClassName operator>>(const U& _Value) const { CheckValid(_InternalValueVariable >> TranslateValue(_Value), ">>"); return _ClassName(_InternalValueVariable >> TranslateValue(_Value)); } \
	template<typename U> bool operator==(const U& _Value) const { CheckThisMax<decltype(TranslateValue(_Value))>(); CheckMax(TranslateValue(_Value), "equal"); return _InternalValueVariable == TranslateValue(_Value); } \
	template<typename U> bool operator!=(const U& _Value) const { CheckThisMax<decltype(TranslateValue(_Value))>(); CheckMax(TranslateValue(_Value), "not equal"); return _InternalValueVariable != TranslateValue(_Value); } \
	template<typename U> bool operator<(const U& _Value) const { CheckValid(TranslateValue(_Value), "less than"); CheckThisMax<decltype(TranslateValue(_Value))>(); CheckMax(TranslateValue(_Value), "less than"); return _InternalValueVariable < TranslateValue(_Value); } \
	template<typename U> bool operator<=(const U& _Value) const { CheckValid(TranslateValue(_Value), "less than or equal to");CheckThisMax<decltype(TranslateValue(_Value))>(); CheckMax(_Value, "less than or equal to"); return _InternalValueVariable <= TranslateValue(_Value); } \
	template<typename U> bool operator>(const U& _Value) const { CheckValid(TranslateValue(_Value), "greater than"); CheckThisMax<decltype(TranslateValue(_Value))>(); CheckMax(TranslateValue(_Value), "greater than"); return _InternalValueVariable > TranslateValue(_Value); } \
	template<typename U> bool operator>=(const U& _Value) const { CheckValid(TranslateValue(_Value), "greater than or equal to"); CheckThisMax<decltype(TranslateValue(_Value))>(); CheckMax(TranslateValue(_Value), "greater than or equal to"); return _InternalValueVariable >= TranslateValue(_Value); } \
	template<typename U> operator const U() const { CheckValid(_InternalValueVariable, "operator const U"); CheckThisMax<U>(); return static_cast<U>(_InternalValueVariable); } \
	template<typename U> _TemplateTypeVariable* operator&() { CheckByteSize(U); return &_InternalValueVariable; } \
	template<typename U> const _TemplateTypeVariable* operator&() const { CheckByteSize(U); return &_InternalValueVariable; } \
	_ClassName operator++() { CheckValid(_InternalValueVariable + 1, "pre-increment"); *this += 1; return *this; } \
	_ClassName operator++(int) { CheckValid(_InternalValueVariable + 1, "post-increment"); _ClassName V = _ClassName(_InternalValueVariable); *this += 1; return V; } \
	_ClassName operator--() { CheckValid(_InternalValueVariable - 1, "pre-decrement"); *this -= 1; return *this; } \
	_ClassName operator--(int) { CheckValid(_InternalValueVariable - 1, "post-decrement"); _ClassName V = _ClassName(_InternalValueVariable); *this -= 1; return V; } \
	inline _TemplateTypeVariable c_type() const { return _InternalValueVariable; } 
	

#endif
