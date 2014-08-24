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
// Facade for various conversion formats. Not all are supported, but could be
// added over time.
#pragma once
#ifndef oSurface_convert_h
#define oSurface_convert_h

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
