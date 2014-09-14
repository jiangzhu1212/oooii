// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.

// API to change the size of a surface through filtering, clipping or padding.
// These APIs do not allocate memory so all buffers should be valid before
// calling any of these.

#pragma once
#include <oSurface/surface.h>

namespace ouro { namespace surface {

enum class filter : uchar
{
	point,
	box,
	triangle,
	lanczos2, // sinc filter
	lanczos3, // sharper than lancsos2, but adds slight ringing

	count,
};

void resize(const info& src_info, const const_mapped_subresource& src, const info& dst_info, const mapped_subresource& dst, const filter& f = filter::lanczos3);
void clip(const info& src_info, const const_mapped_subresource& src, const info& dst_info, const mapped_subresource& dst, uint2 src_offset = uint2(0, 0));
void pad(const info& src_info, const const_mapped_subresource& src, const info& dst_info, const mapped_subresource& dst, uint2 dst_offset = uint2(0, 0));

}}
