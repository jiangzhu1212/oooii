/**************************************************************************
 * The MIT License                                                        *
 * Copyright (c) 2013 OOOii.                                              *
 * antony.arciuolo@oooii.com                                              *
 * kevin.myers@oooii.com                                                  *
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
// API to change the size of an oSURFACE through filtering, clipping or padding.
// These APIs do not allocate memory, so all buffers should be valid before
// calling any of these.
#pragma once
#ifndef oSurfaceResize_h
#define oSurfaceResize_h

#include <oBasis/oSurface.h>

enum oSURFACE_FILTER
{
	oSURFACE_FILTER_POINT,
	oSURFACE_FILTER_BOX,
	oSURFACE_FILTER_TRIANGLE,
	oSURFACE_FILTER_LANCZOS2, // sinc filter
	oSURFACE_FILTER_LANCZOS3, // another sinc filter, sharper than Lanczos2, but also adds a little ringing.

	oSURFACE_FILTER_COUNT,
};

oAPI bool oSurfaceResize(const oSURFACE_DESC& _SrcDesc, const oSURFACE_CONST_MAPPED_SUBRESOURCE& _SrcMap, const oSURFACE_DESC& _DstDesc, oSURFACE_MAPPED_SUBRESOURCE* _DstMap, oSURFACE_FILTER _Filter = oSURFACE_FILTER_LANCZOS3);

oAPI bool oSurfaceClip(const oSURFACE_DESC& _SrcDesc, const oSURFACE_CONST_MAPPED_SUBRESOURCE& _SrcMap, const oSURFACE_DESC& _DstDesc, oSURFACE_MAPPED_SUBRESOURCE* _DstMap, int2 _SrcOffset = int2(0,0));

oAPI bool oSurfacePad(const oSURFACE_DESC& _SrcDesc, const oSURFACE_CONST_MAPPED_SUBRESOURCE& _SrcMap, const oSURFACE_DESC& _DstDesc, oSURFACE_MAPPED_SUBRESOURCE* _DstMap, int2 _DstOffset = int2(0,0));

#endif
