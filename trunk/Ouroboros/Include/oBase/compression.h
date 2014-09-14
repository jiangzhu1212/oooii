// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.

// All compression wrappers should comply with this API definition. Compression
// wrappers are named for their algorithm (i.e. gzip.h or snappy.h).

#pragma once
#include <oCompiler.h>

namespace ouro {

// If dst is nullptr, return an estimation of the size dst should be. If dst is 
// valid, return the actual compressed size. In failure in either case, throw.
typedef size_t (*compress_fn)(void* oRESTRICT dst, size_t dst_size
	, const void* oRESTRICT src, size_t src_size);

// Returns the uncompressed size of src. Call this first with 
// dst = nullptr to get the size value to properly allocate a 
// destination buffer then pass that as dst to finish the decompres-
// sion. NOTE: src must have been a buffer produced with compress_fn 
// because its implementation may have tacked on extra information to the buffer 
// and thus only the paired implementation of decompress knows how to access 
// such a buffer correctly. (Implementation note: if an algorithm doesn't store 
// the uncompressed size, then it should be tacked onto the compressed buffer 
// and accessible by this function to comply with the API specification.)
typedef size_t (*decompress_fn)(void* oRESTRICT dst, size_t dst_size
	, const void* oRESTRICT src, size_t src_size);

}
