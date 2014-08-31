// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.
// Wrapper for the snappy compression library v1.0.
// https://code.google.com/p/snappy/
#pragma once
#ifndef oBase_snappy_h
#define oBase_snappy_h

#include <oBase/compression.h>

namespace ouro {

size_t snappy_compress(void* oRESTRICT dst, size_t dst_size
	, const void* oRESTRICT src, size_t _SizeofSource);

size_t snappy_decompress(void* oRESTRICT dst, size_t dst_size
	, const void* oRESTRICT src, size_t _SizeofSource);

}

#endif
