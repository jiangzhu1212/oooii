// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.
#pragma once
#ifndef oSurface_convert_h
#define oSurface_convert_h

// Facade for various conversion formats. Not all are supported, but could be
// added over time.

#include <oSurface/surface.h>

namespace ouro { namespace surface {

// Converts the specified subresource into the destination subresource. This assumes
// all memory has been properly allocated. If a conversion is not supported this
// throws an exception.
void convert_subresource(const subresource_info& i
	, const const_mapped_subresource& src
	, format dst_format
	, const mapped_subresource& dst
	, const copy_option& option = copy_option::none);

// Converts the specified source into the specified destination. This assumes
// all memory has been properly allocated. If a conversion is not supported this
// throws an exception.
void convert(const info& src_info
	, const const_mapped_subresource& src
	, const info& dst_info
	, const mapped_subresource& dst
	, const copy_option& option = copy_option::none);

// This is a conversion in-place for RGB v. BGR and similar permutations.
void convert_swizzle(const info& i, const format& new_format, const mapped_subresource& mapped);

}}

#endif
