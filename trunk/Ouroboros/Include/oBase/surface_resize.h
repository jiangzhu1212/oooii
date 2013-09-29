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
// API to change the size of a surface through filtering, clipping or padding.
// These APIs do not allocate memory so all buffers should be valid before
// calling any of these.
#pragma once
#ifndef oBase_surface_resize_h
#define oBase_surface_resize_h

#include <oBase/surface.h>

namespace ouro {
	namespace surface {

namespace filter
{	enum value {

	point,
	box,
	triangle,
	lanczos2, // sinc filter
	lanczos3, // sharper than lancsos2, but adds slight ringing
	filter_count,

};}

void resize(const info& _SourceInfo, const const_mapped_subresource& _Source, const info& _DestinationInfo, mapped_subresource* _pDestination, filter::value _Filter = filter::lanczos3);

void clip(const info& _SourceInfo, const const_mapped_subresource& _Source, const info& _DestinationInfo, mapped_subresource* _pDestination, int2 _SourceOffset = int2(0, 0));

void pad(const info& _SourceInfo, const const_mapped_subresource& _Source, const info& _DestinationInfo, mapped_subresource* _pDestination, int2 _DestinationOffset = int2(0, 0));

	} // namespace surface
} // namespace ouro

#endif
