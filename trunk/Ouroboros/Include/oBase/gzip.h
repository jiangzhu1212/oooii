// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.

// Wrapper for the gzip RFC1952 library.
//http://tools.ietf.org/html/rfc1952

#pragma once
#include <oBase/compression.h>

namespace ouro {

size_t gzip_compress(void* oRESTRICT dst, size_t dst_size
	, const void* oRESTRICT src, size_t src_size);

size_t gzip_decompress(void* oRESTRICT dst, size_t dst_size
	, const void* oRESTRICT src, size_t src_size);

}
