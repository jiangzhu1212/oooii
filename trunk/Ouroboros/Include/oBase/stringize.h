/**************************************************************************
 * The MIT License                                                        *
 * Copyright (c) 2014 Antony Arciuolo.                                    *
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
#ifndef oBase_stringize_h
#define oBase_stringize_h

#include <oBase/enum_iterator.h>
#include <oBase/string.h>
#include <vector>

#define oDEFINE_TO_STRING(_T) char* to_string(char* _StrDestination, size_t _SizeofStrDestination, const _T& value) { return detail::to_string(_StrDestination, _SizeofStrDestination, value); }
#define oDEFINE_FROM_STRING(_T, _NumTs) bool from_string(_T* _pValue, const char* _StrSource) { return detail::from_string<_T, _NumTs>(_pValue, _StrSource); }
#define oDEFINE_FROM_STRING_ENUM_CLASS(_T) bool from_string(_T* _pValue, const char* _StrSource) { return detail::from_string_enum<_T>(_pValue, _StrSource); }
#define oDEFINE_FROM_STRING2(_T, _NumTs, _InvalidValue) bool from_string(_T* _pValue, const char* _StrSource) { return detail::from_string<_T, _NumTs>(_pValue, _StrSource, _InvalidValue); }
#define oDEFINE_TO_FROM_STRING(_T, _NumTs) oDEFINE_TO_STRING(_T) oDEFINE_FROM_STRING(_T, _NumTs)

#define oDEFINE_RESIZED_AS_STRING(_T) \
	const char* as_string(const resized_type<_T, unsigned short>& _Type) { return as_string((_T)_Type); } \
	const char* as_string(const resized_type<_T, short>& _Type) { return as_string((_T)_Type); } \
	const char* as_string(const resized_type<_T, unsigned char>& _Type) { return as_string((_T)_Type); } \
	const char* as_string(const resized_type<_T, char>& _Type) { return as_string((_T)_Type); }

#define oDEFINE_RESIZED_TO_STRING(_T) \
	char* to_string(char* _StrDestination, size_t _SizeofStrDestination, const resized_type<_T, unsigned short>& _Type) { return to_string(_StrDestination, _SizeofStrDestination, (_T)_Type); } \
	char* to_string(char* _StrDestination, size_t _SizeofStrDestination, const resized_type<_T, short>& _Type) { return to_string(_StrDestination, _SizeofStrDestination, (_T)_Type); } \
	char* to_string(char* _StrDestination, size_t _SizeofStrDestination, const resized_type<_T, unsigned char>& _Type) { return to_string(_StrDestination, _SizeofStrDestination, (_T)_Type); } \
	char* to_string(char* _StrDestination, size_t _SizeofStrDestination, const resized_type<_T, char>& _Type) { return to_string(_StrDestination, _SizeofStrDestination, (_T)_Type); }

#define oDEFINE_RESIZED_FROM_STRING(_T) \
	bool from_string(resized_type<_T, unsigned short>* _pType, const char* _StrSource) { _T t; bool result = from_string(&t, _StrSource); *_pType = t; return result; } \
	bool from_string(resized_type<_T, short>* _pType, const char* _StrSource) { _T t; bool result = from_string(&t, _StrSource); *_pType = t; return result; } \
	bool from_string(resized_type<_T, unsigned char>* _pType, const char* _StrSource) { _T t; bool result = from_string(&t, _StrSource); *_pType = t; return result; } \
	bool from_string(resized_type<_T, char>* _pType, const char* _StrSource) { _T t; bool result = from_string(&t, _StrSource); *_pType = t; return result; }

namespace ouro {

// Returns a const string representation of the specified value. This is most
// useful for enums when the object's value never changes.
template<typename T> const char* as_string(const T& value);

// Returns _StrDestination, or nullptr if there is a failure.
template<typename T> char* to_string(char* _StrDestination, size_t _SizeofStrDestination, const T& value);
template<typename T, size_t size> char* to_string(char (&_StrDestination)[size], const T& value) { return to_string<T>(_StrDestination, size, value); }

// Fills the specified address with the source string interpretted as that type.
// Returns true if _pValue is valid, or false if _pValue should not be used.
template<typename T> bool from_string(T* _pValue, const char* _StrSource);

// Permutation of from_string for string-to-string (noop/copy) conversion
bool from_string(char* _StrDestination, size_t _SizeofStrDestination, const char* _StrSource);
template<size_t size> bool from_string(char (&_StrDestination)[size], const char* _StrSource) { return from_string(_StrDestination, size, _StrSource); }

namespace detail {

	// A default implementation that copies the as_string to the destination
	template<typename T> char* to_string(char* _StrDestination, size_t _SizeofStrDestination, const T& value) { return strlcpy(_StrDestination, as_string(value), _SizeofStrDestination) < _SizeofStrDestination ? _StrDestination : nullptr; }

	// A default implementation that assumes the enum is by-index and iterates  
	// each into as_string to compare against.
	template<typename T, int NumTs> bool from_string(T* _pValue, const char* _StrSource, const T& _InvalidValue = T(0))
	{	
		*_pValue = _InvalidValue;
		for (int i = 0; i < NumTs; i++)
		{
			if (!_stricmp(_StrSource, as_string(T(i))))
			{
				*_pValue = T(i);
				return true;
			}
		}
		return false;
	}

	// todo: unify this with the above impl... this is for class enum types that assumes a count member
	template<typename T> bool from_string_enum(T* _pValue, const char* _StrSource, const T& _InvalidValue = T(0))
	{
		static_assert(std::is_enum<T>::value, "not enum");
		*_pValue = _InvalidValue;
		for (const auto& e : enum_iterator<T>())
		{
			if (!_stricmp(_StrSource, as_string(e)))
			{
				*_pValue = e;
				return true;
			}
		}
		return false;
	}

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
		const char* tok = ouro::strtok(_StrSource, ",", &ctx);
		while (tok)
		{
			ContainerT::value_type obj;
			tok += strspn(tok, oWHITESPACE);
			if (!ouro::from_string(&obj, tok) || _pContainer->size() == _pContainer->max_size())
			{
				ouro::end_strtok(&ctx);
				return false;
			}
			_pContainer->push_back(obj);
			tok = ouro::strtok(nullptr, ",", &ctx);
		}
		return true;
	}

} // namespace detail


// Container/array support

template <typename T, typename AllocatorT> char* to_string(char* _StrDestination, size_t _SizeofStrDestination, const std::vector<T, AllocatorT>& _Vector) { return detail::to_string_container(_StrDestination, _SizeofStrDestination, _Vector); }
template <typename T, typename AllocatorT> bool from_string(std::vector<T, AllocatorT>* _pValue, const char* _StrSource) { return detail::from_string_container(_pValue, _StrSource); }

// Utility function that will convert a string of floats separated by whitespace
// into the specified array.
bool from_string_float_array(float* _pValue, size_t _NumValues, const char* _StrSource);
bool from_string_double_array(double* _pValue, size_t _NumValues, const char* _StrSource);

}

#endif
