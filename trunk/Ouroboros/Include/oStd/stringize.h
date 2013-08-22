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
// Is there ever a time when easily converting to and from string for simple and
// complex types is not needed in code development? The reason here that 
// cin/cout is not used is to avoid std::string APIs that might end up in DLLs
// as well as there still remains that locale support in cin/cout impedes 
// performance by about 10x, which is not really acceptable.
#pragma once
#ifndef oStd_stringize_h
#define oStd_stringize_h

#include <oStd/string.h>
#include <vector>

namespace oStd {

// Returns _StrDestination, or nullptr if there is a failure.
template<typename T> char* to_string(char* _StrDestination, size_t _SizeofStrDestination, const T& _Value);
template<typename T, size_t size> char* to_string(char (&_StrDestination)[size], const T& _Value) { return to_string<T>(_StrDestination, size, _Value); }

// Fills the specified address with the source string interpretted as that type.
// Returns true if _pValue is valid, or false if _pValue should not be used.
template<typename T> bool from_string(T* _pValue, const char* _StrSource);

// Permutation of from_string for string-to-string (noop/copy) conversion
bool from_string(char* _StrDestination, size_t _SizeofStrDestination, const char* _StrSource);
template<size_t size> bool from_string(char (&_StrDestination)[size], const char* _StrSource) { return from_string(_StrDestination, size, _StrSource); }

// Returns a const string representation of the specified value. This is most
// useful for enums when the object's value never changes.
template<typename T> const char* as_string(const T& _Value);


// Container/array support

namespace detail {

	template<typename ContainerT> char* to_string_container(char* _StrDestination, size_t _SizeofStrDestination, const ContainerT& _Container)
	{
		*_StrDestination = 0;
		typename ContainerT::const_iterator itLast = std::end(_Container) - 1;
		for (typename ContainerT::const_iterator it = std::begin(_Container); it != std::end(_Container); ++it)
		{
			if (!to_string(_StrDestination, _SizeofStrDestination, *it))
				return nullptr;

			size_t len = strlcat(_StrDestination, ",", _SizeofStrDestination);
			if (it != itLast && len >= _SizeofStrDestination)
				return nullptr;
			_StrDestination += len;
			_SizeofStrDestination -= len;
		}
		return _StrDestination;
	}

	template<typename ContainerT> bool from_string_container(ContainerT* _pContainer, const char* _StrSource)
	{
		char* ctx = nullptr;
		const char* tok = oStd::strtok(_StrSource, ",", &ctx);
		while (tok)
		{
			ContainerT::value_type obj;
			tok += strspn(tok, oWHITESPACE);
			if (!oStd::from_string(&obj, tok) || _pContainer->size() == _pContainer->max_size())
			{
				oStd::end_strtok(&ctx);
				return false;
			}
			_pContainer->push_back(obj);
			tok = oStd::strtok(nullptr, ",", &ctx);
		}
		return true;
	}

} // namespace detail

template <typename T, typename AllocatorT> char* to_string(char* _StrDestination, size_t _SizeofStrDestination, const std::vector<T, AllocatorT>& _Vector) { return detail::to_string_container(_StrDestination, _SizeofStrDestination, _Vector); }
template <typename T, typename AllocatorT> bool from_string(std::vector<T, AllocatorT>* _pValue, const char* _StrSource) { return detail::from_string_container(_pValue, _StrSource); }

// Utility function that will convert a string of floats separated by whitespace
// into the specified array.
bool from_string_float_array(float* _pValue, size_t _NumValues, const char* _StrSource);
bool from_string_double_array(double* _pValue, size_t _NumValues, const char* _StrSource);

} // namespace oStd

#endif
