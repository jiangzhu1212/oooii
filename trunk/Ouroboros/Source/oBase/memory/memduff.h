// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.
#pragma once
#ifndef oBase_memduff_h
#define oBase_memduff_h

#include <oBase/memory.h>
#include <oBase/byte.h>
#include <stdexcept>

namespace ouro { namespace detail {

// const void* version
template<typename T> void init_duffs_device_pointers_const(
	const void* oRESTRICT mem
	, size_t bytes
	, const int8_t* oRESTRICT* oRESTRICT pp_prefix
	, size_t* oRESTRICT p_prefix_bytes
	, const T* oRESTRICT* oRESTRICT pp_body
	, const int8_t* oRESTRICT* oRESTRICT pp_postfix
	, size_t* oRESTRICT p_postfix_bytes)
{
	*pp_prefix = (int8_t*)mem;
	*pp_body = (T*)byte_align(mem, sizeof(T));
	*p_prefix_bytes = byte_diff(*pp_prefix, mem);
	const T* end = byte_add(*pp_body, bytes - *p_prefix_bytes);
	*pp_postfix = (int8_t*)byte_align_down(end, sizeof(T));
	*p_postfix_bytes = byte_diff(end, *pp_postfix);

	if (byte_add(mem, bytes) != end) throw std::invalid_argument("end miscalculation");
	if (byte_add(mem, bytes) != byte_add(*pp_postfix, *p_postfix_bytes)) throw std::invalid_argument("postfix miscalculation");
}

// (non-const) void* version
template<typename T> void init_duffs_device_pointers(
	void* oRESTRICT mem
	, size_t bytes
	, int8_t* oRESTRICT* oRESTRICT pp_prefix
	, size_t* oRESTRICT p_prefix_bytes
	, T* oRESTRICT* oRESTRICT pp_body
	, int8_t* oRESTRICT* oRESTRICT pp_postfix
	, size_t* oRESTRICT p_postfix_bytes)
{
	init_duffs_device_pointers_const(mem, bytes, (const int8_t**)pp_prefix, p_prefix_bytes, (const T**)pp_body, (const int8_t**)pp_postfix, p_postfix_bytes);
}

}}

#endif
