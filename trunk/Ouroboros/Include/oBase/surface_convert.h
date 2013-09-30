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
#pragma once
#ifndef oBase_surface_convert_h
#define oBase_surface_convert_h

#include <oBase/surface.h>

namespace ouro {
	namespace surface {

// Converts the specified subresource into the destination subresource.
void convert_subresource(const subresource_info& _SubresourceInfo
	, format _SourceFormat, const const_mapped_subresource& _Source
	, format _DestinationFormat, mapped_subresource* _pDestination);

// Converts the specified source into the specified destination. This assumes
// all memory has been properly allocated. If a conversion is not supported this
// throws an exception.
void convert(const info& _SourceInfo, const const_mapped_subresource& _Source
	, const info& _DestinationInfo, mapped_subresource* _pDestination);

// This is a conversion in-place for RGB v. BGR and similar permutations.
void convert_swizzle(const info& _SurfaceInfo, surface::format _NewFormat, mapped_subresource* _pSurface);

	} // namespace surface
} // namespace ouro

#endif
