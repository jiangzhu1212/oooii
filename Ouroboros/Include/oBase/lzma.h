// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.

// Wrapper for the LZMA compression library v9.2.
// http://www.7-zip.org/sdk.html

#pragma once
#include <oBase/compression.h>

namespace ouro {

size_t lzma_compress(void* oRESTRICT dst, size_t dst_size
	, const void* oRESTRICT src, size_t src_size);

size_t lzma_decompress(void* oRESTRICT dst, size_t dst_size
	, const void* oRESTRICT src, size_t src_size);

}
