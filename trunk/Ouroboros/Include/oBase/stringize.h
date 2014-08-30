// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.
#pragma once
#ifndef oBase_stringize_h
#define oBase_stringize_h

// Template policy for converting data types to and from string without
// requiring std::string.

#include <oBase/enum_iterator.h>
#include <oString/string.h>
#include <vector>

#define oDEFINE_TO_STRING(_T) char* to_string(char* dst, size_t dst_size, const _T& value) { return detail::to_string(dst, dst_size, value); }
#define oDEFINE_FROM_STRING(_T) bool from_string(_T* out_value, const char* src) { return detail::from_string_enum<_T>(out_value, src); }
#define oDEFINE_FROM_STRING2(_T, invalid_value) bool from_string(_T* out_value, const char* src) { return detail::from_string_enum<_T>(out_value, src, invalid_value); }
#define oDEFINE_TO_FROM_STRING(_T) oDEFINE_TO_STRING(_T) oDEFINE_FROM_STRING(_T)

namespace ouro {

// Returns a const string representation of the specified value. This is most
// useful for enums when the object's value never changes.
template<typename T> const char* as_string(const T& value);

// Returns dst, or nullptr if there is a failure.
template<typename T> char* to_string(char* dst, size_t dst_size, const T& value);
template<typename T, size_t size> char* to_string(char (&dst)[size], const T& value) { return to_string<T>(dst, size, value); }

// Fills the specified address with the source string interpretted as that type.
// Returns true if out_value is valid, or false if out_value should not be used.
template<typename T> bool from_string(T* out_value, const char* src);

// Permutation of from_string for string-to-string (noop/copy) conversion
bool from_string(char* dst, size_t dst_size, const char* src);
template<size_t size> bool from_string(char (&dst)[size], const char* src) { return from_string(dst, size, src); }

namespace detail {

	// A default implementation that copies the as_string to the destination
	template<typename T> char* to_string(char* dst, size_t dst_size, const T& value) { return strlcpy(dst, as_string(value), dst_size) < dst_size ? dst : nullptr; }

	template<typename T> bool from_string_enum(T* out_value, const char* src, const T& invalid_value = T(0))
	{
		static_assert(std::is_enum<T>::value, "not enum");
		*out_value = invalid_value;
		for (const auto& e : enum_iterator<T>())
		{
			if (!_stricmp(src, as_string(e)))
			{
				*out_value = e;
				return true;
			}
		}
		return false;
	}

	template<typename ContainerT> char* to_string_container(char* dst, size_t dst_size, const ContainerT& _Container)
	{
		*dst = 0;
		typename ContainerT::const_iterator itLast = std::end(_Container) - 1;
		for (typename ContainerT::const_iterator it = std::begin(_Container); it != std::end(_Container); ++it)
		{
			if (!to_string(dst, dst_size, *it))
				return nullptr;

			size_t len = strlcat(dst, ",", dst_size);
			if (it != itLast && len >= dst_size)
				return nullptr;
			dst += len;
			dst_size -= len;
		}
		return dst;
	}

	template<typename ContainerT> bool from_string_container(ContainerT* out_container, const char* src)
	{
		char* ctx = nullptr;
		const char* tok = ouro::strtok(src, ",", &ctx);
		while (tok)
		{
			ContainerT::value_type obj;
			tok += strspn(tok, oWHITESPACE);
			if (!ouro::from_string(&obj, tok) || out_container->size() == out_container->max_size())
			{
				ouro::end_strtok(&ctx);
				return false;
			}
			out_container->push_back(obj);
			tok = ouro::strtok(nullptr, ",", &ctx);
		}
		return true;
	}

} // namespace detail


// Container/array support

template <typename T, typename AllocatorT> char* to_string(char* dst, size_t dst_size, const std::vector<T, AllocatorT>& vec) { return detail::to_string_container(dst, dst_size, vec); }
template <typename T, typename AllocatorT> bool from_string(std::vector<T, AllocatorT>* out_value, const char* src) { return detail::from_string_container(out_value, src); }

// Utility function that will convert a string of floats separated by whitespace
// into the specified array.
bool from_string_float_array(float* out_value, size_t num_values, const char* src);
bool from_string_double_array(double* out_value, size_t num_values, const char* src);

}

#endif
