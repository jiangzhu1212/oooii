// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.

// Wrapper for the snappy compression library v1.0.
// https://code.google.com/p/snappy/

#pragma once
#include <oBase/compression.h>

namespace ouro {

size_t snappy_compress(void* oRESTRICT dst, size_t dst_size
	, const void* oRESTRICT src, size_t src_size);

size_t snappy_decompress(void* oRESTRICT dst, size_t dst_size
	, const void* oRESTRICT src, size_t src_size);

}
